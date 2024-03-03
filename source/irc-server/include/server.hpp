#ifndef SERVER_HPP
#include <string>

class Server
{
    public:
    Server(const char* _port);
    private:
    std::string port;
    int listenSocket;
    void setup();
};


#endif