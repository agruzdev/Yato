/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_REFLECTION_H_
#define _YATO_REFLECTION_H_

#include "meta.h"

namespace yato
{
    namespace reflection
    {
        /**
         *  Type for counter value
         */
        using counter_type = uint16_t;

        /**
         *  Max counter value is limited by template instantiation max depth
         */
        static YATO_CONSTEXPR_VAR counter_type MAX_COUNTER_VALUE = 255;

        template<typename _MyClass, typename _MyType>
        struct member_info
        {
            using my_class = _MyClass;
            using my_type  = _MyType;
        };


        /**
         *  Reflection manager class accumulating all meta information about a class
         */
        template <typename _T>
        class reflection_manager
        {

        };
    }
}


/**
 *  Static counter
 *  Idea of B. Geller & A. Sermersheim "Compile-Time Counter Using Template & Constexpr Magic"
 *  https://www.youtube.com/watch?v=gI6Qtn4US9E
 */

#define YATO_REFLECTION_SET_COUNTER_VALUE(Value) \
    static YATO_CONSTEXPR_FUNC yato::meta::Number<(Value)> _yato_cs_meta_counter(yato::meta::Number<(Value)>);

#define YATO_REFLECTION_GET_COUNTER_VALUE(VariableName) \
    static YATO_CONSTEXPR_VAR yato::reflection::counter_type VariableName = decltype(_yato_cs_meta_counter(yato::meta::Number<yato::reflection::MAX_COUNTER_VALUE>{}))::value; 

#define YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(Type, Value) \
    static YATO_CONSTEXPR_FUNC yato::meta::Number<(Value)> _yato_cs_meta_counter(Type, yato::meta::Number<(Value)>);

#define YATO_REFLECTION_GET_TYPED_COUNTER_VALUE(Type, VariableName) \
    static YATO_CONSTEXPR_VAR yato::reflection::counter_type VariableName = decltype(_yato_cs_meta_counter(std::declval<Type>(), yato::meta::Number<yato::reflection::MAX_COUNTER_VALUE>{}))::value; 

/**
 *  Initialize reflection for a class
 */
#define YATO_REFLECT_CLASS(Class) \
    using _yato_reflection_my_type = Class;\
    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(_yato_reflection_my_type, 0);\
    friend class yato::reflection::reflection_manager<_yato_reflection_my_type>;

/**
 *  Reflection for class members
 */
#define YATO_REFLECT_VAR(Var) \
    using _yato_reflected_##Var = yato::reflection::member_info<_yato_reflection_my_type, decltype(_yato_reflection_my_type::Var)>;

#define YATO_REFLECT_VAR_INLINE(Var) \
    Var; \
    YATO_REFLECT_VAR(Var)

#endif
