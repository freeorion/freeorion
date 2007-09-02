#include "FleetButton.h"

#include "../util/AppInterface.h"
#include "../universe/Fleet.h"
#include "FleetWnd.h"
#include "../client/human/HumanClientApp.h"
#include "MapWnd.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../universe/System.h"

#include <GG/DrawUtil.h>

#include <algorithm>


namespace {
    bool PlaySounds() {return GetOptionsDB().Get<bool>("UI.sound.enabled");}
    void PlayFleetButtonOpenSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.fleet-button-click"));}
    void PlayFleetButtonRolloverSound() {if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.fleet-button-rollover"));}
}


////////////////////////////////////////////////
// SystemIcon::FleetButton
////////////////////////////////////////////////

FleetButton::FleetButton(GG::Clr color, const std::vector<int>& fleet_IDs) : 
    Button(0, 0, 1, 1, "", boost::shared_ptr<GG::Font>(), color),
    m_orientation(SHAPE_RIGHT)
{
    Universe& universe = GetUniverse();
    for (unsigned int i = 0; i < fleet_IDs.size(); ++i) {
        Fleet* fleet = universe.Object<Fleet>(fleet_IDs[i]);
        m_fleets.push_back(fleet);
    }
}

FleetButton::FleetButton(int x, int y, int w, int h, GG::Clr color, const std::vector<int>& fleet_IDs) :
    Button(x, y, w, h, "", boost::shared_ptr<GG::Font>(), color),
    m_orientation(SHAPE_RIGHT)
{
    Universe& universe = GetUniverse();
    for (unsigned int i = 0; i < fleet_IDs.size(); ++i) {
        Fleet* fleet = universe.Object<Fleet>(fleet_IDs[i]);
        m_fleets.push_back(fleet);
    }
}

bool FleetButton::InWindow(const GG::Pt& pt) const
{
    GG::Pt ul = UpperLeft(), lr = LowerRight(), size = lr - ul;
    ul -= GG::Pt(size.x / 2, size.y / 2);
    lr += GG::Pt(size.x / 2, size.y / 2);
    return InFleetMarker(pt, ul.x, ul.y, lr.x, lr.y, m_orientation);
}

void FleetButton::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (!Disabled() && (!map_wnd || !map_wnd->InProductionViewMode())) {
        if (State() != BN_ROLLOVER && PlaySounds())
            PlayFleetButtonRolloverSound();
        SetState(BN_ROLLOVER);
    }
}

void FleetButton::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (!Disabled() && (!map_wnd || !map_wnd->InProductionViewMode()))
        PlayFleetButtonOpenSound();
    GG::Button::LClick(pt, mod_keys);
}

void FleetButton::RenderUnpressed()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    glDisable(GL_TEXTURE_2D);
    FleetMarker(ul.x, ul.y, lr.x, lr.y, m_orientation, color_to_use);
    glEnable(GL_TEXTURE_2D);
}

void FleetButton::RenderPressed()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    glDisable(GL_TEXTURE_2D);
    FleetMarker(ul.x, ul.y, lr.x, lr.y, m_orientation, color_to_use);
    glEnable(GL_TEXTURE_2D);
}

void FleetButton::RenderRollover()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    glDisable(GL_TEXTURE_2D);
    FleetMarker(ul.x-Width()/4, ul.y-Height()/4, lr.x+Width()/4, lr.y+Height()/4, m_orientation, color_to_use);
    glEnable(GL_TEXTURE_2D);
}
