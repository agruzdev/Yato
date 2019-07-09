/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "ini_config.h"

#include <memory>

#include "ini_value.h"
#include "ini_section.h"

namespace yato {

namespace conf {

    ini_config::ini_config(std::shared_ptr<CSimpleIniA> reader)
        : m_reader(std::move(reader)), m_section(nullptr)
    {
        // represent global section
        std::string root_name = "";
        for (const char* global_name : { "", "GLOBAL", "global" }) {
            m_section = m_reader->GetSection(global_name);
            if (m_section) {
                root_name = global_name;
                break;
            }
        }

        std::list<CSimpleIni::Entry> sections;
        m_reader->GetAllSections(sections);
        for (const auto & e : sections) {
            if (e.pItem == root_name) {
                continue;
            }
            ini_section_ptr nested_section = m_reader->GetSection(e.pItem);
            m_nested_sections.emplace(std::string(e.pItem), nested_section);
        }
    }

    ini_config::ini_config(std::shared_ptr<CSimpleIniA> reader, ini_section_ptr section)
        : m_reader(std::move(reader)), m_section(section)
    {
        YATO_REQUIRES(section != nullptr);
    }

    ini_config::~ini_config() = default;

    size_t ini_config::do_size() const noexcept
    {
        size_t count = m_nested_sections.size();
        if (m_section) {
            count += m_section->size();
        }
        return count;
    }

    config_backend::key_value_t ini_config::do_find(size_t index) const noexcept
    {
        std::string key;
        std::unique_ptr<config_value> value;
        const size_t section_number = m_nested_sections.size();
        if (index < section_number) {
            const auto it = std::next(m_nested_sections.cbegin(), index);
            key   = (*it).first;
            value = std::make_unique<ini_section>(std::make_shared<ini_config>(m_reader, (*it).second));
        }
        else if (m_section && (index - section_number < m_section->size())) {
            const auto it = std::next(m_section->cbegin(), index - section_number);
            key   = (*it).first.pItem;
            value = std::make_unique<ini_value>((*it).second);
        }
        return std::make_pair(key, value.release());
    }

    config_backend::key_value_t ini_config::do_find(const std::string & name) const noexcept
    {
        std::unique_ptr<config_value> value;
        const auto sit = m_nested_sections.find(name);
        if (sit != m_nested_sections.cend()) {
            value = std::make_unique<ini_section>(std::make_shared<ini_config>(m_reader, (*sit).second));
        }
        else if(m_section) {
            const auto vit = m_section->find(name.c_str());
            if(vit != m_section->cend()) {
                value = std::make_unique<ini_value>((*vit).second);
            }
        }
        return std::make_pair(name, value.release());
    }

    void ini_config::do_release(const config_value* val) const noexcept
    {
        delete val;
    }

    bool ini_config::do_is_object() const noexcept
    {
        return true;
    }

} // namespace conf

} // namespace yato

