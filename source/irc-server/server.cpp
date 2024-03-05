#include "server.hpp"
#include <__utility/integer_sequence.h>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <format>

Server::Server(const char* _port)
: port(_port)
{
    setup();
}

void Server::setup()
{
    const int option = 1;
    int status = 0;
    addrinfo* tempInfo = {nullptr};
    addrinfo* serverInfo = {nullptr};
    addrinfo hints = {};

    hints.ai_family = AF_UNSPEC;        /* ipv4 or ipv6 */
    hints.ai_socktype = SOCK_STREAM;    /* socket type */
    hints.ai_flags = AI_PASSIVE;        /* choose ip for the server */
    hints.ai_protocol = IPPROTO_TCP;    /* TCP protocol */

    status = getaddrinfo(nullptr,port.c_str(), &hints, &tempInfo);
    if (status != 0)
        throw std::runtime_error(std::format("{} : {}", __LINE__, gai_strerror(status)));

    serverInfo = tempInfo;
    while (serverInfo != nullptr)
    {
        listenSocket = socket(serverInfo->ai_family
                            , serverInfo->ai_socktype
                            , serverInfo->ai_protocol);
        
        if (listenSocket == -1)
            throw std::runtime_error(std::format("{} : {}", __LINE__, std::strerror(errno)));

        status = setsockopt(listenSocket
                            , SOL_SOCKET
                            , SO_REUSEADDR
                            , &option
                            , sizeof(option));
        if (status == -1)
            throw std::runtime_error(std::format("{} : {}", __LINE__, std::strerror(errno)));
        
        status = bind(listenSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
        
        if (status == -1)
        {
            close(listenSocket);
            std::cerr << "server bind: " << std::strerror(errno) << std::endl;
            continue;
        }
        else
            break;

        serverInfo = serverInfo->ai_next;
    }

    freeaddrinfo(tempInfo);
    if (serverInfo == nullptr)
        throw std::runtime_error("failed to bind");
}

Server::~Server()
{
    close(listenSocket);
}


