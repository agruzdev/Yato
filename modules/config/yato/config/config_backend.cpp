/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "config_backend.h"

namespace yato
{

namespace conf
{

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

} // namespace conf

} // namespace yato
