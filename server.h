#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "common.h"
#include "clientData.h"

#define BUFSZ 1024

void Usage(int argc, char **argv);
void *ClientThread(void *data);

#endif