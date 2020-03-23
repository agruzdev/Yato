/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "../include/test_config_ops.h"
#include <yato/config/manual/manual.h>


TEST(Yato_ConfigOps, manual_config_ops_join)
{
    const auto conf1 = yato::conf::manual_builder::object()
        .put("int", 42)
        .put("float", 7.0f)
        .put("flag", false)
        .create();

    const auto conf2 = yato::conf::manual_builder::object()
        .put("int", 43)
        .put("flag", true)
        .put("string", "text")
        .put("flag2", true)
        .create();

    TestConfig_ObjJoin(conf1, conf2);
}

TEST(Yato_ConfigOps, manual_config_ops_filter)
{
    const auto conf1 = yato::conf::manual_builder::object()
        .put("int", 42)
        .put("float", 7.0f)
        .put("flag", false)
        .create();

    TestConfig_ObjFilter(conf1);
}

TEST(Yato_ConfigOps, manual_config_ops_join2)
{
    const auto conf1 = yato::conf::manual_builder::object()
        .put("int", 42)
        .put("nested", yato::conf::manual_builder::object()
            .put("value1", 80)
            .put("value3", 30)
            .create())
        .create();

    const auto conf2 = yato::conf::manual_builder::object()
        .put("flag", true)
        .put("nested", yato::conf::manual_builder::object()
            .put("value2", 101)
            .create())
        .create();

    TestConfig_ObjJoin2(conf1, conf2);
}
