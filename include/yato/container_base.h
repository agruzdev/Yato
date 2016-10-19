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
    class dimensionality
    {
    private:
        using my_type = dimensionality<DimensionsNum, SizeType>;
        using container_type = std::array<SizeType, DimensionsNum>;
        template <typename T>
        using arg_to_size = cvt_type<T, SizeType>;
        //---------------------------------------------------

    public:
        using size_type = SizeType;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = DimensionsNum;
        using iterator = typename container_type::iterator;
        using const_iterator = typename container_type::const_iterator;
        using hyper_dimensionality = dimensionality<DimensionsNum - 1, SizeType>;
        //---------------------------------------------------

    private:
        container_type m_extents;
        //---------------------------------------------------

        YATO_CONSTEXPR_FUNC
        const SizeType total_size_impl(size_t idx) const
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
        dimensionality(const SizeType & extent1, const Args &... extents) YATO_NOEXCEPT_KEYWORD
            : m_extents({ extent1, extents... })
        {
            // ToDo (a.gruzdev): Make SFINAE friendly if necessary
            static_assert(sizeof...(extents) + 1 == dimensions_number, "yato::dimensionality[dimensionality]: Invalid dimensions number");
        }
YATO_PRAGMA_WARNING_POP

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
