#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <sys/event.h>
#include <vector>
#include <functional>

#define MAX_EVENTS 10000

enum class Tag
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
    std::function<void(struct kevent*)> callback;
    Tag type;
};

class Kqueue
{
    private:
        int kq;
        timespec timeout;
        int maxEvents;
    public:
        Kqueue ();
        ~Kqueue ();
        std::vector<struct kevent> changeList;
        std::unordered_map<int, std::size_t> refChangeList;
        void add_fd (int fd, short filter, Udata* data, unsigned short flags, unsigned int fflags);
        void remove_fd(int fd);
        void handle_events ();
};



#endif