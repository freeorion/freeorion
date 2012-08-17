/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <cassert>

#include <GG/Wnd.h>
#include <GG/GUI.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

display_t& get_main_display()
{
    static display_t display_s;

    return display_s;
}

/****************************************************************************************************/

template <typename DisplayElement>
platform_display_type insert(display_t& display, platform_display_type& parent, DisplayElement& element)
{ return display.insert(parent, element); }

/****************************************************************************************************/

#if 0
    #pragma mark -
#endif

/****************************************************************************************************/

platform_display_type display_t::insert(platform_display_type parent,
                                        platform_display_type element)
{
    static const platform_display_type null_parent_s = platform_display_type();

    if (parent != null_parent_s)// TODO && parent != get_main_display().root())
        parent->AttachChild(element);
    else if (!element->Modal())
        GG::GUI::GetGUI()->Register(element);

    return element;
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
