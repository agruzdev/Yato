/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_H_
#define _YATO_CONFIG_H_

#include <memory>

#include "config_backend.h"
#include "path.h"

namespace yato {

namespace conf {

    namespace details {
        struct object_tag_t {};
        struct array_tag_t {};
    }

    class config final
    {
    private:
        backend_ptr m_backend{ nullptr };
        //---------------------------------------------------------------------

        static constexpr
        int64_t wrap_result_(int64_t val) {
            return val;
        }

        static constexpr
        double wrap_result_(double val) {
            return val;
        }

        static constexpr
        bool wrap_result_(bool val) {
            return val;
        }

        static
        std::string wrap_result_(std::string && val) {
            return std::string(std::move(val));
        }

        static
        config wrap_result_(backend_ptr && val) {
            return config(std::move(val));
        }

        // Applies converter to x-value returned from backend call
        template <typename Ty_, typename Converter_>
        static
        auto apply_convertion_(Ty_ && val, Converter_ && cvt)
        {
            return std::forward<Converter_>(cvt)(wrap_result_(std::forward<Ty_>(val)));
        }

        template <conf::stored_type FetchType_, typename Converter_>
        using converted_result_type = decltype(apply_convertion_(std::declval<typename stored_type_trait<FetchType_>::return_type>(), std::declval<Converter_>()));

        template <conf::stored_type FetchType_, typename Tokenizer_, typename Converter_>
        auto value_(Tokenizer_ & path_tokens, Converter_ && converter) const
            -> yato::optional<converted_result_type<FetchType_, Converter_>>;

        template <conf::stored_type FetchType_, typename Converter_>
        auto value_(size_t index, Converter_ && converter) const
            -> yato::optional<converted_result_type<FetchType_, Converter_>>;

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
         * True if config doesn't exist, i.e. a query hasn't found a nested object.
         * Config may be not null, but empty.
         */
        bool is_null() const
        {
            return !static_cast<bool>(m_backend);
        }

        /**
         * Converts to true if not null
         */
        operator bool() const 
        {
            return !is_null();
        }

        /**
         * Return true if config is an object, i.e. contains named values.
         */
        bool is_object() const
        {
            return m_backend ? m_backend->is_object() : false;
        }

        /**
         * Return number of entries in the config for both object and array.
         * If config is null, then retuns 0.
         */
        size_t size() const
        {
            return m_backend ? m_backend->size() : 0u;
        }

        /**
         * Check is config has no elements, size() equal to 0.
         */
        bool empty() const {
            return size() == 0;
        }

        /**
         * Returns all keys stored in the config object.
         * Order of keys is arbitrary.
         */
        std::vector<std::string> keys() const
        {
            return m_backend ? m_backend->keys() : std::vector<std::string>{};
        }

        /**
         * Get named value.
         * Valid only if is_object() returned `true`
         * @return Optional value of type Ty_
         */
        template <typename Ty_>
        yato::optional<Ty_> value(const conf::path & name) const
        {
            constexpr conf::stored_type fetch_type = conf::config_value_trait<Ty_>::fetch_type;
            using default_converter = typename config_value_trait<Ty_>::converter_type;
            static_assert(std::is_same<Ty_, converted_result_type<fetch_type, default_converter>>::value, "Provided converter must return exactly type Ty_");
            auto path_tokens = name.tokenize();
            return value_<fetch_type>(path_tokens, default_converter{});
        }
        
        /**
         * Get named value.
         * Valid only if is_object() returned `true`
         * @return Optional value of type Ty_
         */
        template <typename Ty_>
        yato::optional<Ty_> value(const std::string & name) const
        {
            constexpr conf::stored_type fetch_type = conf::config_value_trait<Ty_>::fetch_type;
            using default_converter = typename config_value_trait<Ty_>::converter_type;
            static_assert(std::is_same<Ty_, converted_result_type<fetch_type, default_converter>>::value, "Provided converter must return exactly type Ty_");
            auto path_tokens = yato::tokenize(name, conf::path_separator<char>(), conf::path::skips_empty_names);
            return value_<fetch_type>(path_tokens, default_converter{});
        }

        /**
         * Get value of array by index.
         * Valid for any configs.
         * @return Optional value of type Ty_
         */
        template <typename Ty_>
        yato::optional<Ty_> value(size_t idx) const
        {
            constexpr conf::stored_type fetch_type = conf::config_value_trait<Ty_>::fetch_type;
            using default_converter = typename config_value_trait<Ty_>::converter_type;
            static_assert(std::is_same<Ty_, converted_result_type<fetch_type, default_converter>>::value, "Provided converter must return exactly type Ty_");
            return value_<fetch_type>(idx, default_converter{});
        }

        /**
         * Get named value.
         * Valid only if is_object() returned `true`
         * @return Optional value of type Ty_
         */
        template <typename Ty_, typename Converter_>
        yato::optional<Ty_> value(const conf::path & name, Converter_ && converter) const
        {
            constexpr conf::stored_type fetch_type = conf::config_value_trait<Ty_>::fetch_type;
            static_assert(std::is_same<Ty_, converted_result_type<fetch_type, Converter_>>::value, "Provided converter must return exactly type Ty_");
            auto path_tokens = name.tokenize();
            return value_<fetch_type>(path_tokens, std::forward<Converter_>(converter));
        }

        /**
         * Get named value.
         * Valid only if is_object() returned `true`
         * @return Optional value of type Ty_
         */
        template <typename Ty_, typename Converter_>
        yato::optional<Ty_> value(const std::string & name, Converter_ && converter) const
        {
            constexpr conf::stored_type fetch_type = conf::config_value_trait<Ty_>::fetch_type;
            static_assert(std::is_same<Ty_, converted_result_type<fetch_type, Converter_>>::value, "Provided converter must return exactly type Ty_");
            auto path_tokens = yato::tokenize(name, conf::path_separator<char>(), conf::path::skips_empty_names);
            return value_<fetch_type>(path_tokens, std::forward<Converter_>(converter));
        }

        /**
         * Get value of array by index.
         * Valid for any configs.
         * @return Optional value of type Ty_
         */
        template <typename Ty_, typename Converter_>
        yato::optional<Ty_> value(size_t idx, Converter_ && converter) const
        {
            constexpr conf::stored_type fetch_type = conf::config_value_trait<Ty_>::fetch_type;
            static_assert(std::is_same<Ty_, converted_result_type<fetch_type, Converter_>>::value, "Provided converter must return exactly type Ty_");
            return value_<fetch_type>(idx, std::forward<Converter_>(converter));
        }

        /**
         * Get a named value obtained with a provided converter.
         * Valid only if is_object() returned `true`
         * @return Optional value returned from the converter
         */
        template <conf::stored_type FetchType_, typename Converter_>
        auto value(const conf::path & name, Converter_ && converter) const
            -> yato::optional<converted_result_type<FetchType_, Converter_>>
        {
            auto path_tokens = name.tokenize();
            return value_<FetchType_>(path_tokens, std::forward<Converter_>(converter));
        }

        /**
         * Get a named value obtained with a provided converter.
         * Valid only if is_object() returned `true`
         * @return Optional value returned from the converter
         */
        template <conf::stored_type FetchType_, typename Converter_ >
        auto value(const std::string & name, Converter_ && converter) const
            -> yato::optional<converted_result_type<FetchType_, Converter_>>
        {
            auto path_tokens = yato::tokenize(name, conf::path_separator<char>(), conf::path::skips_empty_names);
            return value_<FetchType_>(path_tokens, std::forward<Converter_>(converter));
        }

        /**
         * Get a value, obtained with a provided converter, by index.
         * Valid for any configs.
         * @return Optional value returned from the converter
         */
        template <conf::stored_type FetchType_, typename Converter_ >
        auto value(size_t idx, Converter_ && converter) const
            -> yato::optional<converted_result_type<FetchType_, Converter_>>
        {
            return value_<FetchType_>(idx, std::forward<Converter_>(converter));
        }

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
        static constexpr stored_type fetch_type = stored_type::config;
    };

    template <conf::stored_type FetchType_, typename Tokenizer_, typename Converter_>
    inline
    auto config::value_(Tokenizer_ & path_tokens, Converter_ && converter) const
        -> yato::optional<converted_result_type<FetchType_, Converter_>>
    {
        if (!path_tokens.empty()) {
            backend_ptr current_backend = m_backend;
            while (current_backend) {
                const auto t = path_tokens.next();
                const std::string name{ t.begin(), t.end() };
                if (path_tokens.has_next()) {
                    // We need to go deeper
                    current_backend = current_backend->get<backend_ptr>(name, stored_type::config).get_or(nullptr);
                }
                else {
                    // Fetch a value
                    using return_type = typename stored_type_trait<FetchType_>::return_type;
                    return current_backend->get<return_type>(name, FetchType_).map(
                        [&converter](return_type && val) { return apply_convertion_(std::move(val), std::forward<Converter_>(converter)); }
                    );
                }
            }
        }
        return yato::nullopt_t{};
    }

    template <conf::stored_type FetchType_, typename Converter_>
    inline
    auto config::value_(size_t index, Converter_ && converter) const
        -> yato::optional<converted_result_type<FetchType_, Converter_>>
    {
        if (m_backend) {
            using return_type = typename stored_type_trait<FetchType_>::return_type;
            return m_backend->get<return_type>(index, FetchType_).map(
                [&converter](return_type && val){ return apply_convertion_(std::move(val), std::forward<Converter_>(converter)); }
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
        return value<config>(name).get_or(config{});
    }

    inline
    config config::array(size_t idx) const
    {
        return value<config>(idx).get_or(config{});
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
    using conf::stored_type;
    using conf::config_error;

} // namespace yato

#endif // _YATO_CONFIG_H_
