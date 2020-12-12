/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_SAMPLER_H_
#define _YATO_SAMPLER_H_

#include "assertion.h"
#include "prerequisites.h"
#include "type_traits.h"

namespace yato
{
    namespace details
    {
        template <typename Ty_, typename Vy_, typename = void>
        struct get_return_type_with_arg
        {
            using type = std::add_lvalue_reference_t<std::add_const_t<Vy_>>;
        };

        template <typename Ty_, typename Vy_>
        struct get_return_type_with_arg<Ty_, Vy_,
            yato::void_t<typename Ty_::template return_type<Vy_>>
        >
        {
            using type = typename Ty_::template return_type<Vy_>;
        };

        template <typename Ty_, typename IdxTy_, size_t Dim_, typename = void>
        struct has_transform_index_method
            : std::false_type
        { };

        template <typename Ty_, typename IdxTy_, size_t Dim_>
        struct has_transform_index_method<Ty_, IdxTy_, Dim_,
            std::enable_if_t<std::is_same<bool, decltype(std::declval<Ty_>().template transform_index<Dim_>(std::declval<IdxTy_>(), std::declval<std::size_t>(), std::declval<std::size_t&>()))>::value>
        >
            : std::true_type
        { };

        template <typename Ty_, typename Vy_, typename = void>
        struct has_transform_value_method
            : std::false_type
        { };

        template <typename Ty_, typename Vy_>
        struct has_transform_value_method<Ty_, Vy_,
            yato::void_t<decltype(std::declval<Ty_>().transform_value(std::declval<const Vy_&>()))>
        >
            : std::true_type
        { };

        template <typename Ty_, typename Vy_, typename = void>
        struct has_boundary_value_method
            : std::false_type
        { };

        template <typename Ty_, typename Vy_>
        struct has_boundary_value_method<Ty_, Vy_,
            yato::void_t<decltype(std::declval<Ty_>().template boundary_value<Vy_>())>
        >
            : std::true_type
        { };

    } // namespace details

    /**
     * Provides the standardized access to sampler properties and defines the common user interface
     * Member types:
     * index_type - Sampler::index_type if present, othewise size_t
     * return_type<ValueType> - templated type alias to Sampler::return_type<ValueType> if present, othewise const ValueType&
     * Member methods:
     * return_value<Container::value_type> load(Sampler&, const Container&, indexes...) - performs read operation from the container via transform_index() and transform_value() methods.
     * Sampler requirements:
     * bool transform_index(index_type, size_t, size_t&) - transforms input index to effective index. Calls Sampler::transform_index if present, otherwise performs narrow_cast.
     * return_type<ValueType> transform_value(const ValueType&) - transforms read value to return value. Calls Sampler::transform_value if present, otherwise returns the value as is.
     * return_type<ValueType> bounary_value() - returns a value for out of bounds reads. Calls Sampler::boundary_value if present, othrwise throws out_of_range_error.
     */
    template <typename Sampler_>
    class sampler_traits
    {
    public:
        using index_type = typename details::get_index_type<Sampler_, std::size_t>::type;

        template <typename ValueType_>
        using return_type = typename details::get_return_type_with_arg<Sampler_, ValueType_>::type;

        template <typename Container_, typename... Indexes_>
        static YATO_CONSTEXPR_FUNC
        auto load(const Sampler_& s, const Container_& c, Indexes_&&... indexes)
            -> return_type<typename Container_::value_type>
        {
            return load_impl_<typename Container_::value_type, 0>(s, c, std::forward<Indexes_>(indexes)...);
        }

        template <typename Container_, typename... Indexes_>
        static YATO_CONSTEXPR_FUNC
        auto load(Sampler_& s, const Container_& c, Indexes_&&... indexes)
            -> return_type<typename Container_::value_type>
        {
            return load_impl_<typename Container_::value_type, 0>(s, c, std::forward<Indexes_>(indexes)...);
        }

    private:
        template <typename ValueType_, size_t Dim_, typename SamplerRef_, typename Container_, typename... Indexes_>
        static YATO_CONSTEXPR_FUNC
        return_type<ValueType_> load_impl_(SamplerRef_&& s, const Container_& c, index_type i0, index_type i1, Indexes_&&... indexes_tail)
        {
            using has_transform_index_ = details::has_transform_index_method<Sampler_, index_type, Dim_>;
            using has_boundary_value_  = details::has_boundary_value_method<Sampler_, ValueType_>;
            size_t effective_idx;
            if (invoke_transform_index_<Dim_>(has_transform_index_{}, std::forward<SamplerRef_>(s), i0, c.size(0), effective_idx)) {
                return load_impl_<ValueType_, Dim_ + 1>(std::forward<SamplerRef_>(s), c[effective_idx], i1, std::forward<Indexes_>(indexes_tail)...);
            }
            else {
                return invoke_boundary_value_<ValueType_>(has_boundary_value_{}, std::forward<SamplerRef_>(s));
            }
        }

        template <typename ValueType_, size_t Dim_, typename SamplerRef_, typename Container_>
        static YATO_CONSTEXPR_FUNC
        return_type<ValueType_> load_impl_(SamplerRef_&& s, const Container_& c, index_type i)
        {
            using has_transform_index_ = details::has_transform_index_method<Sampler_, index_type, Dim_>;
            using has_transform_value_ = details::has_transform_value_method<Sampler_, ValueType_>;
            using has_boundary_value_  = details::has_boundary_value_method<Sampler_, ValueType_>;
            size_t effective_idx;
            if (invoke_transform_index_<Dim_>(has_transform_index_{}, std::forward<SamplerRef_>(s), i, c.size(), effective_idx)) {
                return invoke_transform_value_<ValueType_>(has_transform_value_{}, std::forward<SamplerRef_>(s), c[effective_idx]);
            }
            else {
                return invoke_boundary_value_<ValueType_>(has_boundary_value_{}, std::forward<SamplerRef_>(s));
            }
        }

        template <size_t Dim_, typename SamplerRef_>
        static YATO_CONSTEXPR_FUNC
        bool invoke_transform_index_(std::true_type, SamplerRef_&& s, index_type in_idx, std::size_t size, std::size_t& out_idx)
        {
            return std::forward<SamplerRef_>(s).template transform_index<Dim_>(in_idx, size, out_idx);
        }

        template <size_t Dim_, typename SamplerRef_>
        static YATO_CONSTEXPR_FUNC
        bool invoke_transform_index_(std::false_type, SamplerRef_&& /*s*/, index_type in_idx, std::size_t size, std::size_t& out_idx)
        {
            if (in_idx < size) {
                out_idx = in_idx;
                return true;
            }
            return false;
        }

        template <typename ValueType_, typename SamplerRef_>
        static YATO_CONSTEXPR_FUNC
        return_type<ValueType_> invoke_transform_value_(std::true_type, SamplerRef_&& s, const ValueType_& in_val)
        {
            return std::forward<SamplerRef_>(s).transform_value(in_val);
        }

        template <typename ValueType_, typename SamplerRef_>
        static YATO_CONSTEXPR_FUNC
        return_type<ValueType_> invoke_transform_value_(std::false_type, SamplerRef_&& /*s*/, const ValueType_& in_val)
        {
            return in_val;
        }

        template <typename ValueType_, typename SamplerRef_>
        static YATO_CONSTEXPR_FUNC
        return_type<ValueType_> invoke_boundary_value_(std::true_type, SamplerRef_&& s)
        {
            return std::forward<SamplerRef_>(s).template boundary_value<ValueType_>();
        }

        template <typename ValueType_, typename SamplerRef_>
        YATO_NORETURN static
        return_type<ValueType_> invoke_boundary_value_(std::false_type, SamplerRef_&& /*s*/)
        {
            throw yato::out_of_range_error("index is out of bounds");
        }
    };

    template <typename Sampler_, typename Container_, typename... Indexes_>
    inline
    decltype(auto) load(const Container_& c, Indexes_&&... indexes)
    {
        return sampler_traits<Sampler_>::load(Sampler_{}, c, std::forward<Indexes_>(indexes)...);
    }

    template <typename Sampler_, typename Container_, typename... Indexes_>
    inline
    decltype(auto) loads(Sampler_&& s, const Container_& c, Indexes_&&... indexes)
    {
        return sampler_traits<yato::remove_cvref_t<Sampler_>>::load(std::forward<Sampler_>(s), c, std::forward<Indexes_>(indexes)...);
    }



    /**
     * Default sampler behaves similar to standard at() throwing out_of_bounds exception for invalid indexes
     */
    struct sampler_default
    { };

    /**
     * Performs no boundary check allowing any access
     */
    struct sampler_no_check
    {
        using index_type = std::size_t;

        template <size_t Dim_>
        YATO_CONSTEXPR_FUNC
        bool transform_index(index_type in_idx, std::size_t /*size*/, std::size_t& out_idx) const
        {
            out_idx = in_idx;
            return true;
        }
    };

    /**
     * Returns zero for out of bounds access
     */
    struct sampler_zero
    {
        using index_type = yato::int32_t;

        template <typename ValueType_>
        using return_type = ValueType_;

        template <size_t Dim_>
        YATO_CONSTEXPR_FUNC
        bool transform_index(index_type in_idx, std::size_t size, std::size_t& out_idx) const
        {
            if (in_idx >= 0 && static_cast<std::size_t>(in_idx) < size) {
                out_idx = static_cast<std::size_t>(in_idx);
                return true;
            }
            return false;
        }

        template <typename ValueType_>
        return_type<ValueType_> boundary_value() const
        {
            return static_cast<ValueType_>(0);
        }
    };


    /**
     * Clamps access index to [0, size) for the each dimension
     */
    struct sampler_clamp
    {
        using index_type = yato::int32_t;

        template <size_t Dim_>
        YATO_CONSTEXPR_FUNC
        bool transform_index(index_type in_idx, std::size_t size, std::size_t& out_idx) const
        {
            YATO_REQUIRES(size != 0);
            out_idx = std::min<std::size_t>(std::max<index_type>(0, in_idx), size - 1);
            return true;
        }
    };


} // namespace yato


#endif //_YATO_SAMPLER_H_
