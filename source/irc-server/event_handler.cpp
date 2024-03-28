#include "event_handler.hpp"
#include <format>
#include <unistd.h>
namespace EV
{
	EV_Error::EV_Error (const char* msg, const std::source_location& loc)
    {
        message = std::format("{}:{} {}", msg, loc.file_name(), loc.line());
    }

    EV_Error::EV_Error (const std::source_location& loc)
    {
        message = (std::format("{}:{}", loc.file_name(), loc.line()));
    }

    const char* EV_Error::what() noexcept
    {
        return message.c_str();
    }
} /* namespace EV */

#if defined (__APPLE__) || defined (__BSD__)
namespace EV
{
    bool Event::add_read (fileDescriptor fd, mapKey key)
    {
        if (!map.contains(key))
        {
            puts("key not found");
            return false;
        }
        struct kevent64_s ev = {fd, EVFILT_READ, EV_ADD, 0, 0, (uintptr_t)&map[key], 0};
        int ret = ::kevent64(evsfd, &ev, 1, nullptr, 0, 0, &timeout);
        return true;
    }
	void Event::poll()
    {
        std::size_t nEvents;
        struct kevent64_s events[MAXEVENTS];
        nEvents = ::kevent64(evsfd, nullptr, 0,  events, MAXEVENTS, 0, &timeout);
        if (nEvents == -1)
            throw EV_Error (std::strerror(errno));
        for (std::size_t i = 0; i < nEvents; i++)
        {
            auto *ev = &events[i];
            Udata *data = (Udata*)ev->udata;
            data->callback({ev->flags, ev->ident});
        }
    }
} /* namespace KQ */
#endif /* __APPLE__ || __BSD__ */

#if defined (__linux__)
namespace EV
{
    bool Event::add_read (fileDescriptor fd, mapKey key)
    {
        if (!map.contains(key))
        {
            puts("key not found");
            return false;
        }

        auto [it, result] = epollMap.emplace(fd, std::move(key));
        if (!result)
        {
            puts("fd is already in map");
            return false;
        }
        struct epoll_event ev = {};
        ev.events = EPOLLIN | EPOLLRDHUP;
        ev.data.fd = fd;
        if (::epoll_ctl(evsfd, EPOLL_CTL_ADD, fd, &ev) == -1)
            throw EV_Error(std::strerror(errno));
        return true;
    }
    void Event::poll()
    {
        std::size_t nEvents;
        struct epoll_event events[MAXEVENTS];
        nEvents = epoll_wait(evsfd, events, MAXEVENTS, timeout.tv_sec);

        if (nEvents == -1)
            throw EV_Error(std::strerror(errno));
        
        for (std::size_t i = 0; i < nEvents; i++)
        {
            struct epoll_event* ev = &events[i];
            map[epollMap[ev->data.fd]].callback({ev->events, (uint64_t)ev->data.fd});
            if (ev->events & EPOLLRDHUP)
                epollMap.erase(ev->data.fd);
        }
    }
}

#endif /* __linux__ */
