#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <sys/event.h>
#include <vector>
#include <functional>

#define MAX_EVENTS 10000

enum class FD_types
{
    BLOCK_SPECIAL,
    CHAR_SPECIAL,
    DIRECTORY,
    FIF_OR_SOCKET,
    REGULAR_FILE,
    SYMBOLIC_LINK,
    SOCKET,
};

struct Udata
{
    std::function<void()> callback;
    FD_types type;
};

class Kqueue
{
    private:
    int kq;
    timespec timeout;
    int maxEvents;
    public:
    Kqueue ();
    ~Kqueue();
    std::vector<struct kevent> changeList;
    void monitor (int fd, short filter, Udata* data, unsigned short flags, unsigned int fflags);
    void handle_events ();
};



#endif