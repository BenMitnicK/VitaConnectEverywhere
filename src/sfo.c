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

#include "main.h"
#include "sfo.h"

int getSfoValue(void *buffer, const char *name, uint32_t *value) {
  SfoHeader *header = (SfoHeader *)buffer;
  SfoEntry *entries = (SfoEntry *)((uint32_t)buffer + sizeof(SfoHeader));

  if (header->magic != SFO_MAGIC)
    return VITASHELL_ERROR_INVALID_MAGIC;

  int i;
  for (i = 0; i < header->count; i++) {
    if (strcmp(buffer + header->keyofs + entries[i].nameofs, name) == 0) {
      *value = *(uint32_t *)(buffer + header->valofs + entries[i].dataofs);
      return 0;
    }
  }

  return VITASHELL_ERROR_NOT_FOUND;
}

int getSfoString(void *buffer, const char *name, char *string, int length) {
  SfoHeader *header = (SfoHeader *)buffer;
  SfoEntry *entries = (SfoEntry *)((uint32_t)buffer + sizeof(SfoHeader));

  if (header->magic != SFO_MAGIC)
    return VITASHELL_ERROR_INVALID_MAGIC;

  int i;
  for (i = 0; i < header->count; i++) {
    if (strcmp(buffer + header->keyofs + entries[i].nameofs, name) == 0) {
      memset(string, 0, length);
      strncpy(string, buffer + header->valofs + entries[i].dataofs, length);
      string[length - 1] = '\0';
      return 0;
    }
  }

  return VITASHELL_ERROR_NOT_FOUND;
}

int setSfoValue(void *buffer, const char *name, uint32_t value) {
  SfoHeader *header = (SfoHeader *)buffer;
  SfoEntry *entries = (SfoEntry *)((uint32_t)buffer + sizeof(SfoHeader));

  if (header->magic != SFO_MAGIC)
    return VITASHELL_ERROR_INVALID_MAGIC;

  int i;
  for (i = 0; i < header->count; i++) {
    if (strcmp(buffer + header->keyofs + entries[i].nameofs, name) == 0) {
      *(uint32_t *)(buffer + header->valofs + entries[i].dataofs) = value;
      return 0;
    }
  }

  return VITASHELL_ERROR_NOT_FOUND;
}

int setSfoString(void *buffer, const char *name, const char *string) {
  SfoHeader *header = (SfoHeader *)buffer;
  SfoEntry *entries = (SfoEntry *)((uint32_t)buffer + sizeof(SfoHeader));

  if (header->magic != SFO_MAGIC)
    return VITASHELL_ERROR_INVALID_MAGIC;

  int i;
  for (i = 0; i < header->count; i++) {
    if (strcmp(buffer + header->keyofs + entries[i].nameofs, name) == 0) {
      strcpy(buffer + header->valofs + entries[i].dataofs, string);
      return 0;
    }
  }

  return VITASHELL_ERROR_NOT_FOUND;
}
