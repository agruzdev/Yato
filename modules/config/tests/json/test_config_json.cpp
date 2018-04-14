#include "../include/test_config.h"
#include <yato/config/json/json_config.h>


TEST(Yato_Config, json_object)
{
    yato::conf::json_factory factory{};
    const yato::conf::config_ptr conf = factory.create(R"JSON(
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
    yato::conf::json_factory factory{};
    const yato::conf::config_ptr conf = factory.create(R"JSON(
        [10, 20, 30, true, 4, {
            "arr": []
        }]
    )JSON");
    TestConfig_Array(conf);
}

