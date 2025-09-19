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


        inline
        int64_t wrap_result_(int64_t val) {
            return val;
        }

        inline
        double wrap_result_(double val) {
            return val;
        }

        inline
        bool wrap_result_(bool val) {
            return val;
        }

        inline
        const std::string& wrap_result_(const std::string& val) {
            return val;
        }

        inline
        std::string wrap_result_(std::string&& val) {
            return std::string(std::move(val));
        }

        inline
        config wrap_result_(const backend_ptr_t& val);

        inline
        config wrap_result_(backend_ptr_t&& val);



        template <typename Converter_, typename Arg_, typename = void>
        struct invoke_load_impl
        { };

        template <typename Converter_, typename Arg_>
        struct invoke_load_impl<Converter_, Arg_,
                yato::test_type_t<decltype(std::declval<Converter_>()(std::declval<Arg_>()))>
            >
        {
            // fallback to old operator()
            using load_result_type = decltype(std::declval<Converter_>()(std::declval<Arg_>()));

            static
            load_result_type apply(const Converter_& cvt, const Arg_& val)
            {
                return cvt(val);
            }
        };

        template <typename Converter_, typename Arg_>
        struct invoke_load_impl<Converter_, Arg_,
                yato::test_type_t<decltype(std::declval<Converter_>().load(std::declval<Arg_>()))>
            >
        {
            using load_result_type = decltype(std::declval<Converter_>().load(std::declval<Arg_>()));

            static
            load_result_type apply(const Converter_& cvt, const Arg_& val)
            {
                return cvt.load(val);
            }
        };

        template <typename Ty_, typename Converter_>
        using converted_type = typename details::invoke_load_impl<Converter_, decltype(wrap_result_(std::declval<Ty_>()))>::load_result_type;

        template <conf::stored_type FetchType_, typename Converter_>
        using converted_stored_type = converted_type<typename stored_type_trait<FetchType_>::return_type, Converter_>;



        template <typename Converter_, typename Arg_, typename = void>
        struct invoke_skip_impl
        {
            static
            bool apply(const Converter_& /*cvt*/, const Arg_& /*val*/)
            {
                return false;
            }
        };


        template <typename Converter_, typename Arg_>
        struct invoke_skip_impl<Converter_, Arg_,
                yato::test_type_t<decltype(std::declval<Converter_>().skip(std::declval<Arg_>()))>
            >
        {
            static
            bool apply(const Converter_& cvt, const Arg_& val)
            {
                return cvt.skip(val);
            }
        };


    } // details




    template <typename Ty_, typename = void>
    struct conversion_traits
    {
        /**
         *  default implementation does fallback to old traits for backward compatibility
         */
        static constexpr stored_type fetch_type = config_value_trait<Ty_>::fetch_type;

        /**
         * converter input type
         */
        using value_type = decltype(details::wrap_result_(std::declval<typename stored_type_trait<fetch_type>::return_type>()));

        /**
        * converter_type typically supports two methods:
        * Ty_ load(const stored_type&)
        * stored_type store(const Ty_&)
        *
        * extra optional method for skipping stored elements
        * bool skip(const Ty_&)
        *
        * throw config_error on error
        */
        using converter_type = typename config_value_trait<Ty_>::converter_type;
    };


    template <typename Converter_, typename Ty_>
    inline
    auto invoke_load(const Converter_& cvt, const Ty_& val)
    {
        return details::invoke_load_impl<Converter_, Ty_>::apply(cvt, val);
    }

    template <typename Converter_, typename Ty_>
    inline
    bool invoke_skip(const Converter_& cvt, const Ty_& val)
    {
        return details::invoke_skip_impl<Converter_, Ty_>::apply(cvt, val);
    }

    template <typename Converter_, typename Ty_>
    inline
    auto invoke_store(const Converter_& cvt, const Ty_& val)
    {
        return cvt.store(val);
    }




    namespace details {

        template <typename Ty_, typename Converter_>
        static
        auto map_result(const yato::optional<Ty_>& opt, const Converter_& cvt)
            -> yato::optional<decltype(invoke_load(std::declval<Converter_>(), wrap_result_(std::declval<Ty_>())))>
        try
        {
            return opt.map([&cvt](const Ty_& val) { return invoke_load(cvt, wrap_result_(val)); });
        }
        catch (config_error&) {
            return yato::nullopt;
        }

        template <typename Ty_, typename Converter_>
        static
        auto map_result(yato::optional<Ty_>&& opt, const Converter_& cvt)
            -> yato::optional<decltype(invoke_load(std::declval<Converter_>(), wrap_result_(std::declval<Ty_>())))>
        try
        {
            return opt.map([&cvt](Ty_& val) { return invoke_load(cvt, wrap_result_(std::move(val))); });
        }
        catch (config_error&) {
            return yato::nullopt_t{};
        }


        template <conf::stored_type FetchType_, typename Converter_, typename = void>
        struct convert_config
        {
            template <typename Conf_>
            static
            yato::nullopt_t apply(const Converter_& /*converter*/, const Conf_& /*conf*/)
            {
                return yato::nullopt_t{};
            }
        };

        template <typename Converter_>
        struct convert_config<stored_type::config, Converter_, yato::void_t<converted_stored_type<stored_type::config, Converter_>>>
        {
            template <typename Conf_>
            static
            yato::optional<converted_stored_type<stored_type::config, Converter_>> apply(const Converter_& converter, const Conf_& conf)
            try
            {
                return details::invoke_load_impl<Converter_, config>::apply(converter, conf);
            }
            catch (config_error&) {
                return yato::nullopt_t{};
            }
        };

    } // details



    /**
     * Config entry descriptor.
     * In the case of array-like configs key is always empty.
     */
    class config_entry
    {
    public:
        config_entry(backend_ptr_t backend, size_t index)
            : m_find_result(std::move(backend), index)
        { }

        config_entry(backend_ptr_t backend, const conf::path& path)
        {
            if (backend) {
                resolve_path_(std::move(backend), path.tokenize());
            }
        }

        config_entry(const config_entry&) = delete;

        config_entry(config_entry&& other) noexcept = default;

        ~config_entry() = default;

        config_entry& operator=(const config_entry&) = delete;

        config_entry& operator=(config_entry&& other) noexcept = default;

        YATO_ATTR_NODISCARD
        size_t index() const
        {
            return m_find_result.get_index();
        }

        YATO_ATTR_NODISCARD
        const std::string& key() const
        {
            return m_find_result.get_key();
        }

        YATO_ATTR_NODISCARD
        stored_type type() const
        {
            if (!m_find_result) {
                throw yato::conf::config_error("config_entry[type]: entry is empty.");
            }
            return m_find_result->type();
        }

        YATO_ATTR_NODISCARD
        bool is_null() const
        {
            return !static_cast<bool>(m_find_result);
        }

        YATO_ATTR_NODISCARD explicit
        operator bool() const
        {
            return static_cast<bool>(m_find_result);
        }

        template <typename Ty_>
        YATO_ATTR_NODISCARD
        yato::optional<Ty_> value() const
        {
            using default_converter = typename conversion_traits<Ty_>::converter_type;
            return value<Ty_>(default_converter{});
        }

        template <typename Ty_, typename Converter_>
        YATO_ATTR_NODISCARD
        yato::optional<Ty_> value(const Converter_& converter) const
        {
            constexpr conf::stored_type fetch_type = conf::conversion_traits<Ty_>::fetch_type;
            static_assert(std::is_same<Ty_, details::converted_stored_type<fetch_type, Converter_>>::value, 
                "Provided converter must return exactly type Ty_");
            return value<fetch_type>(converter);
        }

        template <stored_type FetchType_, typename Converter_>
        YATO_ATTR_NODISCARD
        auto value(const Converter_& converter) const
            -> yato::optional<details::converted_stored_type<FetchType_, Converter_>>
        {
            if (!m_find_result) {
                return yato::nullopt_t{};
            }
            using return_type = typename stored_type_trait<FetchType_>::return_type;
            return details::map_result(m_find_result->get<return_type>(FetchType_), converter);
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
            return m_find_result.get_value();
        }

    private:
        template <typename Tokenizer_>
        void resolve_path_(backend_ptr_t backend, Tokenizer_ path_iter)
        {
            if (!path_iter.empty()) {
                while (backend) {
                    const auto t = path_iter.next();
                    const std::string name{ t.begin(), t.end() };
                    if (path_iter.has_next()) {
                        // We need to go deeper
                        backend = backend->value_as<backend_ptr_t>(name, stored_type::config).get_or(nullptr);
                    }
                    else {
                        // Fetch a value
                        m_find_result = backend->find(name);
                        break;
                    }
                }
            }
        }

        config_backend::find_result m_find_result{ };
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

        ~config_iterator() = default;

        config_iterator& operator=(const config_iterator&) = default;

        config_iterator& operator=(config_iterator&&) = default;

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

    private:
        config_iterator(backend_ptr_t backend, size_t pos, size_t size)
            : m_backend(std::move(backend))
            , m_idx(yato::narrow_cast<index_type>(pos))
            , m_size(yato::narrow_cast<index_type>(size))
        { }

        config_iterator(backend_ptr_t backend, const conf::path& p, size_t size)
            : m_backend(std::move(backend))
            , m_idx(yato::narrow_cast<index_type>(size))
            , m_size(yato::narrow_cast<index_type>(size))
        {
            config_entry entry(m_backend, p);
            if (entry) {
                m_idx = yato::narrow_cast<index_type>(entry.index());
            }
        }

        YATO_ATTR_NODISCARD
        bool is_dereferencable_() const
        {
            return m_backend && (m_idx >= 0) && (m_idx < m_size);
        }

        backend_ptr_t m_backend;
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
        config(backend_ptr_t backend) 
            : m_backend(std::move(backend))
        { }

        config(const config&) = default;

        config(config&&) noexcept = default;

        ~config() = default;

        config& operator=(const config&) = default;

        config& operator=(config&&) noexcept = default;

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
         * Checks a config property
         */
        YATO_ATTR_NODISCARD
        bool has_property(config_property p) const
        {
            return m_backend ? m_backend->has_property(p) : false;
        }

        /**
         * Return true if config supports associative access by name
         */
        YATO_ATTR_NODISCARD
        bool is_object() const
        {
            return has_property(config_property::associative);
        }

        /**
         * Return true if config supports associative access by name
         */
        YATO_ATTR_NODISCARD
        bool is_associative() const
        {
            return has_property(config_property::associative);
        }

        /**
         * Return true if config supports associative access by name
         */
        YATO_ATTR_NODISCARD
        bool is_ordered() const
        {
            return has_property(config_property::keeps_order);
        }

        /**
         * Return true if config supports associative access by name and allows multiple values for one key
         */
        YATO_ATTR_NODISCARD
        bool is_multi_associative() const
        {
            return has_property(config_property::multi_associative);
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
            return m_backend ? m_backend->enumerate_keys() : std::vector<std::string>{};
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
            return name.is_root()
                ? cvt<Ty_>()
                : config_entry(m_backend, name).value<Ty_>();
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
        yato::optional<Ty_> value(const conf::path& name, const Converter_& converter) const
        {
            return name.is_root()
                ? cvt(converter)
                : config_entry(m_backend, name).value<Ty_>(converter);
        }

        /**
         * Get value of array by index.
         * Valid for any configs.
         * @return Optional value of type Ty_
         */
        template <typename Ty_, typename Converter_>
        YATO_ATTR_NODISCARD
        yato::optional<Ty_> value(size_type idx, const Converter_& converter) const
        {
            return config_entry(m_backend, idx).value<Ty_>(converter);
        }

        /**
         * Get a named value obtained with a provided converter.
         * Valid only if is_object() returned `true`
         * @return Optional value returned from the converter
         */
        template <conf::stored_type FetchType_, typename Converter_>
        YATO_ATTR_NODISCARD
        auto value(const conf::path & name, const Converter_& converter) const
            -> yato::optional<details::converted_stored_type<FetchType_, Converter_>>
        {
            return name.is_root()
                ? cvt(converter)
                : config_entry(m_backend, name).value<FetchType_>(converter);
        }

        /**
         * Get a value, obtained with a provided converter, by index.
         * Valid for any configs.
         * @return Optional value returned from the converter
         */
        template <conf::stored_type FetchType_, typename Converter_ >
        YATO_ATTR_NODISCARD
        auto value(size_type idx, const Converter_& converter) const
            -> yato::optional<details::converted_stored_type<FetchType_, Converter_>>
        {
            return config_entry(m_backend, idx).value<FetchType_>(converter);
        }


        /**
         * Get a value, obtained via converting this config to a given type Ty_.
         * Valid for any configs.
         * @return Optional value of type Ty_
         */
        template <typename Ty_>
        YATO_ATTR_NODISCARD
        yato::optional<Ty_> cvt() const
        {
            using default_converter = typename conversion_traits<Ty_>::converter_type;
            return cvt(default_converter{});
        }


        /**
         * Get a value, obtained via applying given converter to this config.
         * Valid for any configs.
         * @return  Optional value returned from the converter
         */
        template <typename Converter_>
        YATO_ATTR_NODISCARD
        auto cvt(const Converter_& converter) const
        {
            return m_backend
                ? details::convert_config<conf::stored_type::config, Converter_>::apply(converter, *this)
                : yato::nullopt_t{};
        }


        /**
         * Get an entry handle for a key.
         */
        YATO_ATTR_NODISCARD
        config_entry at(const conf::path& name) const
        {
            return config_entry(m_backend, name);
        }

        /**
         * Get an entry handle for a key.
         */
        YATO_ATTR_NODISCARD
        config_entry at(size_type idx) const
        {
            return config_entry(m_backend, idx);
        }

        /**
         * Get an entry handle for a key.
         */
        YATO_ATTR_NODISCARD
        config_iterator find(const conf::path& name) const
        {
            return config_iterator(m_backend, name, size());
        }

        /**
         * Get an entry handle for a key.
         */
        YATO_ATTR_NODISCARD
        config_iterator find(size_type idx) const
        {
            return config_iterator(m_backend, idx, size());
        }

        /**
         * Get nested object config.
         * Alias for value<config>
         */
        YATO_ATTR_NODISCARD
        config object(const conf::path& name) const
        {
            return value<config>(name).get_or(config{});
        }

        /**
         * Get nested object config.
         * Alias for value<config>.
         */
        YATO_ATTR_NODISCARD
        config object(size_type idx) const
        {
            return value<config>(idx).get_or(config{});
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
        const_iterator cbegin() const
        {
            return config_iterator(m_backend, 0, size());
        }

        /**
         * Values range start.
         */
        YATO_ATTR_NODISCARD
        iterator begin() const
        {
            return cbegin();
        }

        /**
         * Values range end.
         */
        YATO_ATTR_NODISCARD
        const_iterator cend() const
        {
            const auto s = size();
            return config_iterator(m_backend, s, s);
        }

        /**
         * Values range end.
         */
        YATO_ATTR_NODISCARD
        iterator end() const
        {
            return cend();
        }

        /**
         * Values range
         */
        YATO_ATTR_NODISCARD
        yato::range<const_iterator> crange() const
        {
            return yato::make_range(cbegin(), cend());
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
                result.emplace_back(at(i).value<Ty_>().get());
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
            return to_map_impl_<Ty_, std::map<std::string, Ty_, Pr_, Alloc_>>();
        }

        /**
         * Converts a plain config to a map of stored values casted to Ty_
         */
        template <typename Ty_, typename Pr_ = std::less<Ty_>, typename Alloc_ = std::allocator<std::pair<const std::string, Ty_>>>
        YATO_ATTR_NODISCARD
        std::multimap<std::string, Ty_, Pr_, Alloc_> to_multimap() const
        {
            return to_map_impl_<Ty_, std::multimap<std::string, Ty_, Pr_, Alloc_>>();
        }

        /**
         * For internal usage.
         * Use it only if you are sure what you are doing.
         */
        YATO_ATTR_NODISCARD
        const backend_ptr_t& backend_handle_() const
        {
            return m_backend;
        }

        /**
         * For internal usage.
         * Use it only if you are sure what you are doing.
         */
        YATO_ATTR_NODISCARD
        backend_ptr_t& backend_handle_()
        {
            return m_backend;
        }

    private:
        YATO_ATTR_NODISCARD
        config with_value_(const conf::path& path, stored_variant value) const;

        /**
         * Converts a plain config to a map of stored values casted to Ty_
         */
        template <typename Ty_, typename MapType_>
        MapType_ to_map_impl_() const
        {
            if (!is_associative()) {
                throw yato::conf::config_error("config[to_map_impl_]: Config must be associative");
            }
            MapType_ result;
            const size_t s = size();
            for (size_t i = 0; i < s; ++i) {
                auto entry = at(i);
                result.emplace(entry.key(), entry.value<Ty_>().get());
            }
            return result;
        }

        //---------------------------------------------------------------------
        backend_ptr_t m_backend{ nullptr };
    };


    template <>
    struct config_value_trait<yato::conf::config>
    {
        using converter_type = details::identity_converter<yato::conf::config, yato::conf::config>;
        static constexpr stored_type fetch_type = stored_type::config;
    };

    inline
    config details::wrap_result_(const backend_ptr_t& val) {
        return config(val);
    }

    inline
    config details::wrap_result_(backend_ptr_t&& val) {
        return config(std::move(val));
    }

    inline
    config config_entry::object() const
    {
        return value<config>().get_or(config{});
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
    using conf::config_property;
    using conf::stored_type;
    using conf::config_error;

} // namespace yato

#endif // _YATO_CONFIG_H_
