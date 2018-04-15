#include "../include/test_config.h"
#include <yato/config/json/json_config.h>

TEST(Yato_Config, json_object)
{
    const auto conf = yato::conf::json_builder().parse(R"JSON(
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
    const auto conf = yato::conf::json_builder().parse(R"JSON(
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
    const auto conf = yato::conf::json_builder().parse(R"JSON(
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
    const auto conf = yato::conf::json_builder().parse(json);
    TestConfig_Example(conf);
}

