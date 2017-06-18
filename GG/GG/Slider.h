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

/** \file Slider.h \brief Contains the Slider class, which provides a slider
    control that allows the user to select a value from a range of an
    arbitrary type (int, double, an enum, etc.). */

#ifndef _GG_Slider_h_
#define _GG_Slider_h_

#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>


namespace GG {

class Button;
class WndEvent;

/** \brief A slider control.

    This control class is templated so that arbitrary data types can be used
    with Slider.  All the built-in numeric types are supported by the code
    here.  If you want to use some other type, such as an enum type, you need
    to define operator+(), and operator-().  This control allows the user to
    drag a tab to a desired setting; it is somewhat like a Scroll.  Sliders
    can be either vertical or horizontal, but cannot switch between the two.
    Unlike vertical Scrolls, whose values increase downward, vertical Sliders
    increase upward by default.  Note that it is acceptable to define a range
    that increases from min to max, or one that decreases from min to max;
    both are legal. */
template <class T>
class Slider : public Control
{
public:
    /** \name Signal Types */ ///@{
    /**emitted whenever the slider is moved; the tab position and the upper
       and lower bounds of the slider's range are indicated, respectively */
    typedef boost::signals2::signal<void (T, T, T)> SlidSignalType;
    /** emitted when the slider's tab is stopped after being dragged, the
        slider is adjusted using the keyboard, or the slider is moved
        programmatically; the tab position and the upper and lower bounds
        of the slider's range are indicated, respectively */
    typedef boost::signals2::signal<void (T, T, T)> SlidAndStoppedSignalType;
    //@}

    /** \name Structors */ ///@{
    Slider(T min, T max, Orientation orientation, Clr color,
           unsigned int tab_width, unsigned int line_width = 5, Flags<WndFlag> flags = INTERACTIVE);
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ ///@{
    Pt MinUsableSize() const override;

    T                    Posn() const;           ///< returns the current tab position
    std::pair<T, T>      SliderRange() const;    ///< returns the defined possible range of control

    /** Returns the current page size, or the amount that the slider
        increments/decrements when a click occurs off of the tab.  If not set,
        this defaults to 10% of the slider's range. */
    T                    PageSize() const;

    Orientation          GetOrientation() const; ///< returns the orientation of the slider (VERTICAL or HORIZONTAL)
    unsigned int         TabWidth() const;       ///< returns the width of the slider's tab, in pixels
    unsigned int         LineWidth() const;      ///< returns the width of the line along which the tab slides, in pixels

    mutable SlidSignalType           SlidSignal;           ///< returns the slid signal object for this Slider
    mutable SlidAndStoppedSignalType SlidAndStoppedSignal; ///< returns the slid-and-stopped signal object for this Slider
    //@}

    /** \name Mutators */ ///@{
    void Render() override;
    void SizeMove(const Pt& ul, const Pt& lr) override;
    void Disable(bool b = true) override;
    void SetColor(Clr c) override;

    void           SizeSlider(T min, T max); ///< sets the logical range of the control; \a min must not equal \a max
    void           SetMax(T max);            ///< sets the maximum value of the control
    void           SetMin(T min);            ///< sets the minimum value of the control
    void           SlideTo(T p);             ///< slides the control to a certain spot

    /** Sets the size of a "page", or the amount that the slider
        increments/decrements when a click occurs off of the tab.  If not set,
        this defaults to 10% of the slider's range.  To disable clicks off the
        tab, set the page size to 0. */
    void           SetPageSize(T size);
    //@}

    static const T INVALID_PAGE_SIZE;

protected:
    /** \name Accessors */ ///@{
    Button* Tab() const;                  ///< returns a pointer to the Button used as this control's sliding tab
    T       PtToPosn(const Pt& pt) const; ///< maps an arbitrary screen point to its nearest logical slider position
    //@}

    /** \name Mutators */ ///@{
    void LClick(const Pt& pt, Flags<ModKey> mod_keys) override;
    void KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys) override;
    bool EventFilter(Wnd* w, const WndEvent& event) override;

    void MoveTabToPosn(); ///< moves the tab to the current logical position
    //@}

private:
    void SlideToImpl(T p, bool signal);
    void UpdatePosn();

    struct SlidEcho
    {
        SlidEcho(const std::string& name);
        void operator()(T pos, T min, T max);
        std::string m_name;
    };

    T                         m_posn;
    T                         m_range_min;
    T                         m_range_max;
    T                         m_page_sz;
    Orientation               m_orientation;
    unsigned int              m_line_width;
    unsigned int              m_tab_width;
    int                       m_tab_drag_offset;
    std::shared_ptr<Button>   m_tab;
    bool                      m_dragging_tab;
};

// template implementations
template <class T>
const T Slider<T>::INVALID_PAGE_SIZE = std::numeric_limits<T>::max();

template <class T>
Slider<T>::Slider(T min, T max, Orientation orientation,
                  Clr color, int unsigned tab_width, int unsigned line_width/* = 5*/,
                  Flags<WndFlag> flags/* = INTERACTIVE*/) :
    Control(X0, Y0, X1, Y1, flags),
    m_posn(min),
    m_range_min(min),
    m_range_max(max),
    m_page_sz(INVALID_PAGE_SIZE),
    m_orientation(orientation),
    m_line_width(line_width),
    m_tab_width(tab_width),
    m_tab_drag_offset(-1),
    m_tab(m_orientation == VERTICAL ?
          GetStyleFactory()->NewVSliderTabButton(color) :
          GetStyleFactory()->NewHSliderTabButton(color)),
    m_dragging_tab(false)
{
    Control::SetColor(color);
}

template <class T>
void Slider<T>::CompleteConstruction()
{
    AttachChild(m_tab);
    m_tab->InstallEventFilter(shared_from_this());
    SizeMove(UpperLeft(), LowerRight());

    if (INSTRUMENT_ALL_SIGNALS) {
        SlidSignal.connect(SlidEcho("Slider<T>::SlidSignal"));
        SlidAndStoppedSignal.connect(SlidEcho("Slider<T>::SlidAndStoppedSignal"));
    }
}

template <class T>
Pt Slider<T>::MinUsableSize() const
{
    Pt tab_min = m_tab->MinUsableSize();
    return Pt(m_orientation == VERTICAL ? tab_min.x : Size().x,
              m_orientation == VERTICAL ? Size().y : tab_min.y);
}

template <class T>
T Slider<T>::Posn() const
{ return m_posn; }

template <class T>
std::pair<T, T> Slider<T>::SliderRange() const
{ return std::pair<T, T>(m_range_min, m_range_max); }

template <class T>
T Slider<T>::PageSize() const
{ return m_page_sz != INVALID_PAGE_SIZE ? m_page_sz : (m_range_max - m_range_min) / 10; }

template <class T>
Orientation Slider<T>::GetOrientation() const
{ return m_orientation; }

template <class T>
unsigned int Slider<T>::TabWidth() const
{ return m_tab_width; }

template <class T>
unsigned int Slider<T>::LineWidth() const
{ return m_line_width; }

template <class T>
void Slider<T>::Render()
{
    const Pt UL = UpperLeft();
    const Pt LR = LowerRight();
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    int tab_width = m_orientation == VERTICAL ? Value(m_tab->Height()) : Value(m_tab->Width());
    Pt ul, lr;
    if (m_orientation == VERTICAL) {
        ul.x = ((LR.x + UL.x) - static_cast<int>(m_line_width)) / 2;
        lr.x = ul.x + static_cast<int>(m_line_width);
        ul.y = UL.y + tab_width / 2;
        lr.y = LR.y - tab_width / 2;
    } else {
        ul.x = UL.x + tab_width / 2;
        lr.x = LR.x - tab_width / 2;
        ul.y = ((LR.y + UL.y) - static_cast<int>(m_line_width)) / 2;
        lr.y = ul.y + static_cast<int>(m_line_width);
    }
    FlatRectangle(ul, lr, color_to_use, CLR_BLACK, 1);
}

template <class T>
void Slider<T>::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    if (m_orientation == VERTICAL)
        m_tab->SizeMove(Pt(), Pt(lr.x - ul.x, Y(m_tab_width)));
    else
        m_tab->SizeMove(Pt(), Pt(X(m_tab_width), lr.y - ul.y));
    MoveTabToPosn();
}

template <class T>
void Slider<T>::Disable(bool b/* = true*/)
{
    Control::Disable(b);
    m_tab->Disable(b);
}

template <class T>
void Slider<T>::SetColor(Clr c)
{
    Control::SetColor(c);
    m_tab->SetColor(c);
}

template <class T>
void Slider<T>::SizeSlider(T min, T max)
{
    assert(m_range_min != m_range_max);
    m_range_min = min;
    m_range_max = max;
    if (m_posn < m_range_min)
        SlideToImpl(m_range_min, false);
    else if (m_range_max < m_posn)
        SlideToImpl(m_range_max, false);
    else
        MoveTabToPosn();
}

template <class T>
void Slider<T>::SetMax(T max)
{ SizeSlider(m_range_min, max); }

template <class T>
void Slider<T>::SetMin(T min)
{ SizeSlider(min, m_range_max); }

template <class T>
void Slider<T>::SlideTo(T p)
{ SlideToImpl(p, false); }

template <class T>
void Slider<T>::SetPageSize(T size)
{ m_page_sz = size; }

template <class T>
Button* Slider<T>::Tab() const
{ return m_tab.get(); }

template <class T>
T Slider<T>::PtToPosn(const Pt& pt) const
{
    Pt ul = UpperLeft(), lr = LowerRight();
    int line_min = 0;
    int line_max = 0;
    int pixel_nearest_to_pt_on_line = 0;
    if (m_orientation == VERTICAL) {
        line_min = Value(m_tab->Height() / 2);
        line_max = Value(Height() - (m_tab->Height() - m_tab->Height() / 2));
        pixel_nearest_to_pt_on_line = std::max(line_min, std::min(Value(lr.y - pt.y), line_max));
    } else {
        line_min = Value(m_tab->Width() / 2);
        line_max = Value(Width() - (m_tab->Width() - m_tab->Width() / 2));
        pixel_nearest_to_pt_on_line = std::max(line_min, std::min(Value(pt.x - ul.x), line_max));
    }
    double fractional_distance = static_cast<double>(pixel_nearest_to_pt_on_line) / (line_max - line_min);
    return m_range_min + static_cast<T>((m_range_max - m_range_min) * fractional_distance);
}

template <class T>
void Slider<T>::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{ SlideToImpl(m_posn < PtToPosn(pt) ? m_posn + PageSize() : m_posn - PageSize(), true); }

template <class T>
void Slider<T>::KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        switch (key) {
        case GGK_HOME:
            SlideToImpl(m_range_min, true);
            break;
        case GGK_END:
            SlideToImpl(m_range_max, true);
            break;
        case GGK_UP:
            if (m_orientation != HORIZONTAL)
                SlideToImpl(m_posn + (0 < (m_range_max - m_range_min) ? 1 : -1), true);
            break;
        case GGK_RIGHT:
            if (m_orientation != VERTICAL)
                SlideToImpl(m_posn + (0 < (m_range_max - m_range_min) ? 1 : -1), true);
            break;
        case GGK_DOWN:
            if (m_orientation != HORIZONTAL)
                SlideToImpl(m_posn - (0 < (m_range_max - m_range_min) ? 1 : -1), true);
            break;
        case GGK_LEFT:
            if (m_orientation != VERTICAL)
                SlideToImpl(m_posn - (0 < (m_range_max - m_range_min) ? 1 : -1), true);
            break;
        case GGK_PLUS:
        case GGK_KP_PLUS:
            SlideToImpl(m_posn + 1, true);
            break;
        case GGK_MINUS:
        case GGK_KP_MINUS:
            SlideToImpl(m_posn - 1, true);
            break;
        default:
            Control::KeyPress(key, key_code_point, mod_keys);
            break;
        }
    } else {
        Control::KeyPress(key, key_code_point, mod_keys);
    }
}

template <class T>
bool Slider<T>::EventFilter(Wnd* w, const WndEvent& event)
{
    if (w == m_tab.get()) {
        switch (event.Type()) {
        case WndEvent::LDrag: {
            if (!Disabled()) {
                Pt new_ul = m_tab->RelativeUpperLeft() + event.DragMove();
                if (m_orientation == VERTICAL) {
                    new_ul.x = m_tab->RelativeUpperLeft().x;
                    new_ul.y = std::max(Y0, std::min(new_ul.y, ClientHeight() - m_tab->Height()));
                } else {
                    new_ul.x = std::max(X0, std::min(new_ul.x, ClientWidth() - m_tab->Width()));
                    new_ul.y = m_tab->RelativeUpperLeft().y;
                }
                m_tab->MoveTo(new_ul);
                UpdatePosn();
            }
            return true;
        }
        case WndEvent::LButtonDown:
            m_dragging_tab = true;
            break;
        case WndEvent::LButtonUp:
        case WndEvent::LClick: {
            if (!Disabled())
                SlidAndStoppedSignal(m_posn, m_range_min, m_range_max);
            m_dragging_tab = false;
            break;
        }
        case WndEvent::MouseLeave:
            return m_dragging_tab;
        default:
            break;
        }
    }
    return false;
}

template <class T>
void Slider<T>::MoveTabToPosn()
{
    assert((m_range_min <= m_posn && m_posn <= m_range_max) ||
           (m_range_max <= m_posn && m_posn <= m_range_min));
    double fractional_distance = static_cast<double>(m_posn - m_range_min) / (m_range_max - m_range_min);
    int tab_width = m_orientation == VERTICAL ? Value(m_tab->Height()) : Value(m_tab->Width());
    int line_length = (m_orientation == VERTICAL ? Value(Height()) : Value(Width())) - tab_width;
    int pixel_distance = static_cast<int>(line_length * fractional_distance);
    if (m_orientation == VERTICAL)
        m_tab->MoveTo(Pt(m_tab->RelativeUpperLeft().x, Height() - tab_width - pixel_distance));
    else
        m_tab->MoveTo(Pt(X(pixel_distance), m_tab->RelativeUpperLeft().y));
}

template <class T>
void Slider<T>::UpdatePosn()
{
    T old_posn = m_posn;
    int line_length = m_orientation == VERTICAL ? Value(Height() - m_tab->Height()) : Value(Width() - m_tab->Width());
    int tab_posn = (m_orientation == VERTICAL ? Value(Height() - m_tab->RelativeLowerRight().y) : Value(m_tab->RelativeUpperLeft().x));
    double fractional_distance = static_cast<double>(tab_posn) / line_length;
    m_posn = m_range_min + static_cast<T>((m_range_max - m_range_min) * fractional_distance);
    if (m_posn != old_posn)
        SlidSignal(m_posn, m_range_min, m_range_max);
}

template <class T>
void Slider<T>::SlideToImpl(T p, bool signal)
{
    T old_posn = m_posn;
    if (0 < (m_range_max - m_range_min) ? p < m_range_min : p > m_range_min)
        m_posn = m_range_min;
    else if (0 < (m_range_max - m_range_min) ? m_range_max < p : m_range_max > p)
        m_posn = m_range_max;
    else
        m_posn = p;
    MoveTabToPosn();
    if (signal && m_posn != old_posn) {
        SlidSignal(m_posn, m_range_min, m_range_max);
        SlidAndStoppedSignal(m_posn, m_range_min, m_range_max);
    }
}

template <class T>
Slider<T>::SlidEcho::SlidEcho(const std::string& name) :
    m_name(name)
{}

template <class T>
void Slider<T>::SlidEcho::operator()(T pos, T min, T max)
{
    std::cerr << "GG SIGNAL : " << m_name
              << "(pos=" << pos << " min=" << min << " max=" << max << ")\n";
}

} // namespace GG

#endif
