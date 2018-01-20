/* GG is a GUI for SDL and OpenGL.

   Copyright (C) 2015 Mitten-O

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

#ifndef GG_SCROLLPANEL_H
#define GG_SCROLLPANEL_H

#include <GG/Wnd.h>
#include <GG/Clr.h>

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
    ScrollPanel();
    //! Create a ScrollPanel with content.
    ScrollPanel(X x, Y y, X w, Y h, std::shared_ptr<Wnd> content);
    virtual ~ScrollPanel();

    void CompleteConstruction() override;

    void SizeMove(const Pt& ul, const Pt& lr) override;
    void Render() override;

    //! Set the scroll position
    void ScrollTo(Y pos);

    //! Sets the background color of the panel.
    void SetBackgroundColor(const Clr& color);

    //! Returns the scroll bar.
    const Scroll* GetScroll() const
    { return m_vscroll.get();}

    void MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys) override;
    void KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys) override;

private:
    std::shared_ptr<Scroll> m_vscroll; //!< The vertical scroll bar.
    std::shared_ptr<Wnd> m_content; //!< The content window of the panel.
    Pt m_content_pos; //!< The position of the content when scrolled properly.
    Clr m_background_color; //!< The color to paint the background with.

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

#endif // GG_SCROLLPANEL_H
