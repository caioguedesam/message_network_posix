#include "client.h"

Client::Client(const char *addr, const char *port) {
    serverAddr = FetchServerAddress(addr, port);
    if(serverAddr == nullptr)
        LogExit("Error while fetching server address on client construction");
    InitializeSocket();
}

// Analisando o endereço do servidor (IPv4/6) e armazenando em estrutura sockaddr
sockaddr* Client::FetchServerAddress(const char *addrStr, const char *portStr) {
    uint16_t port = ParsePortFromDevice(portStr);
    if(port == 0)
        LogExit("Error while parsing port from device when fetching server address on client");

    in_addr inaddr4;
    if(inet_pton(AF_INET, addrStr, &inaddr4))
        return FetchServerAddress4(inaddr4, port);
    in6_addr inaddr6;
    if(inet_pton(AF_INET6, addrStr, &inaddr6))
        return FetchServerAddress6(inaddr6, port);
    
    return nullptr;
}

sockaddr* Client::FetchServerAddress4(in_addr addr, const uint16_t port) {
    sockaddr_in *addr4 = (sockaddr_in *)(&storage);
    addr4->sin_family = AF_INET;
    addr4->sin_port = port;
    addr4->sin_addr = addr;
    return (sockaddr *)(&storage);
}

sockaddr* Client::FetchServerAddress6(in6_addr addr, const uint16_t port) {
    sockaddr_in6 *addr6 = (sockaddr_in6 *)(&storage);
    addr6->sin6_family = AF_INET6;
    addr6->sin6_port = port;
    memcpy(&(addr6->sin6_addr), &addr, sizeof(addr));
    return (sockaddr *)(&storage);
}

// Cria o socket do cliente para comunicação com o servidor
void Client::InitializeSocket() {
    socket = CreateSocket(storage);
}

// Conecta o socket do cliente ao endereço já estabelecido do servidor
void Client::ConnectToServer() {
    // Conectando
    if(connect(socket, serverAddr, sizeof(storage)) != 0)
        LogExit("Error on connecting socket to server");

    // Mandando mensagem de confirmação
    /*char addrstr[BUFSZ];
    AddrToStr(serverAddr, addrstr, BUFSZ);
    printf("Connected to %s\n", addrstr);*/
}

// Recebe mensagem de entrada do cliente para ser enviada ao servidor
void Client::EnterMessage(char *buffer, const int bufferSize) {
    memset(buffer, 0, bufferSize);
    printf("> ");
    fgets(buffer, bufferSize-1, stdin);
}

// Envia mensagem ao servidor após a entrada pelo cliente
void Client::SendMessage(char *buffer, const int bufferSize) {
    // Enviando mensagem pro servidor
    size_t byteCount = send(socket, buffer, strlen(buffer) + 1, 0);
    if(byteCount != strlen(buffer) + 1)
        LogExit("Error on sending message to server");
}

// Recebe mensagem do servidor, enviada por algum outro cliente
void Client::ReceiveMessage(char *buffer) {
    memset(buffer, 0, BUFSZ);
    size_t byteCount = recv(socket, buffer, BUFSZ, 0);
    if(byteCount == 0) {
        // Conexão terminada, fechar cliente
        Exit();
    }
    puts(buffer);
}

void Client::Exit() {
    close(socket);
    exit(EXIT_SUCCESS);
}

void CloseThread(int s) {
    pthread_exit(EXIT_SUCCESS);
}

void *SendMessageThread(void *data) {
    // Temporary for closing clients
    signal(SIGINT, CloseThread);

    Client *client = (Client *)data;
    char buf[BUFSZ];
    while(true) {
        client->EnterMessage(buf, BUFSZ);
        client->SendMessage(buf, BUFSZ);
    }
}

void *ReceiveMessageThread(void *data) {
    // Temporary for closing clients
    signal(SIGINT, CloseThread);

    Client *client = (Client *)data;
    char buf[BUFSZ];
    while(true) {
        client->ReceiveMessage(buf);
    }
}

int main(int argc, char **argv) {
    if(argc < 3)
        Usage(argc, argv);

    Client client = Client(argv[1], argv[2]);
    client.ConnectToServer();

    // Enviando mensagem para o servidor
    /*char buf[BUFSZ];
    client.EnterMessage(buf, BUFSZ);
    client.SendMessage(buf, BUFSZ);*/
    pthread_t sendThreadID;
    pthread_t receiveThreadID;
    pthread_create(&sendThreadID, NULL, SendMessageThread, &client);
    pthread_create(&receiveThreadID, NULL, ReceiveMessageThread, &client);

    // Recebendo mensagem para o servidor
    //client.ReceiveMessage(buf);
    // Fecha a conexão e o cliente
    //client.Exit();
    /*while(true) {

    }*/
    pthread_join(sendThreadID, NULL);
    pthread_join(receiveThreadID, NULL);

    client.Exit();
}

void Usage(int argc, char **argv) {
    printf("usage: %s <server IP> <server port>\n", argv[0]);
    printf("example: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}