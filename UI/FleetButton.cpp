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
    bool PlaySounds()
    {
        return GetOptionsDB().Get<bool>("UI.sound.enabled");
    }

    bool temp_header_bool = RecordHeaderFile(FleetButtonRevision());
    bool temp_source_bool = RecordSourceFile("$Id$");
}


////////////////////////////////////////////////
// SystemIcon::FleetButton
////////////////////////////////////////////////

// static(s)
std::map<Fleet*, FleetWnd*> FleetButton::s_open_fleets;

FleetButton::FleetButton(GG::Clr color, const std::vector<int>& fleet_IDs, double zoom) : 
    Button(0, 0, 1, 1, "", boost::shared_ptr<GG::Font>(), color),
    m_orientation(),
    m_selected_fleet(0),
    m_compliment(0)
{
    Universe& universe = GetUniverse();
    for (unsigned int i = 0; i < fleet_IDs.size(); ++i) {
        Fleet* fleet = universe.Object<Fleet>(fleet_IDs[i]);
        m_fleets.push_back(fleet);
    }
    Fleet* fleet = m_fleets.back();
    double x = fleet->X();
    double y = fleet->Y();
    GG::Pt button_ul(static_cast<int>((x - ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE / 2) * zoom), 
                     static_cast<int>((y - ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE / 2) * zoom));
    SizeMove(GG::Pt(button_ul.x, button_ul.y),
             GG::Pt(static_cast<int>(button_ul.x + ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE * zoom + 0.5), 
                    static_cast<int>(button_ul.y + ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE * zoom + 0.5)));

    m_orientation = GetUniverse().Object<System>(fleet->NextSystemID())->X() - fleet->X() < 0 ? SHAPE_LEFT : SHAPE_RIGHT;

    GG::Connect(ClickedSignal, &FleetButton::Clicked, this);
    GG::Connect(GetUniverse().UniverseObjectDeleteSignal, &FleetButton::FleetDeleted, this);
}

FleetButton::FleetButton(int x, int y, int w, int h, GG::Clr color, const std::vector<int>& fleet_IDs, ShapeOrientation orientation) : 
    Button(x, y, w, h, "", boost::shared_ptr<GG::Font>(), color),
    m_orientation(orientation),
    m_selected_fleet(0),
    m_compliment(0)
{
    Universe& universe = GetUniverse();
    for (unsigned int i = 0; i < fleet_IDs.size(); ++i) {
        Fleet* fleet = universe.Object<Fleet>(fleet_IDs[i]);
        m_fleets.push_back(fleet);
    }
    GG::Connect(ClickedSignal, &FleetButton::Clicked, this);
    GG::Connect(GetUniverse().UniverseObjectDeleteSignal, &FleetButton::FleetDeleted, this);
}

bool FleetButton::InWindow(const GG::Pt& pt) const
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    return InFleetMarker(pt, ul.x, ul.y, lr.x, lr.y, m_orientation);
}

void FleetButton::MouseHere(const GG::Pt& pt, Uint32 keys)
{
    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (!Disabled() && (!map_wnd || !map_wnd->InProductionViewMode())) {
        if (State() != BN_ROLLOVER && PlaySounds())
            HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.fleet-button-rollover"));
        SetState(BN_ROLLOVER);
    }
}

void FleetButton::SelectFleet(Fleet* fleet)
{
    m_selected_fleet = fleet;
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

void FleetButton::Clicked()
{
    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (map_wnd && map_wnd->InProductionViewMode())
        return;

    int selected_fleet = 0;
    std::vector<Fleet*> fleets(m_fleets);
    if (m_compliment)
        fleets.insert(m_orientation == SHAPE_RIGHT ? fleets.end() : fleets.begin(), m_compliment->m_fleets.begin(), m_compliment->m_fleets.end());
    if (m_selected_fleet) {
        selected_fleet = std::distance(fleets.begin(), std::find(fleets.begin(), fleets.end(), m_selected_fleet));
        m_selected_fleet = 0;
    } else if (m_compliment) {
        selected_fleet = m_orientation == SHAPE_RIGHT ? 0 : m_compliment->m_fleets.size();
    }

    bool multiple_fleet_windows = GetOptionsDB().Get<bool>("UI.multiple-fleet-windows");

    if (multiple_fleet_windows) {
        // only open a fleet window if there is not one open already for these fleets
        for (unsigned int i = 0; i < fleets.size(); ++i) {
            if (s_open_fleets.find(fleets[i]) != s_open_fleets.end())
                return;
        }
    } else {
        // close the open fleet window, if there is one
        std::map<Fleet*, FleetWnd*>::iterator it = s_open_fleets.begin();
        if (it != s_open_fleets.end()) {
            it->second->Close();
        }
    }

    if (PlaySounds())
        HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.fleet-button-click"));

    GG::Pt ul = UpperLeft();
    bool read_only = *fleets[0]->Owners().begin() != HumanClientApp::GetApp()->EmpireID() || 
        (fleets[0]->FinalDestinationID() != UniverseObject::INVALID_OBJECT_ID && 
         fleets[0]->SystemID() == UniverseObject::INVALID_OBJECT_ID);
    FleetWnd* fleet_wnd = new FleetWnd(ul.x + 50, ul.y + 50, fleets, selected_fleet, read_only);

    if (multiple_fleet_windows) {
        // for multiple windows, place them at the screen location of the fleet button
        if (GG::GUI::GetGUI()->AppWidth() < fleet_wnd->LowerRight().x)
            fleet_wnd->OffsetMove(GG::Pt(fleet_wnd->LowerRight().x - GG::GUI::GetGUI()->AppWidth() - 5, 0));

        if (GG::GUI::GetGUI()->AppHeight() < fleet_wnd->LowerRight().y)
            fleet_wnd->OffsetMove(GG::Pt(0, fleet_wnd->LowerRight().y - GG::GUI::GetGUI()->AppHeight() - 5));
    } else {
        // for one-fleet-at-a-time, place them in the lower-left corner of the screen, or wherever the user has moved them
        fleet_wnd->MoveTo(FleetWnd::LastPosition() == GG::Pt() ? 
                          GG::Pt(5, GG::GUI::GetGUI()->AppHeight() - fleet_wnd->Height() - 5) : 
                          FleetWnd::LastPosition());
    }

    if (map_wnd) {
        GG::Connect(map_wnd->SystemRightClickedSignal, &FleetWnd::SystemClicked, fleet_wnd);
        GG::Connect(map_wnd->SystemBrowsedSignal, &FleetWnd::SystemBrowsed, fleet_wnd);
    }

    for (unsigned int i = 0; i < fleets.size(); ++i) {
        s_open_fleets[fleets[i]] = fleet_wnd;
    }

    GG::Connect(fleet_wnd->ShowingFleetSignal, &FleetButton::FleetIsBeingExamined);
    GG::Connect(fleet_wnd->NotShowingFleetSignal, &FleetButton::FleetIsNotBeingExamined);
    GG::GUI::GetGUI()->Register(fleet_wnd);
}

void FleetButton::FleetDeleted(const UniverseObject* obj)
{
    if (const Fleet* fleet = dynamic_cast<const Fleet*>(obj)) {
        std::vector<Fleet*>::iterator it = std::find(m_fleets.begin(), m_fleets.end(), const_cast<Fleet*>(fleet));
        if (it != m_fleets.end())
            m_fleets.erase(it);
        s_open_fleets.erase(const_cast<Fleet*>(fleet));
    }
}

void FleetButton::FleetIsBeingExamined(Fleet* fleet, FleetWnd* fleet_wnd)
{
    s_open_fleets[fleet] = fleet_wnd;
}

void FleetButton::FleetIsNotBeingExamined(Fleet* fleet)
{
    s_open_fleets.erase(fleet);
}
