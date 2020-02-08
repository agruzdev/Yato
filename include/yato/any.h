/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

/*
 * Refatored boost::any
 * std::any and boost::any support only CopyConstructible classes
 * This is extended implementation, supporting movable types and atomics as well
 * boost::any http://www.boost.org/libs/any
 */

#ifndef _YATO_ANY_H_
#define _YATO_ANY_H_

#include <algorithm>
#include <memory>
#include <typeindex>

#include "types.h"
#include "optional.h"

namespace yato
{

    class bad_any_cast
        : public std::bad_cast
    {
    public:
        const char* what() const YATO_NOEXCEPT_KEYWORD override
        {
            return "yato::bad_any_cast: failed conversion using yato::any_cast";
        }
    };

    class any_copy_error
        : public std::runtime_error
    {
    public:
        any_copy_error()
            : std::runtime_error("yato::any_copy_error: held type is not copyable")
        { }
    };

    struct nullany_t
    {
        explicit
        nullany_t() = default;
    };

#ifndef YATO_MSVC_2013
    constexpr nullany_t nullany{};
#endif


    namespace details
    {
        class any_placeholder
        {
        public:
            virtual ~any_placeholder() = default;

            virtual const std::type_info & type() const YATO_NOEXCEPT_KEYWORD = 0;

            virtual std::unique_ptr<any_placeholder> clone() const = 0;
        };
        //--------------------------------------------------------------

        template<typename ValueType, typename = void>
        class any_holder
            : public any_placeholder
        {
        public:
            ValueType m_payload;

            any_holder(const ValueType & value)
                : m_payload(value)
            { }

            any_holder(ValueType&& value)
                : m_payload(std::move(value))
            { }

            template <typename ... Args>
            any_holder(yato::in_place_t, Args && ... args)
                : m_payload(std::forward<Args>(args)...)
            { }

            any_holder & operator=(const any_holder &) = delete;
            any_holder & operator=(any_holder &&) = delete;

            ~any_holder() override = default;

            const std::type_info & type() const YATO_NOEXCEPT_KEYWORD override
            {
                return typeid(ValueType);
            }

            std::unique_ptr<any_placeholder> clone() const override
            {
                return std::make_unique<any_holder<ValueType>>(m_payload);
            }
        };
        //--------------------------------------------------------------

        /**
         * Holder for non-CopyConstructible classes
         */
        template<typename ValueType>
        class any_holder<ValueType, typename std::enable_if<
            !std::is_copy_assignable<ValueType>::value && std::is_move_constructible<ValueType>::value
            , void>::type>
            : public any_placeholder
        {
        public:
            ValueType m_payload;

            any_holder(const ValueType &) = delete;

            any_holder(ValueType && value)
                : m_payload(std::move(value))
            { }

            template <typename ... Args>
            any_holder(yato::in_place_t, Args && ... args)
                : m_payload(std::forward<Args>(args)...)
            { }

            any_holder & operator=(const any_holder &) = delete;
            any_holder & operator=(any_holder &&) = delete;

            ~any_holder() override = default;

            const std::type_info & type() const YATO_NOEXCEPT_KEYWORD override
            {
                return typeid(ValueType);
            }

            YATO_NORETURN
            std::unique_ptr<any_placeholder> clone() const override
            {
                // Can't be copied
                throw any_copy_error();
            }
        };
        //--------------------------------------------------------------------

        /**
        * Holder for non-CopyConstructible and non-MoveConstructible classes
        */
        template<typename ValueType>
        class any_holder<ValueType, typename std::enable_if<
            !std::is_copy_assignable<ValueType>::value && !std::is_move_constructible<ValueType>::value
            , void>::type>
            : public any_placeholder
        {
        public:
            ValueType m_payload;

            any_holder(const ValueType &) = delete;

            any_holder(ValueType &&) = delete;

            template <typename ... Args>
            any_holder(yato::in_place_t, Args && ... args)
                : m_payload(std::forward<Args>(args)...)
            {
            }

            any_holder & operator=(const any_holder &) = delete;
            any_holder & operator=(any_holder &&) = delete;

            ~any_holder() override = default;

            const std::type_info & type() const YATO_NOEXCEPT_KEYWORD override
            {
                return typeid(ValueType);
            }

            YATO_NORETURN
            std::unique_ptr<any_placeholder> clone() const override
            {
                // Can't be copied
                throw any_copy_error();
            }
        };

    }

    class any
    {
    private:
        std::unique_ptr<details::any_placeholder> m_content{ nullptr };
        //------------------------------------------------

    public:
        any() = default;

        template <typename ValueType>
        explicit
        any(ValueType && value)
            : m_content(std::make_unique<details::any_holder<typename std::decay<ValueType>::type>>(
                std::forward<ValueType>(value)))
        { }

        any(const any & other)
            : m_content(other.m_content
                ? other.m_content->clone()
                : nullptr)
        { }

        template <typename Ty, typename ... Args>
        explicit
        any(yato::in_place_type_t<Ty>, Args && ... args)
            : m_content(std::make_unique<details::any_holder<Ty>>(yato::in_place_t(), std::forward<Args>(args)...))
        { }

        any(nullany_t)
        { }

        any(any &&) YATO_NOEXCEPT_KEYWORD = default;

        ~any() = default;

        any & swap(any & other) YATO_NOEXCEPT_KEYWORD
        {
            m_content.swap(other.m_content);
            return *this;
        }

        any & operator=(const any & other)
        {
            any(other).swap(*this);
            return *this;
        }

        any & operator=(any && other) YATO_NOEXCEPT_KEYWORD
        {
            other.swap(*this);
            any().swap(other);
            return *this;
        }

        template <class ValueType>
        any & operator=(ValueType && rhs)
        {
            any(std::forward<ValueType>(rhs)).swap(*this);
            return *this;
        }

        bool empty() const YATO_NOEXCEPT_KEYWORD
        {
            return !m_content;
        }

        explicit
        operator bool() const YATO_NOEXCEPT_KEYWORD
        {
            return static_cast<bool>(m_content);
        }

        void clear() YATO_NOEXCEPT_KEYWORD
        {
            m_content.reset();
        }

        const std::type_info & type() const YATO_NOEXCEPT_KEYWORD
        {
            return m_content ? m_content->type() : typeid(void);
        }

        template <typename Ty_>
        bool is_type() const
        {
            return (type() == typeid(Ty_));
        }

        template <typename Ty, typename ... Args>
        void emplace(Args && ... args)
        {
            any tmp(yato::in_place_type_t<Ty>(), std::forward<Args>(args)...);
            tmp.swap(*this);
        }

        template <typename Ty>
        Ty & get_as()
        {
            if (std::type_index(typeid(Ty)) == std::type_index(type())) {
                if (m_content != nullptr) {
                    return static_cast<details::any_holder<Ty>*>(m_content.get())->m_payload;
                }
            }
            throw bad_any_cast();
        }

        template <typename Ty>
        const Ty & get_as() const
        {
            if (std::type_index(typeid(Ty)) == std::type_index(type())) {
                if (m_content != nullptr) {
                    return static_cast<details::any_holder<Ty>*>(m_content.get())->m_payload;
                }
            }
            throw bad_any_cast();
        }

        template <typename Ty>
        Ty & get_as(Ty & default_value) YATO_NOEXCEPT_KEYWORD
        {
            if (std::type_index(typeid(Ty)) == std::type_index(type())) {
                if (m_content != nullptr) {
                    return static_cast<details::any_holder<Ty>*>(m_content.get())->m_payload;
                }
            }
            return default_value;
        }

        template <typename Ty>
        const Ty & get_as(const Ty & default_value) const YATO_NOEXCEPT_KEYWORD
        {
            if (std::type_index(typeid(Ty)) == std::type_index(type())) {
                if (m_content != nullptr) {
                    return static_cast<details::any_holder<Ty>*>(m_content.get())->m_payload;
                }
            }
            return default_value;
        }

        /**
         * Get value without check
         * Use only if you are sure what is stored type
         */
        template <typename Ty>
        Ty & get_as_unsafe()
        {
            assert(m_content != nullptr);
            return static_cast<details::any_holder<Ty>*>(m_content.get())->m_payload;
        }

        /**
         * Get value without check
         * Use only if you are sure what is stored type
         */
        template <typename Ty>
        const Ty & get_as_unsafe() const
        {
            assert(m_content != nullptr);
            return static_cast<details::any_holder<Ty>*>(m_content.get())->m_payload;
        }

        template <typename Ty_>
        yato::optional<Ty_> get_opt()
        {
            return is_type<Ty_>()
                ? yato::make_optional(get_as_unsafe<Ty_>())
                : yato::nullopt_t{};
        }

        template <typename Ty_>
        yato::optional<Ty_> get_opt() const
        {
            return is_type<Ty_>()
                ? yato::make_optional(get_as_unsafe<Ty_>())
                : yato::nullopt_t{};
        }
    };

    inline void swap(any & lhs, any & rhs) YATO_NOEXCEPT_KEYWORD
    {
        lhs.swap(rhs);
    }

    template <typename ValueType>
    ValueType any_cast(any && operand)
    {
        return operand.get_as<ValueType>();
    }

    template <typename ValueType>
    ValueType any_cast(const any & operand)
    {
        return operand.get_as<ValueType>();
    }

    template <class Ty_, class... Args_>
    yato::any make_any(Args_ && ... args)
    {
        return yato::any(yato::in_place_type_t<Ty_>{}, std::forward<Args_>(args)...);
    }
}

#endif

