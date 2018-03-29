#ifndef _YATO_CONFIG_TEST_CONFIG_COMMON_H_
#define _YATO_CONFIG_TEST_CONFIG_COMMON_H_

#include "gtest/gtest.h"

#include <cstdint>

#include <yato/config/config.h>

static
void TestConfig_Object(const std::unique_ptr<yato::conf::config_factory> & factory)
{
    const yato::conf::config_ptr c1 = factory->create(R"JSON(
        {
            "int": 42,
            "str": "test",
            "subobj" : {
                "val": 7.0
            }
        }
    )JSON");
    ASSERT_NE(nullptr, c1);
    EXPECT_TRUE(c1->is_object());

    const auto i = c1->value<int32_t>("int");
    EXPECT_EQ(42, i.get_or(0));

    const auto str = c1->value<std::string>("str");
    EXPECT_EQ("test", str.get_or(""));

    const yato::conf::config_ptr c2 = c1->config("subobj");
    ASSERT_NE(nullptr, c2);
    EXPECT_TRUE(c1->is_object());

    const auto v = c2->value<float>("val").get_or(-1.0f);
    EXPECT_FLOAT_EQ(7.0f, v);
}

static
void TestConfig_Array(const std::unique_ptr<yato::conf::config_factory> & factory)
{
    const yato::conf::config_ptr c1 = factory->create(R"JSON(
        [10, 20, 30, true, 4, {
            "arr": []
        }]
    )JSON");
    ASSERT_NE(nullptr, c1);
    EXPECT_TRUE(c1->is_array());
    EXPECT_EQ(6, c1->size());

    EXPECT_EQ(10, c1->value<int>(0).get_or(-1));
    EXPECT_EQ(20, c1->value<short>(1).get_or(-1));
    EXPECT_EQ(30, c1->value<unsigned long long>(2).get_or(-1));
    EXPECT_EQ(true, c1->value<bool>(3).get_or(false));

    const auto c2 = c1->config(5);
    ASSERT_NE(nullptr, c2);

    const auto arr = c2->config("arr");
    ASSERT_NE(nullptr, arr);

    EXPECT_TRUE(arr->is_array());
    EXPECT_EQ(0, arr->size());
}

#endif // _YATO_CONFIG_TEST_CONFIG_COMMON_H_
