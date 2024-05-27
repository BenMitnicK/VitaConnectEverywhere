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

#include "main.h"
#include "thread.h"
#include "utils.h"
#include "ftpvita.h"
#include "refresh.h"
#include "file.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TAIPOOL_AS_STDLIB

#include <taipool.h>
#include <vitasdk.h>
#include <vita2d.h>

#include <quickmenureborn/qm_reborn.h>

int _newlib_heap_size_user = 128 * 1024 * 1024;

int run, s_mesg, toggle, count = 0, status, run_USB = 0, onPressedFTP = 0;
char *mountType[4] = {"MEMORY CARD", "GAME CARD", "SD2VITA", "PSVSD"};

char mount_Info[30] = "";

//Declare our boolean
bool NotifsON = false, select_WifiUsb = false, select_Wifi = false, select_refresh = false;

SceUID user_modid = -1, kernel_modid = -1, patch_modid = -1;

//Declare our function that will act as the callback for when our button is pressed, Format: BUTTON_HANDLER(name of function)
BUTTON_HANDLER(onPressFTP)
{
	if(!refreshOnOff){
		onPressedFTP = 1;
		if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITA){
			if(select_WifiUsb){
				toggle = 0;
				if(run){
					run = 0;
					ftpvita_fini();
					start_thread();
				}else{
					checkWifiPlane();
					do_net_connected();
				}
			}else{
				toggle = 0;
				
				if (usbdevice_modid >= 0){
					stopUsb(usbdevice_modid);
					usbdeviceOFF = 1;
				}

				SceUdcdDeviceState state;
				sceUdcdGetDeviceState(&state);
				
				if (state.cable & SCE_UDCD_STATUS_CABLE_CONNECTED){
					initUsb();
					start_thread();
				}else if(state.cable & SCE_UDCD_STATUS_CABLE_DISCONNECTED){
					start_thread();
				}
			}
		}else if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITATV){
			if(run){
				run = 0;
				ftpvita_fini();
				start_thread();
			}else{
				checkWifiPlane();
				do_net_connected();
			}
		}
	}else{
		run_USB = 12;
		start_thread();
	}
}

BUTTON_HANDLER(onPressUSB)
{		
	if (!refreshOnOff) {
		if ((kernel_modid >= 0 || kernel_modid == 0x8002D013) && user_modid >= 0 &&
			shellUserIsUx0Redirected("sdstor0:uma-pp-act-a", "sdstor0:uma-lp-act-entire") == 1) {
			if (checkFolderExist("uma0:") || checkFileExist("sdstor0:gcd-lp-ign-entire")) {
				if (mountGamecardUx0() >= 0) {
					mountOthersPartition(2);
					run_USB = 3;
					start_thread();
					QuickMenuRebornSetWidgetSize(BUTTON_USB, 250, 75, 0, 0);
					QuickMenuRebornSetWidgetColor(BUTTON_USB, 1,1,1,1);
					QuickMenuRebornSetWidgetLabel(BUTTON_USB, "Mount USB");
				}else{
					run_USB = 4;
					start_thread();
				}
			}else{
				if (checkFileExist("sdstor0:xmc-lp-ign-userext")){
					if (!checkFolderExist("xmc0:")){
						if(mountXmcUx0() >= 0){
							mountOthersPartition(2);
							run_USB = 3;
							start_thread();
							QuickMenuRebornSetWidgetSize(BUTTON_USB, 250, 75, 0, 0);
							QuickMenuRebornSetWidgetColor(BUTTON_USB, 1,1,1,1);
							QuickMenuRebornSetWidgetLabel(BUTTON_USB, "Mount USB");
						}else{
							run_USB = 4;
							start_thread();
						}
					}else{
						vshIoUmount(0xE00, 0, 0, 0);
						vshIoUmount(0xE00, 1, 0, 0);
						if(mountXmcUx0() >= 0){
							mountOthersPartition(2);
							run_USB = 3;
							start_thread();
							QuickMenuRebornSetWidgetSize(BUTTON_USB, 250, 75, 0, 0);
							QuickMenuRebornSetWidgetColor(BUTTON_USB, 1,1,1,1);
							QuickMenuRebornSetWidgetLabel(BUTTON_USB, "Mount USB");
						}else{
							run_USB = 4;
							start_thread();
						}
					}
				}else{
					if (!checkFolderExist("imc0:")){
						if(mountImcUx0() >= 0){
							mountOthersPartition(2);
							run_USB = 3;
							start_thread();
							QuickMenuRebornSetWidgetSize(BUTTON_USB, 250, 75, 0, 0);
							QuickMenuRebornSetWidgetColor(BUTTON_USB, 1,1,1,1);
							QuickMenuRebornSetWidgetLabel(BUTTON_USB, "Mount USB");
						}else{
							run_USB = 4;
							start_thread();
						}
					}else{
						vshIoUmount(0xD00, 0, 0, 0);
						vshIoUmount(0xD00, 1, 0, 0);
						if(mountImcUx0() >= 0){
							mountOthersPartition(2);
							run_USB = 3;
							start_thread();
							QuickMenuRebornSetWidgetSize(BUTTON_USB, 250, 75, 0, 0);
							QuickMenuRebornSetWidgetColor(BUTTON_USB, 1,1,1,1);
							QuickMenuRebornSetWidgetLabel(BUTTON_USB, "Mount USB");
						}else{
							run_USB = 4;
							start_thread();
						}
					}
				}
			}
		}else{
			if (checkFolderExist("uma0:") && checkFileExist("sdstor0:uma-lp-act-entire")) {
				if (mountUsbUx0() >= 0) {
					mountOthersPartition(1);
					run_USB = 1;
					start_thread();
					QuickMenuRebornSetWidgetSize(BUTTON_USB, 250, 75, 0, 0);
					QuickMenuRebornSetWidgetColor(BUTTON_USB, 0,1,1,1);
					QuickMenuRebornSetWidgetLabel(BUTTON_USB, "Umount USB");
				}else{
					run_USB = 2;
					start_thread();
				}
			}else {
				if (checkFileExist("sdstor0:uma-lp-act-entire")) {
				  int res = vshIoMount(0xF00, NULL, 0, 0, 0, 0);
				  if (res < 0){
					run_USB = 6;
					start_thread();
				  }else{
					if (mountUsbUx0() >= 0) {
						mountOthersPartition(1);
						run_USB = 1;
						start_thread();
						QuickMenuRebornSetWidgetSize(BUTTON_USB, 250, 75, 0, 0);
						QuickMenuRebornSetWidgetColor(BUTTON_USB, 0,1,1,1);
						QuickMenuRebornSetWidgetLabel(BUTTON_USB, "Umount USB");
					}else{
						run_USB = 2;
						start_thread();
					}
				  }
				} else {
					run_USB = 11;
					start_thread();
				}
			}
		}
	}else{
		run_USB = 12;
		start_thread();
	}
}

BUTTON_HANDLER(onPressREFRESH)
{	
	if(select_refresh){
		if(!refreshOnOff){
			SceUID thid = sceKernelCreateThread("refresh_thread", (SceKernelThreadEntry)refresh_thread, 0x40, 0x100000, 0, 0, NULL);
			if (thid >= 0)
				sceKernelStartThread(thid, 0, NULL);
			
			QuickMenuRebornSetWidgetSize(BUTTON_REFRESH, 270, 75, 0, 0);
			QuickMenuRebornSetWidgetColor(BUTTON_REFRESH, 1,0,1,1);
			QuickMenuRebornSetWidgetLabel(BUTTON_REFRESH, "In Progress");
		}else{
			run_USB = 12;
			start_thread();
		}
	}else{
		if(!refreshOnOff){
			/*SceUID thid = sceKernelCreateThread("license_thread", (SceKernelThreadEntry)license_thread, 0x40, 0x100000, 0, 0, NULL);
			if (thid >= 0)
			  sceKernelStartThread(thid, 0, NULL);*/
			run_USB = 14;
			start_thread();
		}else{
			run_USB = 12;
			start_thread();
		}
	}
}

BUTTON_HANDLER(onPressNetwork)
{
	if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITA){
		int val = 0;
		sceRegMgrGetKeyInt("/CONFIG/TEL", "use_debug_settings", &val);
		
		sceIncomingDialogInitialize(0);
		SceIncomingDialogParam params;
		sceIncomingDialogParamInit(&params);
		
			if(!val)
			{	
				sceRegMgrSetKeyInt("/CONFIG/TEL", "use_debug_settings", 1);
				QuickMenuRebornSetWidgetLabel(BUTTON_NETWORK_TEXT, "Warning 3G Disable");
			}else if(val)
			{	
				sceRegMgrSetKeyInt("/CONFIG/TEL", "use_debug_settings", 0);
				QuickMenuRebornSetWidgetLabel(BUTTON_NETWORK_TEXT, "3G Enable");
			}
			
		utf8_to_utf16((uint8_t *)"ok", params.buttonRightText);
	    utf8_to_utf16((uint8_t *)"You Must Reboot Your PSVita For Take Effect.", params.dialogText);
	    sceIncomingDialogOpen(&params);
	}
}

ONLOAD_HANDLER(OnButtonNetwork)
{
	if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITA){
		int val = 0;
		sceRegMgrGetKeyInt("/CONFIG/TEL", "use_debug_settings", &val);
		
			if(!val)
			{	
				QuickMenuRebornSetWidgetLabel(BUTTON_NETWORK_TEXT, "3G Enable");
			}else if(val)
			{	
				QuickMenuRebornSetWidgetLabel(BUTTON_NETWORK_TEXT, "Warning 3G Disable");
			}
	}
}

ONLOAD_HANDLER(OnButtonSelect)
{
	if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITATV){
	
		int val = 0;
		sceRegMgrGetKeyInt("/CONFIG/NET", "wifi_flag", &val);
		
			if(!val)
			{	
				QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, "WiFi is OFF");				
			}else if(val)
			{	
				QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, "WiFi is ON");
			}
	}
}

BUTTON_HANDLER(onPressSelect)
{
	if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITA){
		if(!select_WifiUsb){
			if(count == 3){
				count = 0;
				QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, mountType[count]);
			}else{
				count++;
				QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, mountType[count]);
			}
		}
	}else if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITATV){
	
		int val = 0;
		sceRegMgrGetKeyInt("/CONFIG/NET", "wifi_flag", &val);
		
			if(!val)
			{	
				sceRegMgrSetKeyInt("/CONFIG/NET", "wifi_flag", 1);
				sceWlanSetConfiguration(1);
				QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, "WiFi is ON");
			}else if(val)
			{	
				sceRegMgrSetKeyInt("/CONFIG/NET", "wifi_flag", 0);
				sceWlanSetConfiguration(0);
				QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, "WiFi is OFF");
			}
	}
}

ONLOAD_HANDLER(OnButtonREFRESHLoad)
{
	select_refresh = QuickMenuRebornGetCheckboxValue(CHECKBOX_REFRESH);
	
	if (refreshOnOff) {
		QuickMenuRebornSetWidgetSize(BUTTON_REFRESH, 270, 75, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_REFRESH, 1,0,1,1);
		QuickMenuRebornSetWidgetLabel(BUTTON_REFRESH, "In Progress");
	} else {
		QuickMenuRebornSetWidgetSize(BUTTON_REFRESH, 270, 75, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_REFRESH, 1,1,1,1);
		QuickMenuRebornSetWidgetLabel(BUTTON_REFRESH, "Refresh");
	}
}

ONLOAD_HANDLER(OnButtonFTPLoad)
{
	NotifsON = QuickMenuRebornGetCheckboxValue(CHECKBOX_NOTIF);
	
	if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITA){
		select_WifiUsb = QuickMenuRebornGetCheckboxValue(CHECKBOX_SELECT);
		if(select_WifiUsb){
			if (usbdevice_modid >= 0)
				stopUsb(usbdevice_modid);
			
			QuickMenuRebornSetWidgetLabel(CHECKBOX_SELECT_TEXT, "FTP is Selected");	
			QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, "none");
			
			if(run){
				// Update our widget
				QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
				QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 0,1,0,1);
				QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "FTP Enable");
			}else{
				// Update our widget
				QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
				QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 1,1,1,1);
				QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "FTP Disable");
			}				
		}else{
			if(run){
				run = 0;
				ftpvita_fini();
				toggle = 0;
				start_thread();
			}
			QuickMenuRebornSetWidgetLabel(CHECKBOX_SELECT_TEXT, "USB is Selected");
			QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, mountType[count]);
			
			if (usbdeviceOFF || usbdevice_modid < 0){
				QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
				QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 1,1,1,1);
				QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "USB Disable");
			}else{
				QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
				QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 0,1,1,1);
				QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "USB Enable");
			}
		}
	}else if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITATV){		
		int val = 0;

		sceRegMgrGetKeyInt("/CONFIG/NET", "wifi_flag", &val);
		if (val){		
			QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, "WiFi is ON");
		}else{
			QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, "WiFi is OFF");
		}

		if(run){
			// Update our widget
			QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
			QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 0,1,0,1);
			QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "FTP Enable");
		}else{
			// Update our widget
			QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
			QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 1,1,1,1);
			QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "FTP Disable");
		}
	}
}

ONLOAD_HANDLER(OnButtonUSBLoad)
{
	if ((kernel_modid >= 0 || kernel_modid == 0x8002D013) && user_modid >= 0 &&
		shellUserIsUx0Redirected("sdstor0:uma-pp-act-a", "sdstor0:uma-lp-act-entire") == 1) {
		QuickMenuRebornSetWidgetSize(BUTTON_USB, 250, 75, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_USB, 0,1,1,1);
		QuickMenuRebornSetWidgetLabel(BUTTON_USB, "Umount USB");
	} else {
		QuickMenuRebornSetWidgetSize(BUTTON_USB, 250, 75, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_USB, 1,1,1,1);
		QuickMenuRebornSetWidgetLabel(BUTTON_USB, "Mount USB");
	}
}

BUTTON_HANDLER(OnToggleCheckBoxREFRESH)
{
	select_refresh = QuickMenuRebornGetCheckboxValue(CHECKBOX_REFRESH);
	
	if(select_refresh){
		QuickMenuRebornSetWidgetLabel(CHECKBOX_REFRESH_TEXT, "Refresh LiveArea");
	}else{
		QuickMenuRebornSetWidgetLabel(CHECKBOX_REFRESH_TEXT, "Refresh License(s)");
	}
}

BUTTON_HANDLER(OnToggleCheckBoxFTPUSB)
{
	select_WifiUsb = QuickMenuRebornGetCheckboxValue(CHECKBOX_SELECT);
	
	if(select_WifiUsb){
		if (!usbdeviceOFF){
			stopUsb(usbdevice_modid);
			toggle = 1;
			start_thread();
		}
		
		QuickMenuRebornSetWidgetLabel(CHECKBOX_SELECT_TEXT, "FTP is Selected");
		QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, "none");
		
		QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 1,1,1,1);
		QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "FTP Disable");
	}else{
		if(run){
			run = 0;
			ftpvita_fini();
			toggle = 1;
			start_thread();
		}
		QuickMenuRebornSetWidgetLabel(CHECKBOX_SELECT_TEXT, "USB is Selected");
		QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, mountType[count]);
		
		QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 1,1,1,1);
		QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "USB Disable");
	}
}

BUTTON_HANDLER(onPresswifi)
{    
	int val = 0;
    sceRegMgrGetKeyInt("/CONFIG/NET", "wifi_flag", &val);
	
		if(!val)
		{	
			sceRegMgrSetKeyInt("/CONFIG/NET", "wifi_flag", 1);
			sceWlanSetConfiguration(1);
			QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, "WiFi is ON");
		}else if(val)
		{	
			sceRegMgrSetKeyInt("/CONFIG/NET", "wifi_flag", 0);
			sceWlanSetConfiguration(0);
			QuickMenuRebornSetWidgetLabel(BUTTON_SELECT_TEXT, "WiFi is OFF");
		}
}

BUTTON_HANDLER(OnToggleCheckBoxNotifs)
{
    NotifsON = QuickMenuRebornGetCheckboxValue(CHECKBOX_NOTIF);
}

void __unused _start() __attribute__((weak, alias("module_start")));
int __unused module_start(SceSize argc, const void* args) {

	QuickMenuRebornSeparator(SEPARATOR, SCE_SEPARATOR_HEIGHT);

    //Get our checkboxes saved state
    int ret = QuickMenuRebornGetCheckboxValue(CHECKBOX_NOTIF);
    NotifsON = ret == QMR_CONFIG_MGR_ERROR_NOT_EXIST ? false : ret;
	
	//Get our checkboxes saved state
	int ret3 = QuickMenuRebornGetCheckboxValue(CHECKBOX_REFRESH);
	select_refresh = ret3 == QMR_CONFIG_MGR_ERROR_NOT_EXIST ? false : ret3;

	if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITA){
		//Get our checkboxes saved state
		int ret2 = QuickMenuRebornGetCheckboxValue(CHECKBOX_SELECT);
		select_WifiUsb = ret2 == QMR_CONFIG_MGR_ERROR_NOT_EXIST ? false : ret2;
	}else if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITATV){
		//Get our checkboxes saved state
		int ret2 = QuickMenuRebornGetCheckboxValue(CHECKBOX_SELECT);
		select_Wifi = ret2 == QMR_CONFIG_MGR_ERROR_NOT_EXIST ? false : ret2;
	}

	QuickMenuRebornRegisterWidget(PLANE_TITLE, NULL, plane);
    QuickMenuRebornSetWidgetSize(PLANE_TITLE, SCE_PLANE_WIDTH, 70, 0, 0);
    QuickMenuRebornSetWidgetColor(PLANE_TITLE, 1,1,1,0);

    QuickMenuRebornRegisterWidget(TEXT, PLANE_TITLE, text);
    QuickMenuRebornSetWidgetSize(TEXT, SCE_PLANE_WIDTH, 50, 0, 0);
    QuickMenuRebornSetWidgetColor(TEXT, 1,1,1,1);
    QuickMenuRebornSetWidgetPosition(TEXT, 0, 10, 0, 0);
    QuickMenuRebornSetWidgetLabel(TEXT, "File Transfer");
	
	SceIoStat s;
    if((sceIoGetstat(WIFI_TEXTURE_PATH, &s) >= 0) & (sceIoGetstat(USB_TEXTURE_PATH, &s) >= 0)) //File Exists
    {
		QuickMenuRebornRegisterWidget(USB_PLANE, PLANE_TITLE, plane);
        QuickMenuRebornRegisterTexture(USB_TEXTURE, USB_TEXTURE_PATH);
        QuickMenuRebornSetWidgetSize(USB_PLANE, 58, 42, 0, 0);
        QuickMenuRebornSetWidgetColor(USB_PLANE, 1,1,1,1);
		QuickMenuRebornSetWidgetPosition(USB_PLANE, -120, 10, 0, 0);
        QuickMenuRebornSetWidgetTexture(USB_PLANE, USB_TEXTURE);
		
        QuickMenuRebornRegisterWidget(WIFI_PLANE, PLANE_TITLE, plane);
        QuickMenuRebornRegisterTexture(WIFI_TEXTURE, WIFI_TEXTURE_PATH);
        QuickMenuRebornSetWidgetSize(WIFI_PLANE, 58, 42, 0, 0);
        QuickMenuRebornSetWidgetColor(WIFI_PLANE, 1,1,1,1);
		QuickMenuRebornSetWidgetPosition(WIFI_PLANE, 120, 10, 0, 0);
        QuickMenuRebornSetWidgetTexture(WIFI_PLANE, WIFI_TEXTURE);
    }

    QuickMenuRebornRegisterWidget(PLANE_CHECKBOX_NOTIF, NULL, plane);
    QuickMenuRebornSetWidgetSize(PLANE_CHECKBOX_NOTIF, SCE_PLANE_WIDTH, 70, 0, 0);
    QuickMenuRebornSetWidgetColor(PLANE_CHECKBOX_NOTIF, 1,1,1,0);

	QuickMenuRebornRegisterWidget(CHECKBOX_NOTIF_TEXT, PLANE_CHECKBOX_NOTIF, text);
    QuickMenuRebornSetWidgetColor(CHECKBOX_NOTIF_TEXT, 1,1,1,1);
    QuickMenuRebornSetWidgetSize(CHECKBOX_NOTIF_TEXT, 500, 75, 0, 0);
    QuickMenuRebornSetWidgetPosition(CHECKBOX_NOTIF_TEXT, -200, 0, 0, 0);
    QuickMenuRebornSetWidgetLabel(CHECKBOX_NOTIF_TEXT, "Enable/Disable Notifications (FTP)");

	QuickMenuRebornRegisterWidget(CHECKBOX_NOTIF, PLANE_CHECKBOX_NOTIF, check_box);
    QuickMenuRebornSetWidgetSize(CHECKBOX_NOTIF, 48, 48, 0, 0);
    QuickMenuRebornSetWidgetColor(CHECKBOX_NOTIF, 1,1,1,1);
    QuickMenuRebornSetWidgetPosition(CHECKBOX_NOTIF, 350, 0, 0, 0);
	QuickMenuRebornSetCheckBoxState(CHECKBOX_NOTIF, 1);
	QuickMenuRebornSaveCheckBoxState(CHECKBOX_NOTIF, 1);
    QuickMenuRebornAssignDefaultCheckBoxRecall(CHECKBOX_NOTIF);
    QuickMenuRebornAssignDefaultCheckBoxSave(CHECKBOX_NOTIF);
    QuickMenuRebornRegisterEventHanlder(CHECKBOX_NOTIF, QMR_BUTTON_RELEASE_ID, OnToggleCheckBoxNotifs, NULL);
	
	QuickMenuRebornRegisterWidget(PLANE_CHECKBOX_REFRESH, NULL, plane);
    QuickMenuRebornSetWidgetSize(PLANE_CHECKBOX_REFRESH, SCE_PLANE_WIDTH, 70, 0, 0);
    QuickMenuRebornSetWidgetColor(PLANE_CHECKBOX_REFRESH, 1,1,1,0);
	
	QuickMenuRebornRegisterWidget(CHECKBOX_REFRESH_TEXT, PLANE_CHECKBOX_REFRESH, text);
	QuickMenuRebornSetWidgetColor(CHECKBOX_REFRESH_TEXT, 1,1,1,1);
	QuickMenuRebornSetWidgetSize(CHECKBOX_REFRESH_TEXT, 500, 75, 0, 0);
	QuickMenuRebornSetWidgetPosition(CHECKBOX_REFRESH_TEXT, -200, 0, 0, 0);
	QuickMenuRebornSetWidgetLabel(CHECKBOX_REFRESH_TEXT, "Refresh LiveArea");

	QuickMenuRebornRegisterWidget(CHECKBOX_REFRESH, PLANE_CHECKBOX_REFRESH, check_box);
	QuickMenuRebornSetWidgetSize(CHECKBOX_REFRESH, 48, 48, 0, 0);
	QuickMenuRebornSetWidgetColor(CHECKBOX_REFRESH, 1,1,1,1);
	QuickMenuRebornSetWidgetPosition(CHECKBOX_REFRESH, 350, 0, 0, 0);
	QuickMenuRebornSetCheckBoxState(CHECKBOX_REFRESH, 1);
	QuickMenuRebornSaveCheckBoxState(CHECKBOX_REFRESH, 1);
	QuickMenuRebornAssignDefaultCheckBoxRecall(CHECKBOX_REFRESH);
	QuickMenuRebornAssignDefaultCheckBoxSave(CHECKBOX_REFRESH);
	QuickMenuRebornRegisterEventHanlder(CHECKBOX_REFRESH, QMR_BUTTON_RELEASE_ID, OnToggleCheckBoxREFRESH, NULL);
	
	if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITA){
		QuickMenuRebornRegisterWidget(PLANE_CHECKBOX_SELECT, NULL, plane);
		QuickMenuRebornSetWidgetSize(PLANE_CHECKBOX_SELECT, SCE_PLANE_WIDTH, 70, 0, 0);
		QuickMenuRebornSetWidgetColor(PLANE_CHECKBOX_SELECT, 1,1,1,0);
		
		QuickMenuRebornRegisterWidget(CHECKBOX_SELECT_TEXT, PLANE_CHECKBOX_SELECT, text);
		QuickMenuRebornSetWidgetColor(CHECKBOX_SELECT_TEXT, 1,1,1,1);
		QuickMenuRebornSetWidgetSize(CHECKBOX_SELECT_TEXT, 500, 75, 0, 0);
		QuickMenuRebornSetWidgetPosition(CHECKBOX_SELECT_TEXT, -200, 0, 0, 0);

		QuickMenuRebornRegisterWidget(CHECKBOX_SELECT, PLANE_CHECKBOX_SELECT, check_box);
		QuickMenuRebornSetWidgetSize(CHECKBOX_SELECT, 48, 48, 0, 0);
		QuickMenuRebornSetWidgetColor(CHECKBOX_SELECT, 1,1,1,1);
		QuickMenuRebornSetWidgetPosition(CHECKBOX_SELECT, 350, 0, 0, 0);
		QuickMenuRebornSetCheckBoxState(CHECKBOX_SELECT, 1);
		QuickMenuRebornSaveCheckBoxState(CHECKBOX_SELECT, 1);
		QuickMenuRebornAssignDefaultCheckBoxRecall(CHECKBOX_SELECT);
		QuickMenuRebornAssignDefaultCheckBoxSave(CHECKBOX_SELECT);
		QuickMenuRebornRegisterEventHanlder(CHECKBOX_SELECT, QMR_BUTTON_RELEASE_ID, OnToggleCheckBoxFTPUSB, NULL);
		
		QuickMenuRebornRegisterWidget(PLANE_BUTTON_NETWORK, NULL, plane);
		QuickMenuRebornSetWidgetSize(PLANE_BUTTON_NETWORK, SCE_PLANE_WIDTH, 70, 0, 0);
		QuickMenuRebornSetWidgetColor(PLANE_BUTTON_NETWORK, 1,1,1,0);
			
		QuickMenuRebornRegisterWidget(BUTTON_NETWORK_TEXT, PLANE_BUTTON_NETWORK, text);
		QuickMenuRebornSetWidgetColor(BUTTON_NETWORK_TEXT, 1,1,1,1);
		QuickMenuRebornSetWidgetSize(BUTTON_NETWORK_TEXT, 500, 75, 0, 0);
		QuickMenuRebornSetWidgetPosition(BUTTON_NETWORK_TEXT, -200, 0, 0, 0);
			
		QuickMenuRebornRegisterWidget(BUTTON_NETWORK, PLANE_BUTTON_NETWORK, button);
		QuickMenuRebornSetWidgetSize(BUTTON_NETWORK, 150, 50, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_NETWORK, 1,1,1,1);
		QuickMenuRebornRegisterEventHanlder(BUTTON_NETWORK, QMR_BUTTON_RELEASE_ID, onPressNetwork, NULL);
		QuickMenuRebornSetWidgetLabel(BUTTON_NETWORK, "Select");
		QuickMenuRebornSetWidgetPosition(BUTTON_NETWORK, 300, 0, 0, 0);
		QuickMenuRebornAssignOnLoadHandler(OnButtonNetwork, BUTTON_NETWORK);
	}
	
	QuickMenuRebornRegisterWidget(PLANE_BUTTON_SELECT, NULL, plane);
	QuickMenuRebornSetWidgetSize(PLANE_BUTTON_SELECT, SCE_PLANE_WIDTH, 70, 0, 0);
	QuickMenuRebornSetWidgetColor(PLANE_BUTTON_SELECT, 1,1,1,0);
		
	QuickMenuRebornRegisterWidget(BUTTON_SELECT_TEXT, PLANE_BUTTON_SELECT, text);
	QuickMenuRebornSetWidgetColor(BUTTON_SELECT_TEXT, 1,1,1,1);
	QuickMenuRebornSetWidgetSize(BUTTON_SELECT_TEXT, 500, 75, 0, 0);
	QuickMenuRebornSetWidgetPosition(BUTTON_SELECT_TEXT, -200, 0, 0, 0);
		
	QuickMenuRebornRegisterWidget(BUTTON_SELECT, PLANE_BUTTON_SELECT, button);
	QuickMenuRebornSetWidgetSize(BUTTON_SELECT, 150, 50, 0, 0);
	QuickMenuRebornSetWidgetColor(BUTTON_SELECT, 1,1,1,1);
	QuickMenuRebornRegisterEventHanlder(BUTTON_SELECT, QMR_BUTTON_RELEASE_ID, onPressSelect, NULL);
	QuickMenuRebornSetWidgetLabel(BUTTON_SELECT, "Select");
	QuickMenuRebornSetWidgetPosition(BUTTON_SELECT, 300, 0, 0, 0);
	QuickMenuRebornAssignOnLoadHandler(OnButtonSelect, BUTTON_SELECT);

	QuickMenuRebornRegisterWidget(PLANE_BUTTON_ACTIVATE, NULL, plane);
	QuickMenuRebornSetWidgetSize(PLANE_BUTTON_ACTIVATE, SCE_PLANE_WIDTH, 100, 0, 0);
	QuickMenuRebornSetWidgetColor(PLANE_BUTTON_ACTIVATE, 1,1,1,0);
		
	QuickMenuRebornRegisterWidget(BUTTON_ACTIVATE, PLANE_BUTTON_ACTIVATE, button);
	QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
	QuickMenuRebornRegisterEventHanlder(BUTTON_ACTIVATE, QMR_BUTTON_RELEASE_ID, onPressFTP, NULL);
	QuickMenuRebornSetWidgetPosition(BUTTON_ACTIVATE, -280, 0, 0, 0);
	QuickMenuRebornAssignOnLoadHandler(OnButtonFTPLoad, BUTTON_ACTIVATE);
		
	QuickMenuRebornRegisterWidget(BUTTON_REFRESH, PLANE_BUTTON_ACTIVATE, button);
	QuickMenuRebornSetWidgetSize(BUTTON_REFRESH, 270, 75, 0, 0);
	QuickMenuRebornRegisterEventHanlder(BUTTON_REFRESH, QMR_BUTTON_RELEASE_ID, onPressREFRESH, NULL);
	QuickMenuRebornSetWidgetPosition(BUTTON_REFRESH, 0, 0, 0, 0);
	QuickMenuRebornAssignOnLoadHandler(OnButtonREFRESHLoad, BUTTON_REFRESH);
		
	QuickMenuRebornRegisterWidget(BUTTON_USB, PLANE_BUTTON_ACTIVATE, button);
	QuickMenuRebornSetWidgetSize(BUTTON_USB, 250, 75, 0, 0);
	QuickMenuRebornRegisterEventHanlder(BUTTON_USB, QMR_BUTTON_RELEASE_ID, onPressUSB, NULL);
	QuickMenuRebornSetWidgetPosition(BUTTON_USB, 280, 0, 0, 0);
	QuickMenuRebornAssignOnLoadHandler(OnButtonUSBLoad, BUTTON_USB);
    
	taipool_init(1 * 1024 * 1024); // user plugins can't malloc without Libc which is not available in main
	vitaConnect_start();
	usbdisable_start();
		
	sceSysmoduleLoadModule(SCE_SYSMODULE_NOTIFICATION_UTIL);
	sceSysmoduleLoadModule(SCE_SYSMODULE_INCOMING_DIALOG);
	
	// Load modules
    int search_unk[2];
	  SceUID search_modid;
	  search_modid = _vshKernelSearchModuleByName("VitaConnectPatch", search_unk);
	  if(search_modid < 0) {
		patch_modid = taiLoadKernelModule("ur0:QuickMenuReborn/VitaConnectEverywhere/patch.skprx", 0, NULL);
		if (patch_modid >= 0) {
		  int res = taiStartKernelModule(patch_modid, 0, NULL, 0, NULL, NULL);
		  if (res < 0)
			taiStopUnloadKernelModule(patch_modid, 0, NULL, 0, NULL, NULL);
		}
	  }
	search_modid = _vshKernelSearchModuleByName("VitaConnectKernel2", search_unk);
	  if(search_modid < 0) {
		kernel_modid = taiLoadKernelModule("ur0:QuickMenuReborn/VitaConnectEverywhere/kernel.skprx", 0, NULL);
		if (kernel_modid >= 0) {
		  int res = taiStartKernelModule(kernel_modid, 0, NULL, 0, NULL, NULL);
		  if (res < 0)
			taiStopUnloadKernelModule(kernel_modid, 0, NULL, 0, NULL, NULL);
		}
	  }
	user_modid = sceKernelLoadStartModule("ur0:QuickMenuReborn/VitaConnectEverywhere/user.suprx", 0, NULL, 0, NULL, NULL);
	
#ifdef _DEBUG
	ftpvita_set_info_log_cb(sendNotification);
#endif
	ftpvita_set_notif_log_cb(sendNotification);
	
    return SCE_KERNEL_START_SUCCESS;
}

int __unused module_stop(SceSize argc, const void* args) {
    run = 0;
    sceKernelWaitThreadEnd(net_thid, NULL, NULL);

    if (run) {
        vitaConnect_end();
    }

    taipool_term();

	if (usbdevice_modid >= 0)
		stopUsb(usbdevice_modid);

	if(USB_hook >= 0)
        taiHookRelease(USB_hook, ref);

	sceSysmoduleUnloadModule(SCE_SYSMODULE_NOTIFICATION_UTIL);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_INCOMING_DIALOG);

	//Remove our widgets from the list using our refrence ids, it will no longer be displayed
	QuickMenuRebornUnregisterWidget(BUTTON_REFRESH);
	QuickMenuRebornUnregisterWidget(BUTTON_USB);
	QuickMenuRebornUnregisterWidget(BUTTON_ACTIVATE);
	QuickMenuRebornUnregisterWidget(PLANE_BUTTON_ACTIVATE);
	
	QuickMenuRebornUnregisterWidget(BUTTON_SELECT);
	QuickMenuRebornUnregisterWidget(BUTTON_SELECT_TEXT);
	QuickMenuRebornUnregisterWidget(PLANE_BUTTON_SELECT);
	QuickMenuRebornUnregisterWidget(BUTTON_NETWORK);
	QuickMenuRebornUnregisterWidget(BUTTON_NETWORK_TEXT);
	QuickMenuRebornUnregisterWidget(PLANE_BUTTON_NETWORK);
	QuickMenuRebornUnregisterWidget(CHECKBOX_SELECT);
	QuickMenuRebornUnregisterWidget(CHECKBOX_SELECT_TEXT);
	QuickMenuRebornUnregisterWidget(PLANE_CHECKBOX_SELECT);
	
	QuickMenuRebornUnregisterWidget(CHECKBOX_REFRESH);
	QuickMenuRebornUnregisterWidget(CHECKBOX_REFRESH_TEXT);
	QuickMenuRebornUnregisterWidget(PLANE_CHECKBOX_REFRESH);
	
	QuickMenuRebornUnregisterWidget(CHECKBOX_NOTIF);	
	QuickMenuRebornUnregisterWidget(CHECKBOX_NOTIF_TEXT);
	QuickMenuRebornUnregisterWidget(PLANE_CHECKBOX_NOTIF);
	QuickMenuRebornUnregisterTexture(WIFI_TEXTURE_PATH);
    QuickMenuRebornUnregisterTexture(USB_TEXTURE_PATH);
    QuickMenuRebornUnregisterWidget(TEXT);
    QuickMenuRebornUnregisterWidget(PLANE_TITLE);	
    QuickMenuRebornRemoveSeparator(SEPARATOR); //Don't forget this!

    return SCE_KERNEL_STOP_SUCCESS;
}
