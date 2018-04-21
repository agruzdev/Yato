/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_H_
#define _YATO_CONFIG_H_

#include <memory>

#include "config_backend.h"

namespace yato {

namespace conf {

    class config
    {
    private:
        backend_ptr m_backend{ nullptr };
        //---------------------------------------------------------------------

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

        /**
         * Assumes that FromType_ is wider than ToType_
         */
        template <typename ToType_, typename FromType_>
        static
        ToType_ cast_result_(details::config_check_bounds_tag, FromType_ && val) {
            YATO_REQUIRES(val <= static_cast<FromType_>(std::numeric_limits<ToType_>::max()));
            YATO_REQUIRES(val >= static_cast<FromType_>(std::numeric_limits<ToType_>::min()));
            return static_cast<ToType_>(val);
        }

        /**
         * Creates config from backend pointer
         */
        template <typename ToType_, typename FromType_>
        static
        ToType_ cast_result_(details::config_create_tag, FromType_ && val) {
            return ToType_(std::forward<FromType_>(val));
        }
        //---------------------------------------------------------------------

    public:
        /**
         * Creates an empty config
         */
        config() = default;

        explicit
        config(backend_ptr backend) 
            : m_backend(std::move(backend))
        { }

        ~config() = default;

        config(const config&) = default;
        config(config&&) noexcept = default;

        config& operator = (const config&) = default;
        config& operator = (config&&) noexcept = default;

        /**
         * Check is config is empty
         */
        bool empty() const {
            return !static_cast<bool>(m_backend);
        }

        operator bool () const {
            return static_cast<bool>(m_backend);
        }

        /**
         * Return true if config is an object, i.e. contains named values.
         */
        bool is_object() const {
            return m_backend ? m_backend->do_is_object() : false;
        }

        /**
         * Return true if config is an array, i.e. contains indexed values.
         */
        bool is_array() const {
            return m_backend ? m_backend->do_is_array() : false;
        }

        /**
         * Return number of entires in the config, if config is an array.
         */
        size_t size() const {
            return m_backend ? m_backend->do_get_size() : 0u;
        }

        /**
         * Get named value.
         * Valid only if is_object() returned `true`
         */
        template <typename Ty_>
        yato::optional<Ty_> value(const std::string & name) const {
            if(m_backend) {
                constexpr config_type type = details::config_choose_stored_type<Ty_>::ctype;
                using return_type = typename details::config_type_trait<type>::return_type;
                using cast_tag = typename details::config_choose_stored_type<Ty_>::cast_tag;

                return m_backend->do_get_by_name(name, type).get_opt<return_type>().map(
                    [](return_type && val){ return cast_result_<Ty_>(cast_tag{}, std::move(val)); }
                );
            }
            return yato::nullopt_t{};
        }

        /**
         * Get value of array by index.
         * Valid only if is_array() returned `true`
         */
        template <typename Ty_>
        yato::optional<Ty_> value(size_t idx) const {
            if(m_backend) {
                constexpr config_type type = details::config_choose_stored_type<Ty_>::ctype;
                using return_type = typename details::config_type_trait<type>::return_type;
                using cast_tag = typename details::config_choose_stored_type<Ty_>::cast_tag;

                return m_backend->do_get_by_index(idx, type).get_opt<return_type>().map(
                    [](return_type && val){ return cast_result_<Ty_>(cast_tag{}, std::move(val)); }
                );
            }
            return yato::nullopt_t{};
        }

        /**
         * Get nested object config.
         * Alias for value<config_ptr>
         */
        config object(const std::string & name) const {
            config conf = value<config>(name).get_or(config{});
            YATO_ENSURES(conf.empty() || conf.is_object());
            return conf;
        }

        /**
         * Get nested object config.
         * Alias for value<config_ptr>.
         */
        config object(size_t idx) const {
            config conf = value<config>(idx).get_or(config{});
            YATO_ENSURES(conf.empty() || conf.is_object());
            return conf;
        }

        /**
         * Get nested array config.
         * Alias for value<config_ptr>.
         */
        config array(const std::string & name) const {
            config conf = value<config>(name).get_or(config{});
            YATO_ENSURES(conf.empty() || conf.is_array());
            return conf;
        }

        /**
         * Get nested array config.
         * Alias for value<config_ptr>.
         */
        config array(size_t idx) const {
            config conf = value<config>(idx).get_or(config{});
            YATO_ENSURES(conf.empty() || conf.is_array());
            return conf;
        }

        /**
         * For internal usage.
         * Use it only if you are sure what you are doing.
         */
        const backend_ptr & get_backend() const {
            return m_backend;
        }

        /**
         * For internal usage.
         * Use it only if you are sure what you are doing.
         */
        backend_ptr & get_backend() {
            return m_backend;
        }
    };

    namespace details
    {

        template <typename Ty_>
        struct config_choose_stored_type <
            Ty_,
            std::enable_if_t<std::is_same<yato::conf::config, Ty_>::value>
        >
        {
            using cast_tag = details::config_create_tag;
            static constexpr config_type ctype = config_type::config;
        };

    }
} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_H_
