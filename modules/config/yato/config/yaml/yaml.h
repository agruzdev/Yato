/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_YAML_YAML_H_
#define _YATO_CONFIG_YAML_YAML_H_

#include <iostream>
#include "../config.h"

namespace yato {

namespace conf {

    /**
     * Builder for YAML config
     */
    class yaml_builder
    {
    public:
        yaml_builder();
        ~yaml_builder();

        yaml_builder(const yaml_builder&) = delete;
        yaml_builder(yaml_builder&&) noexcept;

        yaml_builder& operator = (const yaml_builder&) = delete;
        yaml_builder& operator = (yaml_builder&&) noexcept;

        /**
         * Parse YAML from string
         */
        config parse(const char* input) const;

        /**
         * Parse YAML from string
         */
        config parse(const std::string & input) const;

        /**
         * Parse YAML from stream
         */
        config parse(std::istream & input) const;
    };

} // namespace conf

} // namespace yato


#endif //_YATO_CONFIG_YAML_YAML_H_
