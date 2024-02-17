/*
 * Copyright (c) 2019 LUG Nuernberg JK
 *
 * SPDX-License-Identifier: Apache-2.0
*/

/*  Für Umlaute folgende Escape-Sequenzen benutzen:
 *  Ä = \xc4   ,  Ö = \xd6    ,  Ü = \xdc
 *  ä = \xe4   ,  ö = \xf6    ,  ü = \xfc    ,   ß = \xdf
 *
*/

#define LUG_Noris
//#define LUG_Mitterteich


#ifdef LUG_Noris
// Globale Variablen, Feste Anteile der VCD
char *name_vcd = "Neuer Name";
// Erste Anzeigeebene
static const char *qr_text = "https://www.lug-noris.de/index.php";
static const char *text_right = "LUG-Noris.de";
static const char *text_bottom2 = "Linux User Group N\xfcrnberg";
// Zweite Anzeigeebene
static const char *text_sup_top = "LINUX";
static const char *text_sup_middle = "User Group";
static const char *text_sup_middle2 = "N\xfcrnberg";
#endif

#ifdef LUG_Mitterteich
// Globale Variablen, Feste Anteile der VCD
char *name_vcd = "Neuer Name";
// Erste Anzeigeebene
static const char *qr_text = "https://www.linux-mitterteich.de";
static const char *text_right = "LUG-Mitterteich";
static const char *text_bottom2 = "Linux User Group Mitterteich";
// Zweite Anzeigeebene
static const char *text_sup_top = "LINUX";
static const char *text_sup_middle = "User Group";
static const char *text_sup_middle2 = "Mitterteich";
#endif


