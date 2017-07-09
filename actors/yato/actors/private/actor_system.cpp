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
    static std::string POISON_PILL  = "_poison_";
}

namespace yato
{
namespace actors
{


    struct actor_cell
    {
        std::unique_ptr<actor_base> act;
        std::shared_ptr<mailbox> mbox;
    };
    //-------------------------------------------------------

    actor_system::actor_system(const std::string & name)
        : m_name(name)
        , m_actors_num(0)
        , m_dead_letters(this, DEAD_LETTERS)
    {
        if(name.empty()) {
            throw yato::argument_error("System name can't be empty");
        }
        m_logger = logger_factory::create(std::string("ActorSystem[") + m_name + "]");
        m_logger->set_filter(log_level::verbose);
        //m_executor = std::make_unique<pinned_executor>();
        m_executor = std::make_unique<dynamic_executor>(4, 5);
    }
    //-------------------------------------------------------

    actor_system::~actor_system() 
    {
        {
            std::unique_lock<std::mutex> lock(m_actors_mutex);
            m_actors_cv.wait(lock, [this]() { return m_actors_num == 0; });
        }
        m_executor.reset();
    }
    //-------------------------------------------------------

    void actor_system::enqueue_system_signal(mailbox* mbox, const system_signal & signal)
    {
        assert(mbox != nullptr);

        std::unique_lock<std::mutex> lock(mbox->mutex);

        mbox->sys_queue.push(signal);
        mbox->condition.notify_one();
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

            // setup mailbox
            ctx->mbox = std::make_shared<mailbox>();
            ctx->mbox->owner = a.get();
            ctx->mbox->is_open = true; // ToDo (a.gruzdev): Temporal solution.
            ref.set_mailbox(ctx->mbox);

            // setup actor
            a->init_base_(this, ref);
            ctx->act = std::move(a);

            auto res = m_contexts.emplace(ref.get_path(), std::move(ctx));
            assert(res.second && "Failed to insert new actor context"); 

            context = &(*res.first->second);
        }
        {
            std::unique_lock<std::mutex> lock(m_actors_mutex);
            ++m_actors_num;
        }

        enqueue_system_signal(context->mbox.get(), system_signal::start);
        m_executor->execute(context->mbox.get());

        return ref;
    }
    //-------------------------------------------------------

    void actor_system::send_impl(const actor_ref & to_actor, const actor_ref & fromActor, yato::any && userMessage) const 
    {
        if(to_actor == dead_letters()) {
            m_logger->verbose("A message was delivered to DeadLetters.");
            return;
        }

        std::shared_ptr<mailbox> mbox = to_actor.get_mailbox().lock();
        if(mbox == nullptr) {
            m_logger->verbose("Actor " + to_actor.get_path() + " is not found!");
            return;
        }

        auto msg = std::make_unique<message>(std::move(userMessage), fromActor);
        {
            std::unique_lock<std::mutex> lock(mbox->mutex);
            if (mbox->is_open) {
                mbox->queue.push(std::move(msg));
                mbox->condition.notify_one();
            }
        }
        m_executor->execute(mbox.get());
    }
    //-------------------------------------------------------

    void actor_system::stop_impl_(mailbox* mbox) const 
    {
        assert(mbox != nullptr);
        bool execute = false;
        {
            std::unique_lock<std::mutex> lock(mbox->mutex);
            if (mbox->is_open) {
                mbox->is_open = false;
                mbox->sys_queue.push(system_signal::stop);
                execute = true;
            }
        }
        if (execute) {
            m_executor->execute(mbox);
        }
    }
    //-------------------------------------------------------

    void actor_system::stop(const actor_ref & addressee) const 
    {
        std::shared_ptr<mailbox> mbox = addressee.get_mailbox().lock();
        if (mbox == nullptr) {
            m_logger->verbose("Actor " + addressee.get_path() + " is not found!");
            return;
        }
        stop_impl_(mbox.get());
    }
    //--------------------------------------------------------

    void actor_system::stop_all()
    {
        std::for_each(m_contexts.begin(), m_contexts.end(), [this](decltype(m_contexts)::value_type & entry) {
            stop_impl_(entry.second->mbox.get());
        });
    }
    //--------------------------------------------------------

    void actor_system::notify_on_stop_() 
    {
        {
            std::unique_lock<std::mutex> lock(m_actors_mutex);
            assert(m_actors_num > 0);
            if(--m_actors_num == 0) {
                m_actors_cv.notify_one();
            }
        }
    }


} // namespace actors

} // namespace yato
