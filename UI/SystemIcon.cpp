//SystemIcon.cpp

#include "SystemIcon.h"

#include "ClientUI.h"
#include "../universe/Fleet.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "GGTextControl.h"
#include "FleetButton.h"
#include "FleetWindow.h"
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
// SystemIcon
////////////////////////////////////////////////
SystemIcon::SystemIcon(int id, double zoom) :
    GG::Control(0, 0, 1, 1, GG::Wnd::CLICKABLE),
    m_system(*dynamic_cast<const System*>(ClientApp::GetUniverse().Object(id))),
    m_static_graphic(0),
    m_name(0)
{
    Connect(m_system.StateChangedSignal(), &SystemIcon::Refresh, this);

    SetText(m_system.Name());

    //resize to the proper size
    SizeMove((int)(m_system.X() * zoom - ICON_WIDTH / 2),
             (int)(m_system.Y() * zoom - ICON_HEIGHT / 2),
             (int)(m_system.X() * zoom - ICON_WIDTH / 2 + ICON_WIDTH),
             (int)(m_system.Y() * zoom - ICON_HEIGHT / 2 + ICON_HEIGHT)); //to position of this universe object

    //load the proper graphic for the color of the star
    boost::shared_ptr<GG::Texture> graphic;
    switch (m_system.Star()) {
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
    m_name = new GG::TextControl(0, 0, m_system.Name(), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
    PositionSystemName();
    AttachChild(m_name);

    Refresh();
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

int SystemIcon::LClick(const GG::Pt& pt, Uint32 keys) 
{
    if (!Disabled()) 
        m_left_click_signal(m_system.ID()); 
    return 1;
}

int SystemIcon::RClick(const GG::Pt& pt, Uint32 keys) 
{
    if (!Disabled()) 
        m_right_click_signal(m_system.ID()); 
    return 1;
}

int SystemIcon::LDoubleClick(const GG::Pt& pt, Uint32 keys) 
{
    if (!Disabled()) 
        m_left_double_click_signal(m_system.ID()); 
    return 1;
}

void SystemIcon::ShowName()
{
    m_name->Show();
}

void SystemIcon::HideName()
{
    m_name->Hide();
}

void SystemIcon::CreateFleetButtons()
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
    System::ConstObjectVec fleets = m_system.FindObjects(IsFleet);
    int stationary_y = 0;
    int moving_y = size.y - FLEET_BUTTON_SIZE;
    for (EmpireManager::const_iterator it = HumanClientApp::Empires().begin(); it != HumanClientApp::Empires().end(); ++it) {
        std::vector<int> fleet_IDs = m_system.FindObjectIDs(IsStationaryFleetFunctor(it->first));
        if (!fleet_IDs.empty()) {
            m_stationary_fleet_markers[it->first] = 
                new FleetButton(size.x - FLEET_BUTTON_SIZE, stationary_y, FLEET_BUTTON_SIZE, FLEET_BUTTON_SIZE, it->second->Color(), fleet_IDs, SHAPE_LEFT);
            AttachChild(m_stationary_fleet_markers[it->first]);
            stationary_y += FLEET_BUTTON_SIZE;
        }
        fleet_IDs = m_system.FindObjectIDs(IsOrderedMovingFleetFunctor(it->first));
        if (!fleet_IDs.empty()) {
            m_moving_fleet_markers[it->first] = 
                new FleetButton(0, moving_y, FLEET_BUTTON_SIZE, FLEET_BUTTON_SIZE, it->second->Color(), fleet_IDs, SHAPE_RIGHT);
            AttachChild(m_moving_fleet_markers[it->first]);
            moving_y -= FLEET_BUTTON_SIZE;
        }
    }
}

void SystemIcon::Refresh()
{
    SetText(m_system.Name());
    m_name->SetText(m_system.Name());

    const std::set<int>& owners = m_system.Owners();
    GG::Clr text_color = ClientUI::TEXT_COLOR;
    if (!owners.empty()) {
        text_color = HumanClientApp::Empires().Lookup(*owners.begin())->Color();
    }
    m_name->SetColor(text_color);

    System::ConstObjectVec fleets = m_system.FindObjects(&IsFleet);
    for (unsigned int i = 0; i < fleets.size(); ++i)
        Connect(fleets[i]->StateChangedSignal(), &SystemIcon::CreateFleetButtons, this);

    CreateFleetButtons();
}

void SystemIcon::PositionSystemName()
{
    if (m_name)
        m_name->MoveTo(Width() / 2 - m_name->Width() / 2, Height());
}
