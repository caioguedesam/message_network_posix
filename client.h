#ifndef CLIENT_H
#define CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "common.h"

#define BUFSZ 1024

class Client {
    public:
    int socket;
    sockaddr_storage storage;
    sockaddr *serverAddr;
    std::vector<char*> tags;

    Client(const char *addr, const char *port);
    sockaddr *FetchServerAddress(const char *addrStr, const char *portStr);
    sockaddr *FetchServerAddress4(in_addr addr, const uint16_t port);
    sockaddr *FetchServerAddress6(in6_addr addr, const uint16_t port);
    void InitializeSocket();
    void ConnectToServer();
    void EnterMessage(char *buffer, const int bufferSize);
    void SendMessage(char *buffer, const int bufferSize);
    void ReceiveMessage(char *buffer);
    void Subscribe(char *tag);
    void Unsubscribe(char *tag);
    void IsSubscribedToTag(char *tag);
    void Exit();
};

void Usage(int argc, char **argv);

#endif