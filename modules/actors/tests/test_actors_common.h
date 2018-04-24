/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_TEST_ACTORS_COMMON_H_
#define _YATO_TEST_ACTORS_COMMON_H_

#include <yato/config/manual/manual_config.h>

inline yato::config actors_debug_config()
{
    return yato::conf::manual_builder::object()
        .put("log_level", "debug")
        .create();
}

inline yato::config actors_verbose_config()
{
    return yato::conf::manual_builder::object()
        .put("log_level", "debug")
        .create();
}

#endif // _YATO_TEST_ACTORS_COMMON_H_
