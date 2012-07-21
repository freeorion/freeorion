/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/array.hpp>

#include <boost/function.hpp>
#include <boost/filesystem/path.hpp>

#include <utility>

#include <GG/adobe/future/platform_primitives.hpp>
#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

/*!
\defgroup apl_libraries User-Supported (Platform) Libraries
\ingroup asl_home

\defgroup modal_dialog_kit Modal Dialog Interface Kit
\ingroup apl_libraries

The goal of the modal dialog interface kit is to provide a single point
from which a user can have a dialog brought up and managed by the layout
enging (Eve) and the property model manager (Adam). We also want to
allow for easy "scripting" of the dialog through the interface, as well
as being able to remember and reuse dialog-specific state information.
There are several structs to support the one API through which
interaction takes place.
*/

/****************************************************************************************************/
/*!
\ingroup modal_dialog_kit

Display options control how the dialog is to be used during the playback
of scripted command parameters.
*/
enum display_options_t
{
    /*!
        Show the dialog to the user in any instance
    */
    dialog_display_s,

    /*!
        Do not show the dialog to the user unless the command parameters
        are invalid for the model as currently specified. This allows
        for the user to modify the parameters and verify them before
        moving them on to the command.
    */
    dialog_no_display_s,

    /*!
        Do not show the user the dialog for any reason. In this instance
        when the parameter are invalid, error out of the playback.
    */
    dialog_never_display_s
};

/****************************************************************************************************/
/*!
\ingroup modal_dialog_kit

This is the struct returned by the handle_dialog API. 
*/
struct dialog_result_t
{
    dialog_result_t& operator=(const dialog_result_t& x)
    {
        record_m                = x.record_m;
        display_state_m         = x.display_state_m;
        command_m               = x.command_m;
        terminating_action_m    = x.terminating_action_m;
        return *this;
    }
    /*!
        The set and touched record pair that need to be stored in order
        to script the dialog to re-execute the command that is being
        returned (also in this struct). Note that this record
        information stores the "intention" of the user, and may vary
        greatly from the actual values needed by the command in order to
        execute. This information is more "accurate" than the parameters
        needed for this instance of the execution because the input
        values may vary for a command from instance to instance, but
        retaining the intention of the user assures that they get what
        they want out of the command based on the parameters they
        explicitly set. Note that this information can be round-tripped
        over multiple instantiations of the handle_dialog API, resulting
        in a new instance of the dialog having the same intent with
        which the previous instance left off (aka previous values are
        retained).
    */
    dictionary_t record_m;

    /*!
        Retained information as it pertains to the visual layout of the
        view (window location, size, etc.) and also as it pertains to
        the basic model of the view (what tab(s) are currently selected,
        what the state of other basic sheet cells are, etc.)
    */
    dictionary_t display_state_m;

    /*!
        Contains the complete parameter set necessary to execute the
        function of which this dialog is intended to collect the
        parameters. While using handle_dialog it is assumed that there
        exists in the Adam model an output cell called result that will
        be fetched and whose value will be stuffed into this dictionary.
        Note the implication there is that the result cell itself is a
        dictionary of values.
    */
    dictionary_t command_m;

    /*!
        The name of the action that was used to terminate (close) the
        dialog. See the Action Callback section of this documentation
        for more information on how this value is set at runtime.
    */
    name_t terminating_action_m;
};

/****************************************************************************************************/
/*!
    \ingroup modal_dialog_kit

    The modal dialog interface now sports an API call back to the client
    in the case when a button is pushed. In the Eve definition of the
    dialog there must be an action parameter and a bind statement to
    notify the implementation what Adam cell is to be bound to this
    button. In the case when the button is clicked this callback is
    handed the action value and the Adam cell contents of the bound Adam
    cell. In return the callback should send back a bool specifying
    whether or not to close the dialog. If the callback returns true,
    the terminating_action_m field in the dialog return results will be
    this action name.
*/
typedef boost::function <bool (name_t, const any_regular_t&)> action_callback_t;

/****************************************************************************************************/
/*!
    \ingroup modal_dialog_kit

    The class that does the heavy-lifting for the modal dialog kit.
*/
class modal_dialog_t
{
public:
    typedef any_regular_t               model_type;
    typedef auto_ptr<eve_client_holder> auto_view_t;

    modal_dialog_t();

    dialog_result_t go(std::istream& layout, std::istream& sheet);

    dictionary_t            input_m;
    dictionary_t            record_m;
    dictionary_t            display_state_m;
    display_options_t       display_options_m;
    action_callback_t       callback_m;
    boost::filesystem::path working_directory_m;
    platform_display_type   parent_m;
    vm_lookup_t             vm_lookup_m;

    /* Clients: no need to call these. */
    bool end_dialog();
    void display(const model_type& value);

private:
    void              latch_callback(name_t action, const any_regular_t&);

    void              monitor_record(const dictionary_t& record_info);
    void              monitor_invariant(bool valid);

    sheet_t           sheet_m;
    behavior_t        root_behavior_m;
    auto_view_t       view_m;
    bool              defer_view_close_m;
    bool              need_ui_m;
    dialog_result_t   result_m;
    dictionary_t      contributing_m;
};

/****************************************************************************************************/
/*!
    \ingroup modal_dialog_kit

    One function call to display a model dialog (or playback a script).
    We might also want to consider a preview function call back (that
    will get called with the same command dictionary anytime something
    in the sheet changes).

    \param input contains the input values for input cells as they are
    defined in the sheet. Each key in this dictionary should correspond
    to an input cell in the sheet to which the relevant value will be
    imposed. This can be a default-constructed dictionary, in which case
    input cells will be set to the values defined in their sheet
    initializers.

    \param record is the previously recorded script information for the
    dialog, to be used to execute the dialog when running from an action
    or to retain the previoud instance of the dialog's values for the
    new dialog instance.

    \param display_state is the previously recorded layout state
    information for the dialog.

    \param display_options is one of the three enumerations specified
    above, according to which semantic behavior you would like to get
    out of this routine.

    \param layout_definition is a stream that will be parsed as the Eve
    definition for this dialog.

    \param sheet_definition is a stream that will be parsed as the Adam
    definition for this dialog. Note that the only requirement on the
    Adam sheet is that there exist an output cell called result that is
    defined to be a dictionary of values. The value of the result cell
    will be handed back in the command_m field of the dialog_result_t
    upon return from this procedure.

    \param callback is the function proc that is called when a button in
    the modal dialog is invoked by the user. The two parameters to the
    callback are the action name of the button as defined in the Eve
    definition, along with the contents of the Adam cell to which the
    button is bound. The boolean return value specifies whether or not
    the modal dialog interface should terminate the dialog.

    \param working_directory is the directory from which the dialog
    widgets should use to fetch disk-based resources. Whether or not
    this is necessary is predicated on the resource requirements of the
    widgets used in the dialog.

    \param parent is the parent widget that will 'own' the dialog that
    is about to be created. This notion is purely a platform-specific
    one, and is not a requirement as far as ASL goes.

    \return a filled in dialog_result_t.
*/
inline dialog_result_t handle_dialog(const dictionary_t&            input,
                                     const dictionary_t&            record,
                                     const dictionary_t&            display_state,
                                     display_options_t              display_options,
                                     std::istream&                  layout_definition,
                                     std::istream&                  sheet_definition,
                                     action_callback_t              callback,
                                     const boost::filesystem::path& working_directory,
                                     platform_display_type          parent=platform_display_type())
{
    assert ( !layout_definition.fail() );
    assert ( !sheet_definition.fail() );

    modal_dialog_t dialog;

    dialog.input_m = input;
    dialog.record_m = record;
    dialog.display_state_m = display_state;
    dialog.display_options_m = display_options;
    dialog.callback_m = callback;
    dialog.working_directory_m = working_directory;
    dialog.parent_m = parent;

    return dialog.go(layout_definition, sheet_definition);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
