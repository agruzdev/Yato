/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016-2018 Alexey Gruzdev
 */

#ifndef _YATO_TYPES_H_
#define _YATO_TYPES_H_

#include <cstdint>
#include <limits>

#include "assert.h"
#include "type_traits.h"

namespace yato
{
    //-------------------------------------------------------
    // Cast functions

    namespace details
    {
        template <typename Ty_, typename Uy_, typename = void>
        struct check_different_sign_narrowing
        {
            static YATO_CONSTEXPR_FUNC
            bool apply(Ty_, Uy_) {
                return true;
            }
        };

        template <typename Ty_, typename Uy_>
        struct check_different_sign_narrowing<
                Ty_, Uy_,
                std::enable_if_t<!yato::is_same_signedness<Ty_, Uy_>::value>
            >
        {
            static YATO_CONSTEXPR_FUNC
            bool apply(Ty_ t, Uy_ u)
            {
                return ((t < static_cast<Ty_>(0)) == (u < static_cast<Uy_>(0)));
            }
        };

    }

    template<typename TypeTo_, typename TypeFrom_>
    YATO_CONSTEXPR_FUNC YATO_FORCED_INLINE
    auto narrow_cast(const TypeFrom_ & val) YATO_NOEXCEPT_TESTED
        -> typename std::enable_if<std::is_arithmetic<TypeTo_>::value && std::is_arithmetic<TypeFrom_>::value, TypeTo_>::type
    {
        YATO_ASSERT_TESTED(val == static_cast<TypeFrom_>(static_cast<TypeTo_>(val)), "yato::narrow_cast failed!");
        YATO_ASSERT_TESTED((details::check_different_sign_narrowing<TypeTo_, TypeFrom_>::apply(static_cast<TypeTo_>(val), val)), "yato::narrow_cast failed!");
        return static_cast<TypeTo_>(val);
    }


    template<typename _TypeTo, typename _TypeFrom>
    YATO_CONSTEXPR_FUNC YATO_FORCED_INLINE
    _TypeTo pointer_cast(_TypeFrom* ptr) YATO_NOEXCEPT_KEYWORD
    {
        return static_cast<_TypeTo>(static_cast<void*>(ptr));
    }

    template<typename _TypeTo, typename _TypeFrom>
    YATO_CONSTEXPR_FUNC YATO_FORCED_INLINE
    _TypeTo pointer_cast(const _TypeFrom* ptr) YATO_NOEXCEPT_KEYWORD
    {
        return static_cast<_TypeTo>(static_cast<const void*>(ptr));
    }

    template<typename _TypeTo, typename _TypeFrom>
    YATO_CONSTEXPR_FUNC YATO_FORCED_INLINE
    _TypeTo pointer_cast(volatile _TypeFrom* ptr) YATO_NOEXCEPT_KEYWORD
    {
        return static_cast<_TypeTo>(static_cast<volatile void*>(ptr));
    }

    template<typename _TypeTo, typename _TypeFrom>
    YATO_CONSTEXPR_FUNC YATO_FORCED_INLINE
    _TypeTo pointer_cast(const volatile _TypeFrom* ptr) YATO_NOEXCEPT_KEYWORD
    {
        return static_cast<_TypeTo>(static_cast<const volatile void*>(ptr));
    }

    namespace literals
    {
#ifdef YATO_HAS_LITERALS

        YATO_CONSTEXPR_FUNC 
        int8_t operator"" _s8(unsigned long long number) YATO_NOEXCEPT_TESTED
        {
            return narrow_cast<int8_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        uint8_t operator"" _u8(unsigned long long number) YATO_NOEXCEPT_TESTED
        {
            return narrow_cast<uint8_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        int16_t operator"" _s16(unsigned long long number) YATO_NOEXCEPT_TESTED
        {
            return narrow_cast<int16_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        uint16_t operator"" _u16(unsigned long long number) YATO_NOEXCEPT_TESTED
        {
            return narrow_cast<uint16_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        int32_t operator"" _s32(unsigned long long number) YATO_NOEXCEPT_TESTED
        {
            return narrow_cast<int32_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        uint32_t operator"" _u32(unsigned long long number) YATO_NOEXCEPT_TESTED
        {
            return narrow_cast<uint32_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        int64_t operator"" _s64(unsigned long long number) YATO_NOEXCEPT_TESTED
        {
            return narrow_cast<int64_t>(number);
        }

        YATO_CONSTEXPR_FUNC 
        uint64_t operator"" _u64(unsigned long long number) YATO_NOEXCEPT_TESTED
        {
            return narrow_cast<uint64_t>(number);
        }

        YATO_CONSTEXPR_FUNC_EX
        float32_t operator"" _f32(long double number) YATO_NOEXCEPT_TESTED
        {
            YATO_REQUIRES(static_cast<long double>(std::numeric_limits<float32_t>::lowest()) <= number && number <= static_cast<long double>(std::numeric_limits<float32_t>::max()));
            return static_cast<float32_t>(number);
        }

        YATO_CONSTEXPR_FUNC_EX
        float64_t operator"" _f64(long double number) YATO_NOEXCEPT_TESTED
        {
            YATO_REQUIRES(static_cast<long double>(std::numeric_limits<float64_t>::lowest()) <= number && number <= static_cast<long double>(std::numeric_limits<float64_t>::max()));
            return static_cast<float64_t>(number);
        }

        YATO_CONSTEXPR_FUNC_EX
        float80_t operator"" _f80(long double number) YATO_NOEXCEPT_TESTED
        {
            YATO_REQUIRES(static_cast<long double>(std::numeric_limits<float80_t>::lowest()) <= number && number <= static_cast<long double>(std::numeric_limits<float80_t>::max()));
            return static_cast<float80_t>(number);
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
