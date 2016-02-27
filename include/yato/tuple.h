/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_INVOKE_H_
#define _YATO_INVOKE_H_

#include <tuple>
#include "prerequisites.h"

namespace yato
{

    namespace details
    {
        template<typename _Tuple, size_t _Length, size_t... _Idxs>
        struct tuple_transform_impl
        {
            template <template <typename> class _Func>
            YATO_CONSTEXPR_FUNC
            static auto apply(const _Tuple & tuple)
                -> decltype(tuple_transform_impl<_Tuple, _Length - 1, _Length - 1, _Idxs...>::template apply<_Func>(tuple))
            {
                return tuple_transform_impl<_Tuple, _Length - 1, _Length - 1, _Idxs...>::template apply<_Func>(tuple);
            }

            template <template <typename> class _Func>
            YATO_CONSTEXPR_FUNC
            static auto apply(_Tuple && tuple)
                -> decltype(tuple_transform_impl<_Tuple, _Length - 1, _Length - 1, _Idxs...>::template apply<_Func>(std::move(tuple)))
            {
                return tuple_transform_impl<_Tuple, _Length - 1, _Length - 1, _Idxs...>::template apply<_Func>(std::move(tuple));
            }
        };

        template<typename _Tuple, size_t... _Idxs>
        struct tuple_transform_impl<_Tuple, 0, _Idxs...>
        {
            template <template <typename> class _Func>
            YATO_CONSTEXPR_FUNC
            static auto apply(const _Tuple & tuple)
                -> decltype(std::make_tuple(_Func<typename std::tuple_element<_Idxs, _Tuple>::type>{}(std::get<_Idxs>(tuple))...))
            {
                return std::make_tuple(_Func<typename std::tuple_element<_Idxs, _Tuple>::type>{}(std::get<_Idxs>(tuple))...);
            }

            //Will compile only if tuple has 1 element
            template <template <typename> class _Func>
            YATO_CONSTEXPR_FUNC
            static auto apply(_Tuple && tuple)
                -> decltype(std::make_tuple(_Func<typename std::tuple_element<_Idxs, _Tuple>::type>{}(std::get<_Idxs>(std::move(tuple)))...))
            {
                return std::make_tuple(_Func<typename std::tuple_element<_Idxs, _Tuple>::type>{}(std::get<_Idxs>(std::move(tuple)))...);
            }
        };

    }
    /**
     *  Apply a template functor to each tuple element and get the new tuple
     */
    template<template <typename> class _Func, typename _Tuple>
    YATO_CONSTEXPR_FUNC
    auto tuple_transform(_Tuple && tuple)
        -> decltype(details::tuple_transform_impl<typename std::decay<_Tuple>::type, std::tuple_size<typename std::decay<_Tuple>::type>::value>::template apply<_Func>(std::forward<_Tuple>(tuple)))
    {
        return details::tuple_transform_impl<typename std::decay<_Tuple>::type, std::tuple_size<typename std::decay<_Tuple>::type>::value>::template apply<_Func>(std::forward<_Tuple>(tuple));
    }


    namespace details
    {
        template<typename _Tuple1, typename _Tuple2, size_t _Length, size_t... _Idxs>
        struct tuple_transform_2_impl
        {
            template <template <typename, typename> class _Func>
            YATO_CONSTEXPR_FUNC
            static auto apply(const _Tuple1 & tuple1, const _Tuple2 & tuple2)
                -> decltype(tuple_transform_2_impl<_Tuple1, _Tuple2, _Length - 1, _Length - 1, _Idxs...>::template apply<_Func>(tuple1, tuple2))
            {
                return tuple_transform_2_impl<_Tuple1, _Tuple2, _Length - 1, _Length - 1, _Idxs...>::template apply<_Func>(tuple1, tuple2);
            }
        };

        template<typename _Tuple1, typename _Tuple2, size_t... _Idxs>
        struct tuple_transform_2_impl<_Tuple1, _Tuple2, 0, _Idxs...>
        {
            template <template <typename, typename> class _Func>
            YATO_CONSTEXPR_FUNC
            static auto apply(const _Tuple1 & tuple1, const _Tuple2 & tuple2)
                -> decltype(std::make_tuple(_Func<typename std::tuple_element<_Idxs, _Tuple1>::type, typename std::tuple_element<_Idxs, _Tuple2>::type>{}(std::get<_Idxs>(tuple1), std::get<_Idxs>(tuple2))...))
            {
                return std::make_tuple(_Func<typename std::tuple_element<_Idxs, _Tuple1>::type, typename std::tuple_element<_Idxs, _Tuple2>::type>{}(std::get<_Idxs>(tuple1), std::get<_Idxs>(tuple2))...);
            }
        };

    }
    /**
     *  Apply a template binary functor to both tuple element and get tuple of functor results
     */
    template<template <typename, typename> class _Func, typename _Tuple1, typename _Tuple2>
    YATO_CONSTEXPR_FUNC
    auto tuple_transform(_Tuple1 && tuple1, _Tuple2 && tuple2)
        -> typename std::enable_if<(std::tuple_size<typename std::decay<_Tuple1>::type>::value == std::tuple_size<typename std::decay<_Tuple2>::type>::value),
            decltype(details::tuple_transform_2_impl<typename std::decay<_Tuple1>::type, typename std::decay<_Tuple2>::type, std::tuple_size<typename std::decay<_Tuple1>::type>::value>::template apply<_Func>(std::forward<_Tuple1>(tuple1), std::forward<_Tuple2>(tuple2)))>::type
    {
        return details::tuple_transform_2_impl<typename std::decay<_Tuple1>::type, typename std::decay<_Tuple2>::type, std::tuple_size<typename std::decay<_Tuple1>::type>::value>::template apply<_Func>(std::forward<_Tuple1>(tuple1), std::forward<_Tuple2>(tuple2));
    }

    namespace details
    {
        template<typename _Tuple, size_t _Length, size_t _Idx>
        struct tuple_for_each_impl
        {
            template <template <typename> class _Func>
            YATO_CONSTEXPR_FUNC
            static _Tuple & apply(_Tuple & tuple)
            {
                _Func<typename std::tuple_element<_Idx, _Tuple>::type>{}(std::get<_Idx>(tuple));
                return tuple_for_each_impl<_Tuple, _Length, _Idx + 1>::template apply<_Func>(tuple);
            }
        };

        template<typename _Tuple, size_t _Length>
        struct tuple_for_each_impl<_Tuple, _Length, _Length>
        {
            template <template <typename> class _Func>
            YATO_CONSTEXPR_FUNC
            static _Tuple & apply(_Tuple & tuple)
            {
                return tuple;
            }
        };
    }
    /**
     *  Apply functor to the each tuple value
     */
    template<template <typename> class _Func, typename _Tuple>
    YATO_CONSTEXPR_FUNC
    _Tuple & tuple_for_each(_Tuple & tuple)
    {
        return details::tuple_for_each_impl<typename std::decay<_Tuple>::type, std::tuple_size<_Tuple>::value, 0>::template apply<_Func>(tuple);
    }

    namespace details
    {
        template<typename _Tuple, size_t _Length, size_t _Idx>
        struct tuple_all_of_impl
        {
            template <template <typename> class _Pred>
            YATO_CONSTEXPR_FUNC
            static bool apply(const _Tuple & tuple)
            {
                return _Pred<typename std::tuple_element<_Idx, typename std::decay<_Tuple>::type>::type>{}(std::get<_Idx>(tuple)) &&
                    tuple_all_of_impl<_Tuple, _Length, _Idx + 1>::template apply<_Pred>(tuple);
            }
        };

        template<typename _Tuple, size_t _Length>
        struct tuple_all_of_impl<_Tuple, _Length, _Length>
        {
            template <template <typename> class _Pred>
            YATO_CONSTEXPR_FUNC
            static bool apply(const _Tuple & /*tuple*/)
            {
                return true;
            }
        };
    }
    /**
     *  Check if the each element of tuple satisfy a predicate
     */
    template<template <typename> class _Pred, typename _Tuple>
    YATO_CONSTEXPR_FUNC
    bool tuple_all_of(_Tuple && tuple)
    {
        return details::tuple_all_of_impl<typename std::decay<_Tuple>::type, std::tuple_size<typename std::decay<_Tuple>::type>::value, 0>::template apply<_Pred>(std::forward<_Tuple>(tuple));
    }

    namespace details
    {
        template<typename _Tuple, size_t _Length, size_t _Idx>
        struct tuple_any_of_impl
        {
            template <template <typename> class _Pred>
            YATO_CONSTEXPR_FUNC
            static bool apply(const _Tuple & tuple)
            {
                return _Pred<typename std::tuple_element<_Idx, typename std::decay<_Tuple>::type>::type>{}(std::get<_Idx>(tuple)) ||
                    tuple_any_of_impl<_Tuple, _Length, _Idx + 1>::template apply<_Pred>(tuple);
            }
        };

        template<typename _Tuple, size_t _Length>
        struct tuple_any_of_impl<_Tuple, _Length, _Length>
        {
            template <template <typename> class _Pred>
            YATO_CONSTEXPR_FUNC
            static bool apply(const _Tuple & /*tuple*/)
            {
                return false;
            }
        };
    }
    /**
     *  Check if at least one element of tuple satisfy a predicate
     */
    template<template <typename> class _Pred, typename _Tuple>
    YATO_CONSTEXPR_FUNC
    bool tuple_any_of(_Tuple && tuple)
    {
        return details::tuple_any_of_impl<typename std::decay<_Tuple>::type, std::tuple_size<typename std::decay<_Tuple>::type>::value, 0>::template apply<_Pred>(std::forward<_Tuple>(tuple));
    }
}

#endif
