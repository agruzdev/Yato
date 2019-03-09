/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2019 Alexey Gruzdev
*/


#ifndef _YATO_VARIANT_MATCH_H_
#define _YATO_VARIANT_MATCH_H_

#include "variant.h"
#include "case_dispatcher.h"

namespace yato
{

    template <typename CasesTuple_>
    class variant_matcher final
    {
    private:
        CasesTuple_ m_cases;

    public:
        constexpr explicit
        variant_matcher(const CasesTuple_ & cases)
            : m_cases(cases)
        { }

        ~variant_matcher() = default;

        variant_matcher(const variant_matcher&) = default;
        variant_matcher(variant_matcher&&) = default;

        variant_matcher& operator=(const variant_matcher&) = default;
        variant_matcher& operator=(variant_matcher&&) = default;


        template <typename... Alternatives_>
        decltype(auto) operator()(const yato::variant<Alternatives_...> & anyval) const
        {
            using dispatcher = details::match_dispatcher<yato::variant<Alternatives_...>, CasesTuple_>;
            return dispatcher::match(m_cases, anyval);
        }

        template <typename... Alternatives_>
        decltype(auto) operator()(yato::variant<Alternatives_...> & anyval) const
        {
            using dispatcher = details::match_dispatcher<yato::variant<Alternatives_...>, CasesTuple_>;
            return dispatcher::match(m_cases, std::move(anyval));
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
    auto variant_match(Cases_ && ... cases) {
        return variant_matcher<std::tuple<Cases_...>>(std::tuple<Cases_...>(std::forward<Cases_>(cases)...));
    }

}

#endif

