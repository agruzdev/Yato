/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "xml_value.h"
#include "xml_config.h"

namespace yato {

namespace conf {

    xml_value::xml_value(std::shared_ptr<tinyxml2::XMLDocument> document, const tinyxml2::XMLElement* element)
        : m_document(std::move(document)), m_element(element)
    {
        if (m_element) {
            const auto first = m_element->FirstChildElement();
            const auto last  = m_element->LastChildElement();
            const bool is_leaf =  !first || !last;
            if (is_leaf) {
                const tinyxml2::XMLAttribute* attribute = m_element->FindAttribute("value");
                if (attribute) {
                    m_text = attribute->Value();
                }
                if (!m_text) {
                    m_text = m_element->GetText();
                }
            }
        }
    }

    xml_value::~xml_value() = default;

    stored_type xml_value::type() const noexcept
    {
        return m_text ? stored_type::string : stored_type::config;
    }

    stored_variant xml_value::get() const noexcept
    {
        stored_variant res;
        if (m_text) {
            const tinyxml2::XMLAttribute* attribute = m_element->FindAttribute("value");
            if (attribute) {
                const char* attr_text = attribute->Value();
                if (attr_text) {
                    res.emplace<std::string>(attr_text);
                }
            }
            if (res.is_type<void>()) {
                const auto text = m_element->GetText();
                if (text) {
                    res.emplace<std::string>(text);
                }
            }
        }
        else if(m_element) {
            res.emplace<backend_ptr>(std::make_shared<xml_config>(m_document, m_element));
        }
        return res;
    }

} // namespace conf

} // namespace yato

