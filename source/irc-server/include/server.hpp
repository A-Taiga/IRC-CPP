#ifndef SERVER_HPP
#include <string>

class Server
{
    public:
    Server (const char* port);
    ~Server ();
    void run ();
    void listen (int qSize);
    private:
    std::string port;
    int listenSocket;
    void setup ();
    void accept ();
};


#endif