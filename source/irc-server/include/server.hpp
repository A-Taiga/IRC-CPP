#include "event_handler.hpp"
#include <exception>
#include <source_location>

#ifndef SERVER_HPP
#define SERVER_HPP

class Server
{
public:
    Server (const char* port);
    ~Server ();
    void run ();
    void listen (int qSize);

private:
    EV::Event event_handler;
    std::string port;
    EV::Udata serverData;
    EV::Udata clientData;
    int listenSocket;
    void setup ();
    void accept ();
    void server_callback (const EV::event_data&&);
    void client_callback (const EV::event_data&&);
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