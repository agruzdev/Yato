/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#include <memory>

#include <tinyxml2.h>

#include "xml_config.h"

namespace yato {

namespace conf {

    struct xml_config_state
    {
        std::shared_ptr<tinyxml2::XMLDocument> document;
        const tinyxml2::XMLElement* current_element = nullptr;
        mutable std::vector<const tinyxml2::XMLElement*> array_lut;
        mutable bool cache_is_valid = false;
    };

    template <typename Visitor_>
    static
    void visit_childs_(const tinyxml2::XMLElement* elem, Visitor_ && visitor)
    {
        if(elem != nullptr) {
            auto first       = elem->FirstChildElement();
            const auto last  = elem->LastChildElement();
            if(first != nullptr && last != nullptr) {
                while(first != last) {
                    visitor(first);
                    first = first->NextSiblingElement();
                }
                visitor(last);
            }
        }
    }

    template <typename ValueTy_, typename Converter_>
    static
    bool query_value_(const tinyxml2::XMLElement* elem, ValueTy_& out, Converter_ && converter)
    {
        const tinyxml2::XMLAttribute* attribute = elem->FindAttribute("value");
        if(attribute != nullptr) {
            const char* attr_text = attribute->Value();
            if(attr_text != nullptr) {
                return converter(attr_text, &out);
            }
        }
        const auto text = elem->GetText();
        if(text != nullptr) {
            return converter(text, &out);
        }
        return false;
    }

    static
    bool cstring_converter_(const char* str, const char** out)
    {
        YATO_REQUIRES(out != nullptr);
        if(str != nullptr) {
            *out = str;
            return true;
        }
        return false;
    }

    static
    stored_variant get_impl_(const xml_config_state& state, const tinyxml2::XMLElement* elem, config_type type) noexcept
    {
        stored_variant res{};
        if(elem != nullptr) {
            switch (type)
            {
            case config_type::integer: {
                    int64_t val = 0;
                    if (query_value_(elem, val, &tinyxml2::XMLUtil::ToInt64)) {
                        using return_type = stored_type_trait<config_type::integer>::return_type;
                        res.emplace<return_type>(yato::narrow_cast<return_type>(val));
                    }
                }
                break;
            case config_type::floating: {
                    double val = 0.0;
                    if (query_value_(elem, val, &tinyxml2::XMLUtil::ToDouble)) {
                        using return_type = stored_type_trait<config_type::floating>::return_type;
                        res.emplace<return_type>(yato::narrow_cast<return_type>(val));
                    }
                }
                break;
            case config_type::boolean: {
                    bool val = false;
                    if (query_value_(elem, val, &tinyxml2::XMLUtil::ToBool)) {
                        using return_type = stored_type_trait<config_type::boolean>::return_type;
                        res.emplace<return_type>(yato::narrow_cast<return_type>(val));
                    }
                }
                break;
            case config_type::string: {
                    const char* val = nullptr;
                    if (query_value_(elem, val, &cstring_converter_)) {
                        YATO_ENSURES(val != nullptr);
                        using return_type = stored_type_trait<config_type::string>::return_type;
                        res.emplace<return_type>(val);
                    }
                }
                break;
            case config_type::config: {
                    using return_type = stored_type_trait<config_type::config>::return_type;
                    auto child_state = std::make_unique<xml_config_state>();
                    child_state->document = state.document;
                    child_state->current_element = elem;
                    return_type subconfig = std::make_unique<xml_config>(std::move(child_state));
                    res.emplace<return_type>(std::move(subconfig));
                }
                break;
            default:
                break;
            }
        }
        return res;
    }


    //-------------------------------------------------------------------------------------------------------

    xml_config::xml_config(std::unique_ptr<xml_config_state> && impl)
        : m_impl(std::move(impl))
    { }

    xml_config::~xml_config() = default;

    xml_config::xml_config(xml_config&&) noexcept = default;

    xml_config& xml_config::operator=(xml_config&&) noexcept = default;

    void xml_config::init_array_cache() const
    {
        YATO_REQUIRES(m_impl != nullptr && m_impl->document != nullptr && m_impl->current_element != nullptr);
        YATO_REQUIRES(is_array_tag());
        
        if(!m_impl->cache_is_valid) {
            m_impl->array_lut.clear();
            visit_childs_(m_impl->current_element, [this](const tinyxml2::XMLElement* elem) {
                m_impl->array_lut.push_back(elem);
            });
            m_impl->cache_is_valid = true;
        }
    }

    stored_variant xml_config::get_by_name(const std::string & name, config_type type) const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr && m_impl->document != nullptr && m_impl->current_element != nullptr);
        if (!is_array_tag()) {
            const tinyxml2::XMLElement* child = m_impl->current_element->FirstChildElement(name.c_str());
            return get_impl_(*m_impl, child, type);
        }
        return stored_variant{};
    }

    stored_variant xml_config::get_by_index(size_t index, config_type type) const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr && m_impl->document != nullptr && m_impl->current_element != nullptr);
        if (is_array_tag()) {
            init_array_cache();
            if(index < m_impl->array_lut.size()) {
                const tinyxml2::XMLElement* child = m_impl->array_lut[index];
                return get_impl_(*m_impl, child, type);
            }
        }
        return stored_variant{};
    }

    bool xml_config::is_array_tag() const
    {
        YATO_REQUIRES(m_impl != nullptr && m_impl->current_element != nullptr);
        bool val = false;
        if (tinyxml2::XML_SUCCESS == m_impl->current_element->QueryBoolAttribute("is_array", &val)) {
            return val;
        }
        return false;
    }

    bool xml_config::is_object() const noexcept
    {
        return !is_array_tag();
    }

    bool xml_config::is_array() const noexcept
    {
        return is_array_tag();
    }

    std::vector<std::string> xml_config::keys() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr && m_impl->current_element != nullptr);
        std::vector<std::string> res;
        if (!is_array_tag()) {
            visit_childs_(m_impl->current_element, [&res](const tinyxml2::XMLElement* elem) {
                res.emplace_back(elem->Name());
            });
        }
        return res;
    }

    size_t xml_config::size() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        if (is_array_tag()) {
            init_array_cache();
            return m_impl->array_lut.size();
        }
        return 0;
    }

    //---------------------------------------------------------------------------------------------

    xml_builder::xml_builder() = default;

    xml_builder::~xml_builder() = default;

    xml_builder::xml_builder(xml_builder&&) noexcept = default;

    xml_builder& xml_builder::operator=(xml_builder&&) noexcept = default;

    static
    config build_xml_config(std::shared_ptr<tinyxml2::XMLDocument> && document)
    {
        YATO_REQUIRES(document != nullptr);
        if (document->Error()) {
            throw yato::conf::config_error("XML Error: " + std::string(document->ErrorStr()));
        }
        auto state = std::make_unique<xml_config_state>();
        state->document = std::move(document);
        state->current_element = state->document->RootElement();
        if(state->current_element == nullptr) {
            throw yato::conf::config_error("XML Error: No root element");
        }
        return config(std::make_shared<xml_config>(std::move(state)));
    }

    config xml_builder::parse(const char* xml) const
    {
        if(xml == nullptr) {
            throw yato::argument_error("xml_builder[parse]: Null string pointer is passed!");
        }
        auto document = std::make_shared<tinyxml2::XMLDocument>();
        document->Parse(xml);
        return build_xml_config(std::move(document));
    }

    config xml_builder::parse(const std::string & xml) const
    {
        if(!xml.empty()) {
            auto document = std::make_shared<tinyxml2::XMLDocument>();
            document->Parse(xml.data(), xml.size());
            return build_xml_config(std::move(document));
        }
        return config();
    }

    config xml_builder::load(const char* filename) const
    {
        if(filename == nullptr) {
            throw yato::argument_error("xml_builder[parse]: Null string pointer is passed!");
        }
        auto document = std::make_shared<tinyxml2::XMLDocument>();
        document->LoadFile(filename);
        return build_xml_config(std::move(document));
    }

    config xml_builder::load(const std::string & filename) const
    {
        if(!filename.empty()) {
            auto document = std::make_shared<tinyxml2::XMLDocument>();
            document->LoadFile(filename.c_str());
            return build_xml_config(std::move(document));
        }
        return config();
    }

} // namespace conf

} // namespace yato

