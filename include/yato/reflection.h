/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_REFLECTION_H_
#define _YATO_REFLECTION_H_

#include <vector>
#include <map>
#include <memory>
#include <string>
#include "meta.h"
#include "singleton.h"
#include "range.h"

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

        /**
         *  Member reflection interface
         */
        template<typename _MyClass>
        struct member_info
        {
            using my_class = _MyClass;

            virtual ~member_info() {}

            /**
             *  Get member name
             */
            virtual std::string name() const = 0;

            /**
             *  Get pointer to member (only for data members)
             */
            virtual void* ptr(_MyClass*) const = 0;

            /**
             *  Safe access with type check
             */
            template <typename _T>
            auto as_ptr(_MyClass* obj) const
                -> typename std::enable_if<std::is_class<_T>::value, _T*>::type
            {
                auto _cast = dynamic_cast<_T*>(ptr(obj));
                if (_cast == nullptr) {
                    throw yato::assertion_error("yato::member_info[as_ptr]: bad cast!");
                }
                return _cast;
            }

            /**
             *  Safe access with type check
             */
            template <typename _T>
            auto as_ptr(_MyClass* obj) const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<!std::is_class<_T>::value, _T*>::type
            {
                return static_cast<_T*>(ptr(obj));
            }
        };

        template<typename _MyClass, typename _MyType>
        struct data_member_info final
            : public member_info<_MyClass>
        {
            using my_class = _MyClass;
            using my_type  = _MyType;

        private:
            const std::string m_name;
            _MyType _MyClass::* m_ptr;

        public:
            data_member_info(const std::string & name, _MyType _MyClass::* mem_ptr)
                : m_name(name), m_ptr(mem_ptr)
            { }

            ~data_member_info() override {}

            std::string name() const YATO_NOEXCEPT_KEYWORD override
            {
                return m_name;
            }

            void* ptr(_MyClass* obj) const override
            {
                return &(obj->*m_ptr);
            }
        };

        namespace details
        {
            /**
             *  Reflection manager class accumulating all meta information about a class
             */
            template <typename _Class>
            class reflection_manager_impl
            {
                using members_collection = std::map<std::string, std::unique_ptr<member_info<_Class> > >;
                using members_iterator = typename members_collection::const_iterator;

                members_collection m_members {};
                mutable bool m_inited = false;

                void _check_inited() const
                {
                    if (!m_inited) {
                        _Class::_yato_runtime_register(meta::Number<MAX_COUNTER_VALUE>{});
                        m_inited = true;
                    }
                }

                reflection_manager_impl(const reflection_manager_impl&) = delete;
                reflection_manager_impl(reflection_manager_impl&&) = delete;

                reflection_manager_impl& operator=(const reflection_manager_impl&) = delete;
                reflection_manager_impl& operator=(reflection_manager_impl&&) = delete;

            public:
                /**
                 *  Create empty manager
                 */
                YATO_CONSTEXPR_FUNC
                reflection_manager_impl()
                { }
                
                /**
                 *  Destroy
                 */
                ~reflection_manager_impl()
                { }

                /**
                 *  Iterate all members of the class
                 */
                yato::range<members_iterator> members() const
                {
                    _check_inited();
                    return make_range(m_members.cbegin(), m_members.cend());
                }
                /**
                 *  Get class member info by name
                 *  @return info pointer if it is found, nullptr else
                 */
                const member_info<_Class>* get_by_name(const std::string & name)
                {
                    _check_inited();
                    auto it = m_members.find(name);
                    return (it != m_members.cend()) 
                        ? it->second.get() 
                        : nullptr;
                }

                /**
                 *  Registration function for reflection
                 *  Don't call it manually!
                 */
                void _visit(std::unique_ptr<member_info<_Class>> && info)
                {
                    if (m_members.cend() != m_members.find(info->name())) {
                        throw yato::assertion_error("yato::reflection_manager_impl[visit]: Failed to register members with same name!");
                    }
                    m_members[info->name()] = std::move(info);

                }

                friend struct create_using_new<reflection_manager_impl<_Class>>;
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
    using _yato_reflected_##Var = yato::reflection::data_member_info<_yato_reflection_my_type, decltype(_yato_reflection_my_type::Var)>;\
    \
    static void _yato_runtime_register(yato::meta::Number<_yato_reflected_idx_##Var>)\
    {\
        _yato_runtime_register(yato::meta::Number<_yato_reflected_idx_##Var - 1>{});\
        yato::reflection::reflection_manager<_yato_reflection_my_type>::instance()->_visit(\
            std::make_unique<_yato_reflected_##Var>(#Var, &_yato_reflection_my_type::Var));\
    }


#define YATO_REFLECT_VAR_INLINE(Var) \
    Var; \
    YATO_REFLECT_VAR(Var)

#endif
