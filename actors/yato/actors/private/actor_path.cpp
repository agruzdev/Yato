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

    static
    const std::array<std::pair<actor_scope, std::string>, 5> scope_names = {
        std::make_pair(actor_scope::user,   "user"),
        std::make_pair(actor_scope::system, "system"),
        std::make_pair(actor_scope::temp,   "temp"),
        std::make_pair(actor_scope::remote, "remote"),
        std::make_pair(actor_scope::dead,   "deadLetters")
    };

    //---------------------------------------------------
    inline
    std::string get_scope_name(const actor_scope & scope) 
    {
        auto it = std::find_if(scope_names.cbegin(), scope_names.cend(), [&scope](const auto & entry) { return entry.first == scope; });
        return (it != scope_names.cend())
            ? (*it).second
            : std::string();
    }

    inline
    actor_scope get_scope_enum(const std::string & name)
    {
        auto it = std::find_if(scope_names.cbegin(), scope_names.cend(), [&name](const auto & entry) { return entry.second == name; });
        return (it != scope_names.cend())
            ? (*it).first
            : actor_scope::unknown;
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
        assert(scope != actor_scope::unknown);
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
        assert(scope != actor_scope::unknown);
        if (!is_valid_actor_name(actor_name)) {
            throw yato::argument_error("actor_path[actor_path]: Invalid actor name!");
        }
        m_path = actor_path::path_root + system.name() + "/" + get_scope_name(scope) + "/" + actor_name;
    }
    //---------------------------------------------------

    bool actor_path::parce(path_elements & elems) const {
        // Check root
        auto begin = m_path.cbegin();
        if(!std::equal(path_root.cbegin(), path_root.cend(), begin)) {
            return false;
        }
        std::advance(begin, path_root.size());

        // Decode system
        auto end = std::find(begin, m_path.cend(), '/');
        if (end == m_path.cend()) {
            return false;
        }
        elems.system_name = std::string(begin, end);

        // Decode scope
        begin = end;
        if(begin != m_path.cend()) {
            std::advance(begin, 1);
        }
        end   = std::find(begin, m_path.cend(), '/');
        elems.scope = get_scope_enum(std::string(begin, end));

        // Decode names
        elems.names.clear();
        while(end != m_path.cend()) {
            begin = std::next(end);
            end   = std::find(begin, m_path.cend(), '/');
            if(begin != end) {
                elems.names.emplace_back(begin, end);
            }
        }
        return !elems.names.empty();
    }
    //---------------------------------------------------

    bool actor_path::is_system_scope() const 
    {
        //auto scope_name = get_scope_name(actor_scope::system);
        //auto pos = std::find(std::next(m_path.cbegin(), actor_path::path_root.size()), m_path.cend(), '/');
        //return std::equal(scope_name.begin(), scope_name.end(), std::next(pos));
        return false;
    }

}// namespace actors

}// namespace yato

