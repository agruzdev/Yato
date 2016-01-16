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

TEST(Yato_Types, TestLiterals)
{
	using namespace yato::literals;
#if YATO_DEBUG
	EXPECT_THROW(1000_u8, yato::assertion_error);
#else
	EXPECT_TRUE(true);
#endif
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
}
