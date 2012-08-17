/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_COMMAND_SYSTEM_HPP
#define ADOBE_COMMAND_SYSTEM_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/name.hpp>

#include <boost/signals.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#ifndef NDEBUG
    #include <iostream>
#endif

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

class command_system_t : boost::noncopyable
{
public:
    typedef boost::function<void ()>        command_proc_t;
    typedef boost::function<void (bool)>    command_enabled_callback_t;
    typedef boost::signals::connection      connection_t;

    command_system_t();
    ~command_system_t();

    void insert_command(name_t name, const command_proc_t& proc);
    void remove_command(name_t name);
    void enable_command(name_t name, bool enabled);
    void do_command(name_t name);

    connection_t monitor_enabled(name_t name, const command_enabled_callback_t& callback);

private:
    struct implementation_t;

    implementation_t* object_m;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
