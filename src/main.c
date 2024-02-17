/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Änderung: Deutsche Texte auf ePapier: JK LUG Noris 01/2019
 */

#include <zephyr.h>
#include <zephyr/types.h>
#include <sys/printk.h>
#include <device.h>

#include <string.h>
#include <stddef.h>
#include <sys/util.h>
#include <stdio.h>

#include <drivers/display.h>
#include <display/cfb.h>
#include <lvgl.h>
#include "qrcodegen.h"

#include <settings/settings.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/gatt.h>
#include <drivers/sensor.h>
#include <drivers/pwm.h>

#include "mesh.h"
#include "board.h"

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
};

static ssize_t read_name(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 void *buf, u16_t len, u16_t offset)
{
	const char *value = bt_get_name();

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 strlen(value));
}

static ssize_t write_name(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			  const void *buf, u16_t len, u16_t offset,
			  u8_t flags)
{
	char name[CONFIG_BT_DEVICE_NAME_MAX];
	char buf1[CONFIG_BT_DEVICE_NAME_MAX];

	int Umlaut = 0xc3;
	int i1, t1, s1;  i1=0; t1=0; s1=0;

	int err;

	if (offset) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (len >= CONFIG_BT_DEVICE_NAME_MAX) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	memcpy(name, buf, len);
	name[len] = '\0';

	/* Umwandlung UTF-8 nach ASCII */
	strncpy(buf1, name, sizeof(buf1) - 1);
	buf1[sizeof(buf1) - 1] = '\0';
	for (i1 = 0; buf1[i1] != '\0'; i1++) {
		if (buf1[i1] == Umlaut) {
			buf1[i1] = buf1[i1 + 1] + 0x40;
			s1 = i1 + 1;
			/* Shift links um 1 Byte */
			for (t1 = s1; buf1[t1] != '\0'; t1++) {
				buf1[t1] = buf1[t1 + 1];
			}
		}
	}

	err = bt_set_name(buf1);

	if (err) {
		return BT_GATT_ERR(BT_ATT_ERR_INSUFFICIENT_RESOURCES);
	}

	board_refresh_display();

	return len;
}

/*
static struct bt_uuid_128 name_uuid = BT_UUID_INIT_128(
	0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
	0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

static struct bt_uuid_128 name_enc_uuid = BT_UUID_INIT_128(
	0xf1, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
	0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);
*/

static struct bt_uuid_128 name_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0));

static struct bt_uuid_128 name_enc_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1));


#define CPF_FORMAT_UTF8 0x19

static const struct bt_gatt_cpf name_cpf = {
	.format = CPF_FORMAT_UTF8,
};

/* Vendor Primary Service Declaration */
BT_GATT_SERVICE_DEFINE(name_svc,
	/* Vendor Primary Service Declaration */
	BT_GATT_PRIMARY_SERVICE(&name_uuid),
	BT_GATT_CHARACTERISTIC(&name_enc_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT,
			       read_name, write_name, NULL),
	BT_GATT_CUD("Badge Name", BT_GATT_PERM_READ),
	BT_GATT_CPF(&name_cpf),
);

static void passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char buf[20];

	snprintk(buf, sizeof(buf), "Passkey:\n%06u", passkey);

	printk("%s\n", buf);
	board_show_text(buf, false, K_FOREVER);
}

static void passkey_cancel(struct bt_conn *conn)
{
	printk("Abbruch\n");
}

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
	printk("Pairing Komplett\n");

	sound_activ(1000);

	board_show_text("Pairing Komplett", false, K_SECONDS(2));
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
	printk("Pairing Fehler (%d)\n", reason);
	sound_activ(2000); sound_activ(2000);
	board_show_text("Pairing Fehler", false, K_SECONDS(2));
}

const struct bt_conn_auth_cb auth_cb = {
	.passkey_display = passkey_display,
	.cancel = passkey_cancel,
	.pairing_complete = pairing_complete,
	.pairing_failed = pairing_failed,
};

static void connected(struct bt_conn *conn, u8_t err)
{
	printk("Verbindung (err 0x%02x)\n", err);

	if (err) {
		sound_activ(2000); sound_activ(2000);
		board_show_text("Verbindungsfehler", false, K_SECONDS(2));
	} else {
		sound_activ(1000);
		board_show_text("Verbunden", false, K_FOREVER);
	}
}

static void disconnected(struct bt_conn *conn, u8_t reason)
{
	printk("Getrennt (Grund 0x%02x)\n", reason);

	if (strcmp(CONFIG_BT_DEVICE_NAME, bt_get_name()) &&
	    !mesh_is_initialized()) {
		/* Mesh will take over advertising control */
		bt_le_adv_stop();
		mesh_start();
	} else {
		board_show_text("Getrennt", false, K_SECONDS(2));
	}
}

static struct bt_conn_cb conn_cb = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth Init Fehler (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialisiert\n");


	err = mesh_init();
	if (err) {
		printk("Mesh Initialisierungsfehler (err %d)\n", err);
		return;
	}

	printk("Mesh initialisiert\n");

	bt_conn_cb_register(&conn_cb);
	bt_conn_auth_cb_register(&auth_cb);

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}


	if (!mesh_is_initialized()) {
		/* Start advertising */
		err = bt_le_adv_start(BT_LE_ADV_CONN_NAME,
				      ad, ARRAY_SIZE(ad), NULL, 0);
		if (err) {
			printk("Advertising Startfehler (err %d)\n", err);
			return;
		}
	} else {
		printk("Ist schon aktiv\n");
	}

	board_refresh_display();

	printk("Board gestartet\n");
}

void main(void)
{
	int err;

	err = board_init();
	if (err) {
		printk("Board Init Fehler (err %d)\n", err);
		return;
	}

	printk("Start Board Demo\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth Init Fehler (err %d)\n", err);
		return;
	}

	err = periphs_init();
	if (err) {
		printk("Perpherie Initialisierungsfehler (err %d)\n", err);
		return;
	}
}
