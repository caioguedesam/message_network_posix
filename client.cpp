#include "client.h"

void Usage(int argc, char **argv) {
    printf("usage: %s <server IP> <server port>\n", argv[0]);
    printf("example: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if(argc < 3)
        Usage(argc, argv);

    // Inicializando o socket para comunicação
    sockaddr_storage storage;
    sockaddr *addr = FetchSocketAddress(argv[1], argv[2], &storage);
    int clientSocket = CreateSocket(storage);

    // Abrindo a conexão com o servidor
    if(connect(clientSocket, addr, sizeof(storage)) != 0)
        LogExit("Error on connecting socket to server");

    // Enviando mensagem do teclado para o servidor
    char addrstr[BUFSZ];
    AddrToStr(addr, addrstr, BUFSZ);

    printf("Connected to %s\n", addrstr);

    // Armazena mensagem num buffer de char
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    printf("Mensagem> ");
    fgets(buf, BUFSZ-1, stdin);
    // Envia mensagem (guardando o num. de bytes transmitidos na rede)
    size_t byteCount = send(clientSocket, buf, strlen(buf) + 1, 0);
    if(byteCount != strlen(buf) + 1)
        LogExit("Error on sending message to server");

    // Recebendo resposta do servidor
    memset(buf, 0, BUFSZ);
    unsigned byteTotal = 0;
    while(true) {
        byteCount = recv(clientSocket, buf + byteTotal, BUFSZ - byteTotal, 0);
        if(byteCount == 0) {
            // Não recebeu dados, conexão terminada
            break;
        }
        byteTotal += byteCount;
    }

    printf("Received %u bytes\n", byteTotal);
    puts(buf);

    // Fecha a conexão e o cliente
    close(clientSocket);
    exit(EXIT_SUCCESS);
}