#include "AccordionPanel.h"

#include "ClientUI.h"
#include "CUIControls.h"

AccordionPanel::AccordionPanel(GG::X w) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y(ClientUI::Pts()*2), GG::INTERACTIVE),
    m_expand_button(0)
{
    boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

    m_expand_button = new CUIButton(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "downarrownormal.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "downarrowclicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "downarrowmouseover.png")));
    m_expand_button->SetMinSize(GG::Pt(GG::X(16), GG::Y(16)));

    AttachChild(m_expand_button);

    DoLayout();
}

void AccordionPanel::Render() {
    if (Height() < 1 || Width() < 1)
        return;

    // Draw outline and background...
    GG::FlatRectangle(UpperLeft(), LowerRight(), ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}

void AccordionPanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void AccordionPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoLayout();
}

void AccordionPanel::SetCollapsed(bool collapsed) {
    boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

    if (!collapsed) {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "uparrownormal.png"   )));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "uparrowclicked.png"  )));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "uparrowmouseover.png")));
    } else {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "downarrownormal.png"   )));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "downarrowclicked.png"  )));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "downarrowmouseover.png")));
    }

    ExpandCollapseSignal();
}

void AccordionPanel::DoLayout() {
    GG::Pt expand_button_ul(Width() - 16, GG::Y0);
    GG::Pt expand_button_lr = expand_button_ul + GG::Pt(GG::X(16), GG::Y(16));
    m_expand_button->SizeMove(expand_button_ul, expand_button_lr);
}
