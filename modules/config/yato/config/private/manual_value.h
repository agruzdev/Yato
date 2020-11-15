/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
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

    template <stored_type StoredType_>
    class manual_value final
        : public config_value
    {
    public:
        static YATO_CONSTEXPR_VAR stored_type config_type = StoredType_;
        using value_type  = typename stored_type_trait<StoredType_>::return_type;

        manual_value(value_type val)
            : m_value(std::move(val))
        { }

        ~manual_value() = default;

        manual_value(const manual_value&) = delete;
        manual_value(manual_value&&) = default;

        manual_value& operator=(const manual_value&) = delete;
        manual_value& operator=(manual_value&&) = default;

        stored_type type() const noexcept override
        {
            return config_type;
        }

        stored_variant get() const noexcept override
        {
            return stored_variant(yato::in_place_type_t<value_type>{}, m_value);
        }

    private:
        value_type m_value;
    };

} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_MANUAL_PRIVATE_MANUAL_VALUE_H_
