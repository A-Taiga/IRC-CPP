#include "kqueue.hpp"
#include <stdexcept>
#include <format>
#include <unistd.h>
#include <iostream>
#include <sys/stat.h>
#include <span>



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

void Kqueue::monitor (int fd , short filter , Udata* data, unsigned short flags, unsigned int fflags)
{
    changeList.push_back({});
    int ret;

    EV_SET( &changeList.back()
            , fd
            , filter
            , flags
            , fflags
            , 0
            , data);
    ret = ::kevent(kq, &changeList.back(), 1, nullptr, 0, &timeout);
    if (ret == -1)
        throw std::runtime_error(std::strerror(errno));
}

void Kqueue::handle_events ()
{
    int nChanges;
    struct kevent eventList[MAX_EVENTS];
    nChanges = ::kevent(kq
                    , changeList.data()
                    , changeList.size()
                    , eventList
                    , MAX_EVENTS
                    , &timeout);
    if (nChanges == -1)
        throw std::runtime_error(std::strerror(errno));
    for (int i = 0; i < nChanges; i++)
    {
        struct kevent* ev = &eventList[i];
        Udata* ud = (Udata*)ev->udata;

        if (ev -> flags & EV_ERROR)
        {
            std::cout << std::strerror(errno) << std::endl;
        }
        else if (ev -> flags & EV_EOF)
        {
            // client has dissconnected
        }
        else if (ud->type == FD_types::SOCKET)
        {
            ud->callback();
            std::cout << "SOCKET EVENT" << std::endl;
        }
        else if (ud->type == FD_types::DIRECTORY)
        {
            ud->callback();
        }
    }
}