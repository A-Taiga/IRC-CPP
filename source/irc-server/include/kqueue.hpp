#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <sys/event.h>
#include <type_traits>
#include <vector>
#include <functional>

#define MAX_EVENTS 10000

enum class Tag
{
    BLOCK_SPECIAL,
    CHAR_SPECIAL,
    DIRECTORY,
    FIFO_OR_SOCKET,
    REGULAR_FILE,
    SYMBOLIC_LINK,
    SOCKET,
};

enum class EVFILT: short
{
    READ        = EVFILT_READ,
    WRITE       = EVFILT_WRITE,
    AIO         = EVFILT_AIO,
    VNODE       = EVFILT_VNODE,
    PROC        = EVFILT_PROC,
    SIGNAL      = EVFILT_SIGNAL,
    TIMER       = EVFILT_TIMER,
    #if defined(__APPLE__)
        MACHPORT    = EVFILT_MACHPORT,
    #endif
    FS          = EVFILT_FS,
    VM          = EVFILT_VM,
    EXCEPT      = EVFILT_EXCEPT,
};

enum class Type: short
{
    KERNEL,
    USER,
};

struct Udata
{
    std::function<void(struct kevent*)> callback;
    Type type;
};

class Kqueue
{
    private:
        int kq;
        timespec timeout;
        using fileDescriptor = int;
        using identifier = int;
        std::vector<struct kevent> changeList;
        std::vector<struct kevent> userChangeList;
        std::unordered_map<identifier, std::size_t> refChangeList;
        std::unordered_map<identifier, std::size_t> refUserChangeList;
    public:
        Kqueue ();
        ~Kqueue ();
        void register_event (fileDescriptor ident, EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data);
        void unregister_event (fileDescriptor ident);
        void update_event (fileDescriptor ident, EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data);
        void register_user_event (identifier ident, unsigned short flags, unsigned int fflags, Udata& data);
        void unregister_user_event (identifier ident);
        void update_user_event(identifier ident, unsigned short flags, unsigned int fflags, Udata& data);
        void handle_events ();
};

#endif

/*
look into EV_DISPATCH
close everything on SIGINT
move EV_EOF back into this class
*/