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
    m_selected_action(MAS_NoAction),
    m_no_action_button(0),
    m_create_system_button(0),
    m_star_type_drop(0),
    m_create_planet_button(0),
    m_planet_type_drop(0),
    m_planet_size_drop(0),
    m_delete_object_button(0),
    m_set_owner_button(0),
    m_empire_drop(0),
    m_add_starlane_button(0),
    m_remove_starlane_button(0)
{
    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
    ClientUI* ui = ClientUI::GetClientUI();
    GG::Flags<GG::GraphicStyle> style = GG::GRAPHIC_CENTER | GG::GRAPHIC_VCENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE;

    // button for no action
    m_no_action_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_no_action_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "nomoderatoraction.png")));
    m_no_action_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "nomoderatoraction_clicked.png"  )));
    m_no_action_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "nomoderatoraction_mouseover.png")));
    m_no_action_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_no_action_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_NONE"), UserString("MOD_NONE"))));
    AttachChild(m_no_action_button);
    GG::Connect(m_no_action_button->LeftClickedSignal,  &ModeratorActionsWnd::NoActionClicked,      this);

    // button for create system and droplist to select system type to create
    m_create_system_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_create_system_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "addstar.png")));
    m_create_system_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "addstar_clicked.png"  )));
    m_create_system_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "addstar_mouseover.png")));
    m_create_system_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_create_system_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_CREATE_SYSTEM"), UserString("MOD_CREATE_SYSTEM"))));
    AttachChild(m_create_system_button);

    GG::Connect(m_create_system_button->LeftClickedSignal,  &ModeratorActionsWnd::CreateSystemClicked,  this);
    m_star_type_drop = new CUIDropDownList(GG::X0, GG::Y0, DROP_WIDTH, CONTROL_HEIGHT, CONTROL_HEIGHT*8);
    for (StarType star_type = STAR_BLUE; star_type != NUM_STAR_TYPES; star_type = StarType(star_type + 1)) {
        boost::shared_ptr<GG::Texture> disc_texture = ui->GetModuloTexture(
            ClientUI::ArtDir() / "stars", ClientUI::StarTypeFilePrefixes()[star_type], 0);
        GG::DropDownList::Row* row = new GG::DropDownList::Row();
        row->push_back(new GG::StaticGraphic(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT,
                                             disc_texture, style, GG::Flags<GG::WndFlag>()));
        m_star_type_drop->Insert(row);
    }
    m_star_type_drop->Select(m_star_type_drop->begin());        // default select first type
    GG::Connect(m_star_type_drop->SelChangedSignal,     &ModeratorActionsWnd::StarTypeSelected,     this);

    // button for create planet and droplists to select planet type and size
    m_create_planet_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_create_planet_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "addplanet.png")));
    m_create_planet_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "addplanet_clicked.png"  )));
    m_create_planet_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "addplanet_mouseover.png")));
    m_create_planet_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_create_planet_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_CREATE_PLANET"), UserString("MOD_CREATE_PLANET"))));
    AttachChild(m_create_planet_button);
    GG::Connect(m_create_planet_button->LeftClickedSignal,  &ModeratorActionsWnd::CreatePlanetClicked,  this);

    m_planet_type_drop = new CUIDropDownList(GG::X0, GG::Y0, DROP_WIDTH, CONTROL_HEIGHT, CONTROL_HEIGHT*10);
    for (PlanetType planet_type = PT_SWAMP; planet_type != NUM_PLANET_TYPES; planet_type = PlanetType(planet_type + 1)) {
        boost::shared_ptr<GG::Texture> texture = ClientUI::PlanetIcon(planet_type);
        GG::DropDownList::Row* row = new GG::DropDownList::Row();
        row->push_back(new GG::StaticGraphic(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT,
                                             texture, style, GG::Flags<GG::WndFlag>()));
        m_planet_type_drop->Insert(row);
    }
    m_planet_type_drop->Select(m_planet_type_drop->begin());    // default select first type
    GG::Connect(m_planet_type_drop->SelChangedSignal,   &ModeratorActionsWnd::PlanetTypeSelected,   this);

    m_planet_size_drop = new CUIDropDownList(DROP_WIDTH + PAD, GG::Y0, DROP_WIDTH, CONTROL_HEIGHT, CONTROL_HEIGHT*10);
    for (PlanetSize planet_size = SZ_TINY; planet_size != NUM_PLANET_SIZES; planet_size = PlanetSize(planet_size + 1)) {
        boost::shared_ptr<GG::Texture> texture = ClientUI::PlanetSizeIcon(planet_size);
        GG::DropDownList::Row* row = new GG::DropDownList::Row();
        row->push_back(new GG::StaticGraphic(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT,
                                             texture, style, GG::Flags<GG::WndFlag>()));
        m_planet_size_drop->Insert(row);
    }
    GG::DropDownList::iterator it = m_planet_size_drop->begin();
    std::advance(it, 2);
    m_planet_size_drop->Select(it); // default select 3rd size (should be medium?)
    GG::Connect(m_planet_size_drop->SelChangedSignal,   &ModeratorActionsWnd::PlanetSizeSelected,   this);

    // button for destroying object
    m_delete_object_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_delete_object_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "delete.png")));
    m_delete_object_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "delete_clicked.png"  )));
    m_delete_object_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "delete_mouseover.png")));
    m_delete_object_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_delete_object_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_DESTROY"), UserString("MOD_DESTROY"))));
    AttachChild(m_delete_object_button);
    GG::Connect(m_delete_object_button->LeftClickedSignal,  &ModeratorActionsWnd::DeleteObjectClicked,  this);

    // button for setting owner
    m_set_owner_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_set_owner_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "setowner.png")));
    m_set_owner_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "setowner_clicked.png"  )));
    m_set_owner_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "setowner_mouseover.png")));
    m_set_owner_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_set_owner_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_SET_OWNER"), UserString("MOD_SET_OWNER"))));
    AttachChild(m_set_owner_button);

    GG::Connect(m_set_owner_button->LeftClickedSignal,      &ModeratorActionsWnd::SetOwnerClicked,      this);
    m_empire_drop = new CUIDropDownList(GG::X0, GG::Y0, DROP_WIDTH, CONTROL_HEIGHT, CONTROL_HEIGHT*10);
    // empires added later when gamestate info available
    GG::Connect(m_empire_drop->SelChangedSignal,        &ModeratorActionsWnd::EmpireSelected,       this);

    // button for creating starlane
    m_add_starlane_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_add_starlane_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "addstarlane.png")));
    m_add_starlane_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "addstarlane_clicked.png"  )));
    m_add_starlane_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "addstarlane_mouseover.png")));
    m_add_starlane_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_add_starlane_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_ADD_STARLANE"), UserString("MOD_ADD_STARLANE"))));
    AttachChild(m_add_starlane_button);
    GG::Connect(m_add_starlane_button->LeftClickedSignal,&ModeratorActionsWnd::AddStarlane,         this);

    // button for removing starlane
    m_remove_starlane_button = new GG::Button(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, "", font, GG::CLR_WHITE);
    m_remove_starlane_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "removestarlane.png")));
    m_remove_starlane_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "removestarlane_clicked.png"  )));
    m_remove_starlane_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "removestarlane_mouseover.png")));
    m_remove_starlane_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_remove_starlane_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new TextBrowseWnd(UserString("MOD_REMOVE_STARLANE"), UserString("MOD_REMOVE_STARLANE"))));
    AttachChild(m_remove_starlane_button);
    GG::Connect(m_remove_starlane_button->LeftClickedSignal,&ModeratorActionsWnd::RemoveStarlane,   this);

    DoLayout();
}

void ModeratorActionsWnd::NoActionClicked() {
    m_selected_action = MAS_NoAction;
    NoActionSelectedSignal();
    DetachChild(m_star_type_drop);
    DetachChild(m_planet_type_drop);
    DetachChild(m_planet_size_drop);
    DetachChild(m_empire_drop);
}

void ModeratorActionsWnd::CreateSystemClicked() {
    m_selected_action = MAS_CreateSystem;
    CreateSystemActionSelectedSignal(SelectedStarType());
    AttachChild(m_star_type_drop);
    DetachChild(m_planet_type_drop);
    DetachChild(m_planet_size_drop);
    DetachChild(m_empire_drop);
}

void ModeratorActionsWnd::StarTypeSelected(GG::DropDownList::iterator it)
{ CreateSystemClicked(); }

void ModeratorActionsWnd::CreatePlanetClicked() {
    m_selected_action = MAS_CreatePlanet;
    CreatePlanetActionSelectedSignal(SelectedPlanetType());
    DetachChild(m_star_type_drop);
    AttachChild(m_planet_type_drop);
    AttachChild(m_planet_size_drop);
    DetachChild(m_empire_drop);
}

void ModeratorActionsWnd::PlanetTypeSelected(GG::DropDownList::iterator it)
{ CreatePlanetClicked(); }

void ModeratorActionsWnd::PlanetSizeSelected(GG::DropDownList::iterator it)
{ CreatePlanetClicked(); }

void ModeratorActionsWnd::DeleteObjectClicked() {
    m_selected_action = MAS_Destroy;
    DeleteObjectActionSelectedSignal();
    DetachChild(m_star_type_drop);
    DetachChild(m_planet_type_drop);
    DetachChild(m_planet_size_drop);
    DetachChild(m_empire_drop);
}

void ModeratorActionsWnd::SetOwnerClicked() {
    m_selected_action = MAS_SetOwner;
    SetOwnerActionSelectedSignal(SelectedEmpire());
    DetachChild(m_star_type_drop);
    DetachChild(m_planet_type_drop);
    DetachChild(m_planet_size_drop);
    AttachChild(m_empire_drop);
}

void ModeratorActionsWnd::EmpireSelected(GG::DropDownList::iterator it)
{ SetOwnerClicked(); }

void ModeratorActionsWnd::AddStarlane() {
    m_selected_action = MAS_AddStarlane;
    AddStarlaneActionSelectedSignal();
    DetachChild(m_star_type_drop);
    DetachChild(m_planet_type_drop);
    DetachChild(m_planet_size_drop);
    DetachChild(m_empire_drop);
}

void ModeratorActionsWnd::RemoveStarlane() {
    m_selected_action = MAS_RemoveStarlane;
    AddStarlaneActionSelectedSignal();
    DetachChild(m_star_type_drop);
    DetachChild(m_planet_type_drop);
    DetachChild(m_planet_size_drop);
    DetachChild(m_empire_drop);
}

ModeratorActionSetting ModeratorActionsWnd::SelectedAction() const
{ return m_selected_action; }

PlanetType ModeratorActionsWnd::SelectedPlanetType() const
{ return PlanetTypeFromIndex(m_planet_type_drop->CurrentItemIndex()); }

PlanetSize ModeratorActionsWnd::SelectedPlanetSize() const
{ return PlanetSizeFromIndex(m_planet_size_drop->CurrentItemIndex()); }

StarType ModeratorActionsWnd::SelectedStarType() const
{ return StarTypeFromIndex(m_star_type_drop->CurrentItemIndex()); }

int ModeratorActionsWnd::SelectedEmpire() const
{ return EmpireIDFromIndex(m_empire_drop->CurrentItemIndex()); }

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

    m_add_starlane_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;

    m_remove_starlane_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;

    left = GG::X0 + PAD;
    top += CONTROL_HEIGHT + PAD;

    // all at same location; only one shown at a time
    m_star_type_drop->SizeMove(GG::Pt(left, top),   GG::Pt(left + DROP_WIDTH, top + CONTROL_HEIGHT));
    m_empire_drop->SizeMove(GG::Pt(left, top),      GG::Pt(left + DROP_WIDTH, top + CONTROL_HEIGHT));

    m_planet_type_drop->SizeMove(GG::Pt(left, top), GG::Pt(left + DROP_WIDTH, top + CONTROL_HEIGHT));
    left += DROP_WIDTH + PAD;
    m_planet_size_drop->SizeMove(GG::Pt(left, top), GG::Pt(left + DROP_WIDTH, top + CONTROL_HEIGHT));
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

StarType ModeratorActionsWnd::StarTypeFromIndex(std::size_t i) const {
    if (i == -1 || i >= NUM_STAR_TYPES)
        return STAR_BLUE;
    return StarType(i);     // assumes first enum and first index are value 0, and that items in list are in same order as enum values
}

PlanetType ModeratorActionsWnd::PlanetTypeFromIndex(std::size_t i) const {
    if (i == -1 || i >= NUM_PLANET_TYPES)
        return PT_SWAMP;
    return PlanetType(i);   // assumes first enum and first index are value 0, and that items in list are in same order as enum values
}

PlanetSize ModeratorActionsWnd::PlanetSizeFromIndex(std::size_t i) const {
    if (i == -1 || i + 1 >= NUM_PLANET_SIZES)
        return SZ_MEDIUM;
    return PlanetSize(i + 1);// enum index 0 is NO_WORLD, but don't put that into the list, so have to add 1 to all the list indices
}

int ModeratorActionsWnd::EmpireIDFromIndex(std::size_t i) const {
    if (i == -1)
        return ALL_EMPIRES;
    if (i > static_cast<unsigned int>(Empires().NumEmpires()))
        return ALL_EMPIRES;
    EmpireManager::const_iterator it = Empires().begin();
    std::advance(it, i);
    return it->first;
}
