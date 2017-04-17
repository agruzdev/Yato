/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
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

    class assertion_error 
        : public yato::runtime_error
    {
    public:
        assertion_error(const std::string & message)
            : yato::runtime_error(message)
        { }

        assertion_error(const char* message)
            : yato::runtime_error(message)
        { }

        ~assertion_error()
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

#define YATO_THROW_ASSERT_EXCEPT(Message) throw yato::assertion_error(YATO_GET_FILE_LINE " " Message)

/**
 * By default assert() is used
 * define macro YATO_THROW_ON_ASSERT to enable throwing exceptions on assertation failure  
 */

#if YATO_DEBUG
# ifdef YATO_THROW_ON_ASSERT
#  define YATO_REQUIRES(Condition) if(!(Condition)) { YATO_THROW_ASSERT_EXCEPT("Precondition failure!"); }
#  define YATO_ENSURES(Condition)  if(!(Condition)) { YATO_THROW_ASSERT_EXCEPT("Postcondition failure!"); }
#  define YATO_ASSERT(Condition, Message) if(!(Condition)) { YATO_THROW_ASSERT_EXCEPT(Message); }
# else
#  define YATO_REQUIRES(Condition) assert((Condition) && "Precondition failure!");
#  define YATO_ENSURES(Condition)  assert((Condition) && "Postcondition failure!");
#  define YATO_ASSERT(Condition, Message) assert((Condition) && Message);
# endif
#else
#define YATO_REQUIRES(Condition) { }
#define YATO_ENSURES(Condition) { }
#define YATO_ASSERT(Condition, Message) { }
#endif

#endif