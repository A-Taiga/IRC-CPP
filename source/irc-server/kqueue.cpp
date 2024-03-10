#include "kqueue.hpp"
#include <format>
#include <sys/event.h>
#include <unistd.h>
#include <iostream>
#include <cassert>

Kqueue::Kqueue()
: timeout{5, 0}
{
    kq = ::kqueue();
    if (kq == -1)
        throw std::runtime_error(std::strerror(errno));
}

Kqueue::~Kqueue()
{
    close(kq);
}

void Kqueue::register_event (fileDescriptor fd , EVFILT filter, unsigned short flags, unsigned int fflags, const Udata& data)
{
    if (refChangeList.find(fd) != refChangeList.end())
        throw std::runtime_error("fd is already in the kq change list");
    changeList.push_back({});
    EV_SET( &changeList.back(), fd, (short)filter, flags, fflags, 0, (void*) &(data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw std::runtime_error(std::strerror(errno));
    refChangeList[fd] = changeList.size()-1;
}

void Kqueue::unregister_event (fileDescriptor fd)
{
    if (refChangeList.find(fd) == refChangeList.end())
        throw std::runtime_error(std::format("{} fd is not found in the change list", __PRETTY_FUNCTION__));
    
    std::swap(changeList[refChangeList[fd]], changeList.back());
    changeList.pop_back();
    refChangeList.erase(fd);
    close(fd);
}

void Kqueue::register_user_event (identifier ident, unsigned short flags, unsigned int fflags, const Udata& data)
{
    if (refUserChangeList.find(ident) != refUserChangeList.end())
        throw std::runtime_error("ident is already in the kq change list");
    changeList.push_back({});
    EV_SET( &changeList.back(), ident, EVFILT_USER, EV_ADD | flags, fflags, 0, (void*) &(data));
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw std::runtime_error(std::strerror(errno));
    
    refUserChangeList[ident] = changeList.size()-1;
}

void Kqueue::unregister_user_event (identifier ident)
{
    if (refUserChangeList.find(ident) == refUserChangeList.end())
        throw std::runtime_error(std::format("{} ident is not found in the change list", __PRETTY_FUNCTION__));
        
    std::swap(changeList[refUserChangeList[ident]], changeList.back());
    changeList.pop_back();
    refUserChangeList.erase(ident);
}

void Kqueue::update_user_event (identifier ident, unsigned short flags, unsigned int fflags, const Udata& data)
{
    if (refUserChangeList.find(ident) == refUserChangeList.end())
        throw std::runtime_error(std::format("{} ident is not found in the change list", __PRETTY_FUNCTION__));
    
    std::size_t index = refUserChangeList[ident];
    changeList[index].flags = flags;
    changeList[index].fflags = fflags;
    changeList[index].udata = (void*) &data;
}

void Kqueue::handle_events ()
{
    int nChanges;
    struct kevent eventList[MAX_EVENTS];
    nChanges = ::kevent(kq, changeList.data(), changeList.size(), eventList, MAX_EVENTS, &timeout);
    if (nChanges == -1)
        throw std::runtime_error(std::strerror(errno));

    for (int i = 0; i < nChanges; i++)
    {
        
        struct kevent* ev = &eventList[i];
        Udata* ud = (Udata*)ev->udata;

        /* This is only taking user defined stuff. Fix that */
        if (ev->flags & EV_ONESHOT)
        {
            std::swap(changeList[refUserChangeList[ev->ident]], changeList.back());
            changeList.pop_back();
            refUserChangeList.erase(ev->ident);
        }
        ud->callback(ev);
    }
}
