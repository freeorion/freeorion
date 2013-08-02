#include "FieldIcon.h"

#include "ClientUI.h"
#include "../universe/Field.h"
#include "../util/AppInterface.h"
#include "../util/i18n.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/DynamicGraphic.h>
#include <GG/TextControl.h>
#include <GG/WndEvent.h>
#include <GG/Menu.h>

namespace {
    const std::vector<boost::shared_ptr<GG::Texture> >& GetSelectionIndicatorTextures() {
        static std::vector<boost::shared_ptr<GG::Texture> > normal_textures =
            ClientUI::GetClientUI()->GetPrefixedTextures(ClientUI::ArtDir() / "misc" / "system_selection", "system_selection", true);
        return normal_textures;
    }
}

////////////////////////////////////////////////
// FieldIcon
////////////////////////////////////////////////
FieldIcon::FieldIcon(GG::X x, GG::Y y, int field_id) :
    GG::Control(x, y, GG::X1, GG::Y1),
    m_field_id(field_id),
    m_selection_indicator(0),
    m_mouseover_indicator(0),
    m_selected(false),
    m_name(0)
{
    //// selection indicator graphic
    //const std::vector<boost::shared_ptr<GG::Texture> >& textures = GetSelectionIndicatorTextures();
    //GG::X texture_width = textures.at(0)->DefaultWidth();
    //GG::Y texture_height = textures.at(0)->DefaultHeight();
    //m_selection_indicator = new GG::DynamicGraphic(GG::X0, GG::Y0,
    //                                               texture_width, texture_height, true,
    //                                               texture_width, texture_height, 0, textures,
    //                                               GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    //m_selection_indicator->SetFPS(ClientUI::SystemSelectionIndicatorFPS());
    //AttachChild(m_selection_indicator);
    //m_selection_indicator->Play();

    //// mouseover indicator graphic
    //boost::shared_ptr<GG::Texture> mouseover_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "system_mouseover.png");
    //texture_width = mouseover_texture->DefaultWidth();
    //texture_height = mouseover_texture->DefaultHeight();
    //m_mouseover_indicator = new GG::StaticGraphic(GG::X0, GG::Y0, texture_width, texture_height,
    //                                              mouseover_texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

    Refresh();
}

FieldIcon::~FieldIcon() {
    delete m_selection_indicator;
    delete m_mouseover_indicator;
    delete m_name;
}

int FieldIcon::FieldID() const
{ return m_field_id; }

const boost::shared_ptr<GG::Texture>& FieldIcon::FieldTexture() const
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
    TemporaryPtr<const Field> field = GetField(m_field_id);
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

    GG::MenuItem menu_contents;

    TemporaryPtr<const Field> field = GetField(m_field_id);
    if (!field)
        return;
    const std::string& field_type_name = field->FieldTypeName();
    if (field_type_name.empty())
        return;

    std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(field_type_name));
    menu_contents.next_level.push_back(GG::MenuItem(popup_label, 1, false, false));
    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                        ClientUI::WndOuterBorderColor(), ClientUI::WndColor());

    if (!popup.Run() || popup.MenuID() != 1)
        return;

    ClientUI::GetClientUI()->ZoomToFieldType(field_type_name);
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
