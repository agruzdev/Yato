/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ASSERT_H_
#define _YATO_ASSERT_H_

#include <stdexcept>

#include "prerequisites.h"

namespace yato
{

    class assertion_error : public std::runtime_error
    {
    public:
        assertion_error(const std::string & message) 
            : std::runtime_error(message)
        { }

        assertion_error(const char* message) 
            : std::runtime_error(message)
        { }

        ~assertion_error()
        { }
    };

    class runtime_error: public std::runtime_error
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

    class out_of_range_error : public std::runtime_error
    {
    public:
        out_of_range_error(const std::string & message)
            : std::runtime_error(message)
        { }

        out_of_range_error(const char* message)
            : std::runtime_error(message)
        { }

        ~out_of_range_error()
        { }
    };

}

#define YATO_THROW_ASSERT_EXCEPT(Message) throw yato::assertion_error(YATO_GET_FILE_LINE " " Message)

#if YATO_DEBUG
#define YATO_REQUIRES(Condition) if(!(Condition)) { YATO_THROW_ASSERT_EXCEPT("Precondition failure!"); }
#define YATO_ENSURES(Condition)  if(!(Condition)) { YATO_THROW_ASSERT_EXCEPT("Postcondition failure!"); }
#define YATO_ASSERT(Condition, Message) if(!(Condition)) { YATO_THROW_ASSERT_EXCEPT(Message); }
#else
#define YATO_REQUIRES(Condition) { }
#define YATO_ENSURES(Condition) { }
#define YATO_ASSERT(Condition, Message) { }
#endif

#endif