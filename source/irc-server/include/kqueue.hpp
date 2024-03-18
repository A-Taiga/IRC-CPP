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



enum class Type: short
{
    KERNEL,
    USER,
    SIGNAL,
    TIMER,
    UNKNOWN,
};

struct genericDescriptor
{
    int value;
    genericDescriptor (Type _type = Type::UNKNOWN): type(_type) {}
    genericDescriptor (int _value, Type _type): value(_value), type(_type){}
    operator int () const {return value;}
    Type get_type () const {return type;}
    private:
        Type type;
};

template<>
struct std::hash<genericDescriptor>
{
    std::size_t operator () (const genericDescriptor& generic) const
    {
        return std::hash<int>{}(generic.value) ^ std::hash<int>{}(static_cast<int>(generic.get_type()));
    }
};

struct fileD_t : genericDescriptor
{
    fileD_t () : genericDescriptor(Type::KERNEL){}
    fileD_t (int value) : genericDescriptor(value, Type::KERNEL){}
};

struct userD_t : genericDescriptor
{
    userD_t () : genericDescriptor(Type::USER){}
    userD_t (int value) : genericDescriptor(value, Type::USER){}
};

struct signalD_t : genericDescriptor
{
    signalD_t () : genericDescriptor(Type::SIGNAL){}
    signalD_t (int value) : genericDescriptor(value, Type::SIGNAL){}
};

struct timerD_t : genericDescriptor
{
    timerD_t () : genericDescriptor(Type::TIMER){}
    timerD_t (int value) : genericDescriptor(value, Type::SIGNAL){}
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
    FS          = EVFILT_FS,
    NONE,
    #if defined(__APPLE__)
        MACHPORT    = EVFILT_MACHPORT,
        VM          = EVFILT_VM,
        EXCEPT      = EVFILT_EXCEPT,
    #endif
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
        std::vector<struct kevent> changeList;
        std::unordered_map<genericDescriptor, std::size_t, std::hash<genericDescriptor>> indexMap;
        void remove_event(int ident, Type type);
        
    public:
        Kqueue (timespec _timeout);
        ~Kqueue ();
        void register_kEvent (fileD_t ident, EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data);
        void unregister_kEvent (fileD_t ident);
        void update_kEvent  (fileD_t ident, EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data);
        void update_kEvent (fileD_t ident, EVFILT filter, unsigned short flags, unsigned int fflags);
        
        void register_uEvent (userD_t ident, unsigned short flags, unsigned int fflags, Udata& data);
        void unregister_uEvent (userD_t ident);
        void update_uEvent(userD_t ident, unsigned short flags, unsigned int fflags, Udata& data);
        void update_uEvent (userD_t ident, unsigned short flags, unsigned int fflags);

        void register_signal (signalD_t ident, unsigned short flags, unsigned int fflags, Udata& data);
        void unregister_signal (signalD_t ident);
        void update_signal (signalD_t ident, unsigned short flags, unsigned int fflags, Udata& data);
        void update_signal (signalD_t ident, unsigned short flags, unsigned int fflags);

        void register_timer (signalD_t ident, unsigned short flags, unsigned int fflags, Udata& data);
        void unregister_timer (signalD_t ident);
        void update_timer (signalD_t ident, unsigned short flags, unsigned int fflags, Udata& data);
        void update_timer (signalD_t ident, unsigned short flags, unsigned int fflags);

        void handle_events ();
};

class Kqueue_Error : public std::exception
{
    private:
        std::string message;
    public:
        Kqueue_Error (std::string msg, std::source_location location = std::source_location::current());
        Kqueue_Error (std::source_location location = std::source_location::current());

        virtual const char* what() const noexcept;
};

#endif

/*
look into EV_DISPATCH
close everything on SIGINT
move EV_EOF back into this class
*/


