/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_TEST_CONFIG_COMMON_H_
#define _YATO_CONFIG_TEST_CONFIG_COMMON_H_

#include "gtest/gtest.h"

#include <cstdint>

#include <yato/config/config.h>
#include <yato/config/utility.h>

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
    EXPECT_FALSE(conf.is_null());
    EXPECT_TRUE(conf.is_object());

    const auto keys = conf.keys();
    EXPECT_EQ(5u, keys.size());
    EXPECT_EQ(5u, conf.size());

    EXPECT_TRUE(std::find(keys.cbegin(), keys.cend(), "int") != keys.cend());
    EXPECT_TRUE(std::find(keys.cbegin(), keys.cend(), "message") != keys.cend());
    EXPECT_TRUE(std::find(keys.cbegin(), keys.cend(), "flt") != keys.cend());
    EXPECT_TRUE(std::find(keys.cbegin(), keys.cend(), "flag1") != keys.cend());
    EXPECT_TRUE(std::find(keys.cbegin(), keys.cend(), "flag2") != keys.cend());

    const auto i = conf.value<int32_t>("int");
    EXPECT_EQ(42, i.get_or(0));

    const auto ie = conf.find("int");
    EXPECT_FALSE(ie.is_null());
    //EXPECT_EQ(yato::conf::stored_type::integer, ie.type());
    EXPECT_EQ(42, ie.value<int32_t>().get_or(0));

    const auto str = conf.value<std::string>("message");
    EXPECT_EQ("somestr", str.get_or(""));

    const auto f1 = conf.value<bool>("flag1");
    EXPECT_EQ(false, f1.get_or(true));

    const auto f2 = conf.value<bool>("flag2");
    EXPECT_EQ(true, f2.get_or(false));

    EXPECT_EQ(true, conf.flag("flag2"));

    const auto v = conf.value<float>("flt").get_or(-1.0f);
    EXPECT_FLOAT_EQ(7.0f, v);

    EXPECT_NO_THROW(
        for (const auto & entry : conf) {
            EXPECT_FALSE(entry.is_null());
            if (entry.key() == "int") {
                EXPECT_EQ(42, entry.value<int>().get());
            }
            else if (entry.key() == "message") {
                EXPECT_EQ("somestr", entry.value<std::string>().get());
            }
            else if (entry.key() == "flt") {
                EXPECT_EQ(7.0, entry.value<double>().get());
            }
            else if (entry.key() == "flag1") {
                EXPECT_EQ(false, entry.value<bool>().get());
            }
            else if (entry.key() == "flag2") {
                EXPECT_EQ(true, entry.value<bool>().get());
            }
            else {
                GTEST_FAIL() << "Invalid entry key.";
            }
        }
    );


    const auto v1 = conf.to_vector<std::string>();
    ASSERT_EQ(5u, v1.size());

    const auto m1 = conf.to_map<std::string>();
    ASSERT_EQ(5u, m1.size());
    EXPECT_EQ(42, std::stoi(m1.at("int")));
    EXPECT_EQ("somestr", m1.at("message"));
    EXPECT_EQ(7.0, std::stod(m1.at("flt")));
    bool tmp_bool = false;
    ASSERT_TRUE(yato::conf::serializer<yato::conf::stored_type::boolean>::cvt_from(m1.at("flag1"), &tmp_bool));
    EXPECT_EQ(false, tmp_bool);
    ASSERT_TRUE(yato::conf::serializer<yato::conf::stored_type::boolean>::cvt_from(m1.at("flag2"), &tmp_bool));
    EXPECT_EQ(true, tmp_bool);
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
    EXPECT_FALSE(conf.is_null());
    EXPECT_TRUE(conf.is_object());

    EXPECT_EQ(3u, conf.size());

    const auto i = conf.value<int32_t>("int");
    EXPECT_EQ(42, i.get_or(0));

    const auto str = conf.value<std::string>("str");
    EXPECT_EQ("test", str.get_or(""));

    const yato::conf::config c2 = conf.object("subobj");
    ASSERT_FALSE(c2.is_null());
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
    EXPECT_FALSE(conf.is_null());
    EXPECT_EQ(6U, conf.size());

    EXPECT_EQ(10, conf.value<int>(0).get_or(-1));
    EXPECT_EQ(20, conf.value<short>(1).get_or(-1));
    EXPECT_EQ(30U, conf.value<uint64_t>(2).get_or(-1));
    EXPECT_EQ(true, conf.value<bool>(3).get_or(false));
    EXPECT_EQ(true, conf.flag(3));

    const auto c2 = conf.object(5);
    ASSERT_FALSE(c2.is_null());
    ASSERT_FALSE(c2.empty());
    EXPECT_EQ(1u, c2.size());

    const auto arr = c2.array("arr");
    ASSERT_FALSE(arr.is_null());
    ASSERT_TRUE(arr.empty());

    EXPECT_EQ(0U, arr.size());

    EXPECT_NO_THROW(
        auto it = conf.begin();

        ASSERT_TRUE(it.has_next());
        const auto e0 = it.next();
        ASSERT_EQ(10, e0.value<int>().get());

        ASSERT_TRUE(it.has_next());
        const auto e1 = it.next();
        ASSERT_EQ(20, e1.value<int>().get());

        ASSERT_TRUE(it.has_next());
        const auto e2 = it.next();
        ASSERT_EQ(30, e2.value<int>().get());

        ASSERT_TRUE(it.has_next());
        const auto e3 = it.next();
        ASSERT_EQ(true, e3.value<bool>().get());

        ASSERT_TRUE(it.has_next());
        const auto e4 = it.next();
        ASSERT_EQ(4, e4.value<int>().get());

        ASSERT_TRUE(it.has_next());
        const auto e5 = it.next();
        ASSERT_EQ(yato::conf::stored_type::config, e5.type());
        const auto c3 = e5.object();
        ASSERT_FALSE(c3.is_null());

        ASSERT_FALSE(it.has_next());
    );

    EXPECT_NO_THROW(
        auto it = conf.begin();
        auto eit = conf.end();

        ASSERT_TRUE(it != eit);
        const auto e0 = *it;
        ASSERT_EQ(10, e0.value<int>().get());
        ASSERT_EQ(10, it->value<int>().get());

        ++it;
        ASSERT_TRUE(it != eit);
        const auto e1 = *it;
        ASSERT_EQ(20, e1.value<int>().get());
        ASSERT_EQ(20, it->value<int>().get());

        ++it;
        ASSERT_TRUE(it != eit);
        const auto e2 = *it;
        ASSERT_EQ(30, e2.value<int>().get());
        ASSERT_EQ(30, it->value<int>().get());

        it++;
        ASSERT_TRUE(it != eit);
        const auto e3 = *it;
        ASSERT_EQ(true, e3.value<bool>().get());
        ASSERT_EQ(true, it->value<bool>().get());

        it++;
        ASSERT_TRUE(it != eit);
        const auto e4 = *it;
        ASSERT_EQ(4, e4.value<int>().get());
        ASSERT_EQ(4, it->value<int>().get());

        ++it;
        ASSERT_TRUE(it != eit);
        const auto e5 = *it;
        ASSERT_EQ(yato::conf::stored_type::config, it->type());
        const auto c3 = e5.object();
        ASSERT_FALSE(c3.is_null());

        ASSERT_FALSE(it == eit);
    );
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
    EXPECT_FALSE(conf.is_null());

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

    size_t conf_size = 4;

    const yato::conf::config arr = conf.array("fruits");
    if(arr) {
        EXPECT_EQ(3U, arr.size());
        EXPECT_NO_THROW(
            EXPECT_EQ(std::string("apple"),  arr.value<std::string>(0).get());
            EXPECT_EQ(std::string("banana"), arr.value<std::string>(1).get());
            EXPECT_EQ(std::string("kiwi"),   arr.value<std::string>(2).get());
        );
        ++conf_size;
    }

    const yato::conf::config point = conf.object("location");
    if(point) {
        ASSERT_TRUE(point.is_object());
        EXPECT_EQ(2u, point.size());
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
        
        ++conf_size;
    }

    EXPECT_EQ(conf_size, conf.size());
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


    struct TestVec4
    {
        int32_t x = 0;
        int32_t y = 0;
        int32_t z = 0;
        int32_t w = 1;
    };

    struct vec4_converter
    {
        TestVec4 operator()(const yato::conf::config & arr) const
        {
            TestVec4 v;
            v.x = arr.value<int32_t>(0).get_or(-1);
            v.y = arr.value<int32_t>(1).get_or(-1);
            v.z = arr.value<int32_t>(2).get_or(-1);
            v.w = arr.value<int32_t>(3).get_or(-1);
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
            static constexpr stored_type fetch_type = stored_type::integer;
        };

        template<>
        struct config_value_trait<TestVec3>
        {
            using converter_type = vec3_converter;
            static constexpr stored_type fetch_type = stored_type::config;
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
    EXPECT_FALSE(conf.is_null());

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

        // Access without registered converter, pass it ad hoc
        const auto v4 = conf.value<yato::conf::stored_type::config>("vec", vec4_converter{});
        ASSERT_FALSE(vecOpt.empty());
        EXPECT_EQ(20, v4.get().x);
        EXPECT_EQ(98, v4.get().y);
        EXPECT_EQ(-7, v4.get().z);
        EXPECT_EQ(-1, v4.get().w);
    }
}

#endif // _YATO_CONFIG_TEST_CONFIG_COMMON_H_
