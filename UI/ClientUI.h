//ClientUI.h
#ifndef _ClientUI_h_
#define _ClientUI_h_

// include relevant controls
#ifndef _ToolContainer_h_
#include "ToolContainer.h"
#endif

#ifndef _StringTable_h_
#include "StringTable.h"
#endif

#ifndef _GalaxyMapScreen_h_
#include "GalaxyMapScreen.h"
#endif

// include log4cpp stuff
#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

// FreeOrion includes
#ifndef _SitRepEntry_h_
#include "../util/SitRepEntry.h"
#endif

class ClientNetworkCore;
class ClientUniverse;
class ClientEmpire;
class Tech;
class Combat;
class GalaxyMapScreen;

//! \brief ClientUI Main Module
//!This is the main class of the ClientUI module.
//!it contains objects of many other classes and controls the
//!display of all information onscreen.

class ClientUI
{
public:
    //!Internal States
    //!These determine what screen is currently being displayed
    enum        
    {
        STATE_STARTUP     = 0,
        STATE_INTRO       = 1,
        STATE_SETTINGS    = 2,
        STATE_EMPIRESEL   = 3,
        STATE_TURNSTART   = 4,
        STATE_MAP         = 5,
        STATE_SITREP      = 6,
        STATE_PROCESS     = 7,
        STATE_BATTLE      = 8,
        STATE_SAVE        = 9,
        STATE_LOAD        = 10,
        STATE_SHUTDOWN    = 11
    };//enum
    
    //! \name Static Config Data
    //!@{
    static std::string FONT;    //!< The default font to use
    static int         PTS;    //!< default point size
    static std::string TITLE_FONT;    //!< The default font to use for the window title
    static int         TITLE_PTS;    //!< default point size to use for window title
    
    static std::string DIR;     //!< directory currently being used, contains config files
    static std::string ART_DIR;    //!< directory holding artwork, ("*/art/small/" or "*/art/large/"
    
    static GG::Clr     WND_COLOR; //!< color of a UI window
    static GG::Clr     BORDER_COLOR; //!< color of window borders
    static GG::Clr     OUTER_BORDER_COLOR; //!< color of the outermost border
    static GG::Clr     INNER_BORDER_COLOR; //!< color of the innermost border
    static GG::Clr     CTRL_COLOR; //!< color of UI controls
    static GG::Clr     TEXT_COLOR; //!< color of UI text    

    static int         BUTTON_WIDTH;    //!< default width to use for window buttons
    //!@}

    //! \name Structors //!@{
    ClientUI(const std::string& string_table_file = StringTable::S_DEFAULT_FILENAME);        //!< construction (calls Initialize())
    ClientUI(const GG::XMLElement& elem); //!< construct via XML
    ~ClientUI();    //!< destruction (calls Cleanup())

    bool Initialize(const std::string& string_table_file);    //!< provided to setup initial variables. 
    bool Cleanup();        //!< provided to clean up ClientUI's memory space. 
    //!@}
    
    //! \name Accessors //!@{
    const log4cpp::Category& Logger() {return s_logger;}    //!< Returns the logger associated with ClientUI
    
    const std::string&  Language() const;                   //!< Returns the language of the StringTable object associated with ClientUI
    
    const GG::SubTexture& SitRepIcon(SitRepEntry::EntryType type) const; ///< returns the icon for this sitrep entry type; returns the default icon if \a type has no associated icon
    //!@}
    
    //! \name Mutators //!@{
    //! @param width screen width
    //! @param height screen height
    bool ChangeResolution(int width, int height);    //!< changes the screen resolution and modifies any images or data required to make the change.  
    bool Freeze();                        //!< freezes the interface so that user input is ignored.
    bool Unfreeze();                    //!< unfreezes the UI and input is recognized again
    bool Frozen();                //!< returns true if interface is frozen, false otherwise
    
    GG::XMLElement XMLEncode() const; //!< encodes ClientUI to XML
    
    //! @param parent A pointer to the Wnd that should contain the tooltip
    //! @param tool A pointer to a ToolWnd to be associated with that window
    bool AttachToolWnd(GG::Wnd* parent, ToolWnd* tool) {if(m_tooltips) return m_tooltips->AttachToolWnd(parent, tool);};    //!< Adds a ToolWnd to the given window

    // GameCore Interface functions
    // calling these changes internal state to display the proper screen
    // and initializes and displays the screen
    // see Interface Doc for details
    //!@{
    void ScreenIntro();                        //!< Intro Screen
    void ScreenSettings(const ClientNetworkCore &net);    //!< Game/Network Options Screen
    void ScreenEmpireSelect();                    //!< Empire Selection Screen
    void ScreenTurnStart();                    //!< Turn Start Splash Screen
    
    //! @param u address of ClientUniverse module
    //! @param e address of ClientEmpire module
    void ScreenMap();     //!< Universe Map Screen

    //! @param events vector containing all the events to be listed
    void ScreenSitrep(const std::vector<SitRepEntry> &events);    //!< Sitrep Screen
    
    //! @param state integer code pertaining to the message to display on the turn-processing screen
    void ScreenProcessTurn(int state);                //!< Turn-processing screen
    
    //! @param combat pointer to a Combat module
    void ScreenBattle(Combat* combat);                //!< Battle Screen
    
    //! @param show true if the screen is to be displayed, false if it is to be turned off
    void ScreenSave(bool show);                    //!< Savegame Screen
    
    //! @param show true if the screen is to be displayed, false if it is to be turned off
    void ScreenLoad(bool show);                    //!< Load Game Screen

    // Zooming Functions
    //! @param p address of a planet that we wish to zoom to
    //! @return true if successful, false if object doesn't exist
    bool ZoomToPlanet(int id);    //!< Zooms to a particular planet on the galaxy map and opens the planet screen for that planet

    //! @param s address of system that we wish to zoom to
    //! @return true if successful, false if object doesn't exist
    bool ZoomToSystem(int id);    //!< Zooms to a particular system on the galaxy map
    
    //! @param s address of fleet that we wish to zoom to
    //! @return true if successful, false if object doesn't exist
    bool ZoomToFleet(int id);    //!< Zooms to a particular fleet on the galaxy map and opens the fleet window
    
    //! @param s address of ship that we wish to zoom to
    //! @return true if successful, false if object doesn't exist
    bool ZoomToShip(int id);    //!< Zooms to a particular ship on the galaxy map and opens its fleet and/or ship window
    
    //! @param t address of technology that we wish to zoom to
    //! @return true if successful, false if object doesn't exist
    bool ZoomToTech(int id);    //!< Opens the technology screen and presents a description of the given technology
   
    //! @param t address of technology that we wish to zoom to
    //! @return true if successful, false if object doesn't exist
    bool ZoomToEncyclopediaEntry(const std::string& str);    //!< Opens the encyclodedia screen and presents the entry for the given item
    //!@}
    
    static ClientUI*    GetClientUI() {return the_UI;}      //!< returns a pointer to the singleton ClientUI class

    //! @param message The message to display
    static void MessageBox(const std::string& message);    //!< shows a message dialog box with the given message     

    static void LogMessage(const std::string& msg); //!<sends a message to the logger
    static const std::string&  String(const std::string& index);    //!< Returns a lookup from the string table

private:
    void UnregisterCurrent(bool delete_it = false);    //!< removes the current window from the Zlist and deletes it if delete_it is true

    const Uint32 TOOLTIP_DELAY;    //!<number of milliseconds to initialize tooltips to
    ToolContainer* m_tooltips;        //!< the single toolcontainer object
    int m_state;                    //!< represents the screen currently being displayed
    GG::Wnd* m_current_window;    //!< a pointer to the window (screen) currently being displays
    bool m_frozen;    //!< true if the interface is frozen and false if it isn't
    
    StringTable* m_string_table;    //!< a string table to lookup international strings
    
    GalaxyMapScreen* m_galaxy_map;    //!< the galaxy map screen

    static log4cpp::Category& s_logger;        //!< log4cpp logging category
    static ClientUI* the_UI;                   //!<pointer to the one and only ClientUI object
};//ClientUI


#endif
