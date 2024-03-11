#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <exception>
#include <sys/event.h>
#include <type_traits>
#include <vector>
#include <functional>
#include <source_location>

#define MAX_EVENTS 10000


enum class EVFILT: short
{
    READ        = EVFILT_READ,
    WRITE       = EVFILT_WRITE,
    AIO         = EVFILT_AIO,
    VNODE       = EVFILT_VNODE,
    PROC        = EVFILT_PROC,
    SIGNAL      = EVFILT_SIGNAL,
    TIMER       = EVFILT_TIMER,
    FS          = EVFILT_FS,
    NONE,
    #if defined(__APPLE__)
        MACHPORT    = EVFILT_MACHPORT,
        VM          = EVFILT_VM,
        EXCEPT      = EVFILT_EXCEPT,
    #endif
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
        Kqueue (timespec _timeout);
        ~Kqueue ();
        /* register a kernel event */
        void register_kEvent (fileDescriptor ident, EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data);
        /* unregister a kernel event */
        void unregister_kEvent (fileDescriptor ident);
        /* update kernel event */
        void update_kEvent (fileDescriptor ident, EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data);
        /* update kernel event */
        void update_kEvent (fileDescriptor ident, EVFILT filter, unsigned short flags, unsigned int fflags);
        /* register user event */
        void register_uEvent (identifier ident, unsigned short flags, unsigned int fflags, Udata& data);
        /* unregister kernel event */
        void unregister_uEvent (identifier ident);
        /* update user event */
        void update_uEvent(identifier ident, unsigned short flags, unsigned int fflags, Udata& data);
        /* update user event */
        void update_uEvent (identifier ident, unsigned short flags, unsigned int fflags);
        /* run kqueue */
        void handle_events ();
};



class Kqueue_Error : public std::exception
{
    public:
        std::string message;
        std::string fileName;
        int line;
        
        Kqueue_Error (std::string _message, const std::source_location& location = std::source_location::current());
        void what();
};

#endif

/*
look into EV_DISPATCH
close everything on SIGINT
move EV_EOF back into this class
*/


