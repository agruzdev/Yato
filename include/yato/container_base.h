/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
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
    };


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
        container_type m_extents;
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
        SizeType total_size() const
        {
            return total_size_impl_(0);
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


    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t length(_ContainerType && container)
    {
#ifdef YATO_HAS_CONSTEXPR_CXX14
        YATO_REQUIRES(container.dimensions_num() == 1);
#endif
        return container.size(0);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t height_2d(_ContainerType && container)
    {
#ifdef YATO_HAS_CONSTEXPR_CXX14
        YATO_REQUIRES(container.dimensions_num() == 2);
#endif
        return container.size(0);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t width_2d(_ContainerType && container)
    {
#ifdef YATO_HAS_CONSTEXPR_CXX14
        YATO_REQUIRES(container.dimensions_num() == 2);
#endif
        return container.size(1);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t depth_3d(_ContainerType && container)
    {
#ifdef YATO_HAS_CONSTEXPR_CXX14
        YATO_REQUIRES(container.dimensions_num() == 3);
#endif
        return container.size(0);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t height_3d(_ContainerType && container)
    {
#ifdef YATO_HAS_CONSTEXPR_CXX14
        YATO_REQUIRES(container.dimensions_num() == 3);
#endif
        return container.size(1);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t width_3d(_ContainerType && container)
    {
#ifdef YATO_HAS_CONSTEXPR_CXX14
        YATO_REQUIRES(container.dimensions_num() == 3);
#endif
        return container.size(2);
    }

}


#endif
