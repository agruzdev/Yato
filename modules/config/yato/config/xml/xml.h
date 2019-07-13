/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_XML_XML_H_
#define _YATO_CONFIG_XML_XML_H_

#include "../config.h"

namespace yato {

namespace conf {

    /**
     * Builder for XML config
     */
    class xml_builder
    {
    public:
        xml_builder();
        ~xml_builder();

        xml_builder(const xml_builder&) = delete;
        xml_builder(xml_builder&&) noexcept;

        xml_builder& operator=(const xml_builder&) = delete;
        xml_builder& operator=(xml_builder&&) noexcept;

        /**
         * Parse XML from string
         */
        config parse(const char* xml) const;

        /**
         * Parse XML from string
         */
        config parse(const std::string & xml) const;

        /**
         * Load an XML file
         */
        config load(const char* filename) const;

        /**
         * Load an XML file
         */
        config load(const std::string & filename) const;
    };

} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_XML_XML_H_
