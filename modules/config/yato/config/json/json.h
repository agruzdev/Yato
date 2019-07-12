/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_JSON_JSON_H_
#define _YATO_CONFIG_JSON_JSON_H_

#include "../config.h"

namespace yato {

namespace conf {

    /**
     * Builder for Json config
     */
    class json_builder
    {
    public:
        json_builder();
        ~json_builder();

        json_builder(const json_builder&) = delete;
        json_builder(json_builder&&) noexcept;

        json_builder& operator = (const json_builder&) = delete;
        json_builder& operator = (json_builder&&) noexcept;

        /**
         * Parse Json from string
         */
        config parse(const char* json) const;

        /**
         * Parse Json from string
         */
        config parse(const std::string & json) const;
    };

} //namespace conf

} //namespace yato

#endif // _YATO_CONFIG_JSON_JSON_H_
