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

namespace yato
{
    /**
     * Indicates container category
     */
    enum class container_tag
    {
        general,            ///< No assumptions about the container properties. The most general interface will be used. Fits for any STL container, even std::list and std::map.
        continuous          ///< The stored objects are always stored as an continous array. Fits for containers like std::array and std::vector.
    };


    namespace details
    {
        YATO_DEFINE_VALUE_GETTER(get_container_category, container_category, container_tag)
        YATO_DEFINE_TYPE_GETTER(get_size_type, size_type)
        YATO_DEFINE_TYPE_GETTER(get_allocator_type, allocator_type)
        YATO_DEFINE_VALUE_GETTER(get_dimensions_number, dimensions_number, std::size_t)
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
        YATO_DEFINE_VALUE_GETTER(get_has_method_continuous, has_method_continuous, bool)
        YATO_DEFINE_METHOD_CHECK_RET_0AGR(has_method_continuous, continuous)
        YATO_DEFINE_VALUE_GETTER(get_has_operator_subscript, has_operator_subscript, bool)
        YATO_DEFINE_VALUE_GETTER(get_has_operator_csubscript, has_operator_csubscript, bool)
        YATO_DEFINE_METHOD_CHECK_RET_1AGR(has_operator_subscript, operator[])
        YATO_DEFINE_VALUE_GETTER(get_has_method_dimensions, has_method_dimensions, bool)
        YATO_DEFINE_METHOD_CHECK_RET_0AGR(has_method_dimensions, dimensions)
        YATO_DEFINE_VALUE_GETTER(get_has_method_total_size, has_method_total_size, bool)
        YATO_DEFINE_METHOD_CHECK_RET_0AGR(has_method_total_size, total_size)
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

    } // namespace details;


    /**
     * Retruns container category.
     * The category defines change the container_ops behaviour.
     * Sometimes only overriding container_category is enough instead of overriding the whole container_traits and/or container_ops classes.
     */
    template <typename Container_>
    struct container_category_trait
    {
        static YATO_CONSTEXPR_VAR container_tag value = details::get_container_category<Container_, container_tag::general>::value;
    };

    template <typename Ty_, size_t Size_>
    struct container_category_trait<std::array<Ty_, Size_>>
    {
        static YATO_CONSTEXPR_VAR container_tag value = container_tag::continuous;
    };

    template <typename Ty_, typename Alloc_>
    struct container_category_trait<std::vector<Ty_, Alloc_>>
    {
        static YATO_CONSTEXPR_VAR container_tag value = container_tag::continuous;
    };


    /**
     * Provides the standardized access to container properties
     */
    template <typename Container_>
    struct container_traits
    {
        static_assert(std::is_same<yato::remove_cvref_t<Container_>, Container_>::value, "Container_ must be a type without modifiers");

        /**
         * Alias to yato::container_category<Container>
         */
        static YATO_CONSTEXPR_VAR container_tag container_category = yato::container_category_trait<Container_>::value;

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
         * Equals to Container::dimensions_number if present, otherwise 1.
         */
        static YATO_CONSTEXPR_VAR std::size_t dimensions_number = details::get_dimensions_number<Container_, 1>::value;
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
                details::has_method_dimensions<const Container_, bool>::value>::value;

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
            typename yato::container_traits<Ty_>::value_type,
            typename yato::container_traits<Ty_>::iterator,
            typename yato::container_traits<Ty_>::const_iterator
        >::type
    >
        : std::integral_constant<bool,
            ((yato::container_traits<Ty_>::dimensions_number == 1) && yato::container_traits<Ty_>::has_method_size0) || yato::container_traits<Ty_>::has_method_size1
        >
    { };


    /**
     * Provides the standardized access to container properties and interface
     */
    template <typename Container_>
    struct container_ops
        : public container_traits<Container_>
    {
        static_assert(is_container<Container_>::value, "The Container_ type does not satisfy the yato::is_container<T> requirements.");

        using traits_type = container_traits<Container_>;
        using traits_type::container_category;
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
        using traits_type::has_method_size0;
        using traits_type::has_method_size1;
        using traits_type::has_method_continuous;
        using traits_type::has_operator_subscript;
        using traits_type::has_operator_csubscript;
        using traits_type::has_method_dimensions;
        using traits_type::has_method_total_size;
        using traits_type::has_method_data;
        using traits_type::has_method_cdata;


        static YATO_CONSTEXPR_FUNC
        size_type size(yato::disable_if_not_t<has_method_size0, const Container_&> c)
        {
            return c.size();
        }

        static YATO_CONSTEXPR_FUNC
        size_type size(yato::disable_if_t<has_method_size0, const Container_&> c)
        {
            return c.size(0);
        }

        static YATO_CONSTEXPR_FUNC
        size_type size(yato::disable_if_not_t<(dimensions_number > 1), const Container_&> c, size_t dim)
        {
            return c.size(dim);
        }

        static YATO_CONSTEXPR_FUNC
        size_type size(yato::disable_if_not_t<(dimensions_number == 1), const Container_&> c, size_t /*dim*/)
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
        bool continuous(yato::disable_if_not_t<has_method_continuous, const Container_&> c)
        {
            return c.continuous();
        }

        static YATO_CONSTEXPR_FUNC
        bool continuous(yato::disable_if_t<has_method_continuous, const Container_&> /*c*/)
        {
            return (container_category == container_tag::continuous);
        }

        static YATO_CONSTEXPR_FUNC
        reference subscript(yato::disable_if_not_t<has_operator_subscript, Container_&> c, const index_type& i)
        {
            return c[i];
        }

        static YATO_CONSTEXPR_FUNC
        reference subscript(yato::disable_if_t<has_operator_subscript, Container_&> c, const index_type& i)
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
        const_iterator cend(const Container_& c)
        {
            return std::cend(c);
        }

        static YATO_CONSTEXPR_FUNC
        std::add_pointer_t<value_type> data(yato::disable_if_not_t<has_method_data, Container_&> c)
        {
            return c.data();
        }

        static YATO_CONSTEXPR_FUNC
        std::add_pointer_t<std::add_const_t<value_type>> cdata(yato::disable1_if_not_t<has_method_cdata, const Container_&> c)
        {
            return c.cdata();
        }

        static YATO_CONSTEXPR_FUNC
        std::add_pointer_t<std::add_const_t<value_type>> cdata(yato::disable2_if_t<has_method_cdata || !has_method_data, const Container_&> c)
        {
            return c.data();
        }
    };



    template <typename ValueType_, size_t Dimensions_, typename Implementation_>
    class const_container_nd
    {
        static_assert(!std::is_reference<ValueType_>::value, "ValueType_ cannot be a reference");
        static_assert(Dimensions_ > 0, "Invalid dimensions");

        using this_type = const_container_nd<ValueType_, Dimensions_, Implementation_>;

    public:
        using value_type = ValueType_;
        using implementation_type = Implementation_;
        using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = Dimensions_;

        explicit
        operator const Implementation_&() const
        {
            return *this;
        }

        /**
         * Access sub-element
         */
        decltype(auto) operator[](size_t idx) const
        {
            return static_cast<const Implementation_*>(this)->operator[](idx);
        }

        /**
         *  Element access with bounds check
         */
        template<typename... Indexes_>
        decltype(auto) at(Indexes_&&... indexes) const
        {
            return static_cast<const Implementation_*>(this)->at(std::forward<Indexes_>(indexes)...);
        }

        /**
         * Chech is view data has no gaps, i.e. strides are equal to sizes
         * Plain iterators are valid only if view is continuous
         */
        bool continuous() const
        {
            return static_cast<const Implementation_*>(this)->continuous();
        }

        /**
         *  Get size of specified dimension
         */
        size_t size(size_t idx) const
        {
            return static_cast<const Implementation_*>(this)->size(idx);
        }

        /**
         *  Get stride of specified dimension. 
         *  Dimensions number for strides is less by one.
         */
        size_t stride(size_t idx) const
        {
            return static_cast<const Implementation_*>(this)->stride(idx);
        }

        /**
         * Get total number of elements
         */
        size_t total_size() const
        {
            return static_cast<const Implementation_*>(this)->total_size();
        }

        /**
         * Get total number of bytes counting strides
         */
        size_t total_stored() const
        {
            return static_cast<const Implementation_*>(this)->total_stored();
        }

        /**
         * Get dimensions array
         */
        decltype(auto) dimensions() const
        {
            return static_cast<const Implementation_*>(this)->dimensions();
        }

        /**
         * Get dimensions array
         */
        decltype(auto) strides() const
        {
            return static_cast<const Implementation_*>(this)->strides();
        }

        /**
         * Get dimensions range
         */
        decltype(auto) dimensions_range() const
        {
            return static_cast<const Implementation_*>(this)->dimensions_range();
        }

        /**
         * Get strides range
         */
        decltype(auto) strides_range() const
        {
            return static_cast<const Implementation_*>(this)->strides_range();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) cbegin() const
        {
            return static_cast<const Implementation_*>(this)->cbegin();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) cend() const
        {
            return static_cast<const Implementation_*>(this)->cend();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) plain_cbegin() const
        {
            return static_cast<const Implementation_*>(this)->plain_cbegin();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) plain_cend() const
        {
            return static_cast<const Implementation_*>(this)->plain_cend();
        }

        /**
         * Raw pointer to underlying data
         */
        decltype(auto) cdata() const
        {
            return static_cast<const Implementation_*>(this)->cdata();
        }

    };


    template <typename ValueType_, size_t Dimensions_, typename Implementation_>
    class container_nd
        : public const_container_nd<ValueType_, Dimensions_, Implementation_>
    {
        static_assert(!std::is_reference<ValueType_>::value, "ValueType_ cannot be a reference");
        static_assert(Dimensions_ > 0, "Invalid dimensions");

        using this_type = container_nd<ValueType_, Dimensions_, Implementation_>;

    public:
        using value_type = ValueType_;
        using implementation_type = Implementation_;
        using reference = std::add_lvalue_reference_t<value_type>;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = Dimensions_;

        explicit
        operator Implementation_&() const
        {
            return *this;
        }

        /**
         * Access sub-element
         */
        decltype(auto) operator[](size_t idx) const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->operator[](idx);
        }

        /**
         *  Element access with bounds check
         */
        template<typename... Indexes_>
        decltype(auto) at(Indexes_&&... indexes) const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->at(std::forward<Indexes_>(indexes)...);
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) begin() const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->begin();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) end() const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->end();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) plain_begin() const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->plain_begin();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) plain_end() const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->plain_end();
        }

        /**
         * Raw pointer to underlying data
         */
        decltype(auto) data() const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->data();
        }

    };


    namespace details
    {
        template <typename ValueType_, size_t Dimensions_, typename Implementation_>
        using choose_container_interface_t = std::conditional_t<std::is_const<ValueType_>::value,
            yato::const_container_nd<std::remove_const_t<ValueType_>, Dimensions_, Implementation_>,
            yato::container_nd<ValueType_, Dimensions_, Implementation_>
        >;

    } // namespace details


    template <typename ValueType_, typename Implementation_>
    using const_container_1d = const_container_nd<ValueType_, 1, Implementation_>;

    template <typename ValueType_, typename Implementation_>
    using const_container_2d = const_container_nd<ValueType_, 2, Implementation_>;

    template <typename ValueType_, typename Implementation_>
    using const_container_3d = const_container_nd<ValueType_, 3, Implementation_>;

    template <typename ValueType_, typename Implementation_>
    using container_1d = container_nd<ValueType_, 1, Implementation_>;

    template <typename ValueType_, typename Implementation_>
    using container_2d = container_nd<ValueType_, 2, Implementation_>;

    template <typename ValueType_, typename Implementation_>
    using container_3d = container_nd<ValueType_, 3, Implementation_>;

}

#endif //_YATO_CONTAINER_ND_H_
