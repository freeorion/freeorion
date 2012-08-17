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

#include <GG/AdamGlue.h>

#include <GG/Button.h>
#include <GG/DropDownList.h>
#include <GG/MultiEdit.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/TextControl.h>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/string.hpp>


using namespace GG;

////////////////////////////////////////
// AdamCellGlue<TextControl>
////////////////////////////////////////
AdamCellGlue<TextControl, adobe::string_t, std::string>::AdamCellGlue(
    TextControl& text,
    adobe::sheet_t& sheet,
    adobe::name_t cell) :
    m_text(&text)
{ sheet.monitor_value(cell, boost::bind(&ThisType::SheetChanged, this, _1)); }

void AdamCellGlue<TextControl, adobe::string_t, std::string>::SheetChanged(
    const adobe::any_regular_t &any)
{ m_text->SetText(any.cast<adobe::string_t>()); }


////////////////////////////////////////
// AdamCellGlue<Button>
////////////////////////////////////////
AdamCellGlue<Button, adobe::string_t, std::string>::AdamCellGlue(
    Button& button,
    adobe::sheet_t& sheet,
    adobe::name_t cell) :
    m_button(&button),
    m_sheet(&sheet),
    m_cell(cell)
{
    m_sheet->monitor_value(m_cell, boost::bind(&ThisType::SheetChanged, this, _1));
    m_sheet->monitor_enabled(m_cell, 0, 0, boost::bind(&ThisType::Enable, this, _1));
}

void AdamCellGlue<Button, adobe::string_t, std::string>::SheetChanged(
    const adobe::any_regular_t &any)
{ m_button->SetText(detail::AnyCast<adobe::string_t, std::string>(any)); }

void AdamCellGlue<Button, adobe::string_t, std::string>::Enable(bool b)
{ m_button->Disable(!b); }


////////////////////////////////////////
// AdamCellGlue<StateButton>
////////////////////////////////////////
AdamCellGlue<StateButton, bool, bool>::AdamCellGlue(
    StateButton& state_button,
    adobe::sheet_t& sheet,
    adobe::name_t cell) :
    m_state_button(&state_button),
    m_sheet(&sheet),
    m_cell(cell)
{
    m_sheet->monitor_value(m_cell, boost::bind(&ThisType::SheetChanged, this, _1));
    m_sheet->monitor_enabled(m_cell, 0, 0, boost::bind(&ThisType::Enable, this, _1));
    Connect(m_state_button->CheckedSignal, &ThisType::ControlChanged, this);
}

void AdamCellGlue<StateButton, bool, bool>::SheetChanged(const adobe::any_regular_t &any)
{
    bool new_value;
    if (any.cast<bool>(new_value))
        m_state_button->SetCheck(new_value);
    else
        m_state_button->SetText(detail::AnyCast<adobe::string_t, std::string>(any));
}

void AdamCellGlue<StateButton, bool, bool>::Enable(bool b)
{ m_state_button->Disable(!b); }

void AdamCellGlue<StateButton, bool, bool>::ControlChanged(bool checked)
{
    m_sheet->set(m_cell, detail::MakeAny<bool, bool>(checked));
    m_sheet->update();
}


////////////////////////////////////////
// AdamCellGlue<RadioButtonGroup>
////////////////////////////////////////
AdamCellGlue<RadioButtonGroup, double, std::size_t>::AdamCellGlue(
    RadioButtonGroup& radio_button_group,
    adobe::sheet_t& sheet,
    adobe::name_t cell) :
    m_radio_button_group(&radio_button_group),
    m_sheet(&sheet),
    m_cell(cell)
{
    m_sheet->monitor_value(m_cell, boost::bind(&ThisType::SheetChanged, this, _1));
    m_sheet->monitor_enabled(m_cell, 0, 0, boost::bind(&ThisType::Enable, this, _1));
    Connect(m_radio_button_group->ButtonChangedSignal, &ThisType::ControlChanged, this);
}

void AdamCellGlue<RadioButtonGroup, double, std::size_t>::SheetChanged(
    const adobe::any_regular_t &any)
{ m_radio_button_group->SetCheck(detail::AnyCast<double, std::size_t>(any)); }

void AdamCellGlue<RadioButtonGroup, double, std::size_t>::Enable(bool b)
{ m_radio_button_group->Disable(!b); }

void AdamCellGlue<RadioButtonGroup, double, std::size_t>::ControlChanged(std::size_t button)
{
    m_sheet->set(m_cell, detail::MakeAny<double, std::size_t>(button));
    m_sheet->update();
}


////////////////////////////////////////
// AdamCellGlue<MultiEdit>
////////////////////////////////////////
AdamCellGlue<MultiEdit, adobe::string_t, std::string>::AdamCellGlue(
    MultiEdit& multi_edit,
    adobe::sheet_t& sheet,
    adobe::name_t cell) :
    m_multi_edit(&multi_edit),
    m_sheet(&sheet),
    m_cell(cell)
{
    m_sheet->monitor_value(m_cell, boost::bind(&ThisType::SheetChanged, this, _1));
    m_sheet->monitor_enabled(m_cell, 0, 0, boost::bind(&ThisType::Enable, this, _1));
    Connect(m_multi_edit->EditedSignal, &ThisType::ControlChanged, this);
}

void AdamCellGlue<MultiEdit, adobe::string_t, std::string>::SheetChanged(
    const adobe::any_regular_t &any)
{ m_multi_edit->SetText(detail::AnyCast<adobe::string_t, std::string>(any)); }

void AdamCellGlue<MultiEdit, adobe::string_t, std::string>::Enable(bool b)
{ m_multi_edit->Disable(!b); }

void AdamCellGlue<MultiEdit, adobe::string_t, std::string>::ControlChanged(
    const std::string& str)
{
    m_sheet->set(m_cell, detail::MakeAny<adobe::string_t, std::string>(str));
    m_sheet->update();
}


////////////////////////////////////////
// AdamCellGlue<DropDownList>
////////////////////////////////////////
AdamCellGlue<DropDownList, double, std::size_t>::AdamCellGlue(
    DropDownList& drop_list,
    adobe::sheet_t& sheet,
    adobe::name_t cell) :
    m_drop_list(&drop_list),
    m_sheet(&sheet),
    m_cell(cell)
{
    m_sheet->monitor_value(m_cell, boost::bind(&ThisType::SheetChanged, this, _1));
    m_sheet->monitor_enabled(m_cell, 0, 0, boost::bind(&ThisType::Enable, this, _1));
    Connect(m_drop_list->SelChangedSignal, &ThisType::ControlChanged, this);
}

void AdamCellGlue<DropDownList, double, std::size_t>::SheetChanged(
    const adobe::any_regular_t &any)
{ m_drop_list->Select(detail::AnyCast<double, std::size_t>(any)); }

void AdamCellGlue<DropDownList, double, std::size_t>::Enable(bool b)
{ m_drop_list->Disable(!b); }

void AdamCellGlue<DropDownList, double, std::size_t>::ControlChanged(DropDownList::iterator it)
{
    m_sheet->set(m_cell,
                 detail::MakeAny<double, std::size_t>(m_drop_list->IteratorToIndex(it)));
    m_sheet->update();
}


////////////////////////////////////////
// AdamCellGlue<Slider>
////////////////////////////////////////
AdamCellGlue<Slider<int>, double, int>::AdamCellGlue(
    Slider<int>& slider,
    adobe::sheet_t& sheet,
    adobe::name_t cell) :
    m_slider(&slider),
    m_sheet(&sheet),
    m_cell(cell)
{
    m_sheet->monitor_value(m_cell, boost::bind(&ThisType::SheetChanged, this, _1));
    m_sheet->monitor_enabled(m_cell, 0, 0, boost::bind(&ThisType::Enable, this, _1));
    Connect(m_slider->SlidSignal, &ThisType::ControlChanged, this);
}

void AdamCellGlue<Slider<int>, double, int>::SheetChanged(const adobe::any_regular_t &any)
{ m_slider->SlideTo(detail::AnyCast<double, int>(any)); }

void AdamCellGlue<Slider<int>, double, int>::Enable(bool b)
{ m_slider->Disable(!b); }

void AdamCellGlue<Slider<int>, double, int>::ControlChanged(int tab_posn, int min, int max)
{
    m_sheet->set(m_cell, detail::MakeAny<double, int>(tab_posn));
    m_sheet->update();
}
