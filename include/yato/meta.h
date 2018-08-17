/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
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
        template<uint32_t _Id>
        struct id_type 
            : std::integral_constant<uint32_t, _Id>
        { };

        //-------------------------------------------------------
        // meta::number
        //

        /**
         *  Wrapper around unsigned integer value
         */
        template<uint32_t _Num>
        struct number
            : number<_Num - 1>
        {
            static YATO_CONSTEXPR_VAR uint32_t value = _Num;
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
        template<typename _Head, typename... _Tail>
        struct list
        {
            using head = _Head;
            using tail = list<_Tail...>;
        };

        /**
         *  List consisting of one element
         */
        template<typename _Head>
        struct list<_Head>
        {
            using head = _Head;
            using tail = null_list;
        };

        /**
         *  make list from variadic template
         */
        template<typename... _Elems>
        struct make_list
        {
            using type = list<_Elems...>;
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
        template<typename _List, typename... _Elems>
        struct reverse_list
        {
            using type = typename reverse_list<typename _List::tail, typename _List::head, _Elems...>::type;
        };

        template<typename... _Elems>
        struct reverse_list<null_list, _Elems...>
        {
            using type = list<_Elems...>;
        };

        /**
         *  Add list element to the begin
         */
        template<typename _T, typename _List, typename... _Elems>
        struct list_push_front
        {
            using type = typename list_push_front<_T, typename _List::tail, _Elems..., typename _List::head>::type;
        };

        template<typename _T, typename... _Elems>
        struct list_push_front<_T, null_list, _Elems...>
        {
            using type = list<_T, _Elems...>;
        };

        /**
         *  Add list element to the end
         */
        template<typename _List, typename _T, typename... _Elems>
        struct list_push_back
        {
            using type = typename list_push_back<typename _List::tail, _T, _Elems..., typename _List::head>::type;
        };

        template<typename _T, typename... _Elems>
        struct list_push_back<null_list, _T, _Elems...>
        {
            using type = list<_Elems..., _T>;
        };

        /**
         *  Concatenate two lists
         */
        template<typename _List1, typename _List2, typename... _Elems>
        struct list_cat
        {
            using type = typename list_cat<typename _List1::tail, _List2, _Elems..., typename _List1::head>::type;
        };

        template<typename _List2, typename... _Elems>
        struct list_cat<null_list, _List2, _Elems...>
        {
            using type = typename list_cat<null_list, typename _List2::tail, _Elems..., typename _List2::head>::type;
        };

        template<typename... _Elems>
        struct list_cat<null_list, null_list, _Elems...>
        {
            using type = list<_Elems...>;
        };

        /**
         *  Access list element
         */
        template <typename _List, size_t _Idx>
        struct list_at
        {
            using type = typename list_at<typename _List::tail, _Idx - 1>::type;
        };

        template <typename _List>
        struct list_at<_List, 0> 
        {
            using type = typename _List::head;
        };

        template <typename List_, size_t Idx_>
        using list_at_t = typename list_at<List_, Idx_>::type;

        /**
         *  Convert list to std::tuple
         */
        template <typename _List, typename... _Types>
        struct list_to_tuple
        {
            using type = typename list_to_tuple <typename _List::tail, _Types..., typename _List::head>::type;
        };

        template <typename... _Types>
        struct list_to_tuple <null_list, _Types...>
        {
            using type = std::tuple<_Types...>;
        };


        /**
         *  Find type in types list, value is index of found element
         */
        template <typename List, typename Type, size_t Idx = 0>
        struct list_find
            : public std::integral_constant<size_t, 
                std::is_same<typename List::head, Type>::value 
                ? Idx
                : list_find<typename List::tail, Type, Idx + 1>::value>
        { };

        template <typename Type, size_t Idx>
        struct list_find<null_list, Type, Idx>
            : public std::integral_constant<size_t, list_npos>
        { };

        /**
         *  Get list length
         */
        template <typename List>
        struct list_length
            : public std::integral_constant<size_t,
                1 + list_length<typename List::tail>::value>
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
