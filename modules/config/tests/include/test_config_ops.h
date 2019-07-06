/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_TEST_CONFIG_OPS_COMMON_H_
#define _YATO_CONFIG_TEST_CONFIG_OPS_COMMON_H_

#include "gtest/gtest.h"

#include <cstdint>

#include <yato/config/config_operations.h>

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
void TestConfig_ObjJoin(const yato::conf::config & conf1, const yato::conf::config & conf2)
{
    ASSERT_TRUE(conf1.is_object());
    ASSERT_TRUE(conf2.is_object());

    const auto join1 = yato::conf::join(conf1, conf2, yato::conf::priority::left);
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

    const auto join2 = yato::conf::join(conf1, conf2, yato::conf::priority::right);
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

    const auto join3 = yato::conf::join(conf1, conf1);
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

/**
 *  JSON 1
 *  {
 *      "int": 42,
 *      "float" : 7.0f,
 *      "flag" : false
 *  }
 */
inline
void TestConfig_ObjFilter(const yato::conf::config & conf)
{
    ASSERT_TRUE(conf.is_object());
    ASSERT_TRUE(conf.is_object());

    const auto filt1 = yato::conf::filter(conf, {"int", "flag"}, yato::conf::filter_mode::whitelist);
    EXPECT_TRUE(filt1.is_object());
    EXPECT_EQ(42, filt1.value<int>("int").get());
    EXPECT_EQ(false, filt1.value<bool>("flag").get());

    const auto keys1 = filt1.keys();
    EXPECT_EQ(2u, keys1.size());
    EXPECT_TRUE(std::find(keys1.cbegin(), keys1.cend(), "int") != keys1.cend());
    EXPECT_TRUE(std::find(keys1.cbegin(), keys1.cend(), "flag") != keys1.cend());

    const auto filt2 = yato::conf::filter(conf, {}, yato::conf::filter_mode::whitelist);
    EXPECT_TRUE(filt2.is_object());
    EXPECT_EQ(0u, filt2.size());

    const auto filt3 = yato::conf::filter(conf, { "flag" }, yato::conf::filter_mode::blacklist);
    EXPECT_TRUE(filt3.is_object());
    EXPECT_EQ(2u, filt3.size());

    const auto keys3 = filt3.keys();
    EXPECT_TRUE(std::find(keys3.cbegin(), keys3.cend(), "int") != keys3.cend());
    EXPECT_TRUE(std::find(keys3.cbegin(), keys3.cend(), "float") != keys3.cend());
    
    EXPECT_EQ(42, filt3.value<int>("int").get());
    EXPECT_FLOAT_EQ(7.0f, filt3.value<float>("float").get());

    const auto filt4 = yato::conf::filter(conf, {}, yato::conf::filter_mode::blacklist);
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


#if 0
/**
 *  JSON 1
 *  [10, 20, 30]
 *  
 *  JSON 1
 *  [400, 500]
 */
inline
void TestConfig_ArrayCat(const yato::conf::config & conf1, const yato::conf::config & conf2)
{
    ASSERT_TRUE(conf1.is_array());
    ASSERT_TRUE(conf2.is_array());

    const auto cat1 = yato::conf::array_cat(conf1, conf2);
    EXPECT_TRUE(cat1.is_array());
    EXPECT_EQ(5u, cat1.size());
    EXPECT_EQ(10,  cat1.value<int>(0).get());
    EXPECT_EQ(20,  cat1.value<int>(1).get());
    EXPECT_EQ(30,  cat1.value<int>(2).get());
    EXPECT_EQ(400, cat1.value<int>(3).get());
    EXPECT_EQ(500, cat1.value<int>(4).get());

    const auto cat2 = yato::conf::array_cat(conf2, conf1);
    EXPECT_TRUE(cat2.is_array());
    EXPECT_EQ(5u, cat2.size());
    EXPECT_EQ(400, cat2.value<int>(0).get());
    EXPECT_EQ(500, cat2.value<int>(1).get());
    EXPECT_EQ(10,  cat2.value<int>(2).get());
    EXPECT_EQ(20,  cat2.value<int>(3).get());
    EXPECT_EQ(30,  cat2.value<int>(4).get());
}
#endif




#endif // _YATO_CONFIG_TEST_CONFIG_OPS_COMMON_H_
