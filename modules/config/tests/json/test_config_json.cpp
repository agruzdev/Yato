#include "../include/test_config.h"
#include <yato/config/json/json_object.h>


TEST(Yato_Config, json_object)
{
    TestConfig_Object(std::make_unique<yato::conf::json_factory>());
}

TEST(Yato_Config, json_array)
{
    TestConfig_Array(std::make_unique<yato::conf::json_factory>());
}

