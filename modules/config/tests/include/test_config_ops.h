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

    const auto union1 = yato::conf::object_union(conf1, conf2, yato::conf::priority::left);
    EXPECT_TRUE(union1.is_object());
    EXPECT_EQ(42, union1.value<int>("int").get());
    EXPECT_FLOAT_EQ(7.0f, union1.value<float>("float").get());
    EXPECT_EQ(false, union1.value<bool>("flag").get());
    EXPECT_EQ("text", union1.value<std::string>("string").get());
    EXPECT_EQ(true, union1.value<bool>("flag2").get());

    const auto union2 = yato::conf::object_union(conf1, conf2, yato::conf::priority::right);
    EXPECT_TRUE(union2.is_object());
    EXPECT_EQ(43, union2.value<int>("int").get());
    EXPECT_FLOAT_EQ(7.0f, union2.value<float>("float").get());
    EXPECT_EQ(true, union2.value<bool>("flag").get());
    EXPECT_EQ("text", union2.value<std::string>("string").get());
    EXPECT_EQ(true, union2.value<bool>("flag2").get());


    const auto union3 = yato::conf::object_union(conf1, conf1);
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

    const auto intersection1 = yato::conf::object_intersection(conf1, conf2, yato::conf::priority::left);
    EXPECT_TRUE(intersection1.is_object());
    EXPECT_EQ(42, intersection1.value<int>("int").get());
    EXPECT_FLOAT_EQ(false, intersection1.value<bool>("flag").get());

    const auto intersection2 = yato::conf::object_intersection(conf1, conf2, yato::conf::priority::right);
    EXPECT_TRUE(intersection2.is_object());
    EXPECT_EQ(43, intersection2.value<int>("int").get());
    EXPECT_FLOAT_EQ(true, intersection2.value<bool>("flag").get());

    const auto intersection3 = yato::conf::object_intersection(conf1, conf1);
    EXPECT_TRUE(intersection3.is_object());
    EXPECT_EQ(42, intersection3.value<int>("int").get());
    EXPECT_FLOAT_EQ(7.0f, intersection3.value<float>("float").get());
    EXPECT_EQ(false, intersection3.value<bool>("flag").get());
}



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
    EXPECT_EQ(5, cat1.size());
    EXPECT_EQ(10,  cat1.value<int>(0).get());
    EXPECT_EQ(20,  cat1.value<int>(1).get());
    EXPECT_EQ(30,  cat1.value<int>(2).get());
    EXPECT_EQ(400, cat1.value<int>(3).get());
    EXPECT_EQ(500, cat1.value<int>(4).get());

    const auto cat2 = yato::conf::array_cat(conf2, conf1);
    EXPECT_TRUE(cat2.is_array());
    EXPECT_EQ(5, cat2.size());
    EXPECT_EQ(400, cat2.value<int>(0).get());
    EXPECT_EQ(500, cat2.value<int>(1).get());
    EXPECT_EQ(10,  cat2.value<int>(2).get());
    EXPECT_EQ(20,  cat2.value<int>(3).get());
    EXPECT_EQ(30,  cat2.value<int>(4).get());
}





#endif // _YATO_CONFIG_TEST_CONFIG_OPS_COMMON_H_
