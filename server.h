#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "common.h"
#include "clientData.h"
#include "messageParser.h"

#define BUFSZ 1024

class Server {
    public:
    int socket;
    sockaddr_storage storage;
    sockaddr *addr;
    std::map<int, ClientData*> clients;
    MessageParser parser;

    Server(const char* protocol, const char* port);
    ~Server();
    sockaddr* FetchServerAddress(const char* protocol, const char* portstr);
    void InitializeSocket();
    int AwaitClientSocket(sockaddr_storage *clientStorage);
    
    void CreateNewClientThread(const int clientSocket, sockaddr_storage *clientStorage);
    void RegisterClient(ClientData *client);
    void UnregisterClient(ClientData *client);
    int ReceiveMessageFromClient(char *buffer, const int bufferSize, ClientData *clientData);
    int ParseMessageFromClient(const char *buffer, ClientData *clientData);
    void SendMessageToClients(const char *buffer, ClientData *sender, const std::vector<std::string> tags);

    void PrintClients();
};

class ThreadData {
    public:
    Server *server;
    ClientData *clientData;
    ThreadData(Server *server, ClientData *clientData);
};

void Usage(int argc, char **argv);
void *ClientThread(void *data);

#endif