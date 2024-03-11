#include "kqueue.hpp"
#include <_stdio.h>
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
    identity id(std::in_place_index<0>, ident);

    if (refChangeList.find(id) != refChangeList.end())
        throw Kqueue_Error("fd is already in the kq change list");
    
    changeList.push_back({});
    EV_SET( &changeList.back(), ident, static_cast<short> (filter), flags, fflags, 0, (void*) &(data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw Kqueue_Error(std::strerror(errno));

    refChangeList[id] = changeList.size()-1;
    data.type = Type::KERNEL;
    data.id = id;
}

void Kqueue::unregister_kEvent (fileDescriptor ident)
{
    identity id (std::in_place_index<0>, ident);

    if (refChangeList.find(id) == refChangeList.end())
        throw Kqueue_Error ("fd is not found in the change list");
    
    std::swap(changeList[refChangeList[id]], changeList.back());
    changeList.pop_back();
    refChangeList.erase(id);
    close(ident);
}

void Kqueue::update_kEvent (fileDescriptor ident, EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data)
{
    identity id (std::in_place_index<0>, ident);

    if (refChangeList.find(id) != refChangeList.end())
        throw Kqueue_Error("fd is not found in the change list");
    
    std::size_t index = refChangeList[id];
    changeList[index].filter = static_cast<short> (filter);
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    changeList[index].udata = (void*) &data;
    data.type = Type::KERNEL;
    data.id = id;
}

void Kqueue::update_kEvent (fileDescriptor ident, EVFILT filter, unsigned short flags, unsigned int fflags)
{
    identity id (std::in_place_index<0>, ident);

    if (refChangeList.find(id) != refChangeList.end())
        throw Kqueue_Error("fd is not found in the change list");
    
    std::size_t index = refChangeList[id];
    changeList[index].filter = static_cast<short> (filter);
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
}

void Kqueue::register_uEvent (userDescriptor ident, unsigned short flags, unsigned int fflags, Udata& data)
{
    identity id (std::in_place_index<1>, ident);

    if (refChangeList.find(id) != refChangeList.end())
        throw Kqueue_Error("ident is already in the kq change list");
    changeList.push_back({});
    EV_SET( &changeList.back(), ident, EVFILT_USER, EV_ADD | flags, fflags, 0, static_cast<void*>(&data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw Kqueue_Error(std::strerror(errno));
    
    refChangeList[id] = changeList.size()-1;
    data.type = Type::USER;
    data.id = id;
}

void Kqueue::unregister_uEvent (userDescriptor ident)
{
    identity id (std::in_place_index<1>, ident);

    if (refChangeList.find(id) == refChangeList.end())
        throw Kqueue_Error("dent is not found in the change list");
        
    std::swap(changeList[refChangeList[id]], changeList.back());
    changeList.pop_back();
    refChangeList.erase(id);
}

void Kqueue::update_uEvent (userDescriptor ident, unsigned short flags, unsigned int fflags, Udata& data)
{
    identity id (std::in_place_index<1>, ident);

    if (refChangeList.find(id) == refChangeList.end())
        throw Kqueue_Error("ident is not found in the change list");

    std::size_t index = refChangeList[id];
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    changeList[index].udata = (void*) &data;
    data.type = Type::USER;
    data.id = id;

}

void Kqueue::update_uEvent (userDescriptor ident, unsigned short flags, unsigned int fflags)
{
    identity id (std::in_place_index<1>, ident);

    if (refChangeList.find(id) == refChangeList.end())
        throw Kqueue_Error("ident is not found in the change list");
    
    std::size_t index = refChangeList[id];
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
}

void Kqueue::handle_events ()
{
    int nChanges;
    struct kevent eventList[MAX_EVENTS];
    nChanges = ::kevent(kq, changeList.data(), changeList.size(), eventList, MAX_EVENTS, &timeout);
    if (nChanges == -1)
        throw Kqueue_Error(std::strerror(errno));

    for (int i = 0; i < nChanges; i++)
    {
        struct kevent* ev = &eventList[i];
        Udata* ud = (Udata*)ev->udata;

        ud->callback(ev);

        if (ev->flags & EV_ONESHOT || ev->flags & EV_CLEAR)
        {
            
            if (ud->type == Type::KERNEL)
            {
                std::swap(changeList[refChangeList[identity(std::in_place_index<0>, ev->ident)]], changeList.back());
                changeList.pop_back();
            }
            else if (ud->type == Type::USER)
            {
                std::swap(changeList[refChangeList[identity(std::in_place_index<1>, ev->ident)]], changeList.back());
                changeList.pop_back();
            }
        }
    }
}

 
Kqueue_Error::Kqueue_Error (std::string _message, const std::source_location& location)
: message (_message)
, fileName (location.file_name())
, line (location.line())
{}

std::string Kqueue_Error::what()
{
    return std::format("{}:{} {}", fileName, line, message);
}