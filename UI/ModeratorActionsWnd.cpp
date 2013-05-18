#include "ModeratorActionsWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../util/MultiplayerCommon.h"
#include "../util/AppInterface.h"
#include "../util/OptionsDB.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "InfoPanels.h"
#include <GG/Button.h>

namespace {
    const GG::X CONTROL_WIDTH(32);
    const GG::Y CONTROL_HEIGHT(32);
    const GG::X DROP_WIDTH(96);
    const int   PAD(3);
}

////////////////////////////////////////////////
// ModeratorActionsWnd
////////////////////////////////////////////////
ModeratorActionsWnd::ModeratorActionsWnd(GG::X w, GG::Y h) :
    CUIWnd(UserString("MODERATOR"), GG::X1, GG::Y1, w - 1, h - 1, GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE),
    m_actions_enabled(true),
    m_no_action_button(0),
    m_create_system_button(0),
    m_star_type_drop(0),
    m_create_planet_button(0),
    m_planet_type_drop(0),
    m_delete_object_button(0),
    m_set_owner_button(0),
    m_empire_drop(0),
    m_create_starlane_button(0)
{
    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
    ClientUI* ui = ClientUI::GetClientUI();
    GG::Flags<GG::GraphicStyle> style = GG::GRAPHIC_CENTER | GG::GRAPHIC_VCENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE;

    // button for no action
    m_no_action_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_no_action_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "nomoderatoraction.png")));
    m_no_action_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "nomoderatoraction_clicked.png"  )));
    m_no_action_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "nomoderatoraction_mouseover.png")));
    m_no_action_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_no_action_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_NONE"), UserString("MOD_NONE"))));
    AttachChild(m_no_action_button);
    GG::Connect(m_no_action_button->ClickedSignal,      &ModeratorActionsWnd::NoActionClicked,      this);

    // button for create system and droplist to select system type to create
    m_create_system_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_create_system_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "addstar.png")));
    m_create_system_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "addstar_clicked.png"  )));
    m_create_system_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "addstar_mouseover.png")));
    m_create_system_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_create_system_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_CREATE_SYSTEM"), UserString("MOD_CREATE_SYSTEM"))));
    AttachChild(m_create_system_button);

    GG::Connect(m_create_system_button->ClickedSignal,  &ModeratorActionsWnd::CreateSystemClicked,  this);
    m_star_type_drop = new CUIDropDownList(GG::X0, GG::Y0, DROP_WIDTH, CONTROL_HEIGHT, CONTROL_HEIGHT*8);
    for (StarType star_type = STAR_BLUE; star_type != NUM_STAR_TYPES; star_type = StarType(star_type + 1)) {
        boost::shared_ptr<GG::Texture> disc_texture = ui->GetModuloTexture(
            ClientUI::ArtDir() / "stars", ClientUI::StarTypeFilePrefixes()[star_type], 0);
        GG::DropDownList::Row* row = new GG::DropDownList::Row();
        row->push_back(new GG::StaticGraphic(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT,
                                             disc_texture, style, GG::Flags<GG::WndFlag>()));
        m_star_type_drop->Insert(row);
    }
    m_star_type_drop->Select(m_star_type_drop->begin());
    GG::Connect(m_star_type_drop->SelChangedSignal,     &ModeratorActionsWnd::StarTypeSelected,     this);

    // button for create planet and droplist to select planet type to create
    m_create_planet_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_create_planet_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "addplanet.png")));
    m_create_planet_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "addplanet_clicked.png"  )));
    m_create_planet_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "addplanet_mouseover.png")));
    m_create_planet_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_create_planet_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_CREATE_PLANET"), UserString("MOD_CREATE_PLANET"))));
    AttachChild(m_create_planet_button);

    GG::Connect(m_create_planet_button->ClickedSignal,  &ModeratorActionsWnd::CreatePlanetClicked,  this);
    m_planet_type_drop = new CUIDropDownList(GG::X0, GG::Y0, DROP_WIDTH, CONTROL_HEIGHT, CONTROL_HEIGHT*10);
    for (PlanetType planet_type = PT_SWAMP; planet_type != NUM_PLANET_TYPES; planet_type = PlanetType(planet_type + 1)) {
        boost::shared_ptr<GG::Texture> texture = ClientUI::PlanetIcon(planet_type);
        GG::DropDownList::Row* row = new GG::DropDownList::Row();
        row->push_back(new GG::StaticGraphic(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT,
                                             texture, style, GG::Flags<GG::WndFlag>()));
        m_planet_type_drop->Insert(row);
    }
    m_planet_type_drop->Select(m_planet_type_drop->begin());
    GG::Connect(m_planet_type_drop->SelChangedSignal,   &ModeratorActionsWnd::PlanetTypeSelected,   this);

    // button for destroying object
    m_delete_object_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_delete_object_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "delete.png")));
    m_delete_object_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "delete_clicked.png"  )));
    m_delete_object_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "delete_mouseover.png")));
    m_delete_object_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_delete_object_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_DESTROY"), UserString("MOD_DESTROY"))));
    AttachChild(m_delete_object_button);
    GG::Connect(m_delete_object_button->ClickedSignal,  &ModeratorActionsWnd::DeleteObjectClicked,  this);

    // button for setting owner
    m_set_owner_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_set_owner_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "setowner.png")));
    m_set_owner_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "setowner_clicked.png"  )));
    m_set_owner_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "setowner_mouseover.png")));
    m_set_owner_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_set_owner_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_SET_OWNER"), UserString("MOD_SET_OWNER"))));
    AttachChild(m_set_owner_button);

    GG::Connect(m_set_owner_button->ClickedSignal,      &ModeratorActionsWnd::SetOwnerClicked,      this);
    m_empire_drop = new CUIDropDownList(GG::X0, GG::Y0, DROP_WIDTH, CONTROL_HEIGHT, CONTROL_HEIGHT*10);
    // empires added later when gamestate info available
    GG::Connect(m_empire_drop->SelChangedSignal,        &ModeratorActionsWnd::EmpireSelected,       this);

    // button for creating starlane
    m_create_starlane_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_create_starlane_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "addstarlane.png")));
    m_create_starlane_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "addstarlane_clicked.png"  )));
    m_create_starlane_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "moderator" / "addstarlane_mouseover.png")));
    m_create_starlane_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_create_starlane_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_STARLANE"), UserString("MOD_STARLANE"))));
    AttachChild(m_create_starlane_button);
    GG::Connect(m_create_starlane_button->ClickedSignal,&ModeratorActionsWnd::CreateStarlaneClicked,this);

    DoLayout();
}

void ModeratorActionsWnd::NoActionClicked() {
    NoActionSelectedSignal();
    DetachChild(m_planet_type_drop);
    DetachChild(m_empire_drop);
    DetachChild(m_star_type_drop);
}

void ModeratorActionsWnd::CreateSystemClicked() {
    CreateSystemActionSelectedSignal(SelectedStarType());
    DetachChild(m_planet_type_drop);
    DetachChild(m_empire_drop);
    AttachChild(m_star_type_drop);
}

void ModeratorActionsWnd::StarTypeSelected(GG::DropDownList::iterator it)
{ CreateSystemClicked(); }

void ModeratorActionsWnd::CreatePlanetClicked() {
    CreatePlanetActionSelectedSignal(SelectedPlanetType());
    DetachChild(m_star_type_drop);
    DetachChild(m_empire_drop);
    AttachChild(m_planet_type_drop);
}

void ModeratorActionsWnd::PlanetTypeSelected(GG::DropDownList::iterator it)
{ CreatePlanetClicked(); }

void ModeratorActionsWnd::DeleteObjectClicked() {
    DeleteObjectActionSelectedSignal();
    DetachChild(m_planet_type_drop);
    DetachChild(m_empire_drop);
    DetachChild(m_star_type_drop);
}

void ModeratorActionsWnd::SetOwnerClicked() {
    SetOwnerActionSelectedSignal(SelectedEmpire());
    DetachChild(m_star_type_drop);
    DetachChild(m_planet_type_drop);
    AttachChild(m_empire_drop);
}

void ModeratorActionsWnd::EmpireSelected(GG::DropDownList::iterator it)
{ SetOwnerClicked(); }

void ModeratorActionsWnd::CreateStarlaneClicked() {
    CreateStarlaneActionSelectedSignal(); 
    DetachChild(m_planet_type_drop);
    DetachChild(m_empire_drop);
    DetachChild(m_star_type_drop);
}

PlanetType ModeratorActionsWnd::SelectedPlanetType() const
{ return PT_SWAMP; }

StarType ModeratorActionsWnd::SelectedStarType() const
{ return STAR_BLUE; }

int ModeratorActionsWnd::SelectedEmpire() const
{ return ALL_EMPIRES; }

void ModeratorActionsWnd::DoLayout() {
    GG::X left = GG::X0 + PAD;
    GG::Y top = GG::Y0 + PAD;

    m_no_action_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;

    m_create_system_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;

    m_create_planet_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;

    m_delete_object_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;

    m_set_owner_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;

    m_create_starlane_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;

    left = GG::X0 + PAD;
    top += CONTROL_HEIGHT + PAD;

    // all at same location; only one shown at a time
    m_star_type_drop->SizeMove(GG::Pt(left, top),   GG::Pt(left + DROP_WIDTH, top + CONTROL_HEIGHT));
    m_planet_type_drop->SizeMove(GG::Pt(left, top), GG::Pt(left + DROP_WIDTH, top + CONTROL_HEIGHT));
    m_empire_drop->SizeMove(GG::Pt(left, top),      GG::Pt(left + DROP_WIDTH, top + CONTROL_HEIGHT));
}

void ModeratorActionsWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoLayout();
}

void ModeratorActionsWnd::Refresh() {
    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();

    // todo: get currently selected empire, if any, reselect after refresh

    m_empire_drop->Clear();
    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        const Empire* empire = it->second;
        GG::DropDownList::Row* row = new GG::DropDownList::Row();
        row->push_back(new GG::TextControl(GG::X0, GG::Y0, empire->Name(), font, empire->Color()));
        m_empire_drop->Insert(row);
    }
    if (!m_empire_drop->Empty())
        m_empire_drop->Select(m_empire_drop->begin());
}

void ModeratorActionsWnd::EnableActions(bool enable/* = true*/) {
    m_actions_enabled = enable;
}

void ModeratorActionsWnd::CloseClicked()
{ ClosingSignal(); }

