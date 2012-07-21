/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_SEPARATOR_HPP
#define ADOBE_SEPARATOR_HPP

#include <GG/adobe/config.hpp>

#include <boost/utility.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/extents.hpp>
#include <GG/adobe/layout_attributes.hpp>
#include <GG/adobe/widget_attributes.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>


namespace GG {
    class Control;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/
    
struct separator_t : boost::noncopyable, extents_slices_t
{    
    separator_t(bool is_vertical, theme_t theme);
    
    void            measure(extents_t& result);
    
    void            place(const place_data_t& place_data);
    
    void            set_visible(bool make_visible);
    
    GG::Control*    control_m;
    bool            is_vertical_m;
    theme_t         theme_m;
};

/****************************************************************************************************/

}

/****************************************************************************************************/

#endif
