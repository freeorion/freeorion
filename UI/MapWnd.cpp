//MapWnd.cpp

#include "MapWnd.h"

#include "ClientUI.h"
#include "../universe/Universe.h"
#include "GGDrawUtil.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../util/Random.h"
#include "SidePanel.h"
#include "../universe/System.h"
#include "SystemIcon.h"
#include "../universe/UniverseObject.h"
#include "CUIControls.h"
#include "../network/Message.h"
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
}

// static(s)
const double MapWnd::MIN_SCALE_FACTOR = 0.5;
const double MapWnd::MAX_SCALE_FACTOR = 8.0;
const int    MapWnd::NUM_BACKGROUNDS = 3;

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
    return 1;
}

int MapWnd::Keypress (GG::Key key, Uint32 key_mods)
{
    if (key == GG::GGK_RETURN)		// start turn
    {
      HumanClientApp::GetApp()->StartTurn( );
    }
    if (key == GG::GGK_F10)		// If F10 is pressed, loads up the options screen
    					// TODO: Find out why F10 is not recognised until the left
					// mouse button has been clicked
    {
        InGameOptions   m_options;
        m_options.Run();

	if (m_options.m_quit)
	{
	  // TODO: Shut down the game.  The following is temporary
	  HumanClientApp::GetApp()->ShutDown();
	}
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
        if (m_zoom_factor < MAX_SCALE_FACTOR) {
            ul_offset.x = static_cast<int>(ul_offset.x * ZOOM_STEP_SIZE);
            ul_offset.y = static_cast<int>(ul_offset.y * ZOOM_STEP_SIZE);
            m_zoom_factor *= ZOOM_STEP_SIZE;
        } else {
            ul_offset.x = static_cast<int>(ul_offset.x * MAX_SCALE_FACTOR / m_zoom_factor);
            ul_offset.y = static_cast<int>(ul_offset.y * MAX_SCALE_FACTOR / m_zoom_factor);
            m_zoom_factor = MAX_SCALE_FACTOR;
        }
    } else if ( 0 > move ) {
        if (MIN_SCALE_FACTOR < m_zoom_factor) {
            ul_offset.x = static_cast<int>(ul_offset.x / ZOOM_STEP_SIZE);
            ul_offset.y = static_cast<int>(ul_offset.y / ZOOM_STEP_SIZE);
            m_zoom_factor /= ZOOM_STEP_SIZE;
        } else {
            ul_offset.x = static_cast<int>(ul_offset.x * MIN_SCALE_FACTOR / m_zoom_factor);
            ul_offset.y = static_cast<int>(ul_offset.y * MIN_SCALE_FACTOR / m_zoom_factor);
            m_zoom_factor = MIN_SCALE_FACTOR;
        }
    }
    else
      // Windows platform always sends an additional event with a move of 0. This should be ignored
      return 0;

    for (unsigned int i = 0; i < m_stars.size(); ++i) {
        const System* system = 
            dynamic_cast<const System*>(HumanClientApp::GetUniverse().Object(m_stars[i]->SystemID()));
        GG::Pt icon_ul(static_cast<int>((system->X() - SystemIcon::ICON_WIDTH / 2) * m_zoom_factor), 
                       static_cast<int>((system->Y() - SystemIcon::ICON_HEIGHT / 2) * m_zoom_factor));
        m_stars[i]->SizeMove(icon_ul.x, icon_ul.y, 
                             static_cast<int>(icon_ul.x + SystemIcon::ICON_WIDTH * m_zoom_factor + 0.5), 
                             static_cast<int>(icon_ul.y + SystemIcon::ICON_HEIGHT * m_zoom_factor + 0.5));
    }
    GG::Pt map_move = ul_offset + center - ul;
    OffsetMove(map_move);
    m_side_panel->OffsetMove(-map_move);
    m_turn_update->OffsetMove(-map_move);

    // this correction ensures that zooming in doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ul;
    m_side_panel->OffsetMove(-final_move);
    m_turn_update->OffsetMove(-final_move);
    MoveBackgrounds(final_move);
    MoveTo(move_to_pt - GG::Pt(GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight()));

    return 1;
}

void MapWnd::InitTurn( int turn_number )
{
    // remove icons from UI
    for (unsigned int i = 0; i < m_stars.size(); ++i) {
      DeleteChild( m_stars[i] );
    }
    // clear stars
    m_stars.clear();

    Universe::ObjectIDVec system_IDs = ClientApp::GetUniverse().FindObjectIDs(IsSystem);
    for (unsigned int i = 0; i < system_IDs.size(); ++i) {
        SystemIcon* icon = new SystemIcon(system_IDs[i], m_zoom_factor);
        m_stars.push_back(icon);
        AttachChild(icon);
        GG::Connect(icon->LeftClickedSignal(), &MapWnd::SelectSystem, this);
    }        
    MoveChildUp(m_side_panel);
    m_side_panel->Hide();

    // set turn button to current turn
    m_turn_update->SetText( ClientUI::String("MAP_BTN_TURN_UPDATE") + boost::lexical_cast<std::string>(turn_number ) );    
    MoveChildUp( m_turn_update );
}

void MapWnd::ShowSystemNames()
{
    for (unsigned int i = 0; i < m_stars.size(); ++i)
        m_stars[i]->ShowName();
}

void MapWnd::HideSystemNames()
{
    for (unsigned int i = 0; i < m_stars.size(); ++i)
        m_stars[i]->HideName();
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
        GG::Pt ul = ClientUpperLeft() + 
            GG::Pt(static_cast<int>((m_nebula_centers[i].x - m_nebulae[i]->Width() / 2.0) * m_zoom_factor),
                   static_cast<int>((m_nebula_centers[i].y - m_nebulae[i]->Height() / 2.0) * m_zoom_factor));
        m_nebulae[i]->OrthoBlit(ul, 
                                ul + GG::Pt(static_cast<int>(m_nebulae[i]->Width() * m_zoom_factor), 
                                            static_cast<int>(m_nebulae[i]->Width() * m_zoom_factor)), 
                                false);
    }
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

void MapWnd::SelectSystem(int systemID)
{
    m_selected_system_signal(systemID);
}

void MapWnd::CorrectMapPosition(GG::Pt &move_to_pt)
{
    int contents_width = static_cast<int>(m_zoom_factor * Universe::UNIVERSE_WIDTH);
    int app_width =  GG::App::GetApp()->AppWidth();
    int app_height =  GG::App::GetApp()->AppHeight();
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

  
void MapWnd::OnTurnUpdate()
{
  HumanClientApp::GetApp()->StartTurn( );
}



