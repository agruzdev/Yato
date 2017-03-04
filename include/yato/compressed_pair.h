/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_COMPRESSED_PAIR_H_
#define _YATO_COMPRESSED_PAIR_H_

#include <tuple>
#include <utility>
#include "type_traits.h"

namespace yato
{
    namespace details
    {
        template<typename PairTy_, typename Ty_, typename = void>
        struct compressed_pair_getter
        { };

        template<typename PairTy_, typename Ty_>
        struct compressed_pair_getter <PairTy_, Ty_, typename std::enable_if<
            std::is_same<Ty_, typename PairTy_::first_type>::value && !std::is_same<Ty_, typename PairTy_::second_type>::value
        >::type>
        {
            using result_type = typename PairTy_::first_type;

            static result_type & apply(PairTy_ & pair)
            {
                return pair.first();
            }

            static const result_type & apply(const PairTy_ & pair)
            {
                return pair.first();
            }

            static volatile result_type & apply(volatile PairTy_ & pair)
            {
                return pair.first();
            }

            static const volatile result_type & apply(const volatile PairTy_ & pair)
            {
                return pair.first();
            }
        };

        template<typename PairTy_, typename Ty_>
        struct compressed_pair_getter <PairTy_, Ty_, typename std::enable_if<
            std::is_same<Ty_, typename PairTy_::second_type>::value && !std::is_same<Ty_, typename PairTy_::first_type>::value
        >::type>
        {
            using result_type = typename PairTy_::second_type;

            static result_type & apply(PairTy_ & pair)
            {
                return pair.second();
            }

            static const result_type & apply(const PairTy_ & pair)
            {
                return pair.second();
            }

            static volatile result_type & apply(volatile PairTy_ & pair)
            {
                return pair.second();
            }

            static const volatile result_type & apply(const volatile PairTy_ & pair)
            {
                return pair.second();
            }
        };
    }

    /**
     * Compressed pair
     * Compresses two classes with the help of empty base optimization if possible
     * General implementation without deriving from first type
     */
    template<typename FirstTy_, typename SecondTy_, typename = void>
    class compressed_pair
    {
    public:
        using first_type  = FirstTy_;
        using second_type = SecondTy_;
        using is_compressed = std::false_type;
        using super_type = void;
    private:
        using this_type = compressed_pair<FirstTy_, SecondTy_>;
        //--------------------------------------------------------------------

        first_type  m_first;
        second_type m_second;
        //--------------------------------------------------------------------

    public:
        YATO_CONSTEXPR_FUNC
        compressed_pair() = default;

        template<typename... Args2_>
        YATO_CONSTEXPR_FUNC explicit 
        compressed_pair(yato::zero_arg_then_variadic_t, Args2_ && ... args2)
            : m_first(), m_second(std::forward<Args2_>(args2)...)
        { }

        template<typename Arg1_, typename... Args2_>
        YATO_CONSTEXPR_FUNC
        compressed_pair(yato::one_arg_then_variadic_t, Arg1_ && arg1, Args2_ && ... args2)
            : m_first(std::forward<Arg1_>(arg1)), m_second(std::forward<Args2_>(args2)...)
        { }

        compressed_pair(const compressed_pair&) = default;

        compressed_pair& operator= (const compressed_pair&) = default;

#ifndef YATO_MSVC_2013
        compressed_pair(compressed_pair&&) = default;
        compressed_pair& operator= (compressed_pair&&) = default;
#endif

        ~compressed_pair() = default;

        /**
         * Return reference to first
         */
        first_type & first()
        {
            return m_first;
        }

        /**
         * Return reference to first
         */
        const first_type & first() const
        {
            return m_first;
        }

        /**
         * Return reference to first
         */
        volatile first_type & first() volatile
        {
            return m_first;
        }

        /**
         * Return reference to first
         */
        const volatile first_type & first() const volatile
        {
            return m_first;
        }

        /**
         * Return reference to second
         */
        second_type & second()
        {
            return m_second;
        }

        /**
         * Return reference to second
         */
        const second_type & second() const
        {
            return m_second;
        }

        /**
         * Return reference to second
         */
        volatile second_type & second() volatile
        {
            return m_second;
        }

        /**
         * Return reference to second
         */
        const volatile second_type & second() const volatile
        {
            return m_second;
        }

        /**
         * Get element by type, if stored types are different
         */
        template <typename Ty_>
        auto get_as()
            -> typename details::compressed_pair_getter<this_type, Ty_>::result_type &
        {
            return details::compressed_pair_getter<this_type, Ty_>::apply(*this);
        }

        /**
         * Get element by type, if stored types are different
         */
        template <typename Ty_>
        auto get_as() const
            -> const typename details::compressed_pair_getter<this_type, Ty_>::result_type &
        {
            return details::compressed_pair_getter<this_type, Ty_>::apply(*this);
        }

        /**
         * Get element by type, if stored types are different
         */
        template <typename Ty_>
        auto get_as() volatile
            -> volatile typename details::compressed_pair_getter<this_type, Ty_>::result_type &
        {
            return details::compressed_pair_getter<this_type, Ty_>::apply(*this);
        }

        /**
         * Get element by type, if stored types are different
         */
        template <typename Ty_>
        auto get_as() const volatile
            -> const volatile typename details::compressed_pair_getter<this_type, Ty_>::result_type &
        {
            return details::compressed_pair_getter<this_type, Ty_>::apply(*this);
        }

        /**
         * Convert to tuple
         */
        operator std::pair<first_type, second_type>() const
        {
            return std::make_pair(first(), second());
        }

        /**
         * Convert to tuple
         */
        operator std::tuple<first_type, second_type>() const
        {
            return std::make_tuple(first(), second());
        }
    };

    /**
     * Compressed implementation
     */
    template<typename FirstTy_, typename SecondTy_>
    class compressed_pair<FirstTy_, SecondTy_, typename std::enable_if<
        std::is_empty<FirstTy_>::value && !std::is_final<FirstTy_>::value
    >::type>
        : private FirstTy_
    {
    public:
        using first_type = FirstTy_;
        using second_type = SecondTy_;
        using is_compressed = std::true_type;
        using super_type = FirstTy_;
    private:
        using this_type = compressed_pair<FirstTy_, SecondTy_>;
        //--------------------------------------------------------------------

        second_type m_second;
        //--------------------------------------------------------------------

    public:
        YATO_CONSTEXPR_FUNC
        compressed_pair() = default;

        template<typename... Args2_>
        YATO_CONSTEXPR_FUNC explicit
        compressed_pair(yato::zero_arg_then_variadic_t, Args2_ && ... args2)
            : super_type(), m_second(std::forward<Args2_>(args2)...)
        { }

        template<typename Arg1_, typename... Args2_>
        YATO_CONSTEXPR_FUNC
        compressed_pair(yato::one_arg_then_variadic_t, Arg1_ && arg1, Args2_ && ... args2)
            : super_type(std::forward<Arg1_>(arg1)), m_second(std::forward<Args2_>(args2)...)
        { }

        compressed_pair(const compressed_pair&) = default;

        compressed_pair& operator= (const compressed_pair&) = default;

#ifndef YATO_MSVC_2013
        compressed_pair(compressed_pair&&) = default;
        compressed_pair& operator= (compressed_pair&&) = default;
#endif

        ~compressed_pair() = default;

        /**
         * Return reference to first
         */
        first_type & first()
        {
            return *this;
        }

        /**
         * Return reference to first
         */
        const first_type & first() const
        {
            return *this;
        }

        /**
         * Return reference to first
         */
        volatile first_type & first() volatile
        {
            return *this;
        }

        /**
         * Return reference to first
         */
        const volatile first_type & first() const volatile
        {
            return *this;
        }

        /**
         * Return reference to second
         */
        second_type & second()
        {
            return m_second;
        }

        /**
         * Return reference to second
         */
        const second_type & second() const
        {
            return m_second;
        }

        /**
         * Return reference to second
         */
        volatile second_type & second() volatile
        {
            return m_second;
        }

        /**
         * Return reference to second
         */
        const volatile second_type & second() const volatile
        {
            return m_second;
        }

        /**
         * Get element by type, if stored types are different
         */
        template <typename Ty_>
        auto get_as()
            -> typename details::compressed_pair_getter<this_type, Ty_>::result_type &
        {
            return details::compressed_pair_getter<this_type, Ty_>::apply(*this);
        }

        /**
         * Get element by type, if stored types are different
         */
        template <typename Ty_>
        auto get_as() const
            -> const typename details::compressed_pair_getter<this_type, Ty_>::result_type &
        {
            return details::compressed_pair_getter<this_type, Ty_>::apply(*this);
        }

        /**
         * Get element by type, if stored types are different
         */
        template <typename Ty_>
        auto get_as() volatile
            -> volatile typename details::compressed_pair_getter<this_type, Ty_>::result_type &
        {
            return details::compressed_pair_getter<this_type, Ty_>::apply(*this);
        }

        /**
         * Get element by type, if stored types are different
         */
        template <typename Ty_>
        auto get_as() const volatile
            -> const volatile typename details::compressed_pair_getter<this_type, Ty_>::result_type &
        {
            return details::compressed_pair_getter<this_type, Ty_>::apply(*this);
        }

        /**
         * Convert to tuple
         */
        operator std::pair<first_type, second_type>() const
        {
            return std::make_pair(first(), second());
        }

        /**
         * Convert to tuple
         */
        operator std::tuple<first_type, second_type>() const
        {
            return std::make_tuple(first(), second());
        }
    };

    // std::get overloading
    template <size_t Idx_, typename Ty1_, typename Ty2_>
    auto get(const yato::compressed_pair<Ty1_, Ty2_> & pair)
        -> typename std::enable_if<(Idx_ == 0), const Ty1_ &>::type
    {
        return pair.first();
    }

    // std::get overloading
    template <size_t Idx_, typename Ty1_, typename Ty2_>
    auto get(const yato::compressed_pair<Ty1_, Ty2_> & pair)
        -> typename std::enable_if<(Idx_ == 1), const Ty2_ &>::type
    {
        return pair.second();
    }

    // std::get overloading
    template <size_t Idx_, typename Ty1_, typename Ty2_>
    auto get(yato::compressed_pair<Ty1_, Ty2_> & pair)
        -> typename std::enable_if<(Idx_ == 0), Ty1_ &>::type
    {
        return pair.first();
    }

    // std::get overloading
    template <size_t Idx_, typename Ty1_, typename Ty2_>
    auto get(yato::compressed_pair<Ty1_, Ty2_> & pair)
        -> typename std::enable_if<(Idx_ == 1), Ty2_ &>::type
    {
        return pair.second();
    }

    // std::get overloading
    template <size_t Idx_, typename Ty1_, typename Ty2_>
    auto get(yato::compressed_pair<Ty1_, Ty2_> && pair)
        -> typename std::enable_if<(Idx_ == 0), Ty1_ &&>::type
    {
        return pair.first();
    }

    // std::get overloading
    template <size_t Idx_, typename Ty1_, typename Ty2_>
    auto get(yato::compressed_pair<Ty1_, Ty2_> && pair)
        -> typename std::enable_if<(Idx_ == 1), Ty2_ &&>::type
    {
        return pair.second();
    }

    // std::get overloading
    template <typename Ty_, typename Ty1_, typename Ty2_>
    auto get(const yato::compressed_pair<Ty1_, Ty2_> & pair)
        -> decltype(pair.template get_as<Ty_>())
    {
        return pair.template get_as<Ty_>();
    }

    // std::get overloading
    template <typename Ty_, typename Ty1_, typename Ty2_>
    auto get(yato::compressed_pair<Ty1_, Ty2_> & pair)
        -> decltype(pair.template get_as<Ty_>())
    {
        return pair.template get_as<Ty_>();
    }

    // std::get overloading
    template <typename Ty_, typename Ty1_, typename Ty2_>
    auto get(yato::compressed_pair<Ty1_, Ty2_> && pair)
        -> decltype(pair.template get_as<Ty_>())
    {
        return pair.template get_as<Ty_>();
    }

}

#endif //_YATO_COMPRESSED_PAIR_H_
