#include "event_handler.hpp"
#include <cstdint>
#include <ctime>
#include <format>
namespace EV
{
	Parent_handler::Parent_handler (int _fileDescriptor)
	: fileDescriptor(_fileDescriptor)
	{}

    #if defined (__APPLE__) || defined (__BSD__)
        Event::Event (timespec timeout)
        : KQ::Kqueue(timeout)
        {}
    #endif

    #if defined (__linux__)
    Event::Event(timespec timeout)
    : EP::Epoll(timeout)
    {}
    #endif

	Error::Error (const char* msg, const std::source_location& loc)
    {
        message = std::format("{}:{} {}", msg, loc.file_name(), loc.line());
    }

    Error::Error (const std::source_location& loc)
    {
        message = (std::format("{}:{}", loc.file_name(), loc.line()));
    }

    const char* Error::what() noexcept
    {
        return message.c_str();
    }
    
}

#if defined (__APPLE__) || defined (__BSD__)
namespace KQ
{
    Kqueue::Kqueue (timespec _timeout)
    : Parent_handler (::kqueue())
    , timeout {_timeout}
    { 
        if (fileDescriptor == -1)
        {
            puts("kq == -1");
            throw EV::Error(std::strerror(errno));
        }
    }

    void Kqueue::add_read(int fd, const EV::Udata& udata)
    {
        struct kevent64_s ev = {static_cast<uint64_t>(fd), EVFILT_READ, EV_ADD, 0, 0, (uintptr_t)&udata, 0};
        int ret = ::kevent64(fileDescriptor, &ev, 1, nullptr, 0, 0, &timeout);
        if (ret == -1)
            throw EV::Error(std::strerror(errno));
    }

    void Kqueue::poll()
    {
        using namespace EV;
        const std::size_t MAX_EVENTS = 10000;
        kevent64_s eventList[MAX_EVENTS];
        std::size_t N;

        N = kevent64(fileDescriptor, nullptr, 0, eventList, MAX_EVENTS, 0, &timeout);
        if (N == -1)
        {
            puts("kevent64 == -1");
            throw Error(std::strerror(errno));
        }

        for(std::size_t i = 0; i < N; i++)
        {
            kevent64_s *ev = &eventList[i];
            EV::Udata* cb = (EV::Udata*) ev->udata;

            if (ev->flags & EV_ERROR)
            {
                printf("EV_ERROR: %08llu\n", ev->data);
            }
            
            cb->callback(ev);

            if (ev->flags & EV_ONESHOT)
                ev->flags = EV_DELETE;
        }
    }

}
#endif /* __APPLE__ || __BSD__ */

#if defined (__linux__)
#include <sys/epoll.h>
#include <cstring>
namespace EP
{
    Epoll::Epoll(timespec timeout)
    : EV::Parent_handler(::epoll_create1(0))
    , timeout(timeout)
    {
        if (fileDescriptor == -1)
            throw EV::Error(std::strerror(errno));
    }

    void Epoll::add_read(int fd, EV::Udata& udata)
    {
        epoll_event ev = {};
        ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
        udata.fd = fd;
        ev.data.u64 = (uint64_t)&udata;
        int ret = epoll_ctl(fileDescriptor, EPOLL_CTL_ADD, fd, &ev);
        if (ret == -1)
            throw EV::Error(std::strerror(errno));

    }

    void Epoll::poll()
    {
        using namespace EV;
        const std::size_t MAX_EVENTS = 10000;
        epoll_event eventList[MAX_EVENTS];
        std::size_t N;
        N = epoll_wait(fileDescriptor, eventList, MAX_EVENTS, 0);
        if (N == -1)
            throw EV::Error (std::strerror(errno));
        for (std::size_t i = 0; i < N; i++)
        {
            epoll_event* ev = &eventList[i];
            EV::Udata *udata = (EV::Udata*) ev->data.u64;
            udata->callback(ev, udata->fd);
        }
    }
}


#endif /* __linux__ */
