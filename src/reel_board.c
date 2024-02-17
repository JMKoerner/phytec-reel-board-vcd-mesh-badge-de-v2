/*
 * Copyright (c) 2018 Phytec Messtechnik GmbH
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Aenderung: Deutsche Texte auf ePapier: JK LUG-Noris 01/2019
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <display/cfb.h>
#include <sys/printk.h>
#include <drivers/flash.h>
#include <drivers/sensor.h>
#include <drivers/pwm.h>

#include <zephyr/types.h>

#include <stddef.h>
#include <sys/util.h>

#include <drivers/display.h>
#include <lvgl.h>
#include "qrcodegen.h"

#include <string.h>
#include <stdio.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/access.h>

#include "mesh.h"
#include "board.h"

// ##############################################################################################
// Für Board 1507.1 "#define beeper" auskommentieren (PWM), PWM-LED leuchtet dauerhaft!  31.12.21
// #define beeper
// ##############################################################################################

#if defined(DT_ALIAS_PWM_LED0_PWMS_CONTROLLER) && defined(DT_ALIAS_PWM_LED0_PWMS_CHANNEL)
/* get the defines from dt (based on alias 'pwm-led0') */
#define PWM_DRIVER	DT_ALIAS_PWM_LED0_PWMS_CONTROLLER
#define PWM_CHANNEL	DT_ALIAS_PWM_LED0_PWMS_CHANNEL
#else
#error "Choose supported PWM driver"
#endif

// #define PWM_POLARITY_INVERTED

enum font_size {
	FONT_BIG = 2,
	FONT_MEDIUM = 1,
	FONT_SMALL = 0,
};

enum screen_ids {
	SCREEN_TUX = 0,
	SCREEN_VCD = 1,
	SCREEN_SENSORS = 2,
	SCREEN_STATS = 3,
	SCREEN_LUGTEXT = 4,
	SCREEN_LAST,
};

struct font_info {
	u8_t columns;
} fonts[] = {
	[FONT_BIG] =    { .columns = 12 },
	[FONT_MEDIUM] = { .columns = 16 },
	[FONT_SMALL] =  { .columns = 25 },
};

static struct device *pwm_dev;

#define LONG_PRESS_TIMEOUT K_SECONDS(1)

#define STAT_COUNT 128

#define EDGE (GPIO_INT_EDGE | GPIO_INT_DOUBLE_EDGE)

#ifdef DT_ALIAS_SW0_GPIOS_FLAGS
#define PULL_UP DT_ALIAS_SW0_GPIOS_FLAGS
#else
#define PULL_UP 0
#endif

static struct device *epd_dev;
static struct device *display;
static bool pressed;
static bool stat_on = false;
static u8_t screen_id = SCREEN_TUX;
static struct device *gpio;
static struct k_delayed_work epd_work;
static struct k_delayed_work long_press_work;
static char str_buf[256];
static bool vcd_renew = true;
static bool tux_renew  = true;
static u8_t screen_id_neu = 0;
static u8_t screen_id_alt = 0;

static char vcd_name[20]="";
static char vname[20]="";
static char vcd_prof[CONFIG_BT_DEVICE_NAME_MAX]="";
static char buf2[CONFIG_BT_DEVICE_NAME_MAX]="";

static struct {
	struct device *dev;
	const char *name;
	u32_t pin;
} leds[] = {
	{ .name = DT_ALIAS_LED0_GPIOS_CONTROLLER, .pin = DT_ALIAS_LED0_GPIOS_PIN, },
	{ .name = DT_ALIAS_LED1_GPIOS_CONTROLLER, .pin = DT_ALIAS_LED1_GPIOS_PIN, },
	{ .name = DT_ALIAS_LED2_GPIOS_CONTROLLER, .pin = DT_ALIAS_LED2_GPIOS_PIN, },
};

struct k_delayed_work led_timer;

static size_t print_line(enum font_size font_size, int row, const char *text,
			 size_t len, bool center)
{
	u8_t font_height, font_width;
	u8_t line[fonts[FONT_SMALL].columns + 1];
	int pad;

	cfb_framebuffer_set_font(epd_dev, font_size);

	len = MIN(len, fonts[font_size].columns);
	memcpy(line, text, len);
	line[len] = '\0';

	if (center) {
		pad = (fonts[font_size].columns - len) / 2U;
	} else {
		pad = 0;
	}

	cfb_get_font_size(epd_dev, font_size, &font_width, &font_height);

	if (cfb_print(epd_dev, line, font_width * pad, font_height * row)) {
		printk("Failed to print a string\n");
	}

	return len;
}

static size_t get_len(enum font_size font, const char *text)
{
	const char *space = NULL;
	size_t i;

	for (i = 0; i <= fonts[font].columns; i++) {
		switch (text[i]) {
		case '\n':
		case '\0':
			return i;
		case ' ':
			space = &text[i];
			break;
		default:
			continue;
		}
	}

	/* If we got more characters than fits a line, and a space was
	 * encountered, fall back to the last space.
	 */
	if (space) {
		return space - text;
	}

	return fonts[font].columns;
}

void sound_activ(u32_t period)
{
#ifdef beeper

/* Hinweis: Für Zephyr 2.1 rc1 ist der letzte Parameter (hier die 0) bei pwm_pin_set_usec wegzulassen */

/* Sound ein , aktiv nur für 1507.3, bei 1507.1 leuchtet die PWM-LED auf Dauer !!! */
if (pwm_pin_set_usec(pwm_dev, PWM_CHANNEL,
		period, period / 2U, 0)) {   // Parameter 0 für normal; 1 für invertiert
		printk("pwm pin set fails\n");
		return;
		}
k_sleep(K_MSEC(150));  // Verzögerung um 0.15 sec

/* Sound aus */
pwm_pin_set_usec(pwm_dev, PWM_CHANNEL, 0, 0, 0);
k_sleep(K_MSEC(100));  // Warten/Verzögerung um 0.1 sec
#else
printk("PWM (Beeper) nicht aktiv!\n");
#endif
}

void board_blink_leds(void)
{
	k_delayed_work_submit(&led_timer, K_MSEC(100));
}

void name_select(void)
{
	/* Auftrennung BLE-Puffer in Vorname, Nachname, Profession */
	int i, t, s;  s = 0;

	strncpy(buf2, bt_get_name(), sizeof(buf2) - 1);
	buf2[sizeof(buf2) - 1] = '\0';

	/* Weist Vornamen+Nachnamen vc_name zu */
	for (i = 0; buf2[i] != ','; i++) {
		if (i < 21) {		// Max. Länge Vorname+Nachname 20 Zeichen
			vcd_name[i] = buf2[i];
			s= i + 1;
		}
	}

	/* Weist Profession vcd_prof zu */
	for (t = s + 1; buf2[t] != '\0'; t++) {
		vcd_prof[t-s-1] = buf2[t];
	}

	/* Weist Vornamen alleine vname zu */
	for (i = 0; buf2[i] != ' '; i++) {
		if (i < 21) {		// Max. Länge Vorname 20 Zeichen
			vname[i] = buf2[i];
//			printk("i=  %d\n",i);
		}
	vname[i+1] = '\0';
	}
}

void board_show_text(const char *text, bool center, s32_t duration)
{
	int i;

	cfb_framebuffer_clear(epd_dev, false);

	for (i = 0; i < 3; i++) {
		size_t len;

		while (*text == ' ' || *text == '\n') {
			text++;
		}

		len = get_len(FONT_BIG, text);
		if (!len) {
			break;
		}

		text += print_line(FONT_BIG, i, text, len, center);
		if (!*text) {
			break;
		}
	}

	cfb_framebuffer_finalize(epd_dev);

	if (duration != K_FOREVER) {
		k_delayed_work_submit(&epd_work, duration);
	}
}

static struct stat {
	u16_t addr;
	char name[9];
	u8_t min_hops;
	u8_t max_hops;
	u16_t hello_count;
	u16_t heartbeat_count;
} stats[STAT_COUNT] = {
	[0 ... (STAT_COUNT - 1)] = {
		.min_hops = BT_MESH_TTL_MAX,
		.max_hops = 0,
	},
};

static u32_t stat_count;

#define NO_UPDATE -1

static int add_hello(u16_t addr, const char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(stats); i++) {
		struct stat *stat = &stats[i];

		if (!stat->addr) {
			stat->addr = addr;
			strncpy(stat->name, name, sizeof(stat->name) - 1);
			stat->hello_count = 1U;
			stat_count++;
			return i;
		}

		if (stat->addr == addr) {
			/* Update name, incase it has changed */
			strncpy(stat->name, name, sizeof(stat->name) - 1);

			if (stat->hello_count < 0xffff) {
				stat->hello_count++;
				return i;
			}

			return NO_UPDATE;
		}
	}

	return NO_UPDATE;
}

static int add_heartbeat(u16_t addr, u8_t hops)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(stats); i++) {
		struct stat *stat = &stats[i];

		if (!stat->addr) {
			stat->addr = addr;
			stat->heartbeat_count = 1U;
			stat->min_hops = hops;
			stat->max_hops = hops;
			stat_count++;
			return i;
		}

		if (stat->addr == addr) {
			if (hops < stat->min_hops) {
				stat->min_hops = hops;
			} else if (hops > stat->max_hops) {
				stat->max_hops = hops;
			}

			if (stat->heartbeat_count < 0xffff) {
				stat->heartbeat_count++;
				return i;
			}

			return NO_UPDATE;
		}
	}

	return NO_UPDATE;
}

void board_add_hello(u16_t addr, const char *name)
{
	u32_t sort_i;

	sort_i = add_hello(addr, name);
	if (sort_i != NO_UPDATE) {
	}
}

void board_add_heartbeat(u16_t addr, u8_t hops)
{
	u32_t sort_i;

	sort_i = add_heartbeat(addr, hops);
	if (sort_i != NO_UPDATE) {
	}
}

static void show_lugtext(void)
{
	int len, line = 0;
	char str[32];

	cfb_framebuffer_clear(epd_dev, false);

	len = snprintk(str, sizeof(str),
		       "Die LUG Nbg trifft sich:");
	print_line(FONT_SMALL, line++, str, len, false);

	len = snprintk(str, sizeof(str),
		       "========================");
	print_line(FONT_SMALL, line++, str, len, false);

	len = snprintk(str, sizeof(str),
		       "1.und 3.Donnerstag:");
	print_line(FONT_SMALL, line++, str, len, false);

	len = snprintk(str, sizeof(str),
		       "  =>  Beim Theo");
	print_line(FONT_SMALL, line++, str, len, false);

	len = snprintk(str, sizeof(str),
		       "2.und 4.Donnerstag:");
	print_line(FONT_SMALL, line++, str, len, false);

	len = snprintk(str, sizeof(str),
		       "  =>  Virtuell mit Jitsi");
	print_line(FONT_SMALL, line++, str, len, false);

	len = snprintk(str, sizeof(str),
		       "Infos: www.lug-noris.de");
	print_line(FONT_SMALL, line++, str, len, false);

	cfb_framebuffer_finalize(epd_dev);
}

static void show_statistics(void)
{
	int top[4] = { -1, -1, -1, -1 };
	int len, i, line = 0;
	struct stat *stat;
	char str[32];

	cfb_framebuffer_clear(epd_dev, false);

	len = snprintk(str, sizeof(str),
		       "Eigene Adresse: 0x%04x", mesh_get_addr());
	print_line(FONT_SMALL, line++, str, len, false);

	len = snprintk(str, sizeof(str),
		       "Knotenzaehler : %u", stat_count + 1);
	print_line(FONT_SMALL, line++, str, len, false);

	/* Find the top sender */
	for (i = 0; i < ARRAY_SIZE(stats); i++) {
		int j;

		stat = &stats[i];
		if (!stat->addr) {
			break;
		}

		if (!stat->hello_count) {
			continue;
		}

		for (j = 0; j < ARRAY_SIZE(top); j++) {
			if (top[j] < 0) {
				top[j] = i;
				break;
			}

			if (stat->hello_count <= stats[top[j]].hello_count) {
				continue;
			}

			/* Move other elements down the list */
			if (j < ARRAY_SIZE(top) - 1) {
				memmove(&top[j + 1], &top[j],
					((ARRAY_SIZE(top) - j - 1) *
					 sizeof(top[j])));
			}

			top[j] = i;
			break;
		}
	}

	if (stat_count > 0) {
		len = snprintk(str, sizeof(str), "Meiste Nachrichten:");
		print_line(FONT_SMALL, line++, str, len, false);

		for (i = 0; i < ARRAY_SIZE(top); i++) {
			if (top[i] < 0) {
				break;
			}

			stat = &stats[top[i]];

			len = snprintk(str, sizeof(str), "%-3u 0x%04x %s",
				       stat->hello_count, stat->addr,
				       stat->name);
			print_line(FONT_SMALL, line++, str, len, false);
		}
	}

	cfb_framebuffer_finalize(epd_dev);
}

static void show_sensors_data(s32_t interval)
{
	struct sensor_value val[3];
	u8_t line = 0U;
	u16_t len = 0U;

	cfb_framebuffer_clear(epd_dev, false);

	/* hdc1010 */
	if (get_hdc1010_val(val)) {
		goto _error_get;
	}

	len = snprintf(str_buf, sizeof(str_buf), "Temperatur    :  %d.%d C\n",
		       val[0].val1, val[0].val2 / 100000);
	print_line(FONT_SMALL, line++, str_buf, len, false);

	len = snprintf(str_buf, sizeof(str_buf), "Feuchtigkeit  :  %d%%\n",
		       val[1].val1);
	print_line(FONT_SMALL, line++, str_buf, len, false);

	/* mma8652 */
	if (get_mma8652_val(val)) {
		goto _error_get;
	}

	len = snprintf(str_buf, sizeof(str_buf), "B-Sensor - AX :%7.3f\n",
		       sensor_value_to_double(&val[0]));
	print_line(FONT_SMALL, line++, str_buf, len, false);

	len = snprintf(str_buf, sizeof(str_buf), "         - AY :%7.3f\n",
		       sensor_value_to_double(&val[1]));
	print_line(FONT_SMALL, line++, str_buf, len, false);

	len = snprintf(str_buf, sizeof(str_buf), "         - AZ :%7.3f\n",
		       sensor_value_to_double(&val[2]));
	print_line(FONT_SMALL, line++, str_buf, len, false);

	/* apds9960 */
	if (get_apds9960_val(val)) {
		goto _error_get;
	}

	len = snprintf(str_buf, sizeof(str_buf), "Lichtstaerke  :  %d\n", val[0].val1);
	print_line(FONT_SMALL, line++, str_buf, len, false);
	len = snprintf(str_buf, sizeof(str_buf), "Abstand       :  %d\n", val[1].val1);
	print_line(FONT_SMALL, line++, str_buf, len, false);

	cfb_framebuffer_finalize(epd_dev);

	k_delayed_work_submit(&epd_work, interval);

	return;

_error_get:
	printk("Failed to get sensor data or print a string\n");
}

static void show_vcd(void)
{
	cfb_framebuffer_clear(epd_dev, false);

 	/* Aufruf über LVGL (Grafikmode), vcd_renew nur beim Start auf true: */
	vcd_epaper(vcd_name, vcd_prof, vcd_renew);

	/* Handshake zwischen den beiden Modulen screen_vcd und screen_tux */
	tux_renew  = true;

	/* Wenn im Bluetooth-Mode, Display immer direkt beschreiben */
	if (!mesh_is_initialized()) {
		vcd_renew = true;
	} else {
		vcd_renew = false;
	}
}

static void show_tux(void)
{
	cfb_framebuffer_clear(epd_dev, false);

 	/* Aufruf über LVGL (Grafikmode), tux_renew nur beim Start auf true: */
	vcd_epaper_tux(vname, tux_renew);

	/* Handshake zwischen den beiden Modulen screen_vcd und screen_tux */
	vcd_renew = true;

	/* Wenn im Bluetooth-Mode, Display immer direkt beschreiben */
	if (!mesh_is_initialized()) {
		tux_renew = true;
	} else {
		tux_renew = false;
	}
}

static bool button_is_pressed(void)
{
	u32_t val;

	gpio_pin_read(gpio, DT_ALIAS_SW0_GPIOS_PIN, &val);

	return !val;
}

static void epd_update(struct k_work *work)
{
	switch (screen_id) {
	case SCREEN_LUGTEXT:
		show_lugtext();
		return;
	case SCREEN_STATS:
		show_statistics();
		return;
	case SCREEN_SENSORS:
		show_sensors_data(K_SECONDS(2));
		return;
	case SCREEN_VCD:
		name_select();
		show_vcd();
		return;
	case SCREEN_TUX:
		name_select();
		show_tux();
		return;
	}
}

static void long_press(struct k_work *work)
{
	/* Treat as release so actual release doesn't send messages */
	pressed = false;
	screen_id = (screen_id + 1) % SCREEN_LAST;
	printk("Change screen to id = %d\n", screen_id);
	board_refresh_display();
}

static void button_interrupt(struct device *dev, struct gpio_callback *cb,
			     u32_t pins)
{
	u32_t uptime2  = k_uptime_get_32();

	if (button_is_pressed() == pressed) {
		return;
	}

	pressed = !pressed;
	printk("Button %s\n", pressed ? "pressed" : "released");

	if (pressed) {
		k_delayed_work_submit(&long_press_work, LONG_PRESS_TIMEOUT);
		return;
	}

	k_delayed_work_cancel(&long_press_work);

	if (!mesh_is_initialized()) {
		return;
	}

	/* Short press for views */
	switch (screen_id) {
	case SCREEN_SENSORS:
	case SCREEN_STATS:
		stat_on = true;
		return;
	case SCREEN_TUX:
	case SCREEN_VCD:
		if (pins & BIT(DT_ALIAS_SW0_GPIOS_PIN)) {
			u32_t uptime = k_uptime_get_32();
			static u32_t bad_count, press_ts, t_on;

			screen_id_neu = screen_id;

			if (uptime - press_ts < 500) {
				bad_count++;
			} else {
				bad_count = 0U;
			}

			if (bad_count) {
				if (bad_count > 5) {
					mesh_send_baduser();
					bad_count = 0U;
				} else {
					printk("Ignoring press\n");
				}
			} else {
				if (!pressed) {
					if ((screen_id_neu-screen_id_alt) == 0) {
						t_on = uptime - uptime2;
//						printk("Delta Zeit: (t_on  %d)\n", t_on);
						if ((t_on > 0U) && (t_on < 10U)) {
							if (!stat_on) {
								mesh_send_hello();
							} else {
								stat_on = false;
							}
						}
					}
				}
			}
			screen_id_alt = screen_id_neu;
			press_ts = uptime;
		}
		uptime2 = k_uptime_get_32();
		return;
	default:
		return;
	}
}

static int configure_button(void)
{
	static struct gpio_callback button_cb;

	gpio = device_get_binding(DT_ALIAS_SW0_GPIOS_CONTROLLER);
	if (!gpio) {
		return -ENODEV;
	}

	gpio_pin_configure(gpio, DT_ALIAS_SW0_GPIOS_PIN,
			   (GPIO_DIR_IN | GPIO_INT |  PULL_UP | EDGE));

	gpio_init_callback(&button_cb, button_interrupt, BIT(DT_ALIAS_SW0_GPIOS_PIN));
	gpio_add_callback(gpio, &button_cb);

	gpio_pin_enable_callback(gpio, DT_ALIAS_SW0_GPIOS_PIN);

	return 0;
}

static void led_timeout(struct k_work *work)
{
	static int led_cntr;
	int i;

	/* Disable all LEDs */
	for (i = 0; i < ARRAY_SIZE(leds); i++) {
		gpio_pin_write(leds[i].dev, leds[i].pin, 1);
	}

	/* Stop after 5 iterations */
	if (led_cntr >= (ARRAY_SIZE(leds) * 5)) {
		led_cntr = 0;
		return;
	}

	/* Select and enable current LED */
	i = led_cntr++ % ARRAY_SIZE(leds);
	gpio_pin_write(leds[i].dev, leds[i].pin, 0);

	k_delayed_work_submit(&led_timer, K_MSEC(100));
}

static int configure_leds(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(leds); i++) {
		leds[i].dev = device_get_binding(leds[i].name);
		if (!leds[i].dev) {
			printk("Failed to get %s device\n", leds[i].name);
			return -ENODEV;
		}

		gpio_pin_configure(leds[i].dev, leds[i].pin, GPIO_DIR_OUT);
		gpio_pin_write(leds[i].dev, leds[i].pin, 1);

	}

	k_delayed_work_init(&led_timer, led_timeout);
	return 0;
}

static int erase_storage(void)
{
	struct device *dev;

	dev = device_get_binding(DT_FLASH_DEV_NAME);

	flash_write_protection_set(dev, false);

	return flash_erase(dev, DT_FLASH_AREA_STORAGE_OFFSET,
			   DT_FLASH_AREA_STORAGE_SIZE);
}

void board_refresh_display(void)
{
	k_delayed_work_submit(&epd_work, K_NO_WAIT);
}

int board_init(void)
{
	epd_dev = device_get_binding(DT_INST_0_SOLOMON_SSD16XXFB_LABEL);
	if (epd_dev == NULL) {
		printk("SSD16XX device not found\n");
		return -ENODEV;
	}

	pwm_dev = device_get_binding(PWM_DRIVER);
	if (pwm_dev == NULL) {
		printk("Cannot find %s!\n", PWM_DRIVER);
		return -EIO;
	}


	display=device_get_binding(CONFIG_LVGL_DISPLAY_DEV_NAME);
	if (display == NULL) {
		printk("LVGL Display not ok\n");
		return -ENODEV;
	}

	if (cfb_framebuffer_init(epd_dev)) {
		printk("Framebuffer initialization failed\n");
		return -EIO;
	}

	cfb_framebuffer_clear(epd_dev, true);

	if (configure_button()) {
		printk("Failed to configure button\n");
		return -EIO;
	}

	if (configure_leds()) {
		printk("LED init failed\n");
		return -EIO;
	}

	k_delayed_work_init(&epd_work, epd_update);
	k_delayed_work_init(&long_press_work, long_press);

	pressed = button_is_pressed();
	if (pressed) {
		printk("Erasing storage\n");
		board_show_text("Geraet Ruecksetzen", false, K_SECONDS(4));
		erase_storage();
	}

	return 0;
}
