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
#include "MapWnd.h"
#include "../universe/Predicates.h"
#include "../universe/System.h"

#include <boost/lexical_cast.hpp>

const int IMAGES_PER_STAR_TYPE = 2; // number of star images available per star type (named "type1.png", "type2.png", ...)

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
    GG::Pt ul(static_cast<int>((m_system.X() - ClientUI::SYSTEM_ICON_SIZE / 2) * zoom),
              static_cast<int>((m_system.Y() - ClientUI::SYSTEM_ICON_SIZE / 2) * zoom));
    SizeMove(ul.x, ul.y,
             static_cast<int>(ul.x + ClientUI::SYSTEM_ICON_SIZE * zoom + 0.5),
             static_cast<int>(ul.y + ClientUI::SYSTEM_ICON_SIZE * zoom + 0.5));

    // star graphic
    boost::shared_ptr<GG::Texture> graphic;
    std::string system_image = ClientUI::ART_DIR + "stars/";
    switch (m_system.Star()) {
        case System::BLUE: system_image += "blue2"; break;
        case System::WHITE:    system_image += "yellow1"; break;
        case System::YELLOW: system_image += "yellow2"; break;
        case System::ORANGE: system_image += "red4"; break;
        case System::RED: system_image += "red3"; break;
        case System::NEUTRON:   system_image += "blue2"; break;
        case System::BLACK:   system_image += "blue2"; break;
    default:               system_image += "blue2"; break;
    }
    system_image += /*boost::lexical_cast<std::string>((m_system.ID() % IMAGES_PER_STAR_TYPE) + 1) +*/ ".png";
    graphic = HumanClientApp::GetApp()->GetTexture(system_image);

    //setup static graphic
    m_static_graphic = new GG::StaticGraphic(0, 0, Width(), Height(), graphic, GG::GR_FITGRAPHIC);
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

    const int BUTTON_SIZE = static_cast<int>(Height() * ClientUI::FLEET_BUTTON_SIZE);
    GG::Pt size = Size();
    int stationary_y = 0;
    for (std::map<int, FleetButton*>::iterator it = m_stationary_fleet_markers.begin(); it != m_stationary_fleet_markers.end(); ++it) {
        it->second->SizeMove(size.x - BUTTON_SIZE, stationary_y, size.x, stationary_y + BUTTON_SIZE);
        stationary_y += BUTTON_SIZE;
    }
    int moving_y = size.y - BUTTON_SIZE;
    for (std::map<int, FleetButton*>::iterator it = m_moving_fleet_markers.begin(); it != m_moving_fleet_markers.end(); ++it) {
        it->second->SizeMove(0, moving_y, BUTTON_SIZE, moving_y + BUTTON_SIZE);
        moving_y -= BUTTON_SIZE;
    }
}

void SystemIcon::LClick(const GG::Pt& pt, Uint32 keys)
{
    if (!Disabled())
        m_left_click_signal(m_system.ID());
}

void SystemIcon::RClick(const GG::Pt& pt, Uint32 keys)
{
    if (!Disabled())
        m_right_click_signal(m_system.ID());
}

void SystemIcon::LDoubleClick(const GG::Pt& pt, Uint32 keys)
{
    if (!Disabled())
        m_left_double_click_signal(m_system.ID());
}

void SystemIcon::ClickFleetButton(Fleet* fleet)
{
    for (unsigned int i = 0; i < m_stationary_fleet_markers.size(); ++i) {
        if (std::find(m_stationary_fleet_markers[i]->Fleets().begin(), m_stationary_fleet_markers[i]->Fleets().end(), fleet) !=
            m_stationary_fleet_markers[i]->Fleets().end()) {
            m_stationary_fleet_markers[i]->LClick(GG::Pt(), 0);
            return;
        }
    }
    for (unsigned int i = 0; i < m_moving_fleet_markers.size(); ++i) {
        if (std::find(m_moving_fleet_markers[i]->Fleets().begin(), m_moving_fleet_markers[i]->Fleets().end(), fleet) !=
            m_moving_fleet_markers[i]->Fleets().end()) {
            m_moving_fleet_markers[i]->LClick(GG::Pt(), 0);
            return;
        }
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

void SystemIcon::CreateFleetButtons()
{
    // clear out old fleet buttons
    for (std::map<int, FleetButton*>::iterator it = m_stationary_fleet_markers.begin(); it != m_stationary_fleet_markers.end(); ++it)
        DeleteChild(it->second);
    for (std::map<int, FleetButton*>::iterator it = m_moving_fleet_markers.begin(); it != m_moving_fleet_markers.end(); ++it)
        DeleteChild(it->second);
    m_stationary_fleet_markers.clear();
    m_moving_fleet_markers.clear();

    const int BUTTON_SIZE = static_cast<int>(Height() * ClientUI::FLEET_BUTTON_SIZE);
    GG::Pt size = Size();
    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    int stationary_y = 0;
    int moving_y = size.y - BUTTON_SIZE;
    for (EmpireManager::const_iterator it = HumanClientApp::Empires().begin(); it != HumanClientApp::Empires().end(); ++it) {
        std::vector<int> fleet_IDs = m_system.FindObjectIDs(IsStationaryFleetFunctor(it->first));
        if (!fleet_IDs.empty()) {
            FleetButton* fb = new FleetButton(size.x - BUTTON_SIZE, stationary_y, BUTTON_SIZE, BUTTON_SIZE, it->second->Color(), fleet_IDs, SHAPE_LEFT);
            m_stationary_fleet_markers[it->first] = fb;
            AttachChild(m_stationary_fleet_markers[it->first]);
            map_wnd->SetFleetMovement(fb);
            stationary_y += BUTTON_SIZE;
        }
        fleet_IDs = m_system.FindObjectIDs(IsOrderedMovingFleetFunctor(it->first));
        if (!fleet_IDs.empty()) {
            FleetButton* fb = new FleetButton(0, moving_y, BUTTON_SIZE, BUTTON_SIZE, it->second->Color(), fleet_IDs, SHAPE_RIGHT);
            m_moving_fleet_markers[it->first] = fb;
            AttachChild(m_moving_fleet_markers[it->first]);
            map_wnd->SetFleetMovement(fb);
            moving_y -= BUTTON_SIZE;
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

    std::vector<const Fleet*> fleets = m_system.FindObjects<Fleet>();
    for (unsigned int i = 0; i < fleets.size(); ++i)
        Connect(fleets[i]->StateChangedSignal(), &SystemIcon::CreateFleetButtons, this);

    CreateFleetButtons();
}

void SystemIcon::PositionSystemName()
{
    if (m_name)
        m_name->MoveTo(Width() / 2 - m_name->Width() / 2, Height());
}
