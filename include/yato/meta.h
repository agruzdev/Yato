/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_META_H_
#define _YATO_META_H_

#include <tuple>
#include <limits>

#include "prerequisites.h"
#include "primitive_types.h"

namespace yato
{
    namespace meta
    {
        //-------------------------------------------------------
        // meta::id_type
        //

        /**
         *  Generate unique type from integer id
         */
        template<uint32_t Id_>
        struct id_type 
            : std::integral_constant<uint32_t, Id_>
        { };

        //-------------------------------------------------------
        // meta::number
        //

        /**
         *  Wrapper around unsigned integer value
         */
        template<uint32_t Num_>
        struct number
            : number<Num_ - 1>
        {
            static YATO_CONSTEXPR_VAR uint32_t value = Num_;
        };

        template<>
        struct number<0>
        {
            static YATO_CONSTEXPR_VAR uint32_t value = 0;
        };


        //-------------------------------------------------------
        // meta::pair

        /**
         * Pair of types
         */
        template <typename Ty1_, typename Ty2_>
        struct pair
        {
            using first_type  = Ty1_;
            using second_type = Ty2_;
        };

        //-------------------------------------------------------
        // meta::list
        //

        /**
         *  Empty list of types
         */
        struct null_list {};

        /**
         *  Non-position value of index in the list
         */
        YATO_INLINE_VARIABLE constexpr 
        size_t list_npos = std::numeric_limits<size_t>::max();

        /**
         *  List for aggregating a sequence of types
         */
        template<typename Head_, typename... Tail_>
        struct list
        {
            using head = Head_;
            using tail = list<Tail_...>;
        };

        /**
         *  List consisting of one element
         */
        template<typename Head_>
        struct list<Head_>
        {
            using head = Head_;
            using tail = null_list;
        };

        /**
         *  make list from variadic template
         */
        template<typename... Elems_>
        struct make_list
        {
            using type = list<Elems_...>;
        };

        /**
         *  make empty list
         */
        template<>
        struct make_list<>
        {
            using type = null_list;
        };

        template <typename... Elems_>
        using make_list_t = typename make_list<Elems_...>::type;

        /**
         *  Reverse list
         */
        template<typename List_, typename... Elems_>
        struct reverse_list
        {
            using type = typename reverse_list<typename List_::tail, typename List_::head, Elems_...>::type;
        };

        template<typename... Elems_>
        struct reverse_list<null_list, Elems_...>
        {
            using type = list<Elems_...>;
        };

        /**
         *  Add list element to the begin
         */
        template<typename Ty_, typename List_, typename... Elems_>
        struct list_push_front
        {
            using type = typename list_push_front<Ty_, typename List_::tail, Elems_..., typename List_::head>::type;
        };

        template<typename Ty_, typename... Elems_>
        struct list_push_front<Ty_, null_list, Elems_...>
        {
            using type = list<Ty_, Elems_...>;
        };

        /**
         *  Add list element to the end
         */
        template<typename List_, typename Ty_, typename... Elems_>
        struct list_push_back
        {
            using type = typename list_push_back<typename List_::tail, Ty_, Elems_..., typename List_::head>::type;
        };

        template<typename Ty_, typename... Elems_>
        struct list_push_back<null_list, Ty_, Elems_...>
        {
            using type = list<Elems_..., Ty_>;
        };

        /**
         *  Concatenate two lists
         */
        template<typename List1_, typename List2_, typename... Elems_>
        struct list_cat
        {
            using type = typename list_cat<typename List1_::tail, List2_, Elems_..., typename List1_::head>::type;
        };

        template<typename List2_, typename... Elems_>
        struct list_cat<null_list, List2_, Elems_...>
        {
            using type = typename list_cat<null_list, typename List2_::tail, Elems_..., typename List2_::head>::type;
        };

        template <typename... Elems_>
        struct list_cat<null_list, null_list, Elems_...>
        {
            using type = make_list_t<Elems_...>;
        };

        /**
         *  Access list element
         */
        template <typename List_, size_t Idx_>
        struct list_at
        {
            using type = typename list_at<typename List_::tail, Idx_ - 1>::type;
        };

        template <typename List_>
        struct list_at<List_, 0> 
        {
            using type = typename List_::head;
        };

        template <typename List_, size_t Idx_>
        using list_at_t = typename list_at<List_, Idx_>::type;

        /**
         *  Convert list to std::tuple
         */
        template <typename List_, typename... Types_>
        struct list_to_tuple
        {
            using type = typename list_to_tuple <typename List_::tail, Types_..., typename List_::head>::type;
        };

        template <typename... Types_>
        struct list_to_tuple <null_list, Types_...>
        {
            using type = std::tuple<Types_...>;
        };


        /**
         *  Find type in types list, value is index of found element
         */
        template <typename List_, typename Type_, size_t Idx_ = 0>
        struct list_find
            : public std::integral_constant<size_t, 
                std::is_same<typename List_::head, Type_>::value 
                ? Idx_
                : list_find<typename List_::tail, Type_, Idx_ + 1>::value>
        { };

        template <typename Type_, size_t Idx_>
        struct list_find<null_list, Type_, Idx_>
            : public std::integral_constant<size_t, list_npos>
        { };

        /**
         *  Get list length
         */
        template <typename List_>
        struct list_length
            : public std::integral_constant<size_t,
                1 + list_length<typename List_::tail>::value>
        { };

        template <>
        struct list_length<null_list>
            : public std::integral_constant<size_t, 0>
        { };


        /**
         * Split list at the specified position
         */
        template <typename List_, size_t Pos_, typename ... Elems_>
        struct list_split
        {
            using type = typename list_split<typename List_::tail, Pos_ - 1, Elems_..., typename List_::head>::type;
        };

        template <typename List_, typename... Elems_>
        struct list_split<List_, 0, Elems_...>
        {
            using type = meta::pair<meta::make_list_t<Elems_...>, List_>;
        };

        /**
         * Merge two sorted list into one sorted list
         */
        template <typename List1_, typename List2_, template<typename, typename> class Less_>
        struct list_merge
        {
            using type = typename std::conditional< Less_<typename List1_::head, typename List2_::head>::value,
                typename list_push_front< typename List1_::head, typename list_merge< typename List1_::tail, List2_, Less_>::type >::type,
                typename list_push_front< typename List2_::head, typename list_merge< List1_, typename List2_::tail, Less_>::type >::type
            >::type;
        };

        template <typename List1_, template<typename, typename> class Less_>
        struct list_merge<List1_, null_list, Less_>
        {
            using type = List1_;
        };

        template <typename List2_, template<typename, typename> class Less_>
        struct list_merge<null_list, List2_, Less_>
        {
            using type = List2_;
        };


        /**
         * Sort list
         */
        template <typename List_, template<typename, typename> class Less_, typename = void>
        struct list_sort
        {
            using parts = typename meta::list_split<List_, (meta::list_length<List_>::value / 2)>::type;
            using type  = typename meta::list_merge<
                typename list_sort<typename parts::first_type,  Less_>::type,
                typename list_sort<typename parts::second_type, Less_>::type,
                Less_
            >::type;
        };

        template <typename List_, template<typename, typename> class Less_>
        struct list_sort<List_, Less_, typename std::enable_if<
            (meta::list_length<List_>::value < 2)
        >::type>
        {
            using type = List_;
        };

        /**
         * Remove duplicates from the list
         */
        template <typename List_, typename... Elems_>
        struct list_unique
        {
            using type = typename std::conditional<(meta::list_find<meta::make_list_t<Elems_...>, typename List_::head>::value != meta::list_npos),
                typename list_unique<typename List_::tail, Elems_...>::type,
                typename list_unique<typename List_::tail, Elems_..., typename List_::head>::type
            >::type;
        };

        template <typename... Elems_>
        struct list_unique<meta::null_list, Elems_...>
        {
            using type = meta::make_list_t<Elems_...>;
        };


        //-------------------------------------------------------
        // type comparators

        template <typename Ty1_, typename Ty2_>
        struct type_less_sizeof
            : public std::integral_constant<bool, (sizeof(Ty1_) < sizeof(Ty2_))>
        { };

        template <typename Ty2_>
        struct type_less_sizeof<void, Ty2_>
            : public std::true_type
        { };

        template <typename Ty1_>
        struct type_less_sizeof<Ty1_, void>
            : public std::false_type
        { };
    }
}

#endif
