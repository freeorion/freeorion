/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_UI_CORE_FACTORY_HPP
#define ADOBE_UI_CORE_FACTORY_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/adam.hpp>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/basic_sheet.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/eve_parser.hpp>
#include <GG/adobe/eve.hpp>
#include <GG/adobe/future/assemblage.hpp>
#include <GG/adobe/future/behavior.hpp>
#include <GG/adobe/future/debounce.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/visible_change_queue.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/istream.hpp>
#include <GG/adobe/keyboard.hpp>
#include <GG/adobe/memory.hpp>
#include <GG/adobe/name.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/function.hpp>

#include <list>
#include <vector>

/*************************************************************************************************/
//
/// This file defines the factory functions which are used to create widgets specified by
/// the Eve file. The main factory function "default_factory" can be passed to window_server_t
/// as the factory function to create widgets. All it does is call the other factory functions
/// defined here to create specific widgets.
///
/// The motivation behind having the factory split out into a function for each widget, is that
/// it should be really easy for application developers to add custom attributes in Eve for
/// widgets that interest them.
///
/// In order to avoid complexities with passing type information around, the factory methods all
/// insert the widget they create into the given parent. They also register with the assemblage
/// in the factory token to be destroyed when the assemblage is destroyed.
//
/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

struct eve_client_holder : public boost::noncopyable
{
    eve_client_holder(behavior_t& sheet_scope_root_behavior) :
        visible_change_queue_m(eve_m),
        root_behavior_m(sheet_scope_root_behavior),
                root_display_m()
    {
        eve_eval_token_m = root_behavior_m.insert(boost::bind(&visible_change_queue_t::update,
                                                              boost::ref(visible_change_queue_m)));
    }


    ~eve_client_holder()
    {
        root_behavior_m.disconnect(eve_eval_token_m);
    }

    /*
    NOTE (sparent) : Order is important here - we want to destruct window_m prior to destructing
    eve_m so that any notifications sent by the system are sent to valid object. Slots and
    signals on the window widgets would be a better way to handle this connection but I'm not
    there yet.
    */

    //
    /// The Eve engine which all of these widgets are being inserted into. This must
    /// be known by top-level windows, and by widgets which manage trees of Eve
    /// widgets (such as splitter widgets).
    //
    eve_t                                    eve_m;

    //
    /// The layout sheet is a "mini" sheet used to hold the layout state.
    //
    basic_sheet_t                            layout_sheet_m;
    
    //
    /// This is the assemblage which widget factories register the created widget
    /// (or wrapping structure) with, such that the widget gets deleted when the
    /// assemblage does.
    //
    assemblage_t                             assemblage_m;

    //
    /// A set for the contributing factors to the sheet (not the layout sheet)
    //
    dictionary_t                             contributing_m;

    //
    /// Top-level widgets (windows, dialogs, etc) need to be told to show themselves
    /// when all of the child widgets have been created and inserted. This signal
    /// is issued when the top-level window needs to be shown -- so any factory function
    /// which creates a window needs to connect to this signal.
    //
    boost::signal<void ()>                   show_window_m;

    //
    /// REVISIT (sparent) : We really need a generalized mechanism for deferring an action -
    /// a command queue of sorts - which collapses so things don't get done twice. We hack it here.
    //
    visible_change_queue_t                   visible_change_queue_m;
    
    //
    /// Relayout is complicated. We need to maintain a visible update queue (VUQ) per-window,
    /// as we want to know when a window has hide/show elements in their respective queues;
    /// this helps to eliminate unnecessary calls to eve_t::evaluate for a given window if none
    /// is needed. However, we also need to call _all_ the VUQ update routines related to a
    /// given sheet- this is needed to make sure all the views are updated w.r.t. the state of the
    /// sheet so there are no hide/show sync issues. This reference is held to the 'root behavior',
    /// the one scoped the same as the sheet to which this view will be bound, and the behavior
    /// called when a user action requires us to check the VUQ set for the sheet for potential
    /// relayout.
    //
    behavior_t&                               root_behavior_m;

    //
    /// Used by the eve_client_holder to disconnect itself from the root behavior.
    //
    behavior_t::verb_token_t                  eve_eval_token_m;

    //
    /// REVISIT (sparent) : We really need a generalized mechanism for deferring an action -
    /// a command queue of sorts - which collapses so things don't get done twice. We hack it here.
    //
    behavior_t*                              my_behavior_m;

    //
    /// Display token for the root item in the view
    //
    platform_display_type                    root_display_m;

    //
    /// Path to the file loaded for this window
    //
    boost::filesystem::path                  path_m;
};

/*************************************************************************************************/

typedef boost::function<void (name_t action, const any_regular_t&)> button_notifier_t;

/****************************************************************************************************/

/*
    REVISIT (sparent) : Size is inherited from the parent during initialization - the size
    specified to the client is relative to the parent's size. Does this ever need to
    be specified in absolutes?
*/

enum size_enum_t
{
    size_mini_s     = -2,
    size_small_s    = -1,
    size_normal_s   = 0,

    size_minimum_s  = size_mini_s,
    size_maximum_s  = size_normal_s
};

/*************************************************************************************************/

//
/// This structure is given to all factory functions, so that they can correctly create
/// the widgets they need to create. Things inside this structure should not change
/// between creating two widgets.
//
struct factory_token_t
{
    factory_token_t(display_t&                display,
                    sheet_t&                  sheet,
                    eve_client_holder& client_holder,
                    button_notifier_t  notifier) :
        display_m(display),
        sheet_m(sheet),
        client_holder_m(client_holder),
        notifier_m(notifier)
    { }

    //
    /// Created widgets should be made with respect to this display, and inserted
    /// into the given parent by this display.
    //
    display_t& display_m;

    //
    /// The current Adam sheet -- this contains all of the cells which widgets might
    /// want to bind against. 
    //
    sheet_t& sheet_m;

    //
    /// The current eve_client_holder -- keeps all the relevant parts of a view in
    /// one location. Originally the factory token was duplication nearly every
    /// member of the client_holder as a member itself, so this cleans up factory_token
    /// quite a bit.
    //
    eve_client_holder& client_holder_m;

    //
    /// The function to call when a button is pressed. This should be called by
    /// buttons and button-like widgets when they are hit (and have an action,
    /// rather than a state change).
    //
    button_notifier_t notifier_m;
};

/*************************************************************************************************/
//
/// This structure represents a widget registered into Eve and the display. If a widget is
/// a parent then this information is used for inserting child widgets and correctly calculating
/// sizes.
//
struct widget_node_t
{
    widget_node_t(  size_enum_t                     size,
                    const eve_t::iterator&          eve_token,
                    const platform_display_type&    display_token,
                    const keyboard_t::iterator&     keyboard_token
                    ) :
        size_m(size),
        eve_token_m(eve_token),
        display_token_m(display_token),
        keyboard_token_m(keyboard_token)
    { } 
    //
    /// This specifies the size of this widget. Children will use this to find the
    /// size they should use (unless they have an explicit size specified).
    //
    size_enum_t size_m;
    //
    /// The parent as known by Eve.
    //
    eve_t::iterator eve_token_m;
    //
    /// The widget as known by the display.
    /// This should *not* be mutable -- for now it's more convenient.
    //
    mutable platform_display_type display_token_m;

    /// The widget as known by the keyboard.
    //
    keyboard_t::iterator keyboard_token_m;

};

/*************************************************************************************************/
//
/// Create a row container with the given parameters, and insert it into the
/// given parent.
///
/// \param  parameters  a dictionary of parameters for the row.
/// \param  parent      the parent of the row.
/// \param  token       a factory token, containing approprate references.
///
/// \return the node information for the created row. There is nothing to modify
///     once the row has been created, so the general widget_info_t is not
///     returned.
//
widget_node_t row_factory(const dictionary_t& parameters, const widget_node_t& parent, const factory_token_t& token, const widget_factory_t& factory);

/*************************************************************************************************/
//
/// Create a column container with the given parameters, and insert it into the
/// given parent.
///
/// \param  parameters  a dictionary of parameters for the column.
/// \param  parent      the parent of the column.
/// \param  token       a factory token, containing approprate references.
///
/// \return the node information for the created column. There is nothing to modify
///     once the column has been created, so the general widget_info_t is not
///     returned.
//
widget_node_t column_factory(const dictionary_t& parameters, const widget_node_t& parent, const factory_token_t& token, const widget_factory_t& factory);

/*************************************************************************************************/
//
/// Create an overlay container with the given parameters, and insert it into the
/// given parent.
///
/// \param  parameters  a dictionary of parameters for the column.
/// \param  parent      the parent of the column.
/// \param  token       a factory token, containing approprate references.
///
/// \return the node information for the created overlay. There is nothing to modify
///     once the overlay has been created, so the general widget_info_t is not
///     returned.
//
widget_node_t overlay_factory(const dictionary_t& parameters, const widget_node_t& parent, const factory_token_t& token, const widget_factory_t& factory);

/*************************************************************************************************/
//
/// Includes all the ASL widgets library builtin widgets. The factories are regular; clients are
/// allowed to make copies of the default factory and then modify it for custom purposes.
/// Note, however, that some ASL widget implementations expect other ASL implementations to exist
/// (e.g., edit_number has dependencies on edit_text, label, etc.)
//
const widget_factory_t& default_asl_widget_factory();

/*************************************************************************************************/

inline widget_node_t default_factory(const widget_factory_t& factory,
                                     name_t                  widget_type,
                                     const dictionary_t&     parameters,
                                     const widget_node_t&    parent,
                                     const factory_token_t&  token)
{
    return factory.method(widget_type)(parameters, parent, token, factory);
}

/*************************************************************************************************/

typedef boost::function<widget_node_t (name_t                 widget_type,
                                       const dictionary_t&    parameters,
                                       const widget_node_t&   parent,
                                       const factory_token_t& token)> widget_factory_proc_t;

/*************************************************************************************************/

inline widget_factory_proc_t default_widget_factory_proc_with_factory(const widget_factory_t& factory)
{
    return boost::bind(&default_factory,
                       boost::cref(factory),
                       _1, _2, _3, _4);
}

/*************************************************************************************************/

inline widget_factory_proc_t default_widget_factory_proc()
{ return default_widget_factory_proc_with_factory(default_asl_widget_factory()); }

/*************************************************************************************************/

adobe::auto_ptr<eve_client_holder> make_view(name_t                                 file_name,
                                             const line_position_t::getline_proc_t& getline_proc,
                                             std::istream&                          stream,
                                             sheet_t&                               sheet,
                                             behavior_t&                            root_behavior,
                                             const button_notifier_t&               notifier,
                                             size_enum_t                            dialog_size,
                                             const widget_factory_proc_t&           proc = default_widget_factory_proc(),
                                             platform_display_type                  display_root=platform_display_type());

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/

