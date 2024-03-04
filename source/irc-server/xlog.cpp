#include <cerrno>
#include <cstring>
#include <ios>
#include <iostream>
#include <string>
#include <string_view>
#include <filesystem>
#include <mutex>
#include <fstream>

using namespace std::literals::string_view_literals;

/*
  logging for:
  1. output - general messages
  2. messages - messages sent by the users
  3. error - errors that occured during the runtime
  4. access - user lateral movemnt between channels or join
*/

namespace xlog
{
	namespace
	{
		constexpr const std::string_view log_directory_name { "log"sv };
		constexpr const std::string_view output_filename { "log/output.log"sv };
		constexpr const std::string_view messages_filename { "log/message.log"sv };
		constexpr const std::string_view error_filename { "log/error.log"sv };
		constexpr const std::string_view access_filename { "log/access.log"sv };

		/*
			this function should serve as a macro
		*/
		static inline bool fstream_open(std::fstream& stream, const std::string& filename)
		{
			/* open the file witout reseting it */
			stream.open(filename, std::ios_base::app);

			if (false == stream.is_open())
			{
				std::cerr << "failed to open " << filename << "; error: {}" << std::strerror(errno) << std::endl;

				return false;
			}

			return true;
		}
	}

	std::mutex output_mutex { };
	std::mutex messages_mutex { };
	std::mutex error_mutex { };
	std::mutex access_mutex { };
	std::fstream output_stream { };
	std::fstream messages_stream { };
	std::fstream error_stream { };
	std::fstream access_stream { };
	
	bool initialize()
	{
		/* 
			checks if the logging directory exists, if it doesn't it will be created
		*/
		if (false == std::filesystem::exists(xlog::log_directory_name))
		{
			if (false == std::filesystem::create_directory(xlog::log_directory_name))
			{
				std::cerr << "failed to create the folder "
					<< xlog::log_directory_name
					<< "; error: "
					<< std::strerror(errno)
					<< std::endl;

				return false;
			}
		}

		const std::string _log_directory_name(log_directory_name);
		const std::string _output_filename(output_filename);
		const std::string _messages_filename(messages_filename);
		const std::string _error_filename(error_filename);
		const std::string _access_filename(access_filename);

		/* initialize the streams */
		if (false == xlog::fstream_open(xlog::output_stream, _output_filename))
		{
			return false;
		}

		if (false == xlog::fstream_open(xlog::messages_stream, _messages_filename))
		{
			return false;
		}

		if (false == xlog::fstream_open(xlog::error_stream, _error_filename))
		{
			return false;   
		}

		if (false == xlog::fstream_open(xlog::access_stream, _access_filename))
		{
			return false;
		}

		return true;
	}
}
