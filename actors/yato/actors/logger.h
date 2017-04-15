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
#include <memory>

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

        std::string m_name;
        log_level m_filter;
        std::array<const char*, 6> m_tags;

        logger(const std::string & name);
        ~logger();

        void write_message(log_level level, const char* message) const noexcept;

        template <typename ... Args_>
        void format_and_write(log_level level, const char* format, Args_ && ... args) const {
            if (level <= m_filter) {
                std::array<char, MESSAGE_LENGTH + 2> buffer;

                // Firstly write tag
                int len = std::snprintf(buffer.data(), MESSAGE_LENGTH, "%s", m_tags[static_cast<uint16_t>(level)]);
                assert(len > 0);

                // Write logger name
                len += std::snprintf(buffer.data() + len, MESSAGE_LENGTH - len, "%s - ", m_name.c_str());
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

        void set_filter(log_level level) {
            m_filter = level;
        }

        template <typename ... Args_>
        void error(const char* format, Args_ && ... args) const {
            format_and_write(log_level::error, format, std::forward<Args_>(args)...);
        }

        void error(const std::string & str) const {
            format_and_write(log_level::error, "%s", str.c_str());
        }

        // ToDo (a.gruzdev): to be added
        //void error(const std::string & str, const std::exception & exception)

        template <typename ... Args_>
        void warning(const char* format, Args_ && ... args) const {
            format_and_write(log_level::warning, format, std::forward<Args_>(args)...);
        }

        void warning(const std::string & str) const {
            format_and_write(log_level::warning, "%s", str.c_str());
        }

        template <typename ... Args_>
        void info(const char* format, Args_ && ... args) const {
            format_and_write(log_level::info, format, std::forward<Args_>(args)...);
        }

        void info(const std::string & str) const {
            format_and_write(log_level::info, "%s", str.c_str());
        }

#if YATO_DEBUG
        template <typename ... Args_>
        void debug(const char* format, Args_ && ... args) const {
            format_and_write(log_level::debug, format, std::forward<Args_>(args)...);
        }

        void debug(const std::string & str) const {
            format_and_write(log_level::debug, "%s", str.c_str());
        }
#else
        template <typename ... Args_>
        void debug(Args_ && ...) const { }
#endif

        template <typename ... Args_>
        void verbose(const char* format, Args_ && ... args) const {
            format_and_write(log_level::verbose, format, std::forward<Args_>(args)...);
        }

        void verbose(const std::string & str) const {
            format_and_write(log_level::verbose, "%s", str.c_str());
        }

        friend class logger_factory;
    };
    //-------------------------------------------------------


    class logger_deleter
    {
    public:
        void operator()(logger* ptr) const noexcept;
    };
    //-------------------------------------------------------


    using logger_ptr = std::unique_ptr<logger, logger_deleter>;

    class logger_factory
    {
        static
        void destroy(logger* ptr) noexcept;

    public:
        static
        logger_ptr create(const std::string & name);

        friend class logger_deleter;
    };
    //-------------------------------------------------------

}// namespace actors

}// namespace yato

#endif //_YATO_ABSTRACT_EXECUTOR_H_

