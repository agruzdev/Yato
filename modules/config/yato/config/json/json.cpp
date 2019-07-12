/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "json.h"
#include "private/json_config.h"

namespace yato {

namespace conf {

    json_builder::json_builder() = default;
    json_builder::~json_builder() = default;

    json_builder::json_builder(json_builder&&) noexcept = default;
    json_builder& json_builder::operator=(json_builder&&) noexcept = default;

    inline
    config parse_impl_(const nlohmann::detail::input_adapter & input)
    {
        backend_ptr backend = nullptr;
        auto json = std::make_shared<nlohmann::json>(nlohmann::json::parse(input, nullptr, false));
        if(!json->is_discarded()) {
            backend = std::make_shared<json_config>(std::move(json));
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

} //namespace conf

} //namespace yato


