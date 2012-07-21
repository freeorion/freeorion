/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/future/cmd_system.hpp>

#include <boost/signals.hpp>

#include <stdexcept>
#include <map>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct command_system_t::implementation_t
{
    struct command_entry_t
    {
        typedef boost::signal<void (bool)> enable_callback_list_t;

        command_entry_t() :
            enabled_m(false)
            { }

        command_entry_t(const command_entry_t& rhs) :
            enabled_m(rhs.enabled_m)
            { }

        command_entry_t& operator = (const command_entry_t& rhs)
        {
            enabled_m = rhs.enabled_m;

            return *this;
        }

        command_proc_t          proc_m;
        bool                    enabled_m;
        enable_callback_list_t  enabled_callback_m;
    };

    typedef std::map<name_t, command_entry_t> command_map_t;

    void insert_command(name_t name, const command_proc_t& proc);

    void remove_command(name_t name);

    void enable_command(name_t name, bool enabled);

    void do_command(name_t name);

    connection_t monitor_enabled(name_t name, const command_enabled_callback_t& callback);

private:
    command_map_t   command_map_m;
};

/****************************************************************************************************/

command_system_t::command_system_t() :
    object_m(new implementation_t())
    { }

command_system_t::~command_system_t()
    { delete object_m; }

void command_system_t::insert_command(name_t name, const command_proc_t& proc)
    { object_m->insert_command(name, proc); }

void command_system_t::remove_command(name_t name)
    { object_m->remove_command(name); }

void command_system_t::enable_command(name_t name, bool enabled)
    { object_m->enable_command(name, enabled); }

void command_system_t::do_command(name_t name)
    { object_m->do_command(name); }

command_system_t::connection_t command_system_t::monitor_enabled(name_t name, const command_enabled_callback_t& callback)
    { return object_m->monitor_enabled(name, callback); }

/****************************************************************************************************/

void command_system_t::implementation_t::insert_command(name_t name, const command_proc_t& proc)
{
    if (command_map_m.find(name) != command_map_m.end())
        throw std::runtime_error("A command by this name already exists");

    command_entry_t& cmd(command_map_m[name]);

    cmd.proc_m = proc;
}

/****************************************************************************************************/

void command_system_t::implementation_t::remove_command(name_t name)
{
    command_map_t::iterator item(command_map_m.find(name));

    if (item == command_map_m.end())
        throw std::runtime_error("A command by this name could not be found");

    command_map_m.erase(item);
}

/****************************************************************************************************/

void command_system_t::implementation_t::enable_command(name_t name, bool enabled)
{
    if (command_map_m.find(name) == command_map_m.end())
        throw std::runtime_error("A command by this name could not be found");

    command_entry_t& cmd(command_map_m[name]);

    if (cmd.enabled_m == enabled) return;

    cmd.enabled_m = enabled;

    cmd.enabled_callback_m(cmd.enabled_m);
}

/****************************************************************************************************/

void command_system_t::implementation_t::do_command(name_t name)
{
    if (command_map_m.find(name) == command_map_m.end())
        throw std::runtime_error("A command by this name could not be found");

    command_entry_t& cmd(command_map_m[name]);

    if (!cmd.enabled_m) return;

    cmd.proc_m();
}

/****************************************************************************************************/

command_system_t::connection_t command_system_t::implementation_t::monitor_enabled(name_t name, const command_enabled_callback_t& callback)
{
    if (command_map_m.find(name) == command_map_m.end())
        throw std::runtime_error("A command by this name could not be found");

    command_entry_t& cmd(command_map_m[name]);

    return cmd.enabled_callback_m.connect(callback);
}

/****************************************************************************************************/

}// namespace adobe

/****************************************************************************************************/
