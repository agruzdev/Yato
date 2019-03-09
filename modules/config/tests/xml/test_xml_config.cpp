/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "../include/test_config.h"
#include <yato/config/xml/xml_config.h>

TEST(Yato_Config, xml_object)
{
    const auto conf = yato::conf::xml_builder().parse(R"XML(
        <root>
            <int>42</int>
            <message>somestr</message>
            <flt>7.0</flt>
            <flag1>false</flag1>
            <flag2>true</flag2>
        </root>
    )XML");
    TestConfig_PlainObject(conf);
}

TEST(Yato_Config, xml_object2)
{
    const auto conf = yato::conf::xml_builder().parse(R"XML(
        <root>
            <int>42</int>
            <str>test</str>
            <subobj>
                <val>7.0</val>
            </subobj>
        </root>
    )XML");
    TestConfig_Object(conf);
}

TEST(Yato_Config, xml_array)
{
    const auto conf = yato::conf::xml_builder().parse(R"XML(
        <root is_array="true">
            <item>10</item>
            <item>20</item>
            <item>30</item>
            <item>true</item>
            <item>4</item>
            <item>
                <arr is_array="true">
                </arr>
            </item>
        </root>
    )XML");
    TestConfig_Array(conf);
}

TEST(Yato_Config, xml_example)
{
    const char* xml = R"XML(
        <root>
            <answer>42</answer>
            <comment>everything</comment>
            <precision>0.01</precision>
    
            <manual_mode>true</manual_mode>

            <fruits is_array="true">
                <i>apple</i>
                <i>banana</i>
                <i>kiwi</i>
            </fruits>

            <location>
                <x>174</x>
                <y>34</y>
            </location>
        </root>
    )XML";
    const auto conf = yato::conf::xml_builder().parse(xml);
    TestConfig_Example(conf);
}

TEST(Yato_Config, xml_conversion)
{
    const char* xml = R"XML(
        <root>
            <enum1>7</enum1>
            <enum2>14</enum2>
            <vec is_array="true"> 
                <v0>20</v0>
                <v1>98</v1>
                <v2>-7</v2>
            </vec>
        </root>
    )XML";
    const auto conf = yato::conf::xml_builder().parse(xml);
    TestConfig_Conversion(conf);
}

