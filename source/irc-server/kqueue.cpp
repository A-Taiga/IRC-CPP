#include "kqueue.hpp"
#include <_stdio.h>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <format>
#include <sys/event.h>
#include <unistd.h>
#include <iostream>
#include <utility>


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

void Kqueue::register_kEvent (fileDescriptor ident , EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data)
{
    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("fd is already in the kq change list");
    
    changeList.push_back({});
    EV_SET( &changeList.back(), ident, static_cast<short> (filter), flags, fflags, 0, (void*) &(data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw Kqueue_Error("REGISTER");

    indexMap[ident] = changeList.size()-1;
    data.type = Type::KERNEL;
}

void Kqueue::unregister_kEvent (fileDescriptor ident)
{
    if (indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error ("fd is not found in the change list");
    
    std::swap(changeList[indexMap[fileDescriptor(ident)]], changeList.back());
    changeList.pop_back();
    indexMap.erase(ident);
    close(ident);
}

void Kqueue::update_kEvent (fileDescriptor ident, EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data)
{

    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("fd is not found in the change list");
    
    std::size_t index = indexMap[ident];
    changeList[index].filter = static_cast<short> (filter);
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    changeList[index].udata = (void*) &data;
    data.type = Type::KERNEL;
}

void Kqueue::update_kEvent (fileDescriptor ident, EVFILT filter, unsigned short flags, unsigned int fflags)
{
    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("fd is not found in the change list");
    
    std::size_t index = indexMap[ident];
    changeList[index].filter = static_cast<short> (filter);
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
}

void Kqueue::register_uEvent (userDescriptor ident, unsigned short flags, unsigned int fflags, Udata& data)
{

    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("ident is already in the kq change list");
    changeList.push_back({});
    EV_SET( &changeList.back(), ident, EVFILT_USER, EV_ADD | flags, fflags, 0, static_cast<void*>(&data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw Kqueue_Error("REGISTER UEVENT");
    
    indexMap[ident] = changeList.size()-1;
    data.type = Type::USER;
}

void Kqueue::unregister_uEvent (userDescriptor ident)
{
    if (indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error("dent is not found in the change list");
        
    std::swap(changeList[indexMap[ident]], changeList.back());
    changeList.pop_back();
    indexMap.erase(ident);
}

void Kqueue::update_uEvent (userDescriptor ident, unsigned short flags, unsigned int fflags, Udata& data)
{
    if (indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error("ident is not found in the change list");

    std::size_t index = indexMap[ident];
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    changeList[index].udata = (void*) &data;
    data.type = Type::USER;

}

void Kqueue::update_uEvent (userDescriptor ident, unsigned short flags, unsigned int fflags)
{
    if (indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error("ident is not found in the change list");
    
    std::size_t index = indexMap[ident];
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
}

void Kqueue::handle_events ()
{
    int nChanges;
    struct kevent eventList[MAX_EVENTS];
    nChanges = ::kevent(kq, NULL, 0, eventList, MAX_EVENTS, &timeout);
    if (nChanges == -1)
        throw Kqueue_Error("nChanges");

    for (int i = 0; i < nChanges; i++)
    {
        struct kevent* ev = &eventList[i];
        Udata* ud = (Udata*)ev->udata;
        // std::cout << "======================" << std::endl;
        // std::cout << std::format("changeList: {}\nnChanges: {}", changeList.size(), nChanges) << std::endl;
        // std::cout << "======================" << std::endl;

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
        else if (ev->flags & EV_ONESHOT)
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
        else
        {
            ud->callback(ev);
        }
    }
}

void Kqueue::remove_event(int ident, Type type)
{
    switch (type)
    {
        case Type::KERNEL:  unregister_kEvent(ident); break;
        case Type::USER:    unregister_uEvent(ident); break;
        case Type::SIGNAL:  break;
        case Type::UNKNOWN: throw Kqueue_Error("Type is unknown");
    }
}


Kqueue_Error::Kqueue_Error (std::string _message, std::source_location location)
: message (_message)
, fileName (location.file_name())
, line (location.line())
{
    fullmsg = std::format("{}:{} {}", fileName, line, message);
}
const char* Kqueue_Error::what() const noexcept
{
    return fullmsg.c_str();
}
