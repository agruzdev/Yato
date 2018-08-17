/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016-2018 Alexey Gruzdev
 */

#ifndef _YATO_TEST_ACTORS_COMMON_H_
#define _YATO_TEST_ACTORS_COMMON_H_

#include <yato/config/manual/manual_config.h>

inline
yato::config actors_debug_config()
{
    return yato::conf::manual_builder::object()
        .put("log_level", "debug")
        .create();
}

inline
yato::config actors_verbose_config()
{
    return yato::conf::manual_builder::object()
        .put("log_level", "debug")
        .create();
}

inline
yato::config actors_all_contexts_config(const std::string & log_level = "debug")
{
    return yato::conf::manual_builder::object()
        .put("log_level", log_level)
        .put("execution_contexts", yato::conf::manual_builder::array()
            .add(yato::conf::manual_builder::object()
                .put("name", "dynamic")
                .put("type", "thread_pool")
                .put("threads_num", 4)
                .put("throughput", 5)
                .create()
            )
            .add(yato::conf::manual_builder::object()
                .put("name", "pinned")
                .put("type", "pinned")
                .put("threads_limit", 8)
                .create()
            )
            .create()
        )
        .create();
}

inline
yato::config actors_pinned_config(const std::string & log_level = "debug", uint32_t threads_lim = 16)
{
    return yato::conf::manual_builder::object()
        .put("log_level", log_level)
        .put("execution_contexts", yato::conf::manual_builder::array()
            .add(yato::conf::manual_builder::object()
                .put("name", "pinned")
                .put("type", "pinned")
                .put("threads_limit", threads_lim)
                .create()
            )
            .create()
        )
        .put("default_executor" , "pinned")
        .create();
}

#endif // _YATO_TEST_ACTORS_COMMON_H_
