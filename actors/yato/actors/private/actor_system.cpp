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
#include "asking_actor.h"
#include "mailbox.h"
#include "pinned_executor.h"
#include "dynamic_executor.h"
#include "scheduler.h"
#include "name_generator.h"

#ifdef YATO_ACTORS_WITH_IO
#include "../io/tcp.h"
#endif

namespace
{
    const std::string DEAD_LETTERS = "_dead_";
    const std::string POISON_PILL  = "_poison_";
}

namespace yato
{
namespace actors
{
    struct system_message::attach_child
    {
        std::unique_ptr<actor_cell> cell;

        explicit
            attach_child(std::unique_ptr<actor_cell> && cell)
            : cell(std::move(cell))
        { }
    };
    //-------------------------------------------------------------

    class guardian
        : public actor<>
    {
        void pre_start() override
        {
            log().info("Guardian is started!");
        }

        void receive(const yato::any & message) override
        {
            //yato::any_match(
            //    [this](const system_message::attach_child & attach) {
            //        auto child = std::move(const_cast<system_message::attach_child &>(attach).cell);
            //        child->parent = std::make_unique<actor_ref>(self());
            //        auto child_ref = child->act->self();
            //        cell_().children.push_back(std::move(child));
            //        actor_system_ex::send_system_message(system(), child_ref, system_message::start());
            //    }
            //)(message);
        }
    };


    //-------------------------------------------------------

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

        m_user_priority_actors_num = 0;

        // System actors
        if(conf.enable_io) {
#ifdef YATO_ACTORS_WITH_IO
            create_actor_impl_(make_builder<io::tcp_manager>(), actor_path(m_name, actor_scope::system, io::tcp_manager::actor_name()));
#else
            throw yato::argument_error("actor_system[actor_system]: IO can't be enabled. Build with flag YATO_ACTORS_WITH_IO");
#endif
        }

        // Crete guardians
        create_guardian_(actor_path("yato://" + m_name + "/users"));
        create_guardian_(actor_path("yato://" + m_name + "/system"));

    }
    //-------------------------------------------------------

    actor_system::actor_system(const std::string & name)
        : actor_system(name, config::defaults())
    { }
    //-------------------------------------------------------

    actor_system::~actor_system()
    {
        {
            std::unique_lock<std::mutex> lock(m_cells_mutex);
            while(m_user_priority_actors_num != 0) {
                m_cells_condition.wait(lock);
            }
            std::for_each(m_actors.begin(), m_actors.end(), [this](decltype(m_actors)::value_type & entry) {
                stop_impl_(entry.second->mail());
            });
            while(!m_actors.empty()) {
                m_cells_condition.wait(lock);
            }

            while (!m_guardians.empty()) {
                m_cells_condition.wait(lock);
            }
        }
        // Now all actors are stopped

        m_scheduler->stop();
    }
    //-------------------------------------------------------

    template <typename Ty_, typename ... Args_>
    inline
    bool enqueue_system_message_(mailbox* mbox, const actor_ref & sender, Args_ && ... args)
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

    actor_ref actor_system::create_guardian_(const actor_path & path)
    {
        auto cell = std::make_unique<actor_cell>(*this, path, std::make_unique<guardian>());
        auto & ref = cell->ref();

        m_guardians.push_back(std::move(cell));
        send_system_message(ref, system_message::start());

        m_logger->verbose("Guardian %s is started!", path.c_str());

        return ref;
    }

    actor_ref actor_system::create_actor_impl_(const actor_builder & builder, const actor_path & path)
    {
        auto cell = std::make_unique<actor_cell>(*this, path, builder());

        auto ref = cell->ref();

        //actor_cell* pcell = cell.get();
        //{
        //    std::unique_lock<std::mutex> lock(m_cells_mutex);
        //    auto pos = m_actors.find(path);
        //    if(pos != m_actors.end()) {
        //        throw yato::argument_error("Actor with the name " + path.to_string() + " already exists!");
        //    }
        //    m_actors.emplace_hint(pos, path, std::move(cell));
        //    if(!path.is_system_scope()) {
        //        ++m_user_priority_actors_num;
        //    }
        //}
        //m_logger->verbose("Actor %s is created!", path.c_str());

        send_system_message(m_guardians[0]->ref(), system_message::attach_child(std::move(cell)));

        //auto ret = enqueue_system_message_<system_message::start>(pcell->mail(), dead_letters());
        //assert(ret);
        //
        //m_executor->execute(pcell->mail());

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
        m_executor->execute(mbox.get());
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
        m_executor->execute(mbox.get());
    }
    //-------------------------------------------------------

    std::future<yato::any> actor_system::ask_impl_(const actor_ref & addressee, yato::any && message, const timeout_type & timeout) const
    {
        std::promise<yato::any> response;
        auto result = response.get_future();

        auto ask_actor_path = actor_path(*this, actor_scope::temp, m_name_generator->next_indexed("ask"));
        auto ask_actor = const_cast<actor_system*>(this)->create_actor_impl_(make_builder<asking_actor>(std::move(response)), ask_actor_path);
        send_impl_(addressee, ask_actor, std::move(message));

        m_scheduler->enqueue(std::chrono::high_resolution_clock::now() + timeout, [ask_actor]{ ask_actor.stop(); });

        return result;
    }
    //-------------------------------------------------------

    void actor_system::stop_impl_(mailbox* mbox) const 
    {
        assert(mbox != nullptr);
        if(enqueue_system_message_<system_message::stop>(mbox, dead_letters())) {
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
            stop_impl_(entry.second->mail());
        });
    }
    //--------------------------------------------------------

    actor_ref actor_system::select(const actor_path & path) const
    {
        std::unique_lock<std::mutex> lock(m_cells_mutex);
        auto it = m_actors.find(path);
        if(it != m_actors.cend()) {
            return (*it).second->ref();
        }
        return dead_letters();
    }
    //--------------------------------------------------------

    void actor_system::watch(const actor_ref & watchee, const actor_ref & watcher) const
    {
        if(watchee == dead_letters() || watcher == dead_letters()) {
            m_logger->error("DeadLetters can't be used in watching");
            return;
        }
        std::shared_ptr<mailbox> mbox = watchee.get_mailbox().lock();
        if (mbox == nullptr) {
            m_logger->warning("Failed to find watchee. Actor %s is not found!", watchee.get_path().c_str());
            watcher.tell(terminated(watchee)); // Already terminated
            return;
        }
        if (enqueue_system_message_<system_message::watch>(mbox.get(), watcher, watcher)) {
            m_executor->execute(mbox.get());
        }
    }
    //--------------------------------------------------------
    
    void actor_system::unwatch(const actor_ref & watchee, const actor_ref & watcher) const
    {
        if (watchee == dead_letters() || watcher == dead_letters()) {
            m_logger->error("DeadLetters can't be used in watching");
            return;
        }
        std::shared_ptr<mailbox> mbox = watchee.get_mailbox().lock();
        if (mbox == nullptr) {
            m_logger->error("Failed to find watchee. Actor %s is not found!", watchee.get_path().c_str());
            return;
        }
        if (enqueue_system_message_<system_message::unwatch>(mbox.get(), watcher, watcher)) {
            m_executor->execute(mbox.get());
        }
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
            if(!ref.get_path().is_system_scope()) {
                --m_user_priority_actors_num;
            }
        }
        m_cells_condition.notify_one();
        m_logger->verbose("Actor %s is stopped.", ref.get_path().c_str());
    }
    //--------------------------------------------------------


} // namespace actors

} // namespace yato
