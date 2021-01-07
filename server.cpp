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

        char clientAddrStr[BUFSZ];
        AddrToStr(addr, clientAddrStr, BUFSZ);
        printf("[Log] Connection from %s\n", clientAddrStr);

        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        size_t byteCount = recv(clientSocket, buf, BUFSZ, 0);
        printf("[msg] %s, %d bytes: %s\n", clientAddrStr, (int)byteCount, buf);

        sprintf(buf, "remote endpoint: %.1000s\n", clientAddrStr);
        byteCount = send(clientSocket, buf, strlen(buf) + 1, 0);
        if(byteCount != strlen(buf) + 1)
            LogExit("Error on sending message to client");
        close(clientSocket);
    }

    exit(EXIT_SUCCESS);
}