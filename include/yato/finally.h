/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2019 Alexey Gruzdev
*/

#ifndef _YATO_FINALLY_H_
#define _YATO_FINALLY_H_

#include "prerequisites.h"

namespace yato
{

    template <typename ActionTy_>
    class finally
    {
    private:
        ActionTy_ m_action;
        bool m_invoke = true;

    public:
        /**/
        finally(ActionTy_ action)
            : m_action(action)
        { }

        ~finally()
        {
            if (m_invoke) {
                try {
                    m_action();
                }
                catch (...) {
                }
            }
        }

        finally(const finally&) = delete;

        finally(finally&& other) noexcept
            : m_action(std::move(other.m_action))
        {
            other.m_invoke = false;
        }

        finally& operator=(const finally&) = delete;

        finally& operator=(finally && other) noexcept
        {
            m_action = std::move(other.m_action);
            other.m_invoke = false;
        }
    };


    template <typename ActionTy_>
    auto make_finally(const ActionTy_ & a)
    {
        return finally<ActionTy_>(a);
    }

    template <typename ActionTy_>
    auto make_finally(ActionTy_ && a)
    {
        return finally<ActionTy_>(std::move(a));
    }

}

#define yato_finally(Action_) const auto YATO_UNIQUE_NAME(_final_action) = yato::make_finally(Action_);

#endif //_YATO_FINALLY_H_
