/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/


#ifndef _YATO_VARIANT_MATCH_H_
#define _YATO_VARIANT_MATCH_H_

#include "variant.h"
#include "case_dispatcher.h"

namespace yato
{

    template <typename CasesTuple_>
    class variant_matcher
    {
    private:
        CasesTuple_ m_cases;

    public:
        constexpr explicit
        variant_matcher(const CasesTuple_ & cases)
            : m_cases(cases)
        { }

        template <typename... Alternatives_>
        decltype(auto) operator()(const yato::variant<Alternatives_...> & anyval) const
        {
            using dispatcher = details::match_dispatcher<yato::variant<Alternatives_...>, CasesTuple_>;
            return dispatcher::match(m_cases, anyval);
        }

        template <typename... Alternatives_>
        decltype(auto) operator()(yato::variant<Alternatives_...> && anyval) const
        {
            using dispatcher = details::match_dispatcher<yato::variant<Alternatives_...>, CasesTuple_>;
            return dispatcher::match(m_cases, std::move(anyval));
        }
    };

    template <typename... Cases_>
    constexpr
    auto variant_match(const Cases_ & ... cases) {
        return variant_matcher<std::tuple<const Cases_ &...>>(std::tuple<const Cases_ &...>(cases...));
    }


}

#endif

