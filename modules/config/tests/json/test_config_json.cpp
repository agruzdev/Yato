#include "gtest/gtest.h"

#include <cstdint>

#include <yato/config/config.h>
#include <yato/config/json/json_object.h>


TEST(Yato_Config, json_config)
{
    yato::conf::json_factory factory{};
    
    const yato::conf::config_ptr c1 = factory.create(R"JSON(
        {
            "int": 42,
            "str": "test",
            "subobj" : {
                "val": 7.0
            }
        }
    )JSON");
    ASSERT_NE(nullptr, c1);

    const auto i = c1->value<int32_t>("int");
    EXPECT_EQ(42, i.get_or(0));

    const auto str = c1->value<std::string>("str");
    EXPECT_EQ("test", str.get_or(""));

    const yato::conf::config_ptr c2 = c1->config("subobj").get_or(nullptr);
    ASSERT_NE(nullptr, c2);

    const auto v = c2->value<float>("val").get_or(-1.0f);
    EXPECT_FLOAT_EQ(7.0f, v);
}
