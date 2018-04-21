/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_BACKEND_H_
#define _YATO_CONFIG_BACKEND_H_

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

    class config_backend;
    using backend_ptr = std::shared_ptr<config_backend>;

    namespace details
    {
        struct config_no_cast_tag {};
        struct config_narrow_cast_tag {};
        struct config_check_bounds_tag {};
        struct config_create_tag {};

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
            using return_type = yato::conf::backend_ptr;
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

    /**
     * Interface for config implementation
     */
    class config_backend  // NOLINT
    {
    public:
        virtual ~config_backend() = default;

        virtual bool do_is_object() const noexcept = 0;
        virtual details::value_variant do_get_by_name(const std::string & name, config_type type) const noexcept = 0;

        virtual bool do_is_array() const noexcept = 0;
        virtual details::value_variant do_get_by_index(size_t index, config_type type) const noexcept = 0;

        virtual size_t do_get_size() const noexcept = 0;
    };

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_H_
