#include "server.hpp"
#include "event_handler.hpp"
#include <netdb.h>
#include <netinet/in.h>
#include <ostream>
#include <strings.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <format>
#include <cstring>
#include <string>

#define CLEAR   "\e[2J\e[3J\e[H"
#define BLACK   "\x1B[30;1m"
#define RED     "\x1B[31;1m"
#define GREEN   "\x1B[32;1m"
#define YELLOW  "\x1B[33;1m"
#define BLUE    "\x1B[34;1m"
#define MAGENTA "\x1B[35;1m"
#define CYAN    "\x1B[36;1m"
#define WHITE   "\x1B[37;1m"
#define RESET   "\x1B[0m"

#define B_BLACK    "\x1B[40;1m"
#define B_RED      "\x1B[41;1m"
#define B_GREEN    "\x1B[42;1m"
#define B_YELLOW   "\x1B[43;1m"
#define B_BLUE     "\x1B[44;1m"
#define B_MAGENTA  "\x1B[45;1m"
#define B_CYAN     "\x1B[46;1m"
#define B_WHITE    "\x1B[47;1m"

namespace
{

    void* in_addr (sockaddr* sa)
    {
        switch (sa->sa_family)
        {
            case AF_INET:   return &(reinterpret_cast<sockaddr_in*> (sa) -> sin_addr);
            case AF_INET6:  return &(reinterpret_cast<sockaddr_in6*> (sa) -> sin6_addr);
            default:        throw std::runtime_error(std::format("not ipv4 or ipv6 : {} : {}", __LINE__, __PRETTY_FUNCTION__));
        }
    }

    std::string address (sockaddr_storage& client)
    {
        std::string address (INET6_ADDRSTRLEN, '\0');
        inet_ntop (client.ss_family, in_addr(reinterpret_cast<sockaddr*> (&client)), address.data(), address.length());
        return address;
    }
}

Server::Server (const char* _port)
: port (_port)
, event_handler({5,0}
                , EV::tupeType("serverSocket", [&](auto&& arg){server_callback(std::move(arg));})
                , EV::tupeType("clientSocket", [&](auto&& arg){client_callback(std::move(arg));}))
{
    setup();
    event_handler.add_read(listenSocket, "serverSocket");
}

Server::~Server ()
{
    close(listenSocket);
}

void Server::setup ()
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

    status = getaddrinfo(nullptr, port.c_str(), &hints, &tempInfo);
    if (status != 0)
        throw Server_Error(gai_strerror(status));

    serverInfo = tempInfo;
    while (serverInfo != nullptr)
    {
        listenSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
        if (listenSocket == -1)
            throw Server_Error(std::strerror(errno));

        status = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (status == -1)
            throw Server_Error(std::strerror(errno));
        
        status = bind(listenSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
        if (status == -1)
        {
            close(listenSocket);
            std::cerr << "server bind: " << std::strerror(errno) << std::endl;
            continue;
        }
        else break;
        serverInfo = serverInfo->ai_next;
    }

    freeaddrinfo(tempInfo);
    if (serverInfo == nullptr)
        throw Server_Error("failed to bind");
    
    std::cout << GREEN << "server bound" << RESET << std::endl;
}

void Server::run ()
{
    while (true)
    {
        event_handler.poll();
    }
}

void Server::listen (int qSize)
{
    if (::listen(listenSocket, qSize) == -1)
        throw Server_Error(std::strerror(errno));
    
    std::cout << GREEN << "server is listening on port: " << WHITE << port << RESET << std::endl;
}

void Server::accept ()
{
    std::string clientAddress = {};
    sockaddr_storage connection = {};
    socklen_t connectionSize = 0;
    int clientFd = 0;

    connectionSize = sizeof(connection);
    clientFd = ::accept (listenSocket, reinterpret_cast<sockaddr*>(&connection), &connectionSize);
    if (clientFd == -1)
        throw Server_Error(std::strerror(errno));
    
    clientAddress = address(connection);
    event_handler.add_read(clientFd, "clientSocket");
    std::cout << CYAN "CLIENT FD: " << clientFd << " : " << GREEN << "CLIENT CONNECTED" << RESET << std::endl;
}

void Server::server_callback (const EV::event_data&&)
{
    accept();
}

void Server::client_callback (const EV::event_data&& ev)
{
    std::cout << CYAN << "CLIENT FD: " RESET << ev.fd << " : ";
    if (ev.flags & EV::END)
    {
        std::cout << RED << "DISCONNECTED" << RESET << std::endl;
        close (ev.fd);
    }
    else
    {
        char buffer[1024];
        bzero(buffer, sizeof(buffer));
        ::recv(ev.fd, buffer, sizeof(buffer), 0);
        std::cout << YELLOW << "DATA: " << RESET << buffer;
    }
}

Server_Error::Server_Error (std::string msg, std::source_location location)
{
    message = std::format("{}:{} {}", location.file_name(), location.line(), msg);
}

Server_Error::Server_Error (std::source_location location)
{
    message = (std::format("{}:{}", location.file_name(), location.line()));
}

const char* Server_Error::what() const noexcept
{
    return message.c_str();
}
