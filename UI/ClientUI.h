//ClientUI.h
#ifndef _ClientUI_h_
#define _ClientUI_h_

//include relevant controls

#ifndef _ToolContainer_h_
#include "ToolContainer.h"
#endif

#ifndef _StringTable_h_
#include "StringTable.h"
#endif

//#include "CUIControls.h"

//include log4cpp stuff
#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

//FreeOrion defines & includes
//    these shall change as more teams complete their classes
#ifndef _ClientNetworkCore_h_
#include "../network/ClientNetworkCore.h"
#endif

#ifndef _ClientUniverse_h_
#include "../universe/ClientUniverse.h"
#endif

class ClientEmpire;

#ifndef _Planet_h_
#include "../universe/Planet.h"
#endif

#ifndef _System_h_
#include "../universe/System.h"
#endif

#ifndef _Fleet_h_
#include "../universe/Fleet.h"
#endif

#ifndef _Ship_h_
#include "../universe/Ship.h"
#endif

class Tech;

#ifndef _SitRepEntry_h_
#include "../util/SitRepEntry.h"
#endif

class Combat;

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
public:
    //! \name Construction & Initialization
    //!@{
    
    ClientUI(const std::string& string_table_file = StringTable::S_DEFAULT_FILENAME);        //!< construction (calls Initialize())
    ClientUI(const GG::XMLElement& elem); //!< construct via XML
    ~ClientUI();    //!< destruction (calls Cleanup())

    bool Initialize(const std::string& string_table_file);    //!< provided to setup initial variables. 
    bool Cleanup();        //!< provided to clean up ClientUI's memory space. 

    //!@}
    
    //! \name Accessors
    //!@{
    
    //inline const log4cpp::Category& Logger() {return s_logger;}//!< Returns the logger associated with ClientUI
    
    static inline ClientUI* GetClientUI() {if(the_UI) return the_UI; return NULL;} //!< returns a pointer to the singleton ClientUI class
    inline const std::string& Language() const {return m_string_table->Language();} //!< Returns the language of the StringTable object associated with ClientUI
    static inline const std::string& String(std::string index){return the_UI->m_string_table->String(index); } //!< Returns a lookup from the string table
    static inline void LogMessage(const std::string& msg) {s_logger.debug(msg);} //!<sends a message to the logger
    
    //!@}
    
    //! \name Mutators
    //!@{
    
    //! @param width screen width
    //! @param height screen height
    bool ChangeResolution(int width, int height);    //!< changes the screen resolution and modifies any images or data required to make the change.  
    bool Freeze();                        //!< freezes the interface so that user input is ignored.
    bool Unfreeze();                    //!< unfreezes the UI and input is recognized again
    bool Frozen();                //!< returns true if interface is frozen, false otherwise
    
    GG::XMLElement XMLEncode() const; //!< encodes ClientUI to XML
    
    //! @param parent A pointer to the Wnd that should contain the tooltip
    //! @param tool A pointer to a ToolWnd to be associated with that window
    inline bool AttachToolWnd(GG::Wnd* parent, ToolWnd* tool) {if(m_tooltips) return m_tooltips->AttachToolWnd(parent, tool);};    //!< Adds a ToolWnd to the given window
    
    //!@}
    
    //! \name Utilities
    //!@{
    
    //! draws an interface window
    static void DrawWindow(int x1, int y1, int x2, int y2, const std::string& title = "", bool resize = true, const std::string& font = TITLE_FONT, int pts = TITLE_PTS);
    
    //! @param message The message to display
    static void MessageBox(const std::string& message);    //!< shows a message dialog box with the given message     
    
    //!@}

    //! \name GameCore Interface functions
    //! calling these changes internal state to display the proper screen
    //! and initializes and displays the screen
    //! see Interface Doc for details
    
    //!@{
    
    void ScreenIntro();                        //!< Intro Screen
    void ScreenSettings(const ClientNetworkCore &net);    //!< Game/Network Options Screen
    void ScreenEmpireSelect();                    //!< Empire Selection Screen
    void ScreenTurnStart();                    //!< Turn Start Splash Screen
    
    //! @param u address of ClientUniverse module
    //! @param e address of ClientEmpire module
    void ScreenMap(const ClientUniverse &u, const ClientEmpire &e);     //!< Universe Map Screen

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

    //!@}
    
    //! \name Zooming Functions
    //!@{
    
    //! @param p address of a planet that we wish to zoom to
    //! @return true if successful, false if object doesn't exist
    bool ZoomTo(const Planet& p);    //!< Zooms to a particular planet on the galaxy map and opens the planet screen for that planet

    //! @param s address of system that we wish to zoom to
    //! @return true if successful, false if object doesn't exist
    bool ZoomTo(const System& s);    //!< Zooms to a particular system on the galaxy map
    
    //! @param s address of fleet that we wish to zoom to
    //! @return true if successful, false if object doesn't exist
    bool ZoomTo(const Fleet& f);    //!< Zooms to a particular fleet on the galaxy map and opens the fleet window
    
    //! @param s address of ship that we wish to zoom to
    //! @return true if successful, false if object doesn't exist
    bool ZoomTo(const Ship& s);    //!< Zooms to a particular ship on the galaxy map and opens its fleet and/or ship window
    
    //! @param t address of technology that we wish to zoom to
    //! @return true if successful, false if object doesn't exist
    bool ZoomTo(const Tech& t);    //!< Opens the technology screen and presents a description of the given technology
   
   //!@}  

private:
    //! \name Internal Functions
    //!@{
    void UnregisterCurrent(bool delete_it = false);    //!< removes the current window from the Zlist and deletes it if delete_it is true
    
    //!@}

private:
    //! \name Static Members
    //!@{
    static log4cpp::Category& s_logger;        //!< log4cpp logging category
    static ClientUI* the_UI;                   //!<pointer to the one and only ClientUI object
    //!@}

private:
    const Uint32 TOOLTIP_DELAY;    //!<number of milliseconds to initialize tooltips to
    ToolContainer* m_tooltips;        //!< the single toolcontainer object
    int m_state;                    //!< represents the screen currently being displayed
    GG::Wnd* m_current_window;    //!< a pointer to the window (screen) currently being displays
    bool m_frozen;    //!< true if the interface is frozen and false if it isn't
    
    StringTable* m_string_table;    //!< a string table to lookup international strings
    

};//ClientUI


#endif
