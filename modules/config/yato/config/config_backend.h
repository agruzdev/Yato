/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_BACKEND_H_
#define _YATO_CONFIG_BACKEND_H_

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include <yato/optional.h>
#include <yato/variant.h>
#include <yato/assertion.h>
#include "yato/any.h"

namespace yato {

namespace conf {

    class config_error
        : public yato::runtime_error
    {
    public:
        config_error(const std::string & err)
            : yato::runtime_error(err)
        { }
    
        config_error(const char* err)
            : yato::runtime_error(err)
        { }
    };

    enum class stored_type
        : int32_t
    {
        integer,
        real,
        boolean,
        string,
        config
    };

    class config_backend;
    using backend_ptr = std::shared_ptr<config_backend>;



    template <stored_type StoredTy_>
    struct stored_type_trait
    { };

    template <>
    struct stored_type_trait<stored_type::integer>
    {
        using return_type = int64_t;
    };

    template <>
    struct stored_type_trait<stored_type::real>
    {
        using return_type = double;
    };

    template <>
    struct stored_type_trait<stored_type::boolean>
    {
        using return_type = bool;
    };

    template <>
    struct stored_type_trait<stored_type::string>
    {
        using return_type = std::string;
    };

    template <>
    struct stored_type_trait<stored_type::config>
    {
        using return_type = backend_ptr;
    };


    namespace details
    {
        template <typename ToType_, typename FromType_>
        struct identity_converter
        {
            constexpr
            ToType_ operator()(FromType_ && val) const {
                return static_cast<ToType_>(val);
            }
        };

        template <typename ToType_, typename FromType_>
        struct narrow_converter
        {
            constexpr
            ToType_ operator()(FromType_ && val) const {
                return yato::narrow_cast<ToType_>(std::forward<FromType_>(val));
            }
        };

        /**
         * Assumes that FromType_ is wider than ToType_
         */
        template <typename ToType_, typename FromType_>
        struct checked_limits_converter
        {
            YATO_CONSTEXPR_FUNC_CXX14
            ToType_ operator()(FromType_ && val) const {
                YATO_REQUIRES(val <= static_cast<FromType_>(std::numeric_limits<ToType_>::max()));
                YATO_REQUIRES(val >= static_cast<FromType_>(std::numeric_limits<ToType_>::min()));
                return static_cast<ToType_>(val);
            }
        };
    }

    template <typename Ty_>
    struct config_value_trait
    { };

    template <>
    struct config_value_trait<uint8_t>
    {
        using converter_type = details::narrow_converter<uint8_t, int64_t>;
        static constexpr stored_type fetch_type = stored_type::integer;
    };

    template <>
    struct config_value_trait<uint16_t>
    {
        using converter_type = details::narrow_converter<uint16_t, int64_t>;
        static constexpr stored_type fetch_type = stored_type::integer;
    };

    template <>
    struct config_value_trait<uint32_t>
    {
        using converter_type = details::narrow_converter<uint32_t, int64_t>;
        static constexpr stored_type fetch_type = stored_type::integer;
    };

    template <>
    struct config_value_trait<uint64_t>
    {
        using converter_type = details::narrow_converter<uint64_t, int64_t>;
        static constexpr stored_type fetch_type = stored_type::integer;
    };

    template <>
    struct config_value_trait<int8_t>
    {
        using converter_type = details::narrow_converter<int8_t, int64_t>;
        static constexpr stored_type fetch_type = stored_type::integer;
    };

    template <>
    struct config_value_trait<int16_t>
    {
        using converter_type = details::narrow_converter<int16_t, int64_t>;
        static constexpr stored_type fetch_type = stored_type::integer;
    };

    template <>
    struct config_value_trait<int32_t>
    {
        using converter_type = details::narrow_converter<int32_t, int64_t>;
        static constexpr stored_type fetch_type = stored_type::integer;
    };

    template <>
    struct config_value_trait<int64_t>
    {
        using converter_type = details::identity_converter<int64_t, int64_t>;
        static constexpr stored_type fetch_type = stored_type::integer;
    };

    template <>
    struct config_value_trait<float>
    {
        using converter_type = details::checked_limits_converter<float, double>;
        static constexpr stored_type fetch_type = stored_type::real;
    };

    template <>
    struct config_value_trait<double>
    {
        using converter_type = details::identity_converter<double, double>;
        static constexpr stored_type fetch_type = stored_type::real;
    };

    template <>
    struct config_value_trait<long double>
    {
        using converter_type = details::identity_converter<long double, double>;
        static constexpr stored_type fetch_type = stored_type::real;
    };

    template <>
    struct config_value_trait<bool>
    {
        using converter_type = details::identity_converter<bool, bool>;
        static constexpr stored_type fetch_type = stored_type::boolean;
    };

    template <>
    struct config_value_trait<std::string>
    {
        using converter_type = details::identity_converter<std::string, std::string>;
        static constexpr stored_type fetch_type = stored_type::string;
    };



    /**
     * Represents any stored type
     */
    using stored_variant = yato::variant<
        void,
        typename stored_type_trait<stored_type::integer>::return_type,
        typename stored_type_trait<stored_type::real>::return_type,
        typename stored_type_trait<stored_type::boolean>::return_type,
        typename stored_type_trait<stored_type::string>::return_type,
        typename stored_type_trait<stored_type::config>::return_type
    >;


    /**
     * Interface for config implementation
     */
    class config_backend  // NOLINT
    {
    public:
        virtual ~config_backend() = 0;

        /**
         * Get number of stored values.
         */
        virtual size_t size() const noexcept = 0;

        /**
         * Fetch value by index.
         */
        virtual stored_variant get_by_index(size_t index, stored_type type) const noexcept = 0;

        /**
         * Returns true if config stores key-value pairs, otherwise only indexed access is supported.
         */
        virtual bool is_object() const noexcept = 0;

        /**
         * Fetch value by key. Valid only if has_keys() returns true.
         */
        virtual stored_variant get_by_key(const std::string & /*name*/, stored_type /*type*/) const noexcept { return {}; }

        /**
         * Get all stored keys. Valid only for key-value config.
         */
        virtual std::vector<std::string> keys() const noexcept { return {}; }
    };

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_H_
