#include "TextBrowseWnd.h"

namespace {
    /** Returns height of rows of text in InfoTextBrowseWnd. */
    int IconTextBrowseWndRowHeight() {
        return ClientUI::Pts()*3/2;
    }

    const int       EDGE_PAD(3);

    const GG::X     BROWSE_TEXT_WIDTH(200);
    const GG::Y     ICON_BROWSE_ICON_HEIGHT(64);
}

TextBrowseWnd::TextBrowseWnd(const std::string& title_text, const std::string& main_text) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, BROWSE_TEXT_WIDTH, GG::Y1)
{
    const GG::Y ROW_HEIGHT(IconTextBrowseWndRowHeight());

    m_offset = GG::Pt(GG::X0, ICON_BROWSE_ICON_HEIGHT/2); //lower the window

    m_title_text = new CUILabel(title_text, GG::FORMAT_LEFT);
    m_title_text->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, GG::Y0 + m_offset.y));
    m_title_text->Resize(GG::Pt(BROWSE_TEXT_WIDTH, ROW_HEIGHT));
    m_title_text->SetFont(ClientUI::GetBoldFont());

    m_main_text = new CUILabel(main_text, GG::FORMAT_LEFT | GG::FORMAT_TOP | GG::FORMAT_WORDBREAK);
    m_main_text->MoveTo(GG::Pt(GG::X(EDGE_PAD) + m_offset.x, ROW_HEIGHT + m_offset.y));
    m_main_text->Resize(GG::Pt(BROWSE_TEXT_WIDTH, ICON_BROWSE_ICON_HEIGHT));
    m_main_text->SetMinSize(true);
    m_main_text->Resize(m_main_text->MinSize());

    AttachChild(m_main_text);
    AttachChild(m_title_text);

    Resize(GG::Pt(BROWSE_TEXT_WIDTH, ROW_HEIGHT + m_main_text->Height()));
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
