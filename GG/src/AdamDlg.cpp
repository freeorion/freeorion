/*
Portions adapted from the Adobe Open Source libraries.  Here is their original
license:

Copyright (c) 2005 Adobe Systems Incorporated

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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

#include <GG/AdamDlg.h>

#include <GG/SignalsAndSlots.h>
#include <GG/Wnd.h>
#include <GG/adobe/adam_evaluate.hpp>
#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>


using namespace GG;

AdamModalDialog::AdamModalDialog(const std::string& sheet_definition,
                                 const adobe::dictionary_t& input,
                                 const adobe::dictionary_t& previous_property_state,
                                 AdamDialogDisplayOption display_option,
                                 GG::Wnd* dlg,
                                 AdamDialogActionCallback callback,
                                 boost::filesystem::path working_directory) :
    m_input(input),
    m_previous_property_state(previous_property_state),
    m_display_option(display_option),
    m_callback(callback),
    m_working_directory(working_directory),
    m_dlg(dlg)
{
    m_result = AdamDialogResult();

    m_vm_lookup.attach_to(m_sheet);
    m_vm_lookup.attach_to(m_sheet.machine_m);

    std::istringstream iss(sheet_definition);
    try
    { adobe::parse(iss, adobe::line_position_t("Adam"), adobe::bind_to_sheet(m_sheet)); }
    catch (const adobe::stream_error_t& error)
    { throw std::logic_error(adobe::format_stream_error(iss, error)); }

    m_sheet.update();
    m_sheet.set(m_input);
    m_sheet.set(m_previous_property_state);

    m_need_ui = false;

    m_sheet.update();

    m_contributing = m_sheet.contributing();

    try {
        adobe::name_t result_cell(adobe::static_name_t("result"));

        m_sheet.monitor_invariant_dependent(result_cell,
                                            boost::bind(&AdamModalDialog::MonitorInvariants, this, _1));
        m_sheet.monitor_contributing(result_cell,
                                     m_sheet.contributing(),
                                     boost::bind(&AdamModalDialog::MonitorPropertyState, this, _1));
    } catch (...) {
        // result cell wasn't found. While this isn't a deal-breaker, it's not
        // going to do much in the way of getting results from this dialog.
    }

    m_sheet.update();

    if (m_display_option == ADAM_DIALOG_DISPLAY_NEVER && m_need_ui)
        throw std::runtime_error("HandleAdamDialog: Invalid command parameters and UI not permitted.");

    if ((m_display_option == ADAM_DIALOG_DISPLAY_AS_NEEDED && m_need_ui) ||
        m_display_option == ADAM_DIALOG_DISPLAY_ALWAYS) {
        m_sheet.update();
    }

    GG::Connect(DialogActionSignal, &AdamModalDialog::LatchCallback, this);
}

bool AdamModalDialog::NeedUI() const
{
    return
        (m_display_option == ADAM_DIALOG_DISPLAY_AS_NEEDED && m_need_ui) ||
        m_display_option == ADAM_DIALOG_DISPLAY_ALWAYS;
}

AdamDialogResult AdamModalDialog::Result()
{
    m_result.m_result_values = m_sheet.get(adobe::static_name_t("result")).cast<adobe::dictionary_t>();
    return m_result;
}

void AdamModalDialog::LatchCallback(adobe::name_t action, const adobe::any_regular_t& value)
{
    try {
        if (action == adobe::static_name_t("reset")) {
            m_sheet.set(m_contributing);
            m_sheet.update();
#if 0 // TODO: Should this be here, and more like this?  Look at the example code that comes with the SDK.
        } else if (action == adobe::static_name_t("cancel")) {
            m_sheet.set(m_contributing);
            m_sheet.update();
            m_result.m_terminating_action = action;
            m_dlg->EndRun();
#endif
        } else {
            assert(m_callback);
            if (m_callback(action, value)) {
                m_result.m_terminating_action = action;
                m_dlg->EndRun();
            }
        }
    } catch (const std::exception& error)
    { std::cerr << "Exception (AdamModalDialog::LatchCallback) : " << error.what() << '\n'; }
    catch (...)
    { std::cerr << "Unknown exception (AdamModalDialog::LatchCallback)\n"; }
}

void AdamModalDialog::MonitorPropertyState(const adobe::dictionary_t& property_state)
{ m_result.m_property_state = property_state; }

void AdamModalDialog::MonitorInvariants(bool valid)
{ m_need_ui = !valid; }
