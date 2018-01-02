/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_ASSKING_ACTOR_H_
#define _YATO_ACTORS_ASSKING_ACTOR_H_

#include <future>

#include "../actor.h"

namespace yato
{
namespace actors
{

    class asking_actor
        : public yato::actors::actor<>
    {
        std::promise<yato::any> m_promise;

        void receive(yato::any & message) override
        {
            m_promise.set_value(message);
            self().stop();
        }

    public:
        asking_actor(std::promise<yato::any> && promise)
            : m_promise(std::move(promise))
        { }
    };


}// namespace actors

}// namespace yato

#endif //_YATO_ACTORS_ASSKING_ACTOR_H_

