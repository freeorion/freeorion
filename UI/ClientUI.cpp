#include "ClientUI.h"

#include "../util/AppInterface.h"
#include "../universe/Building.h"
#include "CUIControls.h"
#include "../universe/Fleet.h"
#include "FleetWnd.h"
#include "IntroScreen.h"
#include "MapWnd.h"
#include "SidePanel.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Ship.h"
#include "TurnProgressWnd.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"

#include <GG/GUI.h>
#include <GG/Clr.h>
#include <GG/DrawUtil.h>
#include <GG/dialogs/ThreeButtonDlg.h>

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/spirit.hpp>

#include <string>

const Tech* GetTech(const std::string& name);

namespace fs = boost::filesystem;

// static members
fs::path ClientUI::ArtDir()                    { return GetSettingsDir() / "data" / "art"; }
fs::path ClientUI::SoundDir()                  { return GetSettingsDir() / "data" / "sound"; }

std::string ClientUI::Font()                   { return (GetGlobalDir() / GetOptionsDB().Get<std::string>("UI.font")).native_file_string(); }
std::string ClientUI::FontBold()               { return (GetGlobalDir() / GetOptionsDB().Get<std::string>("UI.font-bold")).native_file_string(); }
std::string ClientUI::FontItalic()             { return (GetGlobalDir() / GetOptionsDB().Get<std::string>("UI.font-italic")).native_file_string(); }
std::string ClientUI::FontBoldItalic()         { return (GetGlobalDir() / GetOptionsDB().Get<std::string>("UI.font-bold-italic")).native_file_string(); }
std::string ClientUI::TitleFont()              { return (GetGlobalDir() / GetOptionsDB().Get<std::string>("UI.title-font")).native_file_string(); }

int         ClientUI::Pts()                    { return GetOptionsDB().Get<int>("UI.font-size"); }
int         ClientUI::TitlePts()               { return GetOptionsDB().Get<int>("UI.title-font-size"); }

GG::Clr     ClientUI::TextColor()              { return GetOptionsDB().Get<StreamableColor>("UI.text-color").ToClr(); }

// windows
GG::Clr     ClientUI::WndColor()               { return GetOptionsDB().Get<StreamableColor>("UI.wnd-color").ToClr(); }
GG::Clr     ClientUI::WndBorderColor()         { return GetOptionsDB().Get<StreamableColor>("UI.wnd-border-color").ToClr(); }
GG::Clr     ClientUI::WndOuterBorderColor()    { return GetOptionsDB().Get<StreamableColor>("UI.wnd-outer-border-color").ToClr(); }
GG::Clr     ClientUI::WndInnerBorderColor()    { return GetOptionsDB().Get<StreamableColor>("UI.wnd-inner-border-color").ToClr(); }

// controls
GG::Clr     ClientUI::CtrlColor()              { return GetOptionsDB().Get<StreamableColor>("UI.ctrl-color").ToClr(); }
GG::Clr     ClientUI::CtrlBorderColor()        { return GetOptionsDB().Get<StreamableColor>("UI.ctrl-border-color").ToClr(); }

GG::Clr     ClientUI::ButtonColor()            { return GetOptionsDB().Get<StreamableColor>("UI.wnd-border-color").ToClr(); }

GG::Clr     ClientUI::StateButtonColor()       { return GetOptionsDB().Get<StreamableColor>("UI.state-button-color").ToClr(); }

GG::Clr     ClientUI::ScrollTabColor()         { return GetOptionsDB().Get<StreamableColor>("UI.scroll-tab-color").ToClr(); }
int         ClientUI::ScrollWidth()            { return GetOptionsDB().Get<int>("UI.scroll-width"); }

GG::Clr     ClientUI::DropDownListIntColor()   { return GetOptionsDB().Get<StreamableColor>("UI.dropdownlist-interior-color").ToClr(); }
GG::Clr     ClientUI::DropDownListArrowColor() { return GetOptionsDB().Get<StreamableColor>("UI.dropdownlist-arrow-color").ToClr(); }

GG::Clr     ClientUI::EditHiliteColor()        { return GetOptionsDB().Get<StreamableColor>("UI.edit-hilite").ToClr(); }
GG::Clr     ClientUI::EditIntColor()           { return GetOptionsDB().Get<StreamableColor>("UI.edit-interior").ToClr(); }
GG::Clr     ClientUI::MultieditIntColor()      { return GetOptionsDB().Get<StreamableColor>("UI.multiedit-interior").ToClr(); }

GG::Clr     ClientUI::StatIncrColor()          { return GetOptionsDB().Get<StreamableColor>("UI.stat-increase-color").ToClr(); }
GG::Clr     ClientUI::StatDecrColor()          { return GetOptionsDB().Get<StreamableColor>("UI.stat-decrease-color").ToClr(); }

int         ClientUI::SystemIconSize()         { return GetOptionsDB().Get<int>("UI.system-icon-size"); }
double      ClientUI::FleetButtonSize()        { return GetOptionsDB().Get<double>("UI.fleet-button-size"); }

// game UI windows
GG::Clr     ClientUI::SidePanelColor()         { return GetOptionsDB().Get<StreamableColor>("UI.sidepanel-color").ToClr(); }

// tech screen
GG::Clr     ClientUI::KnownTechFillColor()                   { return GetOptionsDB().Get<StreamableColor>("UI.known-tech").ToClr(); }
GG::Clr     ClientUI::KnownTechTextAndBorderColor()          { return GetOptionsDB().Get<StreamableColor>("UI.known-tech-border").ToClr(); }
GG::Clr     ClientUI::ResearchableTechFillColor()            { return GetOptionsDB().Get<StreamableColor>("UI.researchable-tech").ToClr(); }
GG::Clr     ClientUI::ResearchableTechTextAndBorderColor()   { return GetOptionsDB().Get<StreamableColor>("UI.researchable-tech-border").ToClr(); }
GG::Clr     ClientUI::UnresearchableTechFillColor()          { return GetOptionsDB().Get<StreamableColor>("UI.unresearchable-tech").ToClr(); }
GG::Clr     ClientUI::UnresearchableTechTextAndBorderColor() { return GetOptionsDB().Get<StreamableColor>("UI.unresearchable-tech-border").ToClr(); }
GG::Clr     ClientUI::TechWndProgressBarBackground()         { return GetOptionsDB().Get<StreamableColor>("UI.tech-progress-background").ToClr(); }
GG::Clr     ClientUI::TechWndProgressBar()                   { return GetOptionsDB().Get<StreamableColor>("UI.tech-progress").ToClr(); }


// private static members
log4cpp::Category& ClientUI::s_logger(log4cpp::Category::getRoot());
ClientUI* ClientUI::s_the_UI = 0;

namespace {
    // an internal LUT of string IDs for each SitRep type
    // It's in this module becaue SitReps know nothing about how they
    // should be rendered - this is up to the client UI
    const char* g_string_id_lut[ SitRepEntry::NUM_SITREP_TYPES ] =
    {
        "SITREP_BASE_BUILT",
        "SITREP_SHIP_BUILT",
        "SITREP_BUILDING_BUILT",
        "SITREP_TECH_RESEARCHED",
        "SITREP_COMBAT_SYSTEM_WON",
        "SITREP_COMBAT_SYSTEM_LOST",
        "SITREP_COMBAT_SYSTEM_NO_VICTOR",
        "SITREP_PLANET_LOST_STARVED_TO_DEATH",
        "SITREP_PLANET_COLONIZED",
        "SITREP_FLEET_ARRIVED_AT_DESTINATION"
    };
    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add("app-width", "Sets horizontal app resolution.", 1024, RangedValidator<int>(800, 2048));
        db.Add("app-height", "Sets vertical app resolution.", 768, RangedValidator<int>(600, 1536));
        db.Add('c', "color-depth", "Sets screen color depth, in bits per pixel.", 32, RangedStepValidator<int>(8, 16, 32));

        // sound
        db.Add("UI.sound.enabled", "Toggles UI sound effects on or off.", true, Validator<bool>());
        db.Add("UI.sound.volume", "The volume (0 to 255) at which UI sound effects should be played.", 255, RangedValidator<int>(0, 255));
        db.Add<std::string>("UI.sound.button-rollover", "The sound file played when the mouse moves over a button.", "button_rollover.wav");
        db.Add<std::string>("UI.sound.button-click", "The sound file played when a button is clicked.", "button_click.wav");
        db.Add<std::string>("UI.sound.turn-button-click", "The sound file played when the turn button is clicked.", "turn_button_click.wav");
        db.Add<std::string>("UI.sound.list-select", "The sound file played when a listbox or drop-down list item is selected.", "list_select.wav");
        db.Add<std::string>("UI.sound.item-drop", "The sound file played when an item is dropped into a listbox.", "item_drop.wav");
        db.Add<std::string>("UI.sound.list-pulldown", "The sound file played when the list is opened in a drop-down list.", "list_pulldown.wav");
        db.Add<std::string>("UI.sound.text-typing", "The sound file played when the user types text.", "text_typing.wav");
        db.Add<std::string>("UI.sound.window-maximize", "The sound file played when a window is maximized.", "window_maximize.wav");
        db.Add<std::string>("UI.sound.window-minimize", "The sound file played when a window is minimized.", "window_minimize.wav");
        db.Add<std::string>("UI.sound.window-close", "The sound file played when a window is closed.", "window_close.wav");
        db.Add<std::string>("UI.sound.alert", "The sound file played when an error or illegal action occurs.", "alert.wav");
        db.Add<std::string>("UI.sound.planet-button-click", "The sound file played when a planet button is clicked.", "button_click.wav");
        db.Add<std::string>("UI.sound.fleet-button-rollover", "The sound file played when mouse moves over a fleet button.", "fleet_button_rollover.wav");
        db.Add<std::string>("UI.sound.fleet-button-click", "The sound file played when a fleet button is clicked.", "fleet_button_click.wav");
        db.Add<std::string>("UI.sound.sidepanel-open", "The sound file played when the system side-panel is opened.", "sidepanel_open.wav");
        db.Add<std::string>("UI.sound.farming-focus", "The sound file played when a farming focus button is clicked.", "farm_select.wav");
        db.Add<std::string>("UI.sound.industry-focus", "The sound file played when an industry focus button is clicked.", "industry_select.wav");
        db.Add<std::string>("UI.sound.research-focus", "The sound file played when a research focus button is clicked.", "research_select.wav");
        db.Add<std::string>("UI.sound.mining-focus", "The sound file played when a mining focus button is clicked.", "mining_select.wav");
        // TODO: uncomment when trade is added to side panel
        //db.Add<std::string>("UI.sound.trade-focus", "The sound file played when a trade focus button is clicked.", "trade_select.wav");
        db.Add<std::string>("UI.sound.balanced-focus", "The sound file played when a balanced focus button is clicked.", "balanced_select.wav");

        // fonts
        db.Add<std::string>("UI.font", "Sets UI font resource file.", "DejaVuSans.ttf");
        db.Add<std::string>("UI.font-bold", "Sets UI bold font resource file.", "DejaVuSans-Bold.ttf");
        db.Add<std::string>("UI.font-italic", "Sets UI bold font resource file.", "DejaVuSans-Oblique.ttf");
        db.Add<std::string>("UI.font-bold-italic", "Sets UI bold font resource file.", "DejaVuSans-BoldOblique.ttf");
        db.Add("UI.font-size", "Sets UI font size.", 12, RangedValidator<int>(4, 40));
        db.Add<std::string>("UI.title-font", "Sets UI title font resource file.", "DejaVuSans.ttf");
        db.Add("UI.title-font-size", "Sets UI title font size.", 12, RangedValidator<int>(4, 40));

        // colors
        db.Add("UI.wnd-color", "Sets UI window color.", StreamableColor(GG::Clr(0, 0, 0, 210)), Validator<StreamableColor>());
        db.Add("UI.text-color", "Sets UI text color.", StreamableColor(GG::Clr(255, 255, 255, 255)), Validator<StreamableColor>());
        db.Add("UI.ctrl-color", "Sets UI control color.", StreamableColor(GG::Clr(30, 30, 30, 255)), Validator<StreamableColor>());
        db.Add("UI.ctrl-border-color", "Sets UI control border color.", StreamableColor(GG::Clr(124, 124, 124, 255)), Validator<StreamableColor>());
        db.Add("UI.button-color", "Sets UI button color.", StreamableColor(GG::Clr(0, 0, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.state-button-color", "Sets UI state button color.", StreamableColor(GG::Clr(0, 127, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.scroll-tab-color", "Sets UI scroll tab color.", StreamableColor(GG::Clr(60, 60, 60, 255)), Validator<StreamableColor>());
        db.Add("UI.dropdownlist-interior-color", "Sets UI drop-down-list interior color.", StreamableColor(GG::Clr(0, 0, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.dropdownlist-arrow-color", "Sets UI drop-down-list arrow color.", StreamableColor(GG::Clr(130, 130, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.edit-hilite", "Sets color of hilighting in UI edit controls.", StreamableColor(GG::Clr(43, 81, 102, 255)), Validator<StreamableColor>());
        db.Add("UI.edit-interior", "Sets UI edit controls' interior color.", StreamableColor(GG::Clr(0, 0, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.multiedit-interior", "Sets UI multi-line edit controls' interior color.", StreamableColor(GG::Clr(0, 0, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.stat-increase-color", "Sets the color of increased stats in the UI.", StreamableColor(GG::Clr(127, 255, 127, 255)), Validator<StreamableColor>());
        db.Add("UI.stat-decrease-color", "Sets the color of decreased stats in the UI.", StreamableColor(GG::Clr(255, 127, 127, 255)), Validator<StreamableColor>());
        db.Add("UI.sidepanel-color", "Sets the color of the side-panel.", StreamableColor(GG::Clr(0, 0, 0, 220)), Validator<StreamableColor>());
        db.Add("UI.wnd-outer-border-color", "Sets UI outer border color.", StreamableColor(GG::Clr(64, 64, 64, 255)), Validator<StreamableColor>());
        db.Add("UI.wnd-border-color", "Sets UI border color.", StreamableColor(GG::Clr(0, 0, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.wnd-inner-border-color", "Sets UI inner border color.", StreamableColor(GG::Clr(255, 255, 255, 255)), Validator<StreamableColor>());
        db.Add("UI.known-tech", "Sets color of known techs in the tech tree.", StreamableColor(GG::Clr(72, 72, 72, 255)), Validator<StreamableColor>());
        db.Add("UI.known-tech-border", "Sets text and border color of known techs in the tech tree.", StreamableColor(GG::Clr(164, 164, 164, 255)), Validator<StreamableColor>());
        db.Add("UI.researchable-tech", "Sets color of researchable techs in the tech tree.", StreamableColor(GG::Clr(48, 48, 48, 255)), Validator<StreamableColor>());
        db.Add("UI.researchable-tech-border", "Sets text and border color of researchable techs in the tech tree.", StreamableColor(GG::Clr(164, 164, 164, 255)), Validator<StreamableColor>());
        db.Add("UI.unresearchable-tech", "Sets color of unresearchable techs in the tech tree.", StreamableColor(GG::Clr(30, 30, 30, 255)), Validator<StreamableColor>());
        db.Add("UI.unresearchable-tech-border", "Sets text and border color of unresearchable techs in the tech tree.", StreamableColor(GG::Clr(86, 86, 86, 255)), Validator<StreamableColor>());
        db.Add("UI.tech-progress-background", "Sets background color of progress bars in the tech tree.", StreamableColor(GG::Clr(72, 72, 72, 255)), Validator<StreamableColor>());
        db.Add("UI.tech-progress", "Sets color of progress bars in the tech tree.", StreamableColor(GG::Clr(40, 40, 40, 255)), Validator<StreamableColor>());

        // misc
        db.Add("UI.scroll-width", "Sets UI scroll width.", 14, RangedValidator<int>(8, 30));
        db.Add("UI.system-icon-size", "Sets size of system icons.", 14, RangedValidator<int>(8, 50));
        db.Add("UI.fleet-button-size", "Sets size of fleet buttons.", 0.5, RangedValidator<double>(0.2, 2));

        // tech category colors
        const GG::Clr LEARNING_CATEGORY(93, 155, 246, 255);
        const GG::Clr GROWTH_CATEGORY(116, 225, 107, 255);
        const GG::Clr PRODUCTION_CATEGORY(240, 106, 106, 255);
        const GG::Clr CONSTRUCTION_CATEGORY(241, 233, 87, 255);
        const GG::Clr ECONOMICS_CATEGORY(255, 112, 247, 255);
        db.Add("UI.tech-category-1", "Sets color of the first category of techs defined in techs.xml.", StreamableColor(LEARNING_CATEGORY), Validator<StreamableColor>());
        db.Add("UI.tech-category-2", "Sets color of the second category of techs defined in techs.xml.", StreamableColor(GROWTH_CATEGORY), Validator<StreamableColor>());
        db.Add("UI.tech-category-3", "Sets color of the third category of techs defined in techs.xml.", StreamableColor(PRODUCTION_CATEGORY), Validator<StreamableColor>());
        db.Add("UI.tech-category-4", "Sets color of the fourth category of techs defined in techs.xml.", StreamableColor(CONSTRUCTION_CATEGORY), Validator<StreamableColor>());
        db.Add("UI.tech-category-5", "Sets color of the fifth category of techs defined in techs.xml.", StreamableColor(ECONOMICS_CATEGORY), Validator<StreamableColor>());

        // UI behavior
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

}


////////////////////////////////////////////////
// ClientUI
////////////////////////////////////////////////
//Init and Cleanup//////////////////////////////////////
ClientUI::ClientUI() :
    m_state(STATE_STARTUP),
    m_intro_screen(0),
    m_map_wnd(0),
    m_turn_progress_wnd(0),
    m_previously_shown_system(UniverseObject::INVALID_OBJECT_ID)
{
    s_the_UI = this;
    Initialize();
}

bool ClientUI::Initialize()
{
    //initialize UI state & window
    m_state = STATE_STARTUP;

#ifndef FREEORION_BUILD_UTIL
    m_map_wnd = new MapWnd();
    GG::GUI::GetGUI()->Register(m_map_wnd);
    m_map_wnd->Hide();
#endif

    return true;
}

ClientUI::~ClientUI() 
{
    Cleanup();
}

bool ClientUI::Cleanup()
{
#ifndef FREEORION_BUILD_UTIL
    delete m_intro_screen;
    m_intro_screen = 0;

    delete m_map_wnd;
    m_map_wnd = 0;

    delete m_turn_progress_wnd;
    m_turn_progress_wnd = 0;
#endif

    s_the_UI = 0;

    return true; 
}

XMLElement ClientUI::SaveGameData() const
{
    XMLElement retval("UI");
#ifndef FREEORION_BUILD_UTIL
    retval.AppendChild(m_map_wnd->SaveGameData());
#endif
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

bool ClientUI::ZoomToTech(const std::string& tech_name)
{
#ifndef FREEORION_BUILD_UTIL
    if (!GetTech(tech_name))
        return false;
    m_map_wnd->ShowTech(tech_name);
    return true;
#else
    return false;
#endif
}

bool ClientUI::ZoomToBuildingType(const std::string& building_type_name)
{
#ifndef FREEORION_BUILD_UTIL
    if (!GetBuildingType(building_type_name))
        return false;
    // TODO
    return true;
#else
    return false;
#endif
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

#ifndef FREEORION_BUILD_UTIL
    m_map_wnd->CenterOnSystem(system->ID());
    m_map_wnd->SelectSystem(system->ID());
#endif
}

void ClientUI::ZoomToFleet(Fleet* fleet)
{
    if (!fleet)
        return;

#ifndef FREEORION_BUILD_UTIL
    m_map_wnd->CenterOnFleet(fleet->ID());
    m_map_wnd->SelectFleet(fleet->ID());
    for (FleetWnd::FleetWndItr it = FleetWnd::FleetWndBegin(); it != FleetWnd::FleetWndEnd(); ++it) {
        if ((*it)->ContainsFleet(fleet->ID())) {
            (*it)->SelectFleet(fleet);
            break;
        }
    }
#endif
}

/////////////////////////////////////////////////////

//Screen Functions///////////////////////////////////
void ClientUI::InitTurn(int turn_number)
{
#ifndef FREEORION_BUILD_UTIL
    m_map_wnd->InitTurn(turn_number);
#endif
}

void ClientUI::RestoreFromSaveData(const XMLElement& elem)
{
#ifndef FREEORION_BUILD_UTIL
    m_map_wnd->RestoreFromSaveData(elem.Child("MapWnd"));
#endif
}

void ClientUI::SwitchState(State state)
{
#ifndef FREEORION_BUILD_UTIL
    HideAllWindows();
    // clean up previous windows, based on previous state
    switch (m_state) {
    case STATE_STARTUP:
        break;
    case STATE_INTRO:
        // when loading a game or starting a new game, defer removal of the intro screen until the new game has actually
        // started, since either operation may fail due to the server
        if (state != STATE_NEW_GAME && state != STATE_LOAD) {
            delete m_intro_screen;
            m_intro_screen = 0;
        }
        break;
    case STATE_TURNSTART:
        delete m_turn_progress_wnd;
        m_turn_progress_wnd = 0;
        break;
    case STATE_MAP:
        if (state != STATE_LOAD)
            m_previously_shown_system = m_map_wnd->GetSidePanel()->SystemID();
        //hide sidepanel
        m_map_wnd->SelectSystem(UniverseObject::INVALID_OBJECT_ID);
        break;
    case STATE_COMBAT:
        break;
    case STATE_NEW_GAME:
    case STATE_LOAD:
        if (m_intro_screen) {
            delete m_intro_screen;
            m_intro_screen = 0;
        }
        delete m_turn_progress_wnd;
        m_turn_progress_wnd = 0;
        break;
    default:
        break;
    }

    switch (m_state = state) {
    case STATE_STARTUP:
        m_previously_shown_system = UniverseObject::INVALID_OBJECT_ID;
        break;
    case STATE_INTRO:
        m_previously_shown_system = UniverseObject::INVALID_OBJECT_ID;
        if (!m_intro_screen) {
            m_intro_screen = new IntroScreen();
            GG::GUI::GetGUI()->Register(m_intro_screen);
        }
        m_intro_screen->Show();
        break;
    case STATE_TURNSTART:
        if (!m_turn_progress_wnd) {
            m_turn_progress_wnd = new TurnProgressWnd();
            GG::GUI::GetGUI()->Register(m_turn_progress_wnd);
        }
        m_turn_progress_wnd->Show();
        break;
    case STATE_MAP:
        m_map_wnd->Show();
        m_map_wnd->SelectSystem(m_previously_shown_system);
        break;
    case STATE_COMBAT:
        break;
    case STATE_NEW_GAME:
    case STATE_LOAD:
        m_map_wnd->Sanitize();
        if (!m_turn_progress_wnd) {
            m_turn_progress_wnd = new TurnProgressWnd();
            GG::GUI::GetGUI()->Register(m_turn_progress_wnd);
        }
        m_turn_progress_wnd->UpdateTurnProgress(UserString(m_state == STATE_NEW_GAME ? "NEW_GAME" : "LOADING"), -1);
        m_turn_progress_wnd->Show();
        break;
    default:
        break;
    }
#endif
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

void ClientUI::UpdateTurnProgress(const std::string& phase_str, const int empire_id)
{
#ifndef FREEORION_BUILD_UTIL
    m_turn_progress_wnd->UpdateTurnProgress(phase_str, empire_id);
#endif
}

void ClientUI::UpdateCombatTurnProgress(const std::string& msg)
{
#ifndef FREEORION_BUILD_UTIL
    m_turn_progress_wnd->UpdateCombatTurnProgress(msg);
#endif
}

void ClientUI::ScreenSitrep(const std::vector<SitRepEntry> &events)
{
    ScreenMap();
}

void ClientUI::ScreenNewGame()
{
    SwitchState(STATE_NEW_GAME);
}

void ClientUI::ScreenLoad()
{
    SwitchState(STATE_LOAD);
}

void ClientUI::MessageBox(const std::string& message, bool play_alert_sound/* = false*/)
{
    GG::ThreeButtonDlg dlg(320,200,message,GG::GUI::GetGUI()->GetFont(Font(),Pts()+2),WndColor(), WndBorderColor(), CtrlColor(), TextColor(), 1,
                           UserString("OK"));
#ifndef FREEORION_BUILD_UTIL
    if (play_alert_sound && GetOptionsDB().Get<bool>("UI.sound.enabled"))
        HumanClientApp::GetApp()->PlaySound(SoundDir() / "alert.wav");
#endif
    dlg.Run();
}

void ClientUI::LogMessage(const std::string& msg)
{
    s_logger.debug(msg);
}

void ClientUI::GenerateSitRepText(SitRepEntry *p_sit_rep)
{
    // get template string
    std::string template_str(UserString(g_string_id_lut[p_sit_rep->GetType()]));

    // parse string
    p_sit_rep->GenerateVarText( template_str );
}

boost::shared_ptr<GG::Texture> ClientUI::GetTexture(const boost::filesystem::path& path, bool mipmap/* = false*/)
{
    try {
        return HumanClientApp::GetApp()->GetTexture(path.native_file_string(), mipmap);
    } catch(...) {
        return HumanClientApp::GetApp()->GetTexture((ClientUI::ArtDir() / "misc" / "missing.png").native_file_string(), mipmap);
    }
}

boost::shared_ptr<GG::Texture>
ClientUI::GetNumberedTexture(const std::string& dir_name, const std::map<int, std::string>& types_to_names, 
                             int type, int hash_key)
{
    using boost::lexical_cast;
    using std::string;

    static std::map<int, std::pair<string, int> > image_names;

    if (image_names.empty()) {
        for (std::map<int, std::string>::const_iterator it = types_to_names.begin(); it != types_to_names.end(); ++it) {
            image_names[it->first].first = it->second;
        }

        fs::path star_dir(ClientUI::ArtDir() / dir_name);
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

    int star_variant = image_names[type].second ? (hash_key % image_names[type].second) : 0;
    fs::path path = ClientUI::ArtDir() / "stars" /
        (image_names[type].first + lexical_cast<string>(star_variant + 1) + ".png");
    return GetTexture(path);
}

////////////////////////////////////////////////////
void ClientUI::HideAllWindows()
{
#ifndef FREEORION_BUILD_UTIL
    if (m_intro_screen)
        m_intro_screen->Hide();
    if (m_map_wnd)
        m_map_wnd->Hide();
    if (m_turn_progress_wnd)
        m_turn_progress_wnd->Hide();
#endif
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

StreamableColor::StreamableColor() :
    r(0),
    g(0),
    b(0),
    a(0)    
{}

StreamableColor::StreamableColor(const GG::Clr& clr) :
    r(clr.r),
    g(clr.g),
    b(clr.b),
    a(clr.a)    
{}

GG::Clr StreamableColor::ToClr() const
{
    return GG::Clr(r, g, b, a);
}

std::ostream& operator<<(std::ostream& os, const StreamableColor& clr)
{
    os << "(" << clr.r << "," << clr.g << "," << clr.b << "," << clr.a << ")";
    return os;
}

std::istream& operator>>(std::istream& is, StreamableColor& clr)
{
    using namespace boost::spirit;
    rule<> color_p =
        ch_p('(') >> *space_p >>
        int_p[assign(clr.r)] >> *space_p >> ',' >> *space_p >>
        int_p[assign(clr.g)] >> *space_p >> ',' >> *space_p >>
        int_p[assign(clr.b)] >> *space_p >> ',' >> *space_p >>
        int_p[assign(clr.a)] >> *space_p >> ')';
    std::string str;
    char c;
    do {
        is >> c;
        str += c;
    } while (is && c != ')');
    if (!parse(str.c_str(), color_p).full ||
        clr.r < 0 || 255 < clr.r ||
        clr.g < 0 || 255 < clr.g ||
        clr.b < 0 || 255 < clr.b ||
        clr.a < 0 || 255 < clr.a)
        is.setstate(std::ios_base::failbit);
    return is;
}

