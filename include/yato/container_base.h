/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_CONTAINER_BASE_H_
#define _YATO_CONTAINER_BASE_H_

#include <array>
#include <tuple>
#include "type_traits.h"
#include "range.h"

namespace yato
{
    /**
     * Trait class for dimension descriptors used in containers
     * Stores size and offset
     *
     * [     size 0     ] [        size 1        ] [      size 2          ] ...
     * [   total_size   ] [ sub_view<N - 1> size ] [ sub_view<N - 2> size ] ...
     */
    template <typename SizeType = size_t>
    struct dimension_descriptor
    {
        using size_type = SizeType;
        using type = std::tuple<size_type, size_type>;

        static YATO_CONSTEXPR_VAR size_t idx_size   = 0;
        static YATO_CONSTEXPR_VAR size_t idx_total  = 1;
        static YATO_CONSTEXPR_VAR size_t idx_offset = 1;

        template <typename Ty_>
        static
        size_t offset_to_bytes(size_t offset)
        {
            return offset * sizeof(Ty_);
        }
    };

    /**
     * Trait class for dimension descriptors used in containers
     * Stores size, stride and offset
     *
     * [     size 0       ] [        size 1        ] [      size 2          ] ...
     * [   total_size     ] [ sub_view<N - 1> size ] [ sub_view<N - 2> size ] ...
     * [   total_stored   ] [    offset<N - 1>     ] [    offset<N - 2>     ] ...
     */
    template <typename SizeType = size_t>
    struct dimension_descriptor_strided
    {
        using size_type = SizeType;
        using type = std::tuple<size_type, size_type, size_type>;

        static YATO_CONSTEXPR_VAR size_t idx_size   = 0;
        static YATO_CONSTEXPR_VAR size_t idx_total  = 1;
        static YATO_CONSTEXPR_VAR size_t idx_offset = 2;

        template <typename Ty_>
        static
        size_t offset_to_bytes(size_t offset)
        {
            return offset;
        }
    };


    /**
     * Sampling traits
     * Default sampler behaves similar to standard at() throwing out_of_bounds exception for invalid indexes
     * Each sampler must provide:
     *   index_type - type used as argument
     *   return_type<> - alias template remapping value_type to the type returned by sampler access
     *   transform_index() - function transforming provided input index to effective index. If returns true, then out_idx will be used for element access, else boundary_value() will be returned.
     *   boundary_value() - function providing read-only value returned for invalid indexes.
     */
    struct sampler_default
    {
        using index_type = std::size_t;

        template <typename ValueType_>
        using return_type = std::add_lvalue_reference_t<std::add_const_t<ValueType_>>;

        YATO_CONSTEXPR_FUNC
        bool transform_index(index_type in_idx, std::size_t size, std::ptrdiff_t& out_idx) const
        {
            if (in_idx < size) {
                out_idx = yato::narrow_cast<std::ptrdiff_t>(in_idx);
                return true;
            }
            return false;
        }

        template <typename ValueType_>
        YATO_NORETURN
        return_type<ValueType_> boundary_value() const
        {
            throw yato::out_of_range_error("index is out of bounds");
        }
    };


    /**
     * Performs no boundary check allowing any access
     */
    struct sampler_no_check
    {
        using index_type = std::ptrdiff_t;

        template <typename ValueType_>
        using return_type = ValueType_;

        YATO_CONSTEXPR_FUNC
        bool transform_index(index_type in_idx, std::size_t /*size*/, std::ptrdiff_t& out_idx) const
        {
            out_idx = yato::narrow_cast<std::ptrdiff_t>(in_idx);
            return true;
        }

        template <typename ValueType_>
        return_type<ValueType_> boundary_value() const
        {
            YATO_ASSERT(false, "boundary_value() should not be called.");
            return static_cast<ValueType_>(0);
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

        YATO_CONSTEXPR_FUNC
        bool transform_index(index_type in_idx, std::size_t size, std::ptrdiff_t& out_idx) const
        {
            if (in_idx >= 0 && static_cast<std::size_t>(in_idx) < size) {
                out_idx = yato::narrow_cast<std::ptrdiff_t>(in_idx);
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

        template <typename ValueType_>
        using return_type = std::add_lvalue_reference_t<std::add_const_t<ValueType_>>;

        YATO_CONSTEXPR_FUNC
        bool transform_index(index_type in_idx, std::size_t size, std::ptrdiff_t& out_idx) const
        {
            YATO_REQUIRES(size != 0);
            out_idx = std::min<std::ptrdiff_t>(std::max<std::ptrdiff_t>(0, yato::narrow_cast<std::ptrdiff_t>(in_idx)), size - 1);
            return true;
        }

        template <typename ValueType_>
        return_type<ValueType_> boundary_value() const
        {
            YATO_ASSERT(false, "boundary_value() should not be called.");
            throw std::logic_error("Should not be called");
        }
    };




    namespace details
    {
        // Interpret as byte offset
        template <typename Ty_>
        static
        void advance_bytes(Ty_* & ptr, std::ptrdiff_t diff)
        {
            ptr = yato::pointer_cast<Ty_*>(yato::pointer_cast<typename yato::take_cv_from<Ty_, uint8_t>::type*>(ptr) + diff);
        }
    }

    /**
     * Storage for containers extents
     */
    template <size_t DimensionsNum, typename SizeType = size_t>
    class dimensionality
    {
    private:
        using container_type = std::array<SizeType, DimensionsNum>;
        //---------------------------------------------------

    public:
        using size_type = SizeType;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = DimensionsNum;
        using iterator = typename container_type::iterator;
        using const_iterator = typename container_type::const_iterator;
        using sub_dimensionality = dimensionality<DimensionsNum - 1, SizeType>;
        //---------------------------------------------------

    private:
        container_type m_extents = {};
        //---------------------------------------------------

        YATO_CONSTEXPR_FUNC
        size_type total_size_impl_(size_t idx) const
        {
            return idx >= dimensions_number - 1
                ? m_extents[dimensions_number - 1]
                : m_extents[idx] * total_size_impl_(idx + 1);
        }
        //---------------------------------------------------

    public:
        YATO_CONSTEXPR_FUNC
        dimensionality() = default;

YATO_PRAGMA_WARNING_PUSH
YATO_CLANG_WARNING_IGNORE("-Wmissing-braces")
        template <typename... Args_>
        YATO_CONSTEXPR_FUNC explicit 
        dimensionality(size_type extent1, Args_... extents) YATO_NOEXCEPT_KEYWORD
            : m_extents({ std::move(extent1), std::move(extents)... })
        {
            // ToDo (a.gruzdev): Make SFINAE friendly if necessary
            static_assert(sizeof...(extents) + 1 == dimensions_number, "yato::dimensionality[dimensionality]: Invalid dimensions number");
        }
YATO_PRAGMA_WARNING_POP

        template <typename SizeIter>
        YATO_CONSTEXPR_FUNC_CXX14 explicit
        dimensionality(const range<SizeIter> & extents)
        {
            YATO_REQUIRES(extents.distance() == dimensions_number);
            std::copy(extents.begin(), extents.end(), m_extents.begin());
        }

        YATO_CONSTEXPR_FUNC
        dimensionality(const dimensionality &) = default;

        dimensionality(dimensionality &&) = default;

        dimensionality & operator = (const dimensionality &) = default;

        dimensionality & operator = (dimensionality &&) = default;

        ~dimensionality() = default;

        YATO_CONSTEXPR_FUNC
        const size_type & operator[](size_t idx) const
        {
            return m_extents[idx];
        }

        YATO_CONSTEXPR_FUNC
        size_type & operator[](size_t idx)
        {
            return m_extents[idx];
        }

        const size_type & at(size_t idx) const
        {
            return m_extents.at(idx);
        }

        size_type & at(size_t idx)
        {
            return m_extents.at(idx);
        }

        YATO_CONSTEXPR_FUNC
        size_t dimensions_num() const
        {
            return dimensions_number;
        }

        YATO_CONSTEXPR_FUNC
        size_t size() const
        {
            return dimensions_number;
        }

        YATO_CONSTEXPR_FUNC
        size_type total_size() const
        {
            return total_size_impl_(0);
        }

        YATO_CONSTEXPR_FUNC
        size_type front() const
        {
            return m_extents.front();
        }

        YATO_CONSTEXPR_FUNC
        size_type front()
        {
            return m_extents.front();
        }

        YATO_CONSTEXPR_FUNC
        size_type back() const
        {
            return m_extents.back();
        }

        YATO_CONSTEXPR_FUNC
        size_type back()
        {
            return m_extents.back();
        }

        iterator begin()
        {
            return m_extents.begin();
        }

        const_iterator cbegin() const
        {
            return m_extents.cbegin();
        }

        iterator end()
        {
            return m_extents.end();
        }

        const_iterator cend() const
        {
            return m_extents.cend();
        }

        YATO_CONSTEXPR_FUNC_CXX14
        sub_dimensionality sub_dims() const
        {
            return sub_dimensionality(make_range(std::next(m_extents.cbegin()), m_extents.cend()));
        }
    };

    template <size_t DimsNum_, typename SizeType1_, typename SizeType2_>
    YATO_CONSTEXPR_FUNC
    bool operator == (const dimensionality<DimsNum_, SizeType1_> & dims1, const dimensionality<DimsNum_, SizeType2_> & dims2)
    {
        return std::equal(dims1.cbegin(), dims1.cend(), dims2.cbegin());
    }

    template <size_t DimsNum_, typename SizeType1_, typename SizeType2_>
    YATO_CONSTEXPR_FUNC
    bool operator != (const dimensionality<DimsNum_, SizeType1_> & dims1, const dimensionality<DimsNum_, SizeType2_> & dims2)
    {
        return !(dims1 == dims2);
    }

    /**
     * Zero-dimensional extents
     */
    template <typename SizeType>
    class dimensionality<0, SizeType>
    {
    public:
        using size_type = SizeType;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = 0;

        YATO_CONSTEXPR_FUNC
        dimensionality() = default;

        template <typename SizeIter>
        YATO_CONSTEXPR_FUNC_CXX14 explicit
        dimensionality(const range<SizeIter> &)
        { }

        ~dimensionality() = default;

        YATO_CONSTEXPR_FUNC
        dimensionality(const dimensionality &) = default;

        dimensionality(dimensionality &&) noexcept = default;

        dimensionality & operator = (const dimensionality &) = default;

        dimensionality & operator = (dimensionality &&) noexcept = default;

        YATO_CONSTEXPR_FUNC
        size_t dimensions_num() const
        {
            return dimensions_number;
        }
    };

    /**
     * Helper function for creating dimensionality
     */
    template <typename SizeType_ = size_t, typename... Extents_>
    YATO_CONSTEXPR_FUNC
    auto dims(Extents_... extents)
        -> std::enable_if_t<std::is_integral<SizeType_>::value, dimensionality<sizeof...(Extents_), SizeType_>>
    {
        return dimensionality<sizeof...(Extents_), SizeType_>(narrow_cast<SizeType_>(extents)...);
    }

    /**
     * Helper function for creating dimensionality
     */
#ifdef YATO_CXX17
    // Requires implicit inline modifier for static constexpr
    template <typename SizeType_ = size_t, typename... Extents_>
    YATO_CONSTEXPR_FUNC
    auto dims(Extents_ && ... extents)
        -> std::enable_if_t<!std::is_integral<SizeType_>::value, dimensionality<sizeof...(Extents_), SizeType_>>
    {
        return dimensionality<sizeof...(Extents_), SizeType_>(std::forward<Extents_>(extents)...);
    }
#else
    template <typename SizeType_ = size_t, typename... Extents_>
    YATO_CONSTEXPR_FUNC
    auto dims(Extents_... extents)
        -> std::enable_if_t<!std::is_integral<SizeType_>::value, dimensionality<sizeof...(Extents_), SizeType_>>
    {
        return dimensionality<sizeof...(Extents_), SizeType_>(extents...);
    }
#endif

    //----------------------------------------------------------------------------------

    template <size_t DimensionsNum_, typename SizeType_ = size_t>
    class strides_array
    {
    public:
        static YATO_CONSTEXPR_VAR size_t dimensions_number = DimensionsNum_;
        using size_type = SizeType_;

        using iterator       = typename std::array<size_type, dimensions_number>::iterator;
        using const_iterator = typename std::array<size_type, dimensions_number>::const_iterator;

    private:
        std::array<size_type, dimensions_number> m_values;

    public:
        template <typename... Args_>
        strides_array(std::enable_if_t<(sizeof...(Args_) == dimensions_number - 1), size_type> arg0, Args_... args)
            : m_values{{arg0, args...}}
        { }

        template <typename SizeIter>
        YATO_CONSTEXPR_FUNC_CXX14 explicit
        strides_array(const range<SizeIter> & extents)
        {
            YATO_REQUIRES(extents.distance() == dimensions_number);
            std::copy(extents.begin(), extents.end(), m_values.begin());
        }

        strides_array(const strides_array&) = default;
        strides_array(strides_array&&) = default;

        strides_array& operator=(const strides_array&) = default;
        strides_array& operator=(strides_array&&) = default;

        ~strides_array() = default;

        YATO_CONSTEXPR_FUNC
        const size_type & operator[](size_t idx) const
        {
            return m_values[idx];
        }

        size_type & operator[](size_t idx)
        {
            return m_values[idx];
        }

        iterator begin()
        {
            return m_values.begin();
        }

        const_iterator cbegin() const
        {
            return m_values.cbegin();
        }

        iterator end()
        {
            return m_values.end();
        }

        const_iterator cend() const
        {
            return m_values.cend();
        }
    };

    template <typename SizeType_>
    class strides_array<0, SizeType_>
    {
    public:
        static YATO_CONSTEXPR_VAR size_t dimensions_number = 0;
        using size_type = SizeType_;

        template <typename SizeIter>
        YATO_CONSTEXPR_FUNC_CXX14 explicit
        strides_array(const range<SizeIter> &)
        { }
    };


    template <size_t DimsNum_, typename SizeType1_, typename SizeType2_>
    YATO_CONSTEXPR_FUNC
    bool operator == (const strides_array<DimsNum_, SizeType1_> & dims1, const strides_array<DimsNum_, SizeType2_> & dims2)
    {
        return std::equal(dims1.cbegin(), dims1.cend(), dims2.cbegin());
    }

    template <size_t DimsNum_, typename SizeType1_, typename SizeType2_>
    YATO_CONSTEXPR_FUNC
    bool operator != (const strides_array<DimsNum_, SizeType1_> & dims1, const strides_array<DimsNum_, SizeType2_> & dims2)
    {
        return !(dims1 == dims2);
    }


    /**
     * Helper function for creating dimensionality
     */
    template <typename SizeType_ = size_t, typename... Offsets_>
    YATO_CONSTEXPR_FUNC
    auto strides(Offsets_... offsets)
        -> std::enable_if_t<std::is_integral<SizeType_>::value, strides_array<sizeof...(Offsets_), SizeType_>>
    {
        return strides_array<sizeof...(Offsets_), SizeType_>(narrow_cast<SizeType_>(offsets)...);
    }

    /**
     * Helper function for creating dimensionality
     */
#ifdef YATO_CXX17
    // Requires implicit inline modifier for static constexpr
    template <typename SizeType_ = size_t, typename... Offsets_>
    YATO_CONSTEXPR_FUNC
    auto strides(Offsets_ && ... offsets)
        -> std::enable_if_t<!std::is_integral<SizeType_>::value, strides_array<sizeof...(Offsets_), SizeType_>>
    {
        return strides_array<sizeof...(Offsets_), SizeType_>(std::forward<Offsets_>(offsets)...);
    }
#else
    template <typename SizeType_ = size_t, typename... Offsets_>
    YATO_CONSTEXPR_FUNC
    auto strides(Offsets_... offsets)
        -> std::enable_if_t<!std::is_integral<SizeType_>::value, strides_array<sizeof...(Offsets_), SizeType_>>
    {
        return strides_array<sizeof...(Offsets_), SizeType_>(offsets...);
    }
#endif


    //----------------------------------------------------------------------------------


    template <typename ContainerType_>
    YATO_CONSTEXPR_FUNC
    auto length(ContainerType_ && container)
        -> std::enable_if_t<yato::remove_cvref_t<ContainerType_>::dimensions_number == 1, size_t>
    {
        return container.size(0);
    }

    template <typename ContainerType_>
    YATO_CONSTEXPR_FUNC
    auto width(ContainerType_ && container)
        -> std::enable_if_t<yato::remove_cvref_t<ContainerType_>::dimensions_number == 2, size_t>
    {
        return container.size(1);
    }

    template <typename ContainerType_>
    YATO_CONSTEXPR_FUNC
    auto height(ContainerType_ && container)
        -> std::enable_if_t<yato::remove_cvref_t<ContainerType_>::dimensions_number == 2, size_t>
    {
        return container.size(0);
    }

    template <typename ContainerType_>
    YATO_CONSTEXPR_FUNC
    auto width(ContainerType_ && container)
        -> std::enable_if_t<yato::remove_cvref_t<ContainerType_>::dimensions_number == 3, size_t>
    {
        return container.size(2);
    }

    template <typename ContainerType_>
    YATO_CONSTEXPR_FUNC
    auto height(ContainerType_ && container)
        -> std::enable_if_t<yato::remove_cvref_t<ContainerType_>::dimensions_number == 3, size_t>
    {
        return container.size(1);
    }

    template <typename ContainerType_>
    YATO_CONSTEXPR_FUNC
    auto depth(ContainerType_ && container)
        -> std::enable_if_t<yato::remove_cvref_t<ContainerType_>::dimensions_number == 3, size_t>
    {
        return container.size(0);
    }

    template <typename ContainerType_>
    YATO_CONSTEXPR_FUNC
    auto layers(ContainerType_ && container)
        -> std::enable_if_t<yato::remove_cvref_t<ContainerType_>::dimensions_number == 4, size_t>
    {
        return container.size(0);
    }

    template <typename ContainerType_>
    YATO_CONSTEXPR_FUNC
    auto depth(ContainerType_ && container)
        -> std::enable_if_t<yato::remove_cvref_t<ContainerType_>::dimensions_number == 4, size_t>
    {
        return container.size(1);
    }

    template <typename ContainerType_>
    YATO_CONSTEXPR_FUNC
    auto height(ContainerType_ && container)
        -> std::enable_if_t<yato::remove_cvref_t<ContainerType_>::dimensions_number == 4, size_t>
    {
        return container.size(2);
    }

    template <typename ContainerType_>
    YATO_CONSTEXPR_FUNC
    auto width(ContainerType_ && container)
        -> std::enable_if_t<yato::remove_cvref_t<ContainerType_>::dimensions_number == 4, size_t>
    {
        return container.size(3);
    }



    //-----------------------------------------------------------------------------
    // Multidim iterator/container trait

    template <typename Ty_, typename = void>
    struct is_multidimensional
        : std::false_type
    { };

    template <typename Ty_>
    struct is_multidimensional <
        Ty_,
        std::enable_if_t<(Ty_::dimensions_number >= 0)>
    >
        : std::true_type
    { };


    //-----------------------------------------------------------------------------
    // Access policy

    enum class proxy_access_policy
    {
        lvalue_ref,   ///< Return lvalue referrence to underlying data
        rvalue_ref    ///< Return rvalue referrence to underlying data
    };

    template <typename ValueType_, proxy_access_policy AccessPolicy_>
    struct proxy_access_traits
    { };

    template <typename ValueType_>
    struct proxy_access_traits<ValueType_, proxy_access_policy::lvalue_ref>
    {
        using reference             = std::add_lvalue_reference_t<ValueType_>;
        using plain_iterator        = std::add_pointer_t<ValueType_>;
        using const_plain_iterator  = std::add_pointer_t<std::add_const_t<ValueType_>>;
        using const_value_type      = std::add_const_t<ValueType_>;
    };

    template <typename ValueType_>
    struct proxy_access_traits<ValueType_, proxy_access_policy::rvalue_ref>
    {
        using reference             = std::add_rvalue_reference_t<ValueType_>;
        using plain_iterator        = std::move_iterator<std::add_pointer_t<ValueType_>>;
        using const_plain_iterator  = std::move_iterator<std::add_pointer_t<ValueType_>>;
        using const_value_type      = ValueType_;
    };

    //-----------------------------------------------------------------------------
    //  make_move_iterator with support of array_proxy

    template <typename Iterator_>
    YATO_CONSTEXPR_FUNC
    std::move_iterator<Iterator_> make_move_iterator(Iterator_ it)
    {
        return std::move_iterator<Iterator_>(it);
    }

}


#endif
