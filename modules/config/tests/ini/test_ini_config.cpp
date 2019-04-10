/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

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

TEST(Yato_Config, ini_object2)
{
    const auto conf = yato::conf::ini_builder().parse(R"INI(
        int= 42
        str=test

        [subobj]
        val = 7.0  
    )INI");
    TestConfig_Object(conf);
}

TEST(Yato_Config, ini_example)
{
    const auto conf = yato::conf::ini_builder().parse(R"INI(
        [GLOBAL]
        answer=42
        comment=everything
        precision=0.01
        manual_mode=true

        [location]
        x = 174
        y = 34
    )INI");
    TestConfig_Example(conf);
}



