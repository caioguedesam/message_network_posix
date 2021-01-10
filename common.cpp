#include "common.h"

// Transforma endereço IPv4 string
void AddrToStr(const sockaddr *addr, char *str, size_t size) {
    char addrstr[INET_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if(addr->sa_family == AF_INET) {
        sockaddr_in *addr4 = (sockaddr_in *)addr;
        if(!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr, INET_ADDRSTRLEN + 1))
            LogExit("Error converting network address to presentation in AddrToStr");
        port = ntohs(addr4->sin_port);
    }
    else {
        LogExit("Unsupported protocol family");
    }

    if(str)
        snprintf(str, size, "IPv4 %s %hu", addrstr, port);

}

// Inicializa um socket com o protocolo em storage
int CreateSocket(const sockaddr_storage storage) {
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if(s == -1)
        LogExit("Error on socket init");
    return s;
}

// Transforma a string de uma porta em representação do dispositivo para da rede
uint16_t ParsePortFromDevice(const char *portStr) {
    // Analisando a porta
    uint16_t port = (uint16_t)atoi(portStr);
    if(port == 0)
        return 0;
    // Convertendo a representação do porto do dispositivo para a da rede
    return htons(port);
}

void LogExit(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}