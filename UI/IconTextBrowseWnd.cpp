#include "IconTextBrowseWnd.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

#include "ClientUI.h"
#include "CUIControls.h"

namespace {
    const int       EDGE_PAD(3);

    /** Returns height of rows of text in InfoTextBrowseWnd. */
    int IconTextBrowseWndRowHeight() {
        return ClientUI::Pts()*3/2;
    }

    const GG::X     ICON_BROWSE_TEXT_WIDTH(400);
    const GG::X     ICON_BROWSE_ICON_WIDTH(64);
    const GG::Y     ICON_BROWSE_ICON_HEIGHT(64);
}

IconTextBrowseWnd::IconTextBrowseWnd(const std::shared_ptr<GG::Texture> texture,
                                     const std::string& title_text,
                                     const std::string& main_text) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, ICON_BROWSE_TEXT_WIDTH + ICON_BROWSE_ICON_WIDTH, GG::Y1),
    m_icon(nullptr),
    m_title_text_label(nullptr),
    m_main_text_label(nullptr),
    m_texture(texture),
    m_title_text(title_text),
    m_main_text(main_text)
{ RequirePreRender(); }

bool IconTextBrowseWnd::WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const {
    assert(mode <= wnd->BrowseModes().size());
    return true;
}

void IconTextBrowseWnd::PreRender() {
    GG::Wnd::PreRender();

    m_icon = GG::Wnd::Create<GG::StaticGraphic>(m_texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::INTERACTIVE);
    m_icon->Resize(GG::Pt(ICON_BROWSE_ICON_WIDTH, ICON_BROWSE_ICON_HEIGHT));
    AttachChild(m_icon);

    const GG::Y ROW_HEIGHT(IconTextBrowseWndRowHeight());

    m_title_text_label = GG::Wnd::Create<CUILabel>(m_title_text, GG::FORMAT_LEFT);
    m_title_text_label->MoveTo(GG::Pt(m_icon->Width() + GG::X(EDGE_PAD), GG::Y0));
    m_title_text_label->Resize(GG::Pt(ICON_BROWSE_TEXT_WIDTH, ROW_HEIGHT));
    m_title_text_label->SetFont(ClientUI::GetBoldFont());

    m_main_text_label = GG::Wnd::Create<CUILabel>(m_main_text, GG::FORMAT_LEFT | GG::FORMAT_TOP | GG::FORMAT_WORDBREAK);
    m_main_text_label->MoveTo(GG::Pt(m_icon->Width() + GG::X(EDGE_PAD), ROW_HEIGHT));
    m_main_text_label->Resize(GG::Pt(ICON_BROWSE_TEXT_WIDTH, ICON_BROWSE_ICON_HEIGHT));
    m_main_text_label->SetResetMinSize(true);
    m_main_text_label->Resize(m_main_text_label->MinSize());

    AttachChild(m_title_text_label);
    AttachChild(m_main_text_label);

    Resize(GG::Pt(ICON_BROWSE_TEXT_WIDTH + ICON_BROWSE_ICON_WIDTH,
                  std::max(m_icon->Height(), ROW_HEIGHT + m_main_text_label->Height())));
}

void IconTextBrowseWnd::Render() {
    GG::Pt      ul = UpperLeft();
    GG::Pt      lr = LowerRight();
    const GG::Y ROW_HEIGHT(IconTextBrowseWndRowHeight());
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);    // main background
    GG::FlatRectangle(GG::Pt(ul.x + ICON_BROWSE_ICON_WIDTH, ul.y), GG::Pt(lr.x, ul.y + ROW_HEIGHT),
                      ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);    // top title filled background
}
