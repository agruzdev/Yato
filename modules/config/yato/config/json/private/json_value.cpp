/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "json_value.h"
#include "json_config.h"
#include "../../utility.h"

namespace yato {

namespace conf {

namespace json {

    json_value::json_value(std::shared_ptr<nlohmann::json> root, nlohmann::json::const_iterator iter)
        : m_root(std::move(root)), m_iter(std::move(iter))
    { }

    json_value::~json_value() = default;

    stored_type json_value::type() const noexcept
    {
        switch(m_iter->type()) {
            case nlohmann::json::value_t::number_integer:
            case nlohmann::json::value_t::number_unsigned:
                return stored_type::integer;
            case nlohmann::json::value_t::number_float:
                return stored_type::real;
            case nlohmann::json::value_t::string:
                return stored_type::string;
            case nlohmann::json::value_t::boolean:
                return stored_type::boolean;
            default:
                return stored_type::config;
        }
    }

    stored_variant json_value::get() const noexcept
    {
        using integer_t = stored_type_trait<stored_type::integer>::return_type;
        using real_t    = stored_type_trait<stored_type::real>::return_type;
        using string_t  = stored_type_trait<stored_type::string>::return_type;
        using boolean_t = stored_type_trait<stored_type::boolean>::return_type;

        stored_variant res;
        switch(m_iter->type()) {
            case nlohmann::json::value_t::number_integer:
                res.emplace<integer_t>(yato::narrow_cast<integer_t>(m_iter->get<nlohmann::json::number_integer_t>()));
                break;
            case nlohmann::json::value_t::number_unsigned:
                res.emplace<integer_t>(yato::narrow_cast<integer_t>(m_iter->get<nlohmann::json::number_unsigned_t>()));
                break;
            case nlohmann::json::value_t::number_float:
                res.emplace<real_t>(static_cast<real_t>(m_iter->get<nlohmann::json::number_float_t>()));
                break;
            case nlohmann::json::value_t::string:
                res.emplace<string_t>(static_cast<string_t>(m_iter->get<nlohmann::json::string_t>()));
                break;
            case nlohmann::json::value_t::boolean:
                res.emplace<boolean_t>(static_cast<boolean_t>(m_iter->get<nlohmann::json::boolean_t>()));
                break;
            case nlohmann::json::value_t::array:
            case nlohmann::json::value_t::object:
                res.emplace<backend_ptr>(std::make_shared<json_config>(m_root, m_iter));
                break;
            default:
                // ToDo (a.gruzdev): Report warning
                break;
        }
        return res;
    }

} //namespace json

} //namespace conf

} //namespace yato

