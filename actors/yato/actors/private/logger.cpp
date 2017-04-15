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

    logger::logger(const std::string & name)
        : m_name(name) 
    {
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
    //-------------------------------------------------------

    logger::~logger() {
    }
    //-------------------------------------------------------

    void logger::write_message(log_level level, const char* message) const noexcept {
        YATO_MAYBE_UNUSED(level);
        // Use std::cout providing threadsafe writing to console
        std::cout << message;
    }
    //-------------------------------------------------------

    void logger_deleter::operator()(logger* ptr) const noexcept {
        logger_factory::destroy(ptr);
    }
    //-------------------------------------------------------

    void logger_factory::destroy(logger* ptr) noexcept {
        delete ptr;
    }
    //-------------------------------------------------------

    logger_ptr logger_factory::create(const std::string& name) {
        return logger_ptr(new logger{ name }, logger_deleter{});
    }
    //-------------------------------------------------------

}// namespace actors

}// namespace yato

