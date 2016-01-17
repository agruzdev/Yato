#include "gtest/gtest.h"

#include <yato/array_nd.h>

TEST(Yato_Array_Nd, array_nd)
{
	yato::array_nd<int, 2> array_1d;
	EXPECT_NO_THROW(array_1d[0] = 0);
	EXPECT_NO_THROW(array_1d[1] = 0);
#if YATO_DEBUG
	EXPECT_THROW(array_1d[2] = 0, yato::assertion_error);
#endif


	yato::array_nd<int, 2, 3> array_2d;
	EXPECT_NO_THROW(array_2d[1]);
#if YATO_DEBUG
	EXPECT_THROW(array_2d[3], yato::assertion_error);
#endif
	auto&& p = array_2d[1];
	EXPECT_NO_THROW(p[0] = 1);
#if YATO_DEBUG
	EXPECT_THROW(p[4] = 0, yato::assertion_error);
#endif

	yato::array_nd<int, 2, 3, 4> array_3d;
	EXPECT_NO_THROW(array_3d[1][1][1] = 2);
	EXPECT_NO_THROW(array_3d[1][2][3] = 2);
#if YATO_DEBUG
	EXPECT_THROW(array_3d[2][1][1] = 0, yato::assertion_error);
	EXPECT_THROW(array_3d[1][3][1] = 0, yato::assertion_error);
	EXPECT_THROW(array_3d[1][1][4] = 0, yato::assertion_error);
#endif
};
