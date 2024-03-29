/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_TYPE_MATCH_H_
#define _YATO_TYPE_MATCH_H_

#include <tuple>
#include "type_traits.h"

namespace yato
{

    namespace details
    {
        template <typename _CasesTuple, size_t _CaseIdx = std::tuple_size<_CasesTuple>::value >
        struct match_dispatcher 
            : public match_dispatcher<_CasesTuple, _CaseIdx - 1>
        {
            using arg_type = typename yato::callable_trait<typename yato::remove_cvref<typename std::tuple_element<_CaseIdx - 1, _CasesTuple>::type>::type>::template arg<0>::type;
            using match_dispatcher<_CasesTuple, _CaseIdx - 1>::_invoke_case;
            //-------------------------------------------------------

            YATO_CONSTEXPR_FUNC
            auto _invoke_case(const _CasesTuple & functions, arg_type arg) const
#if YATO_MSVC == YATO_MSVC_2013
                -> typename yato::callable_trait<typename yato::remove_cvref<typename std::tuple_element<_CaseIdx - 1, _CasesTuple>::type>::type>::result_type
#else 
                -> decltype(auto)
#endif
            {
                return std::get<_CaseIdx - 1>(functions)(std::forward<arg_type>(arg));
            }
        };

        template <typename _CasesTuple> 
        struct match_dispatcher<_CasesTuple, 0>
        {
            void _invoke_case(); // dummy for 'using'
        };
    }

    template <typename... _Cases>
    class type_matcher
        : public details::match_dispatcher< std::tuple<const _Cases &...> >
    {
    private:
        using cases_tuple = std::tuple<const _Cases &...>;
        using dispatcher  = details::match_dispatcher<cases_tuple>;
        cases_tuple m_cases;

    public:
        YATO_CONSTEXPR_FUNC
        type_matcher(const cases_tuple & cases)
            : m_cases(cases)
        { }

        template <typename _MatchedType>
        YATO_CONSTEXPR_FUNC
        auto operator()(_MatchedType && val) const
#if YATO_MSVC == YATO_MSVC_2013
            -> decltype(std::declval<dispatcher>()._invoke_case(m_cases, std::forward<_MatchedType>(val)))
#else
            -> decltype(auto)
#endif
        {
            return dispatcher::_invoke_case(m_cases, std::forward<_MatchedType>(val));
        }
    };

    template <typename... _Cases>
    YATO_CONSTEXPR_FUNC
    type_matcher<_Cases...> match(const _Cases &... cases)
    {
        return type_matcher<_Cases...>(std::tuple<const _Cases &...>(cases...));
    }

    template <typename... _Cases>
    YATO_CONSTEXPR_FUNC
    type_matcher<_Cases...> match(std::tuple<const _Cases &...> cases)
    {
        return type_matcher<_Cases...>(cases);
    }

}

#endif
