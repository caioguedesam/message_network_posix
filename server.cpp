#include "server.h"

void Usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if(argc < 3)
        Usage(argc, argv);

    // Inicializando o socket para comunicação
    // Pegando endereço
    sockaddr_storage storage;
    if(ServerSockaddrInit(argv[1], argv[2], &storage) != 0)
        Usage(argc, argv);
    sockaddr *addr = (sockaddr *)(&storage);
    // Criando socket
    int serverSocket;
    serverSocket = socket(storage.ss_family, SOCK_STREAM, 0);
    if(serverSocket == -1)
        LogExit("Error on socket init");
    // Permitindo socket reutilizar endereços
    int enable = 1;
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0)
        LogExit("Failed to set setsockopt(SO_REUSEADDR)");

    // Atrelando um endereço ao socket inicializado
    if(bind(serverSocket, addr, sizeof(storage)) != 0)
        LogExit("Error when binding name to socket");

    // Preparando para aceitar conexões de clientes
    if(listen(serverSocket, 10) != 0)
        LogExit("Error when preparing to listen to connections");

    char addrstr[BUFSZ];
    AddrToStr(addr, addrstr, BUFSZ);
    printf("Bound to %s, waiting connections\n", addrstr);

    // Recebendo mensagens de clientes
    while(true) {
        sockaddr_storage clientStorage;
        sockaddr *clientAddr = (sockaddr *)(&clientStorage);
        socklen_t clientAddrLen = sizeof(clientStorage);

        int clientSocket = accept(serverSocket, clientAddr, &clientAddrLen);
        if(clientSocket == -1)
            LogExit("Error on accepting connection");

        // Criando estrutura de dados para dados do cliente
        ClientData *clientData = new ClientData(clientSocket, &clientStorage);

        // Criando nova thread para lidar com esse cliente
        pthread_t threadID;
        pthread_create(&threadID, NULL, ClientThread, clientData);
    }

    exit(EXIT_SUCCESS);
}

// Cria nova thread para lidar com um cliente individualmente
void *ClientThread(void *data) {
    // Armazenando dados do cliente
    ClientData *clientData = (ClientData *)data;
    sockaddr *clientAddr = (sockaddr *)&(clientData->storage);
    int clientSocket = clientData->socket;

    char clientAddrStr[BUFSZ];
    AddrToStr(clientAddr, clientAddrStr, BUFSZ);
    printf("[Log] Connection from %s\n", clientAddrStr);

    // Recebendo mensagem do socket do cliente
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    size_t byteCount = recv(clientSocket, buf, BUFSZ, 0);
    printf("[msg] %s, %d bytes: %s\n", clientAddrStr, (int)byteCount, buf);

    // Manda resposta de mensagem recebida para o cliente
    sprintf(buf, "remote endpoint: %.1000s\n", clientAddrStr);
    byteCount = send(clientSocket, buf, strlen(buf) + 1, 0);
    if(byteCount != strlen(buf) + 1)
        LogExit("Error on sending message to client");
    
    // Fecha a thread e o socket do cliente com êxito
    close(clientSocket);
    delete clientData;
    pthread_exit(EXIT_SUCCESS);
}