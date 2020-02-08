/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_STL_UTILITY_H_
#define _YATO_STL_UTILITY_H_

#include <sstream>

#include "prerequisites.h"

namespace yato
{
/**
 * @namespace
 * Define STL features missing in some implementations
 */
namespace stl
{
    namespace details
    {
        template <typename Ty_, typename = 
            std::enable_if_t<std::is_arithmetic<Ty_>::value>
        >
        std::string to_string(Ty_ value)
        {
            std::stringstream s;
            s << value;
            return s.str();
        }

    }

#ifdef YATO_ANDROID
    using details::to_string;
    using ::snprintf;
#else
    using std::to_string;
    using std::snprintf;
#endif

} // namespace stl

} // namespace yato


#endif //_YATO_STL_UTILITY_H_
