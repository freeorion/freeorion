/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_TAB_GROUP_FACTORY_HPP
#define ADOBE_TAB_GROUP_FACTORY_HPP

/****************************************************************************************************/

#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/eve.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct widget_node_t;
struct factory_token_t;
class widget_factory_t;

/****************************************************************************************************/

widget_node_t make_tab_group(const dictionary_t&     parameters,
                             const widget_node_t&    parent,
                             const factory_token_t&  token,
                             const widget_factory_t& factory);

/****************************************************************************************************/

inline const layout_attributes_t& tab_group_layout_attributes()
{
    static layout_attributes_t result;
    static bool                       inited(false);

    if (!inited)
    {
        inited = true;

        result.placement_m = eve_t::place_overlay;
        result.vertical().suppress_m = false; // Allow baselines to propagate
        set_margin(result, 10); /* REVISIT FIXED VALUE dialog_margin */
    }

    return result;
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
