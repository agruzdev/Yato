/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_PRIVATE_ACTORS_ASKING_ACTOR_H_
#define _YATO_ACTORS_PRIVATE_ACTORS_ASKING_ACTOR_H_

#include <future>

#include "../../actor.h"

namespace yato
{
namespace actors
{

    class asking_actor
        : public actor
    {
        std::promise<yato::any> m_promise;
        bool m_satisfied;

        void receive(yato::any && message) override
        {
            if(!m_satisfied) {
                m_promise.set_value(message);
                m_satisfied = true;
                self().stop();
            }
        }

        void post_stop() override
        {
            if(!m_satisfied) {
                m_promise.set_value(yato::nullany_t{});
                m_satisfied = true;
            }
        }

    public:
        asking_actor(std::promise<yato::any> && promise)
            : m_promise(std::move(promise)), m_satisfied(false)
        { }
    };


}// namespace actors

}// namespace yato

#endif //_YATO_ACTORS_PRIVATE_ACTORS_ASKING_ACTOR_H_

