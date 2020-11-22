/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_CMD_PRIVATE_CMD_CONFIG_H_
#define _YATO_CONFIG_CMD_PRIVATE_CMD_CONFIG_H_

#include <map>
#include <string>

#include <tclap/CmdLine.h>

#include "../../config_backend.h"
#include "cmd_value.h"

namespace yato {

namespace conf {

    class cmd_config final
        : public config_backend
    {
    public:
        cmd_config(const std::string & description, const std::string & version)
            : m_cmd(description, ' ', version)
        { }

        cmd_config(const cmd_config&) = delete;

        cmd_config(cmd_config&&) = delete;

        ~cmd_config() = default;

        cmd_config& operator=(const cmd_config&) = delete;

        cmd_config& operator=(cmd_config&&) = delete;

        void add(const std::string & name, std::unique_ptr<cmd_value> && arg)
        {
            m_cmd.add(arg->arg_handle());
            m_args[name] = std::move(arg);
        }

        void parse(int argc, const char* const* argv)
        {
            m_cmd.parse(argc, argv);
            prune_();
        }

        void parse(const std::vector<std::string> & args)
        {
            std::vector<std::string> tmp(args);
            m_cmd.parse(tmp);
            prune_();
        }

    private:
        size_t do_size() const noexcept override
        {
            return m_args.size();
        }

        find_index_result_t do_find(size_t index) const override
        {
            find_index_result_t result = config_backend::no_index_result;
            if (index < m_args.size()) {
                const auto it = std::next(m_args.cbegin(), index);
                result = std::make_tuple((*it).first, (*it).second.get());
            }
            return result;
        }

        find_key_result_t do_find(const std::string& name) const override
        {
            find_key_result_t result = config_backend::no_key_result;
            const auto it = m_args.find(name);
            if (it != m_args.cend()) {
                result = std::make_tuple(yato::narrow_cast<size_t>(std::distance(m_args.cbegin(), it)), (*it).second.get());
            }
            return result;
        }

        void do_release(const config_value* /*val*/) const noexcept override
        {
            // do nothig
        }

        bool do_has_property(config_property p) const noexcept override
        {
            switch (p) {
            case config_property::associative:
                return true;
            default:
                return false;
            }
        }

        void prune_()
        {
            decltype(m_args) pruned_args;
            for (auto & entry : m_args) {
                if (entry.second->valid()) {
                    pruned_args.emplace(entry.first, std::move(entry.second));
                }
            }
            m_args = std::move(pruned_args);
        }

        TCLAP::CmdLine m_cmd;
        std::map<std::string, std::unique_ptr<cmd_value>> m_args;
    };

} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_CMD_PRIVATE_CMD_CONFIG_H_
