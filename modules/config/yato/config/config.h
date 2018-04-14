/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_H_
#define _YATO_CONFIG_H_

#include <cstdint>
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
        struct config_integral_tag {};
        struct config_floating_tag {};
        struct config_string_tag   {};

        struct config_no_cast_tag {};
        struct config_narrow_cast_tag {};
    
        template <typename Ty_, typename = void>
        struct config_choose_stored_type
        { };
    
        template <typename Ty_>
        struct config_choose_stored_type <
            Ty_,
            std::enable_if_t<std::is_integral<Ty_>::value && !std::is_same<Ty_, bool>::value>
        >
        {
            using type = int64_t;
            using tag  = config_integral_tag;
            using cast_tag = config_narrow_cast_tag;
            static constexpr config_type ctype = config_type::integer;
        };
    
        template <typename Ty_>
        struct config_choose_stored_type
        <
            Ty_,
            std::enable_if_t<std::is_floating_point<Ty_>::value>
        >
        {
            using type = double;
            using tag  = config_floating_tag;
            using cast_tag = config_narrow_cast_tag;
            static constexpr config_type ctype = config_type::floating;
        };
    
        template <typename Ty_>
        struct config_choose_stored_type <
            Ty_,
            std::enable_if_t<std::is_same<bool, Ty_>::value>
        >
        {
            using type = bool;
            using cast_tag = config_no_cast_tag;
            static constexpr config_type ctype = config_type::boolean;
        };

        template <typename Ty_>
        struct config_choose_stored_type <
            Ty_,
            std::enable_if_t<std::is_same<std::string, Ty_>::value>
        >
        {
            using type = std::string;
            using tag  = config_string_tag;
            using cast_tag = config_no_cast_tag;
            static constexpr config_type ctype = config_type::string;
        };

        template <typename Ty_>
        struct config_choose_stored_type <
            Ty_,
            std::enable_if_t<std::is_same<yato::conf::config_ptr, Ty_>::value>
        >
        {
            using type = std::string;
            using tag  = config_string_tag;
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


    class config_array;
    class config_object;

    /**
     * Scalar value
     */
    class config_value
    {
    private:
        using value_type = yato::variant<int64_t, double, std::string>;
        value_type m_value;

        template <typename Ty_>
        yato::optional<Ty_> get_impl_(details::config_integral_tag) const noexcept {
            return m_value.get_opt<int64_t>().map([](int64_t val) { return yato::narrow_cast<Ty_>(val); });
        }
    
        template <typename Ty_>
        yato::optional<Ty_> get_impl_(details::config_floating_tag) const noexcept {
            return m_value.get_opt<double>().map([](double val) { return yato::narrow_cast<Ty_>(val); });
        }
    
        template <typename Ty_>
        yato::optional<std::string> get_impl_(details::config_string_tag) const noexcept {
            return m_value.get_opt<std::string>();
        }
    
    public:
        config_value(int8_t val)
            : m_value(static_cast<int64_t>(val))
        { }
    
        config_value(int16_t val)
            : m_value(static_cast<int64_t>(val))
        { }
    
        config_value(int32_t val)
            : m_value(static_cast<int64_t>(val))
        { }
    
        config_value(int64_t val)
            : m_value(val)
        { }
    
        config_value(uint8_t val)
            : m_value(static_cast<int64_t>(val))
        { }
    
        config_value(uint16_t val)
            : m_value(static_cast<int64_t>(val))
        { }
    
        config_value(uint32_t val)
            : m_value(static_cast<int64_t>(val))
        { }
    
        config_value(uint64_t val)
            : m_value(yato::narrow_cast<int64_t>(val))
        { }
    
        config_value(float val)
            : m_value(static_cast<double>(val))
        { }
    
        config_value(double val)
            : m_value(val)
        { }
    
        config_value(const std::string & val)
            : m_value(val)
        { }
    
        config_value(std::string && val)
            : m_value(std::move(val))
        { }

        ~config_value() = default;

        config_value(const config_value&) = default;
        config_value(config_value&&) = default;
    
        config_value& operator = (const config_value&) = default;
        config_value& operator = (config_value&&) = default;

        template <typename Ty_>
        yato::optional<Ty_> get_opt() const noexcept {
            using stored_tag = typename details::config_choose_stored_type<typename std::decay<Ty_>::type>::tag;
            return get_impl_<Ty_>(stored_tag{});
        }
    
        template <typename Ty_>
        Ty_ get() const {
            if(const auto opt = get_opt<Ty_>()){
                return opt.get_unsafe();
            }
            else {
                throw config_error("Invalid config_value access.");
            }
        }
    
        template <typename Ty_, typename Uy_>
        Ty_ get(Uy_ && default_value) const noexcept {
            if(const auto opt = get_opt<Ty_>()){
                return opt.get_unsafe();
            }
            else {
                return static_cast<Ty_>(std::forward<Uy_>(default_value));
            }
        }
    };


    /**
     * Set of named values
     */
    class config_object 
    {
    private:
        virtual void* do_get_underlying_type() noexcept = 0;

        virtual std::vector<std::string> do_get_keys() const noexcept = 0;

        virtual yato::optional<config_value> do_get_value(const std::string & key) const noexcept  = 0;
        virtual const config_object* do_get_object(const std::string & key) const noexcept = 0;
        virtual const config_array*  do_get_array(const std::string & key) const noexcept = 0;
    
    public:
        virtual ~config_object() = default;

        yato::optional<const config_object*> object(const std::string & key) const {
            return yato::make_optional(do_get_object(key));
        }
    
        yato::optional<const config_array*> array(const std::string & key) const {
            return yato::make_optional(do_get_array(key));
        }
    
        template <typename Ty_, typename 
            = yato::void_t<typename details::config_choose_stored_type<Ty_>::type>
        >
        yato::optional<Ty_> value(const std::string & key) const {
            return do_get_value(key).map([](config_value && val){ return val.get_opt<Ty_>(); });
        }

        const void* get_underlying_type() const {
            return const_cast<config_object*>(this)->do_get_underlying_type();
        }

        void* get_underlying_type() {
            return do_get_underlying_type();
        }

        std::vector<std::string> keys() const {
            return do_get_keys();
        }
    };

    /**
     * Array interface
     */
    class config_array 
    {
    private:
        virtual void* do_get_underlying_type() noexcept = 0;

        virtual size_t do_get_size() const noexcept = 0;
        virtual config_value do_get_value(size_t idx) const noexcept  = 0;
        virtual const config_object* do_get_object(size_t idx) const noexcept = 0;
        virtual const config_array*  do_get_array(size_t idx) const noexcept  = 0;

    public:
        virtual ~config_array() = default;
    
        size_t size() const {
            return do_get_size();
        }

        yato::optional<const config_object*> object(size_t idx) const {
            return yato::make_optional(do_get_object(idx));
        }
    
        yato::optional<const config_array*> array(size_t idx) const {
            return yato::make_optional(do_get_array(idx));
        }

        template <typename Ty_, typename 
            = yato::void_t<typename details::config_choose_stored_type<Ty_>::type>
        >
        yato::optional<Ty_> value(size_t idx) const {
            return do_get_value(idx).get_opt<Ty_>();
        }
    
        const void* get_underlying_type() const {
            return const_cast<config_array*>(this)->do_get_underlying_type();
        }
    
        void* get_underlying_type() {
            return do_get_underlying_type();
        }
    };



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

    /**
     * Abstract factory for creating config
     */
    class config_factory
    {
    public:
        virtual ~config_factory() = default;

        virtual config_ptr create(const std::string & json) const = 0;
    };


} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_H_
