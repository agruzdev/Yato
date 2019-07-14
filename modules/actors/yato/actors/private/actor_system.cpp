/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2019 Alexey Gruzdev
*/

#include <condition_variable>
#include <memory>

#include <yato/assertion.h>

#include "../actor.h"
#include "../actor_system.h"

#include "actor_cell.h"
#include "mailbox.h"
#include "scheduler.h"
#include "name_generator.h"
#include "system_message.h"
#include "execution_context.h"

#ifdef YATO_ACTORS_WITH_IO
#include "../io/facade.h"
#endif

#include "actors/root.h"
#include "actors/selector.h"
#include "actors/asking_actor.h"

namespace
{
    const std::string DEAD_LETTERS = "_dead_";
}


namespace yato
{
namespace actors
{

    struct system_context
    {
        std::string name;
        logger_ptr log;

        std::mutex terminate_mutex;
        std::condition_variable terminate_cv;
        bool root_stopped;

        name_generator names_gen;
        scheduler global_scheduler;

        std::vector<execution_context> executions;
        std::string default_executor_name;

        std::unique_ptr<actor_cell> root;
        actor_ref dead_letters;
    };

    //-------------------------------------------------------------------------

    inline
    execution_context* find_execution_(system_context& system, const std::string & name)
    {
        const auto it = std::find_if(system.executions.begin(), system.executions.begin(), 
            [&name](const execution_context & execution){ return execution.name == name; }
        );
        return (it != system.executions.cend()) ? &(*it) : nullptr;
    }
    //-------------------------------------------------------------------------

    inline
    properties_internal resolve_props_(system_context& system, const properties & props)
    {
        properties_internal res;
        res.execution = find_execution_(system, props.execution_name);
        return res;
    }
    //-------------------------------------------------------------------------

    inline
    properties_internal default_properties_(system_context& system)
    {
        properties_internal res;
        res.execution = find_execution_(system, system.default_executor_name);
        return res;
    }
    //-------------------------------------------------------------------------

    void actor_system::init_executors_(const yato::config & conf)
    {
        m_context->executions.clear();

        // Put default executor first
        const auto default_name = conf.value<std::string>("default_executor");
        if(default_name) {
            m_context->default_executor_name = default_name.get();
        }
        else {
            m_context->default_executor_name = "default";
            m_context->executions.push_back(execution_context::get_default(this, m_context->default_executor_name));
        }

        // Read custom executors
        if (const auto executors_array = conf.array("execution_contexts")) {
            const execution_context_converter converter(this);
            for (size_t i = 0; i < executors_array.size(); ++i) {
                m_context->executions.push_back(executors_array.value<execution_context>(i, converter).get());
            }
        }

        // Verify
        if(find_execution_(*m_context, m_context->default_executor_name) == nullptr) {
            throw yato::config_error("Default executor \"" + m_context->default_executor_name + "\" is not found!");
        }
    }
    //-------------------------------------------------------------------------

    actor_system::actor_system(const std::string & name, const config & conf)
        : m_context(new system_context())
    {
        if(!actor_path::is_valid_system_name(name)) {
            throw yato::argument_error("actor_system[actor_system]: Invalid name!");
        }
        m_context->name = name;
        m_context->dead_letters = actor_ref(this, actor_path(name, actor_scope::dead, DEAD_LETTERS));

        m_context->log = logger_factory::create(std::string("ActorSystem[") + name + "]");
        m_context->log->set_filter(conf.value<log_level>("log_level").get_or(log_level::info));

        init_executors_(conf);

        const auto root_builder = details::make_cell_builder<actors::root>();
        m_context->root = root_builder(*this, actor_path("yato://" + name), default_properties_(*m_context));
        m_context->root_stopped = false;

        // System actors
        if(conf.value<bool>("enable_io").get_or(false)) {
#ifdef YATO_ACTORS_WITH_IO
            io::facade::init(*this);
#else
            throw yato::argument_error("actor_system[actor_system]: IO can't be enabled. Build with flag YATO_ACTORS_WITH_IO");
#endif
        }
        YATO_ENSURES(m_context->root != nullptr);
        send_system_message(m_context->root->ref(), system_message::start());

        YATO_ENSURES(!m_context->executions.empty());
    }
    //-------------------------------------------------------

    actor_system::actor_system(const std::string & name)
        : actor_system(name, config())
    { }
    //-------------------------------------------------------

    actor_system::~actor_system()
    {
        YATO_REQUIRES(m_context != nullptr);

        shutdown_impl_(false);

        // Important to destroy executor first, so all messages are processed.
        m_context->executions.clear();
    }
    //-------------------------------------------------------

    actor_system::actor_system(actor_system&&) noexcept = default;
    actor_system& actor_system::operator=(actor_system&&) noexcept = default;
    //-------------------------------------------------------

    void actor_system::shutdown_impl_(bool forced)
    {
        YATO_REQUIRES(m_context != nullptr);

        send_message(m_context->root->ref(), root_terminate(forced));
        {
            std::unique_lock<std::mutex> lock(m_context->terminate_mutex);
            m_context->terminate_cv.wait(lock, [this]() { return m_context->root_stopped; });
        }
        // Now all actors are stopped
        //m_scheduler->stop();
    }
    //-------------------------------------------------------

    void actor_system::shutdown()
    {
        shutdown_impl_(true);
    }
    //-------------------------------------------------------

    const actor_ref & actor_system::root() const
    {
        YATO_REQUIRES(m_context != nullptr);
        return m_context->root->ref();
    }
    //-------------------------------------------------------

    const std::string & actor_system::name() const
    {
        YATO_REQUIRES(m_context != nullptr);
        return m_context->name;
    }
    //-------------------------------------------------------

    const logger_ptr & actor_system::logger() const
    {
        YATO_REQUIRES(m_context != nullptr);
        return m_context->log;
    }
    //-------------------------------------------------------

    const actor_ref & actor_system::dead_letters() const
    {
        YATO_REQUIRES(m_context != nullptr);
        return m_context->dead_letters;
    }
    //-------------------------------------------------------

    template <typename Ty_, typename ... Args_>
    bool create_and_enqueue_system_message_(const std::shared_ptr<mailbox> & mbox, const actor_ref & sender, Args_ && ... args)
    {
        yato::any payload(yato::in_place_type_t<Ty_>{}, std::forward<Args_>(args)...);
        return mbox->enqueue_system_message(std::make_unique<message>(std::move(payload), sender));
    }
    //-------------------------------------------------------

    actor_ref actor_system::create_actor_impl_(const details::cell_builder & builder, const yato::optional<properties>& props, const actor_path & path, const actor_ref & parent)
    {
        YATO_REQUIRES(m_context != nullptr);

        path_elements elems;
        path.parce(elems);

        if(elems.scope == actor_scope::unknown) {
            throw yato::argument_error("Invalid actor path!");
        }

        auto cell = builder(*this, path, props ? resolve_props_(*m_context, props.get()) : default_properties_(*m_context));
        auto ref  = cell->ref();

        // Add to the tree
        if(parent.empty()) {
            send_message(m_context->root->ref(), root_add(std::move(cell)));
        } else {
            send_system_message(parent, system_message::attach_child(std::move(cell)));
        }

        return ref;
    }
    //-------------------------------------------------------

    void actor_system::send_user_impl_(const actor_ref & addressee, const actor_ref & sender, yato::any && usrMessage) const
    {
        YATO_REQUIRES(m_context != nullptr);

        if(addressee.empty() || addressee == dead_letters()) {
            logger()->verbose("A message was delivered to DeadLetters.");
            return;
        }

        const std::shared_ptr<mailbox> mbox = addressee.get_mailbox().lock();
        if(mbox == nullptr) {
            logger()->verbose("Failed to send a message. Actor %s is not found!", addressee.get_path().c_str());
            return;
        }

        if(mbox->enqueue_user_message(std::make_unique<message>(std::move(usrMessage), sender))) {
            mbox->schedule_for_execution();
        }
    }
    //-------------------------------------------------------

    void actor_system::send_system_impl_(const actor_ref & addressee, const actor_ref & sender, yato::any && sysMessage) const
    {
        YATO_REQUIRES(m_context != nullptr);

        if (addressee.empty() || addressee == dead_letters()) {
            logger()->verbose("A system message was delivered to DeadLetters.");
            return;
        }

        const std::shared_ptr<mailbox> mbox = addressee.get_mailbox().lock();
        if (mbox == nullptr) {
            logger()->verbose("Failed to send a message. Actor %s is not found!", addressee.get_path().c_str());
            return;
        }

        if(mbox->enqueue_system_message(std::make_unique<message>(std::move(sysMessage), sender))) {
            const bool scheduled = mbox->schedule_for_execution();
            YATO_MAYBE_UNUSED(scheduled);
            YATO_ENSURES(scheduled);
        }
    }
    //-------------------------------------------------------

    std::future<yato::any> actor_system::ask_impl_(const actor_ref & addressee, yato::any && message, const timeout_type & timeout) const
    {
        YATO_REQUIRES(m_context != nullptr);

        std::promise<yato::any> response;
        auto result = response.get_future();

        const auto ask_actor_path = actor_path(*this, actor_scope::temp, m_context->names_gen.next_indexed("ask"));
        const auto ask_actor = const_cast<actor_system*>(this)->create_actor_impl_(
            details::make_cell_builder<asking_actor>(std::move(response)), yato::some(properties()), ask_actor_path, actor_ref{});
        send_user_impl_(addressee, ask_actor, std::move(message));

        m_context->global_scheduler.enqueue(std::chrono::high_resolution_clock::now() + timeout, [ask_actor]{ ask_actor.stop(); });

        return result;
    }
    //-------------------------------------------------------

    void actor_system::stop_impl_(const std::shared_ptr<mailbox> & mbox) const 
    {
        YATO_REQUIRES(m_context != nullptr);
        YATO_REQUIRES(mbox != nullptr);

        if(create_and_enqueue_system_message_<system_message::stop>(mbox, dead_letters())) {
            mbox->schedule_for_execution();
        }
    }
    //-------------------------------------------------------

    void actor_system::stop(const actor_ref & addressee) const 
    {
        YATO_REQUIRES(m_context != nullptr);

        const std::shared_ptr<mailbox> mbox = addressee.get_mailbox().lock();
        if (mbox == nullptr) {
            logger()->verbose("Failed to stop actor. Actor %s is not found!", addressee.get_path().c_str());
            return;
        }
        stop_impl_(mbox);
    }
    //--------------------------------------------------------

    std::future<actor_ref> actor_system::find_impl_(const actor_path & path, const timeout_type & timeout) const
    {
        YATO_REQUIRES(m_context != nullptr);

        std::promise<actor_ref> promise;
        auto result = promise.get_future();

        const auto selector_path = actor_path(*this, actor_scope::temp, m_context->names_gen.next_indexed("find"));
        const auto select_actor  = const_cast<actor_system*>(this)->create_actor_impl_(
            details::make_cell_builder<selector>(path, std::move(promise)), yato::some(properties()), selector_path, actor_ref{});

        m_context->global_scheduler.enqueue(std::chrono::high_resolution_clock::now() + timeout, [select_actor]{ select_actor.stop(); });

        return result;
    }
    //--------------------------------------------------------

    void actor_system::watch(const actor_ref & watchee, const actor_ref & watcher) const
    {
        YATO_REQUIRES(m_context != nullptr);

        if(watchee == dead_letters() || watcher == dead_letters()) {
            logger()->error("DeadLetters can't be used in watching");
            return;
        }
        const std::shared_ptr<mailbox> mbox = watchee.get_mailbox().lock();
        if (mbox == nullptr) {
            logger()->warning("Failed to find watchee. Actor %s is not found!", watchee.get_path().c_str());
            watcher.tell(terminated(watchee)); // Already terminated
            return;
        }
        if (create_and_enqueue_system_message_<system_message::watch>(mbox, watcher, watcher)) {
            mbox->schedule_for_execution();
        }
    }
    //--------------------------------------------------------
    
    void actor_system::unwatch(const actor_ref & watchee, const actor_ref & watcher) const
    {
        YATO_REQUIRES(m_context != nullptr);

        if (watchee == dead_letters() || watcher == dead_letters()) {
            logger()->error("DeadLetters can't be used in watching");
            return;
        }
        const std::shared_ptr<mailbox> mbox = watchee.get_mailbox().lock();
        if (mbox == nullptr) {
            logger()->error("Failed to find watchee. Actor %s is not found!", watchee.get_path().c_str());
            return;
        }
        if (create_and_enqueue_system_message_<system_message::unwatch>(mbox, watcher, watcher)) {
            mbox->schedule_for_execution();
        }
    }
    //--------------------------------------------------------

    void actor_system::notify_on_stop_(const actor_ref & ref)
    {
        YATO_REQUIRES(m_context != nullptr);

        if(ref == m_context->root->ref()) {
            std::unique_lock<std::mutex> lock(m_context->terminate_mutex);
            m_context->root_stopped = true;
            m_context->terminate_cv.notify_one();
            logger()->verbose("The root is stopped.", ref.get_path().c_str());
        }
        // ToDo (a.gruzdev): Consider better scheduler implementation
        else if(ref.get_path() == actor_path::join(root().get_path(), actor_path::scope_to_str(actor_scope::user))) {
            // Stop after all user actors
            m_context->global_scheduler.stop();
        }
        else {
            logger()->verbose("Actor %s is stopped.", ref.get_path().c_str());
        }
    }
    //--------------------------------------------------------


} // namespace actors

} // namespace yato
