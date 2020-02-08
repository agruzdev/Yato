/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_JSON_PRIVATE_JSON_VALUE_H_
#define _YATO_CONFIG_JSON_PRIVATE_JSON_VALUE_H_


#include <nlohmann/json.hpp>

#include "../../config_backend.h"

namespace yato {

namespace conf {

    class json_converter;

    class json_value final
        : public config_value
    {
    public:
        json_value(std::shared_ptr<nlohmann::json> root, nlohmann::json::const_iterator iter);

        json_value(const json_value&) = delete;
        json_value(json_value&&) = delete;

        json_value& operator=(const json_value&) = delete;
        json_value& operator=(json_value&&) = delete;

        ~json_value();

        stored_type type() const noexcept override;

        stored_variant get() const noexcept override;

    private:
        std::shared_ptr<nlohmann::json> m_root;
        nlohmann::json::const_iterator m_iter;
    };

} //namespace conf

} //namespace yato

#endif //_YATO_CONFIG_JSON_PRIVATE_JSON_VALUE_H_
