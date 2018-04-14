#include "../include/test_config.h"

#include <yato/vector_nd.h>
#include <yato/config/cmd/cmd_config.h>

TEST(Yato_Config, cmd_object)
{
    yato::vector_1d<std::string> args;
    args.push_back("TestApp");
    args.push_back("-i");
    args.push_back("42");
    args.push_back("--flt");
    args.push_back("7.0");
    args.push_back("--flag2");

    const yato::conf::config_ptr conf = yato::conf::cmd_builder("Test")
        .integer("i", "int", "test int value", yato::some(1))
        .floating("", "flt", "test float value")
        .string("m", "message", "test string", yato::some<std::string>("somestr"))
        .boolean("f", "flag1", "test flag 1")
        .boolean("", "flag2", "test flag 2")
        .parse(yato::make_view(args.data(), args.size(0)));

    TestConfig_PlainObject(conf);
}


