#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include <ctime>
#include <source_location>
#include <exception>
#include <string>
#include <functional>
#include <unordered_map>
#include <tuple>
#include <cassert>




namespace EV
{

	using fileDescriptor = uint64_t;
	struct event_data
	{
		uint32_t flags;
		fileDescriptor fd;
	};

	using callable = std::function<void(const event_data&&)>;
	using mapKey = std::string;
	using tupeType = std::tuple<std::string, callable>;
	struct Udata
	{
		callable callback;
		fileDescriptor fd;
	};

	class Event_Interface
	{
		protected:
			fileDescriptor evsfd;
			std::unordered_map<mapKey, Udata> map;
		public:
			Event_Interface (fileDescriptor, const std::same_as<tupeType> auto&& ...);
			virtual bool add_read (fileDescriptor, mapKey) = 0;
			virtual void poll() = 0;
	};
	
	class EV_Error : public std::exception
	{
		private:
			std::string message;
		public:
			EV_Error (const char* msg, const std::source_location& = std::source_location::current());
			EV_Error (const std::source_location& = std::source_location::current());
			virtual const char* what() noexcept;
	};
} /* namesapce EV*/


#if defined (__APPLE__) || defined (__BSD__)
#include <sys/event.h>

namespace EV
{
	constexpr uint32_t END = (uint32_t)EV_EOF;
	class Event : public EV::Event_Interface
	{
		private:
		timespec timeout;
		public:	
			Event (timespec _timeout, const std::same_as <tupeType> auto&& ... f);
    		virtual bool add_read (fileDescriptor, mapKey);
			virtual void poll();
	};
} /* namespace KQ */
#endif /* __APPLE__ || __BSD__ */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline EV::Event_Interface::Event_Interface (fileDescriptor fd, const std::same_as <tupeType> auto&& ... f)
: evsfd (fd)
{
	auto l = [&] (auto&& arg)
	{
		auto [it, result] = map.emplace (get<0>(arg), Udata{get<1>(arg)});
		assert (result);
	};
	(l(f),...);
}


inline EV::Event::Event (timespec _timeout, const std::same_as <tupeType> auto&& ... f)
: timeout(_timeout)
, Event_Interface(::kqueue(), std::forward<const tupeType&&>(f)...)
{
	if (evsfd == -1)
		throw EV_Error(std::strerror(errno));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif /* EVENT_HANDLER_HPP */


