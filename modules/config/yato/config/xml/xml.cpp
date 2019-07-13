/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "xml.h"
#include "private/xml_config.h"

namespace yato {

namespace conf {

    xml_builder::xml_builder() = default;

    xml_builder::~xml_builder() = default;

    xml_builder::xml_builder(xml_builder&&) noexcept = default;

    xml_builder& xml_builder::operator=(xml_builder&&) noexcept = default;

    static
    config build_xml_config(const std::shared_ptr<tinyxml2::XMLDocument> & document)
    {
        YATO_REQUIRES(document != nullptr);
        if (document->Error()) {
            throw yato::conf::config_error("xml_builder[build_xml_config]: XML Error: " + std::string(document->ErrorStr()));
        }
        return config(std::make_shared<xml_config>(document));
    }

    config xml_builder::parse(const char* xml) const
    {
        if (xml == nullptr) {
            throw yato::argument_error("xml_builder[parse]: Null string pointer is passed!");
        }
        auto document = std::make_shared<tinyxml2::XMLDocument>();
        document->Parse(xml);
        return build_xml_config(document);
    }

    config xml_builder::parse(const std::string & xml) const
    {
        if (!xml.empty()) {
            auto document = std::make_shared<tinyxml2::XMLDocument>();
            document->Parse(xml.data(), xml.size());
            return build_xml_config(document);
        }
        return config();
    }

    config xml_builder::load(const char* filename) const
    {
        if (filename == nullptr) {
            throw yato::argument_error("xml_builder[parse]: Null string pointer is passed!");
        }
        auto document = std::make_shared<tinyxml2::XMLDocument>();
        document->LoadFile(filename);
        return build_xml_config(document);
    }

    config xml_builder::load(const std::string & filename) const
    {
        if (!filename.empty()) {
            auto document = std::make_shared<tinyxml2::XMLDocument>();
            document->LoadFile(filename.c_str());
            return build_xml_config(document);
        }
        return config();
    }

} // namespace conf

} // namespace yato
