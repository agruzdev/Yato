/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_TEST_CONFIG_OPS_COMMON_H_
#define _YATO_CONFIG_TEST_CONFIG_OPS_COMMON_H_

#include "gtest/gtest.h"

#include <cstdint>

#include "yato/config/config.h"

/**
 *  JSON 1
 *  {
 *      "int": 42,
 *      "float" : 7.0f,
 *      "flag" : false
 *  }
 *  
 *  JSON 2
 *  {
 *      "int": 43,
 *      "flag" : true,
 *      "string" : "text",
 *      "flag2" : true
 *  }
 */
inline
void TestConfig_ObjJoin_Impl(const yato::conf::config & conf1, const yato::conf::config & conf2)
{
    ASSERT_TRUE(conf1.is_object());
    ASSERT_TRUE(conf2.is_object());

    const auto join1 = yato::config::merge(conf1, conf2, yato::conf::priority::left);
    EXPECT_TRUE(join1.is_object());
    EXPECT_EQ(42, join1.value<int>("int").get());
    EXPECT_FLOAT_EQ(7.0f, join1.value<float>("float").get());
    EXPECT_EQ(false, join1.value<bool>("flag").get());
    EXPECT_EQ("text", join1.value<std::string>("string").get());
    EXPECT_EQ(true, join1.value<bool>("flag2").get());

    const auto keys1 = join1.keys();
    EXPECT_EQ(5u, keys1.size());
    EXPECT_TRUE(std::find(keys1.cbegin(), keys1.cend(), "int") != keys1.cend());
    EXPECT_TRUE(std::find(keys1.cbegin(), keys1.cend(), "float") != keys1.cend());
    EXPECT_TRUE(std::find(keys1.cbegin(), keys1.cend(), "flag") != keys1.cend());
    EXPECT_TRUE(std::find(keys1.cbegin(), keys1.cend(), "string") != keys1.cend());
    EXPECT_TRUE(std::find(keys1.cbegin(), keys1.cend(), "flag2") != keys1.cend());

    const auto join2 = yato::config::merge(conf1, conf2, yato::conf::priority::right);
    EXPECT_TRUE(join2.is_object());
    EXPECT_EQ(43, join2.value<int>("int").get());
    EXPECT_FLOAT_EQ(7.0f, join2.value<float>("float").get());
    EXPECT_EQ(true, join2.value<bool>("flag").get());
    EXPECT_EQ("text", join2.value<std::string>("string").get());
    EXPECT_EQ(true, join2.value<bool>("flag2").get());

    const auto keys2 = join2.keys();
    EXPECT_EQ(5u, keys2.size());
    EXPECT_TRUE(std::find(keys2.cbegin(), keys2.cend(), "int") != keys2.cend());
    EXPECT_TRUE(std::find(keys2.cbegin(), keys2.cend(), "float") != keys2.cend());
    EXPECT_TRUE(std::find(keys2.cbegin(), keys2.cend(), "flag") != keys2.cend());
    EXPECT_TRUE(std::find(keys2.cbegin(), keys2.cend(), "string") != keys2.cend());
    EXPECT_TRUE(std::find(keys2.cbegin(), keys2.cend(), "flag2") != keys2.cend());

    const auto join3 = yato::config::merge(conf1, conf1);
    EXPECT_TRUE(join3.is_object());
    EXPECT_EQ(42, join3.value<int>("int").get());
    EXPECT_FLOAT_EQ(7.0f, join3.value<float>("float").get());
    EXPECT_EQ(false, join3.value<bool>("flag").get());

    const auto keys3 = join3.keys();
    EXPECT_EQ(3u, keys3.size());
    EXPECT_TRUE(std::find(keys3.cbegin(), keys3.cend(), "int") != keys3.cend());
    EXPECT_TRUE(std::find(keys3.cbegin(), keys3.cend(), "float") != keys3.cend());
    EXPECT_TRUE(std::find(keys3.cbegin(), keys3.cend(), "flag") != keys3.cend());
}

inline
void TestConfig_ObjJoin(const yato::conf::config& conf1, const yato::conf::config& conf2)
{
    TestConfig_ObjJoin_Impl(conf1, conf2);
    TestConfig_ObjJoin_Impl(conf1.clone(), conf2);
    TestConfig_ObjJoin_Impl(conf1, conf2.clone());
    TestConfig_ObjJoin_Impl(conf1.clone(), conf2.clone());
}

/**
 *  JSON 1
 *  {
 *      "int": 42,
 *      "float" : 7.0f,
 *      "flag" : false
 *  }
 */
inline
void TestConfig_ObjFilter_Impl(const yato::conf::config & conf)
{
    ASSERT_TRUE(conf.is_object());
    ASSERT_TRUE(conf.is_object());

    const auto filt1 = conf.with_whitelist({"int", "flag"});
    EXPECT_TRUE(filt1.is_object());
    EXPECT_EQ(42, filt1.value<int>("int").get());
    EXPECT_EQ(false, filt1.value<bool>("flag").get());

    const auto keys1 = filt1.keys();
    EXPECT_EQ(2u, keys1.size());
    EXPECT_TRUE(std::find(keys1.cbegin(), keys1.cend(), "int") != keys1.cend());
    EXPECT_TRUE(std::find(keys1.cbegin(), keys1.cend(), "flag") != keys1.cend());

    const auto filt2 = conf.with_whitelist({});
    EXPECT_TRUE(filt2.is_object());
    EXPECT_EQ(0u, filt2.size());

    const auto filt3 = conf.with_blacklist({ "flag" });
    EXPECT_TRUE(filt3.is_object());
    EXPECT_EQ(2u, filt3.size());

    const auto keys3 = filt3.keys();
    EXPECT_TRUE(std::find(keys3.cbegin(), keys3.cend(), "int") != keys3.cend());
    EXPECT_TRUE(std::find(keys3.cbegin(), keys3.cend(), "float") != keys3.cend());
    
    EXPECT_EQ(42, filt3.value<int>("int").get());
    EXPECT_FLOAT_EQ(7.0f, filt3.value<float>("float").get());

    const auto filt4 = conf.with_blacklist({});
    EXPECT_TRUE(filt4.is_object());
    EXPECT_EQ(3u, filt4.size());

    const auto keys4 = filt4.keys();
    EXPECT_TRUE(std::find(keys4.cbegin(), keys4.cend(), "int") != keys4.cend());
    EXPECT_TRUE(std::find(keys4.cbegin(), keys4.cend(), "float") != keys4.cend());
    EXPECT_TRUE(std::find(keys4.cbegin(), keys4.cend(), "flag") != keys4.cend());

    EXPECT_EQ(42, filt4.value<int>("int").get());
    EXPECT_EQ(false, filt4.value<bool>("flag").get());
    EXPECT_FLOAT_EQ(7.0f, filt4.value<float>("float").get());
}

inline
void TestConfig_ObjFilter(const yato::conf::config& conf)
{
    TestConfig_ObjFilter_Impl(conf);
    TestConfig_ObjFilter_Impl(conf.clone());
}



/**
 *  JSON 1
 *  {
 *      "int": 42,
 *      "nested": {
 *          "value1": 80
 *          "value3": 30
 *      }
 *  }
 *  
 *  JSON 2
 *  {
 *      "flag" : true,
 *      "nested": {
 *          "value2": 101
 *      }
 *  }
 */
inline
void TestConfig_ObjJoin2_Impl(const yato::conf::config & conf1, const yato::conf::config & conf2)
{
    ASSERT_TRUE(conf1.is_object());
    ASSERT_TRUE(conf2.is_object());

    const auto c1 = conf1.with_value("test", 10);
    EXPECT_EQ(42, c1.value<int>("int").get_or(-1));
    EXPECT_EQ(10, c1.value<int>("test").get_or(-1));
    EXPECT_EQ(80, c1.value<int>("nested.value1").get_or(-1));

    const auto c2 = conf1.with_value("nested.test", 10);
    EXPECT_EQ(42, c2.value<int>("int").get_or(-1));
    EXPECT_EQ(10, c2.value<int>("nested.test").get_or(-1));
    EXPECT_EQ(80, c2.value<int>("nested.value1").get_or(-1));

    const auto c3 = conf1.with_value("sub1.sub2.test", 10);
    EXPECT_EQ(42, c3.value<int>("int").get_or(-1));
    EXPECT_EQ(10, c3.value<int>("sub1.sub2.test").get_or(-1));
    EXPECT_EQ(80, c3.value<int>("nested.value1").get_or(-1));

    const auto c4 = conf1.without_path("int");
    EXPECT_EQ(-1, c4.value<int>("int").get_or(-1));
    EXPECT_EQ(80, c4.value<int>("nested.value1").get_or(-1));

    const auto c5 = conf1.without_path("nested.value1");
    EXPECT_EQ(42, c5.value<int>("int").get_or(-1));
    EXPECT_EQ(-1, c5.value<int>("nested.value1").get_or(-1));
    EXPECT_EQ(30, c5.value<int>("nested.value3").get_or(-1));

    const auto c6 = conf1.with_only_path("nested.value1");
    EXPECT_EQ(-1, c6.value<int>("int").get_or(-1));
    EXPECT_EQ(80, c6.value<int>("nested.value1").get_or(-1));
    EXPECT_EQ(-1, c6.value<int>("nested.value3").get_or(-1));

    const auto c7 = conf1.with_only_path("nested");
    EXPECT_EQ(-1, c7.value<int>("int").get_or(-1));
    EXPECT_EQ(80, c7.value<int>("nested.value1").get_or(-1));
    EXPECT_EQ(30, c7.value<int>("nested.value3").get_or(-1));

    const auto c8 = conf1.merged_with(conf2);
    EXPECT_TRUE(c8.is_object());
    EXPECT_EQ(42, c8.value<int>("int").get_or(-1));
    EXPECT_EQ(true, c8.value<bool>("flag").get_or(false));
    EXPECT_EQ(80, c8.value<int>("nested.value1").get_or(-1));
    EXPECT_EQ(101, c8.value<int>("nested.value2").get_or(-1));
    EXPECT_EQ(30, c8.value<int>("nested.value3").get_or(-1));

    const auto c9 = conf1.with_value("int", 10);
    EXPECT_EQ(10, c9.value<int>("int").get_or(-1));
    EXPECT_EQ(80, c9.value<int>("nested.value1").get_or(-1));
}

inline
void TestConfig_ObjJoin2(const yato::conf::config& conf1, const yato::conf::config& conf2)
{
    TestConfig_ObjJoin2_Impl(conf1, conf2);
    TestConfig_ObjJoin2_Impl(conf1.clone(), conf2);
    TestConfig_ObjJoin2_Impl(conf1, conf2.clone());
    TestConfig_ObjJoin2_Impl(conf1.clone(), conf2.clone());
}


#endif // _YATO_CONFIG_TEST_CONFIG_OPS_COMMON_H_
