/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_H_
#define _YATO_CONFIG_H_

#include <iterator>
#include <map>
#include <memory>
#include <vector>

#include "yato/range.h"
#include "config_backend.h"
#include "path.h"

namespace yato {

namespace conf {
    
    class config;

    /**
     * Defines an operands priority in config operations
     */
    enum class priority
    {
        left,
        right
    };

    namespace details {

        struct object_tag_t {};
        struct array_tag_t {};

        struct value_conversions
        {
        public:
            // Applies converter to x-value returned from backend call
            template <typename Ty_, typename Converter_>
            static
            auto apply(Ty_&& val, Converter_&& cvt)
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
            auto map(const yato::optional<Ty_>& opt, Converter_ && cvt)
                -> yato::optional<converted_type<Ty_, Converter_>>
            {
                return opt.map([&cvt](const Ty_ & val) { return apply(val, std::forward<Converter_>(cvt)); });
            }

            template <typename Ty_, typename Converter_>
            static
            auto map(yato::optional<Ty_>&& opt, Converter_ && cvt)
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
             : m_backend(std::move(backend))
        {
            if (m_backend) {
                std::tie(m_key, m_value) = m_backend->find(index);
            }
        }

        config_entry(std::shared_ptr<config_backend> backend, const conf::path& path)
            : m_backend(std::move(backend))
        {
            if (m_backend) {
                resolve_path_(path.tokenize());
            }
        }

        config_entry(const config_entry&) = delete;
        config_entry& operator=(const config_entry&) = delete;

        config_entry(config_entry&& other) noexcept
            : m_backend(std::move(other.m_backend)), m_value(other.m_value)
        {
            other.m_value = nullptr;
        }

        config_entry& operator=(config_entry&& other) noexcept
        {
            YATO_REQUIRES(this != &other);
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

        YATO_ATTR_NODISCARD
        const std::string & key() const
        {
            return m_key;
        }

        YATO_ATTR_NODISCARD
        stored_type type() const
        {
            if (!m_value) {
                throw yato::conf::config_error("config_entry[type]: entry is empty.");
            }
            return m_value->type();
        }

        YATO_ATTR_NODISCARD
        bool is_null() const
        {
            return m_value == nullptr;
        }

        YATO_ATTR_NODISCARD explicit
        operator bool() const
        {
            return m_value != nullptr;
        }

        template <typename Ty_>
        YATO_ATTR_NODISCARD
        yato::optional<Ty_> value() const
        {
            using default_converter = typename config_value_trait<Ty_>::converter_type;
            return value<Ty_>(default_converter{});
        }

        template <typename Ty_, typename Converter_>
        YATO_ATTR_NODISCARD
        yato::optional<Ty_> value(Converter_ && converter) const
        {
            constexpr conf::stored_type fetch_type = conf::config_value_trait<Ty_>::fetch_type;
            static_assert(std::is_same<Ty_, details::value_conversions::converted_stored_type<fetch_type, Converter_>>::value, 
                "Provided converter must return exactly type Ty_");
            return value<fetch_type>(std::forward<Converter_>(converter));
        }

        template <stored_type FetchType_, typename Converter_>
        YATO_ATTR_NODISCARD
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
        YATO_ATTR_NODISCARD
        config object() const;

        /**
         * Alias for value<config>.
         */
        YATO_ATTR_NODISCARD
        config array() const;

        /**
         * Alias for value<bool>(name).get_or(false).
         */
        YATO_ATTR_NODISCARD
        bool flag() const
        {
            return value<bool>().get_or(false);
        }

        /**
         * Returns value implementation
         */
        YATO_ATTR_NODISCARD
        const config_value* value_handle_() const
        {
            return m_value;
        }

    private:

        template <typename Tokenizer_>
        void resolve_path_(Tokenizer_ path_iter)
        {
            if (!path_iter.empty()) {
                while (m_backend) {
                    const auto t = path_iter.next();
                    const std::string name{ t.begin(), t.end() };
                    if (path_iter.has_next()) {
                        // We need to go deeper
                        m_backend = m_backend->get<backend_ptr>(name, stored_type::config).get_or(nullptr);
                    }
                    else {
                        // Fetch a value
                        std::tie(m_key, m_value) = m_backend->find(name);
                        break;
                    }
                }
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

        YATO_ATTR_NODISCARD
        reference operator*() const
        {
            YATO_ASSERT(is_dereferencable_(), "Invalid iterator state.");
            return config_entry(m_backend, m_idx);
        }

        YATO_ATTR_NODISCARD
        pointer operator->() const
        {
            YATO_ASSERT(is_dereferencable_(), "Invalid iterator state.");
            return std::make_unique<config_entry>(m_backend, m_idx);
        }

        YATO_ATTR_NODISCARD
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

        YATO_ATTR_NODISCARD friend
        config_iterator operator+(const config_iterator & it, difference_type d)
        {
            config_iterator copy(it);
            copy += d;
            return copy;
        }

        YATO_ATTR_NODISCARD friend
        config_iterator operator+(difference_type d, const config_iterator & it)
        {
            config_iterator copy(it);
            copy += d;
            return copy;
        }

        YATO_ATTR_NODISCARD friend
        config_iterator operator-(const config_iterator & it, difference_type d)
        {
            config_iterator copy(it);
            copy -= d;
            return copy;
        }

        YATO_ATTR_NODISCARD friend
        config_iterator operator-(difference_type d, const config_iterator & it)
        {
            config_iterator copy(it);
            copy -= d;
            return copy;
        }

        YATO_ATTR_NODISCARD friend
        difference_type operator-(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx - rhs.m_idx;
        }

        YATO_ATTR_NODISCARD friend
        bool operator==(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx == rhs.m_idx;
        }

        YATO_ATTR_NODISCARD friend
        bool operator!=(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx != rhs.m_idx;
        }

        YATO_ATTR_NODISCARD friend
        bool operator<(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx < rhs.m_idx;
        }

        YATO_ATTR_NODISCARD friend
        bool operator>(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx > rhs.m_idx;
        }

        YATO_ATTR_NODISCARD friend
        bool operator<=(const config_iterator & lhs, const config_iterator & rhs)
        {
            YATO_REQUIRES(lhs.m_backend == rhs.m_backend);
            return lhs.m_idx <= rhs.m_idx;
        }

        YATO_ATTR_NODISCARD friend
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

        YATO_ATTR_NODISCARD
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
        YATO_ATTR_NODISCARD
        bool is_null() const
        {
            return !static_cast<bool>(m_backend);
        }

        /**
         * Converts to true if not null
         */
        YATO_ATTR_NODISCARD explicit
        operator bool() const 
        {
            return !is_null();
        }

        /**
         * Return true if config is an object, i.e. contains named values.
         */
        YATO_ATTR_NODISCARD
        bool is_object() const
        {
            return m_backend ? m_backend->is_object() : false;
        }

        /**
         * Return number of entries in the config for both object and array.
         * If config is null, then retuns 0.
         */
        YATO_ATTR_NODISCARD
        size_type size() const
        {
            return m_backend ? m_backend->size() : 0u;
        }

        /**
         * Check is config has no elements, size() equal to 0.
         */
        YATO_ATTR_NODISCARD
        bool empty() const {
            return size() == 0;
        }

        /**
         * Returns all keys stored in the config object.
         * Order of keys is arbitrary.
         */
        YATO_ATTR_NODISCARD
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
        YATO_ATTR_NODISCARD
        yato::optional<Ty_> value(const conf::path& name) const
        {
            return config_entry(m_backend, name).value<Ty_>();
        }

        /**
         * Get value of array by index.
         * Valid for any configs.
         * @return Optional value of type Ty_
         */
        template <typename Ty_>
        YATO_ATTR_NODISCARD
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
        YATO_ATTR_NODISCARD
        yato::optional<Ty_> value(const conf::path& name, Converter_ && converter) const
        {
            return config_entry(m_backend, name).value<Ty_>(std::forward<Converter_>(converter));
        }

        /**
         * Get value of array by index.
         * Valid for any configs.
         * @return Optional value of type Ty_
         */
        template <typename Ty_, typename Converter_>
        YATO_ATTR_NODISCARD
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
        YATO_ATTR_NODISCARD
        auto value(const conf::path & name, Converter_ && converter) const
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
        YATO_ATTR_NODISCARD
        auto value(size_type idx, Converter_ && converter) const
            -> yato::optional<details::value_conversions::converted_stored_type<FetchType_, Converter_>>
        {
            return config_entry(m_backend, idx).value<FetchType_>(std::forward<Converter_>(converter));
        }

        /**
         * Get an entry handle for a key.
         */
        YATO_ATTR_NODISCARD
        config_entry find(const conf::path& name) const
        {
            return config_entry(m_backend, name);
        }

        /**
         * Get an entry handle for a key.
         */
        YATO_ATTR_NODISCARD
        config_entry find(size_type idx) const
        {
            return config_entry(m_backend, idx);
        }

        /**
         * Get nested object config.
         * Alias for value<config>
         */
        YATO_ATTR_NODISCARD
        config object(const conf::path& name) const
        {
            config conf = value<config>(name).get_or(config{});
            YATO_ENSURES(conf.is_null() || conf.is_object())
            return conf;
        }

        /**
         * Get nested object config.
         * Alias for value<config>.
         */
        YATO_ATTR_NODISCARD
        config object(size_type idx) const
        {
            config conf = value<config>(idx).get_or(config{});
            YATO_ENSURES(conf.is_null() || conf.is_object())
            return conf;
        }

        /**
         * Get nested array config.
         * Alias for value<config>.
         */
        YATO_ATTR_NODISCARD
        config array(const conf::path& name) const
        {
            return value<config>(name).get_or(config{});
        }

        /**
         * Get nested array config.
         * Alias for value<config>.
         */
        YATO_ATTR_NODISCARD
        config array(size_type idx) const
        {
            return value<config>(idx).get_or(config{});
        }

        /**
         * Shortcut for checking boolean flags.
         * Alias for value<bool>(name).get_or(false).
         */
        YATO_ATTR_NODISCARD
        bool flag(const conf::path& name) const
        {
            return value<bool>(name).get_or(false);
        }

        /**
         * Shortcut for checking boolean flags.
         * Alias for value<bool>(name).get_or(false).
         */
        YATO_ATTR_NODISCARD
        bool flag(size_type idx) const
        {
            return value<bool>(idx).get_or(false);
        }

        /**
         * Values range start.
         */
        YATO_ATTR_NODISCARD
        iterator begin() const
        {
            return config_iterator(m_backend, 0, size());
        }

        /**
         * Values range end.
         */
        YATO_ATTR_NODISCARD
        iterator end() const
        {
            const auto s = size();
            return config_iterator(m_backend, s, s);
        }

        /**
         * Values range
         */
        YATO_ATTR_NODISCARD
        yato::range<iterator> range() const
        {
            return yato::make_range(begin(), end());
        }

        /**
         * Creates a deep copy of a config, copyng each value from a native backend to the manual backend.
         * Can be used in order to release backend resources attached to the config.
         */
        YATO_ATTR_NODISCARD
        config clone() const;

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, int64_t value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::integer>::return_type>{},
                    yato::narrow_cast<typename stored_type_trait<stored_type::integer>::return_type>(value)));
        }

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, uint64_t value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::integer>::return_type>{},
                    yato::narrow_cast<typename stored_type_trait<stored_type::integer>::return_type>(value)));
        }

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, int32_t value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::integer>::return_type>{},
                    yato::narrow_cast<typename stored_type_trait<stored_type::integer>::return_type>(value)));
        }

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, uint32_t value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::integer>::return_type>{},
                    yato::narrow_cast<typename stored_type_trait<stored_type::integer>::return_type>(value)));
        }

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, int16_t value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::integer>::return_type>{},
                    yato::narrow_cast<typename stored_type_trait<stored_type::integer>::return_type>(value)));
        }

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, uint16_t value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::integer>::return_type>{},
                    yato::narrow_cast<typename stored_type_trait<stored_type::integer>::return_type>(value)));
        }

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, int8_t value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::integer>::return_type>{},
                    yato::narrow_cast<typename stored_type_trait<stored_type::integer>::return_type>(value)));
        }

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, uint8_t value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::integer>::return_type>{},
                    yato::narrow_cast<typename stored_type_trait<stored_type::integer>::return_type>(value)));
        }

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, double value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::real>::return_type>{}, value));
        }

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, float value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::real>::return_type>{}, value));
        }

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, std::string value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::string>::return_type>{}, std::move(value)));
        }

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, bool value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::boolean>::return_type>{}, value));
        }

        /**
         * Creates a copy of config based on this one but with the given path set to the given value.
         * If value is presented already, then it is replaced.
         */
        YATO_ATTR_NODISCARD
        config with_value(const conf::path& path, config value) const
        {
            return with_value_(path, stored_variant(yato::in_place_type_t<typename stored_type_trait<stored_type::config>::return_type>{}, value.backend_handle_()));
        }

        /**
         * Copy config with the given path removed
         */
        YATO_ATTR_NODISCARD
        config without_path(const conf::path& path) const;

        /**
         * Copy only the given path in the config (keeping children, removing siblings)
         */
        YATO_ATTR_NODISCARD
        config with_only_path(const conf::path& path) const;

        /**
         * Joins two configs recursively, returning a config consisting of copy of all left paths and all right paths.
         * If a path is presented in both arguments, then resulting path is chosen accordingly to the given priority.
         * Only object-like configs are merged recursively, all other values are resolved accordingly to priority.
         */
        YATO_ATTR_NODISCARD
        config merged_with(const config& other, priority p = priority::left) const;

        /**
         * Static version of config::merged_with()
         */
        YATO_ATTR_NODISCARD static
        config merge(const config& lhs, const config& rhs, priority p = priority::left)
        {
            return lhs.merged_with(rhs, p);
        }

        /**
         * Copies all paths starting with the given names.
         * Filtering is applied only to the top level, not recursive.
         */
        YATO_ATTR_NODISCARD
        config with_whitelist(std::vector<std::string> names) const;

        /**
         * Copies all paths except starting with the given names.
         * Filtering is applied only to the top level, not recursive.
         */
        YATO_ATTR_NODISCARD
        config with_blacklist(std::vector<std::string> names) const;

        /**
         * Converts a plain config to a vector of stored values casted to Ty_
         */
        template <typename Ty_, typename Alloc_ = std::allocator<Ty_>>
        YATO_ATTR_NODISCARD
        std::vector<Ty_, Alloc_> to_vector() const
        {
            std::vector<Ty_, Alloc_> result;
            const size_t s = size();
            result.reserve(s);
            for (size_t i = 0; i < s; ++i) {
                result.emplace_back(find(i).value<Ty_>().get());
            }
            return result;
        }

        /**
         * Converts a plain config to a map of stored values casted to Ty_
         */
        template <typename Ty_, typename Pr_ = std::less<Ty_>, typename Alloc_ = std::allocator<std::pair<const std::string, Ty_>>>
        YATO_ATTR_NODISCARD
        std::map<std::string, Ty_, Pr_, Alloc_> to_map() const
        {
            if (!is_object()) {
                throw yato::config_error("config[to_map]: Config must be an object");
            }
            std::map<std::string, Ty_, Pr_, Alloc_> result;
            const size_t s = size();
            for (size_t i = 0; i < s; ++i) {
                auto entry = find(i);
                result.emplace(entry.key(), entry.value<Ty_>().get());
            }
            return result;
        }

        /**
         * For internal usage.
         * Use it only if you are sure what you are doing.
         */
        YATO_ATTR_NODISCARD
        const backend_ptr& backend_handle_() const {
            return m_backend;
        }

        /**
         * For internal usage.
         * Use it only if you are sure what you are doing.
         */
        YATO_ATTR_NODISCARD
        backend_ptr& backend_handle_() {
            return m_backend;
        }

    private:
        YATO_ATTR_NODISCARD
        config with_value_(const conf::path& path, stored_variant value) const;

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


} // namespace conf

    // import names
    using conf::config;
    using conf::config_entry;
    using conf::config_iterator;
    using conf::stored_type;
    using conf::config_error;

} // namespace yato

#endif // _YATO_CONFIG_H_
