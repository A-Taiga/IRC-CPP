#ifndef EVENTIO_HPP
#define EVENTIO_HPP

#include "kqueue.hpp"

#if defined(__APPLE__) | defined(__MACH__)
	#define KQUEUE
#elif defined (__linux__)
	#define EPOLL
#endif


#if defined(KQUEUE)
	class Event_IO : Kqueue
	{
		
	};
#elif defined (LINUX)
	class Event_IO: Epoll
	{
		
	};
#endif


#endif