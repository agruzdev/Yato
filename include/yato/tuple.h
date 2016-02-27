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
            template <template <typename> class _Func, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static auto apply(_Tuple tuple, _Args && ...args)
                -> decltype(tuple_transform_impl<_Tuple, _Length - 1, _Length - 1, _Idxs...>::template apply<_Func>(tuple, std::forward<_Args>(args)...))
            {
                return tuple_transform_impl<_Tuple, _Length - 1, _Length - 1, _Idxs...>::template apply<_Func>(tuple, std::forward<_Args>(args)...);
            }
        };

        template<typename _Tuple, size_t... _Idxs>
        struct tuple_transform_impl<_Tuple, 0, _Idxs...>
        {
            using tuple_pure_type = typename std::decay<_Tuple>::type;

            template <template <typename> class _Func, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static auto apply(_Tuple tuple, _Args && ...args)
                -> std::tuple<decltype(_Func<typename std::tuple_element<_Idxs, tuple_pure_type>::type>{}(std::get<_Idxs>(tuple), std::forward<_Args>(args)...))...>
            {
                return std::tuple<decltype(_Func<typename std::tuple_element<_Idxs, tuple_pure_type>::type>{}(std::get<_Idxs>(tuple), std::forward<_Args>(args)...))...>(
                    _Func<typename std::tuple_element<_Idxs, tuple_pure_type>::type>{}(std::get<_Idxs>(tuple), std::forward<_Args>(args)...)...);
            }
        };

    }
    /**
     *  Apply a template functor to each tuple element and get the new tuple
     */
    template<template <typename> class _Func, typename _Tuple, typename... _Args>
    YATO_CONSTEXPR_FUNC
    auto tuple_transform(_Tuple && tuple, _Args && ...args)
        -> decltype(details::tuple_transform_impl<_Tuple, std::tuple_size<typename std::decay<_Tuple>::type>::value>::template apply<_Func>(std::forward<_Tuple>(tuple), std::forward<_Args>(args)...))
    {
        return details::tuple_transform_impl<_Tuple, std::tuple_size<typename std::decay<_Tuple>::type>::value>::template apply<_Func>(std::forward<_Tuple>(tuple), std::forward<_Args>(args)...);
    }


    namespace details
    {
        template<typename _Tuple1, typename _Tuple2, size_t _Length, size_t... _Idxs>
        struct tuple_transform_2_impl
        {
            template <template <typename, typename> class _Func, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static auto apply(_Tuple1 tuple1, _Tuple2 tuple2, _Args && ...args)
                -> decltype(tuple_transform_2_impl<_Tuple1, _Tuple2, _Length - 1, _Length - 1, _Idxs...>::template apply<_Func>(tuple1, tuple2, std::forward<_Args>(args)...))
            {
                return tuple_transform_2_impl<_Tuple1, _Tuple2, _Length - 1, _Length - 1, _Idxs...>::template apply<_Func>(tuple1, tuple2, std::forward<_Args>(args)...);
            }
        };

        template<typename _Tuple1, typename _Tuple2, size_t... _Idxs>
        struct tuple_transform_2_impl<_Tuple1, _Tuple2, 0, _Idxs...>
        {
            using tuple1_pure_type = typename std::decay<_Tuple1>::type;
            using tuple2_pure_type = typename std::decay<_Tuple2>::type;

            template <template <typename, typename> class _Func, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static auto apply(_Tuple1 tuple1, _Tuple2 tuple2, _Args && ...args)
                -> std::tuple<decltype(_Func<typename std::tuple_element<_Idxs, tuple1_pure_type>::type, typename std::tuple_element<_Idxs, tuple2_pure_type>::type>{}(std::get<_Idxs>(tuple1), std::get<_Idxs>(tuple2), std::forward<_Args>(args)...))...>
            {
                return std::tuple<decltype(_Func<typename std::tuple_element<_Idxs, tuple1_pure_type>::type, typename std::tuple_element<_Idxs, tuple2_pure_type>::type>{}(std::get<_Idxs>(tuple1), std::get<_Idxs>(tuple2), std::forward<_Args>(args)...))...>
                    (_Func<typename std::tuple_element<_Idxs, tuple1_pure_type>::type, typename std::tuple_element<_Idxs, tuple2_pure_type>::type>{}(std::get<_Idxs>(tuple1), std::get<_Idxs>(tuple2), std::forward<_Args>(args)...)...);
            }
        };

    }
    /**
     *  Apply a template binary functor to both tuple element and get tuple of functor results
     */
    template<template <typename, typename> class _Func, typename _Tuple1, typename _Tuple2, typename... _Args>
    YATO_CONSTEXPR_FUNC
    auto tuple_transform(_Tuple1 && tuple1, _Tuple2 && tuple2, _Args && ...args)
        -> typename std::enable_if<(std::tuple_size<typename std::decay<_Tuple1>::type>::value == std::tuple_size<typename std::decay<_Tuple2>::type>::value),
            decltype(details::tuple_transform_2_impl<_Tuple1, _Tuple2, std::tuple_size<typename std::decay<_Tuple1>::type>::value>::template apply<_Func>(std::forward<_Tuple1>(tuple1), std::forward<_Tuple2>(tuple2), std::forward<_Args>(args)...))>::type
    {
        return details::tuple_transform_2_impl<_Tuple1, _Tuple2, std::tuple_size<typename std::decay<_Tuple1>::type>::value>::template apply<_Func>(std::forward<_Tuple1>(tuple1), std::forward<_Tuple2>(tuple2), std::forward<_Args>(args)...);
    }

    namespace details
    {
        template<typename _Tuple, size_t _Length, size_t _Idx>
        struct tuple_for_each_impl
        {
            template <template <typename> class _Func, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static _Tuple & apply(_Tuple & tuple, _Args && ...args)
            {
                _Func<typename std::tuple_element<_Idx, _Tuple>::type>{}(std::get<_Idx>(tuple), std::forward<_Args>(args)...);
                return tuple_for_each_impl<_Tuple, _Length, _Idx + 1>::template apply<_Func>(tuple, std::forward<_Args>(args)...);
            }
        };

        template<typename _Tuple, size_t _Length>
        struct tuple_for_each_impl<_Tuple, _Length, _Length>
        {
            template <template <typename> class _Func, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static _Tuple & apply(_Tuple & tuple, _Args && .../*args*/)
            {
                return tuple;
            }
        };
    }
    /**
     *  Apply functor to the each tuple value
     */
    template<template <typename> class _Func, typename _Tuple, typename... _FuncArgs>
    YATO_CONSTEXPR_FUNC
    _Tuple & tuple_for_each(_Tuple & tuple, _FuncArgs && ...args)
    {
        return details::tuple_for_each_impl<typename std::decay<_Tuple>::type, std::tuple_size<_Tuple>::value, 0>::template apply<_Func>(tuple, std::forward<_FuncArgs>(args)...);
    }

    namespace details
    {
        template<typename _Tuple, size_t _Length, size_t _Idx>
        struct tuple_all_of_impl
        {
            template <template <typename> class _Pred, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static bool apply(_Tuple tuple, _Args && ...args)
            {
                return _Pred<typename std::tuple_element<_Idx, typename std::decay<_Tuple>::type>::type>{}(std::get<_Idx>(tuple), std::forward<_Args>(args)...) &&
                    tuple_all_of_impl<_Tuple, _Length, _Idx + 1>::template apply<_Pred>(tuple, std::forward<_Args>(args)...);
            }
        };

        template<typename _Tuple, size_t _Length>
        struct tuple_all_of_impl<_Tuple, _Length, _Length>
        {
            template <template <typename> class _Pred, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static bool apply(_Tuple /*tuple*/, _Args && .../*args*/)
            {
                return true;
            }
        };
    }
    /**
     *  Check if the each element of tuple satisfy a predicate
     */
    template<template <typename> class _Pred, typename _Tuple, typename... _PredArgs>
    YATO_CONSTEXPR_FUNC
    bool tuple_all_of(_Tuple && tuple, _PredArgs && ...args)
    {
        return details::tuple_all_of_impl<_Tuple, std::tuple_size<typename std::decay<_Tuple>::type>::value, 0>::template apply<_Pred>(std::forward<_Tuple>(tuple), std::forward<_PredArgs>(args)...);
    }

    namespace details
    {
        template<typename _Tuple1, typename _Tuple2, size_t _Length, size_t _Idx>
        struct tuple_all_of_2_impl
        {
            using tuple1_pure_type = typename std::decay<_Tuple1>::type;
            using tuple2_pure_type = typename std::decay<_Tuple2>::type;

            template <template <typename, typename> class _Pred, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static bool apply(_Tuple1 tuple1, _Tuple2 tuple2, _Args && ...args)
            {
                return _Pred<typename std::tuple_element<_Idx, tuple1_pure_type>::type, typename std::tuple_element<_Idx, tuple2_pure_type>::type>{}(std::get<_Idx>(tuple1), std::get<_Idx>(tuple2), std::forward<_Args>(args)...) &&
                    tuple_all_of_2_impl<_Tuple1, _Tuple2, _Length, _Idx + 1>::template apply<_Pred>(tuple1, tuple2, std::forward<_Args>(args)...);
            }
        };

        template<typename _Tuple1, typename _Tuple2, size_t _Length>
        struct tuple_all_of_2_impl<_Tuple1, _Tuple2, _Length, _Length>
        {
            template <template <typename, typename> class _Pred, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static bool apply(_Tuple1 /*tuple1*/, _Tuple2 /*tuple2*/, _Args && .../*args*/)
            {
                return true;
            }
        };
    }
    /**
    *  Check if the each element of two tuples satisfy a binary predicate
    */
    template<template <typename, typename> class _Pred, typename _Tuple1, typename _Tuple2, typename... _PredArgs>
    YATO_CONSTEXPR_FUNC
    auto tuple_all_of(_Tuple1 && tuple1, _Tuple2 && tuple2, _PredArgs && ...args)
        -> typename std::enable_if<(std::tuple_size<typename std::decay<_Tuple1>::type>::value == std::tuple_size<typename std::decay<_Tuple2>::type>::value), bool>::type
    {
        return details::tuple_all_of_2_impl<_Tuple1, _Tuple2, std::tuple_size<typename std::decay<_Tuple1>::type>::value, 0>::template apply<_Pred>(std::forward<_Tuple1>(tuple1), std::forward<_Tuple2>(tuple2), std::forward<_PredArgs>(args)...);
    }


    namespace details
    {
        template<typename _Tuple, size_t _Length, size_t _Idx>
        struct tuple_any_of_impl
        {
            template <template <typename> class _Pred, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static bool apply(_Tuple tuple, _Args && ...args)
            {
                return _Pred<typename std::tuple_element<_Idx, typename std::decay<_Tuple>::type>::type>{}(std::get<_Idx>(tuple), std::forward<_Args>(args)...) ||
                    tuple_any_of_impl<_Tuple, _Length, _Idx + 1>::template apply<_Pred>(tuple, std::forward<_Args>(args)...);
            }
        };

        template<typename _Tuple, size_t _Length>
        struct tuple_any_of_impl<_Tuple, _Length, _Length>
        {
            template <template <typename> class _Pred, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static bool apply(_Tuple /*tuple*/, _Args && .../*args*/)
            {
                return false;
            }
        };
    }
    /**
     *  Check if at least one element of tuple satisfy a predicate
     */
    template<template <typename> class _Pred, typename _Tuple, typename... _PredArgs>
    YATO_CONSTEXPR_FUNC
    bool tuple_any_of(_Tuple && tuple, _PredArgs && ...args)
    {
        return details::tuple_any_of_impl<_Tuple, std::tuple_size<typename std::decay<_Tuple>::type>::value, 0>::template apply<_Pred>(std::forward<_Tuple>(tuple), std::forward<_PredArgs>(args)...);
    }


    namespace details
    {
        template<typename _Tuple1, typename _Tuple2, size_t _Length, size_t _Idx>
        struct tuple_any_of_2_impl
        {
            using tuple1_pure_type = typename std::decay<_Tuple1>::type;
            using tuple2_pure_type = typename std::decay<_Tuple2>::type;

            template <template <typename, typename> class _Pred, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static bool apply(_Tuple1 tuple1, _Tuple2 tuple2, _Args && ...args)
            {
                return _Pred<typename std::tuple_element<_Idx, tuple1_pure_type>::type, typename std::tuple_element<_Idx, tuple2_pure_type>::type>{}(std::get<_Idx>(tuple1), std::get<_Idx>(tuple2), std::forward<_Args>(args)...) ||
                    tuple_any_of_2_impl<_Tuple1, _Tuple2, _Length, _Idx + 1>::template apply<_Pred>(tuple1, tuple2, std::forward<_Args>(args)...);
            }
        };

        template<typename _Tuple1, typename _Tuple2, size_t _Length>
        struct tuple_any_of_2_impl<_Tuple1, _Tuple2, _Length, _Length>
        {
            template <template <typename, typename> class _Pred, typename... _Args>
            YATO_CONSTEXPR_FUNC
            static bool apply(_Tuple1 /*tuple1*/, _Tuple2 /*tuple2*/, _Args && .../*args*/)
            {
                return false;
            }
        };
    }
    /**
     *  Check if at least one element of two tuples satisfy a binary predicate
     */
    template<template <typename, typename> class _Pred, typename _Tuple1, typename _Tuple2, typename... _PredArgs>
    YATO_CONSTEXPR_FUNC
    auto tuple_any_of(_Tuple1 && tuple1, _Tuple2 && tuple2, _PredArgs && ...args)
        -> typename std::enable_if<(std::tuple_size<typename std::decay<_Tuple1>::type>::value == std::tuple_size<typename std::decay<_Tuple2>::type>::value), bool>::type
    {
        return details::tuple_any_of_2_impl<_Tuple1, _Tuple2, std::tuple_size<typename std::decay<_Tuple1>::type>::value, 0>::template apply<_Pred>(std::forward<_Tuple1>(tuple1), std::forward<_Tuple2>(tuple2), std::forward<_PredArgs>(args)...);
    }
}

#endif
