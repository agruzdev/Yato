/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <yato/any_match.h>

#include "../actor.h"

#include "../actor_system.h"
#include "actor_cell.h"
#include "message.h"

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

    void actor_base::receive_message(const message & message) noexcept
    {
        assert(m_context != nullptr);
        if (message.payload.type() == typeid(poison_pill_t)) {
            m_context->ref().stop();
            return;
        }
        do_unwrap_message(message);
    }
    //-------------------------------------------------------

    bool actor_base::receive_system_message(const message & msg) noexcept
    {
        assert(m_context != nullptr);
        return any_match(
            [this](const system_message::start &) {
                try {
                    pre_start();
                }
                catch (std::exception & e) {
                    log().error("actor[system_signal]: Unhandled exception: %s", e.what());
                }
                catch (...) {
                    log().error("actor[system_signal]: Unknown exception!");
                }
                return false;
            },
            [this](const system_message::stop &) {
                try {
                    post_stop();
                }
                catch (std::exception & e) {
                    log().error("actor[system_signal]: Unhandled exception: %s", e.what());
                }
                catch (...) {
                    log().error("actor[system_signal]: Unknown exception!");
                }
                // Notify watchers
                auto & watchers = m_context->watchers();
                std::for_each(watchers.begin(), watchers.end(), [this](const actor_ref & watcher) {
                    watcher.tell(yato::actors::terminated(self()), self());
                });
                return true;
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
            //[this](const system_message::attach_child & attach) {
            //    auto child = std::move(const_cast<system_message::attach_child &>(attach).cell);
            //    child->parent = std::make_unique<actor_ref>(self());
            //    auto child_ref = child->act->self();
            //    cell_().children.push_back(std::move(child));
            //    actor_system_ex::send_system_message(system(), child_ref, system_message::start());
            //},
            [this](match_default_t) {
                assert(false);
                log().error("actor[system_signal]: Unknown system message!");
                return false;
            }
        )
        (msg.payload);
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
