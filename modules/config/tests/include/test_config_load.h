/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_TEST_CONFIG_LOAD_H_
#define _YATO_CONFIG_TEST_CONFIG_LOAD_H_

#include "gtest/gtest.h"

#include <algorithm>
#include <cctype>
#include <string>

inline
void TestConfig_CompareConfigs(std::string first, std::string second)
{
    const auto first_end  = std::remove_if(first.begin(),  first.end(),  std::isspace);
    const auto second_end = std::remove_if(second.begin(), second.end(), std::isspace);
    EXPECT_EQ(std::string(first.begin(), first_end), std::string(second.begin(), second_end));
}

#endif // _YATO_CONFIG_TEST_CONFIG_LOAD_H_
