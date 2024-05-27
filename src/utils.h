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

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdbool.h>

#include <vitasdk.h>

#define	USER_MESSAGE_THREAD_NAME	"user_message"
#define	USER_MESSAGE_THREAD_TIME	(500000)	// 1/2 sec

extern SceUID	usbdevice_modid;
extern int usbdeviceOFF, notifType;

int debugPrintf(const char *text, ...);

int vshIoUmount(int id, int a2, int a3, int a4);
int _vshIoMount(int id, const char *path, int permission, void *buf);
int vshIoMount(int id, const char *path, int permission, int a4, int a5, int a6);

int hasEndSlash(const char *path);

void utf8_to_utf16(const uint8_t *src, uint16_t *dst);

void sendNotification(const char *text, ...);
void sendNotifProgressBar(uint64_t max, int notifType);

void checkWifiPlane();

int checkMemoryCardFreeSpace(const char *path, uint64_t size);

int mountUsbUx0();
int mountUsbUma0();
int mountGamecardUx0();
int mountGamecardUma0();
int mountImcUx0();
int mountXmcUx0();

void mountOthersPartition(int mount);

void initFtp();
void initUsb();
int stopUsb(SceUID modid);
void remount(int id);

#endif
