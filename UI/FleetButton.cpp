#include "FleetButton.h"

#include "../util/AppInterface.h"
#include "FleetWindow.h"
#include "GGDrawUtil.h"
#include "../client/human/HumanClientApp.h"
#include "MapWnd.h"

////////////////////////////////////////////////
// SystemIcon::FleetButton
////////////////////////////////////////////////

// static(s)
std::set<int> FleetButton::s_open_fleet_ids;

FleetButton::FleetButton(int x, int y, int w, int h, GG::Clr color, const std::vector<int>& fleet_ids, ShapeOrientation orientation) : 
    Button(x, y, w, h, "", "", 0, color),
    m_fleet_ids(fleet_ids),
    m_orientation(orientation)
{
    GG::Connect(ClickedSignal(), &FleetButton::Clicked, this);
}

FleetButton::FleetButton(const GG::XMLElement& elem) : 
    Button(elem.Child("GG::Button"))
{
    if (elem.Tag() != "FleetButton")
        throw std::invalid_argument("Attempted to construct a FleetButton from an XMLElement that had a tag other than \"FleetButton\"");

    const GG::XMLElement* curr_elem = &elem.Child("m_orientation");
    m_orientation = ShapeOrientation(boost::lexical_cast<int>(curr_elem->Attribute("value")));

    curr_elem = &elem.Child("m_fleet_ids");
    for (int i = 0; i < curr_elem->NumChildren(); ++i) {
        m_fleet_ids.push_back(boost::lexical_cast<int>(curr_elem->Child(i).Attribute("value")));
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

    temp = GG::XMLElement("m_fleet_ids");
    for (unsigned int i = 0; i < m_fleet_ids.size(); ++i) {
        GG::XMLElement temp2("ID");
        temp2.SetAttribute("value", boost::lexical_cast<std::string>(m_fleet_ids[i]));
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
    FleetMarker(ul.x, ul.y, lr.x, lr.y, m_orientation, color_to_use);
    glEnable(GL_TEXTURE_2D);
}

void FleetButton::Clicked()
{
    // only open a fleet window if there is not one open already for these fleets
    for (int i = 0; i < m_fleet_ids.size(); ++i) {
        if (s_open_fleet_ids.find(m_fleet_ids[i]) != s_open_fleet_ids.end())
            return;
    }
    
    GG::Pt ul = UpperLeft();
    bool read_only = *GetUniverse().Object(m_fleet_ids[0])->Owners().begin() != HumanClientApp::GetApp()->PlayerID();
    FleetWnd* fleet_wnd = new FleetWnd(ul.x + 50, ul.y + 50, m_fleet_ids, read_only);
    Wnd* root_parent = RootParent();
    
    if (MapWnd* map_wnd = dynamic_cast<MapWnd*>(root_parent))
        GG::Connect(map_wnd->SelectedSystemSignal(), &FleetWnd::SystemClicked, fleet_wnd);
    
    if (GG::App::GetApp()->AppWidth() < fleet_wnd->LowerRight().x)
        fleet_wnd->OffsetMove(fleet_wnd->LowerRight().x - GG::App::GetApp()->AppWidth() - 5, 0);
    
    if (GG::App::GetApp()->AppHeight() < fleet_wnd->LowerRight().y)
        fleet_wnd->OffsetMove(0, fleet_wnd->LowerRight().y - GG::App::GetApp()->AppHeight() - 5);

    for (int i = 0; i < m_fleet_ids.size(); ++i) {
        s_open_fleet_ids.insert(m_fleet_ids[i]);
    }

    GG::Connect(fleet_wnd->ShowingFleetSignal(), &FleetButton::FleetIsBeingExamined);
    GG::Connect(fleet_wnd->NotShowingFleetSignal(), &FleetButton::FleetIsNotBeingExamined);
    GG::App::GetApp()->Register(fleet_wnd);
}

void FleetButton::FleetIsBeingExamined(int id)
{
    s_open_fleet_ids.insert(id);
}

void FleetButton::FleetIsNotBeingExamined(int id)
{
    s_open_fleet_ids.erase(id);
}
