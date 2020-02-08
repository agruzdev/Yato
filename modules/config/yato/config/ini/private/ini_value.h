/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_INI_PRIVATE_INI_VALUE_H_
#define _YATO_CONFIG_INI_PRIVATE_INI_VALUE_H_

#include "../../config_backend.h"

namespace yato {

namespace conf {


    class ini_value final
        : public config_value
    {
    public:
        ini_value(std::string data)
            : m_data(std::move(data))
        { }

        ini_value(const ini_value&) = delete;
        ini_value(ini_value&&) = delete;

        ini_value& operator=(const ini_value&) = delete;
        ini_value& operator=(ini_value&&) = delete;

        ~ini_value() override = default;

        stored_type type() const noexcept override
        {
            return stored_type::string;
        }

        stored_variant get() const noexcept override
        {
            return stored_variant(yato::in_place_type_t<std::string>{}, m_data);
        }

    private:
        std::string m_data;
    };


} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_INI_PRIVATE_INI_VALUE_H_
