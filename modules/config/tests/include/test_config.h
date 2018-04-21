#ifndef _YATO_CONFIG_TEST_CONFIG_COMMON_H_
#define _YATO_CONFIG_TEST_CONFIG_COMMON_H_

#include "gtest/gtest.h"

#include <cstdint>

#include <yato/config/config.h>

/**
 *  JSON 
 *  {
 *      "int": 42,
 *      "message": "somestr",
 *      "flt" : 7.0,
 *      "flag1" : false,
 *      "flag2" : true
 *  }
 */
inline
void TestConfig_PlainObject(const yato::conf::config & conf)
{
    EXPECT_FALSE(conf.empty());
    EXPECT_TRUE(conf.is_object());

    const auto i = conf.value<int32_t>("int");
    EXPECT_EQ(42, i.get_or(0));

    const auto str = conf.value<std::string>("message");
    EXPECT_EQ("somestr", str.get_or(""));

    const auto f1 = conf.value<bool>("flag1");
    EXPECT_EQ(false, f1.get_or(true));

    const auto f2 = conf.value<bool>("flag2");
    EXPECT_EQ(true, f2.get_or(false));

    const auto v = conf.value<float>("flt").get_or(-1.0f);
    EXPECT_FLOAT_EQ(7.0f, v);
}

/**
 * JSON 
 * {
 *     "int": 42,
 *     "str": "test",
 *     "subobj" : {
 *         "val": 7.0
 *     }
 * }
 */
inline
void TestConfig_Object(const yato::conf::config & conf)
{
    EXPECT_FALSE(conf.empty());
    EXPECT_TRUE(conf.is_object());

    const auto i = conf.value<int32_t>("int");
    EXPECT_EQ(42, i.get_or(0));

    const auto str = conf.value<std::string>("str");
    EXPECT_EQ("test", str.get_or(""));

    const yato::conf::config c2 = conf.object("subobj");
    ASSERT_FALSE(c2.empty());
    EXPECT_TRUE(c2.is_object());

    const auto v = c2.value<float>("val").get_or(-1.0f);
    EXPECT_FLOAT_EQ(7.0f, v);
}

/**
 * JSON
 *     [10, 20, 30, true, 4, {
 *         "arr": []
 *     }]
 */
inline
void TestConfig_Array(const yato::conf::config & conf)
{
    EXPECT_FALSE(conf.empty());
    EXPECT_TRUE(conf.is_array());
    EXPECT_EQ(6U, conf.size());

    EXPECT_EQ(10, conf.value<int>(0).get_or(-1));
    EXPECT_EQ(20, conf.value<short>(1).get_or(-1));
    EXPECT_EQ(30U, conf.value<unsigned long long>(2).get_or(-1));
    EXPECT_EQ(true, conf.value<bool>(3).get_or(false));

    const auto c2 = conf.object(5);
    ASSERT_FALSE(c2.empty());

    const auto arr = c2.array("arr");
    ASSERT_FALSE(arr.empty());

    EXPECT_TRUE(arr.is_array());
    EXPECT_EQ(0U, arr.size());
}

/**
 * More soft test, since not all backends can support full functionality
 * 
 * JSON
 * {
 *     "answer": 42,
 *     "comment": "everything",
 *     "precision" : 0.01,
 * 
 *     "manual_mode" : true,
 *
 *     "fruits" : [
 *         "apple", "banana", "kiwi"
 *     ]
 *
 *     "location" : {
 *         "x" : 174
 *         "y" : 34
 *     }
 * }
 */
inline
void TestConfig_Example(const yato::conf::config & conf)
{
    EXPECT_FALSE(conf.empty());

    const int answer = conf.value<int>("answer").get_or(-1);
    EXPECT_EQ(42, answer);

    const int answer2 = conf.value<int>("answer2").get_or(-1);
    EXPECT_EQ(-1, answer2);

    EXPECT_NO_THROW(
        const auto comment = conf.value<std::string>("comment").get();
        EXPECT_EQ(std::string("everything"), comment);
    );

    const float precision = conf.value<float>("precision").get_or(0.0f);
    EXPECT_EQ(0.01f, precision);

    const bool is_manual = conf.value<bool>("manual_mode").get_or(false);
    EXPECT_EQ(true, is_manual);

    const yato::conf::config arr = conf.array("fruits");
    if(arr) {
        ASSERT_TRUE(arr.is_array());
        EXPECT_EQ(3U, arr.size());
        EXPECT_NO_THROW(
            EXPECT_EQ(std::string("apple"),  arr.value<std::string>(0).get());
            EXPECT_EQ(std::string("banana"), arr.value<std::string>(1).get());
            EXPECT_EQ(std::string("kiwi"),   arr.value<std::string>(2).get());
        );
    }

    const yato::conf::config point = conf.object("location");
    if(point) {
        ASSERT_TRUE(point.is_object());
        const int x = point.value<int>("x").get_or(-1);
        const int y = point.value<int>("y").get_or(-1);
        EXPECT_EQ(174, x);
        EXPECT_EQ(34,  y);
    }
}

#endif // _YATO_CONFIG_TEST_CONFIG_COMMON_H_
