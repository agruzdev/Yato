/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#ifndef _YATO_ASSERT_H_
#define _YATO_ASSERT_H_

#include <cassert>
#include <stdexcept>

#include "prerequisites.h"

namespace yato
{

    class runtime_error
        : public std::runtime_error
    {
    public:
        runtime_error(const std::string & message)
            : std::runtime_error(message)
        { }

        runtime_error(const char* message)
            : std::runtime_error(message)
        { }

        ~runtime_error()
        { }
    };

    class out_of_range_error 
        : public yato::runtime_error
    {
    public:
        out_of_range_error(const std::string & message)
            : yato::runtime_error(message)
        { }

        out_of_range_error(const char* message)
            : yato::runtime_error(message)
        { }

        ~out_of_range_error()
        { }
    };

    class argument_error 
        : public yato::runtime_error
    {
    public:
        argument_error(const std::string & message)
            : yato::runtime_error(message)
        { }

        argument_error(const char* message)
            : yato::runtime_error(message)
        { }

        ~argument_error()
        { }
    };


    class bad_state_error 
        : public yato::runtime_error
    {
    public:
        bad_state_error(const std::string & message)
            : yato::runtime_error(message)
        { }

        bad_state_error(const char* message)
            : yato::runtime_error(message)
        { }

        ~bad_state_error()
        { }
    };
}

#if YATO_DEBUG
#  define YATO_REQUIRES(Condition) assert((Condition) && "Precondition failure!");
#  define YATO_ENSURES(Condition)  assert((Condition) && "Postcondition failure!");
#  define YATO_ASSERT(Condition, Message) assert((Condition) && Message);
#else
#define YATO_REQUIRES(Condition) { }
#define YATO_ENSURES(Condition) { }
#define YATO_ASSERT(Condition, Message) { }
#endif

#endif