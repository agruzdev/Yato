/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <atomic>
#include <iostream>

#include <yato/assert.h>
#include <yato/any_match.h>

#include "../actor.h"
#include "../actor_system.h"

#include "actor_system_ex.h"
#include "actor_cell.h"
#include "mailbox.h"
#include "pinned_executor.h"
#include "dynamic_executor.h"
#include "scheduler.h"
#include "name_generator.h"
#include "system_message.h"

#ifdef YATO_ACTORS_WITH_IO
#include "../io/tcp.h"
#endif

#include "actors/root.h"
#include "actors/selector.h"
#include "actors/asking_actor.h"

namespace
{
    const std::string DEAD_LETTERS = "_dead_";
    const std::string POISON_PILL  = "_poison_";
}


namespace yato
{
namespace actors
{

    actor_system::actor_system(const std::string & name, const config & conf)
        : m_name(name)
    {
        if(!actor_path::is_valid_system_name(m_name)) {
            throw yato::argument_error("actor_system[actor_system]: Invalid name!");
        }
        m_dead_letters.reset(new actor_ref(this, actor_path(m_name, actor_scope::dead, DEAD_LETTERS)));

        m_logger = logger_factory::create(std::string("ActorSystem[") + m_name + "]");
        m_logger->set_filter(conf.log_filter);

        //m_executor = std::make_unique<pinned_executor>(this);
        m_executor = std::make_unique<dynamic_executor>(this, 4, 5);

        m_scheduler = std::make_unique<scheduler>();
        m_name_generator = std::make_unique<name_generator>();

        m_root = std::make_unique<actor_cell>(*this, actor_path("yato://" + m_name), std::make_unique<actors::root>());
        m_root_stopped = false;

        // System actors
        if(conf.enable_io) {
#ifdef YATO_ACTORS_WITH_IO
            create_actor_impl_(make_builder<io::tcp_manager>(), actor_path(m_name, actor_scope::system, io::tcp_manager::actor_name()));
#else
            throw yato::argument_error("actor_system[actor_system]: IO can't be enabled. Build with flag YATO_ACTORS_WITH_IO");
#endif
        }
        YATO_ENSURES(m_root != nullptr);
        send_system_message(m_root->ref(), system_message::start());
    }
    //-------------------------------------------------------

    actor_system::actor_system(const std::string & name)
        : actor_system(name, config::defaults())
    { }
    //-------------------------------------------------------

    actor_system::~actor_system()
    {
        send_message(m_root->ref(), root_terminate{});

        {
            std::unique_lock<std::mutex> lock(m_terminate_mutex);
            m_terminate_cv.wait(lock, [this]() { return m_root_stopped; });
        }
        // Now all actors are stopped

        //m_scheduler->stop();
    }
    //-------------------------------------------------------

    const actor_ref & actor_system::root() const {
        YATO_REQUIRES(m_root != nullptr);
        return m_root->ref();
    }
    //-------------------------------------------------------

    template <typename Ty_, typename ... Args_>
    inline
    bool enqueue_system_message_(const std::shared_ptr<mailbox> & mbox, const actor_ref & sender, Args_ && ... args)
    {
        assert(mbox != nullptr);
        bool success = false;
        auto payload = yato::any(yato::in_place_type_t<Ty_>{}, std::forward<Args_>(args)...);
        {
            std::unique_lock<std::mutex> lock(mbox->mutex);
            if (mbox->is_open) {
                mbox->sys_queue.push(std::make_unique<message>(std::move(payload), sender));
                mbox->condition.notify_one();
                success = true;
            }
        }
        return success;
    }
    //-------------------------------------------------------

    actor_ref actor_system::create_actor_impl_(const details::cell_builder & builder, const actor_path & path, const actor_ref & parent)
    {
        path_elements elems;
        path.parce(elems);

        if(elems.scope == actor_scope::unknown) {
            throw yato::argument_error("Invalid actor path!");
        }

        auto cell = builder(*this, path);
        auto ref  = cell->ref();

        if(parent.empty()) {
            send_message(m_root->ref(), root_add(std::move(cell)));
        } else {
            send_system_message(parent, system_message::attach_child(std::move(cell)));
        }

        return ref;
    }
    //-------------------------------------------------------

    void actor_system::send_impl_(const actor_ref & addressee, const actor_ref & sender, yato::any && userMessage) const
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
        m_executor->execute(mbox);
    }
    //-------------------------------------------------------

    void actor_system::send_system_impl_(const actor_ref & addressee, const actor_ref & sender, yato::any && userMessage) const
    {
        if (addressee == dead_letters()) {
            m_logger->verbose("A message was delivered to DeadLetters.");
            return;
        }

        std::shared_ptr<mailbox> mbox = addressee.get_mailbox().lock();
        if (mbox == nullptr) {
            m_logger->verbose("Failed to send a message. Actor %s is not found!", addressee.get_path().c_str());
            return;
        }

        auto msg = std::make_unique<message>(std::move(userMessage), sender);
        {
            std::unique_lock<std::mutex> lock(mbox->mutex);
            if (mbox->is_open) {
                mbox->sys_queue.push(std::move(msg));
                mbox->condition.notify_one();
            }
        }
        m_executor->execute(mbox);
    }
    //-------------------------------------------------------

    std::future<yato::any> actor_system::ask_impl_(const actor_ref & addressee, yato::any && message, const timeout_type & timeout) const
    {
        std::promise<yato::any> response;
        auto result = response.get_future();

        const auto ask_actor_path = actor_path(*this, actor_scope::temp, m_name_generator->next_indexed("ask"));
        const auto ask_actor = const_cast<actor_system*>(this)->create_actor_impl_(details::make_cell_builder<asking_actor>(std::move(response)), ask_actor_path, actor_ref{});
        send_impl_(addressee, ask_actor, std::move(message));

        m_scheduler->enqueue(std::chrono::high_resolution_clock::now() + timeout, [ask_actor]{ ask_actor.stop(); });

        return result;
    }
    //-------------------------------------------------------

    void actor_system::stop_impl_(const std::shared_ptr<mailbox> & mbox) const 
    {
        assert(mbox != nullptr);
        if(enqueue_system_message_<system_message::stop>(mbox, dead_letters())) {
            m_executor->execute(mbox);
        }
    }
    //-------------------------------------------------------

    void actor_system::stop(const actor_ref & addressee) const 
    {
        const std::shared_ptr<mailbox> mbox = addressee.get_mailbox().lock();
        if (mbox == nullptr) {
            m_logger->verbose("Failed to stop actor. Actor %s is not found!", addressee.get_path().c_str());
            return;
        }
        stop_impl_(mbox);
    }
    //--------------------------------------------------------

    void actor_system::stop_all()
    {
        throw std::logic_error("To be implemented");
        //std::unique_lock<std::mutex> lock(m_cells_mutex);
        //std::for_each(m_actors.begin(), m_actors.end(), [this](decltype(m_actors)::value_type & entry) {
        //    stop_impl_(entry.second->mail());
        //});
    }
    //--------------------------------------------------------

    std::future<actor_ref> actor_system::find_impl_(const actor_path & path, const timeout_type & timeout) const
    {
        std::promise<actor_ref> promise;
        auto result = promise.get_future();

        const auto selector_path = actor_path(*this, actor_scope::temp, m_name_generator->next_indexed("find"));
        const auto select_actor  = const_cast<actor_system*>(this)->create_actor_impl_(details::make_cell_builder<selector>(path, std::move(promise)), selector_path, actor_ref{});

        m_scheduler->enqueue(std::chrono::high_resolution_clock::now() + timeout, [select_actor]{ select_actor.stop(); });

        return result;
    }
    //--------------------------------------------------------

    void actor_system::watch(const actor_ref & watchee, const actor_ref & watcher) const
    {
        if(watchee == dead_letters() || watcher == dead_letters()) {
            m_logger->error("DeadLetters can't be used in watching");
            return;
        }
        const std::shared_ptr<mailbox> mbox = watchee.get_mailbox().lock();
        if (mbox == nullptr) {
            m_logger->warning("Failed to find watchee. Actor %s is not found!", watchee.get_path().c_str());
            watcher.tell(terminated(watchee)); // Already terminated
            return;
        }
        if (enqueue_system_message_<system_message::watch>(mbox, watcher, watcher)) {
            m_executor->execute(mbox);
        }
    }
    //--------------------------------------------------------
    
    void actor_system::unwatch(const actor_ref & watchee, const actor_ref & watcher) const
    {
        if (watchee == dead_letters() || watcher == dead_letters()) {
            m_logger->error("DeadLetters can't be used in watching");
            return;
        }
        const std::shared_ptr<mailbox> mbox = watchee.get_mailbox().lock();
        if (mbox == nullptr) {
            m_logger->error("Failed to find watchee. Actor %s is not found!", watchee.get_path().c_str());
            return;
        }
        if (enqueue_system_message_<system_message::unwatch>(mbox, watcher, watcher)) {
            m_executor->execute(mbox);
        }
    }
    //--------------------------------------------------------

    void actor_system::notify_on_stop_(const actor_ref & ref)
    {
        if(ref == m_root->ref()) {
            m_root_stopped = true;
            std::unique_lock<std::mutex> lock(m_terminate_mutex);
            m_terminate_cv.notify_one();
            m_logger->verbose("The root is stopped.", ref.get_path().c_str());
        }
        // ToDo (a.gruzdev): Consider better scheduler implementation
        else if(ref.get_path() == actor_path::join(root().get_path(), actor_path::scope_to_str(actor_scope::user))) {
            // Stop after all user actors
            m_scheduler->stop();
        }
        else {
            m_logger->verbose("Actor %s is stopped.", ref.get_path().c_str());
        }
    }
    //--------------------------------------------------------


} // namespace actors

} // namespace yato
