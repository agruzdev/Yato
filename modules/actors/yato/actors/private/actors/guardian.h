/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_PRIVATE_ACTORS_GUARDIAN_H_
#define _YATO_ACTORS_PRIVATE_ACTORS_GUARDIAN_H_

#include "../../actor.h"

namespace yato
{
namespace actors
{
    /**
     * Guardian actor
     * Root for system, user, temp and remote subtrees
     */
    class guardian
        : public actor
    {
        void receive(yato::any &&) override
        { }
    };

} // namespace actors

} // namespace yato

#endif //_YATO_ACTORS_PRIVATE_ACTORS_GUARDIAN_H_

