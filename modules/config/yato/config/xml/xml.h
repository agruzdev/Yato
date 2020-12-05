/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_XML_XML_H_
#define _YATO_CONFIG_XML_XML_H_

#include <istream>
#include <ostream>
#include <string>
#include "../config.h"

namespace yato {

namespace conf {

    namespace xml {

        /**
         * Parse Xml from string
         */
        config read(const char* str, size_t len = yato::nolength);

        /**
         * Parse Xml from string
         */
        config read(const std::string& str);

        /**
         * Parse Xml from stream
         */
        config read(std::istream& is);

        /**
         * Writes config as Xml to a string
         */
        std::string write(const yato::config& c, bool text_value = true, const std::string& root_name = "root", bool indent = true);

        /**
         * Writes config as Xml to a stream
         */
        void write(const yato::config& c, std::ostream& os, bool text_value = true, const std::string& root_name = "root", bool indent = true);

    } // namespace xml


} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_XML_XML_H_
