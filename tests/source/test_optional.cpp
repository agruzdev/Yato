#include "gtest/gtest.h"

#include <string>

#include <yato/optional.h>


#ifdef YATO_HAS_OPTIONAL
namespace
{
    yato::optional<std::string> create(bool b)
    {
        if (b) {
            return yato::make_optional<std::string>("Something");
        }
        else {
            return yato::make_optional<std::string>();
        }
    }
}

TEST(Yato_Optional, common)
{
    EXPECT_EQ("Empty",     create(false).value_or("Empty"));
    EXPECT_EQ("Something", create(true).value_or("Empty"));
}

#endif

