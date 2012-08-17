/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_OPTIONAL_PANEL_HPP
#define ADOBE_WIDGET_OPTIONAL_PANEL_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/extents.hpp>
#include <GG/adobe/layout_attributes.hpp>
#include <GG/adobe/widget_attributes.hpp>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <GG/adobe/future/widgets/headers/platform_panel.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct optional_panel_t : boost::noncopyable
{
    typedef any_regular_t model_type;
    typedef boost::function<void (const boost::function<void ()>&)> optional_display_proc_t;

                        optional_panel_t(const any_regular_t& show_value,
                                         theme_t              theme);

    void                measure(extents_t& result);

    void                place(const place_data_t& place_data);

    void                display(const any_regular_t& value);

    void                set_optional_display_procs(const optional_display_proc_t& show_proc,
                                                   const optional_display_proc_t& hide_proc)
    {
        show_proc_m = show_proc;
        hide_proc_m = hide_proc;
    }

    panel_t                 control_m;
    optional_display_proc_t show_proc_m;
    optional_display_proc_t hide_proc_m;
    bool                    inited_m;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
