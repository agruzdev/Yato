/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_XML_CONFIG_H_
#define _YATO_CONFIG_XML_CONFIG_H_

#include "../config.h"

namespace yato {

namespace conf {
    class json_builder;

    struct xml_config_state;

    class xml_config
        : public config_backend
    {
    private:
        std::unique_ptr<xml_config_state> m_impl;

        size_t size() const noexcept override;

        bool is_object() const noexcept override;

        stored_variant get_by_index(size_t index, stored_type type) const noexcept override;

        stored_variant get_by_key(const std::string & name, stored_type type) const noexcept override;

        std::vector<std::string> keys() const noexcept override;

        bool is_array_tag() const;

    public:
        xml_config(std::unique_ptr<xml_config_state> && impl);
        ~xml_config();

        xml_config(const xml_config&) = delete;
        xml_config(xml_config&&) noexcept;

        xml_config& operator=(const xml_config&) = delete;
        xml_config& operator=(xml_config&&) noexcept;
    };


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

        xml_builder& operator = (const xml_builder&) = delete;
        xml_builder& operator = (xml_builder&&) noexcept;

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

#endif // _YATO_CONFIG_XML_CONFIG_H_
