#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H

#include <sys/socket.h>
#include <string.h>

class ClientData {
    public:
    int socket;
    sockaddr_storage storage;
    ClientData(int socket, sockaddr_storage *storage);
};

#endif