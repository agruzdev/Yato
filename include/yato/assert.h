/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
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


namespace yato
{
    namespace details
    {
        using assert_callback_t = void (*)(bool condition, const char* message);

        inline
        void assert_default_callback(bool condition, const char* message) {
            YATO_MAYBE_UNUSED(condition);
            YATO_MAYBE_UNUSED(message);
            YATO_ASSERT(condition, message);
        }

        inline
        void assert_throwing_callback(bool condition, const char* message) {
            if(!condition) {
                throw yato::assertion_error(message);
            }
        }

        struct assert_callback_holder
        {
            static
            assert_callback_t& instance() {
                static assert_callback_t callback = &assert_default_callback;
                return callback;
            }
        };

        struct throwing_callback_lock
        {
            throwing_callback_lock() {
                assert_callback_holder::instance() = &assert_throwing_callback;
            }

            ~throwing_callback_lock() {
                assert_callback_holder::instance() = &assert_default_callback;
            }

            throwing_callback_lock(const throwing_callback_lock&) = delete;
            throwing_callback_lock& operator = (const throwing_callback_lock&) = delete;
            throwing_callback_lock(throwing_callback_lock&&) = delete;
            throwing_callback_lock& operator = (throwing_callback_lock&&) = delete;
        };
    }
}

/*
 * Define this flag in order to change testeed assertions to throw expressions in test cases.
 * Is correct only if tested function is defined as YATO_NOEXCEPT_TESTED. Using this macro for noexcept function is undefined behaviour.
 */
#ifdef YATO_ENABLE_TESTED_ASSERTIONS
# define YATO_NOEXCEPT_TESTED
# define YATO_REQUIRES_TESTED(Condition_) yato::details::assert_callback_holder::instance()(Condition_, "Precondition failure!")
# define YATO_ENSURES_TESTED(Condition_)  yato::details::assert_callback_holder::instance()(Condition_, "Postcondition failure!")
# define YATO_ASSERT_TESTED(Condition_, Message_) yato::details::assert_callback_holder::instance()(Condition_, Message_)
# define YATO_THROWING_ASSERT_LOCK(LockName_) yato::details::throwing_callback_lock LockName_{}
#else
# define YATO_NOEXCEPT_TESTED YATO_NOEXCEPT_KEYWORD
# define YATO_REQUIRES_TESTED(Condition_) YATO_REQUIRES(Condition_)
# define YATO_ENSURES_TESTED(Condition_) YATO_ENSURES(Condition_)
# define YATO_ASSERT_TESTED(Condition_, Message_) YATO_ASSERT(Condition_, Message_)
#endif

#endif

