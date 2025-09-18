/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2025 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_SMART_POINTERS_H_
#define _YATO_CONFIG_SMART_POINTERS_H_

#include <vector>
#include <optional>
#include <map>

#include "config.h"
#include "config_builder.h"


namespace yato {

namespace conf {

    namespace details
    {
        template <typename Ty_>
        struct std_unique_ptr_converter
        {
            std::unique_ptr<Ty_> load(const typename conversion_traits<Ty_>::value_type& s) const
            {
                using payload_converter_type = conversion_traits<Ty_>::converter_type;
                return std::make_unique<Ty_>(invoke_load(payload_converter_type{}, s));
            }

            bool skip(const std::unique_ptr<Ty_>& p) const
            {
                return (p == nullptr);
            }

            typename conversion_traits<Ty_>::value_type store(const std::unique_ptr<Ty_>& p) const
            {
                using payload_converter_type = conversion_traits<Ty_>::converter_type;
                return yato::conf::invoke_store(payload_converter_type{}, *p);
            }
        };

        template <typename Ty_>
        struct std_shared_ptr_converter
        {
            std::shared_ptr<Ty_> load(const typename conversion_traits<Ty_>::value_type& s) const
            {
                using payload_converter_type = conversion_traits<Ty_>::converter_type;
                return std::make_shared<Ty_>(invoke_load(payload_converter_type{}, s));
            }

            bool skip(const std::shared_ptr<Ty_>& p) const
            {
                return (p == nullptr);
            }

            typename conversion_traits<Ty_>::value_type store(const std::shared_ptr<Ty_>& p) const
            {
                using payload_converter_type = conversion_traits<Ty_>::converter_type;
                return yato::conf::invoke_store(payload_converter_type{}, *p);
            }
        };

    } // namespace details


    template <typename Ty_>
    struct config_value_trait<std::unique_ptr<Ty_>>
    {
        static constexpr stored_type fetch_type = conversion_traits<Ty_>::fetch_type;
        using converter_type = details::std_unique_ptr_converter<Ty_>;
    };

    template <typename Ty_>
    struct config_value_trait<std::shared_ptr<Ty_>>
    {
        static constexpr stored_type fetch_type = conversion_traits<Ty_>::fetch_type;
        using converter_type = details::std_shared_ptr_converter<Ty_>;
    };

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_SMART_POINTERS_H_
