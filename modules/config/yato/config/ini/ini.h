/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_INI_INI_H_
#define _YATO_CONFIG_INI_INI_H_

#include <memory>
#include <string>

#include "yato/config/config.h"

namespace yato {

namespace conf {

    struct ini_builder_state;

    class ini_builder
    {
    public:
        ini_builder();

        ~ini_builder();

        ini_builder(const ini_builder &) = delete;
        ini_builder(ini_builder&&) noexcept = default;

        ini_builder& operator=(const ini_builder&) = delete;
        ini_builder& operator=(ini_builder&&) noexcept = default;


        /**
         * Parse ini file
         */
        config parse_file(const char* filename);

        /**
         * Parse ini file 
         */
        config parse_file(const std::string & filename);

        /**
         * Parse from string
         */
        config parse(const char* ini);

        /**
         * Parse from string
         */
        config parse(const std::string & ini);

    private:
        config finalize_();

        std::unique_ptr<ini_builder_state> m_impl;
    };

} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_INI_INI_H_
