/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#include "../cell_builder.h"
#include "../actor.h"
#include "../actor_system.h"
#include "actor_cell.h"
#include "mailbox.h"
#include "properties_internal.h"

namespace yato
{
namespace actors
{
    namespace details {

        std::unique_ptr<actor_cell> cell_builder::operator()(actor_system & system, const actor_path & path, const properties_internal & props) const noexcept {
            std::unique_ptr<actor_cell> res = nullptr;
            try {
                auto cell = std::make_unique<actor_cell>(system, path, props, m_ctor());
                res = std::move(cell);
                system.logger()->verbose("Actor %s is created.", path.c_str());
            }
            catch(std::exception & err) {
                system.logger()->error("cell_builder[operator()]: Error: %s", err.what());
            }
            catch(...) {
                system.logger()->error("cell_builder[operator()]: Unknown exception!");
            }
            return res;
        }

    } // namespace details
    
} // namespace actors

} // namespace yato

