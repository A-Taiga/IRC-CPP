#include "event_handler.hpp"
#include <ctime>
#include <format>
#include <sys/event.h>

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
        struct kevent64_s events[10000];
        nEvents = ::kevent64(evsfd, nullptr, 0,  events, 10000, 0, &timeout);
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
