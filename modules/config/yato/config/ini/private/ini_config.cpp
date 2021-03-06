/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "ini_config.h"

#include <memory>

#include "ini_value.h"
#include "ini_section.h"

namespace yato {

namespace conf {

    ini_config::ini_config(std::shared_ptr<ini_parser> parser)
        : m_parser(std::move(parser)), m_is_global(true)
    {
        m_values = &m_parser->global_values;
    }

    ini_config::ini_config(std::shared_ptr<ini_parser> parser, const ini_parser::kv_multimap* section_values)
        : m_parser(std::move(parser)), m_values(section_values), m_is_global(false)
    {
        YATO_REQUIRES(section_values != nullptr);
    }

    ini_config::~ini_config() = default;

    size_t ini_config::do_size() const noexcept
    {
        if (m_is_global) {
            return m_parser->sections.size() + m_values->size();
        }
        else {
            return m_values->size();
        }
    }

    std::vector<std::string> ini_config::do_enumerate_keys() const
    {
        std::vector<std::string> keys;
        if (m_is_global) {
            for (auto it = m_parser->sections.cbegin(); it != m_parser->sections.cend(); ++it) {
                keys.push_back((*it).first);
            }
        }
        if (m_values) {
            for (auto it = m_values->cbegin(); it != m_values->cend(); ++it) {
                keys.push_back((*it).first);
            }
        }
        return keys;
    }

    config_backend::find_index_result_t ini_config::do_find(size_t index) const
    {
        config_backend::find_index_result_t result = config_backend::no_index_result;
        const size_t sections_count = m_is_global ? m_parser->sections.size() : 0;
        if (index < sections_count) {
            const auto it = std::next(m_parser->sections.cbegin(), index);
            result = std::make_tuple(
                (*it).first,
                new ini_section(std::make_shared<ini_config>(m_parser, &(*it).second)));
        }
        else if (index < sections_count + m_values->size()) {
            const auto it = std::next(m_values->cbegin(), index - sections_count);
            result = std::make_tuple(
                (*it).first,
                new ini_value((*it).second));
        }
        return result;
    }

    config_backend::find_key_result_t ini_config::do_find(const std::string& name) const
    {
        config_backend::find_key_result_t result = config_backend::no_key_result;
        if (m_is_global) {
            const auto sit = m_parser->sections.find(name);
            if (sit != m_parser->sections.cend()) {
                return std::make_tuple(
                    yato::narrow_cast<size_t>(std::distance(m_parser->sections.begin(), sit)),
                    new ini_section(std::make_shared<ini_config>(m_parser, &(*sit).second)));
            }
        }
        if (m_values) {
            const auto vit = m_values->find(name);
            if (vit != m_values->cend()) {
                const size_t sections_count = m_is_global ? m_parser->sections.size() : 0;
                result = std::make_tuple(
                    sections_count + yato::narrow_cast<size_t>(std::distance(m_values->begin(), vit)),
                    new ini_value((*vit).second)
                );
            }
        }
        return result;
    }

    void ini_config::do_release(const config_value* val) const noexcept
    {
        delete val;
    }

    bool ini_config::do_has_property(config_property p) const noexcept
    {
        switch (p) {
        case config_property::associative:
        case config_property::multi_associative:
            return true;
        default:
            return false;
        }
    }

} // namespace conf

} // namespace yato

