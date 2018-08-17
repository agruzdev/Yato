/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016-2018 Alexey Gruzdev
 */

#ifndef _YATO_PRIMITIVE_TYPES_H_
#define _YATO_PRIMITIVE_TYPES_H_

#include <cstdint>

namespace yato
{
    //-------------------------------------------------------
    // Fixed size integer types

    using int8_t = ::int8_t;
    static_assert(sizeof(int8_t) == 1, "Wrong int8_t type!");
    
    using uint8_t = ::uint8_t;
    static_assert(sizeof(uint8_t) == 1, "Wrong uint8_t type!");
    
    using int16_t = ::int16_t;
    static_assert(sizeof(int16_t) == 2, "Wrong int16_t type!");
    
    using uint16_t = ::uint16_t;
    static_assert(sizeof(uint16_t) == 2, "Wrong uint16_t type!");
    
    using int32_t = ::int32_t;
    static_assert(sizeof(int32_t) == 4, "Wrong int32_t type!");
    
    using uint32_t = ::uint32_t;
    static_assert(sizeof(uint32_t) == 4, "Wrong uint32_t type!");
    
    using int64_t = ::int64_t;
    static_assert(sizeof(int64_t) == 8, "Wrong int64_t type!");
    
    using uint64_t = ::uint64_t;
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
}

#endif
