/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016-2018 Alexey Gruzdev
 */

#include <fstream>
#include <iterator>

#include "ini_config.h"

#define SI_SUPPORT_IOSTREAMS 1
#define SI_CONVERT_GENERIC 1
#include "SimpleIni.h"

namespace yato {

namespace conf {

    static const char GLOBAL_SECTION[] = "";

    struct ini_config_state
    {
        CSimpleIniA reader;
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

    stored_variant ini_config::get_by_name(const std::string & name, config_type type) const noexcept
    {
        assert(m_impl != nullptr);
        stored_variant res{};
        const char* raw_value = m_impl->reader.GetValue(GLOBAL_SECTION, name.c_str(), nullptr);
        if(raw_value != nullptr && raw_value[0] != '\0') {
            switch (type)
            {
            case yato::conf::config_type::integer: {
                    using return_type = stored_type_trait<config_type::integer>::return_type;
                    res.emplace<return_type>(yato::narrow_cast<return_type>(m_impl->reader.GetLongValue(GLOBAL_SECTION, name.c_str())));
                }
                break;
            case yato::conf::config_type::boolean: {
                    using return_type = stored_type_trait<config_type::boolean>::return_type;
                    res.emplace<return_type>(yato::narrow_cast<return_type>(m_impl->reader.GetBoolValue(GLOBAL_SECTION, name.c_str())));
                }
                break;
            case yato::conf::config_type::floating: {
                    using return_type = stored_type_trait<config_type::floating>::return_type;
                    res.emplace<return_type>(static_cast<return_type>(m_impl->reader.GetDoubleValue(GLOBAL_SECTION, name.c_str())));
                }
                break;
            case yato::conf::config_type::string: {
                    using return_type = stored_type_trait<config_type::string>::return_type;
                    res.emplace<return_type>(static_cast<return_type>(raw_value));
                }
                break;
            case yato::conf::config_type::config:
            default:
                break;
            }
        }
        return res;
    }

    bool ini_config::is_array() const noexcept
    {
        return false;
    }

    stored_variant ini_config::get_by_index(size_t /*index*/, config_type /*type*/) const noexcept
    {
        return stored_variant{};
    }

    size_t ini_config::size() const noexcept
    {
        return 0;
    }

    std::vector<std::string> ini_config::keys() const noexcept
    {
        assert(m_impl != nullptr);
        std::vector<std::string> keys;
        std::list<CSimpleIniA::Entry> keys_list;
        if(m_impl->reader.GetAllKeys(GLOBAL_SECTION, keys_list)) {
            keys.reserve(keys_list.size());
            std::transform(keys_list.begin(), keys_list.end(), std::back_inserter(keys), [](CSimpleIniA::Entry & entry) {
                return std::string(entry.pItem);
            });
        }
        return keys;
    }


    //-----------------------------------------------------------------------

    ini_builder::ini_builder()
    {
        m_impl = std::make_unique<ini_config_state>();
        m_impl->reader.SetMultiLine(true);
        m_impl->reader.SetMultiKey(false);
    }

    ini_builder::~ini_builder() = default;

    ini_builder& ini_builder::unicode(bool enable)
    {
        m_impl->reader.SetUnicode(enable);
        return *this;
    }

    ini_builder& ini_builder::multiline(bool enable)
    {
        m_impl->reader.SetMultiLine(enable);
        return *this;
    }

    config ini_builder::parse(std::istream & is)
    {
        m_impl->reader.LoadData(is);
        return config(std::make_shared<ini_config>(std::move(m_impl)));
    }

    config ini_builder::parse(const char* ini)
    {
        m_impl->reader.LoadData(ini, std::strlen(ini));
        return config(std::make_shared<ini_config>(std::move(m_impl)));
    }

    config ini_builder::parse(const std::string& ini)
    {
        m_impl->reader.LoadData(ini.c_str(), ini.size());
        return config(std::make_shared<ini_config>(std::move(m_impl)));
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

} // namespace conf

} //namespace yato
