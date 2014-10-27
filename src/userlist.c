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
userlist_create()
{
	userlist_list_t* tmpList;

	tmpList = (userlist_list_t*)malloc(sizeof(userlist_list_t));
	if (tmpList == NULL) {
		perror("userlist_create()");
		return NULL;
	}

	tmpList->currentSize = 0;

	return tmpList;
}

void
userlist_destroy(userlist_list_t* list)
{
	assert(list != NULL);

	int i, size;
	userlist_info_t* head;
	userlist_info_t* next;

	pthread_mutex_lock(&mutex);		/* LOCK */
	size = list->currentSize;
	head = list->head;
	next = list->head->next;
	for (i = 0; i < size; i++) {
		free(head);
		head = next;
		next = next->next;
	}
	pthread_mutex_unlock(&mutex);		/* UNLOCK */

	free(list);
	pthread_mutex_destroy(&mutex);
}

int
userlist_add(userlist_list_t* list, int fd, const char* name)
{
	assert(list != NULL);

	userlist_info_t* tmp;

	/* New user info chunck */
	tmp = (userlist_info_t*)malloc(sizeof(userlist_info_t));
	strncat(tmp->name, name, USERLIST_MAX_NAME_SIZE);
	tmp->fd = fd;
	tmp->tid = 0;	/* TODO finish this */
	tmp->loginTime = time(NULL);
	tmp->next = tmp;
	tmp->prev = tmp;

	pthread_mutex_lock(&mutex);		/* LOCK */
	switch (list->currentSize) {
	case 0:
		list->head = tmp;
		break;
	default:
		tmp->next = list->head;
		list->head->prev = tmp;
		list->head = tmp;
		break;
	}

	list->currentSize++;
	pthread_mutex_unlock(&mutex);		/* UNLOCK */

	return 0;
}

void
userlist_remove(userlist_list_t* list, const char* who)
{
	assert(list != NULL);

	userlist_info_t* seek;

	pthread_mutex_lock(&mutex);		/* LOCK */
	for (seek = list->head; seek != NULL; seek = seek->next) {
		if (strcmp(seek->name, who) == 0) {
			if (seek == list->head)
				list->head = seek->next;
			if (seek->next)
				seek->next->prev = seek->prev;
			if (seek->prev)
				seek->prev->next = seek->next;
			free(seek);
			break;
		}
	}
	list->currentSize--;
	pthread_mutex_unlock(&mutex);		/* UNLOCK */
}

/* FIXME Not sure if this can be thread safe */
userlist_info_t*
userlist_findByFd(userlist_list_t* list, int fd)
{
	assert(list != NULL);

	userlist_info_t* seek;

	for (seek = list->head; seek != NULL; seek = seek->next) {
		if (seek->fd == fd)
			return seek;
	}

	/* Not found */
	return NULL;
}

/* FIXME Not sure if this can be thread safe */
userlist_info_t*
userlist_findByName(userlist_list_t* list, const char* name)
{
	assert(list != NULL);

	userlist_info_t* seek;

	for (seek = list->head; seek != NULL; seek = seek->next) {
		if (strcmp(seek->name, name) == 0)
			return seek;
	}

	/* Not found */
	return NULL;
}

userlist_info_t*
userlist_findByIndex(userlist_list_t* list, int index)
{
	assert(list != NULL);

	int i;
	userlist_info_t* toReturn;

	for (i = 0, toReturn = list->head;
	     (i < index) && (toReturn != NULL);
	     i++, toReturn = toReturn->next);

	return toReturn;
}

/* Not sure if this can be thread safe */
int
userlist_getCurrentSize(userlist_list_t* list)
{
	return list->currentSize;
}
