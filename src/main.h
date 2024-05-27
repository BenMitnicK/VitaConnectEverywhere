/*
  VitaConnectEverywhere
  Copyright (C) 2024, BenMitnicK

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include <vitasdk.h>

#include <sys/syslimits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vitashell_user.h>
#include <taihen.h>

#include "vitashell_error.h"

#define print printf
#define printf sceClibPrintf

#define VITASHELL_TITLEID "VITATWEAK"
#define MAX_PATH_LENGTH 1024

//define a refrence id for each our widgets and textures, make sure they're all unique, like the ones below. Doesn't need to be anything specific, just something unique
#define SEPARATOR				"separator"
#define TEXT					"text"

#define PLANE_CHECKBOX_NOTIF	"plane_checkbox_notif"
#define CHECKBOX_NOTIF_TEXT		"checkbox_notif_ text"
#define CHECKBOX_NOTIF			"checkbox_notif"

#define PLANE_CHECKBOX_SELECT	"plane_checkbox_select"
#define CHECKBOX_SELECT_TEXT	"checkbox_select_text"
#define CHECKBOX_SELECT			"checkbox_select"

#define PLANE_BUTTON_NETWORK	"plane_button_network"
#define BUTTON_NETWORK_TEXT		"button_network_text"
#define BUTTON_NETWORK			"button_network"

#define PLANE_BUTTON_SELECT		"plane_button_select"
#define BUTTON_SELECT_TEXT		"button_select_text"
#define BUTTON_SELECT			"button_select"

#define PLANE_CHECKBOX_REFRESH	"plane_checkbox_refresh"
#define CHECKBOX_REFRESH_TEXT	"checkbox_refresh_text"
#define CHECKBOX_REFRESH		"checkbox_refresh"

#define PLANE_BUTTON_ACTIVATE	"plane_button_activate"
#define BUTTON_ACTIVATE			"button_activate"
#define BUTTON_USB				"button_usb"
#define BUTTON_REFRESH				"button_refresh"

#define PLANE_TITLE				"plane_title"
#define USB_PLANE				"USB_tex"
#define USB_TEXTURE				"USB_texture"
#define USB_TEXTURE_PATH		"ur0:QuickMenuReborn/VitaConnectEverywhere/USB-logo.png"
#define WIFI_PLANE				"wifi_tex"
#define WIFI_TEXTURE			"wifi_texture"
#define WIFI_TEXTURE_PATH		"ur0:QuickMenuReborn/VitaConnectEverywhere/wifi-logo.png"

extern char *mountType[4];
extern bool select_WifiUsb;
extern int run, toggle, count, s_mesg, run_USB, onPressedFTP;

extern char mount_Info[];

extern bool NotifsON, select_WifiUsb, select_refresh;

extern SceUID user_modid, kernel_modid;

#endif
