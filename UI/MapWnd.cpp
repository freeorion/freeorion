//MapWnd.cpp

#include "MapWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../universe/Fleet.h"
#include "FleetButton.h"
#include "GGDrawUtil.h"
#include "../client/human/HumanClientApp.h"
#include "../network/Message.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../util/Random.h"
#include "SidePanel.h"
#include "SitRepPanel.h"
#include "../universe/System.h"
#include "SystemIcon.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "TurnProgressWnd.h"

#include <vector>


namespace {
const int SIDE_PANEL_WIDTH = 300;
const int MAP_MARGIN_WIDTH = 50;    // the number of pixels of system-less space around all four sides of the starfield
const double ZOOM_STEP_SIZE = 1.25;
const int NUM_NEBULA_TEXTURES = 5;
const int MIN_NEBULAE = 3;
const int MAX_NEBULAE = 8;
const int END_TURN_BTN_WIDTH = 60;
const int SITREP_PANEL_WIDTH = 400;
const int SITREP_PANEL_HEIGHT = 300;
}


////////////////////////////////////////////////
// MapWnd::MovementLineData
////////////////////////////////////////////////
struct MapWnd::MovementLineData
{
    MovementLineData() {}
    MovementLineData(double x_, double y_, System* dest) : x(x_), y(y_), destination(dest) {}
    double x;
    double y;
    System* destination;
    // TODO : color, other properties(?), based on moving empire and destination, etc.
};


////////////////////////////////////////////////
// MapWnd
////////////////////////////////////////////////
// static(s)
const double MapWnd::MIN_SCALE_FACTOR = 0.5;
const double MapWnd::MAX_SCALE_FACTOR = 8.0;
const int    MapWnd::NUM_BACKGROUNDS = 3;


////////////////////////////////////////////////////////////
// MapWndPopup
////////////////////////////////////////////////////////////

MapWndPopup::MapWndPopup( const std::string& t, int x, int y, int h, int w, Uint32 flags ):
    CUI_Wnd( t, x, y, h, w, flags )
{
    // register with map wnd
    HumanClientApp::GetUI()->GetMapWnd()->RegisterPopup( this );
}

MapWndPopup::MapWndPopup(const GG::XMLElement& elem):
    CUI_Wnd( elem )
{

}

MapWndPopup::~MapWndPopup( )
{
    // remove from map wnd
    HumanClientApp::GetUI()->GetMapWnd()->RemovePopup( this );
}

void MapWndPopup::Close( )
{
    // close window as though it's been clicked closed by the user.
    CloseClicked( );
}

////////////////////////////////////////////////////////////
// MapWndPopup
////////////////////////////////////////////////////////////

MapWnd::MapWnd() :
    GG::Wnd(-GG::App::GetApp()->AppWidth(), -GG::App::GetApp()->AppHeight(),
            static_cast<int>(Universe::UNIVERSE_WIDTH * MAX_SCALE_FACTOR) + GG::App::GetApp()->AppWidth() + MAP_MARGIN_WIDTH, 
            static_cast<int>(Universe::UNIVERSE_WIDTH * MAX_SCALE_FACTOR) + GG::App::GetApp()->AppHeight() + MAP_MARGIN_WIDTH, 
            GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE),
    m_backgrounds(NUM_BACKGROUNDS),
    m_bg_scroll_rate(NUM_BACKGROUNDS),
    m_bg_position_X(NUM_BACKGROUNDS),
    m_bg_position_Y(NUM_BACKGROUNDS),
    m_zoom_factor(1.0),
    m_drag_offset(-1, -1),
    m_dragged(false)
{
    SetText("MapWnd");

    m_side_panel = new SidePanel(GG::App::GetApp()->AppWidth() - SIDE_PANEL_WIDTH, 0, SIDE_PANEL_WIDTH, GG::App::GetApp()->AppHeight());
    AttachChild(m_side_panel);
    Connect(SelectedSystemSignal(), &SidePanel::SetSystem, m_side_panel);

    m_sitrep_panel = new SitRepPanel( (GG::App::GetApp()->AppWidth()-SITREP_PANEL_WIDTH)/2, (GG::App::GetApp()->AppHeight()-SITREP_PANEL_HEIGHT)/2, SITREP_PANEL_WIDTH, SITREP_PANEL_HEIGHT );
    AttachChild(m_sitrep_panel);

    //set up background images
    m_backgrounds[0].reset(new GG::Texture());
    m_backgrounds[0]->Load(ClientUI::ART_DIR + "starfield1.png");
    m_bg_position_X[0] = 10.0;
    m_bg_position_Y[0] = 10.0;
    m_bg_scroll_rate[0] = 0.125;

    m_backgrounds[1].reset(new GG::Texture());
    m_backgrounds[1]->Load(ClientUI::ART_DIR + "starfield2.png");
    m_bg_position_X[1] = 10.0;
    m_bg_position_Y[1] = 10.0;
    m_bg_scroll_rate[1] = 0.25;

    m_backgrounds[2].reset(new GG::Texture());
    m_backgrounds[2]->Load(ClientUI::ART_DIR + "starfield3.png");
    m_bg_position_X[2] = 10.0;
    m_bg_position_Y[2] = 10.0;
    m_bg_scroll_rate[2] = 0.5;

    // set up nebulae
    int num_nebulae = RandSmallInt(MIN_NEBULAE, MAX_NEBULAE);
    m_nebulae.resize(num_nebulae);
    m_nebula_centers.resize(num_nebulae);
    SmallIntDistType universe_placement = SmallIntDist(0, static_cast<int>(Universe::UNIVERSE_WIDTH));
    SmallIntDistType nebula_type = SmallIntDist(1, NUM_NEBULA_TEXTURES);
    for (int i = 0; i < num_nebulae; ++i) {
        std::string nebula_filename = "nebula" + boost::lexical_cast<std::string>(nebula_type()) + ".png";
        m_nebulae[i].reset(new GG::Texture());
        m_nebulae[i]->Load(ClientUI::ART_DIR + nebula_filename);
        m_nebula_centers[i] = GG::Pt(universe_placement(), universe_placement());
    }                                                           ///Do seomthing

    // create buttons
    m_turn_update = new CUIButton(GG::App::GetApp()->AppWidth() - END_TURN_BTN_WIDTH - 5, 5, END_TURN_BTN_WIDTH, "" );

    //attach buttons
    AttachChild(m_turn_update);

    //connect signals and slots
    GG::Connect(m_turn_update->ClickedSignal(), &MapWnd::OnTurnUpdate, this);
}

MapWnd::~MapWnd()
{
}

GG::Pt MapWnd::ClientUpperLeft() const
{
    return UpperLeft() + GG::Pt(GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight());
}

int MapWnd::Render()
{
    RenderBackgrounds();
    RenderFleetMovementLines();
    return 1;
}

int MapWnd::Keypress (GG::Key key, Uint32 key_mods)
{
    switch (key) {
    case GG::GGK_RETURN: { // start turn
        HumanClientApp::GetApp()->StartTurn();
        break;
    }

    case GG::GGK_F2: { // show the sitrep window
        if (m_sitrep_panel->Visible())
            m_sitrep_panel->Hide();
        else
            m_sitrep_panel->Show();
        break;
    }

    case GG::GGK_F10: { // load up the options screen
        InGameOptions options;
        options.Run();
        break;
    }

    default:
        break;
    }

    return 1;
}

int MapWnd::LButtonDown (const GG::Pt &pt, Uint32 keys)
{
    m_drag_offset = pt - ClientUpperLeft();
    return 1;
}

int MapWnd::LDrag (const GG::Pt &pt, const GG::Pt &move, Uint32 keys)
{
    GG::Pt move_to_pt = pt - m_drag_offset;
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ClientUpperLeft();
    m_side_panel->OffsetMove(-final_move);
    m_turn_update->OffsetMove(-final_move);
    m_sitrep_panel->OffsetMove(-final_move);
    MoveBackgrounds(final_move);
    MoveTo(move_to_pt - GG::Pt(GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight()));
    m_dragged = true;
    return 1;
}

int MapWnd::LButtonUp (const GG::Pt &pt, Uint32 keys)
{
    m_drag_offset = GG::Pt(-1, -1);
    m_dragged = false;
    return 1;
}

int MapWnd::LClick (const GG::Pt &pt, Uint32 keys)
{
    m_drag_offset = GG::Pt(-1, -1);
    if (!m_dragged)
        m_selected_system_signal(UniverseObject::INVALID_OBJECT_ID);
    m_dragged = false;
    return 1;
}

int MapWnd::MouseWheel(const GG::Pt& pt, int move, Uint32 keys)
{
    GG::Pt ul = ClientUpperLeft();
    GG::Pt center = GG::Pt( GG::App::GetApp()->AppWidth() / 2,  GG::App::GetApp()->AppHeight() / 2);
    GG::Pt ul_offset = ul - center;
    if (0 < move) {
        if (m_zoom_factor * ZOOM_STEP_SIZE < MAX_SCALE_FACTOR) {
            ul_offset.x = static_cast<int>(ul_offset.x * ZOOM_STEP_SIZE);
            ul_offset.y = static_cast<int>(ul_offset.y * ZOOM_STEP_SIZE);
            m_zoom_factor *= ZOOM_STEP_SIZE;
        } else {
            ul_offset.x = static_cast<int>(ul_offset.x * MAX_SCALE_FACTOR / m_zoom_factor);
            ul_offset.y = static_cast<int>(ul_offset.y * MAX_SCALE_FACTOR / m_zoom_factor);
            m_zoom_factor = MAX_SCALE_FACTOR;
        }
    } else if ( 0 > move ) {
        if (MIN_SCALE_FACTOR < m_zoom_factor / ZOOM_STEP_SIZE) {
            ul_offset.x = static_cast<int>(ul_offset.x / ZOOM_STEP_SIZE);
            ul_offset.y = static_cast<int>(ul_offset.y / ZOOM_STEP_SIZE);
            m_zoom_factor /= ZOOM_STEP_SIZE;
        } else {
            ul_offset.x = static_cast<int>(ul_offset.x * MIN_SCALE_FACTOR / m_zoom_factor);
            ul_offset.y = static_cast<int>(ul_offset.y * MIN_SCALE_FACTOR / m_zoom_factor);
            m_zoom_factor = MIN_SCALE_FACTOR;
        }
    } else {
      return 1; // Windows platform always sends an additional event with a move of 0. This should be ignored
    }

    for (unsigned int i = 0; i < m_system_icons.size(); ++i) {
        const System& system = m_system_icons[i]->GetSystem();
        GG::Pt icon_ul(static_cast<int>((system.X() - ClientUI::SYSTEM_ICON_SIZE / 2) * m_zoom_factor), 
                       static_cast<int>((system.Y() - ClientUI::SYSTEM_ICON_SIZE / 2) * m_zoom_factor));
        m_system_icons[i]->SizeMove(icon_ul.x, icon_ul.y, 
                             static_cast<int>(icon_ul.x + ClientUI::SYSTEM_ICON_SIZE * m_zoom_factor + 0.5), 
                             static_cast<int>(icon_ul.y + ClientUI::SYSTEM_ICON_SIZE * m_zoom_factor + 0.5));
    }

    for (unsigned int i = 0; i < m_fleet_buttons.size(); ++i) {
        Fleet* fleet = *m_fleet_buttons[i]->Fleets().begin();
        double x = fleet->X();
        double y = fleet->Y();
        GG::Pt button_ul(static_cast<int>((x - ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE / 2) * m_zoom_factor), 
                         static_cast<int>((y - ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE / 2) * m_zoom_factor));
        m_fleet_buttons[i]->SizeMove(button_ul.x, button_ul.y, 
                                     static_cast<int>(button_ul.x + ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE * m_zoom_factor + 0.5), 
                                     static_cast<int>(button_ul.y + ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE * m_zoom_factor + 0.5));
    }

    GG::Pt map_move = ul_offset + center - ul;
    OffsetMove(map_move);
    MoveBackgrounds(map_move);
    m_side_panel->OffsetMove(-map_move);
    m_turn_update->OffsetMove(-map_move);
    m_sitrep_panel->OffsetMove(-map_move);

    // this correction ensures that zooming in doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ul;
    m_side_panel->OffsetMove(-final_move);
    m_turn_update->OffsetMove(-final_move);
    m_sitrep_panel->OffsetMove(-final_move);
    MoveBackgrounds(final_move);
    MoveTo(move_to_pt - GG::Pt(GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight()));

    return 1;
}

void MapWnd::InitTurn(int turn_number)
{
    Universe& universe = ClientApp::GetUniverse();

    // systems
    for (unsigned int i = 0; i < m_system_icons.size(); ++i) {
        DeleteChild(m_system_icons[i]);
    }
    m_system_icons.clear();

    Universe::ObjectIDVec system_IDs = universe.FindObjectIDs(IsSystem);
    for (unsigned int i = 0; i < system_IDs.size(); ++i) {
        SystemIcon* icon = new SystemIcon(system_IDs[i], m_zoom_factor);
        m_system_icons.push_back(icon);
        AttachChild(icon);
        GG::Connect(icon->LeftClickedSignal(), &MapWnd::SelectSystem, this);
    }        

    // fleets not in systems
    for (unsigned int i = 0; i < m_fleet_buttons.size(); ++i) {
        DeleteChild(m_fleet_buttons[i]);
    }
    m_fleet_buttons.clear();
    m_fleet_lines.clear();

    Universe::ObjectVec fleets = universe.FindObjects(IsMovingFleetFunctor());
    Universe::ObjectVec ordered_moving_fleets = universe.FindObjects(IsOrderedMovingFleetFunctor());
    fleets.insert(fleets.end(), ordered_moving_fleets.begin(), ordered_moving_fleets.end());

    typedef std::multimap<std::pair<double, double>, UniverseObject*> SortedFleetMap;
    SortedFleetMap position_sorted_fleets;
    for (unsigned int i = 0; i < fleets.size(); ++i) {
        position_sorted_fleets.insert(std::make_pair(std::make_pair(fleets[i]->X(), fleets[i]->Y()), fleets[i]));
    }

    SortedFleetMap::iterator it = position_sorted_fleets.begin();
    SortedFleetMap::iterator end_it = position_sorted_fleets.end();
    while (it != end_it) {
        SortedFleetMap::iterator local_end_it = position_sorted_fleets.upper_bound(it->first);
        std::map<int, std::vector<int> > IDs_by_empire_color;
        for (; it != local_end_it; ++it) {
            IDs_by_empire_color[*it->second->Owners().begin()].push_back(it->second->ID());
        }
        for (std::map<int, std::vector<int> >::iterator ID_it = IDs_by_empire_color.begin(); ID_it != IDs_by_empire_color.end(); ++ID_it) {
            FleetButton* fb = new FleetButton(Empires().Lookup(ID_it->first)->Color(), ID_it->second, m_zoom_factor);
            m_fleet_buttons.push_back(fb);
            AttachChild(fb);
            SetFleetMovement(fb);
        }
    }

    MoveChildUp(m_side_panel);
    m_side_panel->Hide();

    // set turn button to current turn
    m_turn_update->SetText( ClientUI::String("MAP_BTN_TURN_UPDATE") + boost::lexical_cast<std::string>(turn_number ) );    
    MoveChildUp( m_turn_update );

    MoveChildUp(m_sitrep_panel);
    // are there any sitreps to show?
    Empire *pEmpire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->PlayerID() );
    m_sitrep_panel->Update();
    if ( pEmpire->NumSitReps( ) )
        m_sitrep_panel->Show();
    else
        m_sitrep_panel->Hide();
}

void MapWnd::ShowSystemNames()
{
    for (unsigned int i = 0; i < m_system_icons.size(); ++i)
        m_system_icons[i]->ShowName();
}

void MapWnd::HideSystemNames()
{
    for (unsigned int i = 0; i < m_system_icons.size(); ++i)
        m_system_icons[i]->HideName();
}

void MapWnd::CenterOnMapCoord(double x, double y)
{
    GG::Pt ul = ClientUpperLeft();
    double current_x = (GG::App::GetApp()->AppWidth() / 2 - ul.x) / m_zoom_factor;
    double current_y = (GG::App::GetApp()->AppHeight() / 2 - ul.y) / m_zoom_factor;
    GG::Pt map_move = GG::Pt(static_cast<int>((current_x - x) * m_zoom_factor), 
                             static_cast<int>((current_y - y) * m_zoom_factor));
    OffsetMove(map_move);
    MoveBackgrounds(map_move);
    m_side_panel->OffsetMove(-map_move);
    m_turn_update->OffsetMove(-map_move);
    m_sitrep_panel->OffsetMove(-map_move);

    // this correction ensures that the centering doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ul;
    m_side_panel->OffsetMove(-final_move);
    m_turn_update->OffsetMove(-final_move);
    m_sitrep_panel->OffsetMove(-final_move);
    MoveBackgrounds(final_move);
    MoveTo(move_to_pt - GG::Pt(GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight()));
}

void MapWnd::CenterOnSystem(int systemID)
{
    if (System* system = dynamic_cast<System*>(GetUniverse().Object(systemID)))
        CenterOnSystem(system);
}

void MapWnd::CenterOnFleet(int fleetID)
{
    if (Fleet* fleet = dynamic_cast<Fleet*>(GetUniverse().Object(fleetID)))
        CenterOnFleet(fleet);
}

void MapWnd::CenterOnSystem(System* system)
{
    CenterOnMapCoord(system->X(), system->Y());
}

void MapWnd::CenterOnFleet(Fleet* fleet)
{
    CenterOnMapCoord(fleet->X(), fleet->Y());
}

void MapWnd::SelectSystem(int systemID)
{
    m_selected_system_signal(systemID);
}

void MapWnd::SelectFleet(int fleetID)
{
    if (Fleet* fleet = dynamic_cast<Fleet*>(GetUniverse().Object(fleetID)))
        SelectFleet(fleet);
}

void MapWnd::SelectSystem(System* system)
{
    SelectSystem(system->ID());
}

void MapWnd::SelectFleet(Fleet* fleet)
{
    if (System* system = fleet->GetSystem()) {
        for (unsigned int i = 0; i < m_system_icons.size(); ++i) {
            if (&m_system_icons[i]->GetSystem() == system) {
                m_system_icons[i]->ClickFleetButton(fleet);
                break;
            }
        }
    } else {
        for (unsigned int i = 0; i < m_fleet_buttons.size(); ++i) {
            if (std::find(m_fleet_buttons[i]->Fleets().begin(), m_fleet_buttons[i]->Fleets().end(), fleet) != m_fleet_buttons[i]->Fleets().end()) {
                m_fleet_buttons[i]->LClick(GG::Pt(), 0);
                break;
            }
        }
    }
}

void MapWnd::SetFleetMovement(FleetButton* fleet_button)
{
    std::set<int> destinations;
    for (unsigned int i = 0; i < fleet_button->Fleets().size(); ++i) {
        Fleet* fleet = fleet_button->Fleets()[i];
        GG::Pt ul = ClientUpperLeft();
        if (fleet->DestinationID() != UniverseObject::INVALID_OBJECT_ID &&
            fleet->DestinationID() != fleet->SystemID() &&
            destinations.find(fleet->DestinationID()) == destinations.end()) {
            destinations.insert(fleet->DestinationID());
            GG::Pt fleet_ul = fleet_button->UpperLeft();
            GG::Pt sz = fleet_button->WindowDimensions();
            m_fleet_lines[fleet] = MovementLineData((fleet_ul.x + sz.x / 2 - ul.x) / m_zoom_factor, 
                                                    (fleet_ul.y + sz.y / 2 - ul.y) / m_zoom_factor, 
                                                    fleet->Destination());
        } else {
            m_fleet_lines.erase(fleet);
        }
    }
}

void MapWnd::OnTurnUpdate()
{
    //hide sidepanel
    m_side_panel->SetSystem(INVALID_OBJECT_ID);
    // delete app popups
    DeleteAllPopups( );

    HumanClientApp::GetApp()->StartTurn();
}

void MapWnd::RenderBackgrounds()
{
    double x, y;
    glColor3d(1.0, 1.0, 1.0);
    for (int i = 0; i < NUM_BACKGROUNDS; ++i) {
        int bg_width = m_backgrounds[i]->Width();
        int bg_height = m_backgrounds[i]->Height();
        int app_width = GG::App::GetApp()->AppWidth();
        int app_height = GG::App::GetApp()->AppHeight();
        x = std::fmod(m_bg_position_X[i], bg_width);
        while (x < app_width + bg_width) {
            y = std::fmod(m_bg_position_Y[i], bg_height);
            while (y < app_height + bg_height) {
                m_backgrounds[i]->OrthoBlit(static_cast<int>(x - bg_width), static_cast<int>(y - bg_height), false);
                y += m_backgrounds[i]->Height();
            }
            x += m_backgrounds[i]->Width();
        }
    }

    for (unsigned int i = 0; i < m_nebulae.size(); ++i) {
        GG::Pt ul = 
            ClientUpperLeft() + 
            GG::Pt(static_cast<int>((m_nebula_centers[i].x - m_nebulae[i]->Width() / 2.0) * m_zoom_factor),
                   static_cast<int>((m_nebula_centers[i].y - m_nebulae[i]->Height() / 2.0) * m_zoom_factor));
        m_nebulae[i]->OrthoBlit(ul, 
                                ul + GG::Pt(static_cast<int>(m_nebulae[i]->Width() * m_zoom_factor), 
                                            static_cast<int>(m_nebulae[i]->Width() * m_zoom_factor)), 
                                false);
    }
}

void MapWnd::RenderFleetMovementLines()
{
    const unsigned int PATTERN = 0xF0F0F0F0;
    const double RATE = 0.5;
    const int SHIFT = static_cast<int>(GG::App::GetApp()->Ticks() * RATE / 32.0) % 32;
    const unsigned int STIPPLE = (PATTERN << SHIFT) | (PATTERN >> (32 - SHIFT));
    const double LINE_SCALE = std::max(1.0, m_zoom_factor / MIN_SCALE_FACTOR);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(static_cast<int>(LINE_SCALE / 2.5), STIPPLE);
    glLineWidth(LINE_SCALE / 2.5);
    glBegin(GL_LINES);

    GG::Pt ul = ClientUpperLeft();
    for (std::map<Fleet*, MovementLineData>::iterator it = m_fleet_lines.begin(); it != m_fleet_lines.end(); ++it) {
        glColor3d(1.0, 1.0, 1.0);
        glVertex2d(ul.x + it->second.x * m_zoom_factor, ul.y + it->second.y * m_zoom_factor);
        glVertex2d(ul.x + it->second.destination->X() * m_zoom_factor, ul.y + it->second.destination->Y() * m_zoom_factor);
    }

    glEnd();
    glLineWidth(1.0);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_LINE_STIPPLE);
    glEnable(GL_TEXTURE_2D);
}

void MapWnd::MoveBackgrounds(const GG::Pt& move)
{   
    double move_x, move_y;

    for (int i = 0; i < NUM_BACKGROUNDS; ++i) {
        move_x = move.x * m_bg_scroll_rate[i];
        move_y = move.y * m_bg_scroll_rate[i];
        m_bg_position_X[i] += move_x;
        m_bg_position_Y[i] += move_y;    
    }
}

void MapWnd::CorrectMapPosition(GG::Pt &move_to_pt)
{
    int contents_width = static_cast<int>(m_zoom_factor * Universe::UNIVERSE_WIDTH);
    int app_width =  GG::App::GetApp()->AppWidth();
    int app_height = GG::App::GetApp()->AppHeight();
    if (app_width < contents_width) {
        if (MAP_MARGIN_WIDTH < move_to_pt.x)
            move_to_pt.x = MAP_MARGIN_WIDTH;
        if (move_to_pt.x < app_width - contents_width - 2 * MAP_MARGIN_WIDTH)
            move_to_pt.x = app_width - contents_width - 2 * MAP_MARGIN_WIDTH;
    } else {
        if (move_to_pt.x < 0)
            move_to_pt.x = 0;
        if (app_width - contents_width < move_to_pt.x)
            move_to_pt.x = app_width - contents_width;
    }

    if (app_height < contents_width) {
        if (MAP_MARGIN_WIDTH < move_to_pt.y)
            move_to_pt.y = MAP_MARGIN_WIDTH;
        if (move_to_pt.y < app_height - contents_width - 2 * MAP_MARGIN_WIDTH)
            move_to_pt.y = app_height - contents_width - 2 * MAP_MARGIN_WIDTH;
    } else {
        if (move_to_pt.y < 0)
            move_to_pt.y = 0;
        if (app_height - contents_width < move_to_pt.y)
            move_to_pt.y = app_height - contents_width;
    }
}


void MapWnd::RegisterPopup( MapWndPopup* popup )
{
    if (popup) {
        m_popups.push_back(popup);
    }
}


void MapWnd::RemovePopup( MapWndPopup* popup )
{
    if (popup) {

        std::list<MapWndPopup*>::iterator it = std::find( m_popups.begin(), m_popups.end(), popup );
        if (it != m_popups.end())
            m_popups.erase(it);
    }
}


void MapWnd::DeleteAllPopups( )
{
    for (std::list<MapWndPopup*>::iterator it = m_popups.begin(); it != m_popups.end(); ++it) {
        // get popup and increment iterator first since closing the popup will change this list by removing the poup
        MapWndPopup *popup = *it;
        it++;
        popup->Close( );
    }   
    // clear list
    m_popups.clear( );
}
