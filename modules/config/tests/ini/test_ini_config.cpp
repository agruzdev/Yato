/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "../include/test_config.h"
#include <yato/config/ini/ini.h>

TEST(Yato_Config, ini_object)
{
    const auto conf = yato::conf::ini::read(R"INI(
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
    const auto conf = yato::conf::ini::read(R"INI(
        int= 42
        str=test

        [subobj]
        val = 7.0  
    )INI");
    TestConfig_Object(conf);
}

TEST(Yato_Config, ini_example)
{
    const auto conf = yato::conf::ini::read(R"INI(
        answer=42
        comment=everything
        precision=0.01
        manual_mode=true

        [location]
        x = 174
        y = 34
    )INI");
    TestConfig_Example(conf, true, false);
    TestConfig_Example(conf.clone(), true, false);
}



