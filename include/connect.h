/*
 * Author: KK <thumbd03803@gmail.com>
 *
 * File: connect.h
 */

#ifndef CONNECT_H
#define CONNECT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MAX_TEXT_SIZE 150

int cnct_Init(int domain, char* sockPath);
int cnct_Quit();
int cnct_Bind();
int cnct_Connect();
int cnct_Listen(int backlog);
int cnct_Accept(struct sockaddr* addr, socklen_t* addr_len);
int cnct_Remove();
int cnct_SendMsg(const char* msg);
int cnct_RecvMsg(const char* buff);

int cnct_Getfd();

#endif /* CONNECT_H */
