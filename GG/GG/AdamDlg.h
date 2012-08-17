// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */
   
/** \file AdamDlg.h \brief Contains a modal dialog interface kit that provides
    a single point from which a user can have a dialog brought up and managed
    by the property model manager (Adam). We also want to allow for easy
    filling of the dialog through the interface, as well as being able to
    remember and reuse dialog-specific state information.  There are several
    structs to support the one API through which interaction takes place. */

#ifndef _AdamDlg_h_
#define _AdamDlg_h_

#include <GG/AdamGlue.h>

#include <GG/adobe/adam.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>

#include <boost/function.hpp>
#include <boost/filesystem/path.hpp>


namespace GG {

class Wnd;

/** Display options to control whether a dialog is to be used during the
    playback of scripted command parameters. */
enum AdamDialogDisplayOption
{
    /** Show the dialog to the user in any instance */
    ADAM_DIALOG_DISPLAY_ALWAYS,

    /** Do not show the dialog to the user unless the command parameters are
        invalid for the model as currently specified. This allows for the user
        to modify the parameters and verify them before moving them on to the
        command. */
    ADAM_DIALOG_DISPLAY_AS_NEEDED,

    /** Do not show the user the dialog for any reason. In this instance when
        the parameter are invalid, error out of the playback. */
    ADAM_DIALOG_DISPLAY_NEVER
};

/** This is the struct returned by HandleAdamDialog(). */
struct AdamDialogResult
{
    /** The set and touched record pair that need to restore the previous
        state of the dialog's property (i.e. Adam) sheet.  Note that this
        information can be round-tripped over multiple executions of
        HandleAdamDialog(), resulting in a new execution of the dialog with
        the same state as the previous execution (i.e. values are retained
        execution to execution). */
    adobe::dictionary_t m_property_state;

    /** Contains the complete set of output values produced by this dialog.
        When using HandleAdamDialog(), it is assumed that there exists in the
        Adam model an output cell called \a result that will be fetched, and
        that \a result's value will be stuffed into this dictionary.  Note
        that this implies that the result cell itself is a dictionary of
        values. */
    adobe::dictionary_t m_result_values;

    /** The name of the action that was used to terminate (close) the
        dialog. See the Action Callback section of this documentation for more
        information on how this value is set at runtime. */
    adobe::name_t m_terminating_action;
};

/** The modal dialog interface includes an API to call back to the client when
    a UI interaction occurs.  In the definition of the GG dialog, there must
    be a slot that binds UI interactions (usually button presses, like Apply,
    Ok, or Cancel) to the generation of (adobe::name_t, adobe::any_regular_t)
    pairs.  When such a UI interaction occurs, this callback is given the
    action name and value.  The callback returns a bool specifying whether or
    not to close the dialog.  If the callback returns true, the
    m_terminating_action field in the resulting AdamDialogResult will be the
    given action name. */
typedef boost::function <bool (adobe::name_t, const adobe::any_regular_t&)> AdamDialogActionCallback;

/** The class that does the heavy lifting for the Adam modal dialog system. */
class AdamModalDialog
{
public:
    /** Ctor.

        \param sheet_definition is a stream that will be parsed as the Adam
        sheet definition for this dialog.  Note that the only requirement on
        the Adam sheet is that there exist an output cell called \a result
        that is defined to be a dictionary of values. The value of the result
        cell will be handed back in the m_result_values field of the
        AdamDialogResult upon return from this procedure.

        \param input contains the input values for input cells as they are
        defined in the sheet. Each key in this dictionary should correspond to
        an input cell in the sheet to which the relevant value will be
        imposed. This can be a default-constructed dictionary, in which case
        input cells will be set to the values defined in their sheet
        initializers.

        \param previous_property_state is the previously recorded property
        sheet state for the dialog, to be used to execute the dialog when
        running from an action or to retain the previoud instance of the
        dialog's values for the new dialog instance.

        \param display_option is one of the three enumerations specified
        above, according to which semantic behavior you would like to get out
        of this routine.

        \param dlg is the GG::Wnd that will act as the user-interactable
        dialog.

        \param callback is the function proc that is called when a UI
        interaction in the modal dialog is performed by the user.  \see
        AdamDialogActionCallback.

        \param working_directory is the directory from which the dialog
        widgets should use to fetch disk-based resources. Whether or not this
        is necessary is predicated on the resource requirements of the widgets
        used in the dialog. */
    AdamModalDialog(const std::string& sheet_definition,
                    const adobe::dictionary_t& input,
                    const adobe::dictionary_t& previous_property_state,
                    AdamDialogDisplayOption display_option,
                    GG::Wnd* dlg,
                    AdamDialogActionCallback callback,
                    boost::filesystem::path working_directory);

    /** Returns true if the GG::Wnd dialog should be run, based on the \a
        display_option, and \a previous_property_state parameters passed to
        the ctor. */
    bool NeedUI() const;

    /** Binds a GG control to the Adam property sheet maintained by *this. */
    template <
        class AdamValueType,
        class GGValueType,
        class ControlType
    >
    void BindCell(ControlType& control, adobe::name_t cell);

    /** Returns the AdamDialogResult results struct. */
    AdamDialogResult Result();

    /** Connect UI interaction signals from the GG::Wnd to this signal to have
        them handled by AdamDialogActionCallback passed to the ctor. */
    boost::signal<void (adobe::name_t, const adobe::any_regular_t&)> DialogActionSignal;

private:
    void LatchCallback(adobe::name_t action, const adobe::any_regular_t& value);
    void MonitorPropertyState(const adobe::dictionary_t& property_state);
    void MonitorInvariants(bool valid);

    adobe::dictionary_t      m_input;
    adobe::dictionary_t      m_previous_property_state;
    AdamDialogDisplayOption  m_display_option;
    AdamDialogActionCallback m_callback;
    boost::filesystem::path  m_working_directory;
    adobe::sheet_t           m_sheet;
    adobe::vm_lookup_t       m_vm_lookup;
    bool                     m_need_ui;
    AdamDialogResult         m_result;
    adobe::dictionary_t      m_contributing;
    GG::Wnd*                 m_dlg;

    std::vector<boost::shared_ptr<AdamCellGlueBase> > m_cells;
};

// implementations

template <
    class AdamValueType,
    class GGValueType,
    class ControlType
>
void AdamModalDialog::BindCell(ControlType& control, adobe::name_t cell)
{
    m_cells.push_back(
        boost::shared_ptr<AdamCellGlueBase>(
            new AdamCellGlue<ControlType, AdamValueType, GGValueType>(
                control, m_sheet, cell
            )
        )
    );
}

}

#endif
