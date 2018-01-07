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
    struct root_terminate {};
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

        bool m_sys_stopped;
        bool m_usr_stopped;
        bool m_tmp_stopped;
        bool m_rmt_stopped;

        //---------------------------------------

        actor_ref create_guard_(const actor_path & path);

        void pre_start() override;

        void receive(yato::any & message) override;
    };


} // namespace actors

} // namespace yato

#endif //_YATO_ACTORS_PRIVATE_ACTORS_ROOT_H_

