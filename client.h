#ifndef CLIENT_H
#define CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

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

    Client();
    void ConnectToServer();
    void EnterMessage(char *buffer, const int bufferSize);
    void SendMessage(char *buffer);
    void Subscribe(char *tag);
    void Unsubscribe(char *tag);
    void IsSubscribedToTag(char *tag);
};

void Usage(int argc, char **argv);

#endif