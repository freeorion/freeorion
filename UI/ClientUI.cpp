//ClientUI.cpp

#include <string>
#include <fstream>

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

#ifndef _GGApp_h_
#include "GGApp.h"
#endif

#ifndef _GGMessageDlg_h_
#include "dialogs/GGMessageDlg.h"
#endif

//static members
log4cpp::Category& ClientUI::s_logger(log4cpp::Category::getRoot());

//Init and Cleanup//////////////////////////////////////

ClientUI::ClientUI():
    TOOLTIP_DELAY(1000) //1 second delay for tooltips to appear
{
    Initialize();
}//ClientUI()

bool ClientUI::Initialize()
{
    //initialize Tooltip engine
    m_tooltips=new ToolContainer(TOOLTIP_DELAY);
    
    //initialize UI state
    m_state=STATE_STARTUP;
    
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
    
    return true;
}//Freeze()

bool ClientUI::Unfreeze()
{
    //TODO: Unfreeze the interface
    // should either enable all windows, or reconnect all signals to sockets
    
    return true;
}//Unfreeze()

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

}//ScreenIntro()
                      
void ClientUI::ScreenSettings(ClientNetwork &net)
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

void ClientUI::ScreenSitrep(const std::vector<SitRepEvent> events)
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

#ifdef DEBUG
void ClientUI::MessageBox(const std::string& message)
{
    string dbg_msg = "MessageBox( \"" + message + "\" )";
    s_logger.debug(dbg_msg);    //write message to log
    
    GG::MessageDlg dlg(320,200,message,"arial.ttf",10,GG::CLR_GRAY,GG::CLR_WHITE);
    dlg.Run();    
}//MessageBox()
#endif

