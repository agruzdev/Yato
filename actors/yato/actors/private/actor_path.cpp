/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <algorithm>

#include "../actor_system.h"
#include "../actor_path.h"

namespace yato
{
namespace actors
{
    const std::locale actor_path::path_locale = std::locale("C");

    const std::string actor_path::path_root = "yato://";

    //---------------------------------------------------
    inline
    const char* get_scope_name(const actor_scope & scope) 
    {
        switch (scope)
        {
        case actor_scope::user:
            return "user";
        case actor_scope::system:
            return "system";
        case actor_scope::temp:
            return "temp";
        case actor_scope::remote:
            return "remote";
        case actor_scope::dead:
            return "deadLetters";
        default:
            assert(false && "Unknown scope!");
            return "unknown";
        }
    }

    //---------------------------------------------------

    inline
    bool is_valid_char(const char & c)
    {
        return std::isalnum(c, actor_path::locale()) || (c == '_');
    }
    //---------------------------------------------------

    inline
    bool is_valid_path(const char & c)
    {
        //return std::isalnum(c, actor_path::locale()) || (c == '_') || (c == '/');
        return std::isgraph(c, actor_path::locale());
    }
    //---------------------------------------------------

    bool actor_path::is_valid_system_name(const std::string & name)
    {
        return !name.empty() && std::all_of(name.cbegin(), name.cend(), &is_valid_char);
    }
    //---------------------------------------------------

    bool actor_path::is_valid_actor_name(const std::string & name)
    {
        return !name.empty() && std::all_of(name.cbegin(), name.cend(), &is_valid_path);
    }
    //---------------------------------------------------

    actor_path::actor_path(const std::string & system_name, const actor_scope & scope, const std::string & actor_name)
    {
        if (!is_valid_system_name(system_name)) {
            throw yato::argument_error("actor_path[actor_path]: Invalid system name!");
        }
        if (!is_valid_actor_name(actor_name)) {
            throw yato::argument_error("actor_path[actor_path]: Invalid actor name!");
        }
        m_path = actor_path::path_root + system_name + "/" + get_scope_name(scope) + "/" + actor_name;
    }
    //---------------------------------------------------

    actor_path::actor_path(const actor_system & system, const actor_scope & scope, const std::string & actor_name)
    {
        if (!is_valid_actor_name(actor_name)) {
            throw yato::argument_error("actor_path[actor_path]: Invalid actor name!");
        }
        m_path = actor_path::path_root + system.name() + "/" + get_scope_name(scope) + "/" + actor_name;
    }
    //---------------------------------------------------

    bool actor_path::is_system_scope() const 
    {
        const char* scope_name = get_scope_name(actor_scope::system);
        auto pos = std::find(std::next(m_path.cbegin(), actor_path::path_root.size()), m_path.cend(), '/');
        return std::equal(scope_name, scope_name + std::strlen(scope_name), std::next(pos));
    }

}// namespace actors

}// namespace yato

