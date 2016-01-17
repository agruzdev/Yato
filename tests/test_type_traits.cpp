#include "gtest/gtest.h"

#include <yato/type_traits.h>
#include <yato/range.h>

#include <vector>
#include <list>

TEST(Yato_TypeTraits, is_smart_ptr)
{
    using sptr = std::shared_ptr<int>;
    using uptr = std::unique_ptr<int>;

    EXPECT_TRUE(yato::is_shared_ptr<sptr>::value);
    EXPECT_TRUE(yato::is_unique_ptr<uptr>::value);
    
    EXPECT_FALSE(yato::is_shared_ptr<uptr>::value);
    EXPECT_FALSE(yato::is_unique_ptr<sptr>::value);
    
    EXPECT_FALSE(yato::is_unique_ptr<int>::value);
    EXPECT_FALSE(yato::is_unique_ptr<void>::value);
}

TEST(Yato_TypeTraits, is_iterator)
{
    EXPECT_TRUE(yato::is_iterator<std::vector<int>::iterator>::value);
    EXPECT_TRUE(yato::is_iterator<std::vector<int>::const_iterator>::value);

    EXPECT_TRUE(yato::is_iterator<std::vector<int>::reverse_iterator>::value);
    EXPECT_TRUE(yato::is_iterator<std::vector<int>::const_reverse_iterator>::value);

    EXPECT_TRUE(yato::is_iterator<std::list<float>::iterator>::value);
    EXPECT_TRUE(yato::is_iterator<std::list<float>::const_iterator>::value);

    EXPECT_FALSE(yato::is_iterator<std::vector<int>>::value);
    EXPECT_FALSE(yato::is_iterator<float>::value);
}

TEST(Yato_TypeTraits, numeric_iterator)
{
    EXPECT_TRUE(yato::is_iterator<yato::numeric_iterator<int>>::value);
    EXPECT_TRUE(yato::is_iterator<yato::numeric_iterator<size_t>>::value);
}