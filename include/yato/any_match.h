/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/


#ifndef _YATO_ANY_MATCH_H_
#define _YATO_ANY_MATCH_H_

#include "any.h"
#include "case_dispatcher.h"

namespace yato
{

    template <typename CasesTuple_>
    class any_matcher
        : public details::match_dispatcher<yato::any, CasesTuple_>
    {
    private:
        using dispatcher = details::match_dispatcher<yato::any, CasesTuple_>;
        CasesTuple_ m_cases;

    public:
        constexpr explicit
        any_matcher(const CasesTuple_ & cases)
            : m_cases(cases)
        { }

        decltype(auto) operator()(const yato::any & anyval) const
        {
            return dispatcher::match(m_cases, anyval);
        }

        decltype(auto) operator()(yato::any && anyval) const
        {
            return dispatcher::match(m_cases, std::move(anyval));
        }
    };

    template <typename... Cases_>
    constexpr
    auto any_match(const Cases_ & ... cases) {
        return any_matcher<std::tuple<const Cases_ &...>>(std::tuple<const Cases_ &...>(cases...));
    }


}

#endif

