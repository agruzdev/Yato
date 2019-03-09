/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

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

    const auto keys = conf.keys();
    EXPECT_EQ(5u, keys.size());

    EXPECT_TRUE(std::find(keys.cbegin(), keys.cend(), "int") != keys.cend());
    EXPECT_TRUE(std::find(keys.cbegin(), keys.cend(), "message") != keys.cend());
    EXPECT_TRUE(std::find(keys.cbegin(), keys.cend(), "flt") != keys.cend());
    EXPECT_TRUE(std::find(keys.cbegin(), keys.cend(), "flag1") != keys.cend());
    EXPECT_TRUE(std::find(keys.cbegin(), keys.cend(), "flag2") != keys.cend());

    const auto i = conf.value<int32_t>("int");
    EXPECT_EQ(42, i.get_or(0));

    const auto str = conf.value<std::string>("message");
    EXPECT_EQ("somestr", str.get_or(""));

    const auto f1 = conf.value<bool>("flag1");
    EXPECT_EQ(false, f1.get_or(true));

    const auto f2 = conf.value<bool>("flag2");
    EXPECT_EQ(true, f2.get_or(false));

    EXPECT_EQ(true, conf.flag("flag2"));

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

    const auto v2 = conf.value<float>("subobj.val").get_or(-1.0f);
    EXPECT_FLOAT_EQ(7.0f, v2);

    const auto v3 = conf.value<float>(yato::conf::path("/subobj/val", '/')).get_or(-1.0f);
    EXPECT_FLOAT_EQ(7.0f, v3);
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
    EXPECT_EQ(30U, conf.value<uint64_t>(2).get_or(-1));
    EXPECT_EQ(true, conf.value<bool>(3).get_or(false));
    EXPECT_EQ(true, conf.flag(3));

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
    EXPECT_EQ(true, conf.flag("manual_mode"));

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

        EXPECT_EQ(174, conf.value<int>("location.x").get_or(-1));
        EXPECT_EQ(34,  conf.value<int>("location.y").get_or(-1));
        EXPECT_EQ(174, conf.value<int>(yato::conf::path("location:x", ':')).get_or(-1));
        EXPECT_EQ(34,  conf.value<int>(yato::conf::path("location:y", ':')).get_or(-1));

        EXPECT_EQ(-1,  conf.value<int>(yato::conf::path("location.z.x", ':')).get_or(-1));
        EXPECT_EQ(-1,  conf.value<int>(yato::conf::path("location.x.z", ':')).get_or(-1));
    }
}

namespace
{
    enum class TestEnum : int32_t
    {
        eNull = 0,
        eVal1 = 7,
        eVal2 = 14,
        eVal3 = 21
    };

    struct enum_converter
    {
        TestEnum operator()(int64_t value) const
        {
            return static_cast<TestEnum>(value);
        }
    };

    struct TestVec3
    {
        int32_t x = 0;
        int32_t y = 0;
        int32_t z = 0;
    };

    struct vec3_converter
    {
        TestVec3 operator()(const yato::conf::config & arr) const
        {
            TestVec3 v;
            v.x = arr.value<int32_t>(0).get_or(-1);
            v.y = arr.value<int32_t>(1).get_or(-1);
            v.z = arr.value<int32_t>(2).get_or(-1);
            return v;
        }
    };
}

namespace yato
{
    namespace conf
    {
        template<>
        struct config_value_trait<TestEnum>
        {
            using converter_type = enum_converter;
            static constexpr config_type stored_type = config_type::integer;
        };

        template<>
        struct config_value_trait<TestVec3>
        {
            using converter_type = vec3_converter;
            static constexpr config_type stored_type = config_type::config;
        };
    }
}

/**
 * JSON
 * {
 *      "enum1" : 7,
 *      "enum2" : 14,
 *      
 *      "vec" : [20, 98, -7]
 * }
 */
inline
void TestConfig_Conversion(const yato::conf::config & conf)
{
    EXPECT_FALSE(conf.empty());

    const TestEnum e1 = conf.value<TestEnum>("enum1").get_or(TestEnum::eNull);
    const TestEnum e2 = conf.value<TestEnum>("enum2").get_or(TestEnum::eNull);
    const TestEnum e3 = conf.value<TestEnum>("enum3").get_or(TestEnum::eNull);

    EXPECT_EQ(TestEnum::eVal1, e1);
    EXPECT_EQ(TestEnum::eVal2, e2);
    EXPECT_EQ(TestEnum::eNull, e3);

    // Not all configs support nested objects.
    // Use string serialization for them.
    if(conf.array("vec")) {
        const auto vecOpt = conf.value<TestVec3>("vec");
        ASSERT_FALSE(vecOpt.empty());
        EXPECT_EQ(20, vecOpt.get().x);
        EXPECT_EQ(98, vecOpt.get().y);
        EXPECT_EQ(-7, vecOpt.get().z);
    }
}

#endif // _YATO_CONFIG_TEST_CONFIG_COMMON_H_
