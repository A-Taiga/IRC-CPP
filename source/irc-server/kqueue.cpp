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

    if (indexMap.find(ident) != indexMap.end())
        throw Kqueue_Error("fd is already in the kq change list");
    
    changeList.push_back({});
    EV_SET( &changeList.back(), ident, static_cast<short> (filter), flags, fflags, 0, (void*) &(data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw Kqueue_Error(std::strerror(errno));

    indexMap[ident] = changeList.size()-1;
    data.type = Type::KERNEL;
}

void Kqueue::unregister_kEvent (fileDescriptor ident)
{
    if (indexMap.find(ident) == indexMap.end())
        throw Kqueue_Error ("fd is not found in the change list");
    
    std::swap(changeList[indexMap[ident]], changeList.back());
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
        throw Kqueue_Error(std::strerror(errno));
    
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
            switch (ud->type)
            {
                case Type::KERNEL:  std::swap(changeList[indexMap[fileDescriptor(ev->ident)]], changeList.back());
                    break;
                case Type::USER:    std::swap(changeList[indexMap[userDescriptor(ev->ident)]], changeList.back());
                    break;
                case Type::SIGNAL:  std::swap(changeList[indexMap[signalDescriptor(ev->ident)]], changeList.back());
            }
            changeList.pop_back();
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