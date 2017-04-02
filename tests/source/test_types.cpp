#include "gtest/gtest.h"

#include <type_traits>
#include <yato/types.h>

TEST(Yato_Types, sizes)
{
    EXPECT_TRUE(1 == sizeof(yato::int8_t));
    EXPECT_TRUE(1 == sizeof(yato::uint8_t));
    EXPECT_TRUE(2 == sizeof(yato::int16_t));
    EXPECT_TRUE(2 == sizeof(yato::uint16_t));
    EXPECT_TRUE(4 == sizeof(yato::int32_t));
    EXPECT_TRUE(4 == sizeof(yato::uint32_t));
    EXPECT_TRUE(8 == sizeof(yato::int64_t));
    EXPECT_TRUE(8 == sizeof(yato::uint64_t));
    EXPECT_TRUE(4 == sizeof(yato::float32_t));
    EXPECT_TRUE(8 == sizeof(yato::float64_t));
}

TEST(Yato_Types, narrow_cast)
{
#if YATO_DEBUG
    EXPECT_THROW(yato::narrow_cast<yato::uint8_t>(1000U), yato::assertion_error);
    EXPECT_THROW(yato::narrow_cast<yato::int32_t>(1.5f), yato::assertion_error);
    EXPECT_THROW(yato::narrow_cast<yato::uint8_t>(-1), yato::assertion_error);
    EXPECT_NO_THROW(yato::narrow_cast<yato::uint8_t>(255U));
    EXPECT_NO_THROW(yato::narrow_cast<yato::int32_t>(1.0f));
#else
    EXPECT_TRUE(true);
#endif
}

#ifdef YATO_MSVC
# pragma warning(push)
# pragma warning(disable: 4189) //local variable is initialized but not referenced
# pragma warning(disable: 4700) //uninitialized local variable 'p1' used
#elif defined(YATO_CLANG)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunused-variable"
#endif

TEST(Yato_Types, pointer_cast)
{
    {
        int x = 0;
        int* p1 = &x;
        float* p2 = yato::pointer_cast<float*>(p1);
        YATO_MAYBE_UNUSED(p2);
    }
    {
        float x = 0.0f;
        const float* p1 = &x;
        const int* p2 = yato::pointer_cast<const int*>(p1);
        float* p3 = const_cast<float*>(yato::pointer_cast<const float*>(p1));
        YATO_MAYBE_UNUSED(p2);
        YATO_MAYBE_UNUSED(p3);
    }
    {
        volatile float x = 0.0f;
        const volatile float* p1 = &x;
        const volatile char* p2 = yato::pointer_cast<const volatile char*>(p1);
        YATO_MAYBE_UNUSED(p2);
    }
}
#ifdef YATO_MSVC
# pragma warning(pop)
#elif defined(YATO_CLANG)
# pragma clang diagnostic pop
#endif

#ifdef YATO_HAS_LITERALS
TEST(Yato_Types, TestLiterals)
{
    using namespace yato::literals;
#if YATO_DEBUG
    EXPECT_THROW(1000_u8, yato::assertion_error);
#else
    EXPECT_TRUE(true);
#endif

    EXPECT_NO_THROW(255_u8);
    EXPECT_NO_THROW(0.0_f32);
    EXPECT_NO_THROW(1.0_f32);
    EXPECT_NO_THROW(0.0000001_f32);
    EXPECT_NO_THROW(1000000.0_f32);
    EXPECT_NO_THROW(0.0_f64);
    EXPECT_NO_THROW(1.0_f64);
    EXPECT_NO_THROW(0.0000001_f64);
    EXPECT_NO_THROW(1000000.0_f64);
    EXPECT_NO_THROW(0.0_f80);
    EXPECT_NO_THROW(1.0_f80);
    EXPECT_NO_THROW(0.0000001_f80);
    EXPECT_NO_THROW(1000000.0_f80);
}

//Compile time
namespace TestLiterals 
{
    using namespace yato::literals;

    static_assert(std::is_same<yato::int8_t,  decltype(0_s8)>::value,  "Failed TestLiterals");
    static_assert(std::is_same<yato::int16_t, decltype(0_s16)>::value, "Failed TestLiterals");
    static_assert(std::is_same<yato::int32_t, decltype(0_s32)>::value, "Failed TestLiterals");
    static_assert(std::is_same<yato::int64_t, decltype(0_s64)>::value, "Failed TestLiterals");

    static_assert(std::is_same<yato::uint8_t,  decltype(0_u8)>::value,  "Failed TestLiterals");
    static_assert(std::is_same<yato::uint16_t, decltype(0_u16)>::value, "Failed TestLiterals");
    static_assert(std::is_same<yato::uint32_t, decltype(0_u32)>::value, "Failed TestLiterals");
    static_assert(std::is_same<yato::uint64_t, decltype(0_u64)>::value, "Failed TestLiterals");

    static_assert(std::is_same<yato::float32_t, decltype(0.0_f32)>::value, "Failed TestLiterals");
    static_assert(std::is_same<yato::float64_t, decltype(0.0_f64)>::value, "Failed TestLiterals");
    static_assert(std::is_same<yato::float80_t, decltype(0.0_f80)>::value, "Failed TestLiterals");
}
#endif
