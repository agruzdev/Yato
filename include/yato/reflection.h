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
#include <string>
#include "meta.h"
#include "singleton.h"

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
        struct member_info final
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

        namespace details
        {
            /**
             *  Reflection manager class accumulating all meta information about a class
             */
            template <typename _T>
            class reflection_manager_impl
            {
                using members_array = std::vector< std::unique_ptr<member_info_base> >;
                members_array m_members{};

                bool m_inited = false;

                void _check_inited()
                {
                    if (!m_inited) {
                        _T::_yato_runtime_register(meta::Number<MAX_COUNTER_VALUE>{});
                        m_inited = true;
                    }
                }

                reflection_manager_impl(const reflection_manager_impl&) = delete;
                reflection_manager_impl(reflection_manager_impl&&) = delete;

                reflection_manager_impl& operator=(const reflection_manager_impl&) = delete;
                reflection_manager_impl& operator=(reflection_manager_impl&&) = delete;

            public:
                YATO_CONSTEXPR_FUNC
                reflection_manager_impl()
                { }

                ~reflection_manager_impl()
                { }

                void visit(std::unique_ptr<member_info_base> && info)
                {
                    m_members.push_back(std::move(info));
                }

                const members_array & members()
                {
                    _check_inited();
                    return m_members;
                }

                friend struct create_using_new<reflection_manager_impl<_T>>;
            };
        }
        template<typename _T>
        using reflection_manager = singleton_holder<details::reflection_manager_impl<_T>, create_using_new>;
      


        /**
         *  Type trait to check is a class has reflection info
         */
        template <typename _T, typename _Enable = void>
        struct is_reflected
            : std::false_type
        { };

        template <typename _T>
        struct is_reflected<_T, 
            typename std::enable_if<std::is_same<void, decltype(_yato_test_reflection_flag(std::declval<_T>()))>::value>::type
        >
            : std::true_type
        { };
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
    struct _yato_reflection_tag {}; \
    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(_yato_reflection_tag, 1)\
    friend class yato::reflection::details::reflection_manager_impl<_yato_reflection_my_type>;\
    friend void _yato_test_reflection_flag(_yato_reflection_my_type); \
    static void _yato_runtime_register(yato::meta::Number<0>) {}\

  /**
   *  Reflection for class members
   */
#define YATO_REFLECT_VAR(Var) \
    YATO_REFLECTION_GET_TYPED_COUNTER_VALUE(_yato_reflection_tag, _yato_reflected_idx_##Var)\
    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(_yato_reflection_tag, _yato_reflected_idx_##Var + 1)\
    \
    using _yato_reflected_##Var = yato::reflection::member_info<_yato_reflection_my_type, decltype(_yato_reflection_my_type::Var)>;\
    \
    static void _yato_runtime_register(yato::meta::Number<_yato_reflected_idx_##Var>)\
    {\
        _yato_runtime_register(yato::meta::Number<_yato_reflected_idx_##Var - 1>{});\
        yato::reflection::reflection_manager<_yato_reflection_my_type>::instance()->visit(std::make_unique<_yato_reflected_##Var>(#Var));\
    }


#define YATO_REFLECT_VAR_INLINE(Var) \
    Var; \
    YATO_REFLECT_VAR(Var)

#endif
