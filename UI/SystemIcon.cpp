//SystemIcon.cpp

#include "SystemIcon.h"

#include "ClientUI.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "GGTextControl.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/Predicates.h"
#include "../universe/System.h"

#include <boost/lexical_cast.hpp>

namespace {
const double FLEET_BUTTON_HEIGHT = 0.2; // the height of a fleet button as a portion of the SystemIcon's size
}

// static(s)
const int SystemIcon::ICON_WIDTH = 32;
const int SystemIcon::ICON_HEIGHT = 32;

////////////////////////////////////////////////
// SystemIcon::FleetButton
////////////////////////////////////////////////
SystemIcon::FleetButton::FleetButton(int x, int y, int w, int h, GG::Clr color, ShapeOrientation orientation) : 
    Button(x, y, w, h, "", "", 0, color),
    m_orientation(orientation)
{
}

SystemIcon::FleetButton::FleetButton(const GG::XMLElement& elem) : 
    Button(elem.Child("GG::Button"))
{
    if (elem.Tag() != "SystemIcon::FleetButton")
        throw std::invalid_argument("Attempted to construct a GG::StateButton from an XMLElement that had a tag other than \"GG::StateButton\"");

    const GG::XMLElement* curr_elem = &elem.Child("m_orientation");
    m_orientation = ShapeOrientation(boost::lexical_cast<int>(curr_elem->Attribute("value")));
}

bool SystemIcon::FleetButton::InWindow(const GG::Pt& pt) const
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    return InFleetMarker(pt, ul.x, ul.y, lr.x, lr.y, m_orientation);
}

GG::XMLElement SystemIcon::FleetButton::XMLEncode() const
{
    GG::XMLElement retval("GG::StateButton");
    retval.AppendChild(Button::XMLEncode());

    GG::XMLElement temp;

    temp = GG::XMLElement("m_orientation");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_orientation));
    retval.AppendChild(temp);

    return retval;
}

void SystemIcon::FleetButton::RenderUnpressed()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    glDisable(GL_TEXTURE_2D);
    FleetMarker(ul.x, ul.y, lr.x, lr.y, m_orientation, color_to_use);
    glEnable(GL_TEXTURE_2D);
}

void SystemIcon::FleetButton::RenderPressed()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    glDisable(GL_TEXTURE_2D);
    FleetMarker(ul.x, ul.y, lr.x, lr.y, m_orientation, color_to_use);
    glEnable(GL_TEXTURE_2D);
}

void SystemIcon::FleetButton::RenderRollover()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    glDisable(GL_TEXTURE_2D);
    FleetMarker(ul.x, ul.y, lr.x, lr.y, m_orientation, color_to_use);
    glEnable(GL_TEXTURE_2D);
}


////////////////////////////////////////////////
// SystemIcon
////////////////////////////////////////////////
SystemIcon::SystemIcon(int id, double zoom) :
    GG::Control(0, 0, 1, 1, GG::Wnd::CLICKABLE),
    m_system_ID(id),
    m_static_graphic(0),
    m_name(0)
{
    const System* sys = dynamic_cast<const System*>(ClientApp::GetUniverse().Object(id));

    Connect(sys->StateChangedSignal(), &SystemIcon::Refresh, this);

    SetText(sys->Name());

    //resize to the proper size
    SizeMove((int)(sys->X() * zoom - ICON_WIDTH / 2),
             (int)(sys->Y() * zoom - ICON_HEIGHT / 2),
             (int)(sys->X() * zoom - ICON_WIDTH / 2 + ICON_WIDTH),
             (int)(sys->Y() * zoom - ICON_HEIGHT / 2 + ICON_HEIGHT)); //to position of this universe object

    //load the proper graphic for the color of the star
    boost::shared_ptr<GG::Texture> graphic;
    switch (sys->Star()) {
    case System::YELLOW:
        graphic = HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "/stars/yellow1.png");
        break;
    case System::RED_GIANT:
        graphic = HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "/stars/red3.png");
        break;
    case System::RED_DWARF:
        graphic = HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "/stars/red4.png");
        break;
    default:
        //just load a blue star for now
        graphic = HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "/stars/blue2.png");
        break;
    }//end switch

    //setup static graphic
    m_static_graphic = new GG::StaticGraphic(0, 0, ICON_WIDTH, ICON_HEIGHT, graphic, GG::SG_FITGRAPHIC);
    AttachChild(m_static_graphic);

    //set up the name text control
    m_name = new GG::TextControl(0, 0, sys->Name(), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
    PositionSystemName();
    AttachChild(m_name);

    CreateFleetButtons(sys);
}


SystemIcon::~SystemIcon()
{
}

void SystemIcon::SizeMove(int x1, int y1, int x2, int y2)
{
    Wnd::SizeMove(x1, y1, x2, y2);
    if (m_static_graphic)
        m_static_graphic->SizeMove(0, 0, x2 - x1, y2 - y1);
    PositionSystemName();

    const int FLEET_BUTTON_SIZE = static_cast<int>(Height() * FLEET_BUTTON_HEIGHT);
    for (std::map<int, FleetButton*>::iterator it = m_stationary_fleet_markers.begin(); it != m_stationary_fleet_markers.end(); ++it) {
        int order = (it->second->UpperLeft().y - ClientUpperLeft().y) / it->second->Height();
        it->second->SizeMove(Width() - FLEET_BUTTON_SIZE, order * FLEET_BUTTON_SIZE, Width(), (order + 1) * FLEET_BUTTON_SIZE);
    }
    for (std::map<int, FleetButton*>::iterator it = m_moving_fleet_markers.begin(); it != m_moving_fleet_markers.end(); ++it) {
        int order = ((ClientUpperLeft().y + Height()) - it->second->LowerRight().y) / it->second->Height();
        it->second->SizeMove(0, Height() - (order + 1) * FLEET_BUTTON_SIZE, FLEET_BUTTON_SIZE, Height() - order * FLEET_BUTTON_SIZE);
    }
}

void SystemIcon::ShowName()
{
    m_name->Show();
}

void SystemIcon::HideName()
{
    m_name->Hide();
}

void SystemIcon::CreateFleetButtons(const System* sys)
{
    // clear out old fleet buttons
    for (std::map<int, FleetButton*>::iterator it = m_stationary_fleet_markers.begin(); it != m_stationary_fleet_markers.end(); ++it)
        DeleteChild(it->second);
    for (std::map<int, FleetButton*>::iterator it = m_moving_fleet_markers.begin(); it != m_moving_fleet_markers.end(); ++it)
        DeleteChild(it->second);
    m_stationary_fleet_markers.clear();
    m_moving_fleet_markers.clear();

    const int FLEET_BUTTON_SIZE = static_cast<int>(Height() * FLEET_BUTTON_HEIGHT);
    GG::Pt size = WindowDimensions();
    System::ConstObjectVec fleets = sys->FindObjects(IsFleet);
    int stationary_y = 0;
    int moving_y = size.y - FLEET_BUTTON_SIZE;
    for (unsigned int i = 0; i < fleets.size(); ++i) {
        const Fleet* fleet = dynamic_cast<const Fleet*>(fleets[i]);
        int owning_empire_id = *fleet->Owners().begin();
        const Empire* empire = HumanClientApp::Empire().Lookup(owning_empire_id);
        if (fleet->MoveOrders() == UniverseObject::INVALID_OBJECT_ID) {
            if (m_stationary_fleet_markers.find(owning_empire_id) == m_stationary_fleet_markers.end()) {
                m_stationary_fleet_markers[owning_empire_id] = 
                    new FleetButton(size.x - FLEET_BUTTON_SIZE, stationary_y, FLEET_BUTTON_SIZE, FLEET_BUTTON_SIZE, empire->Color(), SHAPE_LEFT);
                AttachChild(m_stationary_fleet_markers[owning_empire_id]);
                stationary_y += FLEET_BUTTON_SIZE;
            }
        } else {
            if (m_moving_fleet_markers.find(owning_empire_id) == m_moving_fleet_markers.end()) {
                m_moving_fleet_markers[owning_empire_id] = 
                    new FleetButton(0, moving_y, FLEET_BUTTON_SIZE, FLEET_BUTTON_SIZE, empire->Color(), SHAPE_RIGHT);
                AttachChild(m_moving_fleet_markers[owning_empire_id]);
                moving_y -= FLEET_BUTTON_SIZE;
            }
        }
    }
}

void SystemIcon::Refresh()
{
    const System* sys = dynamic_cast<const System*>(ClientApp::GetUniverse().Object(m_system_ID));

    SetText(sys->Name());
    m_name->SetText(sys->Name());

    CreateFleetButtons(sys);
}

void SystemIcon::PositionSystemName()
{
    if (m_name)
        m_name->MoveTo(Width() / 2 - m_name->Width() / 2, Height());
}
