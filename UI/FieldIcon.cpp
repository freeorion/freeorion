#include "FieldIcon.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"
#include "../client/human/GGHumanClientApp.h"
#include "../universe/Field.h"
#include "../util/AppInterface.h"
#include "../util/i18n.h"

#include <GG/StaticGraphic.h>
#include <GG/DynamicGraphic.h>
#include <GG/WndEvent.h>

////////////////////////////////////////////////
// FieldIcon
////////////////////////////////////////////////
FieldIcon::FieldIcon(int field_id) :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1),
    m_field_id(field_id)
{}

void FieldIcon::CompleteConstruction() {
    GG::Control::CompleteConstruction();

    // mouseover indicator graphic
    auto mouseover_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "field_mouseover.png");
    GG::Pt sz{mouseover_texture->DefaultWidth(), mouseover_texture->DefaultHeight()};
    m_mouseover_indicator = GG::Wnd::Create<GG::StaticGraphic>(
        std::move(mouseover_texture), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_mouseover_indicator->Resize(sz);

    Refresh();
}

void FieldIcon::LButtonDown(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void FieldIcon::LDrag(GG::Pt pt, GG::Pt move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void FieldIcon::LButtonUp(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void FieldIcon::SizeMove(GG::Pt ul, GG::Pt lr) {
    Wnd::SizeMove(ul, lr);

    const auto middle = GG::Pt(Width() / 2, Height() / 2);
    const auto SEL_IND_SIZE = Size();

    // selection indicator
    if (m_selected && m_selection_indicator) {
        GG::Pt sel_ind_ul(static_cast<GG::X>(middle.x - SEL_IND_SIZE.x / 2.0),
                          static_cast<GG::Y>(middle.y - SEL_IND_SIZE.y / 2.0));
        m_selection_indicator->SizeMove(sel_ind_ul, sel_ind_ul + SEL_IND_SIZE);
        AttachChild(m_selection_indicator);
        m_selection_indicator->Show();
    } else if (m_selection_indicator) {
        DetachChild(m_selection_indicator);
        m_selection_indicator->Hide();
    }

    // mouseover indicator - attach / detach / show / hide done by MouseEnter and MouseLeave
    if (m_mouseover_indicator) {
        GG::Pt mouse_ind_ul(static_cast<GG::X>(middle.x - SEL_IND_SIZE.x / 2.0),
                            static_cast<GG::Y>(middle.y - SEL_IND_SIZE.y / 2.0));
        m_mouseover_indicator->SizeMove(mouse_ind_ul, mouse_ind_ul + SEL_IND_SIZE);
    } else {
        DetachChild(m_mouseover_indicator);
    }
}

void FieldIcon::Refresh() {
    if (auto field = GGHumanClientApp::GetApp()->GetContext().ContextObjects().get<Field>(m_field_id))
        m_texture = ClientUI::FieldTexture(field->FieldTypeName());
}

void FieldIcon::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        LeftClickedSignal(m_field_id);
    ForwardEventToParent();
}

void FieldIcon::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        RightClickedSignal(m_field_id);

    auto field = GGHumanClientApp::GetApp()->GetContext().ContextObjects().get<Field>(m_field_id);
    if (!field)
        return;
    const std::string& field_type_name = field->FieldTypeName();
    if (field_type_name.empty())
        return;

    std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(field_type_name));
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    auto pedia_lookup_field_type_action = [field_type_name]() { ClientUI::GetClientUI()->ZoomToFieldType(field_type_name); };
    popup->AddMenuItem(GG::MenuItem(std::move(popup_label), false, false,
                                    pedia_lookup_field_type_action));
    popup->Run();
}

void FieldIcon::LDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        LeftDoubleClickedSignal(m_field_id);
    ForwardEventToParent();
}

void FieldIcon::RDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        RightDoubleClickedSignal(m_field_id);
    ForwardEventToParent();
}

void FieldIcon::MouseEnter(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    // indicate mouseover
    if (m_mouseover_indicator) {
        AttachChild(m_mouseover_indicator);
        MoveChildUp(m_mouseover_indicator);
    } else if (m_mouseover_indicator) {
        DetachChild(m_mouseover_indicator);
    }
    MouseEnteringSignal(m_field_id);
}

void FieldIcon::MouseLeave() {
    // un-indicate mouseover
    if (m_mouseover_indicator)
        DetachChild(m_mouseover_indicator);
    MouseLeavingSignal(m_field_id);
}

void FieldIcon::MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void FieldIcon::SetSelected(bool selected) {
    m_selected = selected;
    Resize(Size());
}

bool FieldIcon::InWindow(GG::Pt pt) const noexcept {
    // find if cursor is within required distance of centre of icon
    const int RADIUS = Value(Width())/2;
    const int RADIUS2 = RADIUS*RADIUS;

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt size = lr - ul;
    GG::Pt half_size = GG::Pt(size.x / 2, size.y / 2);
    GG::Pt middle = ul + half_size;

    GG::Pt delta = pt - middle;

    const int distx = Value(delta.x);
    const int disty = Value(delta.y);

    return distx*distx + disty*disty <= RADIUS2;
}
