/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_TEST_ACTORS_COMMON_H_
#define _YATO_TEST_ACTORS_COMMON_H_

#include <yato/config/config_builder.h>

inline
yato::config actors_debug_config()
{
    return yato::config_builder::object()
        .put("log_level", "debug")
        .create();
}

inline
yato::config actors_verbose_config()
{
    return yato::config_builder::object()
        .put("log_level", "debug")
        .create();
}

inline
yato::config actors_all_contexts_config(const std::string & log_level = "debug")
{
    return yato::config_builder::object()
        .put("log_level", log_level)
        .put("execution_contexts", yato::config_builder::array()
            .add(yato::config_builder::object()
                .put("name", "dynamic")
                .put("type", "thread_pool")
                .put("threads_num", 4)
                .put("throughput", 5)
                .create()
            )
            .add(yato::config_builder::object()
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
    return yato::config_builder::object()
        .put("log_level", log_level)
        .put("execution_contexts", yato::config_builder::array()
            .add(yato::config_builder::object()
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
