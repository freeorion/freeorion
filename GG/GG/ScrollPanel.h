//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2015 Mitten-O
//!  Copyright (C) 2016-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _GG_ScrollPanel_h_
#define _GG_ScrollPanel_h_


#include <GG/Clr.h>
#include <GG/Wnd.h>


namespace GG {

class Scroll;

/**
 * @brief A panel that allows scrolling its contents.
 *
 * Only Vertical scrolling is implemented for now.
 * The scroll panel takes on child and tells it to fill
 * the client area of the panel. If the child decides
 * to be taller than it was told to, the rest of the child
 * can be viewed by scrolling.
 *
 */
class GG_API ScrollPanel : public Wnd
{
public:
    ScrollPanel() = default;
    ScrollPanel(X x, Y y, X w, Y h, std::shared_ptr<Wnd> content) :
        Wnd(x, y, w, h, INTERACTIVE),
        m_content(std::move(content))
    {}
    ~ScrollPanel() = default;
    void CompleteConstruction() override;

    void SizeMove(Pt ul, Pt lr) override;
    void Render() override;

    //! Set the scroll position
    void ScrollTo(Y pos);

    //! Sets the background color of the panel.
    void SetBackgroundColor(Clr color) noexcept { m_background_color = color; }

    //! Returns the scroll bar.
    const Scroll* GetScroll() const noexcept { return m_vscroll.get();}

    void MouseWheel(Pt pt, int move, Flags<ModKey> mod_keys) override;
    void KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys) override;

private:
    std::shared_ptr<Scroll> m_vscroll;      //!< The vertical scroll bar.
    std::shared_ptr<Wnd> m_content;         //!< The content window of the panel.
    Pt m_content_pos;                       //!< The position of the content when scrolled properly.
    Clr m_background_color = GG::CLR_ZERO;  //!< The color to paint the background with.

    /**
     * @brief Refreshes the size and positions of the children of this window.
     *
     * @return void
     */
    void DoLayout();

    //! Called when the scroll position changes.
    void OnScrolled(int tab_min, int tab_max, int min, int max);
};

}


#endif
