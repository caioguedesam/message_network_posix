#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

int AddrParse(const char *addrstr, const char *portstr, sockaddr_storage *storage);
void AddrToStr(const sockaddr *addr, char *addrstr, size_t size);
void LogExit(const char* msg);
int ServerSockaddrInit(const char *proto, const char *portstr, sockaddr_storage *storage);

#endif