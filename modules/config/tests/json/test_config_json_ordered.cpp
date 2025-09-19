/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include <sstream>
#include "../include/test_config.h"
#include <yato/config/json/json.h>

TEST(Yato_Config, json_ordered_object)
{
    const auto conf = yato::conf::json::read(R"JSON(
        {
            "int": 42,
            "message": "somestr",
            "flt" : 7.0,
            "flag1" : false,
            "flag2" : true
        }
    )JSON", yato::nolength, /*ordered=*/true);
    TestConfig_PlainObject(conf);
}

TEST(Yato_Config, json_ordered_object2)
{
    const auto conf = yato::conf::json::read(R"JSON(
        {
            "int": 42,
            "str": "test",
            "subobj" : {
                "val": 7.0
            }
        }
    )JSON", yato::nolength, /*ordered=*/true);
    TestConfig_Object(conf);
}

TEST(Yato_Config, json_ordered_array)
{
    const auto conf = yato::conf::json::read(R"JSON(
        [10, 20, 30, true, 4, {
            "arr": []
        }]
    )JSON", yato::nolength, /*ordered=*/true);
    TestConfig_Array(conf);
}

TEST(Yato_Config, json_ordered_example)
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
    const auto conf = yato::conf::json::read(json, yato::nolength, /*ordered=*/true);
    TestConfig_Example(conf.clone());
    TestConfig_Example(conf);
}

TEST(Yato_Config, json_ordered_conversion)
{
    const char* json = R"JSON(
        {
            "enum1" : 7,
            "enum2" : 14,
            "vec" : [20, 98, -7]
        }
    )JSON";
    const auto conf = yato::conf::json::read(json, yato::nolength, /*ordered=*/true);
    TestConfig_Conversion(conf);
}


TEST(Yato_Config, json_ordered_write)
{
    const auto conf = GetExampleConfig();
    const std::string str = yato::conf::json::write(conf, 2, /*ordered=*/true);
    TestConfig_Example(yato::conf::json::read(str, /*ordered=*/true));
}

TEST(Yato_Config, json_ordered_write_stream)
{
    const auto conf = GetExampleConfig();
    std::stringstream strstr;
    yato::conf::json::write(conf, strstr, /*ordered=*/true);
    TestConfig_Example(yato::conf::json::read(strstr, /*ordered=*/true));
}
