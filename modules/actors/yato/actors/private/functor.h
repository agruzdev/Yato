/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_FUNCTOR_H_
#define _YATO_ACTORS_FUNCTOR_H_

#include <memory>

namespace yato
{
namespace actors
{
    /**
     * Wrapper for non-null callable object
     */
    class void_functor
    {
    public:
        virtual ~void_functor() = default;

        virtual void operator()() = 0;
    };

    template <typename Fn_>
    class typed_functor
        : public void_functor
    {
        Fn_ m_functor;

    public:
        explicit constexpr
        typed_functor(const Fn_ & f)
            : m_functor(f)
        { }

        explicit constexpr
        typed_functor(Fn_ && f)
            : m_functor(std::move(f))
        { }

        ~typed_functor() = default;

        typed_functor(const typed_functor&) = delete;
        typed_functor(typed_functor&&) = delete;

        typed_functor& operator= (const typed_functor&) = delete;
        typed_functor& operator= (typed_functor&&) = delete;

        void operator()() override {
            m_functor();
        }

        const Fn_ & stored_functor() const {
            return m_functor;
        }

        Fn_ & stored_functor() {
            return m_functor;
        }
    };

    template <typename Fn_>
    inline
    typed_functor<Fn_> make_functor(Fn_ && f) {
        return typed_functor<Fn_>(std::forward<Fn_>(f));
    }

    template <typename Fn_>
    inline
    std::unique_ptr<typed_functor<Fn_>> make_functor_ptr(Fn_ && f) {
        return std::make_unique<typed_functor<Fn_>>(std::forward<Fn_>(f));
    }

}// namespace actors

}// namespace yato

#endif //_YATO_ACTORS_FUNCTOR_H_

