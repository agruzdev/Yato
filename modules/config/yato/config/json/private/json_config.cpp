/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "json_config.h"
#include "json_value.h"

namespace yato {

namespace conf {

namespace json {

    json_config::json_config(std::shared_ptr<nlohmann::json> json)
        : m_root(std::move(json)), m_iter(yato::nullopt_t{})
    { }

    json_config::json_config(std::shared_ptr<nlohmann::json> root, nlohmann::json::const_iterator iter)
        : m_root(std::move(root)), m_iter(std::move(iter))
    { }

    json_config::~json_config()
    {
        m_iter.reset();
        m_root.reset();
    }

    nlohmann::json::const_reference json_config::get_() const
    {
        return static_cast<bool>(m_iter) ? *m_iter.get_unsafe() : *m_root;
    }

    size_t json_config::do_size() const noexcept
    {
        return get_().size();
    }

    bool json_config::do_has_property(config_property p) const noexcept
    {
        switch (p) {
        case config_property::associative:
            return get_().is_object();
        case config_property::ordered:
            return get_().is_array();
        default:
            return false;
        }
    }

    config_backend::key_value_t json_config::do_find(const std::string & name) const noexcept
    {
        key_value_t res = config_backend::novalue;
        const auto & json = get_();
        if (json.is_object()) {
            const nlohmann::json::const_iterator it = json.find(name);
            if (it != json.cend()) {
                res.first  = name;
                res.second = new json_value(m_root, it);
            }
        }
        return res;
    }

    config_backend::key_value_t json_config::do_find(size_t index) const noexcept
    {
        key_value_t res = config_backend::novalue;
        const auto & json = get_();
        if (index < json.size()) {
            const nlohmann::json::const_iterator it = std::next(json.cbegin(), index);
            if (json.is_object()) {
                res.first  = it.key();
            }
            res.second = new json_value(m_root, it);
        }
        return res;
    }

    void json_config::do_release(const config_value* val) const noexcept
    {
        YATO_REQUIRES(dynamic_cast<const json_value*>(val) != nullptr);
        delete val;
    }

} //namespace json

} //namespace conf

} //namespace yato
