#include "../GG/ScrollPanel.h"

#include <GG/DrawUtil.h>
#include <GG/Flags.h>
#include <GG/Scroll.h>
#include <GG/Clr.h>
#include <GG/ClrConstants.h>
#include <GG/WndEvent.h>
#include <GG/StyleFactory.h>

#include <algorithm>

namespace GG {

using std::max;
using std::min;

namespace{
    // Minimum acceptable width for a scroll.
    X MIN_SCROLL_WIDTH(14);

    // Space to leave left of the vertical scroll.
    X SCROLL_MARGIN_X(2);
}

void ScrollPanel::SizeMove(const Pt& ul, const Pt& lr)
{
    // Store the new size for input to the layout.
    Wnd::SizeMove(ul, lr);

    // Lay out the children.
    DoLayout();
}

void ScrollPanel::Render()
{
    FlatRectangle(UpperLeft(), LowerRight(), m_background_color, CLR_ZERO, 0);
}

ScrollPanel::ScrollPanel():
m_vscroll(0),
m_content(0),
m_background_color(CLR_ZERO)
{

}

ScrollPanel::ScrollPanel(X x, Y y, X w, Y h, Wnd* content):
Wnd(x, y, w, h, INTERACTIVE),
m_vscroll( 0 ),
m_content( content),
m_background_color(CLR_ZERO)
{
    // Very important to clip the content of this panel,
    // to actually only show the currently viewed part.
    SetChildClippingMode(ClipToClient);

    // Get the scroll bar from the current style factory.
    boost::shared_ptr<StyleFactory> style = GetStyleFactory();
    m_vscroll = style->NewMultiEditVScroll(CLR_WHITE, CLR_BLACK);

    // Don't accept less than MIN_SCROLL_WIDTH pixels wide scrolls.
    if(m_vscroll->Width() < MIN_SCROLL_WIDTH){
        m_vscroll->Resize(GG::Pt(MIN_SCROLL_WIDTH, m_vscroll->Height()));
    }

    AttachChild(m_vscroll);
    AttachChild(content);

    Connect( m_vscroll->ScrolledSignal, &ScrollPanel::OnScrolled, this );

    DoLayout();
}

void GG::ScrollPanel::ScrollTo(X pos)
{
    m_vscroll->ScrollTo(Value(pos));
    SignalScroll(*m_vscroll, true);
}

void ScrollPanel::SetBackgroundColor(const Clr& color)
{
    m_background_color = color;
}

void ScrollPanel::MouseWheel(const Pt& pt, int move, GG::Flags< GG::ModKey > mod_keys)
{
    m_vscroll->ScrollLineIncr(-move);
    SignalScroll(*m_vscroll, true);
}

void ScrollPanel::DoLayout()
{
    // Position the scroll bar to the right edge of the panel.
    Pt scroll_ul(Width() - m_vscroll->Width(), Y0);
    Pt scroll_lr(Width(), Height() - 1);
    m_vscroll->SizeMove(scroll_ul, scroll_lr);

    // Let the content fill all room left from the scroll.
    Pt content_lr(ClientSize().x - m_vscroll->Width() - SCROLL_MARGIN_X, m_content_pos.y + m_content->Height());
    m_content->SizeMove(m_content_pos, content_lr);

    // The scroll position is directly the offset of the
    // content in pixels.

    // Start from no offset.
    m_vscroll->SetMin(0);

    // Allow the bottom of the content to be scrolled to the middle of the viewport.
    m_vscroll->SetMax(Value(m_content->Height() + Height()/2));

    // Move a viewportfull at a time with page up and down.
    m_vscroll->SetPageSize(Value(Height()));

    // Make scrolling fairly speedy.
    m_vscroll->SetLineSize(Value(Height()/10));
}

void ScrollPanel::OnScrolled(int tab_min, int tab_max, int min, int max)
{
    // Move the content up by the offset given by the top of the tab.
    m_content_pos.y = -Y(tab_min);

    // Immediately move the content.
    m_content->MoveTo(m_content_pos);
}


}
