/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "../include/test_config.h"
#include "yato/config/config_builder.h"

TEST(Yato_Config, manual_object)
{
    const auto conf = yato::config_builder::object()
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
    const auto conf = yato::config_builder::object()
        .put("int", 42)
        .put("str", "test")
        .put("subobj", yato::config_builder::object()
            .put("val", 7.0f)
            .create())
        .create();
    TestConfig_Object(conf);
}

TEST(Yato_Config, manual_array)
{
    const auto conf = yato::config_builder::array()
        .add(10)
        .add(20)
        .add(30)
        .add(true)
        .add(4)
        .add(yato::config_builder::object()
            .put("arr", yato::config_builder::array()
                .create())
            .create())
        .create();
    TestConfig_Array(conf);
}

TEST(Yato_Config, manual_example)
{
    const auto conf = yato::config_builder::object()
        .put("answer", 42)
        .put("comment", "everything")
        .put("precision", 0.01f)
        .put("manual_mode", true)
        .put("fruits", yato::config_builder::array()
            .add("apple")
            .add("banana")
            .add("kiwi")
            .create())
        .put("location", yato::config_builder::object()
            .put("x", 174)
            .put("y", 34)
            .create())
        .create();
    TestConfig_Example(conf);
}

TEST(Yato_Config, manual_conversion)
{
    const auto conf = yato::config_builder::object()
        .put("enum1", static_cast<int32_t>(TestEnum::eVal1))
        .put("enum2", static_cast<int32_t>(TestEnum::eVal2))
        .put("vec", yato::config_builder::array()
            .add(20)
            .add(98)
            .add(-7)
            .create()
        )
        .create();
    TestConfig_Conversion(conf);
}


TEST(Yato_Config, manual_conversion_root)
{
    const auto conf = yato::config_builder::object()
        .put("x", 99.0)
        .put("y", 88.0)
        .create();
    TestConfig_ConversionRoot(conf);
}


TEST(Yato_Config, manual_copy)
{
    auto builder1 = yato::config_builder::object()
        .put("int", 42)
        .put("message", "somestr")
        .put("flt", 7.0f)
        .put("flag1", false)
        .put("flag2", true);

    auto builder2 = builder1;

    auto builder3 = builder1;

    TestConfig_PlainObject(builder2.create());

    builder3.put("int", -42);
    builder3.put("int2", 12);

    TestConfig_PlainObject(builder1.create());
}


