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

    template <typename NJson_>
    json_config<NJson_>::json_config(std::shared_ptr<njson> json)
        : m_root(std::move(json)), m_iter(yato::nullopt_t{})
    { }

    template <typename NJson_>
    json_config<NJson_>::json_config(std::shared_ptr<njson> root, njson_citerator iter)
        : m_root(std::move(root)), m_iter(std::move(iter))
    { }

    template <typename NJson_>
    json_config<NJson_>::~json_config()
    {
        m_iter.reset();
        m_root.reset();
    }

    template <typename NJson_>
    auto json_config<NJson_>::get_() const
        -> njson_creference
    {
        return static_cast<bool>(m_iter) ? *m_iter.get_unsafe() : *m_root;
    }

    template <typename NJson_>
    size_t json_config<NJson_>::do_size() const noexcept
    {
        return get_().size();
    }

    template <typename NJson_>
    bool json_config<NJson_>::do_has_property(config_property p) const noexcept
    {
        switch (p) {
        case config_property::associative:
            return get_().is_object();
        case config_property::keeps_order:
            return std::is_same_v<njson, nlohmann::ordered_json> || get_().is_array();
        default:
            return false;
        }
    }

    template <typename NJson_>
    config_backend::find_key_result_t json_config<NJson_>::do_find(const std::string & name) const
    {
        find_key_result_t res = config_backend::no_key_result;
        const auto & json = get_();
        if (json.is_object()) {
            const auto it = json.find(name);
            if (it != json.cend()) {
                res = std::make_tuple(yato::narrow_cast<size_t>(std::distance(json.cbegin(), it)), new json_value<NJson_>(m_root, it));
            }
        }
        return res;
    }

    template <typename NJson_>
    config_backend::find_index_result_t json_config<NJson_>::do_find(size_t index) const
    {
        find_index_result_t res = config_backend::no_index_result;
        const auto & json = get_();
        if (index < json.size()) {
            const auto it = std::next(json.cbegin(), index);
            if (json.is_object()) {
                std::get<0>(res) = it.key();
            }
            std::get<1>(res) = new json_value<NJson_>(m_root, it);
        }
        return res;
    }

    template <typename NJson_>
    void json_config<NJson_>::do_release(const config_value* val) const noexcept
    {
        YATO_REQUIRES(dynamic_cast<const json_value<NJson_>*>(val) != nullptr);
        delete val;
    }


    template class json_config<nlohmann::json>;
    template class json_config<nlohmann::ordered_json>;

} //namespace json

} //namespace conf

} //namespace yato
