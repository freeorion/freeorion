//ClientUI.cpp

#include <string>
#include <fstream>

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif 

#ifndef _GGApp_h_
#include "GGApp.h"
#endif

#ifndef _GGClr_h_
#include "GGClr.h"
#endif

#ifndef _GGMessageDlg_h_
#include "dialogs/GGMessageDlg.h"
#endif

#ifndef _IntroScreen_h_
#include "IntroScreen.h"
#endif

//static members
std::string ClientUI::FONT          = "arial.ttf";    
int         ClientUI::PTS           = 12;   
std::string ClientUI::DIR           = "default/";     
std::string ClientUI::ART_DIR       = ClientUI::DIR + "art/small/";    

GG::Clr     ClientUI::WND_COLOR(0,0,0,210);
GG::Clr     ClientUI::BORDER_COLOR(255,255,255,255);
GG::Clr     ClientUI::CTRL_COLOR(30,30,30,255); 
GG::Clr     ClientUI::TEXT_COLOR(255,255,255,255);

//private static members
log4cpp::Category& ClientUI::s_logger(log4cpp::Category::getRoot());
ClientUI* ClientUI::the_UI = NULL;

//Init and Cleanup//////////////////////////////////////

ClientUI::ClientUI(const std::string& string_table_file /* = StringTable::S_DEFAULT_FILENAME */):
    TOOLTIP_DELAY(1000) //1 second delay for tooltips to appear
{
    the_UI = this;    
    Initialize(string_table_file);
}//ClientUI()
 
bool ClientUI::Initialize(const std::string& string_table_file)
{
    //initialize Tooltip engine
    m_tooltips = new ToolContainer(TOOLTIP_DELAY);
    
    //initialize string table
    m_string_table = new StringTable(string_table_file);
    
    //initialize UI state & window
    m_state = STATE_STARTUP;
    m_current_window = NULL;
    
    //initialize frozen interface
    m_frozen = false;
    
    //TODO: Initialize variables.
    
    
    
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
    s_logger.debug("ClientUI logger initialized.");
    
    return true;
}//Initialize()

ClientUI::~ClientUI() 
{
    s_logger.debug("Shutting down ClientUI.");
    Cleanup();
}//~ClientUI()

bool ClientUI::Cleanup()
{
    if(m_tooltips!=NULL)
        delete(m_tooltips);
    m_tooltips=NULL;
    
    if(m_string_table!=NULL)
        delete(m_string_table);
    m_string_table=NULL;

    the_UI=NULL;
    //TODO: Destroy variables, etc.   

    return true; 
}//Cleanup()

//////////////////////////////////////////////////////////

//Utilities////////////////////////////////////////////////
bool ClientUI::ChangeResolution(int width, int height)
{
    //TODO: Determine ability to reinitialize SDL with OpenGL.

    return false;
}//ChangeResolution()

bool ClientUI::Freeze()
{
    //TODO: Freeze the interface
    // should probably disable all windows, or possibly disconnect all signals from all sockets
    // OTHER POSSIBILITY: store m_frozen variable
    m_frozen = true;
    
    return true;
}//Freeze()

bool ClientUI::Unfreeze()
{
    //TODO: Unfreeze the interface
    // should either enable all windows, or reconnect all signals to sockets
    m_frozen = false;
    return true;
}//Unfreeze()

bool ClientUI::Frozen()
{
    return m_frozen;
}

/////////////////////////////////////////////////////////

//Zoom Functions///////////////////////////////////
bool ClientUI::ZoomTo(const Planet& p)
{
    //TODO: Zooming code
    
    return false;
}//ZoomTo(Planet)

bool ClientUI::ZoomTo(const System& s)
{ 
    //TODO: Zooming code
    
    return false;
}//ZoomTo(Planet)

bool ClientUI::ZoomTo(const Fleet& f)
{
    //TODO: Zooming code
    
    return false;
}//ZoomTo(Fleet)

bool ClientUI::ZoomTo(const Ship& s)
{
    //TODO: Zooming code
    
    return false;
}//ZoomTo(Ship)

bool ClientUI::ZoomTo(const Tech& t)
{ 
    //TODO: Zooming code
    
    return false;
}//ZoomTo(Tech)

/////////////////////////////////////////////////////

//Screen Functions///////////////////////////////////
void ClientUI::ScreenIntro()
{
    //This will display a splash screen and a menu.
    
    //all Screen* functions should call this
    UnregisterCurrent(true); //remove from z-list and delete (if it exists)
    
    m_state = STATE_INTRO;    //set to intro screen state
    
    //TODO: Create options screen
    //TEMP
    m_current_window = new IntroScreen();
    GG::App::GetApp()->RegisterOnTop(m_current_window);

}//ScreenIntro()
                      
void ClientUI::ScreenSettings(const ClientNetworkCore &net)
{

}//ScreenSettings()

void ClientUI::ScreenEmpireSelect()
{

}//ScreenEmpireSelect()

void ClientUI::ScreenTurnStart()
{
 
}//ScreenTurnStart()

void ClientUI::ScreenMap(const ClientUniverse &u, const ClientEmpire &e)
{

}//ScreenMap()

void ClientUI::ScreenSitrep(const std::vector<SitRepEntry> &events)
{

}//ScreenSitrep()

void ClientUI::ScreenProcessTurn(int state)
{

}//ScreenProcessTurn()

void ClientUI::ScreenBattle(Combat* combat)
{

}//ScreenBattle()

void ClientUI::ScreenSave(bool show)
{

}//ScreenSave()

void ClientUI::ScreenLoad(bool show)
{

}//ScreenLoad()

/////////////////////////////////////////////////////
void ClientUI::UnregisterCurrent(bool delete_it /*= false */)
{
    if(m_current_window)
    {
        GG::App::GetApp()->Remove(m_current_window);
        if(delete_it)
        {
            delete(m_current_window);
            m_current_window=NULL;
        }
    }
}//UnregisterCurrent

/////////////////////////////////////////////////////

void ClientUI::MessageBox(const std::string& message)
{
 //   std::string dbg_msg = "MessageBox( \"" + message + "\" )";
 //   s_logger.debug(dbg_msg);    //write message to log
    
    GG::MessageDlg dlg(320,200,message,FONT,PTS+2,WND_COLOR,BORDER_COLOR,TEXT_COLOR,
        new GG::Button( (320-75)/2, 170, 75, 25, "OK", FONT, PTS, CTRL_COLOR, TEXT_COLOR));
    
    dlg.Run();    
}//MessageBox()


