/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/file_monitor.hpp>

#include <GG/adobe/algorithm/find.hpp>
#include <GG/adobe/functional.hpp>

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>

#include <vector>
#include <stdexcept>
#include <utility>

#include <GG/GUI.h>
#include <GG/Timer.h>
#include <GG/Wnd.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

typedef std::vector<adobe::file_monitor_platform_data_t*> file_set_t;

/****************************************************************************************************/

inline file_set_t& file_set()
{
    static file_set_t file_set_s;

    return file_set_s;
}

/****************************************************************************************************/

void timer_callback(unsigned int, GG::Timer*)
{
    file_set_t& set(file_set());

    if (set.empty())
        return;

    for (file_set_t::iterator first(set.begin()), last(set.end());
         first != last;
         ++first)
    {
        if ((*first)->last_write_m == 0)
            continue;

        std::time_t new_write_time =
            boost::filesystem::last_write_time((*first)->path_m);
        if (new_write_time != (*first)->last_write_m)
        {
            (*first)->last_write_m = new_write_time;
            (*first)->proc_m((*first)->path_m,
                             adobe::file_monitor_contents_changed_k);
        }
    }
}

/****************************************************************************************************/

void install_timer()
{
    static bool inited(false);
    static GG::Timer timer(250);

    if (!inited) {
        GG::Connect(timer.FiredSignal, &timer_callback);
        GG::GUI::GetGUI()->RegisterTimer(timer);
        inited = true;
    }
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

file_monitor_platform_data_t::file_monitor_platform_data_t() :
    last_write_m(0)
{ file_set().push_back(this); }

file_monitor_platform_data_t::file_monitor_platform_data_t(const file_monitor_path_type&  path,
                                                           const file_monitor_callback_t& proc) :
    path_m(path),
    proc_m(proc),
    last_write_m(0)
{
    install_timer();

    connect();
}

file_monitor_platform_data_t::file_monitor_platform_data_t(const file_monitor_platform_data_t& rhs) :
    path_m(rhs.path_m),
    proc_m(rhs.proc_m),
    last_write_m(0)
{
    connect();
}

file_monitor_platform_data_t::~file_monitor_platform_data_t()
{
    file_set_t::iterator result(adobe::find(file_set(), this));

    if (result != file_set().end())
        file_set().erase(result); 

    disconnect();
}

file_monitor_platform_data_t& file_monitor_platform_data_t::operator = (const file_monitor_platform_data_t& rhs)
{
    disconnect();

    path_m = rhs.path_m;
    proc_m = rhs.proc_m;

    connect();

    return *this;
}

void file_monitor_platform_data_t::set_path(const file_monitor_path_type& path)
{
    if (!boost::filesystem::exists(path) || path_m == path)
        return;

    install_timer();

    disconnect();

    path_m = path;

    connect();
}

void file_monitor_platform_data_t::connect()
{
    if (!boost::filesystem::exists(path_m))
        return;

    last_write_m = boost::filesystem::last_write_time(path_m);
}

void file_monitor_platform_data_t::disconnect()
{}

/****************************************************************************************************/

file_monitor_t::file_monitor_t()
    { }

file_monitor_t::file_monitor_t(const file_monitor_path_type& path, const file_monitor_callback_t& proc) :
    plat_m(path, proc)
    { }

void file_monitor_t::set_path(const file_monitor_path_type& path)
    { plat_m.set_path(path); }

void file_monitor_t::monitor(const file_monitor_callback_t& proc)
    { plat_m.proc_m = proc; }

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
