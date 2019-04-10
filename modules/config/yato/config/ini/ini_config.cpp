/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include <fstream>
#include <iterator>

#include "yato/prerequisites.h"
#include "yato/config/utility.h"
#include "ini_config.h"

#define SI_SUPPORT_IOSTREAMS 1
#define SI_CONVERT_GENERIC 1
#include "SimpleIni.h"

namespace yato {

namespace conf {

    using ini_section_ptr = decltype(std::declval<CSimpleIniA>().GetSection(std::declval<const char*>()));

    struct ini_config_state
    {
        std::shared_ptr<CSimpleIniA> reader;
        ini_section_ptr root_section = nullptr;
        std::list<CSimpleIni::Entry> nested_sections{};
    };

    //-------------------------------------------------------------------------


    ini_config::ini_config(std::unique_ptr<ini_config_state> && impl)
        : m_impl(std::move(impl))
    { }

    ini_config::~ini_config() = default;

    ini_config::ini_config(ini_config&&) noexcept = default;

    ini_config& ini_config::operator=(ini_config&&) noexcept = default;

    bool ini_config::is_object() const noexcept
    {
        return true;
    }

    stored_variant ini_config::decode_value_(const char* raw_value, stored_type type) const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        stored_variant res{};
        if (raw_value) {
            switch (type)
            {
            case yato::conf::stored_type::integer: {
                    using return_type = stored_type_trait<stored_type::integer>::return_type;
                    auto decoded_value = static_cast<return_type>(0);
                    if (serializer<stored_type::integer>::from_string(raw_value, conf::nolength, &decoded_value)){
                        res.emplace<return_type>(decoded_value);
                    }
                }
                break;
            case yato::conf::stored_type::boolean: {
                    using return_type = stored_type_trait<stored_type::boolean>::return_type;
                    auto decoded_value = static_cast<return_type>(false);
                    if (serializer<stored_type::boolean>::from_string(raw_value, conf::nolength, &decoded_value)){
                        res.emplace<return_type>(decoded_value);
                    }
                }
                break;
            case yato::conf::stored_type::real: {
                    using return_type = stored_type_trait<stored_type::real>::return_type;
                    auto decoded_value = static_cast<return_type>(0);
                    if (serializer<stored_type::real>::from_string(raw_value, conf::nolength, &decoded_value)){
                        res.emplace<return_type>(decoded_value);
                    }
                }
                break;
            case yato::conf::stored_type::string: {
                    using return_type = stored_type_trait<stored_type::string>::return_type;
                    res.emplace<return_type>(static_cast<return_type>(raw_value));
                }
                break;
            default:
                break;
            }
        }
        return res;
    }

    static
    stored_variant make_subconfig(ini_config_state & self, ini_section_ptr section)
    {
        YATO_REQUIRES(section != nullptr);
        using return_type = stored_type_trait<stored_type::config>::return_type;
        auto child = std::make_unique<ini_config_state>();
        child->reader  = self.reader;
        child->root_section = section;
        return stored_variant(yato::in_place_type_t<return_type>{}, std::make_unique<ini_config>(std::move(child)));
    }

    stored_variant ini_config::get_by_key(const std::string & name, stored_type type) const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        stored_variant res{};
        if (type == stored_type::config) {
            const ini_section_ptr subsection_ptr = m_impl->reader->GetSection(name.c_str());
            if (subsection_ptr && (subsection_ptr != m_impl->root_section)) {
                res = make_subconfig(*m_impl, subsection_ptr);
            }
        }
        else if (m_impl->root_section) {
            const auto value_it = m_impl->root_section->find(name.c_str());
            if (value_it != m_impl->root_section->cend()) {
                res = decode_value_(value_it->second, type);
            }
        }
        return res;
    }

    stored_variant ini_config::get_by_index(size_t index, stored_type type) const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        stored_variant res{};
        const size_t sections_num = m_impl->nested_sections.size();
        if (index < sections_num) {
            if (type == stored_type::config) {
                const ini_section_ptr subsection_ptr = m_impl->reader->GetSection(std::next(m_impl->nested_sections.cbegin(), index)->pItem);
                if (subsection_ptr) {
                    res = make_subconfig(*m_impl, subsection_ptr);
                }
            }
        }
        else if(m_impl->root_section) {
            const size_t value_index = index - sections_num;
            if (value_index < m_impl->root_section->size()) {
                res = decode_value_(std::next(m_impl->root_section->cbegin(), value_index)->second, type);
            }
        }
        return res;
    }

    size_t ini_config::size() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        size_t s = m_impl->nested_sections.size();
        if(m_impl->root_section) {
            s += m_impl->root_section->size();
        }
        return s;
    }

    std::vector<std::string> ini_config::keys() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        std::vector<std::string> keys;
        keys.reserve(size());
        for (const auto & sect : m_impl->nested_sections) {
            keys.emplace_back(sect.pItem);
        }
        if (m_impl->root_section) {
            for (const auto & entry : *(m_impl->root_section)) {
                keys.emplace_back(entry.first.pItem);
            }
        }
        keys.erase(std::unique(keys.begin(), keys.end()), keys.cend());
        return keys;
    }


    //-----------------------------------------------------------------------

    ini_builder::ini_builder()
    {
        m_impl = std::make_unique<ini_config_state>();
        m_impl->reader = std::make_shared<CSimpleIniA>();
        m_impl->reader->SetMultiLine(true);
        m_impl->reader->SetMultiKey(false);
        m_impl->reader->SetUnicode(false);
    }

    ini_builder::~ini_builder() = default;

    ini_builder& ini_builder::multiline(bool enable)
    {
        m_impl->reader->SetMultiLine(enable);
        return *this;
    }

    config ini_builder::parse(std::istream & is)
    {
        m_impl->reader->LoadData(is);
        return finalize();
    }

    config ini_builder::parse(const char* ini)
    {
        m_impl->reader->LoadData(ini, std::strlen(ini));
        return finalize();
    }

    config ini_builder::parse(const std::string& ini)
    {
        m_impl->reader->LoadData(ini.c_str(), ini.size());
        return finalize();
    }

    config ini_builder::parse_file(const char* filename)
    {
        std::ifstream file(filename, std::ios::binary);
        return parse(file);
    }

    config ini_builder::parse_file(const std::string & filename)
    {
        std::ifstream file(filename, std::ios::binary);
        return parse(file);
    }

    config ini_builder::finalize()
    {
        std::string root_name{};
        for (const char* global_name : { "", "GLOBAL", "global" }) {
            m_impl->root_section = m_impl->reader->GetSection(global_name);
            if (m_impl->root_section) {
                root_name = global_name;
                break;
            }
        }
        m_impl->reader->GetAllSections(m_impl->nested_sections);
        if (m_impl->root_section) {
            const auto root_it = std::find_if(m_impl->nested_sections.cbegin(), m_impl->nested_sections.cend(), [&root_name](const CSimpleIniA::Entry & e) {
                return root_name == e.pItem;
            });
            if (root_it != m_impl->nested_sections.cend()) {
                m_impl->nested_sections.erase(root_it);
            }
        }
        return config(std::make_shared<ini_config>(std::move(m_impl)));
    }

} // namespace conf

} //namespace yato
