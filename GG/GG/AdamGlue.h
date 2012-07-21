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
   
/** \file AdamGlue.h \brief Contains AdamCellGlueBase and derived classes,
    which automate the binding of GG controls to Adobe Adam property
    models. */

#ifndef _AdamGlue_h_
#define _AdamGlue_h_

#include <GG/DropDownList.h>
#include <GG/Spin.h>
#include <GG/adobe/adam.hpp>

#include <boost/signals.hpp>


namespace GG {

class TextControl;
class Button;
class StateButton;
class RadioButtonGroup;
class Edit;
template <class T>
class Slider;

namespace detail {
    template <class AdobeValueType, class GGValueType>
    GGValueType AnyCast(const adobe::any_regular_t& any);

    template <class AdobeValueType, class GGValueType>
    adobe::any_regular_t MakeAny(GGValueType x);
}

struct AdamCellGlueBase {};

template <
    class ControlType,
    class AdamValueType,
    class GGValueType
>
struct AdamCellGlue;

template <>
struct AdamCellGlue<TextControl, adobe::string_t, std::string> :
    public AdamCellGlueBase
{
    AdamCellGlue(TextControl& text, adobe::sheet_t& sheet, adobe::name_t cell);

private:
    typedef AdamCellGlue<TextControl, adobe::string_t, std::string> ThisType;

    void SheetChanged(const adobe::any_regular_t &any);

    TextControl* m_text;
};

template <>
struct AdamCellGlue<Button, adobe::string_t, std::string> :
    public AdamCellGlueBase
{
    AdamCellGlue(Button& button, adobe::sheet_t& sheet, adobe::name_t cell);

private:
    typedef AdamCellGlue<Button, adobe::string_t, std::string> ThisType;

    void SheetChanged(const adobe::any_regular_t &any);
    void Enable(bool b);

    Button* m_button;
    adobe::sheet_t* m_sheet;
    adobe::name_t m_cell;
};

template <>
struct AdamCellGlue<StateButton, bool, bool> :
    public AdamCellGlueBase
{
    AdamCellGlue(StateButton& state_button, adobe::sheet_t& sheet, adobe::name_t cell);

private:
    typedef AdamCellGlue<StateButton, bool, bool> ThisType;

    void SheetChanged(const adobe::any_regular_t &any);
    void Enable(bool b);
    void ControlChanged(bool checked);

    StateButton* m_state_button;
    adobe::sheet_t* m_sheet;
    adobe::name_t m_cell;
};

template <>
struct AdamCellGlue<RadioButtonGroup, double, std::size_t> :
    public AdamCellGlueBase
{
    AdamCellGlue(RadioButtonGroup& radio_button_group, adobe::sheet_t& sheet, adobe::name_t cell);

private:
    typedef AdamCellGlue<RadioButtonGroup, double, std::size_t> ThisType;

    void SheetChanged(const adobe::any_regular_t &any);
    void Enable(bool b);
    void ControlChanged(std::size_t button);

    RadioButtonGroup* m_radio_button_group;
    adobe::sheet_t* m_sheet;
    adobe::name_t m_cell;
};

template <class AdamValueType, class T>
struct AdamCellGlue<Edit, AdamValueType, T> :
    public AdamCellGlueBase
{
    AdamCellGlue(Edit& edit, adobe::sheet_t& sheet, adobe::name_t cell);

private:
    typedef AdamCellGlue<Edit, AdamValueType, T> ThisType;

    void SheetChanged(const adobe::any_regular_t &any);
    void Enable(bool b);
    void ControlChanged(const std::string& str);

    Edit* m_edit;
    adobe::sheet_t* m_sheet;
    adobe::name_t m_cell;
};

template <>
struct AdamCellGlue<MultiEdit, adobe::string_t, std::string> :
    public AdamCellGlueBase
{
    AdamCellGlue(MultiEdit& multi_edit, adobe::sheet_t& sheet, adobe::name_t cell);

private:
    typedef AdamCellGlue<MultiEdit, adobe::string_t, std::string> ThisType;

    void SheetChanged(const adobe::any_regular_t &any);
    void Enable(bool b);
    void ControlChanged(const std::string& str);

    MultiEdit* m_multi_edit;
    adobe::sheet_t* m_sheet;
    adobe::name_t m_cell;
};

template <class T>
struct AdamCellGlue<Spin<T>, double, T> :
    public AdamCellGlueBase
{
    AdamCellGlue(Spin<T>& spin, adobe::sheet_t& sheet, adobe::name_t cell);

private:
    typedef AdamCellGlue<Spin<T>, double, T> ThisType;

    void SheetChanged(const adobe::any_regular_t &any);
    void Enable(bool b);
    void ControlChanged(T t);

    Spin<T>* m_spin;
    adobe::sheet_t* m_sheet;
    adobe::name_t m_cell;
};

template <>
struct AdamCellGlue<DropDownList, double, std::size_t> :
    public AdamCellGlueBase
{
    AdamCellGlue(DropDownList& drop_list, adobe::sheet_t& sheet, adobe::name_t cell);

private:
    typedef AdamCellGlue<DropDownList, double, std::size_t> ThisType;

    void SheetChanged(const adobe::any_regular_t &any);
    void Enable(bool b);
    void ControlChanged(DropDownList::iterator it);

    DropDownList* m_drop_list;
    adobe::sheet_t* m_sheet;
    adobe::name_t m_cell;
};

template <>
struct AdamCellGlue<Slider<int>, double, int> :
    public AdamCellGlueBase
{
    AdamCellGlue(Slider<int>& slider, adobe::sheet_t& sheet, adobe::name_t cell);

private:
    typedef AdamCellGlue<Slider<int>, double, int> ThisType;

    void SheetChanged(const adobe::any_regular_t &any);
    void Enable(bool b);
    void ControlChanged(int tab_posn, int min, int max);

    Slider<int>* m_slider;
    adobe::sheet_t* m_sheet;
    adobe::name_t m_cell;
};

struct AdamSheetGlue
{
    AdamSheetGlue(const std::string& str);
    AdamSheetGlue(std::istream& stream);

    void SetCell(adobe::name_t cell, const adobe::any_regular_t& value);
    void SetCells(const adobe::dictionary_t& dictionary);

    template <
        class AdamValueType,
        class GGValueType,
        class ControlType
    >
    void BindCell(ControlType& control, adobe::name_t cell);

    adobe::any_regular_t Result();

private:
    void Init(std::istream& stream);

    adobe::sheet_t m_sheet;
    std::vector<boost::shared_ptr<AdamCellGlueBase> > m_cells;
};


// implementations

namespace detail {
    template <class AdobeValueType, class GGValueType>
    GGValueType AnyCast(const adobe::any_regular_t& any)
    { return static_cast<GGValueType>(any.cast<AdobeValueType>()); }

    template <class AdobeValueType, class GGValueType>
    adobe::any_regular_t MakeAny(GGValueType x)
    { return adobe::any_regular_t(static_cast<AdobeValueType>(x)); }
}


// AdamCellGlue<Edit>

template <class AdamValueType, class T>
AdamCellGlue<Edit, AdamValueType, T>::AdamCellGlue(
    Edit& edit,
    adobe::sheet_t& sheet,
    adobe::name_t cell) :
    m_edit(&edit),
    m_sheet(&sheet),
    m_cell(cell)
{
    m_sheet->monitor_value(m_cell, boost::bind(&ThisType::SheetChanged, this, _1));
    m_sheet->monitor_enabled(m_cell, 0, 0, boost::bind(&ThisType::Enable, this, _1));
    m_edit->EditedSignal.connect(boost::bind(&ThisType::ControlChanged, this, _1));
}

template <class AdamValueType, class T>
void AdamCellGlue<Edit, AdamValueType, T>::SheetChanged(const adobe::any_regular_t &any)
{ *m_edit << detail::AnyCast<AdamValueType, T>(any); }

template <class AdamValueType, class T>
void AdamCellGlue<Edit, AdamValueType, T>::Enable(bool b)
{ m_edit->Disable(!b); }

template <class AdamValueType, class T>
void AdamCellGlue<Edit, AdamValueType, T>::ControlChanged(const std::string& str)
{
    try {
        T x = boost::lexical_cast<T>(str);
        m_sheet->set(m_cell, detail::MakeAny<AdamValueType, T>(x));
        m_sheet->update();
    } catch (const boost::bad_lexical_cast&) {}
}


// AdamCellGlue<Spin<T> >

template <class T>
AdamCellGlue<Spin<T>, double, T>::AdamCellGlue(
    Spin<T>& spin,
    adobe::sheet_t& sheet,
    adobe::name_t cell) :
    m_spin(&spin),
    m_sheet(&sheet),
    m_cell(cell)
{
    m_sheet->monitor_value(m_cell, boost::bind(&ThisType::SheetChanged, this, _1));
    m_sheet->monitor_enabled(m_cell, 0, 0, boost::bind(&ThisType::Enable, this, _1));
    m_spin->ValueChangedSignal.connect(boost::bind(&ThisType::ControlChanged, this, _1));
}

template <class T>
void AdamCellGlue<Spin<T>, double, T>::SheetChanged(const adobe::any_regular_t &any)
{ m_spin->SetValue(detail::AnyCast<double, T>(any)); }

template <class T>
void AdamCellGlue<Spin<T>, double, T>::Enable(bool b)
{ m_spin->Disable(!b); }

template <class T>
void AdamCellGlue<Spin<T>, double, T>::ControlChanged(T t)
{
    m_sheet->set(m_cell, detail::MakeAny<double, T>(t));
    m_sheet->update();
}

}

#endif
