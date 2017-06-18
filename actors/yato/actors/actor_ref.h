/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_REF_H_
#define _YATO_ACTOR_REF_H_

#include <string>

namespace yato
{
namespace actors
{
    class actor_system;

    /**
     * Unique handle of an actor
     */
    class actor_ref final
    {
    private:
        // Not owning pointer. Can copy
        actor_system* m_system;

        std::string m_name;
        std::string m_path;

        actor_ref(actor_system* system, const std::string & name);
    public:

        ~actor_ref() = default;

        actor_ref(const actor_ref&) = default;
        actor_ref(actor_ref&&) = default;

        actor_ref& operator=(const actor_ref&) = default;
        actor_ref& operator=(actor_ref&&) = default;

        const std::string & get_name() const 
        {
            return m_name;
        }

        const std::string & get_path() const 
        {
            return m_path;
        }

        template <typename Ty_>
        void tell(Ty_ && message) const;

        template <typename Ty_>
        void tell(Ty_ && message, const actor_ref & sender) const;

        bool operator == (const actor_ref & other) const 
        {
            return m_path == other.m_path;
        }

        friend class actor_system;
    };
    //-------------------------------------------------------

}// namespace actors

}// namespace yato

#endif //_YATO_ACTOR_REF_H_

