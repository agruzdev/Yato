/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_VALUE_H_
#define _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_VALUE_H_

#include "yato/variant.h"
#include "yato/config/config.h"
#include "yato/config/utility.h"
#include "yato/variant_match.h"

namespace yato
{

namespace conf
{

    using manual_scalar = yato::variant<int64_t, double, bool, std::string>;


    class manual_value
        : public config_value
    {
    private:
        yato::variant<manual_scalar, backend_ptr> m_data;

    public:
        manual_value(int64_t val) 
            : m_data(manual_scalar(in_place_type_t<int64_t>{}, val))
        { }

        manual_value(double val) 
            : m_data(manual_scalar(in_place_type_t<double>{}, val))
        { }

        manual_value(bool val) 
            : m_data(manual_scalar(in_place_type_t<bool>{}, val))
        { }

        manual_value(std::string val) 
            : m_data(manual_scalar(in_place_type_t<std::string>{}, std::move(val)))
        { }

        manual_value(backend_ptr val) 
            : m_data(std::move(val))
        { }

        ~manual_value() = default;

        manual_value(const manual_value&) = delete;
        manual_value(manual_value&&) = default;

        manual_value& operator=(const manual_value&) = delete;
        manual_value& operator=(manual_value&&) = default;

        stored_type type() const noexcept override
        {
            if(m_data.is_type<manual_scalar>()) {
                const auto & scalar = m_data.get_as<manual_scalar>();
                if (scalar.is_type<int64_t>()) {
                    return stored_type::integer;
                }
                else if (scalar.is_type<double>()) {
                    return stored_type::real;
                }
                else if (scalar.is_type<bool>()) {
                    return stored_type::boolean;
                }
                else {
                    return stored_type::string;
                }
            }
            return stored_type::config;
        }

        stored_variant get_as(stored_type dst_type) const noexcept override
        {
            stored_variant res{};
            variant_match(
                [&](const backend_ptr & ptr) {
                    res.emplace<backend_ptr>(ptr);
                },
                [&](const manual_scalar & scalar) {
                    variant_match(
                        [&](int64_t val) {
                            cvt_from(val, res, dst_type);
                        },
                        [&](double val) {
                            cvt_from(val, res, dst_type);
                        },
                        [&](bool val) {
                            cvt_from(val, res, dst_type);
                        },
                        [&](const std::string & val) {
                            cvt_from(val, res, dst_type);
                        }
                    )(scalar);
                }
            )(m_data);
            return res;
        }
    };

} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_MANUAL_PRIVATE_MANUAL_VALUE_H_
