/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_WINDOW_SERVER_HPP
#define ADOBE_WIDGET_WINDOW_SERVER_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/adam.hpp>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/basic_sheet.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/eve.hpp>
#include <GG/adobe/future/assemblage.hpp>
#include <GG/adobe/future/debounce.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/istream.hpp>
#include <GG/adobe/memory.hpp>
#include <GG/adobe/name.hpp>

#include <boost/function.hpp>
#include <boost/filesystem/path.hpp>

#include <list>
#include <vector>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*
    This is a basic "Window Server" - it can be called to construct a window by name. The window
    is attached to a sheet as a slave (meaning the Window must be destructed before the sheet).

    REVISIT (sparent) : Here are some thoughts on the direction this design should be going...
    
    We have an assemblage (or package of items with the same lifespace) for the window which
    includes the eve structure and the slave connection (the wire bundle to attach the
    assamblage to a sheet).
*/

struct eve_client_holder;

//
/// The window_server_t class can open Eve definitions from file and
/// input stream, and can display them. It also looks after all of
/// the memory allocated to display an Eve definition.
//
class window_server_t
{
public:
    typedef boost::function<void (const name_t&, const any_regular_t&)> action_fallback_proc_t;
    //
    /// This constructor tells the window_server_t where to find
    /// Eve definitions referenced in @dialog commands, and which
    /// sheet to bind against.
    ///
    /// \param  sheet       the sheet to bind against. This
    ///             sheet should contain all of the
    ///             cells referenced in the Eve file
    ///             which is loaded (either via
    ///             push_back or @dialog).
    //
    window_server_t(sheet_t& sheet, behavior_t& behavior);
    //
    /// Hide and release all Eve dialogs created by this window server.
    //
    ~window_server_t();
    //
    /// Load the given file out of the directory_path and display the
    /// dialog it contains. If the file cannot be found, or the file
    /// contains errors (e.g.: syntax errors, references non-existant
    /// widgets) then an exception is thrown.
    ///
    /// \param  name the name of the Eve file inside
    ///             the directory given to the constructor.
    /// \param  dialog_size the size of the dialog to create,
    ///             note that individual widgets may
    ///             override this value in the Eve file.
    //
    void push_back(const char* name, size_enum_t dialog_size);
    //
    /// Load an Eve definition from the given std::istream and display
    /// the dialog it contains. If any errors are found in the data
    /// then an exception is thrown.
    ///
    /// \param  data an std::istream open on the Eve
    ///             definition to be loaded.
    /// \param  ident_pair identification name and callback proc (two parameters)
    ///             that represents the data stream
    /// \param  dialog_size the size of the dialog to create,
    ///             note that individual widgets may
    ///             override this value in the Eve
    ///             definition.
    //
    void push_back(std::istream& data, const boost::filesystem::path& file_path, const line_position_t::getline_proc_t& getline_proc, size_enum_t dialog_size);
    
    #if 0
    void set_back(const char* name, size_enum_t dialog_size);
    void pop_back(bool cancel);
    #endif
    //
    /// The window server can use different widget factories if
    /// custom widgets need to be made. By default the window
    /// server will use the "default_factory" function,
    /// but an alternative function can be specified here. Note
    /// that push_back just uses the current factory, so if a
    /// custom factory is desired then it must be set before
    /// using push_back.
    ///
    /// \param factory the new widget factory to use.
    //
    void set_widget_factory(const widget_factory_t& factory)
    { widget_factory_m = factory; }
    //
    /// Some widgets, such as buttons in UI
    /// core, have an action property. When an action is invoked
    /// (e.g.: a button with an action value) the procedure given
    /// to this function is invoked. This provides an easy way
    /// to get feedback from a GUI.
    ///
    /// Note that some actions are automatically handled, these
    /// are: @cancel, @ok and @reset.
    ///
    /// \param  proc    the function to call when a widget
    ///         with an action is "activated".
    //
    void set_action_fallback(action_fallback_proc_t proc);    
    // REVISIT (sparent) : Hack. These need to go to the correct window.
    //
    /// This function can be used to send an action to the open windows,
    /// however it is only useful in debug mode (where it can be used to
    /// frame and unframe visible widgets). It will probably be changed
    /// soon.
    ///
    /// \param  action      the action to perform.
    /// \param  parameter   the parameter associated with the action.
    //
    void dispatch_action(name_t action, const any_regular_t& parameter);
    //
    /// Return the number of windows which this window_server has open.
    ///
    /// \return the number of windows which this window_server has open.
    //
    std::size_t size() const { return window_list_m.size(); }

    //
    /// \return The top eve client holder in the window list
    ///
    //
    eve_client_holder& top_client_holder();

private:
    typedef std::list<eve_client_holder*>   window_list_t;
    typedef window_list_t::iterator         iterator;
    
    void dispatch_window_action(iterator window, name_t action, const any_regular_t& parameter);
    void erase(iterator window);
    
    sheet_t&               sheet_m;
    behavior_t&            behavior_m;
    window_list_t          window_list_m;
    action_fallback_proc_t fallback_m;
    widget_factory_t       widget_factory_m;
};

/*************************************************************************************************/

}

/*************************************************************************************************/

#endif
