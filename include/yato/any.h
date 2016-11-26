/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ANY_H_
#define _YATO_ANY_H_

/*
 * C++17 std::any requires CopyConstructible classes
 * This is extended implementtion, supporting only movable types
 * Based on boost::any
 */

# include "details/boost_any.h"
namespace yato
{
    using boost::any;
    using boost::bad_any_cast;
    using boost::any_copy_error;
    using boost::any_cast;
    using boost::unsafe_any_cast;
}

#endif

