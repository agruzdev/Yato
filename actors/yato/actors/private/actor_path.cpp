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
    const std::array<std::pair<actor_scope, std::string>, 6> scope_names = {
        std::make_pair(actor_scope::unknown,  "unknown"),
        std::make_pair(actor_scope::user,     "user"),
        std::make_pair(actor_scope::system,   "system"),
        std::make_pair(actor_scope::temp,     "temp"),
        std::make_pair(actor_scope::remote,   "remote"),
        std::make_pair(actor_scope::dead,     "dead")
    };
    //---------------------------------------------------


    const std::string & actor_path::scope_to_str(const actor_scope & scope)
    {
        const auto it = std::find_if(scope_names.cbegin(), scope_names.cend(), [&scope](const auto & entry) { return entry.first == scope; });
        return (it != scope_names.cend())
            ? (*it).second
            : scope_names[0].second;
    }
    //---------------------------------------------------

    const actor_scope & actor_path::str_to_scope(const std::string & name)
    {
        const auto it = std::find_if(scope_names.cbegin(), scope_names.cend(), [&name](const auto & entry) { return entry.second == name; });
        return (it != scope_names.cend())
            ? (*it).first
            : scope_names[0].first;
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
        return std::isgraph(c, actor_path::locale()) && (c != '/');
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
        m_path = actor_path::path_root + system_name + "/" + scope_to_str(scope) + "/" + actor_name;
    }
    //---------------------------------------------------

    actor_path::actor_path(const actor_system & system, const actor_scope & scope, const std::string & actor_name)
    {
        assert(scope != actor_scope::unknown);
        if (!is_valid_actor_name(actor_name)) {
            throw yato::argument_error("actor_path[actor_path]: Invalid actor name!");
        }
        m_path = actor_path::path_root + system.name() + "/" + scope_to_str(scope) + "/" + actor_name;
    }

    //---------------------------------------------------

    bool actor_path::parce(path_elements & elems, bool header_only) const {
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
        elems.scope = str_to_scope(std::string(begin, end));
        if(elems.scope == actor_scope::unknown) {
            return false;
        }

        if(!header_only) {
            // Decode names
            elems.names.clear();
            while(end != m_path.cend()) {
                begin = std::next(end); // skip '/'
                end   = std::find(begin, m_path.cend(), '/');
                if(begin != end) {
                    elems.names.emplace_back(begin, end);
                }
            }
            if(elems.names.empty()) {
                return false;
            }
        }
        return true;
    }


}// namespace actors

}// namespace yato

