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

/** \file Spin.h \brief Contains the Spin class template, which provides a
    spin-box control that allows the user to select a value from a range an
    arbitrary type (int, double, an enum, etc.). */

#ifndef _GG_Spin_h_
#define _GG_Spin_h_

#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/Edit.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>

#include <cmath>
#include <limits>


namespace GG {

// forward declaration of helper functions and classes
namespace spin_details {
    template <class T> T mod(T, T);
    template <class T> T div(T, T);
}


/** \brief A spin box control.

    This control class is templated so that arbitrary data types can be used
    with Spin.  All the built-in numeric types are supported by the code here.
    If you want to use some other type, such as an enum type, you need to
    define operator+(), operator-(), and template specializations of
    spin_details::mod() and spin_details::div().  Spin controls are optionally
    directly editable by the user.  When the user inputs a value that is not
    valid for the Spin's parameters (not on a step boundary, or outside the
    allowed range), the input gets locked to the nearest valid value.  The
    user is responsible for selecting a min, max, and step size that make
    sense.  For instance, min = 0, max = 4, step = 3 may produce odd results
    if the user increments all the way to the top, then back down, to produce
    the sequence 0, 3, 4, 1, 0.  To avoid this, choose the values so that (max
    - min) mod step == 0.  It is possible to provide custom buttons for a Spin
    to use; if you choose to add custom buttons, make sure they look alright
    at arbitrary sizes, and note that Spin buttons are always H wide by H/2
    tall, where H is the height of the Spin, less the thickness of the Spin's
    border. */
template <class T>
class Spin : public Control
{
public:
    /** \name Signal Types */ ///@{
    /** emitted whenever the value of the Spin has changed */
    typedef typename boost::signals2::signal<void (T)> ValueChangedSignalType;
    //@}

    /** \name Structors */ ///@{
    /** Ctor that does not required height. Height is determined from the font
        and point size used.*/
    Spin(T value, T step, T min, T max, bool edits, const std::shared_ptr<Font>& font,
         Clr color, Clr text_color = CLR_BLACK);
    void CompleteConstruction() override;

    ~Spin();
    //@}

    /** \name Accessors */ ///@{
    Pt MinUsableSize() const override;

    T      Value() const;              ///< returns the current value of the control's text
    T      StepSize() const;           ///< returns the step size of the control
    T      MinValue() const;           ///< returns the minimum value of the control
    T      MaxValue() const;           ///< returns the maximum value of the control
    bool   Editable() const;           ///< returns true if the spinbox can have its value typed in directly

    X      ButtonWidth() const;        ///< returns the width used for the up and down buttons

    Clr    TextColor() const;          ///< returns the text color
    Clr    InteriorColor() const;      ///< returns the the interior color of the control
    Clr    HiliteColor() const;        ///< returns the color used to render hiliting around selected text
    Clr    SelectedTextColor() const;  ///< returns the color used to render selected text

    mutable ValueChangedSignalType ValueChangedSignal; ///< the value changed signal object for this Spin
    //@}

    /** \name Mutators */ ///@{
    void Render() override;
    void SizeMove(const Pt& ul, const Pt& lr) override;
    void Disable(bool b = true) override;
    void SetColor(Clr c) override;
    void Incr();  ///< increments the value of the control's text by StepSize(), up to at most MaxValue()
    void Decr();  ///< decrements the value of the control's text by StepSize(), down to at least MinValue()

    /** sets the value of the control's text to \a value, locked to the
      * range [MinValue(), MaxValue()]*/
    void SetValue(T value);
    void SetStepSize(T step);           ///< sets the step size of the control to \a step
    void SetMinValue(T value);          ///< sets the minimum value of the control to \a value
    void SetMaxValue(T value);          ///< sets the maximum value of the control to \a value

    void SetButtonWidth(X width);       ///< sets the width used for the up and down buttons
    void SetTextColor(Clr c);           ///< sets the text color
    void SetInteriorColor(Clr c);       ///< sets the interior color of the control
    void SetHiliteColor(Clr c);         ///< sets the color used to render hiliting around selected text
    void SetSelectedTextColor(Clr c);   ///< sets the color used to render selected text   
    //@}

protected:
    typedef T ValueType;

    enum {BORDER_THICK = 2, PIXEL_MARGIN = 5};

    /** \name Accessors */ ///@{
    Button*     UpButton() const;   ///< returns a pointer to the Button control used as this control's up button
    Button*     DownButton() const; ///< returns a pointer to the Button control used as this control's down button
    Edit*       GetEdit() const;    ///< returns a pointer to the Edit control used to render this control's text and accept keyboard input
    //@}

    /** \name Mutators */ ///@{
    void KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys) override;
    void MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys) override;
    bool EventFilter(Wnd* w, const WndEvent& event) override;
    virtual void SetEditTextFromValue();
    //@}

    std::shared_ptr<Edit>      m_edit;

private:
    void ConnectSignals();
    void ValueUpdated(const std::string& val_text);
    void IncrImpl(bool signal);
    void DecrImpl(bool signal);
    void SetValueImpl(T value, bool signal);

    T          m_value;
    T          m_step_size;
    T          m_min_value;
    T          m_max_value;

    bool       m_editable;

    std::shared_ptr<Button>    m_up_button;
    std::shared_ptr<Button>    m_down_button;

    X          m_button_width = GG::X(15);

    static void ValueChangedEcho(const T& value);
};


// template implementations
template<class T>
Spin<T>::Spin(T value, T step, T min, T max, bool edits, const std::shared_ptr<Font>& font, Clr color,
              Clr text_color/* = CLR_BLACK*/) :
    Control(X0, Y0, X1, font->Height() + 2 * PIXEL_MARGIN, INTERACTIVE),
    m_value(value),
    m_step_size(step),
    m_min_value(min),
    m_max_value(max),
    m_editable(edits)
{
    const auto& style = GetStyleFactory();
    Control::SetColor(color);
    m_edit = style->NewSpinEdit("", font, CLR_ZERO, text_color, CLR_ZERO);
    auto small_font = GUI::GetGUI()->GetFont(font, static_cast<int>(font->PointSize() * 0.75));
    m_up_button = style->NewSpinIncrButton(small_font, color);
    m_down_button = style->NewSpinDecrButton(small_font, color);

    if (INSTRUMENT_ALL_SIGNALS)
        ValueChangedSignal.connect(&ValueChangedEcho);
}

template<class T>
void Spin<T>::CompleteConstruction()
{
    const auto& style = GetStyleFactory();
    m_edit->InstallEventFilter(shared_from_this());
    m_up_button->InstallEventFilter(shared_from_this());
    m_down_button->InstallEventFilter(shared_from_this());
    AttachChild(m_edit);
    AttachChild(m_up_button);
    AttachChild(m_down_button);
    ConnectSignals();
    SizeMove(UpperLeft(), LowerRight());
    Spin<T>::SetEditTextFromValue();
}

template<class T>
Spin<T>::~Spin()
{}

template<class T>
Pt Spin<T>::MinUsableSize() const
{
    Pt edit_min = m_edit->MinUsableSize();
    Pt up_min = m_up_button->MinUsableSize();
    Pt down_min = m_down_button->MinUsableSize();
    return Pt(edit_min.x + std::max(up_min.x, down_min.x) + 2 * BORDER_THICK,
              std::max(up_min.y + down_min.y, edit_min.y) + 2 * BORDER_THICK);
}

template<class T>
T Spin<T>::Value() const
{ return m_value; }

template<class T>
T Spin<T>::StepSize() const
{ return m_step_size; }

template<class T>
T Spin<T>::MinValue() const
{ return m_min_value; }

template<class T>
T Spin<T>::MaxValue() const
{ return m_max_value; }

template<class T>
bool Spin<T>::Editable() const 
{ return m_editable; }

template<class T>
X Spin<T>::ButtonWidth() const
{ return m_button_width; }

template<class T>
Clr Spin<T>::TextColor() const
{ return m_edit->TextColor(); }

template<class T>
Clr Spin<T>::InteriorColor() const
{ return m_edit->InteriorColor(); }

template<class T>
Clr Spin<T>::HiliteColor() const
{ return m_edit->HiliteColor(); }

template<class T>
Clr Spin<T>::SelectedTextColor() const
{ return m_edit->SelectedTextColor(); }

template<class T>
void Spin<T>::Render()
{
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();
    Pt ul = UpperLeft(), lr = LowerRight();
    BeveledRectangle(ul, lr, int_color_to_use, color_to_use, false, BORDER_THICK);
}

template<class T>
void Spin<T>::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    const X BUTTON_X_POS = Width() - m_button_width - BORDER_THICK;
    const Y BUTTONS_HEIGHT = Height() - 2 * BORDER_THICK; // height of *both* buttons
    m_edit->SizeMove(Pt(), Pt(Width() - m_button_width, Height()));
    m_up_button->SizeMove(Pt(BUTTON_X_POS, Y(BORDER_THICK)),
                          Pt(BUTTON_X_POS + m_button_width, BORDER_THICK + BUTTONS_HEIGHT / 2));
    m_down_button->SizeMove(Pt(BUTTON_X_POS, BORDER_THICK + BUTTONS_HEIGHT / 2),
                            Pt(BUTTON_X_POS + m_button_width, BORDER_THICK + BUTTONS_HEIGHT));
}

template<class T>
void Spin<T>::Disable(bool b/* = true*/)
{
    Control::Disable(b);
    m_edit->Disable(b);
    m_up_button->Disable(b);
    m_down_button->Disable(b);
}

template<class T>
void Spin<T>::SetColor(Clr c)
{
    Control::SetColor(c);
    m_up_button->SetColor(c);
    m_down_button->SetColor(c);
}

template<class T>
void Spin<T>::Incr()
{ SetValueImpl(m_value + m_step_size, false); }

template<class T>
void Spin<T>::Decr()
{ SetValueImpl(m_value - m_step_size, false); }

template<class T>
void Spin<T>::SetValue(T value)
{ SetValueImpl(value, false); }

template<class T>
void Spin<T>::SetStepSize(T step)
{
    m_step_size = step;
    SetValue(m_value);
}

template<class T>
void Spin<T>::SetMinValue(T value)
{
    m_min_value = value;
    if (m_value < m_min_value)
        SetValue(m_min_value);
}

template<class T>
void Spin<T>::SetMaxValue(T value)
{
    m_max_value = value;
    if (m_max_value < m_value)
        SetValue(m_max_value);
}

template<class T>
void Spin<T>::SetTextColor(Clr c)
{ m_edit->SetTextColor(c); }

template<class T>
void Spin<T>::SetButtonWidth(X width)
{
    if (1 <= width) {
        if (Width() - 2 * BORDER_THICK - 1 < width)
            width = Width() - 2 * BORDER_THICK - 1;
        m_button_width = width;
        SizeMove(RelativeUpperLeft(), RelativeLowerRight());
    }
}

template<class T>
void Spin<T>::SetInteriorColor(Clr c)
{ m_edit->SetInteriorColor(c); }

template<class T>
void Spin<T>::SetHiliteColor(Clr c)
{ m_edit->SetHiliteColor(c); }

template<class T>
void Spin<T>::SetSelectedTextColor(Clr c)
{ m_edit->SetSelectedTextColor(c); }

template<class T>
Button* Spin<T>::UpButton() const
{ return m_up_button.get(); }

template<class T>
Button* Spin<T>::DownButton() const
{ return m_down_button.get(); }

template<class T>
Edit* Spin<T>::GetEdit() const
{ return m_edit.get(); }

template<class T>
void Spin<T>::KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    switch (key) {
    case GGK_HOME:
        SetValueImpl(m_min_value, true);
        break;
    case GGK_END:
        SetValueImpl(m_max_value, true);
        break;
    case GGK_PAGEUP:
    case GGK_UP:
    case GGK_PLUS:
    case GGK_KP_PLUS:
        IncrImpl(true);
        break;
    case GGK_PAGEDOWN:
    case GGK_DOWN:
    case GGK_MINUS:
    case GGK_KP_MINUS:
        DecrImpl(true);
        break;
    default:
        break;
    }
}

template<class T>
void Spin<T>::MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys)
{
    for (int i = 0; i < move; ++i) {
        IncrImpl(true);
    }
    for (int i = 0; i < -move; ++i) {
        DecrImpl(true);
    }
}

template<class T>
bool Spin<T>::EventFilter(Wnd* w, const WndEvent& event)
{
    if (w == m_edit.get()) {
        if (!m_editable && event.Type() == WndEvent::GainingFocus) {
            GUI::GetGUI()->SetFocusWnd(shared_from_this());
            return true;
        } else {
            return !m_editable;
        }
    }
    return false;
}

template<class T>
void Spin<T>::SetEditTextFromValue()
{
    if (m_edit)
        m_edit->SetText(std::to_string(m_value));
}

template<class T>
void Spin<T>::ConnectSignals()
{
    m_edit->FocusUpdateSignal.connect(
        boost::bind(&Spin::ValueUpdated, this, _1));
    m_up_button->LeftClickedSignal.connect(
        boost::bind(&Spin::IncrImpl, this, true));
    m_down_button->LeftClickedSignal.connect(
        boost::bind(&Spin::DecrImpl, this, true));
}

template<class T>
void Spin<T>::ValueUpdated(const std::string& val_text)
{
    T value;
    try {
        value = boost::lexical_cast<T>(val_text);
    } catch (boost::bad_lexical_cast) {
        SetValueImpl(m_min_value, true);
        return;
    }
    SetValueImpl(value, true);
}

template<class T>
void Spin<T>::IncrImpl(bool signal)
{ SetValueImpl(static_cast<T>(m_value + m_step_size), signal); }

template<class T>
void Spin<T>::DecrImpl(bool signal)
{ SetValueImpl(static_cast<T>(m_value - m_step_size), signal); }

template<class T>
void Spin<T>::SetValueImpl(T value, bool signal)
{
    //std::cout << "Spin<T>::SetValueImpl(" << value << ", " << signal << ")" << std::endl;
    T old_value = m_value;
    if (value < m_min_value) {
        m_value = m_min_value;
    } else if (m_max_value < value) {
        m_value = m_max_value;
    } else {
        // if the value supplied does not equal a valid value
        if (std::abs(spin_details::mod(static_cast<T>(value - m_min_value), m_step_size)) >
            std::numeric_limits<T>::epsilon())
        {
            // find nearest valid value to the one supplied
            T closest_below =
                static_cast<T>(
                    spin_details::div(static_cast<T>(value - m_min_value), m_step_size)
                        * m_step_size
                    + m_min_value);
            T closest_above =
                static_cast<T>(closest_below + m_step_size);
            //std::cout << " ... closest below: " << closest_below << "  above: " << closest_above << std::endl;
            m_value =
                ((value - closest_below) < (closest_above - value) ?
                 closest_below : closest_above);
        } else {
            m_value = value;
        }
    }
    SetEditTextFromValue();
    if (signal && m_value != old_value)
        ValueChangedSignal(m_value);
}

template <class T>
void Spin<T>::ValueChangedEcho(const T& value)
{ std::cerr << "GG SIGNAL : Spin<>::ValueChangedSignal(value=" << value << ")\n"; }


namespace spin_details {
    // provides a typesafe mod function
    template <class T> inline 
    T mod (T dividend, T divisor) {return static_cast<T>(dividend % divisor);}

    // template specializations
    template <> inline 
    float mod<float> (float dividend, float divisor) {return std::fmod(dividend, divisor);}
    template <> inline 
    double mod<double> (double dividend, double divisor) {return std::fmod(dividend, divisor);}
    template <> inline 
    long double mod<long double> (long double dividend, long double divisor) {return std::fmod(dividend, divisor);}

    // provides a typesafe div function
    template <class T> inline 
    T div (T dividend, T divisor) {return static_cast<T>(dividend / divisor);}

    // template specializations
    template <> inline 
    float div<float> (float dividend, float divisor) {return std::floor(dividend / divisor);}
    template <> inline 
    double div<double> (double dividend, double divisor) {return std::floor(dividend / divisor);}
    template <> inline 
    long double div<long double> (long double dividend, long double divisor) {return std::floor(dividend / divisor);}
} // namespace spin_details

} // namespace GG

#endif

