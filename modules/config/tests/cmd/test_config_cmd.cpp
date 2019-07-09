/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include <vector>

#include "../include/test_config.h"

#include <yato/config/cmd/cmd.h>

TEST(Yato_Config, cmd_object)
{
    std::vector<std::string> args;
    args.emplace_back("TestApp");
    args.emplace_back("-i");
    args.emplace_back("42");
    args.emplace_back("--flt");
    args.emplace_back("7.0");
    args.emplace_back("--flag2");

    using yato::conf::argument_type;
    const yato::conf::config conf = yato::conf::cmd_builder("Test")
        .integer(argument_type::optional,  "i", "int", "test int value", yato::some(1))
        .floating(argument_type::required, "",  "flt", "test float value")
        .string(argument_type::optional,   "m", "message", "test string", yato::some<std::string>("somestr"))
        .boolean("f", "flag1", "test flag 1")
        .boolean("",  "flag2", "test flag 2")
        .parse(yato::make_view(args.data(), args.size()));

    TestConfig_PlainObject(conf);
}

TEST(Yato_Config, cmd_object_2)
{
    std::vector<std::string> args;
    args.emplace_back("TestApp");
    args.emplace_back("--flt");
    args.emplace_back("7.0");
    args.emplace_back("--flag2");

    using yato::conf::argument_type;
    const yato::conf::config conf = yato::conf::cmd_builder("Test")
        .integer(argument_type::optional,  "i", "int", "test int value", yato::some(42))
        .floating(argument_type::required, "", "flt", "test float value")
        .string(argument_type::optional,   "m", "message", "test string", yato::some<std::string>("somestr"))
        .boolean("f", "flag1", "test flag 1")
        .boolean("",  "flag2", "test flag 2")
        .parse(yato::make_view(args.data(), args.size()));

    TestConfig_PlainObject(conf);
}

TEST(Yato_Config, cmd_example)
{
    // Arrays and nested configs are not supported by command line config;
    std::vector<std::string> args;
    args.emplace_back("TestApp");
    args.emplace_back("--answer");
    args.emplace_back("42");
    args.emplace_back("-c");
    args.emplace_back("everything");
    args.emplace_back("--precision");
    args.emplace_back("0.01");
    args.emplace_back("--manual_mode");

    using yato::conf::argument_type;
    const yato::conf::config conf = yato::conf::cmd_builder("Test")
        .integer(argument_type::optional,  "", "answer", "integer argument with default value", yato::some(0))
        .string(argument_type::required,   "c", "comment", "required string argument with one-letter alias")
        .floating(argument_type::required, "", "precision", "required floating-point argument")
        .boolean("", "manual_mode", "boolean flag")
        .parse(yato::make_view(args.data(), args.size()));

    TestConfig_Example(conf);
}

TEST(Yato_Config, cmd_conversion)
{
    std::vector<std::string> args;
    args.emplace_back("TestApp");
    args.emplace_back("--enum1");
    args.emplace_back("7");
    args.emplace_back("--enum2");
    args.emplace_back("14");

    using yato::conf::argument_type;
    const yato::conf::config conf = yato::conf::cmd_builder("Test")
        .integer(argument_type::optional, "", "enum1", "first", yato::some(0))
        .integer(argument_type::optional, "", "enum2", "second", yato::some(0))
        .parse(yato::make_view(args.data(), args.size()));

    TestConfig_Conversion(conf);
}

TEST(Yato_Config, cmd_conversion_2)
{
    std::vector<std::string> args;
    args.emplace_back("TestApp");
    args.emplace_back("7");
    args.emplace_back("14");

    using yato::conf::argument_type;
    const yato::conf::config conf = yato::conf::cmd_builder("Test")
        .integer(argument_type::positional, "", "enum1", "first")
        .integer(argument_type::positional, "", "enum2", "second")
        .parse(yato::make_view(args.data(), args.size()));

    TestConfig_Conversion(conf);
}
