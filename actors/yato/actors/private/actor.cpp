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

    void basic_actor::set_context_(actor_cell* cell) 
    {
        m_context = cell;
        // default behaviour
        m_behaviours.push(this);
    }
    //-------------------------------------------------------

    const actor_ref & basic_actor::self() const 
    {
        if(m_context == nullptr) {
            throw yato::bad_state_error("Actor is not initialized yet");
        }
        return m_context->ref();
    }
    //-------------------------------------------------------

    actor_system & basic_actor::system()
    {
        if (m_context == nullptr) {
            throw yato::bad_state_error("Actor is not initialized yet");
        }
        return m_context->system();
    }
    //-------------------------------------------------------

    const actor_system & basic_actor::system() const
    {
        return const_cast<basic_actor*>(this)->system();
    }
    //-------------------------------------------------------

    const logger & basic_actor::log() const
    {
        if (m_context == nullptr) {
            throw yato::bad_state_error("Actor is not initialized yet");
        }
        return m_context->log();
    }
    //-------------------------------------------------------

    void basic_actor::watch(const actor_ref & watchee) const
    {
        system().watch(watchee, self());
    }
    //-------------------------------------------------------

    void basic_actor::unwatch(const actor_ref & watchee) const
    {
        system().unwatch(watchee, self());
    }
    //-------------------------------------------------------

    void basic_actor::receive_message_(message && message) noexcept
    {
        assert(m_context != nullptr);
        if (message.payload.type() == typeid(poison_pill_t)) {
            m_context->ref().stop();
            return;
        }
        m_sender = &message.sender;
        try {
            YATO_ASSERT(!m_behaviours.empty(), "No behaviours!");
            m_behaviours.top()->receive(std::move(message.payload));
        }
        catch(std::exception & e) {
            log().error("actor[receive]: Unhandled exception: %s", e.what());
        }
        catch (...) {
            log().error("actor[receive]: Unknown exception!");
        }
        m_sender = nullptr;
    }
    //-------------------------------------------------------

    message_consumer* basic_actor::become(message_consumer* behaviour, bool discard_old) noexcept
    {
        if(behaviour == nullptr) {
            log().error("failed to perform become(): the new behaviour is empty.");
            return nullptr;
        }
        if (discard_old) {
            std::swap(m_behaviours.top(), behaviour);
            return behaviour;
        } else {
            m_behaviours.push(behaviour);
            return nullptr;
        }
    }
    //-------------------------------------------------------

    message_consumer* basic_actor::unbecome() noexcept
    {
        if(m_behaviours.size() <= 1) {
            log().error("failed to perform unbecome(): the behaviour stack has only one element.");
            return nullptr;
        }
        const auto tmp = m_behaviours.top();
        m_behaviours.pop();
        return tmp;
    }
    //-------------------------------------------------------

    actor_ref basic_actor::create_child_impl_(const std::string & name, const details::cell_builder & builder)
    {
        return actor_system_ex::create_actor(system(), self(), name, builder);
    }
    //-------------------------------------------------------

    void basic_actor::stop_impl() noexcept
    {
        if(context_().is_started()) {
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
        context_().set_started(false);
        log().verbose("Stopped (%s)", self().get_path().c_str());

        // Notify watchers
        auto & watchers = m_context->watchers();
        std::for_each(watchers.begin(), watchers.end(), [this](const actor_ref & watcher) {
            watcher.tell(yato::actors::terminated(self()), self());
        });
        // Detach from parent
        if (context_().has_parent()) {
            actor_system_ex::send_system_message(system(), context_().parent(), system_message::detach_child(self()));
        }
    }
    //-------------------------------------------------------
    bool basic_actor::receive_system_message_(message && msg) noexcept
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
                context_().set_started(success);
                if(!success) {
                    // if failed to start, then terminate
                    self().stop();
                }
                return false;
            },
            [this](const system_message::stop &) {
                if(context_().children().empty()) {
                    stop_impl();
                    return true;
                } else {
                    // Wait and stop after children
                    context_().set_stop(true);
                    for(auto & child : context_().children()) {
                        child->ref().tell(poison_pill);
                    }
                    return false;
                }
            },
            [this](const system_message::stop_after_children &) {
                if (context_().children().empty()) {
                    stop_impl();
                    return true;
                }
                context_().set_stop(true);
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
                if(context_().stopping()) {
                    context_().log().warning("Child can't be attached. Actor is going to stop.");
                    return false;
                }
                auto child = std::move(const_cast<system_message::attach_child &>(attach).cell);
                auto child_ref = context_().add_child(std::move(child));
                actor_system_ex::send_system_message(system(), child_ref, system_message::start());
                context_().log().verbose("Attached child %s", child_ref.get_path().c_str());
                return false;
            },
            [this](const system_message::detach_child & detach) {
                context_().remove_child(detach.ref);
                context_().log().verbose("Detached child %s", detach.ref.get_path().c_str());
                if(context_().stopping() && context_().children().empty()) {
                    stop_impl();
                    return true;
                }
                return false;
            },
            [this](system_message::selection & select) {
                auto & path = select.path;
                if(path.empty()) {
                    // Reached the path's end
                    select.sender.tell(selection_success(context_().ref()));
                } else {
                    // Forward to a child
                    bool found = false;
                    auto next = std::move(path.back());
                    path.pop_back();
                    for(auto & child : context_().children()) {
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
        )(std::move(msg.payload)));
    }
    //-------------------------------------------------------

    actor_cell & basic_actor::context_() {
        if (m_context == nullptr) {
            throw yato::bad_state_error("Actor is not initialized yet");
        }
        return *m_context;
    }

} // namespace actors

} // namespace yato
