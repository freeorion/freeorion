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

/** \file EveGlue.h \brief Contains the ExecuteModalDialog() and MakelDialog()
    functions and associated typs; these automate the parsing, instantiation,
    binding to GG Wnds, and evaluation of Adobe Adam- and Eve- based
    dialogs. */

#ifndef _EveGlue_h_
#define _EveGlue_h_

#include <GG/adobe/dictionary.hpp>

#include <boost/function.hpp>


namespace GG {

class Wnd;

/** Contains the result of a modal dialog created by ExecuteModalDialog(). */
struct ModalDialogResult
{
    /** A map from adobe::name_t to value (adobe::any_regular_t) of the
        expected results.  The contents of m_results will depend on the value
        specified for "result" in the Adam script associated with the modal
        dialog. */
    adobe::dictionary_t m_result;

    /** Indicates the name of the action that terminated the dialog (e.g. "ok"
        or "cancel"). */
    adobe::name_t m_terminating_action;
};

/** The type of button click handle function expected by ExecuteModalDialog().
    Handlers accept the name of the button clicked, the value emitted by the
    click as specified in the Adam and Eve scripts, and return true if the
    click should result in the closure of the dialog. */
typedef boost::function <bool (adobe::name_t, const adobe::any_regular_t&)> ButtonHandler;

/** Parses \a eve_definition and adam_definition, then instantiates and
    executes a modal dialog.  Any buttons ... TODO handler documentation */
ModalDialogResult ExecuteModalDialog(std::istream& eve_definition,
                                     std::istream& adam_definition,
                                     ButtonHandler handler);

/** Parses \a eve_definition and adam_definition, then instantiates and
    executes a modal dialog.  Any buttons ... TODO handler documentation */
Wnd* MakeDialog(std::istream& eve_definition,
                std::istream& adam_definition,
                ButtonHandler handler);

}

#endif
