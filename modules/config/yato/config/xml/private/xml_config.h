/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_XML_PRIVATE_XML_CONFIG_H_
#define _YATO_CONFIG_XML_PRIVATE_XML_CONFIG_H_

#include <memory>
#include <tinyxml2.h>

#include "../../config_backend.h"

namespace yato {

namespace conf {

    class xml_value;

    class xml_config final
        : public config_backend
    {
    public:
        xml_config(std::shared_ptr<tinyxml2::XMLDocument> document, const tinyxml2::XMLElement* element = nullptr);

        xml_config(const xml_config&) = delete;

        xml_config(xml_config&&) = delete;

        ~xml_config();

        xml_config& operator=(const xml_config&) = delete;

        xml_config& operator=(xml_config&&) = delete;

    private:
        size_t do_size() const noexcept override;

        bool do_has_property(config_property p) const noexcept override;

        key_value_t do_find(size_t index) const noexcept override;

        key_value_t do_find(const std::string & name) const noexcept override;

        void do_release(const config_value* /*val*/) const noexcept override;


        std::shared_ptr<tinyxml2::XMLDocument> m_document;
        const tinyxml2::XMLElement* m_element;
        std::vector<const tinyxml2::XMLElement*> m_children;
    };

} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_XML_PRIVATE_XML_CONFIG_H_

