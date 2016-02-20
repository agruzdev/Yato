/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_REFLECTION_H_
#define _YATO_REFLECTION_H_

#include <vector>
#include <memory>
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

        struct member_info_base
        {
            virtual ~member_info_base() {}

            virtual std::string name() const = 0;
        };

        template<typename _MyClass, typename _MyType>
        struct member_info /* final */
            : public member_info_base
        {
            using my_class = _MyClass;
            using my_type  = _MyType;

        private:
            const std::string m_name;

        public:
            member_info(const std::string & name)
                : m_name(name)
            { }

            ~member_info() override {}

            std::string name() const YATO_NOEXCEPT_KEYWORD override
            {
                return m_name;
            }
        };

        /**
         *  Reflection manager class accumulating all meta information about a class
         */
        template <typename _T>
        class reflection_manager
        {
            using members_array = std::vector< std::unique_ptr<member_info_base> >;
            static members_array m_members;

            static bool m_inited;
            static void _check_init()
            {
                if (!m_inited) {
                    _T::_yato_runtime_register(meta::Number<MAX_COUNTER_VALUE>{});
                }
            }

        public:
            static void visit(std::unique_ptr<member_info_base> && info)
            {
                m_members.push_back(std::move(info));
            }

            static const members_array & members()
            {
                _check_init();
                return m_members;
            }
        };

        template <typename _T>
        typename reflection_manager<_T>::members_array reflection_manager<_T>::m_members;

        template <typename _T>
        bool reflection_manager<_T>::m_inited = false;

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
    struct _yato_reflection_my_tag {}; \
    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(_yato_reflection_my_tag, 1)\
    friend class yato::reflection::reflection_manager<_yato_reflection_my_type>;\
    static void _yato_runtime_register(yato::meta::Number<0>) {}\

  /**
   *  Reflection for class members
   */
#define YATO_REFLECT_VAR(Var) \
    YATO_REFLECTION_GET_TYPED_COUNTER_VALUE(_yato_reflection_my_tag, _yato_reflected_idx_##Var)\
    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(_yato_reflection_my_tag, _yato_reflected_idx_##Var + 1)\
    \
    using _yato_reflected_##Var = yato::reflection::member_info<_yato_reflection_my_type, decltype(_yato_reflection_my_type::Var)>;\
    \
    static void _yato_runtime_register(yato::meta::Number<_yato_reflected_idx_##Var>)\
    {\
        _yato_runtime_register(yato::meta::Number<_yato_reflected_idx_##Var - 1>{});\
        yato::reflection::reflection_manager<_yato_reflection_my_type>::visit(std::make_unique<_yato_reflected_##Var>(#Var));\
    }


#define YATO_REFLECT_VAR_INLINE(Var) \
    Var; \
    YATO_REFLECT_VAR(Var)

#endif
