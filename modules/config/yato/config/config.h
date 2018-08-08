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

        static constexpr
        int64_t cast_result_(int64_t val) {
            return val;
        }

        static constexpr
        double cast_result_(double val) {
            return val;
        }

        static constexpr
        bool cast_result_(bool val) {
            return val;
        }

        static constexpr
        std::string && cast_result_(std::string && val) {
            return std::move(val);
        }

        static
        config cast_result_(backend_ptr && val) {
            return config(std::move(val));
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
            return m_backend ? m_backend->is_object() : false;
        }

        /**
         * Return true if config is an array, i.e. contains indexed values.
         */
        bool is_array() const {
            return m_backend ? m_backend->is_array() : false;
        }

        /**
         * Return number of entires in the config, if config is an array.
         */
        size_t size() const {
            return m_backend ? m_backend->size() : 0u;
        }

        /**
         * Returns all keys stored in the config object.
         * Order of keys is arbitrary.
         */
        std::vector<std::string> keys() const {
            return m_backend ? m_backend->keys() : std::vector<std::string>{};
        }

        /**
         * Get named value.
         * Valid only if is_object() returned `true`
         */
        template <typename Ty_, typename Converter_ = typename config_value_trait<Ty_>::converter_type>
        yato::optional<Ty_> value(const std::string & name, const Converter_ & converter = Converter_()) const;

        /**
         * Get value of array by index.
         * Valid only if is_array() returned `true`
         */
        template <typename Ty_, typename Converter_ = typename config_value_trait<Ty_>::converter_type>
        yato::optional<Ty_> value(size_t idx, const Converter_ & converter = Converter_()) const;

        /**
         * Get nested object config.
         * Alias for value<config_ptr>
         */
        config object(const std::string & name) const;

        /**
         * Get nested object config.
         * Alias for value<config_ptr>.
         */
        config object(size_t idx) const;

        /**
         * Get nested array config.
         * Alias for value<config_ptr>.
         */
        config array(const std::string & name) const;

        /**
         * Get nested array config.
         * Alias for value<config_ptr>.
         */
        config array(size_t idx) const;

        /**
         * Shortcut for checking boolean flags.
         * Alias for value<bool>(name).get_or(false).
         */
        bool flag(const std::string & name) const;

        /**
         * Shortcut for checking boolean flags.
         * Alias for value<bool>(name).get_or(false).
         */
        bool flag(size_t idx) const;

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


    template <>
    struct config_value_trait<yato::conf::config>
    {
        using converter_type = details::identity_converter<yato::conf::config, yato::conf::config>;
        static constexpr config_type stored_type = config_type::config;
    };


    template <typename Ty_, typename Converter_>
    inline
    yato::optional<Ty_> config::value(const std::string & name, const Converter_ & converter) const
    {
        if(m_backend) {
            using trait = yato::conf::config_value_trait<Ty_>;
            using return_type = typename stored_type_trait<trait::stored_type>::return_type;

            return m_backend->get_by_name(name, trait::stored_type).template get_opt<return_type>().map(
                [&converter](return_type && val){ return converter(cast_result_(std::move(val))); }
            );
        }
        return yato::nullopt_t{};
    }

    template <typename Ty_, typename Converter_>
    inline
    yato::optional<Ty_> config::value(size_t idx, const Converter_ & converter) const
    {
        if(m_backend) {
            using trait = yato::conf::config_value_trait<Ty_>;
            using return_type = typename stored_type_trait<trait::stored_type>::return_type;

            return m_backend->get_by_index(idx, trait::stored_type).template get_opt<return_type>().map(
                [&converter](return_type && val){ return converter(cast_result_(std::move(val))); }
            );
        }
        return yato::nullopt_t{};
    }

    inline
    config config::object(const std::string & name) const
    {
        config conf = value<config>(name).get_or(config{});
        YATO_ENSURES(conf.empty() || conf.is_object());
        return conf;
    }

    inline
    config config::object(size_t idx) const
    {
        config conf = value<config>(idx).get_or(config{});
        YATO_ENSURES(conf.empty() || conf.is_object());
        return conf;
    }

    inline
    config config::array(const std::string & name) const
    {
        config conf = value<config>(name).get_or(config{});
        YATO_ENSURES(conf.empty() || conf.is_array());
        return conf;
    }

    inline
    config config::array(size_t idx) const
    {
        config conf = value<config>(idx).get_or(config{});
        YATO_ENSURES(conf.empty() || conf.is_array());
        return conf;
    }

    inline
    bool config::flag(const std::string & name) const
    {
        return value<bool>(name).get_or(false);
    }

    inline
    bool config::flag(size_t idx) const
    {
        return value<bool>(idx).get_or(false);
    }

} // namespace conf

    // import names
    using conf::config;
    using conf::config_type;
    using conf::config_error;

} // namespace yato

#endif // _YATO_CONFIG_H_
