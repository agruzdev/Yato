/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include <nlohmann/json.hpp>

#include "json_config.h"

namespace yato {

namespace conf {


    /**
     * Store read-only iterator
     * Root is shared between all configs
     */
    class json_config_state
    {
        std::shared_ptr<nlohmann::json> m_root;
        yato::optional<nlohmann::json::const_iterator> m_iter;

    public:
        json_config_state(nlohmann::json && json)
            : m_root(std::make_shared<nlohmann::json>(std::move(json)))
            , m_iter(yato::nullopt_t{})
        { }

        json_config_state(const json_config_state & parent, const nlohmann::json::const_iterator & iter)
            : m_root(parent.m_root)
            , m_iter(iter)
        { }

        const nlohmann::json & get() const
        {
            return static_cast<bool>(m_iter) ? *m_iter.get_unsafe() : *m_root;
        }
    };

    //-------------------------------------------------------------------------


    json_config::json_config(std::unique_ptr<json_config_state> && impl)
        : m_impl(std::move(impl))
    { }

    json_config::json_config(json_config&&) noexcept = default;

    json_config& json_config::operator=(json_config&&) noexcept = default;

    bool json_config::is_object() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        const auto & json = m_impl->get();
        return json.is_object();
    }

    std::vector<std::string> json_config::keys() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        std::vector<std::string> res;
        const auto & json = m_impl->get();
        if(json.is_object()) {
            res.reserve(json.size());
            for(auto it = json.cbegin(); it != json.cend(); ++it) {
                res.push_back(it.key());
            }
        }
        return res;
    }

    static
    stored_variant get_impl(const json_config_state* self, config_type type, const nlohmann::json::const_iterator & it)
    {
        stored_variant res{};
        switch (type)
        {
        case yato::conf::config_type::integer:
            if(it->is_number_integer()) {
                using return_type = stored_type_trait<config_type::integer>::return_type;
                res.emplace<return_type>(yato::narrow_cast<return_type>(it->get<nlohmann::json::number_integer_t>()));
            }
            break;
        case yato::conf::config_type::boolean:
            if(it->is_boolean()) {
                using return_type = stored_type_trait<config_type::boolean>::return_type;
                res.emplace<return_type>(it->get<nlohmann::json::boolean_t>());
            }
            break;
        case yato::conf::config_type::floating:
            if(it->is_number_float()) {
                using return_type = stored_type_trait<config_type::floating>::return_type;
                res.emplace<return_type>(yato::float_cast<return_type>(it->get<nlohmann::json::number_float_t>()));
            }
            break;
        case yato::conf::config_type::string:
            if(it->is_string()) {
                using return_type = stored_type_trait<config_type::string>::return_type;
                res.emplace<return_type>(it->get<nlohmann::json::string_t>());
            }
            break;
        case yato::conf::config_type::config:
            if(it->is_object() || it->is_array()) {
                using return_type = stored_type_trait<config_type::config>::return_type;
                auto impl = std::make_unique<json_config_state>(*self, it);
                return_type subconfig = std::make_unique<json_config>(std::move(impl));
                res.emplace<return_type>(std::move(subconfig));
            }
            break;
        default:
            break;
        }
        return res;
    }

    stored_variant json_config::get_by_name(const std::string & name, config_type type) const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        stored_variant res{};

        const auto & json = m_impl->get();
        if(json.is_object()) {
            try {
                const auto it = json.find(name);
                if(it != json.end()) {
                    res = get_impl(m_impl.get(), type, it);
                }
            }
            catch(...) {
                // ToDo (a.gruzdev): Report error somehow
                /* In case of error return empty result */
            }
        }
        return res;
    }

    stored_variant json_config::get_by_index(size_t index, config_type type) const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        stored_variant res{};

        const auto & json = m_impl->get();
        if(json.is_array() && index < json.size()) {
            try {
                const auto it = std::next(json.cbegin(), index);
                if(it != json.end()) {
                    res = get_impl(m_impl.get(), type, it);
                }
            }
            catch(...) {
                // ToDo (a.gruzdev): Report error somehow
                /* In case of error return empty result */
            }
        }
        return res;
    }

    bool json_config::is_array() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);

        const auto & json = m_impl->get();
        return json.is_array();
    }

    size_t json_config::size() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);

        const auto & json = m_impl->get();
        return json.size();
    }

    //--------------------------------------------------------------------------
    // Json builder

    json_builder::json_builder() = default;
    json_builder::~json_builder() = default;

    json_builder::json_builder(json_builder&&) noexcept = default;
    json_builder& json_builder::operator=(json_builder&&) noexcept = default;

    inline
    config parse_impl_(const nlohmann::detail::input_adapter & input)
    {
        backend_ptr backend = nullptr;
        auto impl = std::make_unique<json_config_state>(nlohmann::json::parse(input, nullptr, false));
        if(!impl->get().is_discarded()) {
            backend = std::make_shared<json_config>(std::move(impl));
        }
        return config(backend);
    }

    config json_builder::parse(const char* json) const
    {
        return parse_impl_({ json });
    }

    config json_builder::parse(const std::string & json) const
    {
        return parse_impl_({ json });
    }

} // namespace conf

} // namespace yato
