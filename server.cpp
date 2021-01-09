#include "server.h"

Server::Server(const char* protocol, const char* port) {
    addr = FetchServerAddress(protocol, port);
    if(addr == nullptr)
        LogExit("Error while fetching server address on server construction");
    InitializeSocket();
}

Server::~Server() {
    for(auto it = clients.begin(); it != clients.end(); ) {
        delete it->second;
        it = clients.erase(it);
    }
}

// Pega o endereço de todas as interfaces locais para o servidor
// posteriormente amarrar a um socket
sockaddr* Server::FetchServerAddress(const char* protocol, const char* portstr) {
    uint16_t port = ParsePortFromDevice(portstr);
    if(port == 0)
        LogExit("Error while parsing port from device when fetching server address on server");

    memset(&storage, 0, sizeof(storage));
    if(strcmp(protocol, "v4") == 0) {
        sockaddr_in *addr4 = (sockaddr_in *)(&storage);
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        addr4->sin_port = port;
        return (sockaddr *)(&storage);
    }
    else if(strcmp(protocol, "v6") == 0) {
        sockaddr_in6 *addr6 = (sockaddr_in6 *)(&storage);
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any;
        addr6->sin6_port = port;
        return (sockaddr *)(&storage);
    }
    else {
        return nullptr;
    }
}

// Cria o socket do servidor e o amarra às interfaces locais de comunicação,
// com reuso de endereço.
void Server::InitializeSocket() {
    socket = CreateSocket(storage);
    int enable = 1;
    if(setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0)
        LogExit("Failed to set setsockopt(SO_REUSEADDR)");
    // Amarrando endereço ao socket inicializado (INADDR_ANY para todas as interfaces locais)
    if(bind(socket, addr, sizeof(storage)) != 0)
        LogExit("Error when binding name to socket");
    // Preparando para aceitar conexões de clientes
    if(listen(socket, 10) != 0)
        LogExit("Error when preparing to listen to connections");

    char addrstr[BUFSZ];
    AddrToStr(addr, addrstr, BUFSZ);
    printf("Bound to %s, waiting connections\n", addrstr);
}

// Espera a próxima conexão estabelecida com um cliente
int Server::AwaitClientSocket(sockaddr_storage *clientStorage) {
    sockaddr *clientAddr = (sockaddr *)clientStorage;
    socklen_t clientAddrLen = sizeof(*clientStorage);

    int clientSocket = accept(socket, clientAddr, &clientAddrLen);
    if(clientSocket == -1)
        LogExit("Error on accepting connection");

    return clientSocket;
}

void Server::CreateNewClientThread(const int clientSocket, sockaddr_storage *clientStorage) {
    // Criando estrutura de dados para dados do cliente
    ClientData *clientData = new ClientData(clientSocket, clientStorage);
    RegisterClient(clientData);

    // Criando nova thread para lidar com esse cliente
    pthread_t threadID;
    ThreadData *threadData = new ThreadData(this, clientData);
    //pthread_create(&threadID, NULL, ClientThread, clientData);
    pthread_create(&threadID, NULL, ClientThread, threadData);
}

void Server::RegisterClient(ClientData *client) {
    clients.insert(std::pair<int,ClientData*>(client->socket, client));
}

void Server::UnregisterClient(ClientData *client) {
    if(client == nullptr) return;
    
    auto it = clients.find(client->socket);
    if(it != clients.end())
        clients.erase(it);
    delete client;
}

void Server::PrintClients() {
    for(auto it = clients.begin(); it != clients.end(); ++it) {
        printf("Client socket: %d\n", it->first);
    }
}

int Server::ReceiveMessageFromClient(char *buffer, const int bufferSize, ClientData *clientData) {
    memset(buffer, 0, bufferSize);
    size_t byteCount = recv(clientData->socket, buffer, bufferSize, 0);
    if(byteCount == 0) {
        // Conexão terminada
        return -1;
    }
    // Parse message (register tags or send to other clients)
    return 0;
}

void Server::ParseMessageFromClient(const char *buffer, ClientData *clientData) {
    //std::string message(buffer);
    /*for(auto it = message.begin(); it != message.end(); ++it) {
        if((*it) == '\n')
            printf("\\n");
        else if((*it) == '\0')
            printf("\\0");
        else
            printf("%c", *it);
    }
    printf("\n");*/
    /*std::vector<std::string> tokens = parser.Split(message, '\n');
    for(auto it = tokens.begin(); it != tokens.end(); ++it) {
        const char* token = &(*it)[0];
        puts(token);
        printf(" -- ");
    }*/
    //puts(&message[0]);
    SendMessageToClients(buffer, clientData);
}

void Server::SendMessageToClients(const char *buffer, ClientData *sender) {
    for(auto it = clients.begin(); it != clients.end(); ++it) {
        if(it->second == sender) continue;
        size_t byteCount = send(it->first, buffer, strlen(buffer) + 1, 0);
        if(byteCount != strlen(buffer) + 1)
            LogExit("Error on sending message to client");
    }
}

// Thread para lidar com cada cliente separadamente
void *ClientThread(void *data) {
    // Armazenando dados do cliente
    ThreadData *threadData = (ThreadData *)data;
    Server *server = threadData->server;

    ClientData *clientData = threadData->clientData;
    sockaddr *clientAddr = (sockaddr *)&(clientData->storage);
    int clientSocket = clientData->socket;

    char clientAddrStr[BUFSZ];
    AddrToStr(clientAddr, clientAddrStr, BUFSZ);
    printf("[Log] Connection from %s\n", clientAddrStr);

    // Recebendo mensagem do socket do cliente
    char messageBuffer[BUFSZ];
    while(server->ReceiveMessageFromClient(messageBuffer, BUFSZ, clientData) == 0) {
        printf("Received message from client w/ socket %d\n", clientData->socket);
        // Analisando mensagem e enviando para outros clientes inscritos caso necessário
        server->ParseMessageFromClient(messageBuffer, clientData);
    }
    
    // Terminando o socket do cliente e deleta seus dados do servidor
    printf("Terminating client w/ socket %d\n", clientData->socket);
    close(clientSocket);
    server->UnregisterClient(clientData);
    // Terminando a thread
    delete threadData;
    pthread_exit(EXIT_SUCCESS);
}

ThreadData::ThreadData(Server *server, ClientData *clientData) {
    this->server = server;
    this->clientData = clientData;
}

int main(int argc, char **argv) {
    if(argc < 3)
        Usage(argc, argv);
    
    // Inicializando o servidor com determinado protocolo e porta
    Server server = Server(argv[1], argv[2]);

    // Loop para estabelecer conexão com clientes
    while(true) {
        sockaddr_storage clientStorage;
        int clientSocket = server.AwaitClientSocket(&clientStorage);

        // Ao achar um cliente, cria uma thread pra ele
        server.CreateNewClientThread(clientSocket, &clientStorage);
    }

    exit(EXIT_SUCCESS);
}

void Usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}