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

#include <GG/EveGlue.h>
#include <GG/adobe/future/modal_dialog_interface.hpp>


using namespace GG;

ModalDialogResult GG::ExecuteModalDialog(std::istream& eve_definition,
                                         std::istream& adam_definition,
                                         ButtonHandler handler)
{
    ModalDialogResult retval;

    adobe::dialog_result_t adobe_result =
        adobe::handle_dialog(adobe::dictionary_t(),
                             adobe::dictionary_t(),
                             adobe::dictionary_t(),
                             adobe::dialog_display_s,
                             eve_definition,
                             adam_definition,
                             handler,
                             boost::filesystem::path());

    swap(adobe_result.command_m, retval.m_result);
    retval.m_terminating_action = adobe_result.terminating_action_m;

    return retval;
}

Wnd* GG::MakeDialog(std::istream& eve_definition,
                    std::istream& adam_definition,
                    ButtonHandler handler)
{
    Wnd* retval = 0;
    return retval;
}
