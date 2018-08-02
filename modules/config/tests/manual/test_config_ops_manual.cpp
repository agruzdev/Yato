#include "../include/test_config_ops.h"
#include <yato/config/manual/manual_config.h>


TEST(Yato_ConfigOps, manual_config_ops)
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
    TestConfig_Union(conf1, conf2);
    TestConfig_Intersection(conf1, conf2);
}

TEST(Yato_ConfigOps, manual_array_ops)
{
    const auto conf1 = yato::conf::manual_builder::array()
        .add(10)
        .add(20)
        .add(30)
        .create();
    const auto conf2 = yato::conf::manual_builder::array()
        .add(400)
        .add(500)
        .create();
    TestConfig_ArrayCat(conf1, conf2);
}
