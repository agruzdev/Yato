/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <iostream>

#include "../actor.h"
#include "../actor_system.h"

#include "asking_actor.h"
#include "mailbox.h"
#include "pinned_executor.h"
#include "dynamic_executor.h"
#include "scheduler.h"
#include "name_generator.h"

namespace
{
    const std::string DEAD_LETTERS = "_dead_";
    const std::string POISON_PILL  = "_poison_";
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
    {
        if(!actor_path::is_valid_system_name(m_name)) {
            throw yato::argument_error("actor_system[actor_system]: Invalid name!");
        }
        m_dead_letters.reset(new actor_ref(this, actor_path(this, actor_scope::dead, DEAD_LETTERS)));

        m_logger = logger_factory::create(std::string("ActorSystem[") + m_name + "]");
        m_logger->set_filter(log_level::verbose);

        //m_executor = std::make_unique<pinned_executor>(this);
        m_executor = std::make_unique<dynamic_executor>(this, 4, 5);

        m_scheduler = std::make_unique<scheduler>();
        m_name_generator = std::make_unique<name_generator>();
    }
    //-------------------------------------------------------

    actor_system::~actor_system() 
    {
        {
            std::unique_lock<std::mutex> lock(m_cells_mutex);
            while(!m_actors.empty()) {
                m_cells_condition.wait(lock);
            }
        }
        // Now all actors are stopped

        m_scheduler->stop();
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

    actor_ref actor_system::create_actor_impl(std::unique_ptr<actor_base> && a, const actor_path & path)
    {
        // create mailbox
        auto mbox = std::make_shared<mailbox>();
        mbox->owner = a.get();

        // create ref
        actor_ref ref{ this, path };
        ref.set_mailbox(mbox);

        // setup actor
        a->init_base_(this, ref);

        auto cell = std::make_unique<actor_cell>();
        cell->act = std::move(a);
        cell->mbox = mbox;

        {
            std::unique_lock<std::mutex> lock(m_cells_mutex);
            auto pos = m_actors.find(ref.get_path());
            if(pos != m_actors.end()) {
                throw yato::argument_error("Actor with the name " + path.to_string() + " already exists!");
            }
            m_actors.emplace_hint(pos, ref.get_path(), std::move(cell));
        }
        m_logger->verbose("Actor %s is started!", ref.get_path().c_str());

        enqueue_system_signal(mbox.get(), system_signal::start);
        m_executor->execute(mbox.get());

        return ref;
    }
    //-------------------------------------------------------

    void actor_system::send_impl(const actor_ref & addressee, const actor_ref & sender, yato::any && userMessage) const
    {
        if(addressee == dead_letters()) {
            m_logger->verbose("A message was delivered to DeadLetters.");
            return;
        }

        std::shared_ptr<mailbox> mbox = addressee.get_mailbox().lock();
        if(mbox == nullptr) {
            m_logger->verbose("Failed to send a message. Actor %s is not found!", addressee.get_path().c_str());
            return;
        }

        auto msg = std::make_unique<message>(std::move(userMessage), sender);
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

    std::future<yato::any> actor_system::ask_impl_(const actor_ref & addressee, yato::any && message, const timeout_type & timeout) const
    {
        std::promise<yato::any> response;
        auto result = response.get_future();

        auto ask_actor_path = actor_path(this, actor_scope::temp, m_name_generator->next_indexed("ask"));
        auto ask_actor = const_cast<actor_system*>(this)->create_actor_impl(std::make_unique<asking_actor>(std::move(response)), ask_actor_path);
        send_impl(addressee, ask_actor, std::move(message));

        m_scheduler->enqueue(std::chrono::high_resolution_clock::now() + timeout, [ask_actor]{ ask_actor.stop(); });

        return result;
    }
    //-------------------------------------------------------

    void actor_system::stop_impl_(mailbox* mbox) const 
    {
        assert(mbox != nullptr);
        bool execute = false;
        {
            std::unique_lock<std::mutex> lock(mbox->mutex);
            if (mbox->is_open) {
                mbox->sys_queue.push(system_signal::stop);
                mbox->condition.notify_one();
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
            m_logger->verbose("Failed to stop actor. Actor %s is not found!", addressee.get_path().c_str());
            return;
        }
        stop_impl_(mbox.get());
    }
    //--------------------------------------------------------

    void actor_system::stop_all()
    {
        std::unique_lock<std::mutex> lock(m_cells_mutex);
        std::for_each(m_actors.begin(), m_actors.end(), [this](decltype(m_actors)::value_type & entry) {
            stop_impl_(entry.second->mbox.get());
        });
    }
    //--------------------------------------------------------

    actor_ref actor_system::select(const actor_path & path) const
    {
        std::unique_lock<std::mutex> lock(m_cells_mutex);
        auto it = m_actors.find(path);
        if(it != m_actors.cend()) {
            return (*it).second->act->self();
        }
        return dead_letters();
    }

    //--------------------------------------------------------

    void actor_system::notify_on_stop_(const actor_ref & ref)
    {
        {
            std::unique_lock<std::mutex> lock(m_cells_mutex);
            auto it = m_actors.find(ref.get_path());
            if (it != m_actors.end()) {
                m_actors.erase(it);
            }
            m_cells_condition.notify_one();
        }
        m_logger->verbose("Actor %s is stopped.", ref.get_path().c_str());
    }
    //--------------------------------------------------------


} // namespace actors

} // namespace yato
