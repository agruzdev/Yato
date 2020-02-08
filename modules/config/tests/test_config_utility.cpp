/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"
#include <yato/config/utility.h>

namespace
{

    template <yato::conf::stored_type STy_>
    void TestDecoding(const std::string & str, const typename yato::conf::serializer<STy_>::value_type & ref)
    {
        using value_type = typename yato::conf::serializer<STy_>::value_type;

        // test decode
        value_type dec1{};
        ASSERT_TRUE(yato::conf::serializer<STy_>::cvt_from(str, &dec1));
        ASSERT_EQ(ref, dec1);

        // test encode -> decode
        const std::string enc1 = yato::conf::serializer<STy_>::to_string(ref);
        value_type dec2{};
        ASSERT_TRUE(yato::conf::serializer<STy_>::cvt_from(enc1, &dec2));
        ASSERT_EQ(ref, dec2);
    }

    template <yato::conf::stored_type STy_>
    void TestFromInt(const typename yato::conf::serializer<yato::conf::stored_type::integer>::value_type & val, 
        const typename yato::conf::serializer<STy_>::value_type & ref)
    {
        using value_type = typename yato::conf::serializer<STy_>::value_type;

        value_type dec1{};
        ASSERT_TRUE(yato::conf::serializer<STy_>::cvt_from(static_cast<int64_t>(val), &dec1));
        ASSERT_EQ(ref, dec1);
    }

    template <yato::conf::stored_type STy_>
    void TestFromReal(const typename yato::conf::serializer<yato::conf::stored_type::real>::value_type & val, 
        const typename yato::conf::serializer<STy_>::value_type & ref)
    {
        using value_type = typename yato::conf::serializer<STy_>::value_type;

        value_type dec1{};
        ASSERT_TRUE(yato::conf::serializer<STy_>::cvt_from(static_cast<double>(val), &dec1));
        ASSERT_EQ(ref, dec1);
    }

    template <yato::conf::stored_type STy_>
    void TestFromBool(const typename yato::conf::serializer<yato::conf::stored_type::boolean>::value_type & val, 
        const typename yato::conf::serializer<STy_>::value_type & ref)
    {
        using value_type = typename yato::conf::serializer<STy_>::value_type;

        value_type dec1{};
        ASSERT_TRUE(yato::conf::serializer<STy_>::cvt_from(val, &dec1));
        ASSERT_EQ(ref, dec1);
    }

} // namespace

TEST(Yato_Config, utility_serializer_int)
{
    TestDecoding<yato::conf::stored_type::integer>("42", 42);
    TestDecoding<yato::conf::stored_type::integer>(" 10", 10);
    TestDecoding<yato::conf::stored_type::integer>(" -11  \n ", -11);
    TestDecoding<yato::conf::stored_type::integer>("\t\n 14", 14);
    TestDecoding<yato::conf::stored_type::integer>(" 0 ", 0);

    TestDecoding<yato::conf::stored_type::integer>(" 0xff", 0xff);
    TestDecoding<yato::conf::stored_type::integer>(" 0777 ", 0777);
}

TEST(Yato_Config, utility_serializer_int2)
{
    TestFromInt<yato::conf::stored_type::integer>(42, 42);
    TestFromInt<yato::conf::stored_type::integer>(0, 0);
    TestFromInt<yato::conf::stored_type::integer>(-12, -12);

    TestFromReal<yato::conf::stored_type::integer>(0.0, 0);
    TestFromReal<yato::conf::stored_type::integer>(1.0, 1);
    TestFromReal<yato::conf::stored_type::integer>(42.9, 42);
    TestFromReal<yato::conf::stored_type::integer>(-1.1, -1);

    TestFromBool<yato::conf::stored_type::integer>(true, 1);
    TestFromBool<yato::conf::stored_type::integer>(false, 0);
}

TEST(Yato_Config, utility_serializer_real)
{
    TestDecoding<yato::conf::stored_type::real>("42.5", 42.5);
    TestDecoding<yato::conf::stored_type::real>(" 10.1", 10.1);
    TestDecoding<yato::conf::stored_type::real>(" -11.0  \n ", -11.0);
    TestDecoding<yato::conf::stored_type::real>("\t\n 14", 14.0);
    TestDecoding<yato::conf::stored_type::real>(" 0.0 ", 0.0);
}

TEST(Yato_Config, utility_serializer_real2)
{
    TestFromInt<yato::conf::stored_type::real>(22, 22.0);
    TestFromInt<yato::conf::stored_type::real>(0, 0.0);
    TestFromInt<yato::conf::stored_type::real>(-14, -14.0);

    TestFromReal<yato::conf::stored_type::real>(0.0, 0.0);
    TestFromReal<yato::conf::stored_type::real>(1.0, 1.0);
    TestFromReal<yato::conf::stored_type::real>(53.9, 53.9);
    TestFromReal<yato::conf::stored_type::real>(-1.1, -1.1);
}

TEST(Yato_Config, utility_serializer_bool)
{
    TestDecoding<yato::conf::stored_type::boolean>("Yes", true);
    TestDecoding<yato::conf::stored_type::boolean>(" on", true);
    TestDecoding<yato::conf::stored_type::boolean>(" y  \n ", true);
    TestDecoding<yato::conf::stored_type::boolean>("\t\n TRUE", true);
    TestDecoding<yato::conf::stored_type::boolean>(" T ", true);
    TestDecoding<yato::conf::stored_type::boolean>("1 ", true);

    TestDecoding<yato::conf::stored_type::boolean>("nO", false);
    TestDecoding<yato::conf::stored_type::boolean>(" OFF", false);
    TestDecoding<yato::conf::stored_type::boolean>(" f  \n ", false);
    TestDecoding<yato::conf::stored_type::boolean>("\t\n False", false);
    TestDecoding<yato::conf::stored_type::boolean>(" N ", false);
    TestDecoding<yato::conf::stored_type::boolean>("0 ", false);
}

TEST(Yato_Config, utility_serializer_bool2)
{
    TestFromInt<yato::conf::stored_type::boolean>(1, true);
    TestFromInt<yato::conf::stored_type::boolean>(0, false);

    TestFromBool<yato::conf::stored_type::boolean>(true, true);
    TestFromBool<yato::conf::stored_type::boolean>(false, false);
}
