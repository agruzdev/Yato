/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_INI_PRIVATE_INI_VALUE_H_
#define _YATO_CONFIG_INI_PRIVATE_INI_VALUE_H_

#include "../../config_backend.h"
#include "../../utility.h"
#include "ini_lib.h"

namespace yato {

namespace conf {


    class ini_value final
        : public config_value
    {
    public:
        ini_value(const char* data)
            : m_data(data)
        { }

        ini_value(const ini_value&) = delete;
        ini_value(ini_value&&) = delete;

        ini_value& operator=(const ini_value&) = delete;
        ini_value& operator=(ini_value&&) = delete;

        ~ini_value() = default;

        stored_type type() const noexcept override
        {
            return stored_type::string;
        }

        stored_variant get_as(stored_type dst_type) const noexcept override
        {
            stored_variant res{};
            if (!cvt_from(m_data, res, dst_type)) {
                // ToDo (a.gruzdev): Report warning
            }
            return res;
        }

    private:
        std::string m_data;
    };


} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_INI_PRIVATE_INI_VALUE_H_
