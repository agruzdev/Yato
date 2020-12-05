/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "xml.h"
#include "private/xml_config.h"
#include "yato/assertion.h"
#include "yato/config/utility.h"

namespace yato {

namespace conf {

    namespace xml {

        inline
        config build_xml_config(const std::shared_ptr<tinyxml2::XMLDocument>& document)
        {
            YATO_REQUIRES(document != nullptr);
            if (document->Error()) {
                throw yato::conf::config_error("xml_builder[build_xml_config]: XML Error: " + std::string(document->ErrorStr()));
            }
            return config(std::make_shared<xml_config>(document));
        }

        config read(const char* str, size_t len)
        {
            if (str == nullptr) {
                throw yato::argument_error("xml[read]: Null string pointer is passed!");
            }
            auto document = std::make_shared<tinyxml2::XMLDocument>();
            if (len != yato::nolength) {
                document->Parse(str, len);
            }
            else {
                document->Parse(str);
            }
            return build_xml_config(document);
        }

        config read(const std::string& str)
        {
            return read(str.c_str(), str.size());
        }

        config read(std::istream& is)
        {
            return read(get_text_stream_content(is));
        }

        const char* to_xml_key_(bool is_associative, const std::string& key)
        {
            static const char* kXmlArrayKey = "i";
            return is_associative ? key.c_str() : kXmlArrayKey;
        }

        template <typename Ty_>
        void append_xml_scalar_(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* elem, const char* key, Ty_&& val, bool text_value)
        {
            YATO_REQUIRES(elem != nullptr);
            auto scalar = doc.NewElement(key);
            if (text_value) {
                scalar->SetText(std::forward<Ty_>(val));
            }
            else {
                scalar->SetAttribute("value", std::forward<Ty_>(val));
            }
            elem->InsertEndChild(scalar);
        }

        static
        void fill_xml_element_(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* elem, const yato::config& c, bool text_value)
        {
            if (c) {
                const bool is_associative = c.is_associative();
                for (auto entry : c) {
                    if (entry) {
                        switch (entry.type()) {
                        case stored_type::integer:
                            append_xml_scalar_(doc, elem, to_xml_key_(is_associative, entry.key()), entry.value<int64_t>().get(), text_value);
                            break;
                        case stored_type::real:
                            append_xml_scalar_(doc, elem, to_xml_key_(is_associative, entry.key()), entry.value<double>().get(), text_value);
                            break;
                        case stored_type::boolean:
                            append_xml_scalar_(doc, elem, to_xml_key_(is_associative, entry.key()), entry.value<bool>().get(), text_value);
                            break;
                        case stored_type::string:
                            append_xml_scalar_(doc, elem, to_xml_key_(is_associative, entry.key()), entry.value<std::string>().get().c_str(), text_value);
                            break;
                        case stored_type::config:
                            auto child_elem = doc.NewElement(to_xml_key_(is_associative, entry.key()));
                            fill_xml_element_(doc, child_elem, entry.object(), text_value);
                            elem->InsertEndChild(child_elem);
                            break;
                        }
                    }
                    else {
                        auto child_elem = doc.NewElement(to_xml_key_(is_associative, entry.key()));
                        elem->InsertEndChild(child_elem);
                    }
                }
            }
        }

        std::string write(const yato::config& c, bool text_value, const std::string& root_name, bool indent)
        {
            auto doc = std::make_unique<tinyxml2::XMLDocument>();
            auto root = doc->NewElement(root_name.c_str());
            fill_xml_element_(*doc, root, c, text_value);
            doc->InsertEndChild(root);
            tinyxml2::XMLPrinter printer(nullptr, !indent);
            doc->Print(&printer);
            return std::string(printer.CStr(), std::max(0, printer.CStrSize() - 1));
        }

        void write(const yato::config& c, std::ostream& os, bool text_value, const std::string& root_name, bool indent)
        {
            os << write(c, text_value, root_name, indent);
        }

    } // namespace xml


} // namespace conf

} // namespace yato
