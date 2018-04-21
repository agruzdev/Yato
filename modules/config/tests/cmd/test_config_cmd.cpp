#include <vector>

#include "../include/test_config.h"

#include <yato/config/cmd/cmd_config.h>

TEST(Yato_Config, cmd_object)
{
    std::vector<std::string> args;
    args.emplace_back("TestApp");
    args.emplace_back("-i");
    args.emplace_back("42");
    args.emplace_back("--flt");
    args.emplace_back("7.0");
    args.emplace_back("--flag2");

    const yato::conf::config conf = yato::conf::cmd_builder("Test")
        .integer("i", "int", "test int value", yato::some(1))
        .floating("", "flt", "test float value")
        .string("m", "message", "test string", yato::some<std::string>("somestr"))
        .boolean("f", "flag1", "test flag 1")
        .boolean("", "flag2", "test flag 2")
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

    const yato::conf::config conf = yato::conf::cmd_builder("Test")
        .integer("", "answer", "integer argument with default value", yato::some(0))
        .string("c", "comment", "required string argument with one-letter alias")
        .floating("", "precision", "required floating-point argument")
        .boolean("", "manual_mode", "boolean flag")
        .parse(yato::make_view(args.data(), args.size()));

    TestConfig_Example(conf);
}

