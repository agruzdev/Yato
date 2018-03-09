/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/


#ifndef _YATO_ANY_MATCH_H_
#define _YATO_ANY_MATCH_H_

#include "any.h"
#include "case_dispatcher.h"

namespace yato
{

    template <typename CasesTuple_>
    class any_matcher final
        : public details::match_dispatcher<yato::any, CasesTuple_>
    {
    private:
        using dispatcher = details::match_dispatcher<yato::any, CasesTuple_>;
        CasesTuple_ m_cases;

    public:
        using result_type = typename dispatcher::result_type;

        constexpr explicit
        any_matcher(const CasesTuple_ & cases)
            : m_cases(cases)
        { }

        ~any_matcher() = default;

        any_matcher(const any_matcher&) = default;
        any_matcher(any_matcher&&) = default;

        any_matcher& operator=(const any_matcher&) = default;
        any_matcher& operator=(any_matcher&&) = default;


        result_type operator()(const yato::any & anyval) const
        {
            return dispatcher::match(m_cases, anyval);
        }

        result_type operator()(yato::any & anyval) const
        {
            return dispatcher::match(m_cases, std::move(anyval));
        }

        result_type operator()(yato::any && anyval) const
        {
            return dispatcher::match(m_cases, std::move(anyval));
        }
    };


    template <typename... Cases_>
    constexpr
    auto any_match(Cases_ && ... cases) {
        return any_matcher<std::tuple<Cases_...>>(std::tuple<Cases_...>(std::forward<Cases_>(cases)...));
    }

}

#endif

