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
 *  JSON 1
 *  {
 *      "int": 43,
 *      "flag" : true,
 *      "string" : "text",
 *      "flag2" : true
 *  }
 */
inline
void TestConfig_Union(const yato::conf::config & conf1, const yato::conf::config & conf2)
{
    ASSERT_TRUE(conf1.is_object());
    ASSERT_TRUE(conf2.is_object());

    const auto union1 = yato::conf::config_union(conf1, conf2, yato::conf::priority::left);
    EXPECT_TRUE(union1.is_object());
    EXPECT_EQ(42, union1.value<int>("int").get());
    EXPECT_FLOAT_EQ(7.0f, union1.value<float>("float").get());
    EXPECT_EQ(false, union1.value<bool>("flag").get());
    EXPECT_EQ("text", union1.value<std::string>("string").get());
    EXPECT_EQ(true, union1.value<bool>("flag2").get());

    const auto union2 = yato::conf::config_union(conf1, conf2, yato::conf::priority::right);
    EXPECT_TRUE(union2.is_object());
    EXPECT_EQ(43, union2.value<int>("int").get());
    EXPECT_FLOAT_EQ(7.0f, union2.value<float>("float").get());
    EXPECT_EQ(true, union2.value<bool>("flag").get());
    EXPECT_EQ("text", union2.value<std::string>("string").get());
    EXPECT_EQ(true, union2.value<bool>("flag2").get());


    const auto union3 = yato::conf::config_union(conf1, conf1);
    EXPECT_TRUE(union3.is_object());
    EXPECT_EQ(42, union3.value<int>("int").get());
    EXPECT_FLOAT_EQ(7.0f, union3.value<float>("float").get());
    EXPECT_EQ(false, union3.value<bool>("flag").get());
}

/**
 *  JSON 1
 *  {
 *      "int": 42,
 *      "float" : 7.0f,
 *      "flag" : false
 *  }
 *  
 *  JSON 1
 *  {
 *      "int": 43,
 *      "flag" : true,
 *      "string" : "text",
 *      "flag2" : true
 *  }
 */
inline
void TestConfig_Intersection(const yato::conf::config & conf1, const yato::conf::config & conf2)
{
    ASSERT_TRUE(conf1.is_object());
    ASSERT_TRUE(conf2.is_object());

    const auto intersection1 = yato::conf::config_intersection(conf1, conf2, yato::conf::priority::left);
    EXPECT_TRUE(intersection1.is_object());
    EXPECT_EQ(42, intersection1.value<int>("int").get());
    EXPECT_FLOAT_EQ(false, intersection1.value<bool>("flag").get());

    const auto intersection2 = yato::conf::config_intersection(conf1, conf2, yato::conf::priority::right);
    EXPECT_TRUE(intersection2.is_object());
    EXPECT_EQ(43, intersection2.value<int>("int").get());
    EXPECT_FLOAT_EQ(true, intersection2.value<bool>("flag").get());

    const auto intersection3 = yato::conf::config_intersection(conf1, conf1);
    EXPECT_TRUE(intersection3.is_object());
    EXPECT_EQ(42, intersection3.value<int>("int").get());
    EXPECT_FLOAT_EQ(7.0f, intersection3.value<float>("float").get());
    EXPECT_EQ(false, intersection3.value<bool>("flag").get());
}




#endif // _YATO_CONFIG_TEST_CONFIG_OPS_COMMON_H_
