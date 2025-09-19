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

    template <typename NJson_>
    json_value<NJson_>::json_value(std::shared_ptr<njson> root, njson_citerator iter)
        : m_root(std::move(root)), m_iter(std::move(iter))
    { }

    template <typename NJson_>
    json_value<NJson_>::~json_value() = default;

    template <typename NJson_>
    stored_type json_value<NJson_>::type() const noexcept
    {
        switch(m_iter->type()) {
            case njson::value_t::number_integer:
            case njson::value_t::number_unsigned:
                return stored_type::integer;
            case njson::value_t::number_float:
                return stored_type::real;
            case njson::value_t::string:
                return stored_type::string;
            case njson::value_t::boolean:
                return stored_type::boolean;
            default:
                return stored_type::config;
        }
    }

    template <typename NJson_>
    stored_variant json_value<NJson_>::get() const noexcept
    {
        using integer_t = stored_type_trait<stored_type::integer>::return_type;
        using real_t    = stored_type_trait<stored_type::real>::return_type;
        using string_t  = stored_type_trait<stored_type::string>::return_type;
        using boolean_t = stored_type_trait<stored_type::boolean>::return_type;

        stored_variant res;
        switch(m_iter->type()) {
            case njson::value_t::number_integer:
                res.emplace<integer_t>(yato::narrow_cast<integer_t>(m_iter->get<typename njson::number_integer_t>()));
                break;
            case njson::value_t::number_unsigned:
                res.emplace<integer_t>(yato::narrow_cast<integer_t>(m_iter->get<typename njson::number_unsigned_t>()));
                break;
            case njson::value_t::number_float:
                res.emplace<real_t>(static_cast<real_t>(m_iter->get<typename njson::number_float_t>()));
                break;
            case njson::value_t::string:
                res.emplace<string_t>(static_cast<string_t>(m_iter->get<typename njson::string_t>()));
                break;
            case njson::value_t::boolean:
                res.emplace<boolean_t>(static_cast<boolean_t>(m_iter->get<typename njson::boolean_t>()));
                break;
            case njson::value_t::array:
            case njson::value_t::object:
                res.emplace<backend_ptr_t>(std::make_shared<json_config<NJson_>>(m_root, m_iter));
                break;
            default:
                // ToDo (a.gruzdev): Report warning
                break;
        }
        return res;
    }


    template class json_value<nlohmann::json>;
    template class json_value<nlohmann::ordered_json>;


} //namespace json

} //namespace conf

} //namespace yato

