/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "../include/test_config.h"
#include <yato/config/yaml/yaml.h>

TEST(Yato_Config, yaml_object)
{
    const auto conf = yato::conf::yaml_builder().parse(R"YAML(
    {
        int: 42, # comments
        message: "somestr",
        flt : 7.0,
        "flag1" : false,
        flag2 : true,
    }
    )YAML");
    TestConfig_PlainObject(conf);
}

TEST(Yato_Config, yaml_object2)
{
    const auto conf = yato::conf::yaml_builder().parse(R"YAML(
        {
            "int": 42,
            "str": "test",
            "subobj" : {
                "val": 7.0
            }
        }
    )YAML");
    TestConfig_Object(conf);
}

TEST(Yato_Config, yaml_array)
{
    const auto conf = yato::conf::yaml_builder().parse(R"YAML(
        - 10
        - 20
        - 30
        - true
        - 4 
        - { "arr": [] }
    )YAML");
    TestConfig_Array(conf);
}


TEST(Yato_Config, yaml_example)
{
    const char* yaml = R"YAML(
        answer: 42
        comment: everything
        precision: 0.01

        manual_mode: true

        fruits: [apple, banana, kiwi]

        location: {
            "x" : 174,
            "y" : 34,
        }
    )YAML";
    const auto conf = yato::conf::yaml_builder().parse(yaml);
    TestConfig_Example(conf);
}

TEST(Yato_Config, yaml_conversion)
{
    const char* yaml = R"YAML(
        {
            "enum1" : 7,
            "enum2" : 14,
            "vec" : [20, 98, -7]
        }
    )YAML";
    const auto conf = yato::conf::yaml_builder().parse(yaml);
    TestConfig_Conversion(conf);
}


