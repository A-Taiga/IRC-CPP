#ifndef XLOG_HPP
#define XLOG_HPP

#include <fstream>
#include <mutex>
#include <chrono>
#include <iostream>
#include <sstream>

namespace xlog
{
    extern std::mutex output_mutex;
    extern std::mutex messages_mutex;
    extern std::mutex error_mutex;
    extern std::mutex access_mutex;
    extern std::fstream output_stream;
    extern std::fstream messages_stream;
    extern std::fstream error_stream;
    extern std::fstream access_stream;

    /* initializes the logging module */
    bool initialize();

    template<typename... Args>
    void output(const char* format, const Args&... args)
    {
        xlog::output_mutex.lock();
        
        auto time_now { std::chrono::system_clock::now() };
        /* time_now.time_since_epoch().count() returns the time in nanoseconds */
        auto time_ms { (time_now.time_since_epoch().count() / 1000000) % 1000 };
        auto time_t { std::chrono::system_clock::to_time_t(time_now) };
        auto now_tm { *std::localtime(&time_t) };

        std::stringstream ss;
        ss << std::put_time(&now_tm, "%d-%m-%y %H:%M:%S")
            << "." << time_ms << " " 
            << std::vformat(format, std::make_format_args(args...)) << std::endl;

        std::string message { ss.str() };
        xlog::output_stream << message;
        std::cout << message;
        xlog::output_mutex.unlock();
    }

    template<typename... Args>
    inline void message(const char* format, const Args&... args)
    {
        xlog::messages_mutex.lock();
        
        auto time_now { std::chrono::system_clock::now() };
        /* time_now.time_since_epoch().count() returns the time in nanoseconds */
        auto time_ms { (time_now.time_since_epoch().count() / 1000000) % 1000 };
        auto time_t { std::chrono::system_clock::to_time_t(time_now) };
        auto now_tm { *std::localtime(&time_t) };

        std::stringstream ss;
        ss << std::put_time(&now_tm, "%d-%m-%y %H:%M:%S")
            << "." << time_ms << " " 
            << std::vformat(format, std::make_format_args(args...)) << std::endl;

        std::string message { ss.str() };
        xlog::messages_stream << message;
        std::cout << message;
        xlog::messages_mutex.unlock();
    }

    template<typename... Args>
    inline void error(const char* format, const Args&... args)
    {
        xlog::error_mutex.lock();
        
        auto time_now { std::chrono::system_clock::now() };
        /* time_now.time_since_epoch().count() returns the time in nanoseconds */
        auto time_ms { (time_now.time_since_epoch().count() / 1000000) % 1000 };
        auto time_t { std::chrono::system_clock::to_time_t(time_now) };
        auto now_tm { *std::localtime(&time_t) };

        std::stringstream ss;
        ss << std::put_time(&now_tm, "%d-%m-%y %H:%M:%S")
            << "." << time_ms << " " 
            << std::vformat(format, std::make_format_args(args...)) << std::endl;

        std::string message { ss.str() };
        xlog::error_stream << message;
        std::cout << message;
        xlog::error_mutex.unlock();
    }

    template<typename... Args>
    inline void access(const char* format, const Args&... args)
    {
        xlog::access_mutex.lock();
        
        auto time_now { std::chrono::system_clock::now() };
        /* time_now.time_since_epoch().count() returns the time in nanoseconds */
        auto time_ms { (time_now.time_since_epoch().count() / 1000000) % 1000 };
        auto time_t { std::chrono::system_clock::to_time_t(time_now) };
        auto now_tm { *std::localtime(&time_t) };

        std::stringstream ss;
        ss << std::put_time(&now_tm, "%d-%m-%y %H:%M:%S")
            << "." << time_ms << " " 
            << std::vformat(format, std::make_format_args(args...)) << std::endl;

        std::string message { ss.str() };
        xlog::access_stream << message;
        std::cout << message;
        xlog::access_mutex.unlock();
    }
}

#endif
