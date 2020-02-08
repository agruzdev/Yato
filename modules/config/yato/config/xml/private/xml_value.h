/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_XML_PRIVATE_XML_VALUE_H_
#define _YATO_CONFIG_XML_PRIVATE_XML_VALUE_H_

#include <memory>
#include <tinyxml2.h>

#include "../../config_backend.h"

namespace yato {

namespace conf {

    class xml_value final
        : public config_value
    {
    public:
        xml_value(std::shared_ptr<tinyxml2::XMLDocument> document, const tinyxml2::XMLElement* element);

        xml_value(const xml_value&) = delete;
        xml_value(xml_value&&) = delete;

        xml_value& operator=(const xml_value&) = delete;
        xml_value& operator=(xml_value&&) = delete;

        ~xml_value();

        stored_type type() const noexcept override;

        stored_variant get() const noexcept override;

    private:
        std::shared_ptr<tinyxml2::XMLDocument> m_document;
        const tinyxml2::XMLElement* m_element;
        const char* m_text = nullptr;
    };

} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_XML_PRIVATE_XML_VALUE_H_
