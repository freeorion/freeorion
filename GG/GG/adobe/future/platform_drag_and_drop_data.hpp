/*
    Copyright 2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#ifndef ADOBE_DRAG_AND_DROP_PLATFORM_DATA_HPP
#define ADOBE_DRAG_AND_DROP_PLATFORM_DATA_HPP

/**************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/cstdint.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <GG/adobe/algorithm/find.hpp>
#include <GG/adobe/future/drag_and_drop_fwd.hpp>

#include <vector>


namespace GG {
    class Wnd;
}

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/

template <typename SourceType, typename TargetType>
class drag_and_drop_handler_platform_data : boost::noncopyable
{
public:
    typedef SourceType                                 source_type;
    typedef TargetType                                 target_type;
    typedef boost::function<void (const target_type&)> client_callback_proc_t;

    drag_and_drop_handler_platform_data() :
        chosen_flavor_m(flavor_invalid),
        widget_m(0)
    { }

    template <typename InputIterator>
    drag_and_drop_handler_platform_data(InputIterator first, InputIterator last) :
        flavor_set_m(first, last),
        chosen_flavor_m(flavor_invalid),
        widget_m(0)
    { }

    void init()
    { }

    template <typename Client>
    void attach(const Client& control, const client_callback_proc_t& proc)
    {
        set_callback(proc);

        install(control);
    }

    void detach()
    {
        uninstall();
    }

    void insert_flavor(boost::uint32_t flavor)
    {
        if (adobe::find(flavor_set_m, flavor) == flavor_set_m.end())
            flavor_set_m.push_back(flavor);
    }

    void erase_flavor(boost::uint32_t flavor)
    { flavor_set_m.erase(adobe::find(flavor_set_m, flavor)); }

    void set_callback(const client_callback_proc_t& proc)
    { callback_m = proc; }

    void set_accepts_files(bool does_accept)
    {
#if 0 // TODO
        if (widget_m)
            ::DragAcceptFiles(widget_m, does_accept);
#endif
    }

private:
    void install(GG::Wnd* widget)
    {
        assert (widget_m == 0);
        widget_m = widget;
    }

    void uninstall()
    {
        assert (widget_m != 0);
        set_accepts_files(false);
        widget_m = 0;
    }

    client_callback_proc_t       callback_m;
    std::vector<boost::uint32_t> flavor_set_m;
    boost::uint32_t              chosen_flavor_m;
    GG::Wnd*                     widget_m;
};

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

// ADOBE_DRAG_AND_DROP_PLATFORM_DATA_HPP
#endif

/**************************************************************************************************/
