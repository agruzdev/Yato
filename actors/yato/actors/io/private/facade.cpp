/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/


#include "../facade.h"

#include "../../actor_system.h"
#include "../../private/actor_system_ex.h"
#include "../tcp.h"
#include "../udp.h"
#include "context.h"

namespace yato
{
namespace actors
{
namespace io
{

    void facade::init(actor_system & sys) {
        auto ctx = std::make_shared<io_context>();
        actor_system_ex::create_actor<io::tcp_manager>(sys, actor_scope::system, io::tcp_manager::actor_name(), ctx);
        actor_system_ex::create_actor<io::udp_manager>(sys, actor_scope::system, io::udp_manager::actor_name(), ctx);
    }

} // namespace io

} // namespace actors

} // namespace yato



