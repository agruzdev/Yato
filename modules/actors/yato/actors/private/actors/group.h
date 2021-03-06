/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_PRIVATE_ACTORS_GROUP_H_
#define _YATO_ACTORS_PRIVATE_ACTORS_GROUP_H_

#include "../../actor.h"

namespace yato
{
namespace actors
{
    /**
     * Group actor for using as a root of subtree
     */
    class group
        : public actor
    {
        void receive(yato::any &&) override
        { }
    };

} // namespace actors

} // namespace yato

#endif //_YATO_ACTORS_PRIVATE_ACTORS_GROUP_H_

