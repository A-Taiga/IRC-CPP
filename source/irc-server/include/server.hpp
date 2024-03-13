#include <exception>
#include <source_location>
#ifndef SERVER_HPP
#include <string>

#if defined(__linux__)

#elif defined(__APPLE__) | defined (BSD)
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

#elif defined(__APPLE__) | defined (BSD)
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

class Server_Error : std::exception
{
    private:
        std::string message;
    public:
        Server_Error (std::string msg, std::source_location = std::source_location::current());
        Server_Error (std::source_location = std::source_location::current());
        virtual const char* what() const noexcept;
};

#endif