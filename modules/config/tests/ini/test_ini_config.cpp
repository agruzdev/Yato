#include "../include/test_config.h"
#include <yato/config/ini/ini_config.h>

TEST(Yato_Config, ini_object)
{
    const auto conf = yato::conf::ini_builder().parse(R"INI(
        int=42
        message=somestr
        flt = 7.0
        flag1=false
        flag2 =True
    )INI");
    TestConfig_PlainObject(conf);
}

TEST(Yato_Config, ini_example)
{
    const auto conf = yato::conf::ini_builder().parse(R"INI(
        answer=42
        comment=everything
        precision=0.01
        manual_mode=true
    )INI");
    TestConfig_Example(conf);
}


