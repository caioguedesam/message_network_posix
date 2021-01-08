#include "clientData.h"

ClientData::ClientData(int socket, sockaddr_storage *storage) {
    this->socket = socket;
    memcpy(&(this->storage), storage, sizeof(*storage));
}