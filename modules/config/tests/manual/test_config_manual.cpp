#include "../include/test_config.h"
#include <yato/config/manual/manual_object.h>


TEST(Yato_Config, manual_object)
{
    const auto conf = yato::conf::manual_builder::object()
        .put("int", 42)
        .put("str", "test")
        .put("subobj", yato::conf::manual_builder::object()
            .put("val", 7.0f)
            .create())
        .create();

    ASSERT_NE(nullptr, conf);

    TestConfig_Object(conf);
}

TEST(Yato_Config, manual_array)
{
    //JSON
    //    [10, 20, 30, true, 4, {
    //        "arr": []
    //    }]
    //

    const auto conf = yato::conf::manual_builder::array()
        .add(10)
        .add(20)
        .add(30)
        .add(true)
        .add(4)
        .add(yato::conf::manual_builder::object()
            .put("arr", yato::conf::manual_builder::array()
                .create())
            .create())
        .create();

    ASSERT_NE(nullptr, conf);

    TestConfig_Array(conf);
}
