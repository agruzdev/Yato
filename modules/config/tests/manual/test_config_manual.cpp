#include "gtest/gtest.h"

#include <cstdint>
#include <iostream>

#include <yato/config/manual/manual_value.h>
#include <yato/config/manual/manual_array.h>
#include <yato/config/manual/manual_object.h>

TEST(Yato_Config, manual_value)
{
    yato::manual_value v(static_cast<int64_t>(42));

    EXPECT_EQ(42, v.get<int32_t>(-1));
    EXPECT_FLOAT_EQ(-1.0f, v.get<float>(-1.0f));

    v = 7.0f;
    EXPECT_EQ(-1, v.get<int32_t>(-1));
    EXPECT_FLOAT_EQ(7.0f, v.get<float>(-1.0f));

    EXPECT_THROW(v.get<int32_t>(), yato::config_error);
}

TEST(Yato_Config, manual_array)
{
    yato::manual_array arr;
    arr.resize(3);
    arr.put(0, 10);
    arr.put(1, 20);
    arr.put(2, 30);

    EXPECT_NO_THROW(
        EXPECT_EQ(10, arr.value<int>(0).get());
        EXPECT_EQ(20, arr.value<int>(1).get());
        EXPECT_EQ(30, arr.value<int>(2).get());
    );

    yato::manual_array arr2(arr);
    arr2.resize(4);
    arr2.put(3, 40);
    
    yato::manual_array arr2d;
    arr2d.resize(3);
    arr2d.append(arr);
    arr2d.append(arr);
    arr2d.append(arr);
}

TEST(Yato_Config, manual_object)
{
    yato::manual_array arr;
    arr.resize(3);
    arr.put(0, 10);
    arr.put(1, 20);
    arr.put(2, 30);

    yato::manual_object obj;
    obj.put("len", arr.size());
    obj.put("arr", arr);
    
    //std::cout << "len: "<< obj.get_value("len")->get<size_t>() << std::endl;
    //std::cout << "len: "<< obj.get_value("len").deref_or(yato::manual_value(-1)).get<size_t>() << std::endl;
    //std::cout << "arr: "<< *dynamic_cast<const yato::manual_array*>(obj.get_array("arr").get()) << std::endl;
    //std::cout << obj;

    EXPECT_NO_THROW(
        EXPECT_EQ(3U, obj.value<size_t>("len").get())
    );
}
