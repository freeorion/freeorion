#include "FleetButton.h"

#include "../util/AppInterface.h"
#include "../universe/Fleet.h"
#include "FleetWindow.h"
#include "GGDrawUtil.h"
#include "../client/human/HumanClientApp.h"
#include "MapWnd.h"
#include "../util/OptionsDB.h"
#include "../universe/System.h"

#include <algorithm>

////////////////////////////////////////////////
// SystemIcon::FleetButton
////////////////////////////////////////////////

// static(s)
std::map<Fleet*, FleetWnd*> FleetButton::s_open_fleets;

FleetButton::FleetButton(GG::Clr color, const std::vector<int>& fleet_IDs, double zoom) : 
    Button(0, 0, 1, 1, "", "", 0, color),
    m_orientation(),
    m_compliment(0)
{
    Universe& universe = GetUniverse();
    std::set<Fleet*> fleets_currently_shown;
    std::set<Fleet*> fleets_currently_not_shown;

    for (unsigned int i = 0; i < fleet_IDs.size(); ++i) {
        Fleet* fleet = universe.Object<Fleet>(fleet_IDs[i]);
        m_fleets.push_back(fleet);
        if (s_open_fleets.find(fleet) != s_open_fleets.end())
            fleets_currently_shown.insert(fleet);
        else
            fleets_currently_not_shown.insert(fleet);
    }
    Fleet* fleet = m_fleets.back();
    double x = fleet->X();
    double y = fleet->Y();
    GG::Pt button_ul(static_cast<int>((x - ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE / 2) * zoom), 
                     static_cast<int>((y - ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE / 2) * zoom));
    SizeMove(button_ul.x, button_ul.y, 
             static_cast<int>(button_ul.x + ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE * zoom + 0.5), 
             static_cast<int>(button_ul.y + ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE * zoom + 0.5));

    m_orientation = GetUniverse().Object<System>(fleet->NextSystemID())->X() - fleet->X() < 0 ? SHAPE_LEFT : SHAPE_RIGHT;

    GG::Connect(ClickedSignal(), &FleetButton::Clicked, this);

    if (!fleets_currently_shown.empty()) {
        FleetWnd* fleet_wnd = s_open_fleets[*fleets_currently_shown.begin()];
        for (std::set<Fleet*>::iterator it = fleets_currently_not_shown.begin(); it != fleets_currently_not_shown.end(); ++it) {
            fleet_wnd->AddFleet(*it);
        }
    }
}

FleetButton::FleetButton(int x, int y, int w, int h, GG::Clr color, const std::vector<int>& fleet_IDs, ShapeOrientation orientation) : 
    Button(x, y, w, h, "", "", 0, color),
    m_orientation(orientation),
    m_compliment(0)
{
    Universe& universe = GetUniverse();
    std::set<Fleet*> fleets_currently_shown;
    std::set<Fleet*> fleets_currently_not_shown;

    for (unsigned int i = 0; i < fleet_IDs.size(); ++i) {
        Fleet* fleet = universe.Object<Fleet>(fleet_IDs[i]);
        m_fleets.push_back(fleet);
        if (s_open_fleets.find(fleet) != s_open_fleets.end())
            fleets_currently_shown.insert(fleet);
        else
            fleets_currently_not_shown.insert(fleet);
    }
    GG::Connect(ClickedSignal(), &FleetButton::Clicked, this);

    if (!fleets_currently_shown.empty()) {
        FleetWnd* fleet_wnd = s_open_fleets[*fleets_currently_shown.begin()];
        for (std::set<Fleet*>::iterator it = fleets_currently_not_shown.begin(); it != fleets_currently_not_shown.end(); ++it) {
            fleet_wnd->AddFleet(*it);
        }
    }
}

FleetButton::FleetButton(const GG::XMLElement& elem) : 
    Button(elem.Child("GG::Button")),
    m_compliment(0)
{
    if (elem.Tag() != "FleetButton")
        throw std::invalid_argument("Attempted to construct a FleetButton from an XMLElement that had a tag other than \"FleetButton\"");

    const GG::XMLElement* curr_elem = &elem.Child("m_orientation");
    m_orientation = ShapeOrientation(boost::lexical_cast<int>(curr_elem->Attribute("value")));

    curr_elem = &elem.Child("m_fleets");
    Universe& universe = GetUniverse();
    for (int i = 0; i < curr_elem->NumChildren(); ++i) {
        m_fleets.push_back(universe.Object<Fleet>(boost::lexical_cast<int>(curr_elem->Child(i).Attribute("value"))));
    }

    GG::Connect(ClickedSignal(), &FleetButton::Clicked, this);
}

bool FleetButton::InWindow(const GG::Pt& pt) const
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    return InFleetMarker(pt, ul.x, ul.y, lr.x, lr.y, m_orientation);
}

GG::XMLElement FleetButton::XMLEncode() const
{
    GG::XMLElement retval("FleetButton");
    retval.AppendChild(Button::XMLEncode());

    GG::XMLElement temp;

    temp = GG::XMLElement("m_orientation");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_orientation));
    retval.AppendChild(temp);

    temp = GG::XMLElement("m_fleets");
    for (unsigned int i = 0; i < m_fleets.size(); ++i) {
        GG::XMLElement temp2("ID");
        temp2.SetAttribute("value", boost::lexical_cast<std::string>(m_fleets[i]->ID()));
        temp.AppendChild(temp2);
    }
    retval.AppendChild(temp);

    return retval;
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
    std::vector<Fleet*> fleets(m_fleets);
    if (m_compliment)
        fleets.insert(fleets.end(), m_compliment->m_fleets.begin(), m_compliment->m_fleets.end());

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

    GG::Pt ul = UpperLeft();
    bool read_only = *fleets[0]->Owners().begin() != HumanClientApp::GetApp()->EmpireID() || 
        (fleets[0]->FinalDestinationID() != UniverseObject::INVALID_OBJECT_ID && 
         fleets[0]->SystemID() == UniverseObject::INVALID_OBJECT_ID);
    FleetWnd* fleet_wnd = new FleetWnd(ul.x + 50, ul.y + 50, fleets, read_only);

    if (multiple_fleet_windows) {
        // for multiple windows, place them at the screen location of the fleet button
        if (GG::App::GetApp()->AppWidth() < fleet_wnd->LowerRight().x)
            fleet_wnd->OffsetMove(fleet_wnd->LowerRight().x - GG::App::GetApp()->AppWidth() - 5, 0);

        if (GG::App::GetApp()->AppHeight() < fleet_wnd->LowerRight().y)
            fleet_wnd->OffsetMove(0, fleet_wnd->LowerRight().y - GG::App::GetApp()->AppHeight() - 5);
    } else {
        // for one-fleet-at-a-time, place them in the lower-left corner of the screen, or wherever the user has moved them
        fleet_wnd->MoveTo(FleetWnd::LastPosition() == GG::Pt() ? 
                          GG::Pt(5, GG::App::GetApp()->AppHeight() - fleet_wnd->Height() - 5) : 
                          FleetWnd::LastPosition());
    }

    if (MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd())
        GG::Connect(map_wnd->SystemRightClickedSignal(), &FleetWnd::SystemClicked, fleet_wnd);

    for (unsigned int i = 0; i < fleets.size(); ++i) {
        s_open_fleets[fleets[i]] = fleet_wnd;
    }

    GG::Connect(fleet_wnd->ShowingFleetSignal(), &FleetButton::FleetIsBeingExamined);
    GG::Connect(fleet_wnd->NotShowingFleetSignal(), &FleetButton::FleetIsNotBeingExamined);
    GG::App::GetApp()->Register(fleet_wnd);
}

void FleetButton::FleetIsBeingExamined(Fleet* fleet, FleetWnd* fleet_wnd)
{
    s_open_fleets[fleet] = fleet_wnd;
}

void FleetButton::FleetIsNotBeingExamined(Fleet* fleet)
{
    s_open_fleets.erase(fleet);
}
