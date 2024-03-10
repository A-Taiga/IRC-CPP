#ifndef SERVER_HPP
#include <string>

#if defined(__linux__)

#elif defined(__APPLE__) | defined(__MACH__)
    #include "kqueue.hpp"
#endif

class Server
{
public:
    Server (const char* port);
    ~Server ();
    void run ();
    void listen (int qSize);

private:
#if defined(__linux__)

#elif defined(__APPLE__) | defined(__MACH__)
    Kqueue kq;
    Udata serverData;
    Udata clientData;
    Udata userData;
#endif

    std::string port;
    int listenSocket;
    void setup ();
    void accept ();
    void client_callback(struct kevent* event);
    void server_callback (struct kevent* event);
    void userData_callback (struct kevent* event);
};


#endif