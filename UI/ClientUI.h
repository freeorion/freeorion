// -*- C++ -*-
//ClientUI.h
#ifndef _ClientUI_h_
#define _ClientUI_h_

#ifndef _SitRepEntry_h_
#include "../util/SitRepEntry.h"
#endif

#ifndef _GG_Wnd_h_
#include <GG/Wnd.h>
#endif

#ifndef _GG_SDLGUI_h_
#include <GG/SDL/SDLGUI.h>
#endif

class ClientEmpire;
class ClientNetworkCore;
class ClientUniverse;
class Combat;
class Fleet;
class IntroScreen;
class MapWnd;
class PythonConsoleWnd;
class SitRepEntry;
class System;
class Tech;
class TurnProgressWnd;
class XMLElement;
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
//!display of all information oTURN_PROGRESS_WND_COLORnscreen.
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

    XMLElement SaveGameData() const; //!< returns the relevant data that should be restored after a save-and-load cycle
    //!@}
    
    //! \name Mutators //!@{
    // GameCore Interface functions
    // calling these changes internal state to display the proper screen
    // and initializes and displays the screen
    // see Interface Doc for details
    // takes as a paramter the new turn number
    //!@{
    void InitTurn( int turn_number );      //!< resets all active controls to use the latest data when it has been changed at the beginning of a new turn

    void RestoreFromSaveData(const XMLElement& elem); ///< restores the UI state that was saved in an earlier call to SaveGameData().
    
    void ScreenIntro();                        //!< Intro Screen
    void ScreenProcessTurn();                     //!< Turn Star Progress Splash Screen

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
    //!@}
    
    static ClientUI*    GetClientUI() {return s_the_UI;}   //!< returns a pointer to the singleton ClientUI class

    /** shows a message dialog box with the given message; if \a play_alert_sound is true, and UI sound effects are
        currently enabled, the default alert sound will be played as the message box opens */
    static void MessageBox(const std::string& message, bool play_alert_sound = false);

    static void LogMessage(const std::string& msg); //!<sends a message to the logger

    static void GenerateSitRepText( SitRepEntry *p_sit_rep ); ///< generates a SitRep string from it's XML data.

    /** Loads and returns one of a set of numbered textures.  This is supposed to be used to retrieve textures that are numbered, e.g. the star textures 
        blue1.png, blue2,png, ..., yellow1.png, yellow2.png, etc.  It is assumed that all such files are numbered starting with 1, not 0.  \a dir_name 
        is the name of the directory in which the images are found, relative to ClientUI::ART_DIR.
        \a types_to_names is a map of object types to their base filenames, e.g. (System::BLUE --> "blue"), (PT_SWAMP --> "swamp"), etc.  \a type is 
        the type of object for which you want a texture, which is used to look up the name in \a types_to_names.  \a hash_key is used to pick the numer 
        from [1, N] of the texture to be used.  This number is usually the ID() of the UniverseObject that the texture represents on-screen.  This is 
        used so that the texture used to represent the object is arbitrary, but is always the same, even after a save-reload cycle.  \see SystemIcon.cpp 
        ... for an example of how to use this function. */
    static boost::shared_ptr<GG::Texture> GetNumberedTexture(const std::string& dir_name, const std::map<int, std::string>& types_to_names, 
                                                             int type, int hash_key);

    /** returns the directory in which sound files can be found */
    static const std::string& SoundDir();

    //! \name Static Config Data
    //!@{
    static std::string FONT;             //!< The default font to use
    static std::string FONT_BOLD;        //!< The default bold font to use
    static std::string FONT_ITALIC;      //!< The default italic font to use
    static std::string FONT_BOLD_ITALIC; //!< The default bold and italic font to use
    static int         PTS;              //!< default point size
    static std::string TITLE_FONT;       //!< The default font to use for the window title
    static int         TITLE_PTS;        //!< default point size to use for window title
    
    static std::string DIR;              //!< directory currently being used, contains config files
    static std::string ART_DIR;          //!< directory holding artwork
    static std::string SOUND_DIR;        //!< directory holding sound and music

    static GG::Clr     TEXT_COLOR;       //!< color of UI text
    
    // generic UI windows
    static GG::Clr     WND_COLOR;              //!< color of a UI window
    static GG::Clr     WND_BORDER_COLOR;       //!< color of window borders
    static GG::Clr     WND_OUTER_BORDER_COLOR; //!< color of the outermost border
    static GG::Clr     WND_INNER_BORDER_COLOR; //!< color of the innermost border

    // controls
    static GG::Clr     CTRL_COLOR;         //!< color of UI controls
    static GG::Clr     CTRL_BORDER_COLOR;

    static GG::Clr     BUTTON_COLOR;
    static int         BUTTON_WIDTH;       //!< default width to use for window buttons

    static GG::Clr     STATE_BUTTON_COLOR;

    static GG::Clr     SCROLL_TAB_COLOR;
    static  int        SCROLL_WIDTH;
    
    static GG::Clr     DROP_DOWN_LIST_INT_COLOR;
    static GG::Clr     DROP_DOWN_LIST_ARROW_COLOR;

    static GG::Clr     EDIT_HILITE_COLOR;
    static GG::Clr     EDIT_INT_COLOR;
    static GG::Clr     MULTIEDIT_INT_COLOR;

    static GG::Clr     STAT_INCR_COLOR;   //!< used to color increasing stats text (eg "+2")
    static GG::Clr     STAT_DECR_COLOR;   //!< used to color decreasing stats text (eg "-3")

    static int         SYSTEM_ICON_SIZE;  //!< the width/height of a System/Icon at zoom = 1.0
    static double      FLEET_BUTTON_SIZE; //!< the width/height of a FleetButton at zoom = 1.0, relative to the size of a SystemIcon

    // game UI windows
    static GG::Clr     SIDE_PANEL_COLOR;
    static GG::Clr     SIDE_PANEL_BUILD_PROGRESSBAR_COLOR;
    static int         SIDE_PANEL_PLANET_NAME_PTS;
    static int         SIDE_PANEL_PTS;

    // tech screen
    static GG::Clr     KNOWN_TECH_FILL_COLOR;
    static GG::Clr     KNOWN_TECH_TEXT_AND_BORDER_COLOR;
    static GG::Clr     RESEARCHABLE_TECH_FILL_COLOR;
    static GG::Clr     RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;
    static GG::Clr     UNRESEARCHABLE_TECH_FILL_COLOR;
    static GG::Clr     UNRESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;
    static GG::Clr     TECH_WND_PROGRESS_BAR_BACKGROUND;
    static GG::Clr     TECH_WND_PROGRESS_BAR;
    //!@}

private:
    void HideAllWindows();              //!< hides all the UI windows from view
    
    void SwitchState(State state);      //!< switch current state to >state<, free's last state window and create the one for the new state

    State m_state;                      //!< represents the screen currently being displayed

    IntroScreen*     m_intro_screen;      //!< the intro (and main menu) screen first showed when the game starts up
    MapWnd*          m_map_wnd;           //!< the galaxy map
    PythonConsoleWnd*m_python_console;    //!< the python console
    TurnProgressWnd* m_turn_progress_wnd; //!< the turn progress window

    int              m_previously_shown_system; //!< the ID of the system that was shown in the sidepanel before the last call to HideAllWindows()

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


inline std::string ClientUIRevision()
{return "$Id$";}

#endif // _ClientUI_h_
