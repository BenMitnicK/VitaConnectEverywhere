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

#include "utils.h"
#include "main.h"
#include "thread.h"
#include "ftpvita.h"
#include "file.h"

#include <vitashell_user.h>

#include <taihen.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>

#include <vitasdk.h>

#include <quickmenureborn/qm_reborn.h>

SceUID	usbdevice_modid = -1;
int usbdeviceOFF = 0, notifType = 0;

int debugPrintf(const char *text, ...) {
  va_list list;
  char string[512];

  va_start(list, text);
  vsnprintf(string, sizeof(string), text, list);
  va_end(list);

  SceUID fd = sceIoOpen("ux0:data/VCE_log.txt", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
  if (fd >= 0) {
    sceIoWrite(fd, string, strlen(string));
    sceIoClose(fd);
  }

  return 0;
}

void getSizeString(char string[16], uint64_t size) {
  double double_size = (double)size;

  int i = 0;
  static char *units[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
  while (double_size >= 1024.0) {
    double_size /= 1024.0;
    i++;
  }

  snprintf(string, 16, "%.*f %s", (i == 0) ? 0 : 2, double_size, units[i]);
}

int checkMemoryCardFreeSpace(const char *path, uint64_t size) {
  char device[8];
  uint64_t free_size = 0, max_size = 0, extra_space = 0;
  
  char *p = strchr(path, ':');
  if (p) {
    strncpy(device, path, p - path + 1);
    device[p - path + 1] = '\0';
  }

  if (strcmp(device, "ux0:") == 0) {
    extra_space = 40 * 1024 * 1024;
  }

  if (size >= (free_size + extra_space)) {

    char size_string[16];
    getSizeString(size_string, size - (free_size + extra_space));

    return 1;
  }

  return 0;
}

void utf8_to_utf16(const uint8_t *src, uint16_t *dst) {
  int i;
  for (i = 0; src[i];) {
    if ((src[i] & 0xE0) == 0xE0) {
      *(dst++) = ((src[i] & 0x0F) << 12) | ((src[i + 1] & 0x3F) << 6) | (src[i + 2] & 0x3F);
      i += 3;
    } else if ((src[i] & 0xC0) == 0xC0) {
      *(dst++) = ((src[i] & 0x1F) << 6) | (src[i + 1] & 0x3F);
      i += 2;
    } else {
      *(dst++) = src[i];
      i += 1;
    }
  }

  *dst = '\0';
}

void checkWifiPlane(){
	
	/* network check */

	int wifi, plane;
	
	sceRegMgrGetKeyInt("/CONFIG/NET", "wifi_flag", &wifi);
	sceRegMgrGetKeyInt("/CONFIG/SYSTEM", "flight_mode", &plane);
	
	if (!wifi || plane) {
		s_mesg = 1;

		sceIncomingDialogInitialize(0);
		SceIncomingDialogParam params;
		sceIncomingDialogParamInit(&params);

		utf8_to_utf16((uint8_t *)"ok", params.buttonRightText);
		utf8_to_utf16((uint8_t *)"Wi-Fi is Disabled Or System is in Airplane Mode.", params.dialogText);
		sceIncomingDialogOpen(&params);
		
		run = 0;
		ftpvita_fini();	
		start_thread();

	}
	
}

void sendNotifProgressBar(uint64_t max, int notifType)
{
	SceNotificationUtilProgressInitParam initparam;
	SceNotificationUtilProgressUpdateParam updateparam;
	SceNotificationUtilProgressFinishParam finishparam;

	char buf[SCE_NOTIFICATIONUTIL_TEXT_MAX * 2] = "Refresh Beggin Please wait...";	
	char buf2[SCE_NOTIFICATIONUTIL_TEXT_MAX * 2] = "Refresh In progress...";
	char buf3[SCE_NOTIFICATIONUTIL_TEXT_MAX * 2] = "Refresh Finished :)";
	
	sceClibMemset(&initparam, 0, sizeof(SceNotificationUtilProgressInitParam));
	sceClibMemset(&updateparam, 0, sizeof(SceNotificationUtilProgressUpdateParam));
	sceClibMemset(&finishparam, 0, sizeof(SceNotificationUtilProgressFinishParam));
	
	utf8_to_utf16((uint8_t *)buf, initparam.notificationText);
	utf8_to_utf16((uint8_t *)buf2, updateparam.notificationText);
    utf8_to_utf16((uint8_t *)buf3, finishparam.notificationText);
	
	if(notifType == 1){
		initparam.unk_4EC = max;
		sceNotificationUtilProgressBegin(&initparam);
	}else if(notifType == 2){	
		updateparam.targetProgress = max;
		sceNotificationUtilProgressUpdate(&updateparam);
	}else if(notifType == 3){	
		sceNotificationUtilProgressFinish(&finishparam);
	}
}

void sendNotification(const char *text, ...)
{
	SceNotificationUtilProgressInitParam param;

	char buf[SCE_NOTIFICATIONUTIL_TEXT_MAX * 2];
	va_list argptr;
	va_start(argptr, text);
	sceClibVsnprintf(buf, sizeof(buf), text, argptr);
	va_end(argptr);

	sceClibMemset(&param, 0, sizeof(SceNotificationUtilProgressInitParam));

	utf8_to_utf16((uint8_t *)buf, param.notificationText);

	sceNotificationUtilSendNotification(&param);
}

int vshIoMount(int id, const char *path, int permission, int a4, int a5, int a6) {
  uint32_t buf[3];

  buf[0] = a4;
  buf[1] = a5;
  buf[2] = a6;

  return _vshIoMount(id, path, permission, buf);
}

void remount(int id) {
  vshIoUmount(id, 0, 0, 0);
  vshIoUmount(id, 1, 0, 0);
  vshIoMount(id, NULL, 0, 0, 0, 0);
}

int hasEndSlash(const char *path) {
  return path[strlen(path) - 1] == '/';
}

int mountGamecardUx0() {
  // Destroy other apps
  sceAppMgrDestroyOtherApp();

  // Redirect SD2VITA to ux0:
  shellUserRedirectUx0("sdstor0:gcd-lp-ign-entire", "sdstor0:gcd-lp-ign-entire");

  // Umount uma0:
  vshIoUmount(0xF00, 0, 0, 0);

  // Remount Memory Card
  remount(0x800);

  return 0;
}

int mountGamecardUma0() {
  // Destroy other apps
  sceAppMgrDestroyOtherApp();

  // Redirect ux0: to uma0:
  shellUserRedirectUma0("sdstor0:gcd-lp-ign-entire", "sdstor0:gcd-lp-ign-entire");

  // Remount uma0:
  remount(0xF00);

  return 0;
}

int mountUsbUma0() {
  // Destroy other apps
  sceAppMgrDestroyOtherApp();

  // Redirect ux0: to uma0:
  shellUserRedirectUma0("sdstor0:uma-pp-act-a", "sdstor0:uma-lp-act-entire");

  // Remount uma0:
  remount(0xF00);

  return 0;
}

int mountImcUx0() {
  // Destroy other apps
  sceAppMgrDestroyOtherApp();

  // Redirect imc0: to ux0:
  shellUserRedirectUx0("sdstor0:int-lp-ign-userext", "sdstor0:int-lp-ign-userext");

  // Umount ux0:
  vshIoUmount(0x800, 0, 0, 0);
  
  // Remount Memory Card
  remount(0x800);

  return 0;
}

int mountXmcUx0() {
  // Destroy other apps
  sceAppMgrDestroyOtherApp();

  // Redirect xmc0: to ux0:
  shellUserRedirectUx0("sdstor0:xmc-lp-ign-userext", "sdstor0:xmc-lp-ign-userext");

  // Umount ux0:
  vshIoUmount(0x800, 0, 0, 0);
  
  // Remount Memory Card
  remount(0x800);

  return 0;
}

int mountUsbUx0() {
  // Destroy other apps
  sceAppMgrDestroyOtherApp();

  // Redirect uma0: to ux0:
  shellUserRedirectUx0("sdstor0:uma-pp-act-a", "sdstor0:uma-lp-act-entire");

  // Umount uma0:
  vshIoUmount(0xF00, 0, 0, 0);

  // Remount Memory Card
  remount(0x800);

  return 0;
}

void mountOthersPartition(int mount){
	if (mount == 1){
		if (checkFileExist("sdstor0:gcd-lp-ign-entire")) {
			if(mountGamecardUma0() >= 0){
				run_USB = 5;
				//start_thread();				
			}else{
				run_USB = 6;
				//start_thread();
			}
		}
		if (checkFileExist("sdstor0:xmc-lp-ign-userext") || !checkFolderExist("xmc0:")){
			int res = vshIoMount(0xE00, NULL, 2, 0, 0, 0);
			if (res > 0){
				run_USB = 7;
				//start_thread();
			}else{
				run_USB = 8;
				//start_thread();
			}				
		}
		if (checkFileExist("sdstor0:int-lp-ign-userext") || !checkFolderExist("imc0:")){
			int res = vshIoMount(0xD00, NULL, 2, 0, 0, 0);
			if (res > 0){
				run_USB = 9;
				//start_thread();
			}else{
				run_USB = 10;
				//start_thread();
			}
		}
	}else if (mount == 2){
		if (mountUsbUma0() >= 0){
			run_USB = 5;
			//start_thread();	
		}else{
			run_USB = 6;
			//start_thread();	
		}
		if (checkFileExist("sdstor0:xmc-lp-ign-userext") && !checkFolderExist("xmc0:")){
			int res = vshIoMount(0xE00, NULL, 2, 0, 0, 0);
			if (res > 0){
				run_USB = 7;
				//start_thread();
			}else{
				run_USB = 8;
				//start_thread();
			}
		}
		if (checkFileExist("sdstor0:int-lp-ign-userext") && !checkFolderExist("imc0:")){
			int res = vshIoMount(0xD00, NULL, 2, 0, 0, 0);
			if (res > 0){
				run_USB = 9;
				//start_thread();
			}else{
				run_USB = 10;
				//start_thread();
			}
		}
	}
}

SceUID startUsb(const char *usbDevicePath, const char *imgFilePath, int type) {
  SceUID modid = -1;
  int res;

  // Destroy other apps
  sceAppMgrDestroyOtherApp();

  // Load and start usbdevice module
  res = taiLoadStartKernelModule(usbDevicePath, 0, NULL, 0);
  if (res < 0)
    goto ERROR_LOAD_MODULE;

  modid = res;

  // Stop MTP driver
  res = sceMtpIfStopDriver(1);
  if (res < 0 && res != 0x8054360C)
    goto ERROR_STOP_DRIVER;

  // Set device information
  res = sceUsbstorVStorSetDeviceInfo("\"PS Vita\" MC", "1.00");
  if (res < 0)
    goto ERROR_USBSTOR_VSTOR;

  // Set image file path
  res = sceUsbstorVStorSetImgFilePath(imgFilePath);
  if (res < 0)
    goto ERROR_USBSTOR_VSTOR;

  // Start USB storage
  res = sceUsbstorVStorStart(type);
  if (res < 0){
    goto ERROR_USBSTOR_VSTOR;
  }else{
	print("VitaConnectEverywhereUSB connected\n");
  }

  return modid;

ERROR_USBSTOR_VSTOR:
  sceMtpIfStartDriver(1);

ERROR_STOP_DRIVER:
  taiStopUnloadKernelModule(modid, 0, NULL, 0, NULL, NULL);

ERROR_LOAD_MODULE:
  return res;
}

int stopUsb(SceUID modid) {
  int res;

  // Stop USB storage
  res = sceUsbstorVStorStop();
  if (res < 0)
    return res;

  // Start MTP driver
  res = sceMtpIfStartDriver(1);
  if (res < 0)
    return res;

  // Stop and unload usbdevice module
  res = taiStopUnloadKernelModule(modid, 0, NULL, 0, NULL, NULL);
  if (res < 0)
    return res;
	print("VitaConnectEverywhereUSB connected\n");

  // Remount Memory Card
  remount(0x800);

  // Remount imc0:
  if (checkFolderExist("imc0:"))
    remount(0xD00);

  // Remount xmc0:
  if (checkFolderExist("xmc0:"))
    remount(0xE00);

  // Remount uma0:
  if (checkFolderExist("uma0:"))
    remount(0xF00);

  return 0;
}

void initFtp() {
  // Add all the current mountpoints to ftpvita
  int i;
  for (i = 0; i < getNumberOfDevices(); i++) {
    char **devices = getDevices();
    if (devices[i]) {
      if (strcmp(devices[i], "ux0:") != 0)
        continue;

      ftpvita_add_device(devices[i]);
    }
  }
}

void initUsb() {
  char *path = NULL;
  char driverType[20];
  usbdeviceOFF = 0;
  sprintf(driverType, "%s", mountType[count]);
  
	sceIncomingDialogInitialize(0);
	SceIncomingDialogParam params;
	sceIncomingDialogParamInit(&params);

  if (strcmp(driverType,"MEMORY CARD") == 0) {
    if (checkFileExist("sdstor0:xmc-lp-ign-userext")){
      path = "sdstor0:xmc-lp-ign-userext";
    }else if (checkFileExist("sdstor0:int-lp-ign-userext")){
      path = "sdstor0:int-lp-ign-userext";
    }else{
      utf8_to_utf16((uint8_t *)"ok", params.buttonRightText);
	  utf8_to_utf16((uint8_t *)"Please insert a Memory Card.", params.dialogText);
	  sceIncomingDialogOpen(&params);
	  usbdeviceOFF = 1;
	}
  } else if (strcmp(driverType,"GAME CARD") == 0) {
    if (checkFileExist("sdstor0:gcd-lp-ign-gamero")){
      path = "sdstor0:gcd-lp-ign-gamero";
    }else{
      utf8_to_utf16((uint8_t *)"ok", params.buttonRightText);
	  utf8_to_utf16((uint8_t *)"Please insert a Game Card.", params.dialogText);
	  sceIncomingDialogOpen(&params);
	  usbdeviceOFF = 1;
	}
  } else if (strcmp(driverType,"SD2VITA") == 0) {
    if (checkFileExist("sdstor0:gcd-lp-ign-entire")){
      path = "sdstor0:gcd-lp-ign-entire";
    }else{
      utf8_to_utf16((uint8_t *)"ok", params.buttonRightText);
	  utf8_to_utf16((uint8_t *)"Please insert a microSD.", params.dialogText);
	  sceIncomingDialogOpen(&params);
	  usbdeviceOFF = 1;
	}
  } else if (strcmp(driverType,"PSVSD") == 0) {
    if (checkFileExist("sdstor0:uma-pp-act-a")){
      path = "sdstor0:uma-pp-act-a";
    }else if (checkFileExist("sdstor0:uma-lp-act-entire")){
      path = "sdstor0:uma-lp-act-entire";
    }else{
      utf8_to_utf16((uint8_t *)"ok", params.buttonRightText);
	  utf8_to_utf16((uint8_t *)"Please insert a microSD.", params.dialogText);
	  sceIncomingDialogOpen(&params);
	  usbdeviceOFF = 1;
	}
  }else if (strcmp(driverType,"UR0") == 0) {
    if (checkFileExist("sdstor0:int-lp-ign-user")){
      path = "sdstor0:int-lp-ign-user";
    }else{
      utf8_to_utf16((uint8_t *)"ok", params.buttonRightText);
	  utf8_to_utf16((uint8_t *)"Ho Ho!! UR0 Not Mounted.", params.dialogText);
	  sceIncomingDialogOpen(&params);
	  usbdeviceOFF = 1;
	}
  }

  if (!path)
    return;

  usbdevice_modid = startUsb("ur0:QuickMenuReborn/VitaConnectEverywhere/usbdevice.skprx", path, SCE_USBSTOR_VSTOR_TYPE_FAT);
}
