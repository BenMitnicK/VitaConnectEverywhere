/*
  VitaShell
  Copyright (C) 2015-2018, TheFloW

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

#ifndef __REFRESH_H__
#define __REFRESH_H__

#define COUNTUP_WAIT 100 * 1000
#define DIALOG_WAIT  900 * 1000

extern int nbrRefreshItem, refreshOnOff;

int refresh_thread(SceSize args, void *argp);
int license_thread(SceSize args, void *argp);

#endif
