/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "../include/test_config_ops.h"
#include "yato/config/yaml/yaml.h"


TEST(Yato_ConfigOps, yaml_config_ops_join)
{
    const auto conf1 = yato::conf::yaml::read(R"YAML(
    {
        int: 42,
        float : 7.0,
        flag : false
    }
    )YAML");
    const auto conf2 = yato::conf::yaml::read(R"YAML(
    {
        int: 43,
        flag : true,
        string: text,
        flag2 : true,
    }
    )YAML");
    TestConfig_ObjJoin(conf1, conf2);
}

TEST(Yato_ConfigOps, yaml_config_ops_filter)
{
    const auto conf1 = yato::conf::yaml::read(R"YAML(
    {
        int: 42,
        float : 7.0,
        flag : false
    }
    )YAML");
    TestConfig_ObjFilter(conf1);
}

TEST(Yato_ConfigOps, yaml_config_ops_join2)
{
    const auto conf1 = yato::conf::yaml::read(R"YAML(
    {
        int: 42,
        nested: {
            value1 : 80,
            value3 : 30
        }
    }
    )YAML");
    const auto conf2 = yato::conf::yaml::read(R"YAML(
    {
        flag: true,
        nested: {
            value2 : 101
        },
        nested2:
    }
    )YAML");
    TestConfig_ObjJoin2(conf1, conf2);
}
