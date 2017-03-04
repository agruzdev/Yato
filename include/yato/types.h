/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016 Alexey Gruzdev
 */

#ifndef _YATO_TYPES_H_
#define _YATO_TYPES_H_

#include <cstdint>
#include <limits>

#include "assert.h"

namespace yato
{
    //-------------------------------------------------------
    // Fixed size integer types

#ifndef INT8_C
    using int8_t = signed char;
#else
    using int8_t = ::int8_t;
#endif
    static_assert(sizeof(int8_t) == 1, "Wrong int8_t type!");

#ifndef UINT8_C 
    using uint8_t = unsigned char;
#else
    using uint8_t = ::uint8_t;
#endif
    static_assert(sizeof(uint8_t) == 1, "Wrong uint8_t type!");

#ifndef INT16_C 
    using int16_t = signed short;
#else
    using int16_t = ::int16_t;
#endif
    static_assert(sizeof(int16_t) == 2, "Wrong int16_t type!");

#ifndef UINT16_C 
    using uint16_t = unsigned short;
#else
    using uint16_t = ::uint16_t;
#endif
    static_assert(sizeof(uint16_t) == 2, "Wrong uint16_t type!");

#ifndef INT32_C 
    using int32_t = signed int;
#else
    using int32_t = ::int32_t;
#endif
    static_assert(sizeof(int32_t) == 4, "Wrong int32_t type!");

#ifndef UINT32_C 
    using uint32_t = unsigned int;
#else
    using uint32_t = ::uint32_t;
#endif
    static_assert(sizeof(uint32_t) == 4, "Wrong uint32_t type!");

#ifndef INT64_C 
    using int64_t = signed long long;
#else
    using int64_t = ::int64_t;
#endif
    static_assert(sizeof(int64_t) == 8, "Wrong int64_t type!");

#ifndef UINT64_C 
    using uint64_t = unsigned long long;
#else
    using uint64_t = ::uint64_t;
#endif
    static_assert(sizeof(uint64_t) == 8, "Wrong uint64_t type!");



    //-------------------------------------------------------
    // Fixed size floating point types

    using float32_t = float;
    static_assert(sizeof(float32_t) == 4, "Wrong float32_t type!");

    using float64_t = double;
    static_assert(sizeof(float64_t) == 8, "Wrong float64_t type!");

    /**
     *  Extended precision floating point. May have different implementations, so it's size is at least not less than size of double
     */
    using float80_t = long double;
    static_assert(sizeof(float80_t) >= 8, "Wrong float80_t type!");


    //-------------------------------------------------------
    // Helper for fixed size types

    struct signed_type_tag {};
    struct unsigned_type_tag {}; 
    struct floating_type_tag {};

    template <typename _Tag, size_t _Size>
    struct make_type
    { };

    template <>
    struct make_type<signed_type_tag, 8>
    {
        using type = int8_t;
    };

    template <>
    struct make_type<signed_type_tag, 16>
    {
        using type = int16_t;
    };

    template <>
    struct make_type<signed_type_tag, 32>
    {
        using type = int32_t;
    };

    template <>
    struct make_type<signed_type_tag, 64>
    {
        using type = int64_t;
    };

    template <>
    struct make_type<unsigned_type_tag, 8>
    {
        using type = uint8_t;
    };

    template <>
    struct make_type<unsigned_type_tag, 16>
    {
        using type = uint16_t;
    };

    template <>
    struct make_type<unsigned_type_tag, 32>
    {
        using type = uint32_t;
    };

    template <>
    struct make_type<unsigned_type_tag, 64>
    {
        using type = uint64_t;
    };

    template <>
    struct make_type<floating_type_tag, 32>
    {
        using type = float32_t;
    };

    template <>
    struct make_type<floating_type_tag, 64>
    {
        using type = float64_t;
    };

    template <>
    struct make_type<floating_type_tag, 80>
    {
        using type = float80_t;
    };

    template <typename _Tag, size_t _Size>
    using make_type_t = typename make_type<_Tag, _Size>::type;

    //-------------------------------------------------------
    // Cast functions

    template<typename _TypeTo, typename _TypeFrom>
    YATO_CONSTEXPR_FUNC 
    auto narrow_cast(const _TypeFrom & val) YATO_NOEXCEPT_IN_RELEASE
        -> typename std::enable_if<std::is_arithmetic<_TypeTo>::value && std::is_arithmetic<_TypeFrom>::value, _TypeTo>::type
    {
#if YATO_DEBUG
        return static_cast<_TypeFrom>(static_cast<_TypeTo>(val)) == val 
            ? static_cast<_TypeTo>(val)
            : (YATO_THROW_ASSERT_EXCEPT("narrow_cast failed!"), static_cast<_TypeTo>(0));
#else
        return static_cast<_TypeTo>(val);
#endif
    }

    template<typename _TypeTo, typename _TypeFrom>
    YATO_CONSTEXPR_FUNC 
    _TypeTo pointer_cast(_TypeFrom* ptr) YATO_NOEXCEPT_KEYWORD
    {
        return static_cast<_TypeTo>(static_cast<void*>(ptr));
    }

    template<typename _TypeTo, typename _TypeFrom>
    YATO_CONSTEXPR_FUNC
    _TypeTo pointer_cast(const _TypeFrom* ptr) YATO_NOEXCEPT_KEYWORD
    {
        return static_cast<_TypeTo>(static_cast<const void*>(ptr));
    }

    template<typename _TypeTo, typename _TypeFrom>
    YATO_CONSTEXPR_FUNC
    _TypeTo pointer_cast(volatile _TypeFrom* ptr) YATO_NOEXCEPT_KEYWORD
    {
        return static_cast<_TypeTo>(static_cast<volatile void*>(ptr));
    }

    template<typename _TypeTo, typename _TypeFrom>
    YATO_CONSTEXPR_FUNC
    _TypeTo pointer_cast(const volatile _TypeFrom* ptr) YATO_NOEXCEPT_KEYWORD
    {
        return static_cast<_TypeTo>(static_cast<const volatile void*>(ptr));
    }

    namespace literals
    {
#if defined(YATO_MSVC_2015) || (__cplusplus >= 201400L)
        YATO_CONSTEXPR_FUNC 
        int8_t operator"" _s8(unsigned long long number) YATO_NOEXCEPT_IN_RELEASE
        {
            return narrow_cast<int8_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        uint8_t operator"" _u8(unsigned long long number) YATO_NOEXCEPT_IN_RELEASE
        {
            return narrow_cast<uint8_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        int16_t operator"" _s16(unsigned long long number) YATO_NOEXCEPT_IN_RELEASE
        {
            return narrow_cast<int16_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        uint16_t operator"" _u16(unsigned long long number) YATO_NOEXCEPT_IN_RELEASE
        {
            return narrow_cast<uint16_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        int32_t operator"" _s32(unsigned long long number) YATO_NOEXCEPT_IN_RELEASE
        {
            return narrow_cast<int32_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        uint32_t operator"" _u32(unsigned long long number) YATO_NOEXCEPT_IN_RELEASE
        {
            return narrow_cast<uint32_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        int64_t operator"" _s64(unsigned long long number) YATO_NOEXCEPT_IN_RELEASE
        {
            return narrow_cast<int64_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        uint64_t operator"" _u64(unsigned long long number) YATO_NOEXCEPT_IN_RELEASE
        {
            return narrow_cast<uint64_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        float32_t operator"" _f32(long double number) YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return (static_cast<long double>(std::numeric_limits<float32_t>::lowest()) <= number && number <= static_cast<long double>(std::numeric_limits<float32_t>::max()))
                ? static_cast<float32_t>(number)
                : (YATO_THROW_ASSERT_EXCEPT("yato::literal _f32 is out of range!"), static_cast<float32_t>(0.0));
#else
            return static_cast<float32_t>(number);
#endif
        }

        YATO_CONSTEXPR_FUNC 
        float64_t operator"" _f64(long double number) YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return (static_cast<long double>(std::numeric_limits<float64_t>::lowest()) <= number && number <= static_cast<long double>(std::numeric_limits<float64_t>::max()))
                ? static_cast<float64_t>(number)
                : (YATO_THROW_ASSERT_EXCEPT("yato::literal _f64 is out of range!"), static_cast<float64_t>(0.0));
#else
            return static_cast<float64_t>(number);
#endif
        }

        YATO_CONSTEXPR_FUNC 
        float80_t operator"" _f80(long double number) YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return (static_cast<long double>(std::numeric_limits<float80_t>::lowest()) <= number && number <= static_cast<long double>(std::numeric_limits<float80_t>::max()))
                ? static_cast<float80_t>(number)
                : (YATO_THROW_ASSERT_EXCEPT("yato::literal _f80 is out of range!"), static_cast<float80_t>(0.0));
#else
            return static_cast<float80_t>(number);
#endif
        }
#endif
    }

    //--------------------------------------------------------------------

    /**
     *  Helper type to indicate in place constructors
     */
    struct in_place_t 
    {
        explicit in_place_t() = default;
    };
#ifndef YATO_MSVC_2013
    YATO_INLINE_VARIABLE
    constexpr yato::in_place_t in_place{};
#endif

    /**
     *  Helper type to indicate in place constructors
     */
    template <typename Ty>
    struct in_place_type_t
    {
        explicit in_place_type_t() = default;
    };
#ifndef YATO_MSVC_2013
    template <typename Ty>
    YATO_INLINE_VARIABLE
    constexpr yato::in_place_type_t<Ty> in_place_type{};
#endif

    /**
     *  Helper type to indicate in place constructors
     */
    template <size_t Idx>
    struct in_place_index_t
    {
        explicit in_place_index_t() = default;
    };
#ifndef YATO_MSVC_2013
    template <size_t Idx>
    YATO_INLINE_VARIABLE
    constexpr yato::in_place_index_t<Idx> in_place_index{};
#endif

    /**
     *  Helper types for constructors with variadic number of arguments
     */
    struct zero_arg_then_variadic_t 
    {
        explicit zero_arg_then_variadic_t() = default;
    };
#ifndef YATO_MSVC_2013
    YATO_INLINE_VARIABLE 
    constexpr yato::zero_arg_then_variadic_t zero_arg_then_variadic{};
#endif

    /**
     *  Helper types for constructors with variadic number of arguments
     */
    struct one_arg_then_variadic_t
    {
        explicit one_arg_then_variadic_t() = default;
    };
#ifndef YATO_MSVC_2013
    YATO_INLINE_VARIABLE
    constexpr yato::one_arg_then_variadic_t one_arg_then_variadic{};
#endif

}



#endif
