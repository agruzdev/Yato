/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include <sstream>
#include "../include/test_config.h"
#include <yato/config/json/json.h>

TEST(Yato_Config, json_object)
{
    const auto conf = yato::conf::json::read(R"JSON(
        {
            "int": 42,
            "message": "somestr",
            "flt" : 7.0,
            "flag1" : false,
            "flag2" : true
        }
    )JSON");
    TestConfig_PlainObject(conf);
}

TEST(Yato_Config, json_object2)
{
    const auto conf = yato::conf::json::read(R"JSON(
        {
            "int": 42,
            "str": "test",
            "subobj" : {
                "val": 7.0
            }
        }
    )JSON");
    TestConfig_Object(conf);
}

TEST(Yato_Config, json_array)
{
    const auto conf = yato::conf::json::read(R"JSON(
        [10, 20, 30, true, 4, {
            "arr": []
        }]
    )JSON");
    TestConfig_Array(conf);
}

TEST(Yato_Config, json_example)
{
    const char* json = R"JSON(
        {
            "answer": 42,
            "comment": "everything",
            "precision" : 0.01,
    
            "manual_mode" : true,

            "fruits" : [
                "apple", "banana", "kiwi"
            ],

            "location" : {
                "x" : 174,
                "y" : 34
            }
        }
    )JSON";
    const auto conf = yato::conf::json::read(json);
    TestConfig_Example(conf);
}

TEST(Yato_Config, json_conversion)
{
    const char* json = R"JSON(
        {
            "enum1" : 7,
            "enum2" : 14,
            "vec" : [20, 98, -7]
        }
    )JSON";
    const auto conf = yato::conf::json::read(json);
    TestConfig_Conversion(conf);
}


TEST(Yato_Config, json_write)
{
    const auto conf = GetExampleConfig();
    const std::string str = yato::conf::json::write(conf, 2);
    TestConfig_Example(yato::conf::json::read(str));
}

TEST(Yato_Config, json_write_stream)
{
    const auto conf = GetExampleConfig();
    std::stringstream ss;
    yato::conf::json::write(conf, ss);
    TestConfig_Example(yato::conf::json::read(ss));
}
