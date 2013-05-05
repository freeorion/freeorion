#include "ModeratorActionsWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../util/MultiplayerCommon.h"
#include <GG/Button.h>

namespace {
    const GG::X CONTROL_WIDTH(64);
    const GG::Y CONTROL_HEIGHT(32);
    const int   PAD(3);
}

////////////////////////////////////////////////
// ModeratorActionsWnd
////////////////////////////////////////////////
ModeratorActionsWnd::ModeratorActionsWnd(GG::X w, GG::Y h) :
    CUIWnd(UserString("MODERATOR"), GG::X1, GG::Y1, w - 1, h - 1, GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE),
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
    // button for no action
    m_no_action_button = new CUIButton(GG::X0, GG::Y0, CONTROL_WIDTH, UserString("MOD_NONE"));
    AttachChild(m_no_action_button);
    GG::Connect(m_no_action_button->ClickedSignal,      &ModeratorActionsWnd::NoActionClicked,      this);

    // button for create system and droplist to select system type to create
    m_create_system_button = new CUIButton(GG::X0, GG::Y0, CONTROL_WIDTH, UserString("MOD_CREATE_SYSTEM"));
    AttachChild(m_create_system_button);
    GG::Connect(m_create_system_button->ClickedSignal,  &ModeratorActionsWnd::CreateSystemClicked,  this);
    m_star_type_drop = new CUIDropDownList(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, CONTROL_HEIGHT*10);
    AttachChild(m_star_type_drop);
    GG::Connect(m_star_type_drop->SelChangedSignal,     &ModeratorActionsWnd::StarTypeSelected,     this);

    // button for create planet and droplist to select planet type to create
    m_create_planet_button = new CUIButton(GG::X0, GG::Y0, CONTROL_WIDTH, UserString("MOD_CREATE_PLANET"));
    AttachChild(m_create_planet_button);
    GG::Connect(m_create_planet_button->ClickedSignal,  &ModeratorActionsWnd::CreatePlanetClicked,  this);
    m_planet_type_drop = new CUIDropDownList(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, CONTROL_HEIGHT*10);
    AttachChild(m_planet_type_drop);
    GG::Connect(m_planet_type_drop->SelChangedSignal,   &ModeratorActionsWnd::PlanetTypeSelected,   this);

    // button for destroying object
    m_delete_object_button = new CUIButton(GG::X0, GG::Y0, CONTROL_WIDTH, UserString("MOD_DESTROY"));
    AttachChild(m_delete_object_button);
    GG::Connect(m_delete_object_button->ClickedSignal,  &ModeratorActionsWnd::DeleteObjectClicked,  this);

    // button for setting owner
    m_set_owner_button = new CUIButton(GG::X0, GG::Y0, CONTROL_WIDTH, UserString("MOD_SET_OWNER"));
    AttachChild(m_set_owner_button);
    GG::Connect(m_set_owner_button->ClickedSignal,      &ModeratorActionsWnd::SetOwnerClicked,      this);
    m_empire_drop = new CUIDropDownList(GG::X0, GG::Y0, CONTROL_WIDTH, CONTROL_HEIGHT, CONTROL_HEIGHT*10);
    AttachChild(m_empire_drop);
    GG::Connect(m_empire_drop->SelChangedSignal,        &ModeratorActionsWnd::EmpireSelected,       this);

    // button for creating starlane
    m_create_starlane_button = new CUIButton(GG::X0, GG::Y0, CONTROL_WIDTH, UserString("MOD_STARLANE"));
    AttachChild(m_create_starlane_button);
    GG::Connect(m_create_starlane_button->ClickedSignal,&ModeratorActionsWnd::CreateStarlaneClicked,this);

    DoLayout();
}

void ModeratorActionsWnd::NoActionClicked() {
    NoActionSelectedSignal();
    MarkButtonsUnset();
    m_no_action_button->MarkSelectedGray();
}

void ModeratorActionsWnd::CreateSystemClicked() {
    CreateSystemActionSelectedSignal(SelectedStarType());
    MarkButtonsUnset();
    m_create_system_button->MarkSelectedGray();
}

void ModeratorActionsWnd::StarTypeSelected(GG::DropDownList::iterator it)
{ CreateSystemClicked(); }

void ModeratorActionsWnd::CreatePlanetClicked() {
    CreatePlanetActionSelectedSignal(SelectedPlanetType());
    MarkButtonsUnset();
    m_create_planet_button->MarkSelectedGray();
}

void ModeratorActionsWnd::PlanetTypeSelected(GG::DropDownList::iterator it)
{ CreatePlanetClicked(); }

void ModeratorActionsWnd::DeleteObjectClicked() {
    DeleteObjectActionSelectedSignal();
    MarkButtonsUnset();
    m_delete_object_button->MarkSelectedGray();
}

void ModeratorActionsWnd::SetOwnerClicked() {
    SetOwnerActionSelectedSignal(SelectedEmpire());
    MarkButtonsUnset();
    m_set_owner_button->MarkSelectedGray();
}

void ModeratorActionsWnd::EmpireSelected(GG::DropDownList::iterator it)
{ SetOwnerClicked(); }

void ModeratorActionsWnd::CreateStarlaneClicked() {
    CreateStarlaneActionSelectedSignal();
    MarkButtonsUnset();
    m_create_starlane_button->MarkSelectedGray();
}

PlanetType ModeratorActionsWnd::SelectedPlanetType() const
{ return PT_SWAMP; }

StarType ModeratorActionsWnd::SelectedStarType() const
{ return STAR_BLUE; }

int ModeratorActionsWnd::SelectedEmpire() const
{ return ALL_EMPIRES; }

void ModeratorActionsWnd::DoLayout() {
    GG::X left = GG::X0;
    GG::Y top = GG::Y0;

    m_no_action_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;

    m_create_system_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    top += CONTROL_HEIGHT + PAD;
    m_star_type_drop->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;
    top = GG::Y0;

    m_create_planet_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    top += CONTROL_HEIGHT + PAD;
    m_planet_type_drop->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;
    top = GG::Y0;

    m_delete_object_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;

    m_set_owner_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    top += CONTROL_HEIGHT + PAD;
    m_empire_drop->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;
    top = GG::Y0;

    m_create_starlane_button->SizeMove(GG::Pt(left, top), GG::Pt(left + CONTROL_WIDTH, top + CONTROL_HEIGHT));
    left += CONTROL_WIDTH + PAD;
}

void ModeratorActionsWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoLayout();
}

void ModeratorActionsWnd::MarkButtonsUnset() {
    m_no_action_button->MarkNotSelected();
    m_create_system_button->MarkNotSelected();
    m_create_planet_button->MarkNotSelected();
    m_delete_object_button->MarkNotSelected();
    m_set_owner_button->MarkNotSelected();
    m_create_starlane_button->MarkNotSelected();
}

void ModeratorActionsWnd::Refresh() {
    m_no_action_button;
    m_create_system_button;
    m_star_type_drop;
    m_create_planet_button;
    m_planet_type_drop;
    m_delete_object_button;
    m_set_owner_button;
    m_empire_drop;
    m_create_starlane_button;
}

void ModeratorActionsWnd::CloseClicked()
{ ClosingSignal(); }

