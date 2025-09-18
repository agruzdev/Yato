/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
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
        config_error(const std::string& err)
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
    using backend_ptr_t = std::shared_ptr<const config_backend>;



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
        using return_type = backend_ptr_t;
    };


    namespace details
    {
        template <typename ToType_, typename FromType_>
        struct identity_converter
        {
            constexpr
            const ToType_& load(const FromType_& val) const {
                return val;
            }

            constexpr
            ToType_ load(FromType_&& val) const {
                return static_cast<ToType_>(std::move(val));
            }

            const FromType_& store(const ToType_& val) const
            {
                return val;
            }

            FromType_ store(ToType_&& val) const
            {
                return std::move(val);
            }
        };

        template <typename ToType_, typename FromType_>
        struct narrow_converter
        {
            constexpr
            ToType_ load(const FromType_& val) const {
                return yato::narrow_cast<ToType_>(val);
            }

            constexpr
            FromType_ store(const ToType_& val) const {
                return yato::narrow_cast<FromType_>(val);
            }
        };

        /**
         * Assumes that FromType_ is wider than ToType_
         */
        template <typename ToType_, typename FromType_>
        struct checked_limits_converter
        {
            YATO_CONSTEXPR_FUNC_CXX14
            ToType_ operator()(const FromType_& val) const {
                YATO_REQUIRES(val <= static_cast<FromType_>(std::numeric_limits<ToType_>::max()));
                YATO_REQUIRES(val >= static_cast<FromType_>(std::numeric_limits<ToType_>::lowest()));
                return static_cast<ToType_>(val);
            }
        };
    }

    template <typename Ty_, typename = void>
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



    enum class config_property
        : uint32_t
    {
        associative = 0x1,                          ///< Supports find by key
        multi_associative = 0x2 | associative,      ///< Allows multiple values for one key
        keeps_order = 0x4                           ///< Order of keys is preserved on read/write
    };

    constexpr
    config_property operator|(const config_property& lhs, const config_property& rhs)
    {
        return static_cast<config_property>(static_cast<std::underlying_type_t<config_property>>(lhs) | static_cast<std::underlying_type_t<config_property>>(rhs));
    }

    constexpr
    config_property operator&(const config_property& lhs, const config_property& rhs)
    {
        return static_cast<config_property>(static_cast<std::underlying_type_t<config_property>>(lhs) & static_cast<std::underlying_type_t<config_property>>(rhs));
    }


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
         * Get value with possible conversion.
         */
        virtual stored_variant get() const noexcept = 0;

        /**
         * Converts stored value to desired type if possible.
         */
        template <typename Ty_>
        yato::optional<Ty_> get(stored_type dst_type) const
        {
            auto val = get();
            if (val) {
                val = convert_(dst_type, val);
            }
            return val.get_opt<Ty_>();
        }

    private:
        stored_variant convert_(stored_type dst_type, const stored_variant & src) const;
    };


    /**
     * Facade class providing non-virtual interface for a config implementation
     */
    class config_backend
        : public std::enable_shared_from_this<config_backend>
    {
    public:
        using find_index_result_t = std::tuple<std::string, config_value*>;
        using find_key_result_t   = std::tuple<size_t, config_value*>;

        /**
         * RAII wrapper for find result
         */
        class find_result
        {
        public:
            YATO_CONSTEXPR_FUNC
            find_result() = default;

            find_result(backend_ptr_t parent, size_t index)
                : m_parent(std::move(parent))
                , m_index(index)
            {
                if (m_parent) {
                    try {
                        std::tie(m_key, m_value) = m_parent->do_find(m_index);
                    }
                    catch (...) {
                        // ToDo (a.gruzdev): Report error
                        m_value = nullptr;
                    }
                }
            }

            find_result(backend_ptr_t parent, std::string key)
                : m_parent(std::move(parent))
                , m_key(std::move(key))
            {
                if (m_parent) {
                    try {
                        std::tie(m_index, m_value) = m_parent->do_find(m_key);
                    }
                    catch (...) {
                        // ToDo (a.gruzdev): Report error
                        m_value = nullptr;
                    }
                }
            }

            find_result(const find_result& other) = delete;

            find_result(find_result&& other) noexcept
                : m_parent(std::move(other.m_parent))
                , m_index(other.m_index)
                , m_key(std::move(other.m_key))
                , m_value(other.m_value)
            {
                other.m_value = nullptr;
            }

            ~find_result()
            {
                if (m_value) {
                    m_parent->do_release(m_value);
                }
            }

            find_result& operator=(const find_result& other) = delete;

            find_result& operator=(find_result&& other) noexcept
            {
                YATO_REQUIRES(this != &other);
                if (m_value) {
                    m_parent->do_release(m_value);
                }
                m_parent = std::move(other.m_parent);
                m_index = other.m_index;
                m_key = std::move(other.m_key);
                m_value = other.m_value;
                other.m_value = nullptr;
                return *this;
            }

            YATO_CONSTEXPR_FUNC
            operator bool() const
            {
                return m_value != nullptr;
            }

            YATO_CONSTEXPR_FUNC
            const config_value* operator->() const
            {
                YATO_REQUIRES(m_value != nullptr);
                return m_value;
            }

            YATO_CONSTEXPR_FUNC
            config_value* operator->()
            {
                YATO_REQUIRES(m_value != nullptr);
                return m_value;
            }

            YATO_CONSTEXPR_FUNC
            size_t get_index() const
            {
                return m_index;
            }

            YATO_CONSTEXPR_FUNC
            const std::string& get_key() const
            {
                return m_key;
            }

            YATO_CONSTEXPR_FUNC
            config_value* get_value() const
            {
                return m_value;
            }

        private:
            backend_ptr_t m_parent{ nullptr };
            size_t m_index{ };
            std::string m_key{ };
            config_value* m_value{ nullptr };
        };


        /**
         * Constant representing an empty find result
         */
        static const find_index_result_t no_index_result;

        /**
         * Constant representing an empty find result
         */
        static const find_key_result_t no_key_result;


        virtual ~config_backend() = default;

        /**
         * Get number of stored values.
         */
        size_t size() const
        {
            assert_valid_();
            return do_size();
        }

        /**
         * Checks config property
         */
        bool has_property(config_property p) const
        {
            assert_valid_();
            return do_has_property(p);
        }

        /**
         * Find value by index.
         */
        find_result find(size_t index) const
        {
            assert_valid_();
            return find_result(shared_from_this(), index);
        }

        /**
         * Find value by name.
         */
        find_result find(std::string key) const
        {
            assert_valid_();
            return find_result(shared_from_this(), std::move(key));
        }

        /**
         * Enumerate object's keys
         */
        std::vector<std::string> enumerate_keys() const
        {
            assert_valid_();
            try {
                return do_enumerate_keys();
            }
            catch (...) {
                // ToDo (a.gruzdev): Report error
            }
            return {};
        }

        /**
         * Helper method wrapping returned type into optional
         */
        template <typename Ty_>
        yato::optional<Ty_> value_as(size_t index, stored_type dst_type) const
        {
            const find_result value = find(index);
            if (value) {
                return value->get<Ty_>(dst_type);
            }
            return yato::nullopt_t{};
        }

        /**
         * Helper method wrapping returned type into optional
         */
        template <typename Ty_>
        yato::optional<Ty_> value_as(const std::string& name, stored_type dst_type) const
        {
            const find_result value = find(name);
            if (value) {
                return value->get<Ty_>(dst_type);
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
         * If not null value is returned, then the returned key must be valid.
         */
        virtual find_index_result_t do_find(size_t index) const = 0;
        
        /**
         * Release value handle obtained from do_find().
         */
        virtual void do_release(const config_value* val) const noexcept = 0;

        /**
         * Returns false by default
         */
        virtual bool do_has_property(config_property p) const noexcept;

        /**
         * Find value by name.
         * Returns config_backend::novalue by default;
         * If not null value is returned, then the returned index must be valid.
         */
        virtual find_key_result_t do_find(const std::string& key) const;

        /**
         * Default implementation calls find() for all valid indexes.
         */
        virtual std::vector<std::string> do_enumerate_keys() const;

    private:
        void assert_valid_() const
        {
            YATO_ASSERT(shared_from_this() != nullptr, "config_backend must be managed by std::shared_ptr.");
        }
    };

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_BACKEND_H_
