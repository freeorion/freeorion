// -*- C++ -*-
#ifndef _ClientUI_h_
#define _ClientUI_h_

#include "../universe/Enums.h"
#include "../util/SitRepEntry.h"
#include "../util/Random.h"

#ifndef _GG_Wnd_h_
#include <GG/Wnd.h>
#endif

#ifndef _GG_SDLGUI_h_
#include <GG/SDL/SDLGUI.h>
#endif

#include <boost/filesystem/path.hpp>


class ClientNetworkCore;
class Combat;
class Fleet;
class IntroScreen;
class MapWnd;
class PythonConsoleWnd;
class SitRepEntry;
struct SaveGameUIData;
class System;
class Tech;
class TurnProgressWnd;
namespace GG {
    class Clr;
    class SubTexture;
}
namespace log4cpp {
    class Category;
}

//! \brief ClientUI Main Module
//!This is the main class of the ClientUI module.
//!it contains objects of many other classes and controls the
//!display of all information onscreen.
class ClientUI
{
public:
    //!Internal States
    //!These determine what screen is currently being displayed
    enum State {
        STATE_STARTUP,
        STATE_INTRO,
        STATE_TURNSTART,
        STATE_MAP,
        STATE_COMBAT,
        STATE_NEW_GAME,
        STATE_LOAD
    };

    //! \name Structors //!@{
    ClientUI();     //!< construction (calls Initialize())
    ~ClientUI();    //!< destruction (calls Cleanup())

    bool Initialize();    //!< provided to setup initial variables. 
    bool Cleanup();       //!< provided to clean up ClientUI's memory space. 
    //!@}
    
    //! \name Accessors //!@{
    const log4cpp::Category& Logger() {return s_logger;}    //!< Returns the logger associated with ClientUI

    const std::string&  Language() const;                   //!< 

    MapWnd* GetMapWnd() {return m_map_wnd;}                 //!< Returns the main map window (may be null).

    const GG::SubTexture& SitRepIcon(SitRepEntry::EntryType type) const; //!< returns the icon for this sitrep entry type; returns the default icon if \a type has no associated icon

    void GetSaveGameUIData(SaveGameUIData& data) const; //!< populates the relevant UI state that should be restored after a save-and-load cycle
    //!@}

    //! \name Mutators //!@{
    // GameCore Interface functions
    // calling these changes internal state to display the proper screen
    // and initializes and displays the screen
    // see Interface Doc for details
    // takes as a paramter the new turn number
    //!@{
    void InitTurn( int turn_number );      //!< resets all active controls to use the latest data when it has been changed at the beginning of a new turn

    void RestoreFromSaveData(const SaveGameUIData& elem); ///< restores the UI state that was saved in an earlier call to GetSaveGameUIData().
    
    void ScreenIntro();                        //!< Intro Screen
    void ScreenProcessTurn();                  //!< Turn Star Progress Splash Screen

    void ScreenMap();     //!< Universe Map Screen

    //!< Updates turn progress window
    void UpdateTurnProgress( const std::string& phase_str, const int empire_id );
    
    //!< Updates combat turn progress window
    void UpdateCombatTurnProgress( const std::string& msg);

    //! @param events vector containing all the events to be listed
    void ScreenSitrep(const std::vector<SitRepEntry>& events);    //!< Sitrep Screen

    void ScreenNewGame();    //!< New Game Screen
    void ScreenLoad();       //!< Loading Screen

    // Zooming Functions
    bool ZoomToPlanet(int id);  //!< Zooms to a particular planet on the galaxy map and opens the planet screen for that planet
    bool ZoomToSystem(int id);  //!< Zooms to a particular system on the galaxy map
    bool ZoomToFleet(int id);   //!< Zooms to a particular fleet on the galaxy map and opens the fleet window
    bool ZoomToShip(int id);    //!< Zooms to a particular ship on the galaxy map and opens its fleet and/or ship window
    bool ZoomToTech(const std::string& tech_name); //!< Opens the technology screen and presents a description of the given technology
    bool ZoomToBuildingType(const std::string& building_type_name); //!< Opens the building type screen and presents a description of the given building type
    bool ZoomToEncyclopediaEntry(const std::string& str); //!< Opens the encyclodedia screen and presents the entry for the given item
    void ZoomToSystem(System* system); //!< Zooms to a particular system on the galaxy map
    void ZoomToFleet(Fleet* fleet);    //!< Zooms to a particular fleet on the galaxy map and opens the fleet window

    /** Loads a texture at random from the set of files starting with \a prefix in directory \a dir. */
    boost::shared_ptr<GG::Texture> GetRandomTexture(const boost::filesystem::path& dir, const std::string& prefix);

    /** Loads texture \a n % N from the set of files starting with \a prefix in directory \a dir, where N is the number
        of files found in \a dir with prefix \a prefix. */
    boost::shared_ptr<GG::Texture> GetModuloTexture(const boost::filesystem::path& dir, const std::string& prefix, int n);
    //!@}
    
    static ClientUI*    GetClientUI() {return s_the_UI;}   //!< returns a pointer to the singleton ClientUI class

    /** shows a message dialog box with the given message; if \a play_alert_sound is true, and UI sound effects are
        currently enabled, the default alert sound will be played as the message box opens */
    static void MessageBox(const std::string& message, bool play_alert_sound = false);

    static void LogMessage(const std::string& msg); //!<sends a message to the logger

    static void GenerateSitRepText(SitRepEntry *sit_rep); ///< generates a SitRep string from \a sit_rep.

    /** Loads the requested texture from file \a name; mipmap textures are generated if \a mipmap is true load default
        missing.png if name isn't found. */
    static boost::shared_ptr<GG::Texture> GetTexture(const boost::filesystem::path& path, bool mipmap = false);

    //!@{
    static boost::filesystem::path ArtDir();   //!< directory holding artwork
    static boost::filesystem::path SoundDir(); //!< directory holding sound and music

    static std::string Font();            //!< The default font to use
    static std::string FontBold();        //!< The default bold font to use
    static std::string FontItalic();      //!< The default italic font to use
    static std::string FontBoldItalic();  //!< The default bold and italic font to use
    static int         Pts();             //!< default point size
    static std::string TitleFont();       //!< The default font to use for the window title
    static int         TitlePts();        //!< default point size to use for window title
    
    static GG::Clr     TextColor();       //!< color of UI text
    
    // generic UI windows
    static GG::Clr     WndColor();            //!< color of a UI window
    static GG::Clr     WndBorderColor();      //!< color of window borders
    static GG::Clr     WndOuterBorderColor(); //!< color of the outermost border
    static GG::Clr     WndInnerBorderColor(); //!< color of the innermost border

    // controls
    static GG::Clr     CtrlColor();           //!< color of UI controls
    static GG::Clr     CtrlBorderColor();

    static GG::Clr     ButtonColor();

    static GG::Clr     StateButtonColor();

    static GG::Clr     ScrollTabColor();
    static  int        ScrollWidth();
    
    static GG::Clr     DropDownListIntColor();
    static GG::Clr     DropDownListArrowColor();

    static GG::Clr     EditHiliteColor();
    static GG::Clr     EditIntColor();
    static GG::Clr     MultieditIntColor();

    static GG::Clr     StatIncrColor();   //!< used to color increasing stats text (eg "+2")
    static GG::Clr     StatDecrColor();   //!< used to color decreasing stats text (eg "-3")

    static int         SystemIconSize();                //!< the width/height of a System/Icon at zoom = 1.0
    static double      FleetButtonSize();               //!< the width/height of a FleetButton, relative to the size of a SystemIcon
    static double      SystemSelectionIndicatorSize();  //!< the width/height of a System Selection Indicator, relative to the size of a SystemIcon

    // game UI windows
    static GG::Clr     SidePanelColor();

    // tech screen
    static GG::Clr     KnownTechFillColor();
    static GG::Clr     KnownTechTextAndBorderColor();
    static GG::Clr     ResearchableTechFillColor();
    static GG::Clr     ResearchableTechTextAndBorderColor();
    static GG::Clr     UnresearchableTechFillColor();
    static GG::Clr     UnresearchableTechTextAndBorderColor();
    static GG::Clr     TechWndProgressBarBackground();
    static GG::Clr     TechWndProgressBar();

    static GG::Clr     CategoryColor(const std::string& category_name);

    static std::map<StarType, std::string>& StarTypeFilePrefixes();
    static std::map<StarType, std::string>& HaloStarTypeFilePrefixes();
    //!@}

private:
    void HideAllWindows();              //!< hides all the UI windows from view
    
    void SwitchState(State state);      //!< switch current state to \a state, free's last state window and create the one for the new state

    typedef std::pair<std::vector<boost::shared_ptr<GG::Texture> >, boost::shared_ptr<SmallIntDistType> > TexturesAndDist;
    typedef std::map<std::string, TexturesAndDist> PrefixedTextures;
    TexturesAndDist PrefixedTexturesAndDist(const boost::filesystem::path& dir, const std::string& prefix);

    State m_state;                      //!< represents the screen currently being displayed

    IntroScreen*     m_intro_screen;      //!< the intro (and main menu) screen first showed when the game starts up
    MapWnd*          m_map_wnd;           //!< the galaxy map
    PythonConsoleWnd*m_python_console;    //!< the python console
    TurnProgressWnd* m_turn_progress_wnd; //!< the turn progress window

    int              m_previously_shown_system; //!< the ID of the system that was shown in the sidepanel before the last call to HideAllWindows()

    PrefixedTextures m_prefixed_textures;

    static log4cpp::Category& s_logger; //!< log4cpp logging category
    static ClientUI* s_the_UI;          //!< pointer to the one and only ClientUI object
};

/** temporarily disables UI sound effects, saving the old state (on or off), for later restoration upon object destruction.  TempSoundDisablers
    should be created at the beginning of any function in which Controls that emit sounds are to be programmatically altered, e.g. the
    ctor of a window class that contains a ListBox with an initially-selected item.  If this were not done, the list-select sound would be
    played when the window was constructed, which would make the sound seem to be malfunctioning. */
struct TempUISoundDisabler
{
    TempUISoundDisabler();
    ~TempUISoundDisabler();

private:
    bool m_was_enabled;
};

/** This exists as a way of allowing UI colors to be specified on the command line with one option and "(r,g,b,a)", instead of one option per color
    channel.  GG::Clr is not streamable using the normal means, due to what appears to be a bug in MSVC 7.1. */
struct StreamableColor
{
    StreamableColor();
    StreamableColor(const GG::Clr& clr);
    GG::Clr ToClr() const;
    int r, g, b, a;
};
std::ostream& operator<<(std::ostream& os, const StreamableColor& clr);
std::istream& operator>>(std::istream& is, StreamableColor& clr);


#endif // _ClientUI_h_
