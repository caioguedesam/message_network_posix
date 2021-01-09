#include "common.h"

// Analisa o endereço addrstr (decide entre IPv4/6) e o porto, e armazena em storage
int AddrParse(const char* addrstr, const char* portstr, sockaddr_storage* storage) {
    if(addrstr == NULL || portstr == NULL)
        return -1;

    // Analisando o porto
    u_int16_t port = (u_int16_t)atoi(portstr);
    if(port == 0)
        return -1;
    // Convertendo a representação do porto do dispositivo para a da rede (s = short)
    port = htons(port);

    // Analisando o endereço (IPv4 ou IPv6)
    // Tentando protocolo IPv4
    in_addr inaddr4;    // Endereço IPv4 32-bits
    if(inet_pton(AF_INET, addrstr, &inaddr4)) {
        // Confirmado IPv4, preenchendo dados do endereço em estrutura sockaddr_in
        sockaddr_in *addr4 = (sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }
    // Tentando protocolo IPv6
    in6_addr inaddr6;    // Endereço IPv6 128-bits
    if(inet_pton(AF_INET6, addrstr, &inaddr6)) {
        // Confirmado IPv6, preenchendo dados do endereço em estrutura sockaddr_in6
        sockaddr_in6 *addr6 = (sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

// Transforma endereço IPv4 ou IPv6 em string
void AddrToStr(const sockaddr *addr, char *str, size_t size) {
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if(addr->sa_family == AF_INET) {
        version = 4;
        sockaddr_in *addr4 = (sockaddr_in *)addr;
        if(!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr, INET6_ADDRSTRLEN + 1))
            LogExit("Error converting network address to presentation in AddrToStr");
        port = ntohs(addr4->sin_port);
    }
    else if(addr->sa_family == AF_INET6) {
        version = 6;
        sockaddr_in6 *addr6 = (sockaddr_in6 *)addr;
        if(!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr, INET6_ADDRSTRLEN + 1))
            LogExit("Error converting network address to presentation in AddrToStr");
        port = ntohs(addr6->sin6_port);
    }
    else {
        LogExit("Unknown protocol family");
    }

    if(str)
        snprintf(str, size, "IPv%d %s %hu", version, addrstr, port);

    
}

// Armazena o endereço do socket numa estrutura de dados sockaddr,
// baseado nos argumentos do cliente
sockaddr *FetchSocketAddress(const char* arg, const char* port, sockaddr_storage *storage) {
    if(AddrParse(arg, port, storage) != 0)
            LogExit("Error on parsing IP address from arguments");
    return (sockaddr *)storage;
}

// Inicializa um socket com o protocolo em storage
int CreateSocket(const sockaddr_storage storage) {
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if(s == -1)
        LogExit("Error on socket init");
    return s;
}

void LogExit(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}