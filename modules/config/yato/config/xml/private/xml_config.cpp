/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "xml_config.h"
#include "xml_value.h"

namespace yato {

namespace conf {

    namespace
    {
        template <typename Visitor_>
        static
        void visit_children_(const tinyxml2::XMLElement* elem, Visitor_ && visitor)
        {
            if(elem != nullptr) {
                auto first       = elem->FirstChildElement();
                const auto last  = elem->LastChildElement();
                if(first != nullptr && last != nullptr) {
                    while (first != last) {
                        visitor(first);
                        first = first->NextSiblingElement();
                        if (!first) {
                            break;
                        }
                    }
                    visitor(last);
                }
            }
        }
    }

    xml_config::xml_config(std::shared_ptr<tinyxml2::XMLDocument> document, const tinyxml2::XMLElement* element)
        : m_document(std::move(document)), m_element(element)
    {
        if (!m_element) {
            m_element = m_document->RootElement();
        }
        if (m_element) {
            visit_children_(m_element, [this](const tinyxml2::XMLElement* elem) {
                m_children.push_back(elem);
            });
        }
    }

    xml_config::~xml_config() = default;

    size_t xml_config::do_size() const noexcept
    {
        return m_children.size();
    }

    bool xml_config::do_has_property(config_property p) const noexcept
    {
        switch (p) {
        case config_property::associative:
        case config_property::multi_associative:
        case config_property::keeps_order:
            return true;
        default:
            return false;
        }
    }

    config_backend::find_index_result_t xml_config::do_find(size_t index) const
    {
        find_index_result_t res = config_backend::no_index_result;
        if (index < m_children.size()) {
            const auto it = std::next(m_children.cbegin(), index);
            res = std::make_tuple((*it)->Name(), new xml_value(m_document, *it));
        }
        return res;
    }

    config_backend::find_key_result_t xml_config::do_find(const std::string& name) const
    {
        size_t index = 0;
        for (auto child = m_element->FirstChildElement(); child; child = child->NextSiblingElement()) {
            if (child->Name() == name) {
                return std::make_tuple(index, new xml_value(m_document, child));
            }
            ++index;
        }
        return config_backend::no_key_result;
    }

    void xml_config::do_release(const config_value* val) const noexcept
    {
        delete val;
    }



} // namespace conf

} // namespace yato

