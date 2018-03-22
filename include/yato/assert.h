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

        ~runtime_error() = default;
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

        ~out_of_range_error() = default;
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

        ~argument_error() = default;
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

        ~bad_state_error() = default;
    };

    /**
     * Exception class for testing.
     * Do not use this exception class for other logic.
     */
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

        ~assertion_error() = default;
    };

}

#define YATO_ASSERT_IMPL_(Condition_, Message_) assert((Condition_) && Message_);
#define YATO_THROW_ASSERT_IMPL_(Condition_, Message_) { if(!(Condition_)) throw yato::assertion_error(Message_); }

#if YATO_DEBUG
# define YATO_REQUIRES(Condition_) YATO_ASSERT_IMPL_(Condition_, "Precondition failure!")
# define YATO_ENSURES(Condition_)  YATO_ASSERT_IMPL_(Condition_, "Postcondition failure!")
# define YATO_ASSERT(Condition_, Message_) YATO_ASSERT_IMPL_(Condition_, Message_)
#else
# define YATO_REQUIRES(Condition_) { }
# define YATO_ENSURES(Condition_) { }
# define YATO_ASSERT(Condition_, Message_) { }
#endif

/**
 * Define this flag in order to change testeed assertions to throw expressions in test cases.
 * Is correct only if tested function is defined as YATO_NOEXCEPT_TESTED. Using this macro for noexcept function is undefined behaviour.
 */
#ifdef YATO_THROW_ON_TESTED_ASSERTIONS
# define YATO_NOEXCEPT_TESTED
# define YATO_REQUIRES_TESTED(Condition_) YATO_THROW_ASSERT_IMPL_(Condition_, "Precondition failure!")
# define YATO_ENSURES_TESTED(Condition_)  YATO_THROW_ASSERT_IMPL_(Condition_, "Postcondition failure!")
# define YATO_ASSERT_TESTED(Condition_, Message_) YATO_THROW_ASSERT_IMPL_(Condition_, Message_)
#else
# define YATO_NOEXCEPT_TESTED YATO_NOEXCEPT_KEYWORD
# define YATO_REQUIRES_TESTED(Condition_) YATO_REQUIRES(Condition_)
# define YATO_ENSURES_TESTED(Condition_) YATO_ENSURES(Condition_)
# define YATO_ASSERT_TESTED(Condition_, Message_) YATO_ASSERT(Condition_, Message_)
#endif

#endif

