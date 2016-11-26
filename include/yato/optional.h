/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_OPTIONAL_H_
#define _YATO_OPTIONAL_H_

#if (defined(_MSC_VER) && (_MSC_VER > 1900)) ||\
    (defined(__cplusplus) && (__cplusplus >= 201700L))
# include <optional>
# define YATO_HAS_OPTIONAL
namespace yato
{
    using std::optional;
    using std::nullopt_t;
    using std::bad_optional_access;
    using std::make_optional;
    using std::in_place;
}
#elif defined(__GNUC__) &&\
     (defined(__cplusplus) && (__cplusplus >= 201400L))
# include <experimental/optional>
# define YATO_HAS_OPTIONAL
namespace yato
{
    using std::experimental::optional;
    using std::experimental::nullopt_t;
    using std::experimental::bad_optional_access;
    using std::experimental::make_optional;
    using std::experimental::in_place;

    template< class T, class... Args >
    constexpr std::experimental::optional<T> make_optional(Args&&... args)
    {
        return std::experimental::optional<T>(std::experimental::in_place, std::forward<Args>(args)...);
    }
}
#endif

#endif

