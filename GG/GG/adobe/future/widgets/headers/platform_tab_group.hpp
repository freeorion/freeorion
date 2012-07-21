/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_TAB_GROUP_HPP
#define ADOBE_TAB_GROUP_HPP

#include <GG/adobe/config.hpp>

#include <vector>

#include <boost/utility.hpp>
#include <boost/function.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/extents.hpp>
#include <GG/adobe/layout_attributes.hpp>
#include <GG/adobe/widget_attributes.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>


namespace GG {
    class TabBar;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct tab_group_t : boost::noncopyable
{
    typedef any_regular_t model_type;
    typedef tab_group_t  widget_type_t;
    typedef boost::function<void (const any_regular_t&)> tab_group_value_proc_t;

    struct tab_t
    {
        std::string      name_m;
        any_regular_t    value_m;
    };

    typedef std::vector<tab_t>  tab_set_t;
    
    tab_group_t(const tab_t* first, const tab_t* last, theme_t theme);


    void        measure(extents_t& result);
    void        place(const place_data_t& place_data);

    void        display(const any_regular_t& new_value);
    void        monitor(const tab_group_value_proc_t& proc);

    void        enable(bool make_enabled);


    GG::TabBar*             control_m;
    theme_t                 theme_m;
    tab_group_value_proc_t  value_proc_m;
    tab_set_t               items_m;
};

/****************************************************************************************************/

} //namespace adobe

/****************************************************************************************************/

#endif 
