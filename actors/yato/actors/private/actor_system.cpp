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
#include "pinned_executor.h"
#include "dynamic_executor.h"

namespace
{
    static std::string DEAD_LETTERS = "_dead_";
}

namespace yato
{
namespace actors
{


    struct actor_cell
    {
        std::unique_ptr<actor_base> act;
        mailbox mbox;
        //bool active;
    };
    //-------------------------------------------------------

    actor_system::actor_system(const std::string & name)
        : m_name(name)
        , m_dead_letters(this, "")
    {
        if(name.empty()) {
            throw yato::argument_error("System name can't be empty");
        }
        m_logger   = logger_factory::create(m_name);
        //m_executor = std::make_unique<pinned_executor>();
        m_executor = std::make_unique<dynamic_executor>(4, 5);
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
        if(name.empty()) {
            throw yato::argument_error("Actor name can't be empty!");
        }
        if(name == DEAD_LETTERS) {
            throw yato::argument_error("Actor name " + DEAD_LETTERS + "is reserved!");
        }
        if(m_contexts.find(name) != m_contexts.cend()) {
            throw yato::argument_error("Actor with the name " + name + " already exists!");
        }

        actor_ref ref{this, name};

        actor_cell* context = nullptr;
        {
            auto ctx = std::make_unique<actor_cell>();
            ctx->act = std::move(a);
            ctx->act->init_base(ref);
            ctx->mbox.owner = ctx->act.get();
            ctx->mbox.isOpen = true; // ToDo (a.gruzdev): Temporal solution.
            auto res = m_contexts.emplace(ref.get_path(), std::move(ctx));
            assert(res.second && "Failed to insert new actor context"); 

            context = &(*res.first->second);
        }

        return ref;
    }
    //-------------------------------------------------------

    void actor_system::send_impl(const actor_ref & toActor, const actor_ref & fromActor, yato::any && userMessage)
    {
        if(toActor.get_name() == DEAD_LETTERS) {
            m_logger->verbose("A message was delivered to deadLetters.");
            return;
        }
        auto it = m_contexts.find(toActor.get_path());
        if(it == m_contexts.end()) {
            throw yato::runtime_error("Actor " + toActor.get_path() + " is not found!");
        }

        auto & mbox = it->second->mbox;

        auto msg = std::make_unique<message>(std::move(userMessage), fromActor);
        {
            std::unique_lock<std::mutex> lock(mbox.mutex);
            if (it->second->mbox.isOpen) {
                mbox.queue.push(std::move(msg));
                mbox.condition.notify_one();
            }
        }
        m_executor->execute(&mbox);
    }
    //-------------------------------------------------------


} // namespace actors

} // namespace yato
