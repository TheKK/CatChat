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

/* ===================== Headers ===================== */
#include "mediaManager.h"

/* ===================== Variables  ===================== */
char g_basePath[MEDIA_MANAGER_MAX_FILE_NAME_LEN];

/* ===================== Prototypes ===================== */
int mdManager_init(const char* basePath);
void mdManager_quit();

int mdManager_addfile(const char* inputPath, const char* outputName);
int mdManager_ifFileExist(const char* filePath);

int mdManager_removeDir(const char* where);

/* ===================== Functions ===================== */
int
mdManager_init(const char* basePath)
{
	mdManager_removeDir(basePath);
	if (mkdir(basePath, 0700) == -1) {
		return 1;
	}

	strncpy(g_basePath, basePath, MEDIA_MANAGER_MAX_FILE_NAME_LEN);

	return 0;
}

void
mdManager_quit()
{
	mdManager_removeDir(g_basePath);
}


int
mdManager_fcopy(const char* inputPath, const char* outputName)
{
	FILE* inputFd;
	FILE* outputFd;
	char buf[MAX_BUFFER_SIZE];
	char outputPath[MEDIA_MANAGER_MAX_FILE_NAME_LEN + sizeof(outputName)];
	uint64_t size, w;

	sprintf(outputPath, "%s/%s", g_basePath, outputName);

	inputFd = fopen(inputPath, "rb");
	if (inputFd == NULL) {
		return 1;
	}

	outputFd = fopen(outputPath, "wb");
	if (outputFd == NULL) {
		fclose(inputFd);
		return 1;
	}

	/* Get file size */
	fseek(inputFd, 0, SEEK_END);
	size = ftell(inputFd);
	fseek(inputFd, 0, SEEK_SET);
	
	/* Write file content */
	while (size) {
		if (size > MAX_BUFFER_SIZE)
			w = MAX_BUFFER_SIZE;
		else
			w = size;

		fread(buf, 1, w, inputFd);
		fwrite(buf, 1, w, outputFd);

		size -= w;
	}

	fclose(inputFd);
	fclose(outputFd);

	return 0;
}

FILE*
mdManager_fopen(const char* fileName, const char* mode)
{
	char path[MEDIA_MANAGER_MAX_FILE_NAME_LEN + sizeof(fileName)];
	FILE* fd;

	fd = fopen(path, mode);
	if (!fd)
		return NULL;

	return fd;
}

int
mdManager_ifFileExist(const char* filePath)
{
	FILE* input;
	char fullPath[MEDIA_MANAGER_MAX_FILE_NAME_LEN + sizeof(filePath)];

	strcpy(fullPath, g_basePath);
	strcat(fullPath, filePath);

	input = fopen(fullPath, "rb");
	if (input) {
		fclose(input);
		return 1;
	} else
		return 0;
}

/* ===================== Private functions ===================== */
int
mdManager_removeDir(const char* where)
{
	DIR* dir;
	char file[MEDIA_MANAGER_MAX_FILE_NAME_LEN];
	struct dirent* ent;

	dir = opendir(where);
	if (dir == NULL) {
		return 0;
	}

	while ((ent = readdir(dir))) {
		if (ent->d_type == DT_DIR) {
			if ((strcmp(ent->d_name, ".") == 0) ||
			    (strcmp(ent->d_name, "..") == 0))
				continue;

			sprintf(file, "%s/%s", where, ent->d_name);

			if (mdManager_removeDir(file) == -1) {
				closedir(dir);
				return -1;
			}
			continue;
		}
		sprintf(file, "%s/%s", where, ent->d_name);
		if (remove(file)) {
			closedir(dir);
			return -1;
		}
	}

	remove(where);
	closedir(dir);

	return 0;
}
