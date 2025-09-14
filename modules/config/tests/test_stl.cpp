/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"
#include <yato/config/config_builder.h>
#include <yato/config/stl.h>


TEST(Yato_Config, stl_vector)
{
    auto c = yato::config_builder::array()
        .add(10).add(20).add(30).create();

    auto v1 = c.cvt<std::vector<int32_t>>();

    ASSERT_FALSE(v1.empty());
    ASSERT_EQ(v1.get().size(), 3);
    ASSERT_EQ(v1.get().at(0), 10);
    ASSERT_EQ(v1.get().at(1), 20);
    ASSERT_EQ(v1.get().at(2), 30);

    auto v2 = c.cvt<std::vector<double>>();
    ASSERT_FALSE(v2.empty());

    auto v3 = c.cvt<std::vector<std::string>>();
    ASSERT_FALSE(v3.empty());

    auto v4 = c.cvt<std::vector<yato::config>>();
    ASSERT_TRUE(v4.empty());

}


TEST(Yato_Config, stl_optional)
{
    auto c = yato::config_builder::object()
        .put("key1", 10.0)
        .put("key2", "test")
        .put("key3", yato::config_builder::array().add("a").add("b").create())
        .create();

    auto v1 = c.value<std::optional<double>>("key1");
    ASSERT_FALSE(v1.empty());
    //ASSERT_DOUBLE_EQ(v1.get(), 10.0);

    auto v2 = c.value<std::optional<double>>("none");
    ASSERT_TRUE(v2.empty());

    auto v3 = c.value<std::optional<std::vector<std::string>>>("key3");
    ASSERT_FALSE(v3.empty());
    ASSERT_EQ(v3.get()->size(), 2);
}


TEST(Yato_Config, stl_map)
{
    auto c = yato::config_builder::object()
        .put("key1", 10.0)
        .put("key2", 7)
        .put("key3", 90.0)
        .create();

    auto m = c.cvt<std::map<std::string, double>>();
    ASSERT_EQ(m.get().size(), 3);
    ASSERT_EQ(m.get()["key1"], 10);
    ASSERT_EQ(m.get()["key2"], 7);
    ASSERT_EQ(m.get()["key3"], 90);
}

TEST(Yato_Config, stl_smart_ptr)
{
    auto c = yato::config_builder::object()
        .put("key1", 10.0)
        .put("key2", "test")
        .put("key3", yato::config_builder::array().add("a").add("b").create())
        .create();

    auto p1 = c.value<std::unique_ptr<std::vector<std::string>>>("key3");
    ASSERT_FALSE(p1.empty());
    ASSERT_TRUE(p1.get() != nullptr);
    ASSERT_TRUE(p1.get()->size() == 2);

    auto p2 = c.value<std::shared_ptr<std::vector<std::string>>>("key3");
    ASSERT_FALSE(p2.empty());
    ASSERT_TRUE(p2.get() != nullptr);
    ASSERT_TRUE(p2.get()->size() == 2);
}

