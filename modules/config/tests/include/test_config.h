#ifndef _YATO_CONFIG_TEST_CONFIG_COMMON_H_
#define _YATO_CONFIG_TEST_CONFIG_COMMON_H_

#include "gtest/gtest.h"

#include <cstdint>

#include <yato/config/config.h>

inline
void TestConfig_PlainObject(const yato::conf::config_ptr & conf)
{
    ASSERT_NE(nullptr, conf);
    EXPECT_TRUE(conf->is_object());

    const auto i = conf->value<int32_t>("int");
    EXPECT_EQ(42, i.get_or(0));

    const auto str = conf->value<std::string>("message");
    EXPECT_EQ("somestr", str.get_or(""));

    const auto f1 = conf->value<bool>("flag1");
    EXPECT_EQ(false, f1.get_or(true));

    const auto f2 = conf->value<bool>("flag2");
    EXPECT_EQ(true, f2.get_or(false));

    const auto v = conf->value<float>("flt").get_or(-1.0f);
    EXPECT_FLOAT_EQ(7.0f, v);
}

inline
void TestConfig_Object(const yato::conf::config_ptr & conf)
{
    ASSERT_NE(nullptr, conf);
    EXPECT_TRUE(conf->is_object());

    const auto i = conf->value<int32_t>("int");
    EXPECT_EQ(42, i.get_or(0));

    const auto str = conf->value<std::string>("str");
    EXPECT_EQ("test", str.get_or(""));

    const yato::conf::config_ptr c2 = conf->config("subobj");
    ASSERT_NE(nullptr, c2);
    EXPECT_TRUE(c2->is_object());

    const auto v = c2->value<float>("val").get_or(-1.0f);
    EXPECT_FLOAT_EQ(7.0f, v);
}

inline
void TestConfig_Array(const yato::conf::config_ptr & conf)
{
    ASSERT_NE(nullptr, conf);
    EXPECT_TRUE(conf->is_array());
    EXPECT_EQ(6U, conf->size());

    EXPECT_EQ(10, conf->value<int>(0).get_or(-1));
    EXPECT_EQ(20, conf->value<short>(1).get_or(-1));
    EXPECT_EQ(30U, conf->value<unsigned long long>(2).get_or(-1));
    EXPECT_EQ(true, conf->value<bool>(3).get_or(false));

    const auto c2 = conf->config(5);
    ASSERT_NE(nullptr, c2);

    const auto arr = c2->array("arr");
    ASSERT_NE(nullptr, arr);

    EXPECT_TRUE(arr->is_array());
    EXPECT_EQ(0U, arr->size());
}

#endif // _YATO_CONFIG_TEST_CONFIG_COMMON_H_
