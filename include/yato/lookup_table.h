/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_LOOKUP_TABLE_H_
#define _YATO_LOOKUP_TABLE_H_

#include <array>
#include <tuple>

#include "type_traits.h"

namespace yato
{

    struct lookup_strategy_nearest {};
    struct lookup_strategy_linear  {};

    namespace details
    {

        template <typename ValueType_>
        struct lut_value_sample
        {
            using value_type = ValueType_;

            value_type value;

            void set(const value_type & val)
            {
                value = val;
            }

            const value_type & get() const
            {
                return value;
            }
        };
        //--------------------------------------------------------------------


        template <typename ValueType_>
        struct lut_interpolator_nearest
        {
            static YATO_CONSTEXPR_VAR uint8_t samples_number = 1;

            YATO_CONSTEXPR_FUNC
                ValueType_ operator()(float alpha, const std::tuple<ValueType_> & y) const
            {
                YATO_MAYBE_UNUSED(alpha);
                return std::get<0>(y);
            }
        };

        template <typename ValueType_>
        struct lut_interpolator_linear
        {
            static YATO_CONSTEXPR_VAR uint8_t samples_number = 2;

            YATO_CONSTEXPR_FUNC
                ValueType_ operator()(float alpha, const std::tuple<ValueType_, ValueType_> & y) const
            {
                return std::get<0>(y) + alpha * (std::get<1>(y) - std::get<0>(y));
            }
        };
        //--------------------------------------------------------------------

        struct lut_fxpoint_alpha
        {
            using value_type = uint32_t;
            using wide_type  = uint64_t;
            static constexpr value_type bits_number = 10;
            static constexpr value_type one = 1 << bits_number;

            value_type value = 0;
        };

        //--------------------------------------------------------------------


        template <typename ValTy_, typename ArgTy_, size_t Size_, typename = void>
        class lut_base
        { };

        template <typename ValTy_, typename ArgTy_, size_t Size_>
        class lut_base<ValTy_, ArgTy_, Size_,
            std::enable_if_t<yato::is_integer<ArgTy_>::value>
        >
        {
        protected:
            using value_type = ValTy_;
            using argument_type = ArgTy_;
            using index_type = int32_t;
            using wide_index_type = typename yato::wider_type<index_type>::type;
            using sample_type = typename lut_value_sample<ValTy_>;
            using sample_creference = const typename lut_value_sample<ValTy_>&;
            using alpha_type = lut_fxpoint_alpha;
            static constexpr size_t   table_size = Size_;

            static_assert(table_size >= 2, "Table size can't be less than 2");

            std::array<sample_type, Size_> m_samples;
            index_type m_first = 0;
            index_type m_length = 0;
            std::tuple<index_type, index_type> m_fetch1_cache;

            template <typename FunTy_>
            void init_base_(FunTy_ && function, argument_type first, argument_type last)
            {
                assert(last > first);
                // Precompute index coefficients
                m_first = first;
                m_length = last - first;
                m_fetch1_cache = std::make_tuple(
                    narrow_cast<index_type>(m_length - 2 * table_size * m_first),
                    narrow_cast<index_type>(2 * m_length)
                );
                // Sample function
                for (size_t idx = 0; idx < table_size; ++idx) {
                    argument_type x = narrow_cast<argument_type>((first * (table_size - 1) + idx * (last - first)) / (table_size - 1));
                    m_samples[idx].set(function(x));
                }
            }

            index_type get_index_(meta::number<1>, argument_type x) const
            {
                constexpr auto multipler = 2 * (table_size - 1);
                return (multipler * x + std::get<0>(m_fetch1_cache)) / std::get<1>(m_fetch1_cache);
            }

            // Return floored index and alpha between index and index + 1
            std::tuple<index_type, index_type> get_index_(meta::number<2>, argument_type x) const
            {
                constexpr auto multipler = alpha_type::one * (table_size - 1);
                const wide_index_type tmp = (multipler * static_cast<wide_index_type>((x - m_first))) / m_length; // fixed point value = index * alpha_base
                const index_type idx   = std::min<index_type>(narrow_cast<index_type>(tmp >> alpha_type::bits_number), table_size - 2);
                const index_type alpha = narrow_cast<index_type>(tmp - (static_cast<wide_index_type>(idx) << alpha_type::bits_number));
                return std::make_tuple(idx, alpha);
            }

            std::tuple<sample_creference> fetch_(meta::number<1>, argument_type x, alpha_type*) const
            {
                auto idx = get_index_(meta::number<1>{}, x);
                return std::tie(m_samples[idx]);
            }

            std::tuple<sample_creference, sample_creference> fetch_(meta::number<2>, argument_type x, alpha_type* alpha) const
            {
                auto tmp = get_index_(meta::number<2>{}, x);
                index_type idx = std::get<0>(tmp);
                alpha->value = std::get<1>(tmp);
                assert(idx < table_size - 1);
                return std::tie(m_samples[idx], m_samples[idx + 1]);
            }


        };
    }

    //--------------------------------------------------------------------
    // Interpolators

    // ToDo (a.gruzdev): make samles and alpha types derived from LUT type

    template <typename ValTy_>
    struct lut_nearest_interpolator
    {
        static constexpr uint8_t samples_number = 1;

        ValTy_ operator()(const std::tuple<details::lut_value_sample<ValTy_>> & samples, const details::lut_fxpoint_alpha & /*alpha*/) const {
            return std::get<0>(samples).get();
        }
    };

    template <typename ValTy_>
    struct lut_linear_interpolator
    {  

        static constexpr uint8_t samples_number = 2;

        ValTy_ operator()(const std::tuple<details::lut_value_sample<ValTy_>, details::lut_value_sample<ValTy_>> & samples, const details::lut_fxpoint_alpha & alpha) const {
            return ((details::lut_fxpoint_alpha::one - alpha.value) * std::get<0>(samples).get() + alpha.value * std::get<1>(samples).get()) / details::lut_fxpoint_alpha::one;
        }
    };

    //--------------------------------------------------------------------

    template <typename ValTy_, typename ArgTy_, size_t Size_, typename Interpolator_ = lut_nearest_interpolator<ValTy_>>
    class lookup_table
        : private details::lut_base<ValTy_, ArgTy_, Size_>
    {
    private:
        using this_type = lookup_table<ValTy_, ArgTy_, Size_, Interpolator_>;
        using base_type = details::lut_base<ValTy_, ArgTy_, Size_>;
    public:
        using typename base_type::value_type;
        using typename base_type::argument_type;
        using typename base_type::index_type;
        using typename base_type::alpha_type;
        using base_type::table_size;

    private:
        Interpolator_ m_interpolator;


    public:
        lookup_table() 
        { }

        ~lookup_table()
        { }

        template <typename FunTy_>
        void init(FunTy_ && function, argument_type first, argument_type last)
        {
            base_type::init_base_(function, first, last);
        }

        value_type get(const argument_type & x) const
        {
            alpha_type alpha;
            auto values = base_type::fetch_(meta::number<Interpolator_::samples_number>{}, x, &alpha);
            return m_interpolator(values, alpha);
        }
    };


}

#endif
