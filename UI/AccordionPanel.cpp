#include "AccordionPanel.h"

#include "ClientUI.h"
#include "CUIControls.h"

namespace {
    constexpr int EXPAND_BUTTON_SIZE = 20;
}

AccordionPanel::AccordionPanel(GG::X w, GG::Y h, bool is_button_on_left) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
    m_is_left(is_button_on_left),
    m_interior_color(ClientUI::WndColor())
{}

void AccordionPanel::CompleteConstruction() {
    GG::Control::CompleteConstruction();
    boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

    m_expand_button = Wnd::Create<CUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "downarrownormal.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "downarrowclicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "downarrowmouseover.png")));
    m_expand_button->SetMinSize(GG::Pt(GG::X(EXPAND_BUTTON_SIZE), GG::Y(EXPAND_BUTTON_SIZE)));
    m_expand_button->NonClientChild(true);

    AttachChild(m_expand_button);

    // Don't call the virtual function, derived classes may not be completed
    AccordionPanel::DoLayout();
    InitBuffer();
}

void AccordionPanel::InitBuffer() {
    const auto sz = Size();
    m_border_buffer.clear();
    m_border_buffer.store(0.0f,        0.0f);
    m_border_buffer.store(Value(sz.x), 0.0f);
    m_border_buffer.store(Value(sz.x), Value(sz.y));
    m_border_buffer.store(0.0f,        Value(sz.y));
    m_border_buffer.store(0.0f,        0.0f);
    m_border_buffer.createServerBuffer();
}

GG::Pt AccordionPanel::ClientUpperLeft() const noexcept
{ return UpperLeft() + GG::Pt((m_is_left ? GG::X(EXPAND_BUTTON_SIZE + m_border_margin) : GG::X0), GG::Y0); }

GG::Pt AccordionPanel::ClientLowerRight() const noexcept
{ return LowerRight() - GG::Pt((m_is_left ? GG::X0 : GG::X(EXPAND_BUTTON_SIZE + m_border_margin)), GG::Y0); }

void AccordionPanel::SetInteriorColor(GG::Clr c)
{ m_interior_color = c; }

void AccordionPanel::SetBorderMargin(int margin)
{ m_border_margin = std::max<int>(0, margin); }

void AccordionPanel::Render() {
    if (Height() < GG::Y1 || Width() < GG::X1)
        return;

    GG::Pt ul = UpperLeft();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(static_cast<GLfloat>(Value(ul.x)), static_cast<GLfloat>(Value(ul.y)), 0.0f);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.0f);
    glEnableClientState(GL_VERTEX_ARRAY);

    m_border_buffer.activate();
    glColor(m_interior_color);
    glDrawArrays(GL_TRIANGLE_FAN,   0, m_border_buffer.size() - 1);
    glColor(ClientUI::WndOuterBorderColor());
    glDrawArrays(GL_LINE_STRIP,     0, m_border_buffer.size());

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);
}

void AccordionPanel::MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void AccordionPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size()) {
        DoLayout();
        InitBuffer();
    }
}

void AccordionPanel::SetCollapsed(bool collapsed) {
    if (collapsed == m_collapsed)
        return;

    boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

    m_collapsed = collapsed;
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

bool AccordionPanel::IsCollapsed() const
{ return m_collapsed; }

void AccordionPanel::DoLayout() {
    GG::Pt expand_button_ul(m_is_left ? GG::X(-(EXPAND_BUTTON_SIZE + m_border_margin))
                            : (Width() + GG::X(-(EXPAND_BUTTON_SIZE + m_border_margin))), GG::Y0);
    GG::Pt expand_button_lr = expand_button_ul + GG::Pt(GG::X(EXPAND_BUTTON_SIZE), GG::Y(EXPAND_BUTTON_SIZE));
    Wnd::MoveChildUp(m_expand_button.get());
    m_expand_button->SizeMove(expand_button_ul, expand_button_lr);
}
