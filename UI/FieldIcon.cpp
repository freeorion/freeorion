#include "FieldIcon.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"
#include "../universe/Field.h"
#include "../util/AppInterface.h"
#include "../util/i18n.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/DynamicGraphic.h>
#include <GG/WndEvent.h>

////////////////////////////////////////////////
// FieldIcon
////////////////////////////////////////////////
FieldIcon::FieldIcon(int field_id) :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1),
    m_field_id(field_id),
    m_selection_indicator(nullptr),
    m_mouseover_indicator(nullptr),
    m_selected(false),
    m_name(nullptr)
{
    Refresh();
}

FieldIcon::~FieldIcon()
{}

int FieldIcon::FieldID() const
{ return m_field_id; }

const std::shared_ptr<GG::Texture>& FieldIcon::FieldTexture() const
{ return m_texture; }

void FieldIcon::LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void FieldIcon::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void FieldIcon::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void FieldIcon::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    Wnd::SizeMove(ul, lr);

    GG::Pt middle = GG::Pt(Width() / 2, Height() / 2);

    const int SEL_IND_WIDTH_HEIGHT = ClientUI::SystemSelectionIndicatorSize() * Value(Width()) / ClientUI::SystemIconSize();
    const GG::Pt SEL_IND_SIZE = GG::Pt(GG::X(SEL_IND_WIDTH_HEIGHT), GG::Y(SEL_IND_WIDTH_HEIGHT));

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
    std::shared_ptr<const Field> field = GetField(m_field_id);
    if (!field)
        return;
    m_texture = ClientUI::FieldTexture(field->FieldTypeName());
}

void FieldIcon::Render()
{}

void FieldIcon::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        LeftClickedSignal(m_field_id);
    ForwardEventToParent();
}

void FieldIcon::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        RightClickedSignal(m_field_id);

    std::shared_ptr<const Field> field = GetField(m_field_id);
    if (!field)
        return;
    const std::string& field_type_name = field->FieldTypeName();
    if (field_type_name.empty())
        return;

    std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(field_type_name));
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    auto pedia_lookup_field_type_action = [field_type_name]() { ClientUI::GetClientUI()->ZoomToFieldType(field_type_name); };
    popup->AddMenuItem(GG::MenuItem(popup_label, false, false, pedia_lookup_field_type_action));
    popup->Run();
}

void FieldIcon::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        LeftDoubleClickedSignal(m_field_id);
    ForwardEventToParent();
}

void FieldIcon::RDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        RightDoubleClickedSignal(m_field_id);
    ForwardEventToParent();
}

void FieldIcon::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    //// indicate mouseover
    //if (m_mouseover_indicator) {
    //    AttachChild(m_mouseover_indicator);
    //    MoveChildUp(m_mouseover_indicator);
    //} else if (m_mouseover_indicator) {
    //    DetachChild(m_mouseover_indicator);
    //}
    //MouseEnteringSignal(m_field_id);
}

void FieldIcon::MouseLeave() {
    //// un-indicate mouseover
    //if (m_mouseover_indicator)
    //    DetachChild(m_mouseover_indicator);

    //MouseLeavingSignal(m_field_id);
}

void FieldIcon::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void FieldIcon::SetSelected(bool selected) {
    m_selected = selected;
    Resize(Size());
}

bool FieldIcon::InWindow(const GG::Pt& pt) const {
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

    //std::cout << "Radius: " << RADIUS << "  dist: " << std::sqrt<double>(distx*distx + disty*disty) << std::endl;

    return distx*distx + disty*disty <= RADIUS2;
}
