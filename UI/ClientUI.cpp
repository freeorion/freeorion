//ClientUI.cpp

#include "ClientUI.h"

#include "../util/AppInterface.h"
#include "CUIControls.h"
#include "../universe/Fleet.h"
#include "GGApp.h"
#include "GGClr.h"
#include "GGDrawUtil.h"
#include "dialogs/GGThreeButtonDlg.h"
#include "IntroScreen.h"
#include "MapWnd.h"
#include "SidePanel.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Ship.h"
#include "ToolContainer.h"
#include "TurnProgressWnd.h"
#include "../client/human/HumanClientApp.h"
#include "../util/OptionsDB.h"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <string>
#include <fstream>


//static members
std::string ClientUI::FONT          = "Vera.ttf";
int         ClientUI::PTS           = 12;
std::string ClientUI::TITLE_FONT    = "Vera.ttf";
int         ClientUI::TITLE_PTS     = 12;

std::string ClientUI::DIR           = "default/";
std::string ClientUI::ART_DIR       = ClientUI::DIR + "art/small/";

GG::Clr     ClientUI::TEXT_COLOR(255,255,255,255);

// windows
GG::Clr     ClientUI::WND_COLOR(0,0,0,210);
GG::Clr     ClientUI::WND_BORDER_COLOR(0,0,0,255);
GG::Clr     ClientUI::WND_OUTER_BORDER_COLOR(64,64,64,255);
GG::Clr     ClientUI::WND_INNER_BORDER_COLOR(255,255,255,255);

// controls
GG::Clr     ClientUI::CTRL_COLOR(30,30,30,255); 
GG::Clr     ClientUI::CTRL_BORDER_COLOR(124, 124, 124, 255);

GG::Clr     ClientUI::BUTTON_COLOR(0, 0, 0, 255);
int         ClientUI::BUTTON_WIDTH = 7;

GG::Clr     ClientUI::STATE_BUTTON_COLOR(0, 127, 0, 255);

GG::Clr     ClientUI::SCROLL_TAB_COLOR(60, 60, 60, 255);
int         ClientUI::SCROLL_WIDTH = 13;

GG::Clr     ClientUI::DROP_DOWN_LIST_INT_COLOR(0, 0, 0, 255);
GG::Clr     ClientUI::DROP_DOWN_LIST_ARROW_COLOR(130, 130, 0, 255);

GG::Clr     ClientUI::EDIT_INT_COLOR(0, 0, 0, 255);

GG::Clr     ClientUI::MULTIEDIT_INT_COLOR(0, 0, 0, 255);

GG::Clr     ClientUI::STAT_INCR_COLOR(127, 255, 127, 255);
GG::Clr     ClientUI::STAT_DECR_COLOR(255, 127, 127, 255);

int         ClientUI::SYSTEM_ICON_SIZE = 32;
double      ClientUI::FLEET_BUTTON_SIZE = 0.2;

// game UI windows
GG::Clr     ClientUI::SIDE_PANEL_COLOR(0, 0, 0, 220);
GG::Clr     ClientUI::SIDE_PANEL_BUILD_PROGRESSBAR_COLOR(25, 40, 140, 150);
int         ClientUI::SIDE_PANEL_PLANET_NAME_PTS = 15;
int         ClientUI::SIDE_PANEL_PTS = 11;


//private static members
log4cpp::Category& ClientUI::s_logger(log4cpp::Category::getRoot());
ClientUI* ClientUI::s_the_UI = 0;

namespace {
    // an internal LUT of string IDs for each SitRep type
    // It's in this module becaue SitReps know nothing about how they
    // should be rendered - this is up to the client UI
    const char* g_string_id_lut[ SitRepEntry::NUM_SITREP_TYPES ] =
	{
	    "SITREP_MAX_INDUSTRY",
	    "SITREP_SHIP_BUILT",
	    "SITREP_TECH_RESEARCHED",
	    "SITREP_BASE_BUILT",
	    "SITREP_COMBAT_SYSTEM_WON",
	    "SITREP_COMBAT_SYSTEM_LOST",
	    "SITREP_COMBAT_SYSTEM_NO_VICTOR"
	};
    // command-line options
    void AddOptions(OptionsDB& db)
    {
	db.Add(    "app-width", "Sets horizontal app resolution.", 1024, RangedValidator<int>(640, 1600));
	db.Add(    "app-height", "Sets vertical app resolution.", 768, RangedValidator<int>(480, 1200));
	db.Add('c', "color-depth", "Sets screen color depth, in bits per pixel.", 32, RangedStepValidator<int>(8, 16, 32));

	db.Add<std::string>("UI.dir", "Sets UI resource directory root.", "default");
	db.Add<std::string>("UI.art-dir", "Sets UI art resource directory under \'[UI.dir]/art\'.", "small");
	db.Add<std::string>("UI.font", "Sets UI font resource file.", "Vera.ttf");
	db.Add("UI.font-size", "Sets UI font size.", 12, RangedValidator<int>(4, 40));
	db.Add<std::string>("UI.title-font", "Sets UI title font resource file.", "Vera.ttf");
	db.Add("UI.title-font-size", "Sets UI title font size.", 12, RangedValidator<int>(4, 40));
        
	db.Add("UI.wnd-color.red", "Sets UI window color (red).", 0, RangedValidator<int>(0, 255));
	db.Add("UI.wnd-color.green", "Sets UI window color (green).", 0, RangedValidator<int>(0, 255));
	db.Add("UI.wnd-color.blue", "Sets UI window color (blue).", 0, RangedValidator<int>(0, 255));
	db.Add("UI.wnd-color.alpha", "Sets UI window color (alpha).", 210, RangedValidator<int>(0, 255));
        
	db.Add("UI.text-color.red", "Sets UI text color (red).", 255, RangedValidator<int>(0, 255));
	db.Add("UI.text-color.green", "Sets UI text color (green).", 255, RangedValidator<int>(0, 255));
	db.Add("UI.text-color.blue", "Sets UI text color (blue).", 255, RangedValidator<int>(0, 255));
	db.Add("UI.text-color.alpha", "Sets UI text color (alpha).", 255, RangedValidator<int>(0, 255));

	db.Add("UI.ctrl-color.red", "Sets UI control color (red).", 30, RangedValidator<int>(0, 255));
	db.Add("UI.ctrl-color.green", "Sets UI control color (green).", 30, RangedValidator<int>(0, 255));
	db.Add("UI.ctrl-color.blue", "Sets UI control color (blue).", 30, RangedValidator<int>(0, 255));
	db.Add("UI.ctrl-color.alpha", "Sets UI control color (alpha).", 255, RangedValidator<int>(0, 255));

	db.Add("UI.wnd-outer-border-color.red", "Sets UI outer border color (red).", 64, RangedValidator<int>(0, 255));
	db.Add("UI.wnd-outer-border-color.green", "Sets UI outer border color (green).", 64, RangedValidator<int>(0, 255));
	db.Add("UI.wnd-outer-border-color.blue", "Sets UI outer border color (blue).", 64, RangedValidator<int>(0, 255));
	db.Add("UI.wnd-outer-border-color.alpha", "Sets UI outer border color (alpha).", 255, RangedValidator<int>(0, 255));

	db.Add("UI.wnd-border-color.red", "Sets UI border color (red).", 0, RangedValidator<int>(0, 255));
	db.Add("UI.wnd-border-color.green", "Sets UI border color (green).", 0, RangedValidator<int>(0, 255));
	db.Add("UI.wnd-border-color.blue", "Sets UI border color (blue).", 0, RangedValidator<int>(0, 255));
	db.Add("UI.wnd-border-color.alpha", "Sets UI border color (alpha).", 255, RangedValidator<int>(0, 255));

	db.Add("UI.wnd-inner-border-color.red", "Sets UI inner border color (red).", 255, RangedValidator<int>(0, 255));
	db.Add("UI.wnd-inner-border-color.green", "Sets UI inner border color (green).", 255, RangedValidator<int>(0, 255));
	db.Add("UI.wnd-inner-border-color.blue", "Sets UI inner border color (blue).", 255, RangedValidator<int>(0, 255));
	db.Add("UI.wnd-inner-border-color.alpha", "Sets UI inner border color (alpha).", 255, RangedValidator<int>(0, 255));

	db.Add("UI.tooltip-delay", "Sets UI tooltip popup delay, in ms.", 1000, RangedValidator<int>(0, 3000));
	db.Add<std::string>("UI.stringtable-filename", "Sets UI string table filename.", "eng_stringtable.txt");
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    static SDL_Cursor *createColonizeCursor( )
    {
	/* cursor code taken from SDL examples */

	static const char *image[] = {

	    /* width height num_colors chars_per_pixel */
	    "    32    32        3            1",
	    /* colors */
	    "X c #000000",
	    ". c #ffffff",
	    "  c None",
	    /* pixels */
	    "X                               ",
	    "XX                              ",
	    "X.X                             ",
	    "X..X                            ",
	    "X...X                           ",
	    "X....X         XXXXXXXXX        ",
	    "X.....X        X.......X        ",
	    "X......X       XXXXXXX.X        ",
	    "X.......X            X.X        ",
	    "X........X           X.X        ",
	    "X.....XXXXX          X.X        ",
	    "X..X..X           XXXX.X        ",
	    "X.X X..X          X....X        ",
	    "XX  X..X          X.XXXX        ",
	    "X    X..X         X.X           ",
	    "     X..X         XXX           ",
	    "      X..X                      ",
	    "      X..X        XXXX          ",
	    "       XX         X..X          ",
	    "                  XXXX          ",
	    "                                ",
	    "                                ",
	    "                                ",
	    "                                ",
	    "                                ",
	    "                                ",
	    "                                ",
	    "                                ",
	    "                                ",
	    "                                ",
	    "                                ",
	    "                                ",
	    "0,0"
	};

	int i, row, col;
	Uint8 data[4*32];
	Uint8 mask[4*32];
	int hot_x, hot_y;

	i = -1;
	for ( row=0; row<32; ++row ) {
	    for ( col=0; col<32; ++col ) {
		if ( col % 8 ) {
		    data[i] <<= 1;
		    mask[i] <<= 1;
		} else {
		    ++i;
		    data[i] = mask[i] = 0;
		}
		switch (image[4+row][col]) {
		case 'X':
		    data[i] |= 0x01;
		    mask[i] |= 0x01;
		    break;
		case '.':
		    mask[i] |= 0x01;
		    break;
		case ' ':
		    break;
		}
	    }
	}
	sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);

	return( SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y) );
    }

    class PlanetPicker : public GG::Wnd
    {
    public:
	PlanetPicker(int system_id) : 
	    Wnd(0, 0, GG::App::GetApp()->AppWidth() - 1, GG::App::GetApp()->AppHeight() - 1, CLICKABLE | MODAL),
	    m_side_panel(new SidePanel(GG::App::GetApp()->AppWidth() - MapWnd::SIDE_PANEL_WIDTH, 0, MapWnd::SIDE_PANEL_WIDTH, GG::App::GetApp()->AppHeight())),
	    m_planet_selected(-1)
	{
	    m_side_panel->SetSystem(system_id);
	    AttachChild(m_side_panel);
	    for (int i = 0; i < m_side_panel->PlanetPanels(); ++i) {
		GG::Connect(m_side_panel->GetPlanetPanel(i)->LeftClickedSignal(), &PlanetPicker::PlanetClicked, this);
	    }
	    HumanClientApp::GetUI()->SetCursor(ClientUI::CURSOR_COLONIZE);
	}
	~PlanetPicker() {HumanClientApp::GetUI()->SetCursor(ClientUI::CURSOR_DEFAULT);}
	int PlanetPicked() const {return m_planet_selected;}
	int LButtonUp(const GG::Pt& pt, Uint32 keys) {m_done = true; return 1;}
	int LClick(const GG::Pt& pt, Uint32 keys) {return LButtonUp(pt, keys);}
	int RButtonUp(const GG::Pt& pt, Uint32 keys) {return LButtonUp(pt, keys);}
	int RClick(const GG::Pt& pt, Uint32 keys) {return LButtonUp(pt, keys);}

    private:
	void PlanetClicked(int planet_id) {m_planet_selected = planet_id; m_done = true;}

	SidePanel*       m_side_panel;
	int              m_planet_selected;
    };
}



//Init and Cleanup//////////////////////////////////////
ClientUI::ClientUI() :
    TOOLTIP_DELAY(GetOptionsDB().Get<int>("UI.tooltip-delay")), // 1 second delay for tooltips to appear
    m_tooltips(0),
    m_state(STATE_STARTUP),
    m_string_table(0),
    m_intro_screen(0),
    m_map_wnd(0),
    m_turn_progress_wnd(0),
    m_default_cursor(NULL)
{
    using namespace GG;

    s_the_UI = this;

    WND_BORDER_COLOR = Clr(GetOptionsDB().Get<int>("UI.wnd-border-color.red"),
                           GetOptionsDB().Get<int>("UI.wnd-border-color.green"),
                           GetOptionsDB().Get<int>("UI.wnd-border-color.blue"),
                           GetOptionsDB().Get<int>("UI.wnd-border-color.alpha"));

    CTRL_COLOR = Clr(GetOptionsDB().Get<int>("UI.ctrl-color.red"),
		     GetOptionsDB().Get<int>("UI.ctrl-color.green"),
		     GetOptionsDB().Get<int>("UI.ctrl-color.blue"),
		     GetOptionsDB().Get<int>("UI.ctrl-color.alpha"));

    WND_INNER_BORDER_COLOR = Clr(GetOptionsDB().Get<int>("UI.wnd-inner-border-color.red"),
				 GetOptionsDB().Get<int>("UI.wnd-inner-border-color.green"),
				 GetOptionsDB().Get<int>("UI.wnd-inner-border-color.blue"),
				 GetOptionsDB().Get<int>("UI.wnd-inner-border-color.alpha"));

    WND_OUTER_BORDER_COLOR = Clr(GetOptionsDB().Get<int>("UI.wnd-outer-border-color.red"),
				 GetOptionsDB().Get<int>("UI.wnd-outer-border-color.green"),
				 GetOptionsDB().Get<int>("UI.wnd-outer-border-color.blue"),
				 GetOptionsDB().Get<int>("UI.wnd-outer-border-color.alpha"));

    TEXT_COLOR = Clr(GetOptionsDB().Get<int>("UI.text-color.red"),
		     GetOptionsDB().Get<int>("UI.text-color.green"),
		     GetOptionsDB().Get<int>("UI.text-color.blue"),
		     GetOptionsDB().Get<int>("UI.text-color.alpha"));

    WND_COLOR = Clr(GetOptionsDB().Get<int>("UI.wnd-color.red"),
		    GetOptionsDB().Get<int>("UI.wnd-color.green"),
		    GetOptionsDB().Get<int>("UI.wnd-color.blue"),
		    GetOptionsDB().Get<int>("UI.wnd-color.alpha"));

    PTS       = GetOptionsDB().Get<int>("UI.font-size");
    TITLE_PTS = GetOptionsDB().Get<int>("UI.title-font-size");
    DIR       = GetOptionsDB().Get<std::string>("UI.dir") + "/";
    FONT      = GetOptionsDB().Get<std::string>("UI.font");
    TITLE_FONT= GetOptionsDB().Get<std::string>("UI.title-font");
    ART_DIR   = DIR + "art/" + GetOptionsDB().Get<std::string>("UI.art-dir") + "/";

    //call initialize with stringtable filename
    Initialize(DIR + GetOptionsDB().Get<std::string>("UI.stringtable-filename"));
}

bool ClientUI::Initialize(const std::string& string_table_file)
{
    //initialize Tooltip engine
    m_tooltips = new ToolContainer(TOOLTIP_DELAY);
    
    //initialize string table
    m_string_table = new StringTable(string_table_file);
    
    //initialize UI state & window
    m_state = STATE_STARTUP;
    
    m_map_wnd = new MapWnd();
    GG::App::GetApp()->Register(m_map_wnd);
    m_map_wnd->Hide();

    //clear logging file
    std::ofstream ofs_outfile("ClientUI.log");    
    ofs_outfile.close();

    //setup logger
    log4cpp::Appender* appender = new log4cpp::FileAppender("Appender", "ClientUI.log");
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d %p : %m%n");
    appender->setLayout(layout);
    s_logger.setAdditivity(false);  // make appender the only appender used...
    s_logger.setAppender(appender);
    s_logger.setAdditivity(true);   // ...but allow the addition of others later
    s_logger.setPriority(log4cpp::Priority::DEBUG);
    
    return true;
}

ClientUI::~ClientUI() 
{
    Cleanup();
}

bool ClientUI::Cleanup()
{
    delete m_tooltips;
    m_tooltips = 0;
    
    delete m_string_table;
    m_string_table = 0;

    delete m_intro_screen;
    m_intro_screen = 0;

    delete m_map_wnd;
    m_map_wnd = 0;

    delete m_turn_progress_wnd;
    m_turn_progress_wnd = 0;

    s_the_UI = 0;

    //TODO: Destroy variables, etc.   

    return true; 
}//Cleanup()

const std::string& ClientUI::Language() const 
{
    return m_string_table->Language();
}

bool ClientUI::AttachToolWnd(GG::Wnd* parent, ToolWnd* tool) 
{
    return (m_tooltips ? m_tooltips->AttachToolWnd(parent, tool) : false);
}


//////////////////////////////////////////////////////////

//Utilities////////////////////////////////////////////////
bool ClientUI::ChangeResolution(int width, int height)
{
    // TODO: Determine ability to reinitialize SDL with OpenGL.

    return false;
}//ChangeResolution()


///////////////////////////////////////////////////

//Zoom Functions///////////////////////////////////
bool ClientUI::ZoomToPlanet(int id)
{
    // this just zooms to the appropriate system, until we create a planet window of some kind
    if (Planet* planet = dynamic_cast<Planet*>(GetUniverse().Object(id))) {
        ZoomToSystem(planet->GetSystem());
        return true;
    }
    return false;
}

bool ClientUI::ZoomToSystem(int id)
{
    if (System* system = dynamic_cast<System*>(GetUniverse().Object(id))) {
        ZoomToSystem(system);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToFleet(int id)
{
    if (Fleet* fleet = dynamic_cast<Fleet*>(GetUniverse().Object(id))) {
        ZoomToFleet(fleet);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToShip(int id)
{
    // this just zooms to the appropriate fleet window, until we create a ship window of some kind
    if (Ship* ship = dynamic_cast<Ship*>(GetUniverse().Object(id))) {
        ZoomToFleet(ship->GetFleet());
        return true;
    }
    return false;
}

bool ClientUI::ZoomToTech(int id)
{ 
    // TODO: Zooming code
    
    return false;
}

bool ClientUI::ZoomToEncyclopediaEntry(const std::string& str)
{ 
    // TODO: Zooming code
    
    return false;
}

void ClientUI::ZoomToSystem(System* system)
{
    if (!system)
        return;

    m_map_wnd->CenterOnSystem(system->ID());
    m_map_wnd->SelectSystem(system->ID());
}

void ClientUI::ZoomToFleet(Fleet* fleet)
{
    if (!fleet)
        return;

    m_map_wnd->CenterOnFleet(fleet->ID());
    m_map_wnd->SelectFleet(fleet->ID());
}

/////////////////////////////////////////////////////

//Screen Functions///////////////////////////////////
void ClientUI::InitTurn( int turn_number )
{
    m_map_wnd->InitTurn( turn_number );
}
    
void ClientUI::ScreenIntro()
{
    HideAllWindows();
    
    m_state = STATE_INTRO; // set to intro screen state
    
    m_intro_screen = new IntroScreen();
    GG::App::GetApp()->Register(m_intro_screen);

}//ScreenIntro()
      

void ClientUI::ScreenProcessTurn()
{
    HideAllWindows();
    
    m_state = STATE_TURNSTART; // set to turn start
    
    m_turn_progress_wnd = new TurnProgressWnd();
    GG::App::GetApp()->Register(m_turn_progress_wnd);

}//ScreenTurnStart()

                
void ClientUI::ScreenSettings(const ClientNetworkCore &net)
{
    // TODO: modally run options dialog here on top of whatever screen(s) is(are) already active

}//ScreenSettings()

void ClientUI::ScreenEmpireSelect()
{
    // TODO: run modally

}//ScreenEmpireSelect()

void ClientUI::ScreenMap()
{
    HideAllWindows();

    // clean up previous windows, based on previous state
    switch (m_state) {
    case STATE_STARTUP:
        break;
    case STATE_INTRO:
        GG::App::GetApp()->Remove(m_intro_screen);
        delete m_intro_screen;
        m_intro_screen = 0;
        break;
    case STATE_SETTINGS:
        break;
    case STATE_EMPIRESEL:
        break;
    case STATE_TURNSTART:
        GG::App::GetApp()->Remove(m_turn_progress_wnd );
        delete m_turn_progress_wnd;
        m_turn_progress_wnd = 0;
        break;
    case STATE_MAP:
        break;
    case STATE_SITREP:
        break;
    case STATE_PROCESS:
        break;
    case STATE_BATTLE:
        break;
    case STATE_SAVE:
        break;
    case STATE_LOAD:
        break;
    case STATE_SHUTDOWN:
        break;
    default:
        break;
    }

    m_state = STATE_MAP;

    m_map_wnd->Show();

}//ScreenMap()

void ClientUI::ScreenSitrep(const std::vector<SitRepEntry> &events)
{
    ScreenMap();

    // TODO: run sitrep as an on-top window, layered over the main map

}//ScreenSitrep()

void ClientUI::ScreenBattle(Combat* combat)
{
    // TODO: run battle screen by iteself

}//ScreenBattle()

void ClientUI::ScreenSave(bool show)
{
    // TODO: modally run save dialog here on top of whatever screen(s) is(are) already active

}//ScreenSave()

void ClientUI::ScreenLoad(bool show)
{
    // TODO: modally run load dialog here on top of whatever screen(s) is(are) already active

}//ScreenLoad()

void ClientUI::MessageBox(const std::string& message)
{
    GG::ThreeButtonDlg dlg(320,200,message,FONT,PTS+2,WND_COLOR, WND_BORDER_COLOR, CTRL_COLOR, TEXT_COLOR, 1,
                           new CUIButton((320-75)/2, 170, 75, "OK"));
    dlg.Run();    
}//MessageBox()

void ClientUI::LogMessage(const std::string& msg)
{
    s_logger.debug(msg);
}

const std::string& ClientUI::String(const std::string& index)
{
    return s_the_UI->m_string_table->String(index);
}

void ClientUI::UpdateTurnProgress( const std::string& phase_str, const int empire_id )
{
  m_turn_progress_wnd->UpdateTurnProgress( phase_str, empire_id );
}


void ClientUI::GenerateSitRepText( SitRepEntry *p_sit_rep )
{
  // get template string
  std::string template_str( String( g_string_id_lut[ p_sit_rep->GetType() ] ) );

  // parse string
  p_sit_rep->GenerateVarText( template_str );

}

////////////////////////////////////////////////////
void ClientUI::HideAllWindows()
{
    if (m_intro_screen)
        m_intro_screen->Hide();
    if (m_map_wnd)
        m_map_wnd->Hide();
    if (m_turn_progress_wnd)
        m_turn_progress_wnd->Hide();
}

int ClientUI::SelectPlanet(int system_id)
{
    PlanetPicker planet_picker(system_id);
    planet_picker.Run();
    return planet_picker.PlanetPicked();
}

void ClientUI::SetCursor( Cursor new_cursor_type )
{
    SDL_Cursor *new_cursor;
    SDL_Cursor *old_cursor;

    if ( new_cursor_type == CURSOR_DEFAULT )
    {
        /* if m_default_cursor is not assigned, all we've ever had is the default, do nothing */
        if ( m_default_cursor )
        {
            SDL_SetCursor( m_default_cursor );
        }
        return;
    }

    /* save default cursor if not assigned yet */
    if ( !m_default_cursor )
    {
        m_default_cursor = SDL_GetCursor( );
    }

    /* If there are a lot of cursors, eventually we may want to pre-create them or at least */
    /* cache them once created */
    switch ( new_cursor_type )
    {
        case CURSOR_COLONIZE:

            /* create colonize cursor */
            new_cursor = createColonizeCursor( );
            break;

        default:
            throw std::invalid_argument("Invalid cursor type sent to SetCursor()");
            break;
    }

    /* get current cursor*/
    old_cursor = SDL_GetCursor( );

    /* set new one */
    SDL_SetCursor( new_cursor );

    /* delete old if not the default */
    if ( old_cursor != m_default_cursor )
    {
        SDL_FreeCursor( old_cursor );
    }
}
