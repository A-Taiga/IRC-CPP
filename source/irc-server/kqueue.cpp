#include "kqueue.hpp"
#include <format>
#include <unistd.h>
#include <iostream>


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

void Kqueue::add_fd (int fd , short filter , Udata* data, unsigned short flags, unsigned int fflags)
{
    if (refChangeList.find(fd) != refChangeList.end())
        throw std::runtime_error("fd is already in the kq change list");
    changeList.push_back({});
    EV_SET( &changeList.back(), fd, filter, flags, fflags, 0, data);
    int ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw std::runtime_error(std::strerror(errno));
    refChangeList[fd] = changeList.size()-1;

}

void Kqueue::remove_fd(int fd)
{
    if (refChangeList.find(fd) == refChangeList.end())
        throw std::runtime_error(std::format("{} fd is not found in the change list", __PRETTY_FUNCTION__));
    
    std::swap(changeList[refChangeList[fd]], changeList.back());
    changeList.pop_back();
    refChangeList.erase(fd);
    close(fd);
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

        ud->callback(ev);
    }
}
