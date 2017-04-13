/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_LOGGER_H_
#define _YATO_ACTORS_LOGGER_H_

#include <cassert>
#include <array>

namespace yato
{
namespace actors
{

    enum class log_level
        : uint16_t
    {
        silent,   ///< No log messages
        error,    ///< Only errors
        warning,  ///< Enable warnings
        info,     ///< Information mesages. The default level
        debug,    ///< Debug messages. Are written only in debug builds
        verbose   ///< Verbose messages
    };


    class logger
    {
    private:
        static constexpr size_t MESSAGE_LENGTH = 2048;

        log_level m_filter;
        std::array<const char*, 6> m_tags;

        logger();
        ~logger();

        void write_message(log_level level, const char* message) noexcept;

        template <typename ... Args_>
        void format_and_write(log_level level, const char* format, Args_ && ... args) {
            if (level <= m_filter) {
                std::array<char, MESSAGE_LENGTH + 2> buffer;
                // Firstly write tag
                int len = std::snprintf(buffer.data(), MESSAGE_LENGTH, "%s", m_tags[static_cast<uint16_t>(level)]);
                assert(len > 0);
                // Write message after the tag
                len += std::snprintf(buffer.data() + len, MESSAGE_LENGTH - len, format, std::forward<Args_>(args)...);
                assert(len > 0);
                // Put newline if necessary
                if(buffer[len - 1] != '\n') {
                    buffer[len]     = '\n';
                    buffer[len + 1] = '\0'; // +1 is valid since buffer if bigger than MESSAGE_LENGTH
                }
                write_message(level, buffer.data());
            }
        }

    public:

        logger(const logger&) = delete;
        logger(logger&&) = delete;

        logger& operator=(const logger&) = delete;
        logger& operator=(logger&&) = delete;

        static
        logger & instance();

        void set_filter(log_level level) {
            m_filter = level;
        }

        template <typename ... Args_>
        void error(const char* format, Args_ && ... args) {
            format_and_write(log_level::error, std::forward<Args_>(args)...);
        }

        void error(const std::string & str) {
            format_and_write(log_level::error, str.c_str());
        }

        // ToDo (a.gruzdev): to be added
        //void error(const std::string & str, const std::exception & exception)

        template <typename ... Args_>
        void warning(const char* format, Args_ && ... args) {
            format_and_write(log_level::warning, std::forward<Args_>(args)...);
        }

        void warning(const std::string & str) {
            format_and_write(log_level::warning, str.c_str());
        }

        template <typename ... Args_>
        void info(const char* format, Args_ && ... args) {
            format_and_write(log_level::info, std::forward<Args_>(args)...);
        }

        void info(const std::string & str) {
            format_and_write(log_level::info, str.c_str());
        }

#if YATO_DEBUG
        template <typename ... Args_>
        void debug(const char* format, Args_ && ... args) {
            format_and_write(log_level::debug, std::forward<Args_>(args)...);
        }

        void debug(const std::string & str) {
            format_and_write(log_level::debug, str.c_str());
        }
#else
        template <typename ... Args_>
        void debug(Args_ && ...) { }
#endif

        template <typename ... Args_>
        void verbose(const char* format, Args_ && ... args) {
            format_and_write(log_level::verbose, std::forward<Args_>(args)...);
        }

        void verbose(const std::string & str) {
            format_and_write(log_level::verbose, str.c_str());
        }

    };

}// namespace actors

}// namespace yato

#endif //_YATO_ABSTRACT_EXECUTOR_H_

