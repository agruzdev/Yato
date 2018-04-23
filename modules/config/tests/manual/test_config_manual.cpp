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
    TestConfig_Array(conf);
}

TEST(Yato_Config, manual_example)
{
    const auto conf = yato::conf::manual_builder::object()
        .put("answer", 42)
        .put("comment", "everything")
        .put("precision", 0.01f)
        .put("manual_mode", true)
        .put("fruits", yato::conf::manual_builder::array()
            .add("apple")
            .add("banana")
            .add("kiwi")
            .create())
        .put("location", yato::conf::manual_builder::object()
            .put("x", 174)
            .put("y", 34)
            .create())
        .create();
    TestConfig_Example(conf);
}

TEST(Yato_Config, manual_conversion)
{
    const auto conf = yato::conf::manual_builder::object()
        .put("enum1", static_cast<int32_t>(TestEnum::eVal1))
        .put("enum2", static_cast<int32_t>(TestEnum::eVal2))
        .put("vec", yato::conf::manual_builder::array()
            .add(20)
            .add(98)
            .add(-7)
            .create()
        )
        .create();
    TestConfig_Conversion(conf);
}

