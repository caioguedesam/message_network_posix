#include "server.h"

Server::Server(const char* port) {
    addr = FetchServerAddress(port);
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
sockaddr* Server::FetchServerAddress(const char* portstr) {
    uint16_t port = ParsePortFromDevice(portstr);
    if(port == 0)
        LogExit("Error while parsing port from device when fetching server address on server");

    memset(&storage, 0, sizeof(storage));
    sockaddr_in *addr4 = (sockaddr_in *)(&storage);
    addr4->sin_family = AF_INET;
    addr4->sin_addr.s_addr = INADDR_ANY;
    addr4->sin_port = port;
    return (sockaddr *)(&storage);
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

    // Lidando com mensagens incompletas
    std::string message(buffer);
    size_t nCount = std::count(message.begin(), message.end(), '\n');
    while(nCount == 0) {
        byteCount += recv(clientData->socket, buffer, bufferSize, 0);
        message += std::string(buffer);
        nCount = std::count(message.begin(), message.end(), '\n');
    }
    buffer = &message[0];

    if(byteCount == 0) {
        // Conexão terminada
        return -1;
    }
    return 0;
}

int Server::ParseMessageFromClient(const char *buffer, ClientData *clientData) {
    // Verificando se a mensagem são mais de uma mensagem separadas por \n
    std::string rawMessage(buffer);
    std::vector<std::string> messages = parser.Split(rawMessage, '\n');
    // Para cada mensagem identificada, faz o processo de análise
    for(auto it = messages.begin(); it != messages.end(); ++it) {
        std::string message = *it;

        int clientSocket = clientData->socket;

        // Mensagem inválida
        if(!parser.IsValid(message)) {
            printf("Invalid Message from %d\n", clientSocket);
            return -1;
        }

        // Inscrevendo/Desinscrevendo em tags
        if(parser.IsSubscribe(message)) {
            std::string tag = message;
            // Apaga o identificador +
            tag.erase(0, 1);
            // Vê se a tag já está inscrita
            if(std::find(clientTags[clientSocket].begin(), clientTags[clientSocket].end(), tag) != clientTags[clientSocket].end()) {
                std::string warning = "already subscribed +" + tag + "\n";
                SendMessageToClient(warning, clientSocket);
            }
            else {
                clientTags[clientSocket].push_back(tag);
                std::string warning = "subscribed +" + tag + "\n";
                SendMessageToClient(warning, clientSocket);
            }
        }
        else if(parser.IsUnsubscribe(message)) {
            std::string tag = message;
            // // Apaga o identificador -
            tag.erase(0, 1);
            // Vê se a tag já está inscrita
            auto it = std::find(clientTags[clientSocket].begin(), clientTags[clientSocket].end(), tag);
            if(it == clientTags[clientSocket].end()) {
                std::string warning = "not subscribed -" + tag + "\n";
                SendMessageToClient(warning, clientSocket);
            }
            else {
                clientTags[clientSocket].erase(it);
                std::string warning = "unsubscribed -" + tag + "\n";
                SendMessageToClient(warning, clientSocket);
            }
        }
        // Mata servidor e conexões com clientes
        if(parser.IsKill(message)) {
            KillAll();
        }

        // Envia mensagem normalmente para inscritos nas tags
        std::vector<std::string> tags = parser.GetTags(message);
        // Se não tiver nenhuma tag, continua sem enviar mensagem para outros clientes
        if(tags.empty()) continue;
        message += '\n';
        SendMessageToClients(&message[0], clientData, tags);
    }
    
    return 0;
}

// Manda mensagem qualquer para um cliente, sem \0
void Server::SendMessageToClient(std::string message, const int clientSocket) {
    std::remove(message.begin(), message.end(), '\0');
    send(clientSocket, &message[0], strlen(&message[0]), 0);
}

// Manda mensagem para os clientes que tiverem tags na lista de tags passada na função
void Server::SendMessageToClients(const char *buffer, ClientData *sender, const std::vector<std::string> tags) {
    for(auto it = clients.begin(); it != clients.end(); ++it) {
        // Não manda a mensagem para o remetente
        if(it->second == sender) continue;
        // Não manda pra quem não estiver inscrito em alguma tag da lista
        if(!IsSubscribedToTag(it->second->socket, tags)) {
            continue;
        }
        
        size_t byteCount = send(it->first, buffer, strlen(buffer) + 1, 0);
        if(byteCount != strlen(buffer) + 1)
            LogExit("Error on sending message to client");
    }
}

bool Server::IsSubscribedToTag(const int clientID, const std::vector<std::string> tags) {
    for(auto it = tags.begin(); it != tags.end(); ++it) {
        auto it2 = std::find(clientTags[clientID].begin(), clientTags[clientID].end(), *it);
        // Achou pelo menos uma tag inscrita
        if(it2 != clientTags[clientID].end()) return true;
    }
    return false;
}

void Server::TerminateClientConnection(const int clientID) {
    auto it = clients.find(clientID);
    if(it == clients.end()) return;
    close(clientID);
    delete clients[clientID];
    clients.erase(it);
    printf("Terminated connection with client %d\n", clientID);
}

void Server::KillAll() {
    std::vector<int> clientIDs;
    for(auto it = clients.begin(); it != clients.end(); ++it)
        clientIDs.push_back(it->first);
    for(auto it = clientIDs.begin(); it != clientIDs.end(); ++it)
        TerminateClientConnection(*it);

    exit(EXIT_SUCCESS);
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
        // Analisando mensagem e enviando para outros clientes inscritos caso necessário
        if(server->ParseMessageFromClient(messageBuffer, clientData) == -1)
            break;
    }
    
    // Terminando o socket do cliente e deleta seus dados do servidor
    server->TerminateClientConnection(clientSocket);
    // Terminando a thread
    delete threadData;
    pthread_exit(EXIT_SUCCESS);
}

ThreadData::ThreadData(Server *server, ClientData *clientData) {
    this->server = server;
    this->clientData = clientData;
}

int main(int argc, char **argv) {
    if(argc < 2)
        Usage(argc, argv);
    
    // Inicializando o servidor com determinado protocolo e porta
    Server server = Server(argv[1]);

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
    printf("usage: %s <server port>\n", argv[0]);
    printf("example: %s 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}