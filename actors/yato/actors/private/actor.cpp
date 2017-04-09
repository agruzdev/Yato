/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <iostream>

#include "../actor.h"
#include "../message.h"

namespace yato
{
namespace actors
{


    void actor_base::receive_message(const message & message) noexcept
    {
        try {
            unwrap_message(message);
        }
        catch (...) {
            // ToDo (a.gruzdev): Add logging
            std::cout << "Fail!" << std::endl;
        }
    }
    //-------------------------------------------------------



} // namespace actors

} // namespace yato
