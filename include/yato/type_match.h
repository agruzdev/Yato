/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016 Alexey Gruzdev
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
            using arg_type = typename yato::callable_trait<typename std::decay<typename std::tuple_element<_CaseIdx - 1, _CasesTuple>::type>::type>::template arg<0>::type;
            using match_dispatcher<_CasesTuple, _CaseIdx - 1>::_case;

            YATO_CONSTEXPR_FUNC
            auto _case(const _CasesTuple & functions, arg_type arg) const
#ifdef YATO_MSVC_2013
                -> typename yato::callable_trait<typename std::decay<typename std::tuple_element<_CaseIdx - 1, _CasesTuple>::type>::type>::result_type
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
            void _case(); // dummy for 'using'
        };
    }

#pragma warning(push)
#pragma warning(disable: 4814) // in C++14 'constexpr' will not imply 'const'; consider explicitly specifying 'const'
    template <typename _ValueRef>
    struct matcher
    {
        _ValueRef m_value;

    public:
        YATO_CONSTEXPR_FUNC
        matcher(_ValueRef v) 
            : m_value(v)
        { }
        
        template <typename... _Cases>
        YATO_CONSTEXPR_FUNC
        auto operator()(const _Cases & ...cases)
#ifdef YATO_MSVC_2013
            -> decltype(details::match_dispatcher< std::tuple<const _Cases & ...> >()._case(std::make_tuple(cases...), m_value))
#else
            -> decltype(auto)
#endif
        {
            return details::match_dispatcher< std::tuple<const _Cases & ...> >()._case(std::make_tuple(cases...), m_value);
        }
    };
#pragma warning(pop)

    template <typename _T>
    YATO_CONSTEXPR_FUNC
    matcher<_T> match(_T && value)
    {
        return matcher<_T>{std::forward<_T>(value)};
    }

}

#endif
