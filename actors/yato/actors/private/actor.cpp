/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include "../actor.h"
#include "../message.h"
#include "../logger.h"

namespace yato
{
namespace actors
{


    void actor_base::receive_message(const message & message) noexcept
    {
        try {
            unwrap_message(message);
        }
        catch(std::exception & e) {
            logger::instance().error("actor_base[receive_message]: Unhandled exception: %s", e.what());
        }
        catch (...) {
            logger::instance().error("actor_base[receive_message]: Unknown exception!");
        }
    }
    //-------------------------------------------------------



} // namespace actors

} // namespace yato
