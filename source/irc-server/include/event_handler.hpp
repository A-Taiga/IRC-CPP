#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include <ctime>
#include <source_location>
#include <exception>
#include <string>
#include <functional>
#include <sys/epoll.h>


#if defined (__APPLE__) || defined (__BSD__)
	typedef struct kevent64_s kevent64_s;
	typedef kevent64_s event_struct;
#endif


#if defined (__linux__)
	typedef struct epoll_event event_struct;
#endif

namespace EV
{
 	struct Udata
    {
		#if defined (__APPLE__) || defined (__BSD__)
        	std::function <void(kevent64_s*)> callback;
		#elif defined (__linux__)
        	std::function <void(event_struct*, int)> callback;
		#endif
		int fd;
    };

	class Parent_handler
	{
		protected:
			int fileDescriptor;
		public:
			Parent_handler (int _fileDescriptor);
			virtual void add_read (int fd, Udata& udata) = 0;
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
namespace EP
{
	class Epoll : public EV::Parent_handler
	{
		private:
		timespec timeout;
		public:
		Epoll (timespec _timeout);
		void add_read(int fd, EV::Udata& udata);
		void poll();
	};
};


namespace EV
{
	class Event : public EP::Epoll
	{
		public:
		Event(timespec timeout);
	};

};
#endif /* __linux__ */

#endif /* EVENT_HANDLER_HPP */