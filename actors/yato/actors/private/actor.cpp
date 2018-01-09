/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <yato/any_match.h>

#include "../actor.h"
#include "../actor_system.h"

#include "actor_system_ex.h"
#include "actor_cell.h"
#include "message.h"
#include "system_message.h"

#include "actors/selector.h"

namespace yato
{
namespace actors
{

    actor_base::actor_base()
    { }
    //-------------------------------------------------------

    actor_base::~actor_base()
    { }
    //-------------------------------------------------------

    void actor_base::init_base_(actor_cell* cell) 
    {
        m_context = cell;
    }
    //-------------------------------------------------------

    const actor_ref & actor_base::self() const 
    {
        if(m_context == nullptr) {
            throw yato::bad_state_error("Actor is not initialized yet");
        }
        return m_context->ref();
    }
    //-------------------------------------------------------

    actor_system & actor_base::system()
    {
        if (m_context == nullptr) {
            throw yato::bad_state_error("Actor is not initialized yet");
        }
        return m_context->system();
    }
    //-------------------------------------------------------

    const actor_system & actor_base::system() const
    {
        return const_cast<actor_base*>(this)->system();
    }
    //-------------------------------------------------------

    const logger & actor_base::log() const
    {
        if (m_context == nullptr) {
            throw yato::bad_state_error("Actor is not initialized yet");
        }
        return m_context->log();
    }
    //-------------------------------------------------------

    void actor_base::watch(const actor_ref & watchee) const
    {
        system().watch(watchee, self());
    }
    //-------------------------------------------------------

    void actor_base::unwatch(const actor_ref & watchee) const
    {
        system().unwatch(watchee, self());
    }
    //-------------------------------------------------------

    void actor_base::receive_message(message & message) noexcept
    {
        assert(m_context != nullptr);
        if (message.payload.type() == typeid(poison_pill_t)) {
            m_context->ref().stop();
            return;
        }
        do_unwrap_message(message);
    }
    //-------------------------------------------------------

    actor_ref actor_base::create_child_impl_(const std::string & name, const details::cell_builder & builder)
    {
        return actor_system_ex::create_actor(system(), self(), name, builder);
    }
    //-------------------------------------------------------

    void actor_base::stop_impl() noexcept
    {
        if(context().is_started()) {
            try {
                post_stop();
            }
            catch (std::exception & e) {
                log().error("actor[system_signal]: Unhandled exception: %s", e.what());
            }
            catch (...) {
                log().error("actor[system_signal]: Unknown exception!");
            }
        }
        context().set_started(false);
        log().verbose("Stopped (%s)", self().get_path().c_str());

        // Notify watchers
        auto & watchers = m_context->watchers();
        std::for_each(watchers.begin(), watchers.end(), [this](const actor_ref & watcher) {
            watcher.tell(yato::actors::terminated(self()), self());
        });
        // Detach from parent
        if (context().has_parent()) {
            actor_system_ex::send_system_message(system(), context().parent(), system_message::detach_child(self()));
        }
    }
    //-------------------------------------------------------
    bool actor_base::receive_system_message(message & msg) noexcept
    {
        assert(m_context != nullptr);
        // (a.gruzdev) static_cast for ReSharper's calmness
        return static_cast<bool>(any_match(
            [this](const system_message::start &) {
                bool success = false;
                try {
                    pre_start();
                    success = true;
                    log().verbose("Started (%s)", self().get_path().c_str());
                }
                catch (std::exception & e) {
                    log().error("actor[system_signal]: Unhandled exception: %s", e.what());
                }
                catch (...) {
                    log().error("actor[system_signal]: Unknown exception!");
                }
                context().set_started(success);
                if(!success) {
                    // if failed to start, then terminate
                    self().stop();
                }
                return false;
            },
            [this](const system_message::stop &) {
                if(context().children().empty()) {
                    stop_impl();
                    return true;
                } else {
                    // Wait and stop after children
                    context().set_stop(true);
                    for(auto & child : context().children()) {
                        child->ref().tell(poison_pill);
                    }
                    return false;
                }
            },
            [this](const system_message::stop_after_children &) {
                if (context().children().empty()) {
                    stop_impl();
                    return true;
                }
                context().set_stop(true);
                return false;
            },
            [this](const system_message::watch & watch) {
                auto & watchers = m_context->watchers();
                auto pos = std::find(watchers.begin(), watchers.end(), watch.watcher);
                if(pos == watchers.end()) {
                    watchers.push_back(watch.watcher);
                }
                return false;
            },
            [this](const system_message::unwatch & watch) {
                auto & watchers = m_context->watchers();
                auto pos = std::find(watchers.begin(), watchers.end(), watch.watcher);
                if (pos != watchers.end()) {
                    watchers.erase(pos);
                }
                return false;
            },
            [this](const system_message::attach_child & attach) {
                if(context().stopping()) {
                    context().log().warning("Child can't be attached. Actor is going to stop.");
                    return false;
                }
                auto child = std::move(const_cast<system_message::attach_child &>(attach).cell);
                auto child_ref = context().add_child(std::move(child));
                actor_system_ex::send_system_message(system(), child_ref, system_message::start());
                context().log().verbose("Attached child %s", child_ref.get_path().c_str());
                return false;
            },
            [this](const system_message::detach_child & detach) {
                context().remove_child(detach.ref);
                context().log().verbose("Detached child %s", detach.ref.get_path().c_str());
                if(context().stopping() && context().children().empty()) {
                    stop_impl();
                    return true;
                }
                return false;
            },
            [this](system_message::selection & select) {
                auto & path = select.path;
                if(path.empty()) {
                    // Reached the path's end
                    select.sender.tell(selection_success(context().ref()));
                } else {
                    // Forward to a child
                    bool found = false;
                    auto next = std::move(path.back());
                    path.pop_back();
                    for(auto & child : context().children()) {
                        if(next == child->ref().name()) {
                            actor_system_ex::send_system_message(system(), child->ref(), std::move(select));
                            found = true;
                        }
                    }
                    if(!found) {
                        select.sender.tell(selection_failure("Selection target is not found."));
                    }
                }
                return false;
            },
            [this](match_default_t) {
                assert(false);
                log().error("actor[system_signal]: Unknown system message!");
                return false;
            }
        )(msg.payload));
    }
    //-------------------------------------------------------

    actor_cell & actor_base::context() {
        if (m_context == nullptr) {
            throw yato::bad_state_error("Actor is not initialized yet");
        }
        return *m_context;
    }

} // namespace actors

} // namespace yato
