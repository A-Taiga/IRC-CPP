#include "kqueue.hpp"
#include <_stdio.h>
#include <format>
#include <sys/event.h>
#include <unistd.h>
#include <iostream>

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

void Kqueue::register_kEvent (fileDescriptor fd , EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data)
{
    if (refChangeList.find(fd) != refChangeList.end())
        throw Kqueue_Error("fd is already in the kq change list");
    
    changeList.push_back({});
    EV_SET( &changeList.back(), fd, static_cast<short> (filter), flags, fflags, 0, (void*) &(data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw Kqueue_Error(std::strerror(errno));

    refChangeList[fd] = changeList.size()-1;
    data.type = Type::KERNEL;
}

void Kqueue::unregister_kEvent (fileDescriptor fd)
{
    if (refChangeList.find(fd) == refChangeList.end())
        throw Kqueue_Error ("fd is not found in the change list");
    
    std::swap(changeList[refChangeList[fd]], changeList.back());
    changeList.pop_back();
    refChangeList.erase(fd);
    close(fd);
}

void Kqueue::update_kEvent (fileDescriptor ident, EVFILT filter, unsigned short flags, unsigned int fflags, Udata& data)
{
    if (refChangeList.find(ident) != refChangeList.end())
        throw Kqueue_Error("fd is not found in the change list");
    
    std::size_t index = refChangeList[ident];
    changeList[index].filter = static_cast<short> (filter);
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    changeList[index].udata = (void*) &data;
    data.type = Type::KERNEL;
}

void Kqueue::update_kEvent (fileDescriptor ident, EVFILT filter, unsigned short flags, unsigned int fflags)
{
    if (refChangeList.find(ident) != refChangeList.end())
        throw Kqueue_Error("fd is not found in the change list");
    
    std::size_t index = refChangeList[ident];
    changeList[index].filter = static_cast<short> (filter);
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
}

void Kqueue::register_uEvent (identifier ident, unsigned short flags, unsigned int fflags, Udata& data)
{
    if (refUserChangeList.find(ident) != refUserChangeList.end())
        throw Kqueue_Error("ident is already in the kq change list");
    changeList.push_back({});
    EV_SET( &changeList.back(), ident, EVFILT_USER, EV_ADD | flags, fflags, 0, static_cast<void*>(&data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw Kqueue_Error(std::strerror(errno));
    
    refUserChangeList[ident] = changeList.size()-1;
    data.type = Type::USER;
}

void Kqueue::unregister_uEvent (identifier ident)
{
    if (refUserChangeList.find(ident) == refUserChangeList.end())
        throw Kqueue_Error("dent is not found in the change list");
        
    std::swap(changeList[refUserChangeList[ident]], changeList.back());
    changeList.pop_back();
    refUserChangeList.erase(ident);
}

void Kqueue::update_uEvent (identifier ident, unsigned short flags, unsigned int fflags, Udata& data)
{
    if (refUserChangeList.find(ident) == refUserChangeList.end())
        throw Kqueue_Error("ident is not found in the change list");

    std::size_t index = refUserChangeList[ident];
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    changeList[index].udata = (void*) &data;
    data.type = Type::USER;
}

void Kqueue::update_uEvent (identifier ident, unsigned short flags, unsigned int fflags)
{
    if (refUserChangeList.find(ident) == refUserChangeList.end())
        throw Kqueue_Error("ident is not found in the change list");
    
    std::size_t index = refUserChangeList[ident];
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
                std::swap(changeList[refChangeList[ev->ident]], changeList.back());
                changeList.pop_back();
                refChangeList.erase(ev->ident);
            }
            else if (ud->type == Type::USER)
            {
                std::swap(changeList[refUserChangeList[ev->ident]], changeList.back());
                changeList.pop_back();
                refUserChangeList.erase(ev->ident);
            }
        }
    }
}

 
Kqueue_Error::Kqueue_Error (std::string _message, const std::source_location location)
: message (_message)
, functionName (location.function_name())
, fileName (location.file_name())
, line (location.line())
{}

void Kqueue_Error::what()
{
    std::cout << fileName << ":" << line << " " <<  functionName << " " << message << std::endl;
}