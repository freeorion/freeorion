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
#include "ToolWnd.h"
#include "ToolContainer.h"
#include "TurnProgressWnd.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <string>
#include <fstream>


//static members
std::string ClientUI::FONT          = "Vera.ttf";
std::string ClientUI::FONT_BOLD     = "VeraBd.ttf";
std::string ClientUI::FONT_ITALIC   = "VeraIt.ttf";
std::string ClientUI::FONT_BOLD_ITALIC= "VeraBI.ttf";
int         ClientUI::PTS           = 12;
std::string ClientUI::TITLE_FONT    = "Vera.ttf";
int         ClientUI::TITLE_PTS     = 12;

std::string ClientUI::DIR           = "default/";
std::string ClientUI::ART_DIR       = "default/data/art/";
std::string ClientUI::SOUND_DIR	    = "default/data/sound/";

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

int         ClientUI::SYSTEM_ICON_SIZE = 24;
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
        "SITREP_COMBAT_SYSTEM_NO_VICTOR",
        "PLANET_LOST_STARVED_TO_DEATH"
	};
    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add(    "app-width", "Sets horizontal app resolution.", 1024, RangedValidator<int>(640, 2048));
        db.Add(    "app-height", "Sets vertical app resolution.", 768, RangedValidator<int>(480, 1536));
        db.Add('c', "color-depth", "Sets screen color depth, in bits per pixel.", 32, RangedStepValidator<int>(8, 16, 32));

        db.Add<std::string>("art-dir", "Sets UI art resource directory.", "default/data/art/");
        db.Add<std::string>("sound-dir", "Sets UI sound and music resource directory.", "default/data/sound/");
        db.Add("UI.sound.enabled", "Toggles UI sound effects on or off.", true, Validator<bool>());
        db.Add("UI.sound.volume", "The volume (0 to 255) at which UI sound effects should be played.", 255, RangedValidator<int>(0, 255));
        db.Add<std::string>("UI.sound.button-rollover", "The sound file played when the mouse moves over a button.", "button_rollover.wav");
        db.Add<std::string>("UI.sound.button-click", "The sound file played when a button is clicked.", "button_click.wav");
        db.Add<std::string>("UI.sound.turn-button-click", "The sound file played when the turn button is clicked.", "turn_button_click.wav");
        db.Add<std::string>("UI.sound.list-select", "The sound file played when a listbox or drop-down list item is selected.", "list_select.wav");
        db.Add<std::string>("UI.sound.item-drop", "The sound file played when an item is dropped into a listbox.", "item_drop.wav");
        db.Add<std::string>("UI.sound.text-typing", "The sound file played when the user types text.", "text_typing.wav");
        db.Add<std::string>("UI.sound.window-maximize", "The sound file played when a window is maximized.", "window_maximize.wav");
        db.Add<std::string>("UI.sound.window-minimize", "The sound file played when a window is minimized.", "window_minimize.wav");
        db.Add<std::string>("UI.sound.window-close", "The sound file played when a window is closed.", "window_close.wav");
        db.Add<std::string>("UI.sound.alert", "The sound file played when an error or illegal action occurs.", "alert.wav");
        db.Add<std::string>("UI.sound.planet-button-click", "The sound file played when a planet button is clicked.", "planet_button_click.wav");
        db.Add<std::string>("UI.sound.fleet-button-rollover", "The sound file played when mouse moves over a fleet button.", "fleet_button_rollover.wav");
        db.Add<std::string>("UI.sound.fleet-button-click", "The sound file played when a fleet button is clicked.", "fleet_button_click.wav");
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

        db.Add("UI.multiple-fleet-windows", "If true, clicks on multiple fleet buttons will open multiple fleet "
               "windows at the same time.  Otherwise, opening a fleet window will close any currently-open fleet window.", 
               false);

        db.Add("UI.fleet-autoselect", "Auto-select the top fleet when a fleet window "
               "is opened.  Consider using this flag if you use UI.multiple-fleet-windows.", true);

        db.Add("UI.window-quickclose", "Close open windows such as fleet windows and the system-view side panel when you "
               "right-click on the main map.", true);
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
            m_planet_selected(-1),
            m_mapwnd_sidepanel_visible(HumanClientApp::GetUI()->GetMapWnd()->GetSidePanel()->Visible())
        {
            if(m_mapwnd_sidepanel_visible)
                HumanClientApp::GetUI()->GetMapWnd()->GetSidePanel()->Hide();

            m_side_panel->SetSystem(system_id);
            AttachChild(m_side_panel);
            for (int i = 0; i < m_side_panel->PlanetPanels(); ++i) {
                GG::Connect(m_side_panel->GetPlanetPanel(i)->PlanetImageLClickedSignal(), &PlanetPicker::PlanetClicked, this);
            }
            HumanClientApp::GetUI()->SetCursor(ClientUI::CURSOR_COLONIZE);
        }
        ~PlanetPicker()
        {
            HumanClientApp::GetUI()->SetCursor(ClientUI::CURSOR_DEFAULT);
            if(m_mapwnd_sidepanel_visible)
                HumanClientApp::GetUI()->GetMapWnd()->GetSidePanel()->Show();
        }
        int PlanetPicked() const {return m_planet_selected;}
        void LButtonUp(const GG::Pt& pt, Uint32 keys) {m_done = true;}
        void LClick(const GG::Pt& pt, Uint32 keys) {LButtonUp(pt, keys);}
        void RButtonUp(const GG::Pt& pt, Uint32 keys) {LButtonUp(pt, keys);}
        void RClick(const GG::Pt& pt, Uint32 keys) {LButtonUp(pt, keys);}

    private:
        void PlanetClicked(int planet_id) {m_planet_selected = planet_id; m_done = true;}

        SidePanel*       m_side_panel;
        int              m_planet_selected;
        bool             m_mapwnd_sidepanel_visible;
    };

    bool temp_header_bool = RecordHeaderFile(ClientUIRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


////////////////////////////////////////////////
// ClientUI
////////////////////////////////////////////////
//Init and Cleanup//////////////////////////////////////
ClientUI::ClientUI() :
    TOOLTIP_DELAY(GetOptionsDB().Get<int>("UI.tooltip-delay")), // 1 second delay for tooltips to appear
    m_tooltips(0),
    m_state(STATE_STARTUP),
    m_intro_screen(0),
    m_map_wnd(0),
    m_turn_progress_wnd(0),
    m_default_cursor(NULL),
    m_previously_shown_system(UniverseObject::INVALID_OBJECT_ID)
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
    DIR       = GetOptionsDB().Get<std::string>("settings-dir");
    if (!DIR.empty() && DIR[DIR.size() - 1] != '/')
        DIR += '/';
    FONT      = GetOptionsDB().Get<std::string>("UI.font");
    TITLE_FONT= GetOptionsDB().Get<std::string>("UI.title-font");
    ART_DIR   = GetOptionsDB().Get<std::string>("art-dir");
    if (!ART_DIR.empty() && ART_DIR[ART_DIR.size() - 1] != '/')
        ART_DIR += '/';
    SOUND_DIR   = GetOptionsDB().Get<std::string>("sound-dir");
    if (!SOUND_DIR.empty() && SOUND_DIR[SOUND_DIR.size() - 1] != '/')
        SOUND_DIR += '/';

    Initialize();
}

bool ClientUI::Initialize()
{
    //initialize Tooltip engine
    m_tooltips = new ToolContainer(TOOLTIP_DELAY);

    //initialize UI state & window
    m_state = STATE_STARTUP;

    m_map_wnd = new MapWnd();
    GG::App::GetApp()->Register(m_map_wnd);
    m_map_wnd->Hide();

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
    
    delete m_intro_screen;
    m_intro_screen = 0;

    delete m_map_wnd;
    m_map_wnd = 0;

    delete m_turn_progress_wnd;
    m_turn_progress_wnd = 0;

    s_the_UI = 0;

    return true; 
}

bool ClientUI::AttachToolWnd(GG::Wnd* parent, ToolWnd* tool)
{
    return (m_tooltips ? m_tooltips->AttachToolWnd(parent, tool) : false);
}

GG::XMLElement ClientUI::SaveGameData() const
{
    GG::XMLElement retval("UI");
    retval.AppendChild(m_map_wnd->SaveGameData());
    return retval;
}


///////////////////////////////////////////////////

//Zoom Functions///////////////////////////////////
bool ClientUI::ZoomToPlanet(int id)
{
    // this just zooms to the appropriate system, until we create a planet window of some kind
    if (Planet* planet = GetUniverse().Object<Planet>(id)) {
        ZoomToSystem(planet->GetSystem());
        return true;
    }
    return false;
}

bool ClientUI::ZoomToSystem(int id)
{
    if (System* system = GetUniverse().Object<System>(id)) {
        ZoomToSystem(system);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToFleet(int id)
{
    if (Fleet* fleet = GetUniverse().Object<Fleet>(id)) {
        ZoomToFleet(fleet);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToShip(int id)
{
    // this just zooms to the appropriate fleet window, until we create a ship window of some kind
    if (Ship* ship = GetUniverse().Object<Ship>(id)) {
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

void ClientUI::RestoreFromSaveData(const GG::XMLElement& elem)
{
    m_map_wnd->RestoreFromSaveData(elem.Child("MapWnd"));
}

void ClientUI::SwitchState(State state)
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
        if (state != STATE_LOAD)
            m_previously_shown_system = m_map_wnd->GetSidePanel()->SystemID();
        //hide sidepanel
        m_map_wnd->SelectSystem(UniverseObject::INVALID_OBJECT_ID);
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
        GG::App::GetApp()->Remove(m_turn_progress_wnd );
        delete m_turn_progress_wnd;
        m_turn_progress_wnd = 0;
        break;
    case STATE_SHUTDOWN:
        break;
    default:
        break;
    }

    switch (m_state=state) {
    case STATE_STARTUP:
        m_previously_shown_system = UniverseObject::INVALID_OBJECT_ID;
        break;
    case STATE_INTRO:
        m_previously_shown_system = UniverseObject::INVALID_OBJECT_ID;
        if(m_intro_screen==0) {
          m_intro_screen = new IntroScreen();
          GG::App::GetApp()->Register(m_intro_screen);
        }
        m_intro_screen->Show();
        break;
    case STATE_SETTINGS:
        break;
    case STATE_EMPIRESEL:
        break;
    case STATE_TURNSTART:
        if(m_turn_progress_wnd==0) {
          m_turn_progress_wnd = new TurnProgressWnd();
          GG::App::GetApp()->Register(m_turn_progress_wnd);
        }
        m_turn_progress_wnd->Show();
        break;
    case STATE_MAP:
        m_map_wnd->Show();
        m_map_wnd->SelectSystem(m_previously_shown_system);
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
        if(m_turn_progress_wnd==0) {
          m_turn_progress_wnd = new TurnProgressWnd();
          GG::App::GetApp()->Register(m_turn_progress_wnd);
        }
        m_turn_progress_wnd->UpdateTurnProgress( "Loading ...",-1);
        m_turn_progress_wnd->Show();
        break;
    case STATE_SHUTDOWN:
        break;
    default:
        break;
    }
}

void ClientUI::ScreenIntro()
{
    SwitchState(STATE_INTRO); // set to intro screen state
}

void ClientUI::ScreenProcessTurn()
{
    SwitchState(STATE_TURNSTART); // set to turn start
}

void ClientUI::ScreenMap()
{
    SwitchState(STATE_MAP);
}

void ClientUI::UpdateTurnProgress( const std::string& phase_str, const int empire_id )
{
    m_turn_progress_wnd->UpdateTurnProgress( phase_str, empire_id );
}

void ClientUI::UpdateCombatTurnProgress( const std::string& msg)
{
    m_turn_progress_wnd->UpdateCombatTurnProgress(msg);
}

void ClientUI::ScreenSitrep(const std::vector<SitRepEntry> &events)
{
    ScreenMap();
}

void ClientUI::MessageBox(const std::string& message, bool play_alert_sound/* = false*/)
{
    GG::ThreeButtonDlg dlg(320,200,message,FONT,PTS+2,WND_COLOR, WND_BORDER_COLOR, CTRL_COLOR, TEXT_COLOR, 1,
                           new CUIButton((320-75)/2, 170, 75, UserString("OK")));
    if (play_alert_sound && GetOptionsDB().Get<bool>("UI.sound.enabled"))
        HumanClientApp::GetApp()->PlaySound(SoundDir() + "alert.wav");
    dlg.Run();
}

void ClientUI::LogMessage(const std::string& msg)
{
    s_logger.debug(msg);
}

void ClientUI::GenerateSitRepText( SitRepEntry *p_sit_rep )
{
  // get template string
  std::string template_str( UserString( g_string_id_lut[ p_sit_rep->GetType() ] ) );

  // parse string
  p_sit_rep->GenerateVarText( template_str );
}

boost::shared_ptr<GG::Texture> ClientUI::GetNumberedTexture(const std::string& dir_name, const std::map<int, std::string>& types_to_names, 
                                                            int type, int hash_key)
{
    using boost::lexical_cast;
    using std::string;

    static std::map<int, std::pair<string, int> > image_names;

    if (image_names.empty()) {
        for (std::map<int, std::string>::const_iterator it = types_to_names.begin(); it != types_to_names.end(); ++it) {
            image_names[it->first].first = it->second;
        }

        namespace fs = boost::filesystem;
        fs::path star_dir = ClientUI::ART_DIR + dir_name;
        fs::directory_iterator end_it;
        for (fs::directory_iterator it(star_dir); it != end_it; ++it) {
            if (!fs::is_directory(*it)) {
                if (it->leaf().find("blue") == 0) {
                    ++image_names[STAR_BLUE].second; 
                } else if (it->leaf().find("white") == 0) {
                    ++image_names[STAR_WHITE].second; 
                } else if (it->leaf().find("yellow") == 0) {
                    ++image_names[STAR_YELLOW].second; 
                } else if (it->leaf().find("orange") == 0) {
                    ++image_names[STAR_ORANGE].second; 
                } else if (it->leaf().find("red") == 0) {
                    ++image_names[STAR_RED].second; 
                } else if (it->leaf().find("neutron") == 0) {
                    ++image_names[STAR_NEUTRON].second; 
                } else if (it->leaf().find("black") == 0) {
                    ++image_names[STAR_BLACK].second; 
                }
            }
        }
    }

    int star_variant = image_names[type].second!=0?(hash_key % image_names[type].second):0;
    std::string filename = ClientUI::ART_DIR + "stars/" + 
        image_names[type].first + lexical_cast<string>(star_variant + 1) + ".png";
    return HumanClientApp::GetApp()->GetTexture(filename);
}

const std::string& ClientUI::SoundDir()
{
    static std::string retval;
    if (retval == "") {
        retval = GetOptionsDB().Get<std::string>("settings-dir");
        if (!retval.empty() && retval[retval.size() - 1] != '/')
            retval += '/';
        retval += "data/sound/";
    }
    return retval;
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

TempUISoundDisabler::TempUISoundDisabler() :
    m_was_enabled(GetOptionsDB().Get<bool>("UI.sound.enabled"))
{
    if (m_was_enabled)
        GetOptionsDB().Set("UI.sound.enabled", false);
}

TempUISoundDisabler::~TempUISoundDisabler()
{
    if (m_was_enabled)
        GetOptionsDB().Set("UI.sound.enabled", true);
}
