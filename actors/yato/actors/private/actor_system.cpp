/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <iostream>

#include "../actor.h"
#include "../actor_system.h"

#include "mailbox.h"
#include "thread_pool.h"


namespace yato
{
namespace actors
{

    struct actor_context
    {
        std::unique_ptr<actor_base> act;
        mailbox mbox;
        //bool active;
    };
    //-------------------------------------------------------

    actor_system::actor_system(const std::string & name) 
        : m_name(name)
    {
        m_executor = std::make_unique<pinned_thread_pool>();
    }
    //-------------------------------------------------------

    actor_system::~actor_system() 
    {
        for(auto & entry : m_contexts) {
            std::unique_lock<std::mutex> lock(entry.second->mbox.mutex);
            entry.second->mbox.isOpen = false;
            entry.second->mbox.condition.notify_one();
        }
        m_executor.reset();
    }
    //-------------------------------------------------------

    actor_ref actor_system::create_actor_impl(std::unique_ptr<actor_base> && a, const std::string & name)
    {
        if(m_contexts.find(name) != m_contexts.cend()) {
            throw yato::runtime_error("Actor with the name " + name + " already exists!");
        }

        actor_ref ref;
        ref.name = name;
        ref.path = m_name + "/" + name;

        actor_context* context = nullptr;
        {
            auto ctx = std::make_unique<actor_context>();
            ctx->act = std::move(a);
            ctx->act->set_name(name);
            ctx->mbox.owner = ctx->act.get();
            auto res = m_contexts.emplace(ref.path, std::move(ctx));
            assert(res.second && "Failed to insert new actor context"); 

            context = &(*res.first->second);
        }

        m_executor->execute(&context->mbox);

        return ref;
    }
    //-------------------------------------------------------

    void actor_system::tell_impl(const actor_ref & toActor, yato::any && userMessage)
    {
        auto it = m_contexts.find(toActor.path);
        if(it == m_contexts.end()) {
            throw yato::runtime_error("Actor " + toActor.path + " is not found!");
        }

        auto & mbox = it->second->mbox;

        auto msg = std::make_unique<message>();
        msg->payload = std::move(userMessage);

        {
            std::unique_lock<std::mutex> lock(mbox.mutex);
            if (it->second->mbox.isOpen) {
                mbox.queue.push(std::move(msg));
                mbox.condition.notify_one();
            }
        }
    }
    //-------------------------------------------------------

} // namespace actors

} // namespace yato
