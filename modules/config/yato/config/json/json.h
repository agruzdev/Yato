/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_JSON_JSON_H_
#define _YATO_CONFIG_JSON_JSON_H_

#include <istream>
#include "../config.h"

namespace yato {

namespace conf {


    namespace json
    {
        /**
         * Parse Json from string
         */
        config read(const char* str, size_t len = yato::nolength, bool ordered = false);

        /**
         * Parse Json from string
         */
        config read(const std::string& str, bool ordered = false);

        /**
         * Parse Json from stream
         */
        config read(std::istream& is, bool ordered = false);

        /**
         * Writes config as Json to a string
         * if 'ordered' is not passed, then deduced from confid properties
         */
        std::string write(const yato::config& c, uint32_t indent = 0, yato::optional<bool> ordered = {});

        /**
         * Writes config as Json to a stream
         * if 'ordered' is not passed, then deduced from confid properties
         */
        void write(const yato::config& c, std::ostream& os, uint32_t indent = 0, yato::optional<bool> ordered = {});

    } // namespace json



} //namespace conf

} //namespace yato

#endif // _YATO_CONFIG_JSON_JSON_H_
