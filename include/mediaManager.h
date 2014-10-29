/*
 * CatChat
 * Copyright (C) 2014 TheKK <thumbd03803@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef MEDIA_MANAGER_H
#define MEDIA_MANAGER_H

/* ===================== Macros ===================== */
#define MEDIA_MANAGER_MAX_FILE_NAME_LEN 100 + 1
#define MAX_BUFFER_SIZE	16 * 1024

/* ===================== Headers ===================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdint.h>
#include <sys/stat.h>

/* ===================== Functions ===================== */
int mdManager_init(const char* basePath);
void mdManager_quit();

int mdManager_fcopy(const char* inputPath, const char* outputName);

FILE* mdManager_fopen(const char* fileName, const char* mode);

int mdManager_rm(const char* fileName);

int mdManager_fileExist(const char* filePath);

#endif	/* MEDIA_MANAGER_H */
