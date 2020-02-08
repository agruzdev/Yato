/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_H_
#define _YATO_CONFIG_H_

#include <iterator>
#include <memory>

#include "yato/range.h"

#include "config_backend.h"
#include "path.h"

namespace yato {

namespace conf {
    
    class config;

    namespace details {

        struct object_tag_t {};
        struct array_tag_t {};

        struct value_conversions
        {
        public:
            // Applies converter to x-value returned from backend call
            template <typename Ty_, typename Converter_>
            static
            auto apply(Ty_ && val, Converter_ && cvt)
            {
                return std::forward<Converter_>(cvt)(wrap_result_(std::forward<Ty_>(val)));
            }

            // Workaround for vc15
            template <typename Ty_, typename Converter_>
            struct converted_type_impl_
            {
                using type = decltype(apply(std::declval<Ty_>(), std::declval<Converter_>()));
            };

            template <typename Ty_, typename Converter_>
            using converted_type = typename converted_type_impl_<Ty_, Converter_>::type;

            template <conf::stored_type FetchType_, typename Converter_>
            using converted_stored_type = typename converted_type_impl_<typename stored_type_trait<FetchType_>::return_type, Converter_>::type;


            template <typename Ty_, typename Converter_>
            static
            auto map(const yato::optional<Ty_> & opt, Converter_ && cvt)
                -> yato::optional<converted_type<Ty_, Converter_>>
            {
                return opt.map([&cvt](const Ty_ & val) { return apply(val, std::forward<Converter_>(cvt)); });
            }

            template <typename Ty_, typename Converter_>
            static
            auto map(yato::optional<Ty_> && opt, Converter_ && cvt)
                -> yato::optional<converted_type<Ty_, Converter_>>
            {
                return std::move(opt).map([&cvt](Ty_ && val) { return apply(std::move(val), std::forward<Converter_>(cvt)); });
            }

        private:
            static
            int64_t wrap_result_(int64_t val) {
                return val;
            }

            static
            double wrap_result_(double val) {
                return val;
            }

            static
            bool wrap_result_(bool val) {
                return val;
            }

            static
            std::string wrap_result_(const std::string & val) {
                return val;
            }

            static
            std::string wrap_result_(std::string && val) {
                return std::string(std::move(val));
            }

            static
            config wrap_result_(const backend_ptr & val);

            static
            config wrap_result_(backend_ptr && val);
        };
    }

    /**
     * Config entry descriptor.
     * in the case of array-like configs key is always empty.
     */
    class config_entry
    {
    public:
        config_entry(std::shared_ptr<config_backend> backend, size_t index)
             : m_backend(backend)
        {
            if (m_backend) {
                std::tie(m_key, m_value) = m_backend->find(index);
            }
        }

        config_entry(std::shared_ptr<config_backend> backend, const conf::path & path)
            : m_backend(backend)
        {
            auto path_tokens = path.tokenize();
            resolve_path_(path_tokens, m_backend);
        }

        config_entry(std::shared_ptr<config_backend> backend, const std::string & name)
             : m_backend(backend)
        {
            auto path_tokens = yato::tokenize(name, conf::path_separator<char>(), conf::path::skips_empty_names);
            resolve_path_(path_tokens, m_backend);
        }

        config_entry(const config_entry&) = delete;
        config_entry& operator=(const config_entry&) = delete;

        config_entry(config_entry && other) noexcept
            : m_backend(std::move(other.m_backend)), m_value(other.m_value)
        {
            other.m_value = nullptr;
        }

        config_entry& operator=(config_entry && other) noexcept
        {
            if (m_backend && m_value) {
                m_backend->release(m_value);
            }
            m_backend = std::move(other.m_backend);
            m_value   = other.m_value;
            other.m_value = nullptr;
            return *this;
        }

        ~config_entry()
        {
            if (m_backend && m_value) {
                m_backend->release(m_value);
            }
        }

        const std::string & key() const
        {
            return m_key;
        }

        stored_type type() const
        {
            if (!m_value) {
                throw yato::conf::config_error("config_entry[type]: entry is empty.");
            }
            return m_value->type();
        }

        bool is_null() const
        {
            return m_value == nullptr;
        }

        operator bool() const
        {
            return m_value != nullptr;
        }

        template <typename Ty_>
        yato::optional<Ty_> value() const
        {
            using default_converter = typename config_value_trait<Ty_>::converter_type;
            return value<Ty_>(default_converter{});
        }

        template <typename Ty_, typename Converter_>
        yato::optional<Ty_> value(Converter_ && converter) const
        {
            constexpr conf::stored_type fetch_type = conf::config_value_trait<Ty_>::fetch_type;
            static_assert(std::is_same<Ty_, details::value_conversions::converted_stored_type<fetch_type, Converter_>>::value, 
                "Provided converter must return exactly type Ty_");
            return value<fetch_type>(std::forward<Converter_>(converter));
        }

        template <stored_type FetchType_, typename Converter_>
        auto value(Converter_ && converter) const
            -> yato::optional<details::value_conversions::converted_stored_type<FetchType_, Converter_>>
        {
            if (!m_value) {
                return yato::nullopt_t{};
            }
            using return_type = typename stored_type_trait<FetchType_>::return_type;
            return details::value_conversions::map<return_type>(m_value->get<return_type>(FetchType_), std::forward<Converter_>(converter));
        }

        /**
         * Alias for value<config>
         */
        config object() const;

        /**
         * Alias for value<config>.
         */
        config array() const;

        /**
         * Alias for value<bool>(name).get_or(false).
         */
        bool flag() const;

    private:

        template <typename Tokenizer_>
        bool resolve_path_impl_(Tokenizer_ path_tokens, backend_ptr backend)
        {
            assert(!path_tokens.empty());
            const auto t = path_tokens.next();
            const std::string name{ t.begin(), t.end() };
            if (path_tokens.has_next()) {
                // We need to go deeper
                config_value* value_iter = backend->find(name).second;
                if (value_iter) {
                    yato_finally(( [&]{ backend->release(value_iter); } ));
                    do {
                        auto next_backend = value_iter->get<backend_ptr>(stored_type::config);
                        if (next_backend) {
                            if (resolve_path_impl_(path_tokens, std::move(next_backend.get()))) {
                                return true;
                            }
                        }
                    } while(value_iter->next());
                }
            }
            else {
                // Fetch a value
                m_backend = backend;
                std::tie(m_key, m_value) = backend->find(name);
                return (m_value != nullptr);
            }
            return false;
        }

        template <typename Tokenizer_>
        void resolve_path_(const Tokenizer_& path_tokens, backend_ptr root)
        {
            if (!path_tokens.empty() && root) {
                resolve_path_impl_(path_tokens, std::move(root));
            }
        }

        std::shared_ptr<config_backend> m_backend;
        std::string m_key;
        const config_value* m_value = nullptr;
    };

    /**
     * Iterates config values.
     * Provides random access. Always const.
     */
    class config_iterator
    {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = config_entry;
        using pointer    = std::unique_ptr<config_entry>;
        using reference  = config_entry;
        using iterator_category = std::random_access_iterator_tag;

        using index_type = std::ptrdiff_t; // In order to support position before begin

        config_iterator(const config_iterator&) = default;
        config_iterator(config_iterator&&) = default;

        config_iterator& operator=(const config_iterator&) = default;
        config_iterator& operator=(config_iterator&&) = default;

        ~config_iterator() = default;

        reference operator*() const
        {
            YATO_ASSERT(is_dereferencable_(), "Invalid iterator state.");
            return config_entry(m_backend, m_idx);
        }

        pointer operator->() const
        {
            YATO_ASSERT(is_dereferencable_(), "Invalid iterator state.");
            return std::make_unique<config_entry>(m_backend, m_idx);
        }

        bool has_next() const
        {
            return m_idx < m_size;
        }

        config_entry next()
        {
            YATO_REQUIRES(has_next());
            return config_entry(m_backend, m_idx++);
        }

        config_iterator & operator++()
        {
            YATO_REQUIRES(m_idx < std::numeric_limits<index_type>::max());
            ++m_idx;
            return *this;
        }

        config_iterator operator++(int)
        {
            YATO_REQUIRES(m_idx < std::numeric_limits<index_type>::max());
            auto copy{*this};
            ++(*this);
            return copy;
        }

        config_iterator & operator--()
        {
            YATO_REQUIRES(m_idx > std::numeric_limits<index_type>::min());
            --m_idx;
            return *this;
        }

        config_iterator operator--(int)
        {
            YATO_REQUIRES(m_idx > std::numeric_limits<index_type>::min());
            auto copy{*this};
            --(*this);
            return copy;
        }

        config_iterator & operator+=(difference_type d)
        {
            YATO_REQUIRES(m_idx <= std::numeric_limits<index_type>::max() - d);
            m_idx += d;
            return *this;
        }

        config_iterator & operator-=(difference_type d)
        {
            YATO_REQUIRES(m_idx >= std::numeric_limits<index_type>::min() + d);
            m_idx -= d;
            return *this;
        }

        friend
        config_iterator operator+(const config_iterator & it, difference_type d)
        {
            config_iterator copy(it);
            copy += d;
            return copy;
        }

        friend
        config_iterator operator+(difference_type d, const config_iterator & it)
        {
            config_iterator copy(it);
            copy += d;
            return copy;
        }

        friend
        config_iterator operator-(const config_iterator & it, difference_type d)
        {
            config_iterator copy(it);
            copy -= d;
            return copy;
        }

        friend
        config_iterator operator-(difference_type d, const config_iterator & it)
        {
            config_iterator copy(it);
            copy -= d;
            return copy;
        }

        friend
        difference_type operator-(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx - rhs.m_idx;
        }

        friend
        bool operator==(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx == rhs.m_idx;
        }

        friend
        bool operator!=(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx != rhs.m_idx;
        }

        friend
        bool operator<(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx < rhs.m_idx;
        }

        friend
        bool operator>(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx > rhs.m_idx;
        }

        friend
        bool operator<=(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx <= rhs.m_idx;
        }

        friend
        bool operator>=(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx >= rhs.m_idx;
        }

    private:
        config_iterator(backend_ptr backend, size_t pos, size_t size)
            : m_backend(std::move(backend))
            , m_idx(yato::narrow_cast<index_type>(pos))
            , m_size(yato::narrow_cast<index_type>(size))
        { }

        bool is_dereferencable_() const
        {
            return m_backend && (m_idx >= 0) && (m_idx < m_size);
        }

        backend_ptr m_backend;
        index_type m_idx;
        index_type m_size;

        friend class config;
    };


    class config final
    {
    public:
        using size_type = size_t;
        using iterator  = config_iterator;
        using const_iterator = config_iterator; // config iterator is always const


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
        size_type size() const
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
            return config_entry(m_backend, name).value<Ty_>();
        }
        
        /**
         * Get named value.
         * Valid only if is_object() returned `true`
         * @return Optional value of type Ty_
         */
        template <typename Ty_>
        yato::optional<Ty_> value(const std::string & name) const
        {
            return config_entry(m_backend, name).value<Ty_>();
        }

        /**
         * Get value of array by index.
         * Valid for any configs.
         * @return Optional value of type Ty_
         */
        template <typename Ty_>
        yato::optional<Ty_> value(size_type idx) const
        {
            return config_entry(m_backend, idx).value<Ty_>();
        }

        /**
         * Get named value.
         * Valid only if is_object() returned `true`
         * @return Optional value of type Ty_
         */
        template <typename Ty_, typename Converter_>
        yato::optional<Ty_> value(const conf::path & name, Converter_ && converter) const
        {
            return config_entry(m_backend, name).value<Ty_>(std::forward<Converter_>(converter));
        }

        /**
         * Get named value.
         * Valid only if is_object() returned `true`
         * @return Optional value of type Ty_
         */
        template <typename Ty_, typename Converter_>
        yato::optional<Ty_> value(const std::string & name, Converter_ && converter) const
        {
            return config_entry(m_backend, name).value<Ty_>(std::forward<Converter_>(converter));
        }

        /**
         * Get value of array by index.
         * Valid for any configs.
         * @return Optional value of type Ty_
         */
        template <typename Ty_, typename Converter_>
        yato::optional<Ty_> value(size_type idx, Converter_ && converter) const
        {
            return config_entry(m_backend, idx).value<Ty_>(std::forward<Converter_>(converter));
        }

        /**
         * Get a named value obtained with a provided converter.
         * Valid only if is_object() returned `true`
         * @return Optional value returned from the converter
         */
        template <conf::stored_type FetchType_, typename Converter_>
        auto value(const conf::path & name, Converter_ && converter) const
            -> yato::optional<details::value_conversions::converted_stored_type<FetchType_, Converter_>>
        {
            return config_entry(m_backend, name).value<FetchType_>(std::forward<Converter_>(converter));
        }

        /**
         * Get a named value obtained with a provided converter.
         * Valid only if is_object() returned `true`
         * @return Optional value returned from the converter
         */
        template <conf::stored_type FetchType_, typename Converter_ >
        auto value(const std::string & name, Converter_ && converter) const
            -> yato::optional<details::value_conversions::converted_stored_type<FetchType_, Converter_>>
        {
            return config_entry(m_backend, name).value<FetchType_>(std::forward<Converter_>(converter));
        }

        /**
         * Get a value, obtained with a provided converter, by index.
         * Valid for any configs.
         * @return Optional value returned from the converter
         */
        template <conf::stored_type FetchType_, typename Converter_ >
        auto value(size_type idx, Converter_ && converter) const
            -> yato::optional<details::value_conversions::converted_stored_type<FetchType_, Converter_>>
        {
            return config_entry(m_backend, idx).value<FetchType_>(std::forward<Converter_>(converter));
        }

        /**
         * Get an entry handle for a key.
         */
        config_entry find(const std::string & name) const
        {
            return config_entry(m_backend, name);
        }

        /**
         * Get an entry handle for a key.
         */
        config_entry find(size_type idx) const
        {
            return config_entry(m_backend, idx);
        }

        /**
         * Get nested object config.
         * Alias for value<config>
         */
        config object(const std::string & name) const;

        /**
         * Get nested object config.
         * Alias for value<config>
         */
        config object(const conf::path & name) const;

        /**
         * Get nested object config.
         * Alias for value<config>.
         */
        config object(size_type idx) const;

        /**
         * Get nested array config.
         * Alias for value<config>.
         */
        config array(const std::string & name) const;

        /**
         * Get nested array config.
         * Alias for value<config>.
         */
        config array(const conf::path & name) const;

        /**
         * Get nested array config.
         * Alias for value<config>.
         */
        config array(size_type idx) const;

        /**
         * Shortcut for checking boolean flags.
         * Alias for value<bool>(name).get_or(false).
         */
        bool flag(const std::string & name) const;

        /**
         * Shortcut for checking boolean flags.
         * Alias for value<bool>(name).get_or(false).
         */
        bool flag(const conf::path & name) const;

        /**
         * Shortcut for checking boolean flags.
         * Alias for value<bool>(name).get_or(false).
         */
        bool flag(size_type idx) const;

        /**
         * Values range start.
         */
        iterator begin() const
        {
            return config_iterator(m_backend, 0, size());
        }

        /**
         * Values range end.
         */
        iterator end() const
        {
            const auto s = size();
            return config_iterator(m_backend, s, s);
        }

        /**
         * Values range
         */
        yato::range<iterator> range() const
        {
            return yato::make_range(begin(), end());
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


        //---------------------------------------------------------------------
        backend_ptr m_backend{ nullptr };
    };


    template <>
    struct config_value_trait<yato::conf::config>
    {
        using converter_type = details::identity_converter<yato::conf::config, yato::conf::config>;
        static constexpr stored_type fetch_type = stored_type::config;
    };

    inline
    config details::value_conversions::wrap_result_(const backend_ptr & val) {
        return config(val);
    }

    inline
    config details::value_conversions::wrap_result_(backend_ptr && val) {
        return config(std::move(val));
    }


    inline
    config config_entry::object() const
    {
        config conf = value<config>().get_or(config{});
        YATO_ENSURES(conf.is_null() || conf.is_object());
        return conf;
    }

    inline
    config config_entry::array() const
    {
        return value<config>().get_or(config{});
    }

    inline
    bool config_entry::flag() const
    {
        return value<bool>().get_or(false);
    }

    inline
    config config::object(const std::string & name) const
    {
        config conf = value<config>(name).get_or(config{});
        YATO_ENSURES(conf.is_null() || conf.is_object());
        return conf;
    }

    inline
    config config::object(const conf::path & name) const
    {
        config conf = value<config>(name).get_or(config{});
        YATO_ENSURES(conf.is_null() || conf.is_object());
        return conf;
    }

    inline
    config config::object(size_type idx) const
    {
        config conf = value<config>(idx).get_or(config{});
        YATO_ENSURES(conf.is_null() || conf.is_object());
        return conf;
    }

    inline
    config config::array(const std::string & name) const
    {
        return value<config>(name).get_or(config{});
    }

    inline
    config config::array(const conf::path & name) const
    {
        return value<config>(name).get_or(config{});
    }

    inline
    config config::array(size_type idx) const
    {
        return value<config>(idx).get_or(config{});
    }

    inline
    bool config::flag(const std::string & name) const
    {
        return value<bool>(name).get_or(false);
    }

    inline
    bool config::flag(const conf::path & name) const
    {
        return value<bool>(name).get_or(false);
    }

    inline
    bool config::flag(size_type idx) const
    {
        return value<bool>(idx).get_or(false);
    }


} // namespace conf

    // import names
    using conf::config;
    using conf::config_entry;
    using conf::config_iterator;
    using conf::stored_type;
    using conf::config_error;

} // namespace yato

#endif // _YATO_CONFIG_H_
