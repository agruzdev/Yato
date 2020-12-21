/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_CONTAINER_ND_H_
#define _YATO_CONTAINER_ND_H_

#include <array>
#include <iterator>
#include <type_traits>
#include <vector>
#include "assertion.h"
#include "types.h"
#include "range.h"

#include "container_base.h"
#include "numeric_iterator.h"

namespace yato
{
    struct sampler_default;

    namespace details
    {
        YATO_DEFINE_TYPE_GETTER(get_size_type, size_type)
        YATO_DEFINE_TYPE_GETTER(get_allocator_type, allocator_type)
        YATO_DEFINE_TYPE_GETTER(get_dimensions_type, dimensions_type)
        YATO_DEFINE_TYPE_GETTER(get_strides_type, strides_type)
        YATO_DEFINE_TYPE_GETTER(get_plain_iterator, plain_iterator)
        YATO_DEFINE_TYPE_GETTER(get_const_plain_iterator, const_plain_iterator)
        YATO_DEFINE_TYPE_GETTER(get_reference, reference)
        YATO_DEFINE_TYPE_GETTER(get_const_reference, const_reference)
        YATO_DEFINE_TYPE_GETTER(get_value_reference, value_reference)
        YATO_DEFINE_TYPE_GETTER(get_const_value_reference, const_value_reference)
        YATO_DEFINE_VALUE_GETTER(get_has_method_size0, has_method_size0, bool)
        YATO_DEFINE_METHOD_CHECK_RET_0AGR(has_method_size0, size)
        YATO_DEFINE_VALUE_GETTER(get_has_method_size1, has_method_size1, bool)
        YATO_DEFINE_METHOD_CHECK_RET_1AGR(has_method_size1, size)
        YATO_DEFINE_VALUE_GETTER(get_has_method_stride, has_method_stride, bool)
        YATO_DEFINE_METHOD_CHECK_RET_1AGR(has_method_stride, stride)
        YATO_DEFINE_VALUE_GETTER(get_has_method_continuous, has_method_continuous, bool)
        YATO_DEFINE_METHOD_CHECK_RET_0AGR(has_method_continuous, continuous)
        YATO_DEFINE_VALUE_GETTER(get_has_operator_subscript, has_operator_subscript, bool)
        YATO_DEFINE_VALUE_GETTER(get_has_operator_csubscript, has_operator_csubscript, bool)
        YATO_DEFINE_METHOD_CHECK_RET_1AGR(has_operator_subscript, operator[])
        YATO_DEFINE_VALUE_GETTER(get_has_method_dimensions, has_method_dimensions, bool)
        YATO_DEFINE_METHOD_CHECK_RET_0AGR(has_method_dimensions, dimensions)
        YATO_DEFINE_VALUE_GETTER(get_has_method_dimensions_range, has_method_dimensions_range, bool)
        YATO_DEFINE_METHOD_CHECK_0AGR(has_method_dimensions_range, dimensions_range)
        YATO_DEFINE_VALUE_GETTER(get_has_method_total_size, has_method_total_size, bool)
        YATO_DEFINE_METHOD_CHECK_RET_0AGR(has_method_total_size, total_size)
        YATO_DEFINE_VALUE_GETTER(get_has_method_strides, has_method_strides, bool)
        YATO_DEFINE_METHOD_CHECK_RET_0AGR(has_method_strides, strides)
        YATO_DEFINE_VALUE_GETTER(get_has_method_data, has_method_data, bool)
        YATO_DEFINE_METHOD_CHECK_RET_0AGR(has_method_data, data)
        YATO_DEFINE_VALUE_GETTER(get_has_method_cdata, has_method_cdata, bool)
        YATO_DEFINE_METHOD_CHECK_RET_0AGR(has_method_cdata, cdata)

        template <typename C_, typename DimsType_, size_t DimsNum_, size_t... Indexes_>
        struct make_dims_from_size_
        {
            static YATO_CONSTEXPR_FUNC
            DimsType_ apply(const C_& c)
            {
                return make_dims_from_size_<C_, DimsType_, DimsNum_ - 1, DimsNum_ - 1, Indexes_...>::apply(c);
            }
        };

        template <typename C_, typename DimsType_, size_t... Indexes_>
        struct make_dims_from_size_<C_, DimsType_, 0, Indexes_...>
        {
            static YATO_CONSTEXPR_FUNC
            DimsType_ apply(yato::disable_if_not_t<sizeof...(Indexes_) != 1, const C_&> c)
            {
                return DimsType_(c.size(Indexes_)...);
            }

            static YATO_CONSTEXPR_FUNC
            DimsType_ apply(yato::disable_if_not_t<sizeof...(Indexes_) == 1, const C_&> c)
            {
                return DimsType_(c.size());
            }
        };

        template <typename C_, typename ReturnType_, size_t DimsNum_, size_t... Indexes_>
        struct make_array_from_strides_
        {
            static YATO_CONSTEXPR_FUNC
            ReturnType_ apply(const C_& c)
            {
                return make_array_from_strides_<C_, ReturnType_, DimsNum_ - 1, DimsNum_ - 1, Indexes_...>::apply(c);
            }
        };

        template <typename C_, typename ReturnType_, size_t... Indexes_>
        struct make_array_from_strides_<C_, ReturnType_, 0, Indexes_...>
        {
            static YATO_CONSTEXPR_FUNC
            ReturnType_ apply(const C_& c)
            {
                return ReturnType_(c.stride(Indexes_)...);
            }
        };

    } // namespace details;



    /**
     * Provides the standardized access to container properties
     */
    template <typename Container_>
    struct container_traits
    {
        //static_assert(std::is_same<yato::remove_cvref_t<Container_>, Container_>::value, "Container_ must be a type without modifiers");

        /**
         * Alias to Container::value_type. Required member.
         */
        using value_type = typename Container_::value_type;

        /**
         * Type returned by the method Container::size(...).
         * Alias to Container::size_type if present, otherwise size_t.
         */
        using size_type = typename details::get_size_type<Container_, std::size_t>::type;

        /**
         * Argument type for operator[] and at(...).
         * Alias to Container::index_type if present, otherwise size_t.
         */
        using index_type = typename details::get_index_type<Container_, std::size_t>::type;

        /**
         * Alias to Container::allocator_type if present, otherwise void.
         */
        using allocator_type = typename details::get_allocator_type<Container_, void>::type;

        /**
         * Number of container dimensions. Required member.
         */
        static YATO_CONSTEXPR_VAR std::size_t dimensions_number = Container_::dimensions_number;
        static_assert(dimensions_number > 0, "dimensions_number must be positive");

        /**
         * Type returned by the method Container::dimensions().
         * Alias to Container::dimensions_type if present, otherwise yato::dimensionality.
         */
        using dimensions_type = typename details::get_dimensions_type<Container_, yato::dimensionality<dimensions_number, size_type>>::type;

        /**
         * Type returned by the method Container::strides().
         * Alias to Container::strides_type if present, otherwise yato::strides_array.
         */
        using strides_type = typename details::get_strides_type<Container_, yato::strides_array<(dimensions_number > 0 ? dimensions_number - 1 : 0), size_type>>::type;


        /**
         * Alias to Container::iterator. Required member.
         */
        using iterator = typename Container_::iterator;

        /**
         * Alias to Container::const_iterator. Required member.
         */
        using const_iterator = typename Container_::const_iterator;

        /**
         * Both plain iterator and last dimension iterator.
         * Alias to Container::plain_iterator, otherwise aias to Container::iterator.
         */
        using plain_iterator = typename details::get_plain_iterator<Container_, iterator>::type;

        /**
         * Both plain const_iterator and last dimension const_iterator.
         * Alias to Container::const_plain_iterator, otherwise aias to Container::plain_iterator.
         */
        using const_plain_iterator = typename details::get_const_plain_iterator<Container_, const_iterator>::type;


        /**
         * Alias to Container::reference if present, otherise std::iterator_traits<iterator>::reference.
         */
        using reference = typename details::get_reference<Container_, typename std::iterator_traits<iterator>::reference>::type;

        /**
         * Alias to Container::const_reference if present, otherise std::iterator_traits<const_iterator>::reference.
         */
        using const_reference = typename details::get_const_reference<Container_, typename std::iterator_traits<const_iterator>::reference>::type;

        /**
         * Alias to Container::value_reference if present, otherise alias to reference.
         */
        using value_reference = typename details::get_value_reference<Container_, reference>::type;

        /**
         * Alias to Container::const_value_reference if present, otherise alias to const_reference.
         */
        using const_value_reference = typename details::get_value_reference<Container_, const_reference>::type;



        /**
         * Checks if the method Container::size() is present.
         * Alias to Container::has_method_size0 is present, otherwise tries to deduce.
         */
        static YATO_CONSTEXPR_VAR bool has_method_size0 = details::get_has_method_size0<Container_,
                details::has_method_size0<const Container_, size_type>::value>::value;

        /**
         * Checks if the method Container::size(dim) is present.
         * Alias to Container::has_method_size1 is present, otherwise tries to deduce.
         */
        static YATO_CONSTEXPR_VAR bool has_method_size1 = details::get_has_method_size1<Container_,
                details::has_method_size1<const Container_, size_type, std::size_t>::value>::value;

        /**
         * Checks if the method Container::stride(dim) is present.
         * Alias to Container::has_method_stride is present, otherwise tries to deduce.
         */
        static YATO_CONSTEXPR_VAR bool has_method_stride = details::get_has_method_stride<Container_,
                details::has_method_stride<const Container_, std::size_t, std::size_t>::value>::value;

        /**
         * Checks if the method Container::continuous() is present.
         * Alias to Container::has_method_continuous is present, otherwise tries to deduce.
         */
        static YATO_CONSTEXPR_VAR bool has_method_continuous = details::get_has_method_continuous<Container_,
                details::has_method_continuous<const Container_, bool>::value>::value;

        /**
         * Checks if the method Container::operator[i] is present and returned type is consistent with Iterator.
         * Alias to Container::has_operator_subscript is present, otherwise tries to deduce.
         */
        static YATO_CONSTEXPR_VAR bool has_operator_subscript = details::get_has_operator_subscript<Container_,
                details::has_operator_subscript<Container_, reference, yato::add_lvalue_reference_to_const_t<index_type>>::value>::value;

        /**
         * Checks if the method Container::operator[i] (const) is present and returned type is consistent with Iterator.
         * Alias to Container::has_operator_csubscript is present, otherwise tries to deduce.
         */
        static YATO_CONSTEXPR_VAR bool has_operator_csubscript = details::get_has_operator_csubscript<Container_,
                details::has_operator_subscript<const Container_, const_reference, yato::add_lvalue_reference_to_const_t<index_type>>::value>::value;

        /**
         * Checks if the method Container::dimensions() is present.
         * Alias to Container::has_method_dimensions is present, otherwise tries to deduce.
         */
        static YATO_CONSTEXPR_VAR bool has_method_dimensions = details::get_has_method_dimensions<Container_,
                details::has_method_dimensions<const Container_, dimensions_type>::value>::value;

        /**
         * Checks if the method Container::dimensions_range() is present.
         * Alias to Container::has_method_dimensions_range is present, otherwise tries to deduce.
         */
        static YATO_CONSTEXPR_VAR bool has_method_dimensions_range = details::get_has_method_dimensions_range<Container_,
                details::has_method_dimensions_range<const Container_>::value>::value;

        /**
         * Checks if the method Container::strides() is present.
         * Alias to Container::has_method_strides is present, otherwise tries to deduce.
         */
        static YATO_CONSTEXPR_VAR bool has_method_strides = details::get_has_method_strides<Container_,
                details::has_method_strides<const Container_, strides_type>::value>::value;

        /**
         * Checks if the method Container::total_size() is present.
         * Alias to Container::has_method_total_size is present, otherwise tries to deduce.
         */
        static YATO_CONSTEXPR_VAR bool has_method_total_size = details::get_has_method_total_size<Container_,
                details::has_method_total_size<const Container_, size_type>::value>::value;

        /**
         * Checks if the method Container::data() is present.
         * Alias to Container::has_method_data is present, otherwise tries to deduce.
         */
        static YATO_CONSTEXPR_VAR bool has_method_data = details::get_has_method_data<Container_,
                details::has_method_data<Container_, std::add_pointer_t<value_type>>::value>::value;

        /**
         * Checks if the method Container::cdata() is present.
         * Alias to Container::has_operator_csubscript is present, otherwise tries to deduce.
         */
        static YATO_CONSTEXPR_VAR bool has_method_cdata = details::get_has_method_cdata<Container_,
                details::has_method_cdata<const Container_, std::add_pointer_t<std::add_const_t<value_type>>>::value>::value;

    };


    template <typename Ty_, typename = void>
    struct is_container
        : std::false_type
    { };

    template <typename Ty_>
    struct is_container<Ty_,
        typename yato::test_type<
            decltype(Ty_::dimensions_number),
            typename Ty_::value_type,
            typename Ty_::iterator,
            typename Ty_::const_iterator
        >::type
    >
        : std::true_type
    { };



    template <typename Ty_, std::size_t N_>
    struct container_traits<Ty_[N_]>
    {
        using value_type = Ty_;
        using size_type  = std::size_t;
        using index_type = std::ptrdiff_t;

        using allocator_type = void;

        static YATO_CONSTEXPR_VAR std::size_t dimensions_number = 1;

        using dimensions_type = yato::dimensionality<dimensions_number, size_type>;
        using strides_type = yato::strides_array<0, size_type>;

        using iterator = std::add_pointer_t<Ty_>;
        using const_iterator = std::add_pointer_t<std::add_const_t<Ty_>>;
        using plain_iterator = iterator;
        using const_plain_iterator = const_iterator;

        using reference = std::add_lvalue_reference_t<Ty_>;
        using const_reference = std::add_lvalue_reference_t<std::add_const_t<Ty_>>;
        using value_reference = reference;
        using const_value_reference = const_reference;

        static YATO_CONSTEXPR_VAR bool has_method_size0 = false;
        static YATO_CONSTEXPR_VAR bool has_method_size1 = false;
        static YATO_CONSTEXPR_VAR bool has_method_stride = false;
        static YATO_CONSTEXPR_VAR bool has_method_continuous = false;
        static YATO_CONSTEXPR_VAR bool has_operator_subscript = false;
        static YATO_CONSTEXPR_VAR bool has_operator_csubscript = false;
        static YATO_CONSTEXPR_VAR bool has_method_dimensions = false;
        static YATO_CONSTEXPR_VAR bool has_method_dimensions_range = false;
        static YATO_CONSTEXPR_VAR bool has_method_strides = false;
        static YATO_CONSTEXPR_VAR bool has_method_total_size = false;
        static YATO_CONSTEXPR_VAR bool has_method_data = false;
        static YATO_CONSTEXPR_VAR bool has_method_cdata = false;
    };

    template <typename Ty_, std::size_t N_>
    struct is_container<Ty_[N_]>
        : std::true_type
    { };


    template <typename Ty_, std::size_t N_>
    struct container_traits<std::array<Ty_, N_>>
    {
        using std_array_type_ = std::array<Ty_, N_>;

        using value_type = typename std_array_type_::value_type;
        using size_type  = typename std_array_type_::size_type;
        using index_type = typename std_array_type_::size_type;

        using allocator_type = void;

        static YATO_CONSTEXPR_VAR std::size_t dimensions_number = 1;

        using dimensions_type = yato::dimensionality<dimensions_number, size_type>;
        using strides_type = yato::strides_array<0, size_type>;

        using iterator = typename std_array_type_::iterator;
        using const_iterator = typename std_array_type_::const_iterator;
        using plain_iterator = iterator;
        using const_plain_iterator = const_iterator;

        using reference = typename std_array_type_::reference;
        using const_reference = typename std_array_type_::const_reference;
        using value_reference = reference;
        using const_value_reference = const_reference;

        static YATO_CONSTEXPR_VAR bool has_method_size0 = true;
        static YATO_CONSTEXPR_VAR bool has_method_size1 = false;
        static YATO_CONSTEXPR_VAR bool has_method_stride = false;
        static YATO_CONSTEXPR_VAR bool has_method_continuous = false;
        static YATO_CONSTEXPR_VAR bool has_operator_subscript = true;
        static YATO_CONSTEXPR_VAR bool has_operator_csubscript = true;
        static YATO_CONSTEXPR_VAR bool has_method_dimensions = false;
        static YATO_CONSTEXPR_VAR bool has_method_dimensions_range = false;
        static YATO_CONSTEXPR_VAR bool has_method_strides = false;
        static YATO_CONSTEXPR_VAR bool has_method_total_size = false;
        static YATO_CONSTEXPR_VAR bool has_method_data = true;
        static YATO_CONSTEXPR_VAR bool has_method_cdata = false;
    };

    template <typename Ty_, std::size_t N_>
    struct is_container<std::array<Ty_, N_>>
        : std::true_type
    { };


    template <typename Ty_, typename Alloc_>
    struct container_traits<std::vector<Ty_, Alloc_>>
    {
        using std_vector_type_ = std::vector<Ty_, Alloc_>;

        using value_type = typename std_vector_type_::value_type;
        using size_type  = typename std_vector_type_::size_type;
        using index_type = typename std_vector_type_::size_type;

        using allocator_type = Alloc_;

        static YATO_CONSTEXPR_VAR std::size_t dimensions_number = 1;

        using dimensions_type = yato::dimensionality<dimensions_number, size_type>;
        using strides_type = yato::strides_array<0, size_type>;

        using iterator = typename std_vector_type_::iterator;
        using const_iterator = typename std_vector_type_::const_iterator;
        using plain_iterator = iterator;
        using const_plain_iterator = const_iterator;

        using reference = typename std_vector_type_::reference;
        using const_reference = typename std_vector_type_::const_reference;
        using value_reference = reference;
        using const_value_reference = const_reference;

        static YATO_CONSTEXPR_VAR bool has_method_size0 = true;
        static YATO_CONSTEXPR_VAR bool has_method_size1 = false;
        static YATO_CONSTEXPR_VAR bool has_method_stride = false;
        static YATO_CONSTEXPR_VAR bool has_method_continuous = false;
        static YATO_CONSTEXPR_VAR bool has_operator_subscript = true;
        static YATO_CONSTEXPR_VAR bool has_operator_csubscript = true;
        static YATO_CONSTEXPR_VAR bool has_method_dimensions = false;
        static YATO_CONSTEXPR_VAR bool has_method_dimensions_range = false;
        static YATO_CONSTEXPR_VAR bool has_method_strides = false;
        static YATO_CONSTEXPR_VAR bool has_method_total_size = false;
        static YATO_CONSTEXPR_VAR bool has_method_data = true;
        static YATO_CONSTEXPR_VAR bool has_method_cdata = false;
    };

    template <typename Ty_, typename Alloc_>
    struct is_container<std::vector<Ty_, Alloc_>>
        : std::true_type
    { };


    /**
     * Provides the standardized access to container properties and interface
     */
    template <typename Container_>
    struct container_ops
        : public container_traits<Container_>
    {
        static_assert(std::is_same<yato::remove_cvref_t<Container_>, Container_>::value, "Container_ must be a type without modifiers");
        static_assert(is_container<Container_>::value, "The Container_ type does not satisfy the yato::is_container<T> requirements.");

        using traits_type = container_traits<Container_>;
        using traits_type::dimensions_number;
        using typename traits_type::value_type;
        using typename traits_type::size_type;
        using typename traits_type::index_type;
        using typename traits_type::allocator_type;
        using typename traits_type::dimensions_type;
        using typename traits_type::strides_type;
        using typename traits_type::iterator;
        using typename traits_type::const_iterator;
        using typename traits_type::plain_iterator;
        using typename traits_type::const_plain_iterator;
        using typename traits_type::reference;
        using typename traits_type::const_reference;
        using typename traits_type::value_reference;
        using typename traits_type::const_value_reference;
        using traits_type::has_method_stride;
        using traits_type::has_method_continuous;
        using traits_type::has_operator_subscript;
        using traits_type::has_operator_csubscript;
        using traits_type::has_method_dimensions;
        using traits_type::has_method_total_size;
        using traits_type::has_method_data;
        using traits_type::has_method_cdata;

        static YATO_CONSTEXPR_VAR bool has_method_size0 = traits_type::has_method_size0 || traits_type::has_method_size1;
        static YATO_CONSTEXPR_VAR bool has_method_size1 = (dimensions_number < 1 && traits_type::has_method_size0) || traits_type::has_method_size1;
        static YATO_CONSTEXPR_VAR bool has_method_strides = traits_type::has_method_stride || traits_type::has_method_strides;
        static YATO_CONSTEXPR_VAR bool has_method_load = (traits_type::has_method_size0 || traits_type::has_method_size1) && traits_type::has_operator_csubscript;
        static YATO_CONSTEXPR_VAR bool has_method_loads = has_method_load;
        static YATO_CONSTEXPR_VAR bool has_method_dimensions_range = traits_type::has_method_dimensions_range || has_method_size1;


        static YATO_CONSTEXPR_FUNC
        size_type size(yato::disable1_if_not_t<traits_type::has_method_size0, const Container_&> c)
        {
            return c.size();
        }

        static YATO_CONSTEXPR_FUNC
        size_type size(yato::disable2_if_not_t<!traits_type::has_method_size0 && traits_type::has_method_size1, const Container_&> c)
        {
            return c.size(0);
        }

        static YATO_CONSTEXPR_FUNC
        size_type size(yato::disable1_if_not_t<traits_type::has_method_size1, const Container_&> c, std::size_t dim)
        {
            return c.size(dim);
        }

        static YATO_CONSTEXPR_FUNC
        size_type size(yato::disable2_if_not_t<!traits_type::has_method_size1 && traits_type::has_method_size0, const Container_&> c, std::size_t /*dim*/)
        {
            return c.size();
        }

        static YATO_CONSTEXPR_FUNC
        dimensions_type dimensions(yato::disable_if_not_t<has_method_dimensions, const Container_&> c)
        {
            return c.dimensions();
        }

        static YATO_CONSTEXPR_FUNC
        dimensions_type dimensions(yato::disable_if_t<has_method_dimensions, const Container_&> c)
        {
            return details::make_dims_from_size_<Container_, dimensions_type, dimensions_number>::apply(c);
        }

        static YATO_CONSTEXPR_FUNC
        auto dimensions_range(yato::disable1_if_not_t<traits_type::has_method_dimensions_range, const Container_&> c)
        {
            return c.dimensions_range();
        }

        static YATO_CONSTEXPR_FUNC
        auto dimensions_range(yato::disable2_if_not_t<!traits_type::has_method_dimensions_range && has_method_size1, const Container_&> c)
        {
            return yato::numeric_range(dimensions_number).map([&c](std::size_t dim) -> size_type { return size(c, dim); });
        }

        static YATO_CONSTEXPR_FUNC
        size_type total_size(yato::disable_if_not_t<has_method_total_size, const Container_&> c)
        {
            return c.total_size();
        }

        static YATO_CONSTEXPR_FUNC
        size_type total_size(yato::disable_if_t<has_method_total_size, const Container_&> c)
        {
            return dimensions(c).total_size();
        }

        static YATO_CONSTEXPR_FUNC
        bool continuous(yato::disable1_if_not_t<has_method_continuous, const Container_&> c)
        {
            return c.continuous();
        }

        static YATO_CONSTEXPR_FUNC
        bool continuous(yato::disable1_if_t<has_method_continuous, const Container_&> /*c*/)
        {
            return (dimensions_number == 1);
        }

        static YATO_CONSTEXPR_FUNC
        reference subscript(yato::disable1_if_not_t<has_operator_subscript, Container_&> c, const index_type& i)
        {
            return c[i];
        }

        static YATO_CONSTEXPR_FUNC
        reference subscript(yato::disable1_if_t<has_operator_subscript, Container_&> c, const index_type& i)
        {
            return *yato::next(std::begin(c), i);
        }

        static YATO_CONSTEXPR_FUNC
        decltype(auto) subscript(yato::disable2_if_not_t<has_operator_subscript, const Container_&> c, const index_type& i)
        {
            return c[i];
        }

        static YATO_CONSTEXPR_FUNC
        decltype(auto) subscript(yato::disable2_if_t<has_operator_subscript, const Container_&> c, const index_type& i)
        {
            return *yato::next(std::begin(c), i);
        }

        static YATO_CONSTEXPR_FUNC
        const_reference csubscript(yato::disable_if_not_t<has_operator_csubscript, const Container_&> c, const index_type& i)
        {
            return c[i];
        }

        static YATO_CONSTEXPR_FUNC
        const_reference csubscript(yato::disable_if_t<has_operator_csubscript, const Container_&> c, const index_type& i)
        {
            return *yato::next(std::cbegin(c), i);
        }

        static YATO_CONSTEXPR_FUNC
        iterator begin(Container_& c)
        {
            return std::begin(c);
        }

        static YATO_CONSTEXPR_FUNC
        decltype(auto) begin(const Container_& c)
        {
            return std::begin(c);
        }

        static YATO_CONSTEXPR_FUNC
        const_iterator cbegin(const Container_& c)
        {
            return std::cbegin(c);
        }

        static YATO_CONSTEXPR_FUNC
        iterator end(Container_& c)
        {
            return std::end(c);
        }

        static YATO_CONSTEXPR_FUNC
        decltype(auto) end(const Container_& c)
        {
            return std::end(c);
        }

        static YATO_CONSTEXPR_FUNC
        const_iterator cend(const Container_& c)
        {
            return std::cend(c);
        }

        /**
         * Optional interface
         */
        static YATO_CONSTEXPR_FUNC
        std::add_pointer_t<value_type> data(yato::disable1_if_not_t<has_method_data, Container_&> c)
        {
            return c.data();
        }

        /**
         * Optional interface
         */
        static YATO_CONSTEXPR_FUNC
        decltype(auto) data(yato::disable2_if_not_t<has_method_data, const Container_&> c)
        {
            return c.data();
        }

        /**
         * Optional interface
         */
        static YATO_CONSTEXPR_FUNC
        std::add_pointer_t<std::add_const_t<value_type>> cdata(yato::disable1_if_not_t<has_method_cdata, const Container_&> c)
        {
            return c.cdata();
        }

        /**
         * Optional interface
         */
        static YATO_CONSTEXPR_FUNC
        std::add_pointer_t<std::add_const_t<value_type>> cdata(yato::disable2_if_t<has_method_cdata || !has_method_data, const Container_&> c)
        {
            return c.data();
        }

        /**
         * Optional interface
         */
        static YATO_CONSTEXPR_FUNC
        std::size_t stride(yato::disable_if_not_t<traits_type::has_method_stride, const Container_&> c, std::size_t dim)
        {
            return c.stride(dim);
        }

        /**
         * Optional interface
         */
        static YATO_CONSTEXPR_FUNC
        strides_type strides(yato::disable1_if_not_t<traits_type::has_method_strides, const Container_&> c)
        {
            return c.strides();
        }

        /**
         * Optional interface
         */
        static YATO_CONSTEXPR_FUNC
        strides_type strides(yato::disable2_if_not_t<!traits_type::has_method_strides && traits_type::has_method_stride, const Container_&> c)
        {
            return details::make_array_from_strides_<Container_, strides_type, (dimensions_number > 0 ? dimensions_number - 1 : 0)>::apply(c);
        }

        /**
         * Optional interface
         */
        template <typename Sampler_ = sampler_default, typename... Indexes_>
        static YATO_CONSTEXPR_FUNC
        decltype(auto) load(const Container_& c, Indexes_&&... indexes);

        /**
         * Optional interface
         */
        template <typename Sampler_, typename... Indexes_>
        static YATO_CONSTEXPR_FUNC
        decltype(auto) loads(Sampler_&& s, const Container_& c, Indexes_&&... indexes);
    };



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
            -> return_type<typename container_traits<Container_>::value_type>
        {
            return load_impl_<typename container_traits<Container_>::value_type, 0>(s, c, std::forward<Indexes_>(indexes)...);
        }

        template <typename Container_, typename... Indexes_>
        static YATO_CONSTEXPR_FUNC
        auto load(Sampler_& s, const Container_& c, Indexes_&&... indexes)
            -> return_type<typename container_traits<Container_>::value_type>
        {
            return load_impl_<typename container_traits<Container_>::value_type, 0>(s, c, std::forward<Indexes_>(indexes)...);
        }

    private:
        template <typename ValueType_, size_t Dim_, typename SamplerRef_, typename Container_, typename... Indexes_>
        static YATO_CONSTEXPR_FUNC_CXX14
        return_type<ValueType_> load_impl_(SamplerRef_&& s, const Container_& c, index_type i0, index_type i1, Indexes_&&... indexes_tail)
        {
            using has_transform_index_ = details::has_transform_index_method<Sampler_, index_type, Dim_>;
            using has_boundary_value_  = details::has_boundary_value_method<Sampler_, ValueType_>;
            using container_ops = container_ops<Container_>;
            size_t effective_idx{};
            if (invoke_transform_index_<Dim_>(has_transform_index_{}, std::forward<SamplerRef_>(s), i0, container_ops::size(c, 0), effective_idx)) {
                return load_impl_<ValueType_, Dim_ + 1>(std::forward<SamplerRef_>(s), container_ops::csubscript(c, effective_idx), i1, std::forward<Indexes_>(indexes_tail)...);
            }
            else {
                return invoke_boundary_value_<ValueType_>(has_boundary_value_{}, std::forward<SamplerRef_>(s));
            }
        }

        template <typename ValueType_, size_t Dim_, typename SamplerRef_, typename Container_>
        static YATO_CONSTEXPR_FUNC_CXX14
        return_type<ValueType_> load_impl_(SamplerRef_&& s, const Container_& c, index_type i)
        {
            using has_transform_index_ = details::has_transform_index_method<Sampler_, index_type, Dim_>;
            using has_transform_value_ = details::has_transform_value_method<Sampler_, ValueType_>;
            using has_boundary_value_  = details::has_boundary_value_method<Sampler_, ValueType_>;
            using container_ops = container_ops<Container_>;
            size_t effective_idx{};
            if (invoke_transform_index_<Dim_>(has_transform_index_{}, std::forward<SamplerRef_>(s), i, container_ops::size(c, 0), effective_idx)) {
                return invoke_transform_value_<ValueType_>(has_transform_value_{}, std::forward<SamplerRef_>(s), container_ops::csubscript(c, effective_idx));
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
        static YATO_CONSTEXPR_FUNC_CXX14
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



    template <typename Container_>
    template <typename Sampler_, typename... Indexes_>
    YATO_CONSTEXPR_FUNC
    decltype(auto) container_ops<Container_>::load(const Container_& c, Indexes_&&... indexes)
    {
        return sampler_traits<Sampler_>::load(Sampler_{}, c, std::forward<Indexes_>(indexes)...);
    }

    template <typename Container_>
    template <typename Sampler_, typename... Indexes_>
    YATO_CONSTEXPR_FUNC
    decltype(auto) container_ops<Container_>::loads(Sampler_&& s, const Container_& c, Indexes_&&... indexes)
    {
        return sampler_traits<yato::remove_cvref_t<Sampler_>>::load(std::forward<Sampler_>(s), c, std::forward<Indexes_>(indexes)...);
    }


    template <typename Sampler_ = sampler_default, typename Container_, typename... Indexes_>
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

}

#endif //_YATO_CONTAINER_ND_H_
