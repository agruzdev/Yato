/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "../include/test_config.h"
#include <yato/config/xml/xml_config.h>

TEST(Yato_Config, xml_object_attr)
{
    const auto conf = yato::conf::xml_builder().parse(R"XML(
        <root>
            <int value="42"/>
            <message value="somestr"/>
            <flt value="7.0"/>
            <flag1 value="false"/>
            <flag2 value="true"/>
        </root>
    )XML");
    TestConfig_PlainObject(conf);
}

TEST(Yato_Config, xml_object_mix)
{
    const auto conf = yato::conf::xml_builder().parse(R"XML(
        <root>
            <int value="42"/>
            <message>somestr</message>
            <flt value="7.0"/>
            <flag1 value="false"/>
            <flag2>true</flag2>
        </root>
    )XML");
    TestConfig_PlainObject(conf);
}

TEST(Yato_Config, xml_object2_attr)
{
    const auto conf = yato::conf::xml_builder().parse(R"XML(
        <root>
            <int value="42"/>
            <str value="test"/>
            <subobj>
                <val value="7.0"/>
            </subobj>
        </root>
    )XML");
    TestConfig_Object(conf);
}

TEST(Yato_Config, xml_array_attr)
{
    const auto conf = yato::conf::xml_builder().parse(R"XML(
        <root is_array="true">
            <item value="10"/>
            <item value="20"/>
            <item value="30"/>
            <item value="true"/>
            <item value="4"/>
            <item>
                <arr is_array="true"/>
            </item>
        </root>
    )XML");
    TestConfig_Array(conf);
}

TEST(Yato_Config, xml_example_attr)
{
    const char* xml = R"XML(
        <root>
            <answer value="42"/>
            <comment value="everything"/>
            <precision value="0.01"/>
    
            <manual_mode value="true"/>

            <fruits is_array="true">
                <i value="apple"/>
                <i value="banana"/>
                <i value="kiwi"/>
            </fruits>

            <location>
                <x value="174"/>
                <y value="34"/>
            </location>
        </root>
    )XML";
    const auto conf = yato::conf::xml_builder().parse(xml);
    TestConfig_Example(conf);
}

TEST(Yato_Config, xml_conversion_attr)
{
    const char* xml = R"XML(
        <root>
            <enum1 value="7"/>
            <enum2 value="14"/>
            <vec is_array="true"> 
                <v0 value="20"/>
                <v1 value="98"/>
                <v2 value="-7"/>
            </vec>
        </root>
    )XML";
    const auto conf = yato::conf::xml_builder().parse(xml);
    TestConfig_Conversion(conf);
}

