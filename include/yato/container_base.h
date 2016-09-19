/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_CONTAINER_BASE_H_
#define _YATO_CONTAINER_BASE_H_

#include "prerequisites.h"

namespace yato
{

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
