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

FleetButton::FleetButton(GG::Clr color, const std::vector<int>& fleet_IDs, double zoom) : 
    Button(0, 0, 1, 1, "", boost::shared_ptr<GG::Font>(), color),
    m_orientation()
{
    Universe& universe = GetUniverse();
    for (unsigned int i = 0; i < fleet_IDs.size(); ++i) {
        Fleet* fleet = universe.Object<Fleet>(fleet_IDs[i]);
        m_fleets.push_back(fleet);
    }
    Fleet* fleet = m_fleets.back();
    double x = fleet->X();
    double y = fleet->Y();
    GG::Pt button_ul(static_cast<int>((x - ClientUI::SystemIconSize() * ClientUI::FleetButtonSize() / 2) * zoom), 
                     static_cast<int>((y - ClientUI::SystemIconSize() * ClientUI::FleetButtonSize() / 2) * zoom));
    SizeMove(GG::Pt(button_ul.x, button_ul.y),
             GG::Pt(static_cast<int>(button_ul.x + ClientUI::SystemIconSize() * ClientUI::FleetButtonSize() * zoom + 0.5), 
                    static_cast<int>(button_ul.y + ClientUI::SystemIconSize() * ClientUI::FleetButtonSize() * zoom + 0.5)));

    m_orientation = GetUniverse().Object<System>(fleet->NextSystemID())->X() - fleet->X() < 0 ? SHAPE_LEFT : SHAPE_RIGHT;
}

FleetButton::FleetButton(int x, int y, int w, int h, GG::Clr color, const std::vector<int>& fleet_IDs, ShapeOrientation orientation) : 
    Button(x, y, w, h, "", boost::shared_ptr<GG::Font>(), color),
    m_orientation(orientation)
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

void FleetButton::MouseHere(const GG::Pt& pt, Uint32 keys)
{
    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (!Disabled() && (!map_wnd || !map_wnd->InProductionViewMode())) {
        if (State() != BN_ROLLOVER && PlaySounds())
            PlayFleetButtonRolloverSound();
        SetState(BN_ROLLOVER);
    }
}

void FleetButton::LClick(const GG::Pt& pt, Uint32 keys)
{
    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (!Disabled() && (!map_wnd || !map_wnd->InProductionViewMode()))
        PlayFleetButtonOpenSound();
    GG::Button::LClick(pt, keys);
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
