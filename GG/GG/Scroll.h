//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Scroll.h
//!
//! Contains the Scroll scrollbar control class.

#ifndef _GG_Scroll_h_
#define _GG_Scroll_h_


#include <boost/signals2/signal.hpp>
#include <GG/Control.h>
#include <GG/GLClientAndServerBuffer.h>


namespace GG {

class Button;

/** \brief This is a basic scrollbar control.

    The range of the values the scrollbar represents is [m_range_min,
    m_range_max].  However, m_posn can only range over [m_range_min,
    m_range_max - m_page_sz], because the tab has a logical width of
    m_page_sz.  So the region of the scrollbar's range being viewed at any one
    time is [m_posn, m_posn + m_page_sz].  (m_posn + m_page_sz is actually the
    last + 1 element of the range.)  The parent of the control is notified of
    a scroll via ScrolledSignalType signals; these are emitted from the
    Scroll*() functions and the UpdatePosn() function.  This should cover
    every instance in which m_posn is altered.  The parent can poll the
    control to get its current view area with a call to GetPosnRange().  An
    increase in a vertical scroll is down, and a decrease is up; since GG
    assumes the y-coordinates are downwardly increasing.  The rather plain
    default buttons and tab can be replaced by any Button-derived controls
    desired. */
class GG_API Scroll : public Control
{
public:
    /// the clickable regions of a Scroll
    GG_CLASS_ENUM(ScrollRegion, uint8_t,
        SBR_NONE,
        SBR_PAGE_DN,
        SBR_PAGE_UP
    )

    /** emitted whenever the scrollbar is moved; the upper and lower extents
        of the tab and the upper and lower bounds of the scroll's range are
        indicated, respectively */
    typedef boost::signals2::signal<void (int, int, int, int)> ScrolledSignalType;
    /** emitted when the scrollbar's tab is stopped after being dragged, the
        scrollbar is adjusted using the keyboard, or the scrollbar is moved
        programmatically; the upper and lower extents of the tab and the
        upper and lower bounds of the scroll's range are indicated,
        respectively */
    typedef boost::signals2::signal<void (int, int, int, int)> ScrolledAndStoppedSignalType;

    Scroll(Orientation orientation, Clr color, Clr interior);
    void CompleteConstruction() override;

    Pt MinUsableSize() const override;

    std::pair<int, int>  PosnRange() const;         ///< range currently being viewed
    std::pair<int, int>  ScrollRange() const;       ///< defined possible range of control
    unsigned int         LineSize() const;          ///< returns the current line size
    unsigned int         PageSize() const;          ///< returns the current page size

    Clr                  InteriorColor() const;     ///< returns the color used to render the interior of the Scroll
    Orientation          ScrollOrientation() const; ///< returns the orientation of the Scroll

    mutable ScrolledSignalType           ScrolledSignal;           ///< the scrolled signal object for this Scroll
    mutable ScrolledAndStoppedSignalType ScrolledAndStoppedSignal; ///< the scrolled-and-stopped signal object for this Scroll

    void Render() override;

    void SizeMove(Pt ul, Pt lr) override;

    void Disable(bool b = true) override;
    void SetColor(Clr c) noexcept override;

    virtual void DoLayout();

    void SetInteriorColor(Clr c) noexcept { m_int_color = c; }; ///< sets the color painted into the client area of the control
    void SizeScroll(int min, int max, unsigned int line, unsigned int page); ///< sets the logical ranges of the control, and the logical increment values
    void SetMax(int max);         ///< sets the maximum value of the scroll
    void SetMin(int min);         ///< sets the minimum value of the scroll
    void SetLineSize(unsigned int line); ///< sets the size of a line in the scroll. This is the number of logical units the tab moves when either of the up or down buttons is pressed.
    void SetPageSize(unsigned int page); ///< sets the size of a line page in the scroll. This is the number of logical units the tab moves when either of the page-up or page-down areas is clicked.

    void ScrollTo(int p);  ///< scrolls the control to a certain spot
    void ScrollLineIncr(int lines = 1); ///< scrolls the control down (or right) by \a lines lines
    void ScrollLineDecr(int lines = 1); ///< scrolls the control up (or left) by \a lines lines
    void ScrollPageIncr(); ///< scrolls the control down (or right) by a page
    void ScrollPageDecr(); ///< scrolls the control up (or left) by a page

protected:
    unsigned int  TabSpace() const;          ///< returns the space the tab has to move about in (the control's width less the width of the incr & decr buttons)
    unsigned int  TabWidth() const;          ///< returns the calculated width of the tab, based on PageSize() and the logical size of the control, in pixels
    ScrollRegion  RegionUnder(Pt pt); ///< determines whether a pt is in the incr or decr or tab buttons, or in PgUp/PgDn regions in between

    Button*       TabButton() const;     ///< returns the button representing the tab
    Button*       IncrButton() const;    ///< returns the increase button (line down/line right)
    Button*       DecrButton() const;    ///< returns the decrease button (line up/line left)

    void LButtonDown(Pt pt, Flags<ModKey> mod_keys) override;
    void LButtonUp(Pt pt, Flags<ModKey> mod_keys) override;
    void LClick(Pt pt, Flags<ModKey> mod_keys) override;
    void MouseHere(Pt pt, Flags<ModKey> mod_keys) override;
    bool EventFilter(Wnd* w, const WndEvent& event) override;

    virtual void InitBuffer();

    GG::GL2DVertexBuffer    m_buffer;

private:
    void UpdatePosn();                      ///< adjusts m_posn due to a tab-drag
    void MoveTabToPosn();                   ///< adjusts tab due to a button click, PgUp, etc.
    void ScrollLineIncrDecrImpl(bool signal, int lines);

    Clr                     m_int_color;    ///< color inside border of slide area
    const Orientation       m_orientation;  ///< vertical or horizontal scroll? (use enum for these declared above)
    int                     m_posn;         ///< current position of tab in logical coords (will be in [m_range_min, m_range_max - m_page_sz])
    int                     m_range_min;    ///< lowest value in range of scrollbar
    int                     m_range_max;    ///< highest value "
    unsigned int            m_line_sz;      ///< logical units traversed in a line movement (such as a click on either end button)
    unsigned int            m_page_sz;      ///< logical units traversed for a page movement (such as a click in non-tab middle area, or PgUp/PgDn)
    std::shared_ptr<Button> m_tab;          ///< the button representing the tab
    std::shared_ptr<Button> m_incr;         ///< the increase button (line down/line right)
    std::shared_ptr<Button> m_decr;         ///< the decrease button (line up/line left)
    ScrollRegion            m_initial_depressed_region; ///< the part of the scrollbar originally under cursor in LButtonDown msg
    ScrollRegion            m_depressed_region;         ///< the part of the scrollbar currently being "depressed" by held-down mouse button
    bool                    m_dragging_tab = false;
    bool                    m_tab_dragged = false;
};

/** A convenience function that signals \a scroll's position, via
    Scroll::ScrolledSignal.  If \a stopped is true, the position is
    additionally signalled on Scroll::ScrolledAndStoppedSignal. */
GG_API void SignalScroll(const Scroll& scroll, bool stopped);

}


#endif
