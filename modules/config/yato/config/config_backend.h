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
#include "yato/finally.h"

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
        integer = 0,
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
     * Return type description string
     */
    std::string to_string(stored_type type);

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
     * Config element handle
     */
    class config_value
    {
    public:
        virtual ~config_value() = default;

        /**
         * Gives a hint what is an original type for the value.
         * Usually string for text configs.
         */
        virtual stored_type type() const noexcept = 0;

        /**
         * Get value with possible convertion.
         */
        //virtual stored_variant get_as(stored_type dst_type) const noexcept = 0;
        virtual stored_variant get() const noexcept = 0;

        ///**
        // * Helper method wrapping returned type into optional
        // */
        //template <typename Ty_>
        //yato::optional<Ty_> get_opt(stored_type dst_type) const 
        //{
        //    return get_as(dst_type).get_opt<Ty_>();
        //}

        /**
         * Converts stored value to desired type if possible.
         */
        template <typename Ty_>
        yato::optional<Ty_> get_with_conversion(stored_type dst_type) const 
        {
            auto val = get();
            if (!val.is_type<void>()) {
                val = convert_(dst_type, val);
            }
            return val.get_opt<Ty_>();
        }

    private:
        stored_variant convert_(stored_type dst_type, const stored_variant & src) const;
    };

    /**
     * Facade class providing a non-virtual interface for config implementations 
     */
    class config_backend  // NOLINT
    {
    public:
        using key_value_t = std::pair<std::string, const config_value*>;

        /**
         * Constant representing null key value pair
         */
        const static key_value_t novalue;


        virtual ~config_backend() = default;

        /**
         * Get number of stored values.
         */
        size_t size() const
        {
            return do_size();
        }

        /**
         * Returns true if config stores key-value pairs, otherwise only indexed access is supported.
         */
        bool is_object() const
        {
            return do_is_object();
        }

        /**
         * Find value by index.
         */
        key_value_t find(size_t index) const
        {
            return do_find(index);
        }

        /**
         * Find value by name.
         */
        key_value_t find(const std::string & name) const
        {
            return do_find(name);
        }

        /**
         * Release value handle obtained from find().
         */
        void release(const config_value* val) const
        {
            do_release(val);
        }

        /**
         * Enumerate object's keys
         */
        std::vector<std::string> keys() const
        {
            return do_keys();
        }

        /**
         * Helper method wrapping returned type into optional
         */
        template <typename Ty_>
        yato::optional<Ty_> get(size_t index, stored_type dst_type) const 
        {
            const config_value* value = find(index).second;
            if (value) {
                yato_finally(([this, value]{ release(value); }));
                return value->get_with_conversion<Ty_>(dst_type);
            }
            return yato::nullopt_t{};
        }

        /**
         * Helper method wrapping returned type into optional
         */
        template <typename Ty_>
        yato::optional<Ty_> get(const std::string & name, stored_type dst_type) const 
        {
            const config_value* value = find(name).second;
            if (value) {
                yato_finally(([this, value]{ release(value); }));
                return value->get_with_conversion<Ty_>(dst_type);
            }
            return yato::nullopt_t{};
        }

    protected:

        /**
         * Get number of stored values.
         */
        virtual size_t do_size() const noexcept = 0;

        /**
         * Find value by index.
         */
        virtual key_value_t do_find(size_t index) const noexcept = 0;
        
        /**
         * Release value handle obtained from do_find().
         */
        virtual void do_release(const config_value* val) const noexcept = 0;

        /**
         * Returns false by default
         */
        virtual bool do_is_object() const noexcept;

        /**
         * Find value by name.
         * Returns config_backend::novalue by default;
         */
        virtual key_value_t do_find(const std::string & name) const noexcept;

        /**
         * Default implementation calls find() for all valid indexes.
         */
        virtual std::vector<std::string> do_keys() const noexcept;
    };

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_H_
