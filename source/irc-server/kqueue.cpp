#include "kqueue.hpp"
#include <_stdio.h>
#include <cstdlib>
#include <cstring>
#include <sys/event.h>
#include <unistd.h>
#include <iostream>
#include <format>


genericDescriptor::genericDescriptor (Type _type): type(_type) {}
genericDescriptor::genericDescriptor (int _value, Type _type): value(_value), type(_type){}
genericDescriptor:: operator int () const {return value;}
Type genericDescriptor::get_type () const {return type;}



fileD_t::fileD_t () : genericDescriptor(Type::KERNEL){}
fileD_t::fileD_t (int value) : genericDescriptor(value, Type::KERNEL){}


userD_t::userD_t () : genericDescriptor(Type::USER){}
userD_t::userD_t (int value) : genericDescriptor(value, Type::USER){}


signalD_t::signalD_t () : genericDescriptor(Type::SIGNAL){}
signalD_t::signalD_t (int value) : genericDescriptor(value, Type::SIGNAL){}

timerD_t::timerD_t () : genericDescriptor(Type::TIMER){}
timerD_t::timerD_t (int value) : genericDescriptor(value, Type::SIGNAL){}




Kqueue::Kqueue(timespec _timeout)
: timeout(_timeout)
{
    kq = ::kqueue();
    if (kq == -1)
        throw std::runtime_error(std::strerror(errno));
}

Kqueue::~Kqueue()
{
    close(kq);
}

void Kqueue::register_kEvent (fileD_t ident , EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data)
{
    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("fd is already in the kq change list");
    
    changeList.push_back({});
    EV_SET( &changeList.back(), ident, static_cast<short> (filter), EV_ADD | flags, fflags, 0, (void*) &(data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw Kqueue_Error(std::strerror(errno));

    indexMap[ident] = changeList.size()-1;
    data.type = Type::KERNEL;
}

void Kqueue::unregister_kEvent (fileD_t ident)
{
    if (indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error ("fd is not found in the change list");

    changeList[indexMap[ident]].flags = EV_DELETE;
    ::kevent(kq, &changeList[indexMap[ident]], 1, nullptr, 0, &timeout);

    std::swap(changeList[indexMap[ident]], changeList.back());
    changeList.pop_back();
    indexMap.erase(ident);
    close(ident);
}

void Kqueue::update_kEvent (fileD_t ident, EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data)
{
    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("fd is not found in the change list");
    
    std::size_t index = indexMap[ident];
    changeList[index].filter = static_cast<short> (filter);
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    changeList[index].udata = (void*) &data;
    data.type = Type::KERNEL;
    kevent(kq, &changeList[indexMap[ident]], 1, nullptr, 0, &timeout);
}

void Kqueue::update_kEvent (fileD_t ident, EVFILT filter, unsigned short flags, unsigned int fflags)
{
    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("fd is not found in the change list");
    
    std::size_t index = indexMap[ident];
    changeList[index].filter = static_cast<short> (filter);
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    kevent(kq, &changeList[indexMap[ident]], 1, nullptr, 0, &timeout);
}

void Kqueue::register_uEvent (userD_t ident, unsigned short flags, unsigned int fflags, Udata& data)
{

    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("ident is already in the kq change list");
    changeList.push_back({});
    EV_SET( &changeList.back(), ident, EVFILT_USER, EV_ADD | flags, fflags, 0, static_cast<void*>(&data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw Kqueue_Error();
    
    indexMap[ident] = changeList.size()-1;
    data.type = Type::USER;
}

void Kqueue::unregister_uEvent (userD_t ident)
{
    if (indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error("dent is not found in the change list");
    
    changeList[indexMap[ident]].flags = EV_DELETE;
    ::kevent(kq, &changeList[indexMap[ident]], 1, nullptr, 0, &timeout);
    
    std::swap(changeList[indexMap[ident]], changeList.back());
    changeList.pop_back();
    indexMap.erase(ident);
}

void Kqueue::update_uEvent (userD_t ident, unsigned short flags, unsigned int fflags, Udata& data)
{
    if (indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error("ident is not found in the change list");

    std::size_t index = indexMap[ident];
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    changeList[index].udata = (void*) &data;
    data.type = Type::USER;
    kevent(kq, &changeList[indexMap[ident]], 1, nullptr, 0, &timeout);
}

void Kqueue::update_uEvent (userD_t ident, unsigned short flags, unsigned int fflags)
{
    if (indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error("ident is not found in the change list");
    
    std::size_t index = indexMap[ident];
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    kevent(kq, &changeList[indexMap[ident]], 1, nullptr, 0, &timeout);
}

void Kqueue::register_signal (signalD_t ident, unsigned short flags, unsigned int fflags, Udata& data)
{
    if(indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("signal is already in the kq change list");
    
    changeList.push_back({});
    EV_SET(&changeList.back(), ident, static_cast<short> (EVFILT::SIGNAL), EV_ADD | flags, fflags, 0, (void*) &(data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw Kqueue_Error(std::strerror(errno));

    indexMap[ident] = changeList.size()-1;
    data.type = Type::SIGNAL;
}

void Kqueue::unregister_signal (signalD_t ident)
{
    if(indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error ("signal is not found in the change list");

    changeList[indexMap[ident]].flags = EV_DELETE;
    ::kevent(kq, &changeList[indexMap[ident]], 1, nullptr, 0, &timeout);

    std::swap(changeList[indexMap[ident]], changeList.back());
    changeList.pop_back();
    indexMap.erase(ident);
}

void Kqueue::update_signal (signalD_t ident, unsigned short flags, unsigned int fflags, Udata& data)
{
    if (indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error("signal is not found in the change list");

    std::size_t index = indexMap[ident];
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    changeList[index].udata = (void*) &data;
    data.type = Type::SIGNAL;
    kevent(kq, &changeList[indexMap[ident]], 1, nullptr, 0, &timeout);
}

void Kqueue::update_signal (signalD_t ident, unsigned short flags, unsigned int fflags)
{
    if (indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error("signal is not found in the change list");
    
    std::size_t index = indexMap[ident];
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    kevent(kq, &changeList[indexMap[ident]], 1, nullptr, 0, &timeout);
}

void Kqueue::register_timer_seconds (timerD_t ident, int time, Udata& data, bool once)
{
    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("timer is already in the kq change list");
    data.type = Type::TIMER;
    timer_helper(ident, time, once ? EV_ONESHOT : 0, NOTE_SECONDS, data);
}

void Kqueue::register_timer_milliseconds (timerD_t ident, int time, Udata& data, bool once)
{
    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("timer is already in the kq change list");
    data.type = Type::TIMER;
    timer_helper(ident, time, once ? EV_ONESHOT : 0, 0, data);
}

void Kqueue::register_timer_microseconds (timerD_t ident, int time, Udata& data, bool once)
{
    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("timer is already in the kq change list");
    data.type = Type::TIMER;
    timer_helper(ident, time, once ? EV_ONESHOT : 0, NOTE_USECONDS, data);
}

void Kqueue::register_timer_nanoseconds (timerD_t ident, int time, Udata& data, bool once)
{
    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("timer is already in the kq change list");
    data.type = Type::TIMER;
    timer_helper(ident, time, once ? EV_ONESHOT : 0, NOTE_NSECONDS, data);
}

#if defined(__MACH__) || defined(__APPLE__)
void Kqueue::register_timer_machtime (timerD_t ident, int time, Udata& data, bool once)
{
    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("timer is already in the kq change list");
    data.type = Type::TIMER;
    timer_helper(ident, time, once ? EV_ONESHOT : 0, NOTE_MACHTIME, data);
}
#endif

void Kqueue::remove_timer (timerD_t ident)
{
    if(indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error ("timer is not found in the change list");
    
    changeList[indexMap[ident]].flags = EV_DELETE;
    ::kevent(kq, &changeList[indexMap[ident]], 1, nullptr, 0, &timeout);
    
    std::swap(changeList[indexMap[ident]], changeList.back());
    changeList.pop_back();
    indexMap.erase(ident);
}

void Kqueue::timer_helper (const timerD_t& ident, const int& time, unsigned short flags, unsigned int fflag, const Udata& data)
{
    changeList.push_back({});
    EV_SET( &changeList.back(), ident, static_cast<short> (EVFILT::TIMER), EV_ADD | flags, fflag, time, (void*) &(data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw Kqueue_Error(std::strerror(errno));
    indexMap[ident] = changeList.size()-1;
}

void Kqueue::handle_events ()
{
    int nChanges;
    struct kevent eventList[MAX_EVENTS];
    nChanges = ::kevent(kq, NULL, 0, eventList, MAX_EVENTS, &timeout);
    if (nChanges == -1)
        throw Kqueue_Error();

    for (int i = 0; i < nChanges; i++)
    {
        struct kevent* ev = &eventList[i];
        Udata* ud = (Udata*)ev->udata;
        if (ev->flags & EV_ERROR)
        {
            std::cout << strerror(errno) << std::endl;
            try
            {
                remove_event(ev->ident, ud->type);
            }
            catch (Kqueue_Error& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
        else
        {
            ud->callback(ev);
        }
        if (ev->flags & EV_ONESHOT)
        {
            try
            {
                remove_event(ev->ident, ud->type);
            } 
            catch (Kqueue_Error& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
    }
}

void Kqueue::remove_event(int ident, Type type)
{
    switch (type)
    {
        case Type::KERNEL:  unregister_kEvent(ident); break;
        case Type::USER:    unregister_uEvent(ident); break;
        case Type::SIGNAL:  unregister_signal(ident); break;
        case Type::TIMER:   remove_timer(ident); break;
        case Type::UNKNOWN: throw Kqueue_Error("Type is unknown");
    }
}

Kqueue_Error::Kqueue_Error (std::string msg, std::source_location location)
: message(std::format("{}:{} {}", location.file_name(), location.line(), msg))
{}
Kqueue_Error::Kqueue_Error (std::source_location location)
: message(std::format("{}:{}", location.file_name(), location.line()))
{}
const char* Kqueue_Error::what() const noexcept
{
    return message.c_str();
}
