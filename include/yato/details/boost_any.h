// See http://www.boost.org/libs/any for Documentation.

#ifndef BOOST_ANY_INCLUDED
#define BOOST_ANY_INCLUDED

#if defined(_MSC_VER)
# pragma once
#endif

// what:  variant type boost::any
// who:   contributed by Kevlin Henney,
//        with features contributed and bugs found by
//        Antony Polukhin, Ed Brey, Mark Rodgers, 
//        Peter Dimov, and James Curran
// when:  July 2001, April 2013 - May 2013

#include <algorithm>

//#include "boost/config.hpp"
//#include <boost/type_index.hpp>
//#include <boost/type_traits/remove_reference.hpp>
//#include <boost/type_traits/decay.hpp>
//#include <boost/type_traits/remove_cv.hpp>
//#include <boost/type_traits/add_reference.hpp>
//#include <boost/type_traits/is_reference.hpp>
//#include <boost/type_traits/is_const.hpp>
//#include <boost/throw_exception.hpp>
//#include <boost/static_assert.hpp>
//#include <boost/utility/enable_if.hpp>
//#include <boost/type_traits/is_same.hpp>
//#include <boost/type_traits/is_const.hpp>
//#include <boost/mpl/if.hpp>

#include <memory>
#include <typeindex>
#include "../type_traits.h"

namespace boost
{

    class bad_any_cast
        : public std::bad_cast
    {
    public:
        virtual const char * what() const YATO_NOEXCEPT_KEYWORD
        {
            return "boost::bad_any_cast: "
                "failed conversion using boost::any_cast";
        }
    };

    class any_copy_error
        : public std::exception
    {
    public:
        virtual const char * what() const YATO_NOEXCEPT_KEYWORD
        {
            return "boost::any_copy_error: "
                "held type is not copyable";
        }
    };


    namespace details
    {



        class placeholder
        {
        public:
            virtual ~placeholder() = default;

            virtual const std::type_info & type() const YATO_NOEXCEPT_KEYWORD = 0;

            virtual std::unique_ptr<placeholder> clone() const = 0;
        };
        //--------------------------------------------------------------

        template<typename ValueType, typename = void>
        class holder
            : public placeholder
        {
        public:
            ValueType m_payload;
            
            holder(const ValueType & value)
                : m_payload(value)
            { }

            holder(ValueType&& value)
                : m_payload(std::move(value))
            { }

            holder & operator=(const holder &) = delete;
            holder & operator=(holder&&) = delete;

            ~holder() override = default;

            const std::type_info & type() const YATO_NOEXCEPT_KEYWORD override
            {
                return typeid(ValueType);
            }

            std::unique_ptr<placeholder> clone() const override
            {
                return std::make_unique<holder<ValueType>>(m_payload);
            }
        };
        //--------------------------------------------------------------

        /**
         * Holder for non-CopyConstructible classes
         */
        template<typename ValueType>
        class holder <ValueType, typename std::enable_if<
                !std::is_copy_assignable<ValueType>::value
            , void>::type>
            : public placeholder
        {
        public:
            ValueType m_payload;

            holder(const ValueType &) = delete;

            holder(ValueType && value)
                : m_payload(std::move(value))
            { }

            holder & operator=(const holder &) = delete;
            holder & operator=(holder&&) = delete;

            ~holder() override = default;

            const std::type_info & type() const YATO_NOEXCEPT_KEYWORD override
            {
                return typeid(ValueType);
            }

            YATO_NORETURN
            std::unique_ptr<placeholder> clone() const override
            {
                // Can't be copied
                throw any_copy_error();
            }
        };

    }

    class any
    {
    private:
        std::unique_ptr<details::placeholder> m_content;
        //------------------------------------------------

    public:
        any() = default;

        template<typename ValueType>
        any(ValueType && value)
            : m_content(std::make_unique<details::holder<typename std::decay<ValueType>::type>>(
                std::forward<ValueType>(value)))
        { }

        any(const any & other)
            : m_content(other.m_content 
                ? other.m_content->clone() 
                : nullptr)
        { }

#ifndef YATO_MSVC_2013
        any(any &&) = default;
#else
        any(any && other)
            : m_content(std::move(other.m_content))
        { }
#endif

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

        void clear() YATO_NOEXCEPT_KEYWORD
        {
            any().swap(*this);
        }

        const std::type_info & type() const YATO_NOEXCEPT_KEYWORD
        {
            return m_content ? m_content->type() : typeid(void);
        }

    private:

        template<typename ValueType>
        friend ValueType* any_cast(any*) YATO_NOEXCEPT_KEYWORD;

        template<typename ValueType>
        friend ValueType* unsafe_any_cast(any*) YATO_NOEXCEPT_KEYWORD;

        template<typename ValueType>
        friend ValueType& unsafe_any_cast(any&) YATO_NOEXCEPT_KEYWORD;
    };
 
    inline void swap(any & lhs, any & rhs) YATO_NOEXCEPT_KEYWORD
    {
        lhs.swap(rhs);
    }

    template<typename ValueType>
    ValueType * any_cast(any * operand) YATO_NOEXCEPT_KEYWORD
    {
        //return operand && operand->type() == boost::typeindex::type_id<ValueType>()
        return operand && operand->type() == typeid(ValueType)
            ? &static_cast<details::holder<typename std::remove_cv<ValueType>::type > * > (operand->m_content.get())->m_payload
            : 0;
    }

    template<typename ValueType>
    inline const ValueType * any_cast(const any * operand) YATO_NOEXCEPT_KEYWORD
    {
        return any_cast<ValueType>(const_cast<any *>(operand));
    }

    template<typename ValueType>
    ValueType any_cast(any & operand)
    {
        typedef typename std::remove_reference<ValueType>::type nonref;


        nonref * result = any_cast<nonref>(&operand);
        if(!result)
            throw bad_any_cast();

        // Attempt to avoid construction of a temporary object in cases when 
        // `ValueType` is not a reference. Example:
        // `static_cast<std::string>(*result);` 
        // which is equal to `std::string(*result);`
        typedef typename std::conditional<
            std::is_reference<ValueType>::value,
            ValueType,
            typename std::add_lvalue_reference<ValueType>::type
        >::type ref_type;

        return static_cast<ref_type>(*result);
    }

    template<typename ValueType>
    inline ValueType any_cast(const any & operand)
    {
        typedef typename std::remove_reference<ValueType>::type nonref;
        return any_cast<const nonref &>(const_cast<any &>(operand));
    }

    template<typename ValueType>
    inline ValueType any_cast(any&& operand)
    {
        static_assert(
            std::is_rvalue_reference<ValueType&&>::value /*true if ValueType is rvalue or just a value*/
                || std::is_const< typename std::remove_reference<ValueType>::type >::value,
            "boost::any_cast shall not be used for getting nonconst references to temporary objects" 
        );
        return any_cast<ValueType>(operand);
    }


    // Note: The "unsafe" versions of any_cast are not part of the
    // public interface and may be removed at any time. They are
    // required where we know what type is stored in the any and can't
    // use typeid() comparison, e.g., when our types may travel across
    // different shared libraries.
    template<typename ValueType>
    inline ValueType * unsafe_any_cast(any * operand) YATO_NOEXCEPT_KEYWORD
    {
        return &static_cast<details::holder<ValueType>*>(operand->m_content.get())->m_payload;
    }

    template<typename ValueType>
    inline const ValueType * unsafe_any_cast(const any * operand) YATO_NOEXCEPT_KEYWORD
    {
        return unsafe_any_cast<ValueType>(const_cast<any*>(operand));
    }

    template<typename ValueType>
    inline ValueType & unsafe_any_cast(any & operand) YATO_NOEXCEPT_KEYWORD
    {
        return static_cast<details::holder<ValueType>*>(operand.m_content.get())->m_payload;
    }

    template<typename ValueType>
    inline const ValueType & unsafe_any_cast(const any & operand) YATO_NOEXCEPT_KEYWORD
    {
        return unsafe_any_cast<ValueType>(const_cast<any&>(operand));
    }
}

// Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#endif

