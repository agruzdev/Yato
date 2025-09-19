/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2025 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_CONTAINERS_H_
#define _YATO_CONFIG_CONTAINERS_H_

#include <vector>
#include <optional>
#include <map>

#include "config.h"
#include "config_builder.h"


namespace yato {

namespace conf {

    namespace details
    {
        template <typename Ty_, typename Alloc_>
        struct std_vector_converter
        {
            std::vector<Ty_, Alloc_> load(const yato::config& c) const
            {
                std::vector<Ty_, Alloc_> res{};
                res.reserve(c.size());
                for (const auto& entry : c) {
                    res.emplace_back(entry.value<Ty_>().template get_or_throw<config_error>("std_vector_converter[load]: Failed to decode array element at " + std::to_string(entry.index())));
                }
                return res;
            }

            yato::config store(const std::vector<Ty_, Alloc_>& vec) const
            {
                auto b = config_builder::array();
                for (const auto& v : vec) {
                    b.add(v);
                }
                return b.create();
            }
        };

        template <typename Kty_, typename Ty_, typename Pr_, typename Alloc_>
        struct std_map_converter
        {
            std::map<Kty_, Ty_, Pr_, Alloc_> load(const yato::config& c) const
            {
                if (!c.is_associative()) {
                    throw config_error("std_map_converter[load]: Input config is not associative.");
                }
                std::map<Kty_, Ty_, Pr_, Alloc_> res{};
                for (const auto& entry : c) {
                    res.emplace(entry.key(), entry.value<Ty_>().template get_or_throw<config_error>("std_map_converter[load]: Failed to decode array element at " + entry.key()));
                }
                return res;
            }

            yato::config store(const std::map<Kty_, Ty_, Pr_, Alloc_>& m) const
            {
                auto b = config_builder::object();
                for (const auto& [k, v] : m) {
                    b.put(k, v);
                }
                return b.create();
            }
        };

        template <typename Ty_>
        struct std_optional_converter
        {
            std::optional<Ty_> load(const typename conversion_traits<Ty_>::value_type& s) const
            {
                using payload_converter_type = typename conversion_traits<Ty_>::converter_type;
                return invoke_load(payload_converter_type{}, s);
            }

            bool skip(const std::optional<Ty_>& opt) const
            {
                return !opt.has_value();
            }

            typename conversion_traits<Ty_>::value_type store(const std::optional<Ty_>& opt) const
            {
                using payload_converter_type = typename conversion_traits<Ty_>::converter_type;
                return yato::conf::invoke_store(payload_converter_type{}, opt.value());
            }
        };

        template <typename Ty_>
        struct yato_optional_converter
        {
            yato::optional<Ty_> load(const typename conversion_traits<Ty_>::value_type& s) const
            {
                using payload_converter_type = typename conversion_traits<Ty_>::converter_type;
                return invoke_load(payload_converter_type{}, s);
            }

            bool skip(const yato::optional<Ty_>& opt) const
            {
                return opt.empty();
            }

            typename conversion_traits<Ty_>::value_type store(const yato::optional<Ty_>& opt) const
            {
                using payload_converter_type = typename conversion_traits<Ty_>::converter_type;
                return yato::conf::invoke_store(payload_converter_type{}, opt.get());
            }
        };


    } // namespace details

    template <typename Ty_,typename Alloc_>
    struct config_value_trait<std::vector<Ty_, Alloc_>>
    {
        static constexpr stored_type fetch_type = stored_type::config;
        using converter_type = details::std_vector_converter<Ty_, Alloc_>;
    };

    template <typename Kty_, typename Ty_, typename Pr_, typename Alloc_>
    struct config_value_trait<std::map<Kty_, Ty_, Pr_, Alloc_>>
    {
        static constexpr stored_type fetch_type = stored_type::config;
        using converter_type = details::std_map_converter<Kty_, Ty_, Pr_, Alloc_>;
    };

    template <typename Ty_>
    struct config_value_trait<std::optional<Ty_>>
    {
        static constexpr stored_type fetch_type = conversion_traits<Ty_>::fetch_type;
        using converter_type = details::std_optional_converter<Ty_>;
    };

    template <typename Ty_>
    struct config_value_trait<yato::optional<Ty_>>
    {
        static constexpr stored_type fetch_type = conversion_traits<Ty_>::fetch_type;
        using converter_type = details::yato_optional_converter<Ty_>;
    };


} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_CONTAINERS_H_
