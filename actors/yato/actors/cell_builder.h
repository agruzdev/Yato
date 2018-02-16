/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_CELL_BUILDER_H_
#define _YATO_ACTORS_CELL_BUILDER_H_

#include <functional>
#include <memory>

namespace yato
{
namespace actors
{
    class basic_actor;
    class actor_cell;
    class actor_path;
    class actor_system;

    namespace details 
    {
        /**
         * Helper class for uniform construction of an arbitraty actor
         */
        class cell_builder
        {
            std::function<std::unique_ptr<basic_actor>()> m_ctor;

        public:
            cell_builder(const std::function<std::unique_ptr<basic_actor>()> & ctor)
                : m_ctor(ctor)
            { }
            
            cell_builder(std::function<std::unique_ptr<basic_actor>()> && ctor)
                : m_ctor(std::move(ctor))
            { }

            std::unique_ptr<actor_cell> operator()(actor_system & system, const actor_path & path) const noexcept;
        };

        template <typename Ty_, typename ... Args_>
        cell_builder make_cell_builder(Args_ && ... args) {
            return cell_builder([&] { return std::unique_ptr<Ty_>(new Ty_(std::forward<Args_>(args)...)); });
        }
    }

}// namespace actors

}// namespace yato

#endif //_YATO_ACTORS_CELL_BUILDER_H_
