/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_TUPLE_H_
#define _YATO_TUPLE_H_

#include <tuple>
#include "prerequisites.h"
#include "type_traits.h"

namespace yato
{

    namespace details
    {
        template <size_t Length_, size_t... Idxs_>
        struct tuple_transform_impl
        {
            template <typename Tuple_, typename Func_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            decltype(auto) apply(Tuple_ && tuple, Func_ && functor, Args_ &&... args)
            {
                return tuple_transform_impl<Length_ - 1, Length_ - 1, Idxs_...>::apply(std::forward<Tuple_>(tuple), std::forward<Func_>(functor), std::forward<Args_>(args)...);
            }
        };

        template<size_t... Idxs_>
        struct tuple_transform_impl<0, Idxs_...>
        {
            template <typename Tuple_, typename Func_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            decltype(auto) apply(Tuple_ && tuple, Func_ && functor, Args_ &&... args)
            {
                return std::tuple<decltype(functor(std::get<Idxs_>(std::forward<Tuple_>(tuple)), std::forward<Args_>(args)...))...>(
                    functor(std::get<Idxs_>(std::forward<Tuple_>(tuple)), std::forward<Args_>(args)...)...);
            }
        };

    }

    /**
     *  Apply a template functor to each tuple element and get the new tuple
     */
    template<typename Tuple_, typename Func_, typename... Args_>
    YATO_CONSTEXPR_FUNC
    decltype(auto) tuple_transform(Tuple_ && tuple, Func_ && functor, Args_ &&... args)
    {
        return details::tuple_transform_impl<std::tuple_size<yato::remove_cvref_t<Tuple_>>::value>::apply(std::forward<Tuple_>(tuple), std::forward<Func_>(functor), std::forward<Args_>(args)...);
    }


    namespace details
    {
        template<size_t Length_, size_t... Idxs_>
        struct tuple_transform_2_impl
        {
            template <typename Tuple1_, typename Tuple2_, typename Func_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            decltype(auto) apply(Tuple1_ && tuple1, Tuple2_ && tuple2, Func_ && functor, Args_ &&... args)
            {
                return tuple_transform_2_impl<Length_ - 1, Length_ - 1, Idxs_...>::apply(std::forward<Tuple1_>(tuple1), std::forward<Tuple2_>(tuple2), std::forward<Func_>(functor), std::forward<Args_>(args)...);
            }
        };

        template<size_t... Idxs_>
        struct tuple_transform_2_impl<0, Idxs_...>
        {
            template <typename Tuple1_, typename Tuple2_, typename Func_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            decltype(auto) apply(Tuple1_ && tuple1, Tuple2_ && tuple2, Func_ && functor, Args_ &&... args)
            {
                return std::tuple<decltype(functor(std::get<Idxs_>(std::forward<Tuple1_>(tuple1)), std::get<Idxs_>(std::forward<Tuple2_>(tuple2)), std::forward<Args_>(args)...))...>(
                    functor(std::get<Idxs_>(std::forward<Tuple1_>(tuple1)), std::get<Idxs_>(std::forward<Tuple2_>(tuple2)), std::forward<Args_>(args)...)...);
            }
        };

    }

    /**
     *  Apply a template binary functor to both tuple element and get tuple of functor results
     */
    template <typename Tuple1_, typename Tuple2_, typename Func_, typename... Args_>
    YATO_CONSTEXPR_FUNC
    auto tuple_transform(Tuple1_ && tuple1, Tuple2_ && tuple2, Func_ && functor, Args_ &&... args)
        -> std::enable_if_t<
            std::tuple_size<yato::remove_cvref_t<Tuple1_>>::value == std::tuple_size<yato::remove_cvref_t<Tuple2_>>::value,
            decltype(details::tuple_transform_2_impl<std::tuple_size<yato::remove_cvref_t<Tuple1_>>::value>::apply(std::forward<Tuple1_>(tuple1), std::forward<Tuple2_>(tuple2), std::forward<Func_>(functor), std::forward<Args_>(args)...))
        >
    {
        return details::tuple_transform_2_impl<std::tuple_size<yato::remove_cvref_t<Tuple1_>>::value>::apply(std::forward<Tuple1_>(tuple1), std::forward<Tuple2_>(tuple2), std::forward<Func_>(functor), std::forward<Args_>(args)...);
    }


    namespace details
    {
        template <size_t Length_, size_t Idx_>
        struct tuple_for_each_impl
        {
            template <typename Tuple_, typename Func_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            auto apply(Tuple_ && tuple, Func_ && functor, Args_ &&... args)
                -> decltype(std::forward<Func_>(functor))
            {
                return functor(std::get<Idx_>(std::forward<Tuple_>(tuple)), std::forward<Args_>(args)...),
                       tuple_for_each_impl<Length_, Idx_ + 1>::apply(std::forward<Tuple_>(tuple), std::forward<Func_>(functor), std::forward<Args_>(args)...);
            }
        };

        template <size_t Length_>
        struct tuple_for_each_impl<Length_, Length_>
        {
            template <typename Tuple_, typename Func_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            auto apply(Tuple_&&, Func_ && functor, Args_&&...)
                -> decltype(std::forward<Func_>(functor))
            {
                return std::forward<Func_>(functor);
            }
        };
    }

    /**
     *  Apply functor to the each tuple value
     */
    template<typename Tuple_, typename Func_, typename... Args_>
    YATO_CONSTEXPR_FUNC
    auto tuple_for_each(Tuple_ && tuple, Func_ && functor, Args_ &&... args)
        -> decltype(std::forward<Func_>(functor))
    {
        return details::tuple_for_each_impl<std::tuple_size<yato::remove_cvref_t<Tuple_>>::value, 0>::apply(std::forward<Tuple_>(tuple), std::forward<Func_>(functor), std::forward<Args_>(args)...);
    }


    namespace details
    {
        template<size_t Length_, size_t Idx_>
        struct tuple_all_of_impl
        {
            template <typename Tuple_, typename Pred_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            bool apply(Tuple_ && tuple, Pred_ && predicate, Args_ &&... args)
            {
                return predicate(std::get<Idx_>(std::forward<Tuple_>(tuple)), std::forward<Args_>(args)...) &&
                    tuple_all_of_impl<Length_, Idx_ + 1>::apply(std::forward<Tuple_>(tuple), std::forward<Pred_>(predicate), std::forward<Args_>(args)...);
            }
        };

        template<size_t Length_>
        struct tuple_all_of_impl<Length_, Length_>
        {
            template <typename Tuple_, typename Pred_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            bool apply(Tuple_ &&, Pred_ &&, Args_ &&...)
            {
                return true;
            }
        };
    }

    /**
     *  Check if the each element of tuple satisfy a predicate
     */
    template <typename Tuple_, typename Pred_, typename... Args_>
    YATO_CONSTEXPR_FUNC
    bool tuple_all_of(Tuple_ && tuple, Pred_ && predicate, Args_ &&... args)
    {
        return details::tuple_all_of_impl<std::tuple_size<typename yato::remove_cvref_t<Tuple_>>::value, 0>::apply(std::forward<Tuple_>(tuple), std::forward<Pred_>(predicate), std::forward<Args_>(args)...);
    }


    namespace details
    {
        template <size_t Length_, size_t Idx_>
        struct tuple_all_of_2_impl
        {
            template <typename Tuple1_, typename Tuple2_, typename Pred_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            bool apply(Tuple1_ && tuple1, Tuple2_ && tuple2, Pred_ && predicate, Args_ &&... args)
            {
                return predicate(std::get<Idx_>(std::forward<Tuple1_>(tuple1)), std::get<Idx_>(std::forward<Tuple2_>(tuple2)), std::forward<Args_>(args)...) &&
                    tuple_all_of_2_impl<Length_, Idx_ + 1>::apply(std::forward<Tuple1_>(tuple1), std::forward<Tuple2_>(tuple2), std::forward<Pred_>(predicate), std::forward<Args_>(args)...);
            }
        };

        template <size_t Length_>
        struct tuple_all_of_2_impl<Length_, Length_>
        {
            template <typename Tuple1_, typename Tuple2_, typename Pred_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            bool apply(Tuple1_ &&, Tuple2_ &&, Pred_ &&, Args_ &&...)
            {
                return true;
            }
        };
    }

    /**
    *  Check if the each element of two tuples satisfy a binary predicate
    */
    template<typename Tuple1_, typename Tuple2_, typename Pred_, typename... Args_>
    YATO_CONSTEXPR_FUNC
    auto tuple_all_of(Tuple1_ && tuple1, Tuple2_ && tuple2, Pred_ && predicate, Args_ &&... args)
        -> std::enable_if_t<
            std::tuple_size<yato::remove_cvref_t<Tuple1_>>::value == std::tuple_size<yato::remove_cvref_t<Tuple2_>>::value, 
            bool
        >
    {
        return details::tuple_all_of_2_impl<std::tuple_size<yato::remove_cvref_t<Tuple1_>>::value, 0>::apply(std::forward<Tuple1_>(tuple1), std::forward<Tuple2_>(tuple2), std::forward<Pred_>(predicate), std::forward<Args_>(args)...);
    }


    namespace details
    {
        template <size_t Length_, size_t Idx_>
        struct tuple_any_of_impl
        {
            template <typename Tuple_, typename Pred_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            bool apply(Tuple_ && tuple, Pred_ && predicate, Args_ &&... args)
            {
                return predicate(std::get<Idx_>(std::forward<Tuple_>(tuple)), std::forward<Args_>(args)...) ||
                    tuple_any_of_impl<Length_, Idx_ + 1>::apply(std::forward<Tuple_>(tuple), std::forward<Pred_>(predicate), std::forward<Args_>(args)...);
            }
        };

        template <size_t Length_>
        struct tuple_any_of_impl<Length_, Length_>
        {
            template <typename Tuple_, typename Pred_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            bool apply(Tuple_ &&, Pred_ &&, Args_ &&...)
            {
                return false;
            }
        };
    }

    /**
     *  Check if at least one element of tuple satisfy a predicate
     */
    template <typename Tuple_, typename Pred_, typename... Args_>
    YATO_CONSTEXPR_FUNC
    bool tuple_any_of(Tuple_ && tuple, Pred_ && predicate, Args_ &&... args)
    {
        return details::tuple_any_of_impl<std::tuple_size<yato::remove_cvref_t<Tuple_>>::value, 0>::apply(std::forward<Tuple_>(tuple), std::forward<Pred_>(predicate), std::forward<Args_>(args)...);
    }


    namespace details
    {
        template <size_t Length_, size_t Idx_>
        struct tuple_any_of_2_impl
        {
            template <typename Tuple1_, typename Tuple2_, typename Pred_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            bool apply(Tuple1_ && tuple1, Tuple2_ && tuple2, Pred_ && predicate, Args_ &&... args)
            {
                return predicate(std::get<Idx_>(std::forward<Tuple1_>(tuple1)), std::get<Idx_>(std::forward<Tuple2_>(tuple2)), std::forward<Args_>(args)...) ||
                    tuple_any_of_2_impl<Length_, Idx_ + 1>::apply(std::forward<Tuple1_>(tuple1), std::forward<Tuple2_>(tuple2), std::forward<Pred_>(predicate), std::forward<Args_>(args)...);
            }
        };

        template <size_t Length_>
        struct tuple_any_of_2_impl<Length_, Length_>
        {
            template <typename Tuple1_, typename Tuple2_, typename Pred_, typename... Args_>
            YATO_CONSTEXPR_FUNC static
            bool apply(Tuple1_ &&, Tuple2_ &&, Pred_ &&, Args_ &&...)
            {
                return false;
            }
        };
    }

    /**
     *  Check if at least one element of two tuples satisfy a binary predicate
     */
    template <typename Tuple1_, typename Tuple2_, typename Pred_, typename... Args_>
    YATO_CONSTEXPR_FUNC
    auto tuple_any_of(Tuple1_ && tuple1, Tuple2_ && tuple2, Pred_ && predicate, Args_ &&... args)
        -> std::enable_if_t<
            std::tuple_size<yato::remove_cvref_t<Tuple1_>>::value == std::tuple_size<yato::remove_cvref_t<Tuple2_>>::value, 
            bool
        >
    {
        return details::tuple_any_of_2_impl<std::tuple_size<yato::remove_cvref_t<Tuple1_>>::value, 0>::apply(std::forward<Tuple1_>(tuple1), std::forward<Tuple2_>(tuple2), std::forward<Pred_>(predicate), std::forward<Args_>(args)...);
    }


    /**
     * Functor for accessing tuple element by reference
     */
    template <size_t Idx_>
    struct tuple_getter
    {
        template <typename Tuple_>
        decltype(auto) operator()(Tuple_ && t) const
        {
            return std::get<Idx_>(std::forward<Tuple_>(t));
        }
    };

    /**
     * Functor for accessing tuple element by const reference
     */
    template <size_t Idx_>
    struct tuple_cgetter
    {
        template <typename Tuple_>
        decltype(auto) operator()(const Tuple_ & t) const
        {
            return std::get<Idx_>(t);
        }
    };


    /**
     * Make tuple of Num_ types Ty_
     */
    template <typename Ty_, size_t Num_, typename... Elems_>
    struct tuple_dup
    {
        using type = typename tuple_dup<Ty_, Num_ - 1, Ty_, Elems_...>::type; 
    };

    template <typename Ty_, typename... Elems_>
    struct tuple_dup<Ty_, 0, Elems_...>
    {
        using type = std::tuple<Elems_...>;
    };

    /**
     * Make tuple of Num_ types Ty_
     */
    template <typename Ty_, size_t Num_>
    using tuple_dup_t = typename tuple_dup<Ty_, Num_>::type;


    /**
     * Split tuple to head and tail
     */
    template <typename Tuple_>
    struct tuple_split
    { };

    template <typename Head_, typename... Tail_>
    struct tuple_split<
        std::tuple<Head_, Tail_...>
    >
    {
        using head = Head_;
        using tail = std::tuple<Tail_...>;
    };
}

#endif
