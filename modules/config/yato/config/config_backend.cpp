/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "config_backend.h"
#include "utility.h"

namespace yato {

namespace conf {

    stored_variant config_value::convert_(stored_type dst_type, const stored_variant & src) const
    {
        return value_converter::instance().apply(dst_type, src);
    }


    const config_backend::find_index_result_t config_backend::no_index_result = std::make_tuple(std::string{}, nullptr);

    const config_backend::find_key_result_t config_backend::no_key_result = std::make_tuple(0, nullptr);

    bool config_backend::do_has_property(config_property /*p*/) const noexcept
    {
        return false;
    }

    config_backend::find_key_result_t config_backend::do_find(const std::string & /*name*/) const
    {
        return config_backend::no_key_result;
    }

    std::vector<std::string> config_backend::do_enumerate_keys() const
    {
        std::vector<std::string> res;
        if (do_has_property(config_property::associative)) {
            const size_t count = size();
            std::vector<std::string> tmp;
            tmp.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                tmp.emplace_back(find(i).get_key());
            }
            res.swap(tmp);
        }
        return res;
    }

    std::string to_string(stored_type type)
    {
        switch(type)
        {
            case stored_type::boolean:
                return "Boolean";
            case stored_type::integer:
                return "Integer";
            case stored_type::real:
                return "Real";
            case stored_type::string:
                return "String";
            case stored_type::config:
                return "Config";
            default:
                return "Unknown";
        }
    }

} // namespace conf

} // namespace yato
