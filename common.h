#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

void AddrToStr(const sockaddr *addr, char *addrstr, size_t size);
void LogExit(const char* msg);

int CreateSocket(const sockaddr_storage storage);
uint16_t ParsePortFromDevice(const char *portStr);

#endif