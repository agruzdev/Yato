/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_META_H_
#define _YATO_META_H_

#include <tuple>
#include "types.h"

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
        struct number: public number<_Num - 1>
        {
            static YATO_CONSTEXPR_VAR uint32_t value = _Num;
        };

        template<>
        struct number<0>
        {
            static YATO_CONSTEXPR_VAR uint32_t value = 0;
        };



        //-------------------------------------------------------
        // meta::list
        //

        /**
         *  Empty list of types
         */
        struct null_list {};

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

    }
}

#endif