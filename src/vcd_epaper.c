/*
 * Copyright (c) 2019 PHYTEC Messtechnik GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Erweiterungen von J.Körner LUG Nürnberg 12/21
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <sys/util.h>
#include <stdio.h>

#include <drivers/display.h>
#include <display/cfb.h>
#include <lvgl.h>

#include "qrcodegen.h"
#include "vcd.h"

#define CANVAS_HEIGHT 300
#define CANVAS_WIDTH  300
#define CANVAS_H 20
#define CANVAS_W 250
#define SCALE_QR 2
#define SCALE_SM 2

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif
#ifndef LV_ATTRIBUTE_IMG_TUX_MONO_8
#define LV_ATTRIBUTE_IMG_TUX_MONO_8
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_IMG_TUX_MONO_8 uint8_t tux_mono_8_map[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf8, 0x7f, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3e, 0x3f, 0xe0, 0x1f, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x7c, 0x1f, 0xe0, 0x1f, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x78, 0x0f, 0xc0, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x78, 0x0f, 0xc2, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x70, 0x8f, 0x87, 0x87, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x31, 0xc7, 0x87, 0x87, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x33, 0xc7, 0x8f, 0x87, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x33, 0xc7, 0x8f, 0xc7, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x33, 0xff, 0xef, 0xc7, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x39, 0xe0, 0x1f, 0x87, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x39, 0xc0, 0x07, 0x8f, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3d, 0x80, 0x01, 0xcf, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x07, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x47, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x87, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x01, 0x07, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x04, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x7c, 0x40, 0x10, 0x17, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x7e, 0x10, 0xc0, 0x67, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x7d, 0x80, 0x01, 0xc3, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xf8, 0xc0, 0x06, 0x03, 0xff, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xf8, 0x60, 0x08, 0x03, 0xff, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0xf8, 0x18, 0x70, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0xf8, 0x07, 0x80, 0x01, 0xff, 0xe0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x7f, 0xfc, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0xf0, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x1f, 0xe0, 0x06, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0xe0, 0x60, 0x08, 0x00, 0x3f, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x7f, 0xc6, 0x00, 0x08, 0x00, 0x3f, 0xff, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x08, 0x00, 0x1f, 0xff, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x04, 0x00, 0x1f, 0x7f, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x14, 0x00, 0x0f, 0xbf, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x38, 0x00, 0x04, 0x00, 0x04, 0x00, 0x0f, 0xdf, 0xf8, 0x00, 0x00,
  0x00, 0x00, 0x20, 0x00, 0x00, 0x80, 0x00, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x00,
  0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x07, 0xef, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x03, 0xf7, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x03, 0xf7, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0x10, 0x04, 0x00, 0xff, 0xfe, 0x00, 0x01, 0xfb, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0x30, 0x80, 0x0f, 0xff, 0xff, 0x00, 0x01, 0xfb, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x30, 0x00, 0x7f, 0xff, 0xff, 0x00, 0x00, 0xfd, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x70, 0x07, 0xff, 0xff, 0xff, 0x00, 0x00, 0xfd, 0xff, 0x80, 0x00,
  0x00, 0x00, 0x78, 0x7f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x7c, 0xff, 0x80, 0x00,
  0x00, 0x00, 0xfb, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x7e, 0xff, 0xc0, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x7e, 0xff, 0xc0, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x3e, 0xff, 0xc0, 0x00,
  0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x3e, 0xff, 0xc0, 0x00,
  0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x3e, 0x7f, 0xe0, 0x00,
  0x00, 0x01, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x1e, 0x7f, 0xe0, 0x00,
  0x00, 0x03, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x1e, 0x7f, 0xe0, 0x00,
  0x00, 0x03, 0xfb, 0xff, 0xff, 0xff, 0xef, 0xe0, 0x00, 0x1c, 0xff, 0xe0, 0x00,
  0x00, 0x03, 0xfb, 0xff, 0xff, 0xff, 0xe7, 0xe0, 0x00, 0x0c, 0xff, 0xe0, 0x00,
  0x00, 0x03, 0xf9, 0xff, 0xff, 0xff, 0xe3, 0xe0, 0x00, 0x00, 0xff, 0xe0, 0x00,
  0x00, 0x03, 0xf9, 0xff, 0xff, 0xbf, 0xe1, 0xe0, 0x00, 0x1c, 0x3f, 0xe0, 0x00,
  0x00, 0x07, 0xf9, 0xff, 0xff, 0x1f, 0xe0, 0xf0, 0x00, 0x7f, 0x8f, 0xe0, 0x00,
  0x00, 0x07, 0xfd, 0xff, 0xff, 0x1f, 0xe0, 0xf0, 0x00, 0x7f, 0xe7, 0xe0, 0x00,
  0x00, 0x07, 0xfc, 0xfd, 0xff, 0x0f, 0xc0, 0x70, 0x00, 0xff, 0xfb, 0xe0, 0x00,
  0x00, 0x07, 0xfe, 0xfc, 0xff, 0x07, 0xc0, 0x38, 0x00, 0xff, 0xfd, 0xe0, 0x00,
  0x00, 0x07, 0xff, 0xfc, 0x7f, 0x03, 0xc0, 0x18, 0x03, 0xff, 0xfd, 0xe0, 0x00,
  0x00, 0x07, 0xff, 0xf8, 0x3f, 0x03, 0xc0, 0x08, 0x07, 0xff, 0xfe, 0xe0, 0x00,
  0x00, 0x06, 0x07, 0xf8, 0x3f, 0x01, 0xc0, 0x08, 0x0e, 0xff, 0xff, 0xc0, 0x00,
  0x00, 0x0c, 0x03, 0xf8, 0x1f, 0x00, 0xc0, 0x08, 0x0c, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x18, 0x01, 0xf8, 0x0e, 0x00, 0x40, 0x08, 0x18, 0x7f, 0xfc, 0xf0, 0x00,
  0x00, 0x38, 0x00, 0xfc, 0x06, 0x00, 0x00, 0x08, 0x18, 0x7f, 0xf0, 0x18, 0x00,
  0x00, 0x70, 0x00, 0xfe, 0x02, 0x00, 0x00, 0x08, 0x18, 0x3f, 0xe0, 0x18, 0x00,
  0x00, 0xe0, 0x00, 0x7f, 0x82, 0x00, 0x00, 0x08, 0x10, 0x1f, 0x80, 0x0c, 0x00,
  0x07, 0xc0, 0x00, 0x7f, 0xc0, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x0c, 0x00,
  0x1e, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x10, 0x30, 0x00, 0x00, 0x04, 0x00,
  0x38, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x10, 0x30, 0x00, 0x00, 0x06, 0x00,
  0x70, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0x00, 0x20, 0x30, 0x00, 0x00, 0x06, 0x00,
  0x60, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x20, 0x30, 0x00, 0x00, 0x06, 0x00,
  0x60, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x40, 0x30, 0x00, 0x00, 0x03, 0x00,
  0x40, 0x00, 0x00, 0x07, 0xfc, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x03, 0x00,
  0x40, 0x00, 0x00, 0x07, 0xfc, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x01, 0x80,
  0x60, 0x00, 0x00, 0x03, 0xfc, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0xc0,
  0x60, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00, 0xe0,
  0x60, 0x00, 0x00, 0x01, 0xf8, 0x00, 0x08, 0x00, 0x60, 0x00, 0x00, 0x00, 0x60,
  0x60, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x20, 0x00, 0x60, 0x00, 0x00, 0x00, 0x30,
  0x60, 0x00, 0x00, 0x00, 0x63, 0x83, 0x80, 0x00, 0x60, 0x00, 0x00, 0x00, 0x30,
  0x60, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x30,
  0x60, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x30,
  0x60, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x60,
  0xc0, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x01, 0xc0,
  0xc0, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x07, 0x80,
  0xc0, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0xff, 0x80, 0x00, 0x00, 0x1e, 0x00,
  0xc0, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x78, 0x00,
  0xc0, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x80, 0x00, 0x01, 0xe0, 0x00,
  0xc0, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x80, 0x00, 0x03, 0x80, 0x00,
  0x70, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x00, 0x00, 0x07, 0x00, 0x00,
  0x3f, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x80, 0x00, 0x0c, 0x00, 0x00,
  0x0f, 0xfe, 0x00, 0x00, 0x07, 0xf8, 0x00, 0xff, 0x80, 0x00, 0x18, 0x00, 0x00,
  0x00, 0x3f, 0xe0, 0x00, 0x06, 0x00, 0x00, 0x01, 0x80, 0x00, 0x30, 0x00, 0x00,
  0x00, 0x00, 0xf8, 0x00, 0x0c, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x60, 0x00, 0x00,
  0x00, 0x00, 0x1e, 0x00, 0x18, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x07, 0xc0, 0xf0, 0x00, 0x00, 0x00, 0x60, 0x03, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x38, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00,
};

const lv_img_dsc_t tux_mono_8 = {
  .header.always_zero = 0,
  .header.w = 100,
  .header.h = 121,
  .data_size = 1574,
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .data = tux_mono_8_map,
};

static void lv_canvas_for_Qr(const char *qr_text){
	enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;
	uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX*SCALE_QR];
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX*SCALE_QR];
  	bool ok = qrcodegen_encodeText(qr_text, tempBuffer, qrcode, errCorLvl,qrcodegen_VERSION_MIN,
	  qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
	if (ok){
		printf ("Generate QR Code\n");
		int size = qrcodegen_getSize(qrcode);
		printf("OriginalSize: %d, QRSizeNeu: %d\n", size, size*SCALE_QR);

		static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(150, 150)];
		lv_obj_t * canvas = lv_canvas_create(lv_scr_act(), NULL);
		lv_canvas_set_buffer(canvas, cbuf, size*SCALE_QR, size*SCALE_QR, LV_IMG_CF_TRUE_COLOR);
		lv_obj_align(canvas, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
		lv_canvas_fill_bg(canvas, LV_COLOR_BLACK);

		int border = 0;
		int count = 0;
//		printf("White Pixel: X    Y\n");
		for (int y = -border; y < size + border; y++){
			for(int x = -border; x < size + border; x++){
				bool check_color = qrcodegen_getModule(qrcode, x , y);
					if (check_color == false){		// if pixel white
						/*
						if(x>9)
							printf("             %d   %d\n", x, y);
						else
							printf("             %d    %d\n", x, y);
						*/
						for (int i =0; i < SCALE_QR; i++){
							for (int j = 0; j < SCALE_QR; j++){
								lv_canvas_set_px(canvas, x*SCALE_QR+i, y*SCALE_QR+j, LV_COLOR_WHITE);
					  		}
				  		}
			  		}

	  		}
			count ++;
  		}
		printf("Number of QR-Pixel: %d\n", count);
	}
	else
		printf("QR-Fehler!");
}

static void lv_text_bottom (const char *text_arr, const char *text_arr2){

	static lv_style_t style;
	lv_style_copy(&style, &lv_style_plain);
	style.text.color = LV_COLOR_BLACK;
	style.text.font = &lv_font_roboto_28;

	static lv_style_t style2;
	lv_style_copy(&style2, &lv_style_plain);
	style2.text.color = LV_COLOR_WHITE;
	style2.text.font = &lv_font_roboto_16;

	static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(250, 50)];
	lv_obj_t *canvas_bottom = lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(canvas_bottom, cbuf, 250, 30, LV_IMG_CF_TRUE_COLOR);
	lv_obj_align(canvas_bottom, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, -30);
	lv_canvas_fill_bg(canvas_bottom, LV_COLOR_WHITE);
	lv_canvas_draw_text(canvas_bottom, -20, 0, 300, &style, text_arr, LV_LABEL_ALIGN_CENTER);

	static lv_color_t cbuf2[LV_CANVAS_BUF_SIZE_TRUE_COLOR(250, 50)];
	lv_obj_t *canvas_bottom2 = lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(canvas_bottom2, cbuf2, 250, 25, LV_IMG_CF_TRUE_COLOR);
	lv_obj_align(canvas_bottom2, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
	lv_canvas_fill_bg(canvas_bottom2, LV_COLOR_BLACK);
	lv_canvas_draw_text(canvas_bottom2, -20, 3, 300, &style2, text_arr2, LV_LABEL_ALIGN_CENTER);
}

static void lv_text_right(const char *text_right, const char *text_right2){
	static lv_style_t style;
	lv_style_copy(&style, &lv_style_plain);
	style.text.color = LV_COLOR_BLACK;
	style.text.font = &lv_font_roboto_28;

	static lv_style_t style2;
	lv_style_copy(&style2, &lv_style_plain);
	style2.text.color = LV_COLOR_BLACK;
	style2.text.font = &lv_font_roboto_16;

	static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(300, 50)];
	lv_obj_t *canvas_right1= lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(canvas_right1, cbuf, 185, 30, LV_IMG_CF_TRUE_COLOR);
	lv_obj_align(canvas_right1, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, -2);
	lv_canvas_fill_bg(canvas_right1, LV_COLOR_WHITE);
	lv_canvas_draw_text(canvas_right1, 0, 0, 300, &style, text_right, LV_LABEL_ALIGN_LEFT);

	static lv_color_t cbuf2[LV_CANVAS_BUF_SIZE_TRUE_COLOR(300, 80)];
	lv_obj_t *canvas_right2= lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(canvas_right2, cbuf2, 185, 20, LV_IMG_CF_TRUE_COLOR);
	lv_obj_align(canvas_right2, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 34);
	lv_canvas_fill_bg(canvas_right2, LV_COLOR_WHITE);
	lv_canvas_draw_text(canvas_right2, 0, 0, 300, &style2, text_right2, LV_LABEL_ALIGN_LEFT);
}

// static void lv_text_vname (const char *text_arr, const char *text_arr2){
static void lv_text_vname (const char *text_arr){

	static lv_style_t style;
	lv_style_copy(&style, &lv_style_plain);
	style.text.color = LV_COLOR_BLACK;
	style.text.font = &lv_font_roboto_28;

	static lv_style_t style2;
	lv_style_copy(&style2, &lv_style_plain);
	style2.text.color = LV_COLOR_WHITE;
	style2.text.font = &lv_font_roboto_16;

	static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(250, 80)];
	lv_obj_t *canvas_bottom = lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(canvas_bottom, cbuf, 250, 30, LV_IMG_CF_TRUE_COLOR);
	lv_obj_align(canvas_bottom, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, -18);
	lv_canvas_fill_bg(canvas_bottom, LV_COLOR_WHITE);
	lv_canvas_draw_text(canvas_bottom, 0, 0, 145, &style, text_arr, LV_LABEL_ALIGN_CENTER);
/*
	static lv_color_t cbuf2[LV_CANVAS_BUF_SIZE_TRUE_COLOR(250, 50)];
	lv_obj_t *canvas_bottom2 = lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(canvas_bottom2, cbuf2, 250, 25, LV_IMG_CF_TRUE_COLOR);
	lv_obj_align(canvas_bottom2, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
	lv_canvas_fill_bg(canvas_bottom2, LV_COLOR_BLACK);
	lv_canvas_draw_text(canvas_bottom2, -20, 3, 300, &style2, text_arr2, LV_LABEL_ALIGN_CENTER);
*/
}

static void lv_text_middle(const char *text_sup_1, const char *text_sup_2, const char *text_sup_3){

	static lv_style_t style;
	lv_style_copy(&style, &lv_style_plain);
	style.text.color = LV_COLOR_BLACK;
	style.text.font = &lv_font_roboto_28;

	static lv_style_t style2;
	lv_style_copy(&style2, &lv_style_plain);
	style2.text.color = LV_COLOR_BLACK;
	style2.text.font = &lv_font_roboto_16;

	static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(300, 50)];
	lv_obj_t *canvas_mid1= lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(canvas_mid1, cbuf, 169, 30, LV_IMG_CF_TRUE_COLOR);
	lv_obj_align(canvas_mid1, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, -4);
	lv_canvas_fill_bg(canvas_mid1, LV_COLOR_WHITE);
	lv_canvas_draw_text(canvas_mid1, 0, 0, 300, &style, text_sup_1, LV_LABEL_ALIGN_LEFT);

	static lv_color_t cbuf2[LV_CANVAS_BUF_SIZE_TRUE_COLOR(300, 80)];
	lv_obj_t *canvas_mid2= lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(canvas_mid2, cbuf2, 169, 20, LV_IMG_CF_TRUE_COLOR);
	lv_obj_align(canvas_mid2, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 25);
	lv_canvas_fill_bg(canvas_mid2, LV_COLOR_WHITE);
	lv_canvas_draw_text(canvas_mid2, 0, 0, 300, &style2, text_sup_2, LV_LABEL_ALIGN_LEFT);

	static lv_color_t cbuf3[LV_CANVAS_BUF_SIZE_TRUE_COLOR(300, 80)];
	lv_obj_t *canvas_mid3= lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(canvas_mid3, cbuf3, 169, 50, LV_IMG_CF_TRUE_COLOR);
	lv_obj_align(canvas_mid3, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 42);
	lv_canvas_fill_bg(canvas_mid3, LV_COLOR_WHITE);
	lv_canvas_draw_text(canvas_mid3, 0, 0, 150, &style2, text_sup_3, LV_LABEL_ALIGN_LEFT);
}


void lv_ex_img_1(void)
{
	LV_IMG_DECLARE(tux_mono_8);

	lv_obj_t * img1 = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(img1, &tux_mono_8);
	lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 76, -1);
}


void vcd_epaper(const char *text_bottom, const char *vcdProf, bool scr_new)
{
	if (scr_new) {
		lv_obj_t *default_screen = lv_scr_act();
		lv_obj_clean(default_screen);

		lv_canvas_for_Qr(qr_text);

		lv_text_bottom(text_bottom, text_bottom2);
		lv_text_right(text_right, vcdProf);
	}
	lv_scr_load(lv_scr_act());

	lv_task_handler();
	printk("lv_task_handler()\n");
}

void vcd_epaper_tux(const char *v_name, bool scr_new)
{
	if (scr_new) {

		lv_obj_t *default_screen = lv_scr_act();
		lv_obj_clean(default_screen);

		lv_canvas_for_Qr(qr_text);

		lv_text_middle(text_sup_top, text_sup_middle, text_sup_middle2);
		lv_text_vname (v_name);
		lv_ex_img_1();
	}

	lv_scr_load(lv_scr_act());

	lv_task_handler();
	printk("lv_task_handler()\n");
}

