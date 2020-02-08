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


    const config_backend::key_value_t config_backend::novalue = std::make_pair(std::string{}, nullptr);

    bool config_backend::do_is_object() const noexcept
    {
        return false;
    }

    config_backend::key_value_t config_backend::do_find(const std::string & /*name*/) const noexcept
    {
        return config_backend::novalue;
    }

    std::vector<std::string> config_backend::do_keys() const noexcept
    {
        std::vector<std::string> res;
        if (is_object()) {
            try {
                const size_t count = size();
                std::vector<std::string> tmp;
                tmp.reserve(count);
                for (size_t i = 0; i < count; ++i) {
                    auto kv = find(i);
                    tmp.push_back(kv.first);
                    release(kv.second);
                }
                res.swap(tmp);
            }
            catch(...) {
                //ToDo (a.gruzdev): Add error callbacks
            }
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
