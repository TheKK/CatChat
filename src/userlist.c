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
#include "userlist.h"

/* ===================== Global variables ===================== */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* ===================== Functions ===================== */

userlist_list_t*
userlist_create(int maxSize)
{
	int i;
	userlist_list_t* tmpList;

	tmpList = (userlist_list_t*)malloc(sizeof(userlist_list_t));
	if (tmpList == NULL) {
		perror("userlist_create()");
		return NULL;
	}

	tmpList->head = (userlist_info_t*)malloc(sizeof(userlist_info_t) *
						 maxSize);
	tmpList->firstAvailable = tmpList->head;
	tmpList->maxSize = maxSize;
	tmpList->currentSize = 0;

	for (i = 0; i < maxSize - 1; i++) {
		tmpList->head[i].next = &tmpList->head[i + 1];
		tmpList->head[i].fd = -1;
	}
	tmpList->head[maxSize - 1].next = NULL;
	tmpList->head[maxSize - 1].fd = -1;

	return tmpList;
}

void
userlist_destroy(userlist_list_t* list)
{
	free(list->head);
	pthread_mutex_destroy(&mutex);
}

int
userlist_add(userlist_list_t* list, int fd, const char* name)
{
	pthread_mutex_lock(&mutex);		/* LOCK */
	assert(list != NULL);

	if (list->firstAvailable == NULL)
		return -1;

	strncpy(list->firstAvailable->name, name,
		USERLIST_MAX_NAME_SIZE);
	list->firstAvailable->fd = fd;
	list->firstAvailable->loginTime = time(NULL);
	list->firstAvailable = list->firstAvailable->next;

	list->currentSize++;
	pthread_mutex_unlock(&mutex);		/* UNLOCK */

	return 0;
}

void
userlist_remove(userlist_list_t* list, int which)
{
	pthread_mutex_lock(&mutex);		/* LOCK */
	assert((which >= 0) && (which <= list->maxSize));

	list->head[which].fd = -1;
	list->head[which].next = list->firstAvailable;
	list->firstAvailable = &list->head[which];
	list->currentSize--;
	pthread_mutex_unlock(&mutex);		/* UNLOCK */
}

/* Not sure if this can be thread safe */
int
userlist_findByFd(userlist_list_t* list, int fd)
{
	int i;

	for (i = 0; i < list->currentSize; i++) {
		if (fd == list->head[i].fd)
			return i;
	}

	/* Not found */
	return -1;
}

/* Not sure if this can be thread safe */
int
userlist_findByName(userlist_list_t* list, const char* name)
{
	int i;

	for (i = 0; i < list->currentSize; i++) {
		if (strcmp(name, list->head[i].name) == 0)
			return i;
	}

	/* Not found */
	return -1;
}

/* Not sure if this can be thread safe */
char*
userlist_getName(userlist_list_t* list, int which)
{
	return (char*) &(list->head[which].name);
}

int
userlist_getFd(userlist_list_t* list, int which)
{
	return list->head[which].fd;
}

/* Not sure if this can be thread safe */
int
userlist_getCurrentSize(userlist_list_t* list)
{
	return list->currentSize;
}

int
userlist_getMaxSize(userlist_list_t* list)
{
	return list->maxSize;
}

int
userlist_isFull(userlist_list_t* list)
{
	return (list->maxSize == list->currentSize);
}

int
userlist_isUsed(userlist_list_t* list, int which)
{
	return (list->head[which].fd != -1);
}
