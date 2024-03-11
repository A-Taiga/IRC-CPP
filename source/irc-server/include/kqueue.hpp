#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <exception>
#include <sys/event.h>
#include <type_traits>
#include <vector>
#include <functional>
#include <source_location>
#include <variant>

#define MAX_EVENTS 10000


using fileDescriptor = int;
using userDescriptor = int;
using signalDescriptor = int;
using identity = std::variant<fileDescriptor, userDescriptor, signalDescriptor>;


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
    identity id;
};

class Kqueue
{
    private:
        int kq;
        timespec timeout;
        std::vector<struct kevent> changeList;
        std::unordered_map<identity, std::size_t, std::hash<identity>> indexMap;
        
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
        void register_uEvent (userDescriptor ident, unsigned short flags, unsigned int fflags, Udata& data);
        /* unregister kernel event */
        void unregister_uEvent (userDescriptor ident);
        /* update user event */
        void update_uEvent(userDescriptor ident, unsigned short flags, unsigned int fflags, Udata& data);
        /* update user event */
        void update_uEvent (userDescriptor ident, unsigned short flags, unsigned int fflags);
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
        std::string what();
};

#endif

/*
look into EV_DISPATCH
close everything on SIGINT
move EV_EOF back into this class
maybe change the variant to strongly typed ints
*/


