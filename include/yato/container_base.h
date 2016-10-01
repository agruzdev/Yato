/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_CONTAINER_BASE_H_
#define _YATO_CONTAINER_BASE_H_

#include <array>
#include "type_traits.h"

namespace yato
{
    /**
     * Storage for containers extents
     */
    template <size_t DimensionsNum, typename SizeType = size_t>
    class dimensions
    {
    private:
        using my_type = dimensions<DimensionsNum, SizeType>;
        using container_type = std::array<SizeType, DimensionsNum>;
        template <typename T>
        using arg_to_size = cvt_type<T, SizeType>;
        //---------------------------------------------------

    public:
        using size_type = SizeType;
        static YATO_CONSTEXPR_VAR size_t dimensions_num = DimensionsNum;
        using iterator = typename container_type::iterator;
        using const_iterator = typename container_type::const_iterator;
        using hyper_dimensions = dimensions<DimensionsNum - 1, SizeType>;
        //---------------------------------------------------

    private:
        container_type m_extents;
        //---------------------------------------------------

    public:
        YATO_CONSTEXPR_FUNC
        dimensions()
            : m_extents()
        { }

        template <typename... Args>
        YATO_CONSTEXPR_FUNC explicit 
        dimensions(const SizeType & extent1, const Args &... extents) YATO_NOEXCEPT_KEYWORD
            : m_extents({ extent1, extents... })
        {
            // ToDo (a.gruzdev): Make SFINAE friendly if necessary
            static_assert(sizeof...(extents) + 1 == dimensions_num, "yato::dimensions[dimensions]: Invalid dimensions number");
        }

        YATO_CONSTEXPR_FUNC
        dimensions(const my_type &) = default;
#ifndef YATO_MSVC_2013
        dimensions(my_type &&) = default;
#endif
        ~dimensions()
        { }

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
        const size_t size() const
        {
            return m_extents.size();
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
    };


    /**
     * Zero-dimensional extents
     */
    template <typename SizeType>
    class dimensions<0, SizeType>
    {
    public:
        using size_type = SizeType;
        static YATO_CONSTEXPR_VAR size_t dimensions_num = 0;

        YATO_CONSTEXPR_FUNC
        dimensions()
        { }

        YATO_CONSTEXPR_FUNC
        const size_t size() const
        {
            return 0;
        }
    };

    /**
     * Helper function for creating dimensions
     */
    template <typename SizeType = size_t, typename... Extents>
    YATO_CONSTEXPR_FUNC
    dimensions<sizeof...(Extents), SizeType> dims(const Extents &... extents)
    {
        return dimensions<sizeof...(Extents), SizeType>(narrow_cast<SizeType>(extents)...);
    }

    //----------------------------------------------------------------------------------


    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t length(_ContainerType && container)
    {
        YATO_REQUIRES(container.dimensions() == 1);
        return container.size(0);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t height_2d(_ContainerType && container)
    {
        YATO_REQUIRES(container.dimensions() == 2);
        return container.size(0);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t width_2d(_ContainerType && container)
    {
        YATO_REQUIRES(container.dimensions() == 2);
        return container.size(1);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t depth_3d(_ContainerType && container)
    {
        YATO_REQUIRES(container.dimensions() == 3);
        return container.size(0);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t height_3d(_ContainerType && container)
    {
        YATO_REQUIRES(container.dimensions() == 3);
        return container.size(1);
    }

    template <typename _ContainerType>
    YATO_CONSTEXPR_FUNC
    size_t width_3d(_ContainerType && container)
    {
        YATO_REQUIRES(container.dimensions() == 3);
        return container.size(2);
    }

}


#endif
