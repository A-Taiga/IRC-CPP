#include <cerrno>
#include <cstring>
#include <ios>
#include <iostream>
#include <string>
#include <filesystem>
#include <mutex>
#include <fstream>

/*
	logging for:
	    output - general messages
	    messages - messages sent by the users
	    error - errors that occured during the runtime
	    access - user lateral movemnt between channels or join
*/

namespace xlog
{
	static const char* log_directory_name { "log" };
	static const char* output_filename { "log/output.log" };
	static const char* messages_filename { "log/message.log" };
	static const char* error_filename { "log/error.log" };
	static const char* access_filename { "log/access.log" };

	std::mutex output_mutex { };
	std::mutex messages_mutex { };
	std::mutex error_mutex { };
	std::mutex access_mutex { };
	std::fstream output_stream { };
	std::fstream messages_stream { };
	std::fstream error_stream { };
	std::fstream access_stream { };


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

		/* initialize the streams */
		if (false == xlog::fstream_open(xlog::output_stream, xlog::output_filename))
		{
			return false;
		}

		if (false == xlog::fstream_open(xlog::messages_stream, xlog::messages_filename))
		{
			return false;
		}

		if (false == xlog::fstream_open(xlog::error_stream, xlog::error_filename))
		{
			return false;   
		}

		if (false == xlog::fstream_open(xlog::access_stream, xlog::access_filename))
		{
			return false;
		}

		return true;
	}
}
