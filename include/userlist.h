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
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ===================== Macros ===================== */
#define USERLIST_MAX_NAME_SIZE 20

/* ===================== Global variables ===================== */
typedef struct userlist_info_t
{
	int fd;
	char name[USERLIST_MAX_NAME_SIZE];
	time_t loginTime;

	struct userlist_info_t* next;
}userlist_info_t;

typedef struct userlist_list_t
{
	struct userlist_info_t* head;
	struct userlist_info_t* firstAvailable;
	int maxSize;
	int currentSize;
}userlist_list_t;

/* ===================== Functions ===================== */
userlist_list_t* userlist_create(int maxSize);
void userlist_destroy(userlist_list_t* list);

int userlist_add(userlist_list_t* list, int fd, const char* name);
void userlist_remove(userlist_list_t* list, int which);

int userlist_findByFd(userlist_list_t* list, int fd);
int userlist_findByName(userlist_list_t* list, const char* name);

char* userlist_getName(userlist_list_t* list, int which);
int userlist_getFd(userlist_list_t* list, int which);
int userlist_getCurrentSize(userlist_list_t* list);
int userlist_getMaxSize(userlist_list_t* list);

int userlist_isFull(userlist_list_t* list);
int userlist_isUsed(userlist_list_t* list, int which);
