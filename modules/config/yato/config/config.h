/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_H_
#define _YATO_CONFIG_H_

#include <cstdint>
#include <limits>
#include <string>

#include <yato/optional.h>
#include <yato/variant.h>
#include <yato/assert.h>
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

    enum class config_type
    {
        integer,
        floating,
        boolean,
        string,
        config
    };

    class basic_config;
    using config_ptr = std::shared_ptr<const basic_config>;

    namespace details
    {
        struct config_no_cast_tag {};
        struct config_narrow_cast_tag {};
        struct config_check_bounds_tag {};

        template <typename Ty_, typename = void>
        struct config_choose_stored_type
        { };
    
        template <typename Ty_>
        struct config_choose_stored_type <
            Ty_,
            std::enable_if_t<std::is_integral<Ty_>::value && !std::is_same<Ty_, bool>::value>
        >
        {
            using cast_tag = config_narrow_cast_tag;
            static constexpr config_type ctype = config_type::integer;
        };
    
        template <typename Ty_>
        struct config_choose_stored_type <
            Ty_,
            std::enable_if_t<std::is_floating_point<Ty_>::value>
        >
        {
            using cast_tag = config_check_bounds_tag;
            static constexpr config_type ctype = config_type::floating;
        };
    
        template <typename Ty_>
        struct config_choose_stored_type <
            Ty_,
            std::enable_if_t<std::is_same<bool, Ty_>::value>
        >
        {
            using cast_tag = config_no_cast_tag;
            static constexpr config_type ctype = config_type::boolean;
        };

        template <typename Ty_>
        struct config_choose_stored_type <
            Ty_,
            std::enable_if_t<std::is_same<std::string, Ty_>::value>
        >
        {
            using cast_tag = config_no_cast_tag;
            static constexpr config_type ctype = config_type::string;
        };

        template <typename Ty_>
        struct config_choose_stored_type <
            Ty_,
            std::enable_if_t<std::is_same<yato::conf::config_ptr, Ty_>::value>
        >
        {
            using cast_tag = config_no_cast_tag;
            static constexpr config_type ctype = config_type::config;
        };

        template <config_type Type_>
        struct config_type_trait
        { };

        template <>
        struct config_type_trait <
            config_type::integer
        >
        {
            using return_type = int64_t;
        };

        template <>
        struct config_type_trait <
            config_type::floating
        >
        {
            using return_type = double;
        };

        template <>
        struct config_type_trait <
            config_type::boolean
        >
        {
            using return_type = bool;
        };

        template <>
        struct config_type_trait <
            config_type::string
        >
        {
            using return_type = std::string;
        };

        template <>
        struct config_type_trait <
            config_type::config
        >
        {
            using return_type = yato::conf::config_ptr;
        };


        using value_variant = yato::variant<
            void,
            typename config_type_trait<config_type::integer>::return_type,
            typename config_type_trait<config_type::floating>::return_type,
            typename config_type_trait<config_type::boolean>::return_type,
            typename config_type_trait<config_type::string>::return_type,
            typename config_type_trait<config_type::config>::return_type
        >;
    }



    class basic_config
    {
    private:
        virtual bool do_is_object() const noexcept = 0;
        virtual details::value_variant do_get_by_name(const std::string & name, config_type type) const noexcept = 0;

        virtual bool do_is_array() const noexcept = 0;
        virtual details::value_variant do_get_by_index(size_t index, config_type type) const noexcept = 0;

        virtual size_t do_get_size() const noexcept = 0;

        template <typename ToType_, typename FromType_>
        static
        ToType_ cast_result_(details::config_no_cast_tag, FromType_ && val) {
            return std::forward<FromType_>(val);
        }

        template <typename ToType_, typename FromType_>
        static
        ToType_ cast_result_(details::config_narrow_cast_tag, FromType_ && val) {
            return yato::narrow_cast<ToType_>(std::forward<FromType_>(val));
        }

        template <typename ToType_, typename FromType_>
        static
        ToType_ cast_result_(details::config_check_bounds_tag, FromType_ && val) {
            YATO_REQUIRES(static_cast<ToType_>(val) <= std::numeric_limits<ToType_>::max());
            YATO_REQUIRES(static_cast<ToType_>(val) >= std::numeric_limits<ToType_>::min());
            return static_cast<ToType_>(val);
        }

    public:
        basic_config() = default;
        virtual ~basic_config() = default;

        /**
         * Return true if config is an object, i.e. contains named values.
         */
        bool is_object() const {
            return do_is_object();
        }

        /**
         * Return true if config is an array, i.e. contains indexed values.
         */
        bool is_array() const {
            return do_is_array();
        }

        /**
         * Return number of entires in the config, if config is an array.
         */
        size_t size() const {
            return do_get_size();
        }

        /**
         * Get named value.
         * Valid only if is_object() returned `true`
         */
        template <typename Ty_>
        yato::optional<Ty_> value(const std::string & name) const {
            constexpr config_type type = details::config_choose_stored_type<Ty_>::ctype;
            using return_type = typename details::config_type_trait<type>::return_type;
            using cast_tag = typename details::config_choose_stored_type<Ty_>::cast_tag;

            return do_get_by_name(name, type).get_opt<return_type>().map(
                [](return_type && val){ return cast_result_<Ty_>(cast_tag{}, std::move(val)); }
            );
        }

        /**
         * Get value of array by index.
         * Valid only if is_array() returned `true`
         */
        template <typename Ty_>
        yato::optional<Ty_> value(size_t idx) const {
            constexpr config_type type = details::config_choose_stored_type<Ty_>::ctype;
            using return_type = typename details::config_type_trait<type>::return_type;
            using cast_tag = typename details::config_choose_stored_type<Ty_>::cast_tag;

            return do_get_by_index(idx, type).get_opt<return_type>().map(
                [](return_type && val){ return cast_result_<Ty_>(cast_tag{}, std::move(val)); }
            );
        }

        /**
         * Alias for value<config_ptr>
         */
        config_ptr config(const std::string & name) const {
            return value<config_ptr>(name).get_or(nullptr);
        }

        /**
         * Alias for value<config_ptr>
         */
        config_ptr config(size_t idx) const {
            return value<config_ptr>(idx).get_or(nullptr);
        }

        /**
         * Alias for value<config_ptr>
         */
        config_ptr array(const std::string & name) const {
            return value<config_ptr>(name).get_or(nullptr);
        }

        /**
         * Alias for value<config_ptr>
         */
        config_ptr array(size_t idx) const {
            return value<config_ptr>(idx).get_or(nullptr);
        }
    };


} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_H_
