#include "TextBrowseWnd.h"

#include "CUIControls.h"

namespace {
    /** Returns height of rows of text in InfoTextBrowseWnd. */
    int IconTextBrowseWndRowHeight() {
        return ClientUI::Pts()*3/2;
    }

    const int       EDGE_PAD(3);

    const GG::Y     ICON_BROWSE_ICON_HEIGHT(64);
}

TextBrowseWnd::TextBrowseWnd(const std::string& title_text, const std::string& main_text, GG::X w /* = GG::X(200) */) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, w, GG::Y1)
{
    const GG::Y ROW_HEIGHT(IconTextBrowseWndRowHeight());

    m_offset = GG::Pt(GG::X0, ICON_BROWSE_ICON_HEIGHT/2); //lower the window

    m_title_text = GG::Wnd::Create<CUILabel>(title_text, GG::FORMAT_LEFT);
    m_title_text->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, GG::Y0 + m_offset.y));
    m_title_text->Resize(GG::Pt(w, ROW_HEIGHT));
    m_title_text->SetFont(ClientUI::GetBoldFont());

    m_main_text = GG::Wnd::Create<CUILabel>(main_text, GG::FORMAT_LEFT | GG::FORMAT_TOP | GG::FORMAT_WORDBREAK);
    m_main_text->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, ROW_HEIGHT + m_offset.y));
    m_main_text->Resize(GG::Pt(w, ICON_BROWSE_ICON_HEIGHT));
    m_main_text->SetResetMinSize(true);
    m_main_text->Resize(m_main_text->MinSize());
}

void TextBrowseWnd::CompleteConstruction() {
    GG::BrowseInfoWnd::CompleteConstruction();

    AttachChild(m_main_text);
    AttachChild(m_title_text);

    Resize(GG::Pt(Width(), IconTextBrowseWndRowHeight() + m_main_text->Height()));
}

bool TextBrowseWnd::WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const {
    assert(mode <= wnd->BrowseModes().size());
    return true;
}

void TextBrowseWnd::Render() {
    GG::Pt      ul = UpperLeft();
    GG::Pt      lr = LowerRight();
    const GG::Y ROW_HEIGHT(IconTextBrowseWndRowHeight());
    GG::FlatRectangle(ul + m_offset, lr + m_offset, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);    // main background
    GG::FlatRectangle(ul + m_offset, GG::Pt(lr.x, ul.y + ROW_HEIGHT) + m_offset, ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);    // top title filled background
}
