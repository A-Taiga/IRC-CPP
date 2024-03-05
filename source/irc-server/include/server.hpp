#ifndef SERVER_HPP
#include <string>


typedef class File_descriptor
{
    private:
    int fd;
    public:
    File_descriptor();
    File_descriptor(int _fd);
    File_descriptor& operator=(File_descriptor&& other) noexcept;
    ~File_descriptor();
    operator int() const {return fd;}


} fd_type;


namespace Server
{
    void Server(const char* _port);
    void listen(int queueSize);
};


// class Server
// {
//     public:
//     Server(const char* _port);
//     void listen(int queueSize);
//     ~Server();
//     private:
//     std::string port;
//     int listenSocket;
//     int testFD;
//     void setup();
// };






#endif