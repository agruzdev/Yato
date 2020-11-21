/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_YAML_YAML_H_
#define _YATO_CONFIG_YAML_YAML_H_

#include <iostream>
#include "../config.h"

namespace yato {

namespace conf {

    namespace yaml {

        /**
         * Parse Yaml from string
         */
        config read(const char* str, size_t len = yato::nolength);

        /**
         * Parse Yaml from string
         */
        config read(const std::string& str);

        /**
         * Parse Yaml from stream
         */
        config read(std::istream& is);

        /**
         * Writes config as Yaml to a string
         */
        std::string write(const yato::config& c, uint32_t indent = 0);

        /**
         * Writes config as Yaml to a stream
         */
        void write(const yato::config& c, std::ostream& os, uint32_t indent = 0);

    } // namespace yaml

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
