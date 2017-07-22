/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_PATH_H_
#define _YATO_ACTOR_PATH_H_

#include <string>
#include <locale>

#include <yato/prerequisites.h>
#include <yato/assert.h>

namespace yato
{
namespace actors
{

    class actor_system;

    enum class actor_scope
    {
        user,   ///< all user-created actors
        system, ///< all system-created actors
        temp,   ///< all short-lived system-created actors
        remote, ///< all actors representing remote entities
        dead,   ///< virtual dead letter actors

        unknown
    };


    struct path_elements
    {
        std::string system_name;
        actor_scope scope;
        std::vector<std::string> names;
    };


    class actor_path
    {
    private:
        static const std::locale path_locale;
        static const std::string path_root;
        //----------------------------------------------------

    public:
        static bool is_valid_system_name(const std::string & name);
        static bool is_valid_actor_name(const std::string & name);
        //----------------------------------------------------

    private:
        std::string m_path;
        //----------------------------------------------------

    public:
        explicit
        actor_path(const std::string & path)
            : m_path(path)
        { }
        
        explicit
        actor_path(const char* path)
            : m_path(path)
        { }

        actor_path(const std::string & system_name, const actor_scope & scope, const std::string & actor_name);

        actor_path(const actor_system & system, const actor_scope & scope, const std::string & actor_name);

        ~actor_path() = default;

        actor_path(const actor_path&) = default;
        actor_path(actor_path&&) = default;

        actor_path& operator=(const actor_path&) = default;
        actor_path& operator=(actor_path&&) = default;

        const std::string & to_string() const 
        {
            return m_path;
        }

        const char* c_str() const
        {
            return m_path.c_str();
        }

        std::string get_name() const 
        {
            //ToDo (a.gruzdev): temporal solution
            //auto pos = std::find(m_path.begin() + actor_path::path_root.size(), m_path.end(), '/');
            //pos = std::find(pos + 1, m_path.end(), '/');
            //return m_path.substr(pos + 1 - m_path.begin());
            return m_path;
        }

        /**
         * Check if this path represents actor in system scope
         */
        bool is_system_scope() const;

        /**
         * Parce actor path
         */
        bool parce(path_elements & elems) const;

        friend
        bool operator == (const actor_path & one, const actor_path & another)
        {
            return one.m_path == another.m_path;
        }

        friend
        bool operator != (const actor_path & one, const actor_path & another)
        {
            return one.m_path != another.m_path;
        }

        friend 
        bool operator < (const actor_path & one, const actor_path & another)
        {
            return one.m_path < another.m_path;
        }

        static
        const std::string & default_root()
        {
            return actor_path::path_root;
        }

        static
        const std::locale & locale() 
        {
            return actor_path::path_locale;
        }

        static
        actor_path join(const actor_path & path, const std::string & name) {
            return actor_path(path.m_path + "/" + name);
        }
    };


}// namespace actors

}// namespace yato

#endif //_YATO_ACTOR_PATH_H_

