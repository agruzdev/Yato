/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
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
     * [  total_reserved  ] [    offset<N - 1>     ] [    offset<N - 2>     ] ...
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
        using my_type = dimensionality<DimensionsNum, SizeType>;
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
        const size_type total_size_impl(size_t idx) const
        {
            return idx >= dimensions_number - 1
                ? m_extents[dimensions_number - 1]
                : m_extents[idx] * total_size_impl(idx + 1);
        }
        //---------------------------------------------------

    public:
        YATO_CONSTEXPR_FUNC
        dimensionality()
            : m_extents()
        { }

YATO_PRAGMA_WARNING_PUSH
YATO_CLANG_WARNING_IGNORE("-Wmissing-braces")
        template <typename... Args>
        YATO_CONSTEXPR_FUNC explicit 
        dimensionality(const size_type & extent1, const Args &... extents) YATO_NOEXCEPT_KEYWORD
            : m_extents({ extent1, extents... })
        {
            // ToDo (a.gruzdev): Make SFINAE friendly if necessary
            static_assert(sizeof...(extents) + 1 == dimensions_number, "yato::dimensionality[dimensionality]: Invalid dimensions number");
        }
YATO_PRAGMA_WARNING_POP

        template <typename SizeIter>
        YATO_CONSTEXPR_FUNC_EX explicit
        dimensionality(const range<SizeIter> & extents)
        {
            YATO_REQUIRES(extents.distance() == dimensions_number);
            std::copy(extents.begin(), extents.end(), m_extents.begin());
        }

        YATO_CONSTEXPR_FUNC
        dimensionality(const my_type &) = default;
#ifndef YATO_MSVC_2013
        dimensionality(my_type &&) = default;
#endif
        ~dimensionality() = default;

        my_type & operator = (const my_type &) = default;
#ifndef YATO_MSVC_2013
        my_type & operator = (my_type &&) = default;
#endif
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
        const size_t dimensions_num() const
        {
            return dimensions_number;
        }

        YATO_CONSTEXPR_FUNC
        const SizeType total_size() const
        {
            return total_size_impl(0);
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

        YATO_CONSTEXPR_FUNC_EX
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
        dimensionality()
        { }

        template <typename SizeIter>
        YATO_CONSTEXPR_FUNC_EX explicit
        dimensionality(const range<SizeIter> &)
        { }

        ~dimensionality() = default;

        YATO_CONSTEXPR_FUNC
        const size_t dimensions_num() const
        {
            return dimensions_number;
        }
    };

    /**
     * Helper function for creating dimensionality
     */
    template <typename SizeType = size_t, typename... Extents>
    YATO_CONSTEXPR_FUNC
    dimensionality<sizeof...(Extents), SizeType> dims(const Extents &... extents)
    {
        return dimensionality<sizeof...(Extents), SizeType>(narrow_cast<SizeType>(extents)...);
    }

    //----------------------------------------------------------------------------------


    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t length(_ContainerType && container)
    {
        YATO_REQUIRES(container.dimensions_num() == 1);
        return container.size(0);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t height_2d(_ContainerType && container)
    {
        YATO_REQUIRES(container.dimensions_num() == 2);
        return container.size(0);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t width_2d(_ContainerType && container)
    {
        YATO_REQUIRES(container.dimensions_num() == 2);
        return container.size(1);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t depth_3d(_ContainerType && container)
    {
        YATO_REQUIRES(container.dimensions_num() == 3);
        return container.size(0);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t height_3d(_ContainerType && container)
    {
        YATO_REQUIRES(container.dimensions_num() == 3);
        return container.size(1);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t width_3d(_ContainerType && container)
    {
        YATO_REQUIRES(container.dimensions_num() == 3);
        return container.size(2);
    }

}


#endif
