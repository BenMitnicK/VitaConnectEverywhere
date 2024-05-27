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

#ifndef THREAD_H
#define THREAD_H

#include <vitasdk.h>
#include <taihen.h>

extern char vita_ip[16];
extern unsigned short int vita_port;
extern SceUID	net_thid, USB_hook;
extern tai_hook_ref_t ref;
extern int run_USB;

void vitaConnect_start();
void vitaConnect_end();
int vitaConnect_thread(unsigned int args, void *argp);
int start_thread(void);
int usbdisable_start(void);
void do_net_connected();

#endif
