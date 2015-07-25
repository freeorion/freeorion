#ifndef GG_SCROLLPANEL_H
#define GG_SCROLLPANEL_H

#include <GG/Wnd.h>
#include <GG/Clr.h>

namespace GG {

class Scroll;

/**
 * @brief A panel that allows scrollin its contents.
 *
 * Only Vertical scrolling is implemented for now.
 * The scroll panel takes on child and tells it to fill
 * the client area of the panel. If the child decides
 * to be taller than it was told to, the rest of the child
 * can be viewed by scrolling.
 *
 */
class ScrollPanel : public Wnd
{
public:
    virtual void SizeMove(const Pt& ul, const Pt& lr);
    virtual void Render();
    ScrollPanel();
    //! Create a ScrollPanel with content.
    ScrollPanel(X x, Y y, X w, Y h, Wnd* content);

    //! Set the scroll position
    void ScrollTo(X pos);

    //! Sets the background color of the panel.
    void SetBackgroundColor(const Clr& color);
protected:
    virtual void MouseWheel(const Pt& pt, int move, GG::Flags< GG::ModKey > mod_keys);

private:
    Scroll* m_vscroll; //!< The vertical scroll bar.
    Wnd* m_content; //!< The content window of the panel.
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
