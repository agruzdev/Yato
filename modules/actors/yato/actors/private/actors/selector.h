/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_PRIVATE_ACTORS_SELECTOR_H_
#define _YATO_ACTORS_PRIVATE_ACTORS_SELECTOR_H_

#include "../../actor.h"
#include "../../actor_ref.h"

namespace yato
{
namespace actors
{

    //struct selection_start
    //{
    //    actor_path path;
    //
    //    explicit
    //    selection_start(const actor_path & path)
    //        : path(path)
    //    { }
    //};
    //------------------------------------------------------


    struct selection_success
    {
        actor_ref result;

        explicit
        selection_success(const actor_ref & result)
            : result(result)
        { }
    };
    //------------------------------------------------------


    struct selection_failure
    {
        std::string reason;

        explicit
        selection_failure(const std::string & reason)
            : reason(reason)
        { }

        explicit
        selection_failure(std::string && reason)
            : reason(std::move(reason))
        { }
    };
    //------------------------------------------------------


    /**
     * Runs search of a path in the actors tree
     */
    class selector
        : public actor
    {
    private:
        std::promise<actor_ref> m_result;
        bool m_satisfied;
        actor_path m_target;
        //------------------------------------------------------

        void pre_start() override;

        void receive(yato::any &&) override;

        void post_stop() override;

    public:
        selector(const actor_path & target, std::promise<actor_ref> && result)
            : m_result(std::move(result)), m_satisfied(false), m_target(target)
        { }
    };

} // namespace actors

} // namespace yato

#endif //_YATO_ACTORS_PRIVATE_ACTORS_SELECTOR_H_

