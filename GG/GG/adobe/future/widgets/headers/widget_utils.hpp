 /*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_UTILITIES_HPP
#define ADOBE_WIDGET_UTILITIES_HPP

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>

#include <GG/adobe/future/platform_primitives.hpp>

#include <boost/filesystem/path.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

/*!
    \ingroup apl_widgets

    A utility function for the widgets that allows them to have cells
    within the layout sheet that are "unlikely" to be trampled on by
    other widgets. This is accomplished by returning a unique cell name
    to be used by a widget. The unique name is incremented and prepended
    with a builtin zuid to prevent collisions with other cell names.

    \return a name likely to be unique when used as a cell name within a sheet.
*/
name_t state_cell_unique_name();

/****************************************************************************************************/

/*!
    \ingroup apl_widgets

    This utility function takes two rectangle slices and "aligns" them
    by the first point of interest (for a vertical slice, the baseline).
    It takes the first slice as a reference, and modifies it such that
    it's height can accommodate the aligned slices.

    This function does NOT touch frame, inset and outset values! They
    might be invalid once the function has returned.

    \param slice_one the first slice to align, and the one which is modified.
    \param slice_two the slice to align with the first slice.
*/

void align_slices(extents_t::slice_t& slice_one, extents_t::slice_t slice_two);

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

/*!
    \ingroup apl_widgets

    A utility function that hooks into a platform-specific
    implementation to provide for alt-text behavior for a widget.
    Typically this is a "ToolTip" -- a small window that shows up when a
    user hovers over a control with the mouse that gives further detail
    about the property the widget modifies. The implementation is
    platform-specific.

    \param control is the platform widget that will receive the alt-text
    \param alt_text is the text to have shown when further widget description is warranted
*/
void set_control_alt_text(platform_control_type control, const std::string& alt_text);

/****************************************************************************************************/

/*!
    \ingroup apl_widgets

    A utility function that will open a dialog allowing the user to pick
    a file. The picked file must already exist.The implementation is
    platform-specific.

    \param path is the resultant path to the file picked by the user

    \return whether or not \c path is a valid, user-selected path.
*/
bool pick_file(boost::filesystem::path& path);

/****************************************************************************************************/

/*!
    \ingroup apl_widgets

    A utility function that will open a dialog allowing the user to pick
    a path (typically to save a file). The picked file need not already
    exist. The implementation is platform-specific.

    \param path is the resultant path to the file picked by the user

    \return whether or not \c path is a valid, user-selected path.
*/
bool pick_save_path(boost::filesystem::path& path);

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

/*!
    \ingroup apl_widgets

    A utility function that, given a platform display variable (a
    control, dialog, widget, etc.) will return the top-level window that
    contains it. The implementation is platform-specific.

    \param display_element is the element whose parent is to be returned

    \return the parent of the display element passed
*/
platform_display_type get_top_level_window(platform_display_type display_element);

/****************************************************************************************************/

/*!
    \ingroup apl_widgets

    Causes the computer to beep once. Current implementations only exist for Mac and Windows.
*/
inline void system_beep() {}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

// ADOBE_WIDGET_UTILITIES_HPP
#endif

/****************************************************************************************************/
