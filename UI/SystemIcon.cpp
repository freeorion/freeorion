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
    m_system(*ClientApp::GetUniverse().Object<const System>(id)),
    m_static_graphic(0)
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
    case System::BLUE:    system_image += "blue"; break;
    case System::WHITE:   system_image += "white"; break;
    case System::YELLOW:  system_image += "yellow"; break;
    case System::ORANGE:  system_image += "orange"; break;
    case System::RED:     system_image += "red"; break;
    case System::NEUTRON: system_image += "blue"; break;
    case System::BLACK:   system_image += "black"; break;
    default:              system_image += "blue"; break;
    }
    system_image += boost::lexical_cast<std::string>((m_system.ID() % 2/*IMAGES_PER_STAR_TYPE*/) + 1) + ".png";
    graphic = HumanClientApp::GetApp()->GetTexture(system_image);

    //setup static graphic
    m_static_graphic = new GG::StaticGraphic(0, 0, Width(), Height(), graphic, GG::GR_FITGRAPHIC);
    AttachChild(m_static_graphic);

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
    for (unsigned int i = 0; i < m_name.size(); ++i) {
        m_name[i]->Show();
    }
}

void SystemIcon::HideName()
{
    for (unsigned int i = 0; i < m_name.size(); ++i) {
        m_name[i]->Hide();
    }
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
        FleetButton* stationary_fb = 0;
        if (!fleet_IDs.empty()) {
            stationary_fb = new FleetButton(size.x - BUTTON_SIZE, stationary_y, BUTTON_SIZE, BUTTON_SIZE, it->second->Color(), fleet_IDs, SHAPE_LEFT);
            m_stationary_fleet_markers[it->first] = stationary_fb;
            AttachChild(m_stationary_fleet_markers[it->first]);
            map_wnd->SetFleetMovement(stationary_fb);
            stationary_y += BUTTON_SIZE;
        }
        fleet_IDs = m_system.FindObjectIDs(IsOrderedMovingFleetFunctor(it->first));
        FleetButton* moving_fb = 0;
        if (!fleet_IDs.empty()) {
            moving_fb = new FleetButton(0, moving_y, BUTTON_SIZE, BUTTON_SIZE, it->second->Color(), fleet_IDs, SHAPE_RIGHT);
            m_moving_fleet_markers[it->first] = moving_fb;
            AttachChild(m_moving_fleet_markers[it->first]);
            map_wnd->SetFleetMovement(moving_fb);
            moving_y -= BUTTON_SIZE;
        }
        if (stationary_fb && moving_fb) {
            moving_fb->SetCompliment(stationary_fb);
            stationary_fb->SetCompliment(moving_fb);
        }
    }
}

void SystemIcon::Refresh()
{
    SetText(m_system.Name());

    // set up the name text controls
    for (unsigned int i = 0; i < m_name.size(); ++i) {
        DeleteChild(m_name[i]);
    }
    m_name.clear();

    const std::set<int>& owners = m_system.Owners();
    if (owners.size() < 2) {
        GG::Clr text_color = ClientUI::TEXT_COLOR;
        if (!owners.empty()) {
            text_color = HumanClientApp::Empires().Lookup(*owners.begin())->Color();
        }
        m_name.push_back(new GG::TextControl(0, 0, m_system.Name(), ClientUI::FONT, ClientUI::PTS, text_color));
        AttachChild(m_name[0]);
    } else {
        boost::shared_ptr<GG::Font> font = GG::App::GetApp()->GetFont(ClientUI::FONT, ClientUI::PTS);
        Uint32 format = 0;
        std::vector<GG::Font::LineData> lines;
        GG::Pt extent = font->DetermineLines(m_system.Name(), format, 1000, lines);
        unsigned int first_char_pos = 0;
        unsigned int last_char_pos = 0;
        int pixels_per_owner = extent.x / owners.size() + 1; // the +1 is to make sure there is not a stray character left off the end
        int owner_idx = 1;
        for (std::set<int>::const_iterator it = owners.begin(); it != owners.end(); ++it, ++owner_idx) {
            while (last_char_pos < m_system.Name().size() && lines[0].extents[last_char_pos] < (owner_idx * pixels_per_owner)) {
                ++last_char_pos;
            }
            m_name.push_back(new GG::TextControl(0, 0, m_system.Name().substr(first_char_pos, last_char_pos - first_char_pos), 
                                                 ClientUI::FONT, ClientUI::PTS, HumanClientApp::Empires().Lookup(*it)->Color()));
            AttachChild(m_name.back());
            first_char_pos = last_char_pos;
        }
    }
    PositionSystemName();

    std::vector<const Fleet*> fleets = m_system.FindObjects<Fleet>();
    for (unsigned int i = 0; i < fleets.size(); ++i)
        Connect(fleets[i]->StateChangedSignal(), &SystemIcon::CreateFleetButtons, this);

    CreateFleetButtons();
}

void SystemIcon::PositionSystemName()
{
    if (!m_name.empty()) {
        int total_width = 0;
        std::vector<int> extents;
        for (unsigned int i = 0; i < m_name.size(); ++i) {
            extents.push_back(total_width);
            total_width += m_name[i]->Width();
        }
        for (unsigned int i = 0; i < m_name.size(); ++i) {
            m_name[i]->MoveTo((Width() - total_width) / 2 + extents[i], Height());
        }
    }
}
