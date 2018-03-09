/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
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

            virtual ~member_info() = default;

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
                    throw yato::runtime_error("yato::member_info[as_ptr]: bad cast!");
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

        //-------------------------------------------------------
        // Data members

        template<typename _MyClass, typename _MyType, counter_type _Idx>
        struct data_member_info final
            : public member_info<_MyClass>
        {
            using my_class = _MyClass;
            using my_type  = _MyType;

            static YATO_CONSTEXPR_VAR counter_type my_idx = _Idx;
            //-------------------------------------------------------

        private:
            const std::string m_name;
            _MyType _MyClass::* m_ptr;
            //-------------------------------------------------------

        public:
            data_member_info(const std::string & name, _MyType _MyClass::* mem_ptr)
                : m_name(name), m_ptr(mem_ptr)
            { }

            ~data_member_info() override = default;

            std::string name() const YATO_NOEXCEPT_KEYWORD override
            {
                return m_name;
            }

            void* ptr(_MyClass* obj) const override
            {
                return &(obj->*m_ptr);
            }
        };

        //-------------------------------------------------------
        // methods

        

        template<typename _MyClass, typename _MyType, counter_type _Idx>
        struct member_function_info final
            : public member_info<_MyClass>
        {
            using my_class = _MyClass;
            using my_type = _MyType;
            using result_type = typename yato::callable_trait<my_type>::result_type;
            using arguments_list = typename yato::callable_trait<my_type>::arguments_list;

            static YATO_CONSTEXPR_VAR counter_type my_idx = _Idx;
            //-------------------------------------------------------

        private:
            const std::string m_name;
            //-------------------------------------------------------

        public:
            member_function_info(const std::string & name)
                : m_name(name)
            { }

            ~member_function_info() override = default;

            std::string name() const YATO_NOEXCEPT_KEYWORD override
            {
                return m_name;
            }

            void* ptr(_MyClass*) const override
            {
                return nullptr;
            }
        };

        //-------------------------------------------------------
        namespace details
        {
            /**
             *  Reflection manager class accumulating all meta information about a class
             */
            template <typename _Class>
            class reflection_manager_impl final
            {
            public:
                using class_type = _Class;
                using data_members_list = decltype(_Class::_yato_data_members_list_getter(meta::number<MAX_COUNTER_VALUE>{}));
                using member_functions_list = decltype(_Class::_yato_member_functions_list_getter(meta::number<MAX_COUNTER_VALUE>{}));
                //-------------------------------------------------------

            private:
                using members_collection = std::map<std::string, std::unique_ptr<member_info<_Class> > >;
                using members_iterator = typename members_collection::const_iterator;

                members_collection m_members {};
                mutable bool m_inited = false;
                //-------------------------------------------------------

                void _check_inited() const
                {
                    if (!m_inited) {
                        _Class::_yato_runtime_register(meta::number<MAX_COUNTER_VALUE>{});
                        m_inited = true;
                    }
                }

                reflection_manager_impl(const reflection_manager_impl&) = delete;
                reflection_manager_impl(reflection_manager_impl&&) = delete;

                reflection_manager_impl& operator=(const reflection_manager_impl&) = delete;
                reflection_manager_impl& operator=(reflection_manager_impl&&) = delete;
                //-------------------------------------------------------

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
                void _register_member(std::unique_ptr<member_info<_Class>> && info)
                {
                    if (m_members.cend() != m_members.find(info->name())) {
                        throw yato::runtime_error("yato::reflection_manager_impl[_register_member]: Failed to register members with same name!");
                    }
                    m_members[info->name()] = std::move(info);
                }

                friend struct create_using_new<reflection_manager_impl<_Class>>;
            };
        }
        template<typename _T>
        using reflection_manager = singleton_holder<details::reflection_manager_impl<_T>, create_using_new>;
      

        template<typename _T>
        std::false_type _yato_test_reflection_flag(_T);

        /**
        *  Type trait to check is a class has reflection info
        */
        template <typename _T>
        struct is_reflected
            : decltype(_yato_test_reflection_flag(std::declval<_T>()))
        { };

        template <>
        struct is_reflected<void>
            : std::false_type
        { };

        //ToDo: Accessing private member doesn't cause substitution fail in MSVC 2013 or MinGW 
#ifdef YATO_MSVC_2015
        /**
         *  Check if the class member with specified reflection index is public
         *  <=> is accessible from out of the class
         */
        template <typename _T, counter_type _MemberIdx, typename _Enable = void>
        struct is_public
            : std::false_type
        { };

        template <typename _T, counter_type _MemberIdx>
        struct is_public<_T, _MemberIdx, decltype(_T::template _yato_access_tag<_MemberIdx>())>
            : std::true_type
        { };
#endif

    }
}


/**
 *  Static counter
 *  Idea of B. Geller & A. Sermersheim "Compile-Time Counter Using Template & Constexpr Magic"
 *  https://www.youtube.com/watch?v=gI6Qtn4US9E
 */

#define YATO_REFLECTION_SET_COUNTER_VALUE(Value) \
    static YATO_CONSTEXPR_FUNC yato::meta::number<(Value)> _yato_cs_meta_counter(yato::meta::number<(Value)>);

#define YATO_REFLECTION_GET_COUNTER_VALUE(VariableName) \
    static YATO_CONSTEXPR_VAR yato::reflection::counter_type VariableName = decltype(_yato_cs_meta_counter(yato::meta::number<yato::reflection::MAX_COUNTER_VALUE>{}))::value; 

#define YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(Type, Value) \
    static YATO_CONSTEXPR_FUNC yato::meta::number<(Value)> _yato_cs_meta_counter(Type, yato::meta::number<(Value)>);

#define YATO_REFLECTION_GET_TYPED_COUNTER_VALUE(Type, VariableName) \
    static YATO_CONSTEXPR_VAR yato::reflection::counter_type VariableName = decltype(_yato_cs_meta_counter(std::declval<Type>(), yato::meta::number<yato::reflection::MAX_COUNTER_VALUE>{}))::value; 

 /**
  *  Initialize reflection for a class
  */
#define YATO_REFLECT_CLASS(Class) \
    /* define current class */ \
    using _yato_reflection_my_type = Class;\
    struct _yato_reflection_tag {}; \
    \
    /* set base for registration function and counter */ \
    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(_yato_reflection_tag, 1)\
    static void _yato_runtime_register(yato::meta::number<0>) {}\
    \
    /* base declarator for static list of member info */ \
    template <typename _Yato_Typename_Dummy = void> \
    static yato::meta::null_list _yato_data_members_list_getter(yato::meta::number<0>); \
    template <typename _Yato_Typename_Dummy = void> \
    static yato::meta::null_list _yato_member_functions_list_getter(yato::meta::number<0>); \
    \
    /* declare manager as friend to have access to registration functions */ \
    friend class yato::reflection::details::reflection_manager_impl<_yato_reflection_my_type>;\
    \
    /* friend function for the reflected trait; reachable by ADL */ \
    friend std::true_type _yato_test_reflection_flag(_yato_reflection_my_type); \

  /**
   *  Reflection for class members
   */
#define YATO_REFLECT_VAR(Var) \
    /* increment counter */ \
    YATO_REFLECTION_GET_TYPED_COUNTER_VALUE(_yato_reflection_tag, _yato_reflected_idx_##Var)\
    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(_yato_reflection_tag, _yato_reflected_idx_##Var + 1)\
    \
    /* static reflection info */ \
    using _yato_reflected_##Var = yato::reflection::data_member_info<_yato_reflection_my_type, decltype(_yato_reflection_my_type::Var), _yato_reflected_idx_##Var>;\
    static auto _yato_data_members_list_getter(yato::meta::number<_yato_reflected_idx_##Var>) \
        -> typename yato::meta::list_push_back<decltype(_yato_data_members_list_getter(yato::meta::number<_yato_reflected_idx_##Var - 1>{})), _yato_reflected_##Var>::type; \
    \
    /* dynamic reflection info */ \
    static void _yato_runtime_register(yato::meta::number<_yato_reflected_idx_##Var>)\
    {\
        _yato_runtime_register(yato::meta::number<_yato_reflected_idx_##Var - 1>{});\
        yato::reflection::reflection_manager<_yato_reflection_my_type>::instance()->_register_member(\
            std::make_unique<_yato_reflected_##Var>(#Var, &_yato_reflection_my_type::Var));\
    }\
    /* tag for checking access */ \
    template <yato::reflection::counter_type _Idx = _yato_reflected_idx_##Var>\
    static auto _yato_access_tag() -> typename std::enable_if<(_Idx == _yato_reflected_idx_##Var), void>::type;


#define YATO_REFLECT_VAR_INLINE(Var) \
    Var; \
    YATO_REFLECT_VAR(Var)

#define YATO_REFLECT_VAR_INLINE_INIT(Var, Initializer) \
    Var { Initializer }; \
    YATO_REFLECT_VAR(Var)


   /**
    *  Reflection for methods
    */
#define YATO_REFLECT_METHOD(Method) \
    /* increment counter */ \
    YATO_REFLECTION_GET_TYPED_COUNTER_VALUE(_yato_reflection_tag, _yato_reflected_fidx_##Method)\
    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(_yato_reflection_tag, _yato_reflected_fidx_##Method + 1)\
    \
    /* static reflection info */ \
    template <typename _Yato_Dummy_Argument = void>\
    using _yato_reflected_method_impl_##Method = yato::reflection::member_function_info<_yato_reflection_my_type, typename std::remove_pointer<decltype(&_yato_reflection_my_type::Method)>::type, _yato_reflected_fidx_##Method>;\
    using _yato_reflected_method_##Method = _yato_reflected_method_impl_##Method<void>;\
    static auto _yato_member_functions_list_getter(yato::meta::number<_yato_reflected_fidx_##Method>) \
        -> typename yato::meta::list_push_back<decltype(_yato_data_members_list_getter(yato::meta::number<_yato_reflected_fidx_##Method - 1>{})), _yato_reflected_method_##Method>::type; \


#endif
