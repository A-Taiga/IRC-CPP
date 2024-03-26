#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include <ctime>
#include <source_location>
#include <exception>
#include <string>
#include <functional>

#include <sys/event.h>

typedef struct kevent64_s kevent64_s;

namespace EV
{
 	struct Udata
    {
        std::function <void(kevent64_s*)> callback;
    };

	class Parent_handler
	{
		protected:
			int fileDescriptor;
		public:
			Parent_handler (int _fileDescriptor);
			virtual void add_read (int fd, const Udata& udata) = 0;
			virtual void poll() = 0;
	};

	class Error : public std::exception
    {
        private:
            std::string message;
        public:
            Error (const char* msg, const std::source_location& = std::source_location::current());
            Error (const std::source_location& = std::source_location::current());
            virtual const char* what() noexcept;
    };
};

#if defined (__APPLE__) || defined (__BSD__)
	#include <sys/event.h>

namespace KQ 
{
    class Kqueue : public EV::Parent_handler
    {
        private:
        timespec timeout;
        public:
        Kqueue (timespec _timeout);
        void add_read(int fd, const EV::Udata& udata);
        void poll();
    };
}

namespace EV 
{
		class Event : public KQ::Kqueue
		{
			public:
			Event (timespec timeout);
		};
};
#endif /* __APPLE__ || __BSD__ */

#if defined (__linux__)
namespace EPOLL
{
	class Epoll : public EV::Parent_handler
	{

	};
};
#endif /* __linux__ */


#endif /* EVENT_HANDLER_HPP */