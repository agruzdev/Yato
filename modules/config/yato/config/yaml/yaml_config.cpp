/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016-2018 Alexey Gruzdev
 */

#include "yato/prerequisites.h"

#ifndef _NOEXCEPT
# define _NOEXCEPT YATO_NOEXCEPT_KEYWORD
#endif
YATO_PRAGMA_WARNING_PUSH
YATO_MSCV_WARNING_IGNORE(4127)
#include "yaml-cpp/yaml.h"
YATO_PRAGMA_WARNING_POP

#include "yaml_config.h"
#include "yato/config/json/json_config.h"


namespace yato {

namespace conf {


    class yaml_config_state
    {
    public:
        // shares state inside
        YAML::Node node;
    };

    //-------------------------------------------------------------------------

    yaml_config::yaml_config(std::unique_ptr<yaml_config_state> && impl)
        : m_impl(std::move(impl))
    { }

    yaml_config::yaml_config(yaml_config&&) noexcept = default;

    yaml_config& yaml_config::operator=(yaml_config&&) noexcept = default;

    bool yaml_config::is_object() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        return m_impl->node.IsMap();
    }

    std::vector<std::string> yaml_config::keys() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        std::vector<std::string> res;
        const auto & node = m_impl->node;
        if(node.IsMap()) {
            res.reserve(node.size());
            for(auto it = node.begin(); it != node.end(); ++it) {
                res.push_back((*it).first.as<std::string>());
            }
        }
        return res;
    }

    static
    stored_variant get_impl(config_type type, const YAML::Node & it)
    {
        stored_variant res{ };
        switch (type)
        {
        case yato::conf::config_type::integer:
            if(it.IsScalar()) {
                using return_type = stored_type_trait<config_type::integer>::return_type;
                try {
                    res.emplace<return_type>(it.as<return_type>());
                }
                catch(YAML::Exception &) {
                    
                }
            }
            break;
        case yato::conf::config_type::boolean:
            if(it.IsScalar()) {
                using return_type = stored_type_trait<config_type::boolean>::return_type;
                try {
                    res.emplace<return_type>(it.as<return_type>());
                }
                catch(YAML::Exception &) {
                    
                }
            }
            break;
        case yato::conf::config_type::floating:
            if(it.IsScalar()) {
                using return_type = stored_type_trait<config_type::floating>::return_type;
                try {
                    res.emplace<return_type>(it.as<return_type>());
                }
                catch(YAML::Exception &) {
                    
                }
            }
            break;
        case yato::conf::config_type::string:
            if(it.IsScalar()) {
                using return_type = stored_type_trait<config_type::string>::return_type;
                try {
                    res.emplace<return_type>(it.as<return_type>());
                }
                catch(YAML::Exception &) {
                    
                }
            }
            break;
        case yato::conf::config_type::config:
            if(it.IsMap() || it.IsSequence()) {
                using return_type = stored_type_trait<config_type::config>::return_type;
                auto impl = std::make_unique<yaml_config_state>();
                impl->node = it;
                return_type subconfig = std::make_unique<yaml_config>(std::move(impl));
                res.emplace<return_type>(std::move(subconfig));
            }
            break;
        default:
            break;
        }
        return res;
    }

    stored_variant yaml_config::get_by_name(const std::string & name, config_type type) const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        stored_variant res{ };

        const auto & node = m_impl->node;
        if (node.IsMap()) {
            try {
                const YAML::Node it = node[name];
                if (it.IsDefined()) {
                    res = get_impl(type, it);
                }
            }
            catch(...) {
                // ToDo (a.gruzdev): Report error somehow
                /* In case of error return empty result */
            }
        }
        return res;
    }

    stored_variant yaml_config::get_by_index(size_t index, config_type type) const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        stored_variant res{ };

        const auto & node = m_impl->node;
        if (node.IsSequence() && index < node.size()) {
            try {
                const YAML::Node it = node[index];
                if (it.IsDefined()) {
                    res = get_impl(type, it);
                }
            }
            catch(...) {
                // ToDo (a.gruzdev): Report error somehow
                /* In case of error return empty result */
            }
        }
        return res;
    }

    bool yaml_config::is_array() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        return m_impl->node.IsSequence();
    }

    size_t yaml_config::size() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        return m_impl->node.size();
    }

    //--------------------------------------------------------------------------
    // Json builder

    yaml_builder::yaml_builder() = default;
    yaml_builder::~yaml_builder() = default;

    yaml_builder::yaml_builder(yaml_builder&&) noexcept = default;
    yaml_builder& yaml_builder::operator=(yaml_builder&&) noexcept = default;

    template <typename InputTy_>
    config parse_yaml_(InputTy_ && input)
    {
        backend_ptr backend = nullptr;
        auto impl{ std::make_unique<yaml_config_state>() };
        impl->node = YAML::Load(std::forward<InputTy_>(input));
        if(impl->node.IsDefined()) {
            backend = std::make_shared<yaml_config>(std::move(impl));
        }
        return config{ backend };
    }

    config yaml_builder::parse(const char* yaml) const
    {
        return parse_yaml_(yaml);
    }

    config yaml_builder::parse(const std::string & yaml) const
    {
        return parse_yaml_(yaml);
    }

} // namespace conf

} // namespace yato
