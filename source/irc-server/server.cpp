#include "server.hpp"
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

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

/*
    REPLACE ERROR
    maybe with try catch blocks? 
    to handle the errors
*/
#define ERROR(msg)                                                                             \
	{                                                                                          \
		std::cout << WHITE << __FILE__ << ":" << __LINE__ << RESET << ":"                      \
		          << __PRETTY_FUNCTION__ << ":" << RED << "errno:" << RESET << strerror(errno) \
		          << ":" << RED << msg << RESET << std::endl;                                  \
		std::exit(EXIT_FAILURE);                                                               \
	}



File_descriptor::File_descriptor(): fd(-1){}
File_descriptor::File_descriptor(int _fd): fd(_fd){}
File_descriptor::~File_descriptor() 
{
    if (fd != -1) 
        close (fd);
}

File_descriptor& File_descriptor::operator=(File_descriptor&& other) noexcept
{
    std::swap(other.fd, fd);
    return *this;
}

namespace Server
{

    namespace
    {
        std::string port;
        fd_type listenSocket;
        fd_type testFD;
        void setup();
    };

    void Server(const char* _port)
    {
        port = _port;
        setup();
    }

namespace
{
    void setup()
    {
        const int option      = 1;
        int status            = 0;
        addrinfo* tempInfo    = {};
        addrinfo* serverInfo  = {};
        addrinfo hints        = {};

        hints.ai_family     = AF_UNSPEC;      /* ipv4 or ipv6 */
        hints.ai_socktype   = SOCK_STREAM;    /* socket type */
        hints.ai_flags      = AI_PASSIVE;     /* choose ip for the server */
        hints.ai_protocol   = IPPROTO_TCP;    /* TCP protocol */

        status = getaddrinfo(nullptr,port.c_str(), &hints, &tempInfo);
        if (status != 0)
            std::cerr << __FUNCTION__ << " " << gai_strerror(status) << std::endl;
        
        serverInfo = tempInfo;
        while (serverInfo != nullptr)
        {

            
            listenSocket = socket(serverInfo->ai_family
                                , serverInfo->ai_socktype
                                , serverInfo->ai_protocol);
            if (listenSocket == -1)
                ERROR("socket()");
                
            status = setsockopt(listenSocket
                                , SOL_SOCKET
                                , SO_REUSEADDR
                                , &option
                                , sizeof(option));
            if (status == -1)
                ERROR("setsockopt()");
            
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
            ERROR("failed to bind");
    }
}
    /* current implementation for testing only */
    void listen(int queueSize)
    {
        int s = ::listen(listenSocket, queueSize);
        if (s == -1)
            ERROR("listen()");
        std::cout << GREEN << "server started" << RESET << std::endl;


        sockaddr_in testAddr = {};
        socklen_t clinetLength = 0;
        testFD = accept(listenSocket, (struct sockaddr*)&testAddr, (socklen_t*) &clinetLength);
        if (testFD == -1)
            ERROR("accept()");

        puts("HI");
        char buffer[1024];
        ::recv(listenSocket, buffer, sizeof(buffer), MSG_WAITALL);
        std::cout << buffer << std::endl;
    }

};

