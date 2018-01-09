/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_PRIVATE_ACTORS_ROOT_H_
#define _YATO_ACTORS_PRIVATE_ACTORS_ROOT_H_

#include "../../actor.h"

namespace yato
{
namespace actors
{

    /**
     * Add new actor to the tree
     */
    struct root_add
    {
        std::unique_ptr<actor_cell> cell;

        explicit
        root_add(std::unique_ptr<actor_cell> && cell)
            : cell(std::move(cell))
        { }
    };
    //---------------------------------------------------------


    /**
     * Message for terminating root on exit of actor system
     */
    struct root_terminate
    {
        bool stop_user;

        explicit
        root_terminate(bool stop_user)
            : stop_user(stop_user)
        { }
    };
    //---------------------------------------------------------


    /**
     * Root of the actors tree
     */
    class root
        : public actor<>
    {
    private:
        actor_ref m_sys_guard;
        actor_ref m_usr_guard;
        actor_ref m_tmp_guard;
        actor_ref m_rmt_guard;

        bool m_sys_stopped = false;
        bool m_usr_stopped = false;
        bool m_tmp_stopped = false;
        bool m_rmt_stopped = false;

        //---------------------------------------

        actor_ref create_guard_(const actor_path & path);

        void pre_start() override;

        void receive(yato::any & message) override;
    };


} // namespace actors

} // namespace yato

#endif //_YATO_ACTORS_PRIVATE_ACTORS_ROOT_H_

