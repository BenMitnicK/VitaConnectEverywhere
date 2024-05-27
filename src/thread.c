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
#include "io_process.h"

#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <taihen.h>

#include <quickmenureborn/qm_reborn.h>

SceUID	s_msgThreadId, s_npbThreadId, net_thid, USB_hook, usbdisable_thread;
tai_hook_ref_t ref;

char vita_ip[16];
unsigned short int vita_port;

static int netctl_cb_id;

void vitaConnect_start() {
    net_thid = sceKernelCreateThread("vitaConnect_thread", vitaConnect_thread, 0x40, 0x10000, 0, 0, NULL);
    sceKernelStartThread(net_thid, 0, NULL);
}

void vitaConnect_end() {
    sceNetCtlInetUnregisterCallback(netctl_cb_id);
    ftpvita_fini();
}

void do_net_connected() {

    print("VitaConnectEverywhereFTP connected\n");

    ftpvita_set_file_buf_size(512 * 1024);

    if (ftpvita_init(vita_ip, &vita_port) >= 0) {

		ftpvita_add_device("ux0:");
		ftpvita_add_device("ur0:");
		ftpvita_add_device("uma0:");
		ftpvita_add_device("imc0:");
		ftpvita_add_device("xmc0:");

		ftpvita_add_device("gro0:");
		ftpvita_add_device("grw0:");

		ftpvita_add_device("os0:");
		ftpvita_add_device("pd0:");
		ftpvita_add_device("sa0:");
		ftpvita_add_device("tm0:");
		ftpvita_add_device("ud0:");
		ftpvita_add_device("vd0:");
		ftpvita_add_device("vs0:");

		ftpvita_add_device("app0:");
		ftpvita_add_device("savedata0:");
		
		ftpvita_add_device("music0:");
		ftpvita_add_device("photo0:");
		
		run = 1;
		s_mesg = 0;
		start_thread();
    }
}

static void* netctl_cb(int event_type, void* arg) {
    print("netctl cb: %d\n", event_type);

    // TODO sceNetCtlInetGetResult

    if ((event_type == 1 || event_type == 2 || event_type == 3) && run_USB == 0) {
		if(select_WifiUsb){
			if(run){
				run = 0;
				onPressedFTP = 1;
				ftpvita_fini();
				start_thread();
			}
		}else{
			if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITA){
				if (!usbdeviceOFF && !select_WifiUsb){
					stopUsb(usbdevice_modid);
					usbdeviceOFF = 1;
					start_thread();
				}
			}
		}
    }

    return NULL;
}

SceUInt32 USBDisable_Patch(void)
{
    //No need to display dialog (Pretend we've shown it before and it connected successfully)
    return 1;
}

int vitaConnect_thread(unsigned int args, void* argp) {
    int ret;

    sceKernelDelayThread(3 * 1000 * 1000);
	
    ret = sceNetCtlInit();
    print("sceNetCtlInit: 0x%08X\n", ret);

    // if already connected to Wi-Fi
    static int state;
    sceNetCtlInetGetState(&state);
    print("sceNetCtlInetGetState: 0x%08X\n", state);
    netctl_cb(state, NULL);

    // FIXME: Add a mutex here, network status might change right before the callback is registered

    ret = sceNetCtlInetRegisterCallback(netctl_cb, NULL, &netctl_cb_id);
    print("sceNetCtlInetRegisterCallback: 0x%08X\n", ret);
	
    while (1) {
        sceNetCtlCheckCallback();
        sceKernelDelayThread(1000 * 1000);
		
		if(!s_mesg)
			checkWifiPlane();	

		SceUdcdDeviceState state;
		sceUdcdGetDeviceState(&state);
		if (state.cable & SCE_UDCD_STATUS_CABLE_DISCONNECTED){
			if(!usbdeviceOFF && !select_WifiUsb){
				stopUsb(usbdevice_modid);
				usbdeviceOFF = 1;
				start_thread();
			}
		}
			
    }

    return 0;
}

SceInt32 thread_user_message(SceSize args, void *argc)
{
	int		res;
	char sendNotifText[256] = "";

	if(select_WifiUsb && sceKernelGetModel() == SCE_KERNEL_MODEL_VITA){
		if(toggle){
			sprintf(sendNotifText, "VitaConnect: USB Disable");
			usbdeviceOFF = 1;
		}else{
			if(onPressedFTP){
				if(run){
					sprintf(sendNotifText, "IP: %s\nPort: %i", vita_ip, vita_port);
					// Set Values
					QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
					QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 0,1,0,1);
					QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "FTP Enable");
				}else{
					sprintf(sendNotifText, "VitaConnect: FTP Disable");
					// Set values
					QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
					QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 1,1,1,1);
					QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "FTP Disable");
				}
				onPressedFTP = 0;
			}
		}
	}else if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITATV){
		if(onPressedFTP){
			if(run){
				sprintf(sendNotifText, "IP: %s\nPort: %i", vita_ip, vita_port);
				// Set Values
				QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
				QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 0,1,0,1);
				QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "FTP Enable");
			}else{
				sprintf(sendNotifText, "VitaConnect: FTP Disable");
				// Set values
				QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
				QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 1,1,1,1);
				QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "FTP Disable");
			}
			onPressedFTP = 0;
		}
	}
	
	if(run_USB == 1){
		sprintf(sendNotifText, "VitaConnect: USB Mounted");
	}else if(run_USB == 2){
		sprintf(sendNotifText, "VitaConnect: USB Not Mounted (Error)");
	}else if(run_USB == 3){
		sprintf(sendNotifText, "VitaConnect: USB Umounted");
	}else if(run_USB == 4){
		sprintf(sendNotifText, "VitaConnect: USB Not Umounted (Error)");
	}else if(run_USB == 5){
		sprintf(sendNotifText, "VitaConnect: UMA0 Mounted");
	}else if(run_USB == 6){
		sprintf(sendNotifText, "VitaConnect: UMA0 Not Mounted (Error)");
	}else if(run_USB == 7){
		sprintf(sendNotifText, "VitaConnect: XMC0 Mounted");
	}else if(run_USB == 8){
		sprintf(sendNotifText, "VitaConnect: XMC0 Not Mounted (Error)");
	}else if(run_USB == 9){
		sprintf(sendNotifText, "VitaConnect: IMC0 Mounted");
	}else if(run_USB == 10){
		sprintf(sendNotifText, "VitaConnect: IMC0 Not Mounted (Error)");
	}else if(run_USB == 11){
		sprintf(sendNotifText, "VitaConnect: Please plug your USB Or PSVSD");
	}else if(run_USB == 12){
		sprintf(sendNotifText, "VitaConnect: Refresh In progress wait...");
	}else if(run_USB == 13){
		if(nbrRefreshItem > 0){
			if(select_refresh){
				sprintf(sendNotifText, "VitaConnect: Refreshed %d items :)", nbrRefreshItem);
			}else{
				sprintf(sendNotifText, "VitaConnect: Imported %d license(s) :)", nbrRefreshItem);
			}
		}else{
			if(select_refresh){
				sprintf(sendNotifText, "VitaConnect: Refreshed %d items :(", nbrRefreshItem);
			}else{
				sprintf(sendNotifText, "VitaConnect: Imported %d license(s) :(", nbrRefreshItem);
			}
		}
	}else if(run_USB == 14){			
		sprintf(sendNotifText, "VitaConnect: Coming Soon :)");
	}
	
	sceKernelDelayThread(USER_MESSAGE_THREAD_TIME);
	
	if(sceKernelGetModel() == SCE_KERNEL_MODEL_VITA){
		if(!select_WifiUsb){
			if(toggle){
				sprintf(sendNotifText, "VitaConnect: FTP Disable");
			}else{
				SceUdcdDeviceState state;
				sceUdcdGetDeviceState(&state);
				if(state.cable & SCE_UDCD_STATUS_CABLE_DISCONNECTED){
					sprintf(sendNotifText, "VitaConnect: Please Connect USB Cable to PC");
					QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
					QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 1,1,1,1);
					QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "USB Disable");
				}else{
					if(usbdeviceOFF){
						QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
						QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 1,1,1,1);
						QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "USB Disable");
						sprintf(sendNotifText, "VitaConnect: USB Disable");
					}else{
						QuickMenuRebornSetWidgetSize(BUTTON_ACTIVATE, 250, 75, 0, 0);
						QuickMenuRebornSetWidgetColor(BUTTON_ACTIVATE, 0,1,1,1);
						QuickMenuRebornSetWidgetLabel(BUTTON_ACTIVATE, "USB Enable");   
						sprintf(sendNotifText, "VitaConnect: USB Connected");
					}
				}
			}
		}
	}
	
	if(strlen(sendNotifText) != '\0')
		sendNotification(sendNotifText); // Send Notification

	run_USB = 0;
	res = sceKernelExitDeleteThread(0);
	return res;
}

int start_thread(void)
{
	int		res;

	res = sceKernelCreateThread(USER_MESSAGE_THREAD_NAME, thread_user_message, 0x40, 0x10000, 0, 0, NULL);
	if (res < SCE_OK) {
		printf("Error: sceKernelCreateThread %08x\n", res);
		return res;
	}
	s_msgThreadId = res;

	res = sceKernelStartThread(s_msgThreadId, 0, NULL);
	if (res < SCE_OK) {
		printf("Error: sceKernelStartThread %08x\n", res);
		return res;
	}

	return res;
}

SceInt32 DisableThread(SceSize args, void *argc)
{
    sceKernelDelayThread(5 * 1000000);

    tai_module_info_t modinfo;
    modinfo.size = sizeof(modinfo);

    if(taiGetModuleInfo("SceShell", &modinfo) >= 0)
    {
        switch (modinfo.module_nid)
        {
            case 0x0552F692: // 3.60 retail
                USB_hook = taiHookFunctionOffset(&ref, modinfo.modid, 0, 0x1c9bfa, 1, USBDisable_Patch); 
                break;

            case 0x587F9CED: // 3.65 testkit
                USB_hook = taiHookFunctionOffset(&ref, modinfo.modid, 0, 0x1c20f6, 1, USBDisable_Patch);
                break;
                
            case 0x5549BF1F: // 3.65 retail
            case 0x34B4D82E: // 3.67 retail
            case 0x12DAC0F3: // 3.68 retail
            case 0x0703C828: // 3.69 retail
            case 0x2053B5A5: // 3.70 retail
            case 0xF476E785: // 3.71 retail
            case 0x939FFBE9: // 3.72 retail
            case 0x734D476A: // 3.73 retail
            case 0x51CB6207: // 3.74 retail
                USB_hook = taiHookFunctionOffset(&ref, modinfo.modid, 0, 0x1c9cc2, 1, USBDisable_Patch);
                break;
        }
    }

    return sceKernelExitDeleteThread(0);
}

int usbdisable_start(void) {
	int		res;
	printf("USBDisable by Ibrahim\n");

	res = sceKernelCreateThread("usbDisable", DisableThread, 191, 0x1000, 0, 0, NULL);
	if (res < SCE_OK) {
		printf("Error: sceKernelCreateThread %08x\n", res);
		return res;
	}
	usbdisable_thread = res;

	res = sceKernelStartThread(usbdisable_thread, 0, NULL);
	printf("USBDisable: %08x - %d\n", res, res);
	if (res < SCE_OK) {
		printf("Error: sceKernelStartThread %08x\n", res);
		return res;
	}

	return res;
}
