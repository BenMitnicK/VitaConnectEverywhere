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

#ifndef __FILE_H__
#define __FILE_H__

#define SCE_ERROR_ERRNO_EEXIST 0x80010011
#define SCE_ERROR_ERRNO_ENODEV 0x80010013

#define MAX_PATH_LENGTH 1024
#define MAX_PATH2_LENGTH 2048
#define MAX_NAME_LENGTH 256
#define MAX_SHORT_NAME_LENGTH 64
#define MAX_MOUNT_POINT_LENGTH 16

#define DIRECTORY_SIZE (4 * 1024)
#define TRANSFER_SIZE (128 * 1024)

#define SYMLINK_HEADER_SIZE 4
#define SYMLINK_MAX_SIZE  (SYMLINK_HEADER_SIZE + MAX_PATH_LENGTH)
#define SYMLINK_EXT "lnk"
extern const char symlink_header_bytes[SYMLINK_HEADER_SIZE];


enum FileTypes {
  FILE_TYPE_UNKNOWN,
  FILE_TYPE_ARCHIVE,
  FILE_TYPE_BMP,
  FILE_TYPE_INI,
  FILE_TYPE_JPEG,
  FILE_TYPE_MP3,
  FILE_TYPE_MP4,
  FILE_TYPE_OGG,
  FILE_TYPE_PNG,
  FILE_TYPE_PSP2DMP,
  FILE_TYPE_SFO,
  FILE_TYPE_TXT,
  FILE_TYPE_VPK,
  FILE_TYPE_XML,
};

enum FileSortFlags {
  SORT_NONE,
  SORT_BY_NAME,
  SORT_BY_SIZE,
  SORT_BY_DATE,
};

enum FileMoveFlags {
  MOVE_INTEGRATE  = 0x1, // Integrate directories
  MOVE_REPLACE    = 0x2, // Replace files
};

typedef struct {
  int to_file; // 1: to file, 0: to directory
  char *target_path;
  int target_path_length;
} Symlink;

typedef struct {
  uint64_t *value;
  uint64_t max;
  void (* SetProgress)(uint64_t value, uint64_t max);
  int (* cancelHandler)();
} FileProcessParam;

int allocateReadFile(const char *file, void **buffer);
int ReadFile(const char *file, void *buf, int size);
int WriteFile(const char *file, const void *buf, int size);

int checkFileExist(const char *file);
int checkFolderExist(const char *folder);

char * getBaseDirectory(const char *path);
char * getFilename(const char *path);

int getFileSize(const char *file);
int getFileSha1(const char *file, uint8_t *pSha1Out, FileProcessParam *param);
int getPathInfo(const char *path, uint64_t *size, uint32_t *folders, uint32_t *files, int (* handler)(const char *path));
int removePath(const char *path, FileProcessParam *param);
int copyFile(const char *src_path, const char *dst_path, FileProcessParam *param);
int copyPath(const char *src_path, const char *dst_path, FileProcessParam *param);
int movePath(const char *src_path, const char *dst_path, int flags, FileProcessParam *param);

int getFileType(const char *file);

int getNumberOfDevices();
char **getDevices();

int resolveSimLink(Symlink* symlink, const char *target);
int createSymLink(const char *source_location, const char *target);

#endif
