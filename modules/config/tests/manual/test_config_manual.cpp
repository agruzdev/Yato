#include "../include/test_config.h"
#include <yato/config/manual/manual_config.h>

TEST(Yato_Config, manual_object)
{
    const auto conf = yato::conf::manual_builder::object()
        .put("int", 42)
        .put("message", "somestr")
        .put("flt", 7.0f)
        .put("flag1", false)
        .put("flag2", true)
        .create();

    ASSERT_NE(nullptr, conf);

    TestConfig_PlainObject(conf);
}

TEST(Yato_Config, manual_object2)
{
    const auto conf = yato::conf::manual_builder::object()
        .put("int", 42)
        .put("str", "test")
        .put("subobj", yato::conf::manual_builder::object()
            .put("val", 7.0f)
            .create())
        .create();

    ASSERT_NE(nullptr, conf);

    TestConfig_Object(conf);
}

TEST(Yato_Config, manual_array)
{
    const auto conf = yato::conf::manual_builder::array()
        .add(10)
        .add(20)
        .add(30)
        .add(true)
        .add(4)
        .add(yato::conf::manual_builder::object()
            .put("arr", yato::conf::manual_builder::array()
                .create())
            .create())
        .create();

    ASSERT_NE(nullptr, conf);

    TestConfig_Array(conf);
}
