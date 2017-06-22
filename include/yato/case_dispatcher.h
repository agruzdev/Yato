/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_CASE_DISPATCHER_H_
#define _YATO_CASE_DISPATCHER_H_

#include <typeindex>
#include "assert.h"
#include "type_traits.h"

namespace yato
{
    /**
     * Indicates default case
     */
    struct match_default_t {};

    /**
     * Indicate case of empty any
     */
    struct match_empty_t {};


    /**
     * Thrown if match not found
     */
    class bad_match_error
        : public std::runtime_error
    {
    public:
        bad_match_error()
            : std::runtime_error("yato::bad_match_error")
        { }

        ~bad_match_error()
        { }
    };



    namespace details
    {
        YATO_INLINE_VARIABLE constexpr 
        size_t match_case_npos = std::numeric_limits<size_t>::max();

        template <typename Ty_, typename CasesTuple_, size_t Len_ = std::tuple_size<CasesTuple_>::value>
        struct find_match_case
        {
            using callable_type  = typename std::decay<typename std::tuple_element<Len_ - 1, CasesTuple_>::type>::type;
            using arg_type       = typename yato::callable_trait<callable_type>::template arg<0>::type;
            using arg_decay_type = typename std::decay<arg_type>::type;

            static constexpr size_t value = std::is_same<arg_decay_type, Ty_>::value
                ? Len_ - 1
                : find_match_case<Ty_, CasesTuple_, Len_ - 1>::value;
        };

        template <typename Ty_, typename CasesTuple_>
        struct find_match_case <Ty_, CasesTuple_, 0>
        {
            static constexpr size_t value = match_case_npos;
        };


        template <typename AnyTy_, typename CasesTuple_, typename OnDefault_, size_t Len_ = std::tuple_size<CasesTuple_>::value>
        struct match_dispatcher_impl
            : public match_dispatcher_impl<AnyTy_, CasesTuple_, OnDefault_, Len_ - 1>
        {
        private:
            static constexpr size_t case_index = std::tuple_size<CasesTuple_>::value - Len_;

            using next_dispatcher = match_dispatcher_impl<AnyTy_, CasesTuple_, OnDefault_, Len_ - 1>;
            using callable_type   = typename std::decay<typename std::tuple_element<case_index, CasesTuple_>::type>::type;
            using arg_type        = typename yato::callable_trait<callable_type>::template arg<0>::type;
            using arg_decay_type  = typename std::decay<arg_type>::type;
            //-------------------------------------------------------

            static
            decltype(auto) try_next_(const std::type_index & stored_type, const CasesTuple_ & functions, const OnDefault_ & on_default, const AnyTy_ & anyval)
            {
                return next_dispatcher::invoke_case(stored_type, functions, on_default, anyval);
            }

            static
            decltype(auto) try_next_(const std::type_index & stored_type, const CasesTuple_ & functions, const OnDefault_ & on_default, AnyTy_ && anyval)
            {
                return next_dispatcher::invoke_case(stored_type, functions, on_default, std::move(anyval));
            }
            //-------------------------------------------------------

            static
            decltype(auto) invoke_case_impl_(std::true_type /* invocable */, const std::type_index & stored_type, const CasesTuple_ & functions, const OnDefault_ & on_default, const AnyTy_ & anyval)
            {
                return (std::type_index(typeid(arg_decay_type)) == stored_type)
                    ? std::get<case_index>(functions)(anyval.template get_as_unsafe<arg_decay_type>())
                    : try_next_(stored_type, functions, on_default, anyval);
            }

            static
            decltype(auto) invoke_case_impl_(std::false_type /* invocable */, const std::type_index & stored_type, const CasesTuple_ & functions, const OnDefault_ & on_default, const AnyTy_ & anyval)
            {
                return try_next_(stored_type, functions, on_default, std::move(anyval));
            }

            static
            decltype(auto) invoke_case_impl_(std::true_type /* invocable */, const std::type_index & stored_type, const CasesTuple_ & functions, const OnDefault_ & on_default, AnyTy_ && anyval)
            {
                return (std::type_index(typeid(arg_decay_type)) == stored_type)
                    ? std::get<case_index>(functions)(static_cast<arg_type&&>(anyval.template get_as_unsafe<arg_decay_type>()))
                    : try_next_(stored_type, functions, on_default, std::move(anyval));
            }

            static
            decltype(auto) invoke_case_impl_(std::false_type /* invocable */, const std::type_index & stored_type, const CasesTuple_ & functions, const OnDefault_ & on_default, AnyTy_ && anyval)
            {
                return try_next_(stored_type, functions, on_default, std::move(anyval));
            }
            //-------------------------------------------------------

        public:
            static
            decltype(auto) invoke_case(const std::type_index & stored_type, const CasesTuple_ & functions, const OnDefault_ & on_default, const AnyTy_ & anyval)
            {
                using callable_trait = yato::is_invocable<callable_type, decltype(anyval.template get_as_unsafe<arg_decay_type>())>;
                return invoke_case_impl_(callable_trait{}, stored_type, functions, on_default, anyval);
            }

            static
            decltype(auto) invoke_case(const std::type_index & stored_type, const CasesTuple_ & functions, const OnDefault_ & on_default, AnyTy_ && anyval)
            {
                using callable_trait = yato::is_invocable<callable_type, decltype(static_cast<arg_type&&>(anyval.template get_as_unsafe<arg_decay_type>()))>;
                return invoke_case_impl_(callable_trait{}, stored_type, functions, on_default, std::move(anyval));
            }
        };

        template <typename AnyTy_, typename CasesTuple_, typename OnDefault_>
        struct match_dispatcher_impl<AnyTy_, CasesTuple_, OnDefault_, 0>
        {
            template <typename... Args_>
            static
            decltype(auto) invoke_case(const std::type_index &, const CasesTuple_ & cases, const OnDefault_ & on_default, Args_ && ...)
            {
                return on_default(cases);
            }
        };


        template <typename AnyTy_, typename CasesTuple_>
        struct match_dispatcher
        {
        private:
            static constexpr size_t empty_index   = details::find_match_case<match_empty_t, CasesTuple_>::value;
            static constexpr size_t default_index = details::find_match_case<match_default_t, CasesTuple_>::value;

            using has_empty_case   = std::integral_constant<bool, (empty_index != match_case_npos)>;
            using has_default_case = std::integral_constant<bool, (default_index != match_case_npos)>;

            static
            decltype(auto) on_default_impl_(std::true_type /*default case*/, const CasesTuple_ & cases)
            {
                return std::get<default_index>(cases)(match_default_t{});
            }

            static
            decltype(auto) on_default_impl_(std::false_type /*default case*/, const CasesTuple_ & /*cases*/)
            {
                throw yato::bad_match_error{};
            }

            static
            decltype(auto) on_default_(const CasesTuple_ & cases)
            {
                return on_default_impl_(has_default_case{}, cases);
            }

            using dispatcher = match_dispatcher_impl<AnyTy_, CasesTuple_, decltype(&on_default_)>;

            template <typename AnyRef_>
            static
            decltype(auto) match_impl_(std::true_type /*has empty case*/, const CasesTuple_ & cases, AnyRef_ && anyval)
            {
                return (std::type_index(typeid(void)) == std::type_index(anyval.type()))
                    ? std::get<empty_index>(cases)(match_empty_t{})
                    : dispatcher::invoke_case(std::type_index(anyval.type()), cases, &on_default_, std::forward<AnyRef_>(anyval));
            }

            template <typename AnyRef_>
            static
            decltype(auto) match_impl_(std::false_type /*has empty case*/, const CasesTuple_ & cases, AnyRef_ && anyval)
            {
                return (std::type_index(typeid(void)) == std::type_index(anyval.type()))
                    ? on_default_(cases)
                    : dispatcher::invoke_case(std::type_index(anyval.type()), cases, &on_default_, std::forward<AnyRef_>(anyval));
            }

        public:
            static
            decltype(auto) match(const CasesTuple_ & cases, const AnyTy_ & anyval)
            {
                return match_impl_(has_empty_case{}, cases, anyval);
            }

            static
            decltype(auto) match(const CasesTuple_ & cases, AnyTy_ && anyval)
            {
                return match_impl_(has_empty_case{}, cases, std::move(anyval));
            }
        };



    }
}

#endif

