#ifndef SERVER_HPP
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

#endif