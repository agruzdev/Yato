/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <iostream>

#include <yato/prerequisites.h>

#include "../logger.h"

namespace yato
{
namespace actors
{

    logger::logger() {
#if YATO_DEBUG
        m_filter = log_level::debug;
#else
        m_filter = log_level::info;
#endif
        m_tags[static_cast<uint16_t>(log_level::silent)]  = nullptr;
        m_tags[static_cast<uint16_t>(log_level::error)]   = "[ERROR]   ";
        m_tags[static_cast<uint16_t>(log_level::warning)] = "[WARNING] ";
        m_tags[static_cast<uint16_t>(log_level::info)]    = "[INFO]    ";
        m_tags[static_cast<uint16_t>(log_level::debug)]   = "[DEBUG]   ";
        m_tags[static_cast<uint16_t>(log_level::verbose)] = "[VERBOSE] ";
    }

    logger::~logger() {
    }

    logger & logger::instance() {
        static logger instance;
        return instance;
    }

    void logger::write_message(log_level level, const char* message) noexcept {
        YATO_MAYBE_UNUSED(level);
        // Use std::cout providing threadsafe writing to console
        std::cout << message;
    }

}// namespace actors

}// namespace yato

