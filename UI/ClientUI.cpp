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
#include "dialogs/GGThreeButtonDlg.h"
#endif

#ifndef _IntroScreen_h_
#include "IntroScreen.h"
#endif

#ifndef _GGDrawUtil_h_
#include "GGDrawUtil.h"
#endif

//include OpenGL headers
#include <GL/gl.h>
#include <GL/glu.h>

//static members
std::string ClientUI::FONT          = "arial.ttf";    
int         ClientUI::PTS           = 12;   
std::string ClientUI::TITLE_FONT    = "arial.ttf";
int         ClientUI::TITLE_PTS     = 9;

std::string ClientUI::DIR           = "default/";     
std::string ClientUI::ART_DIR       = ClientUI::DIR + "art/small/";    

GG::Clr     ClientUI::WND_COLOR(0,0,0,210);
GG::Clr     ClientUI::BORDER_COLOR(0,0,0,255);
GG::Clr     ClientUI::OUTER_BORDER_COLOR(64,64,64,255);
GG::Clr     ClientUI::INNER_BORDER_COLOR(255,255,255,255);
GG::Clr     ClientUI::CTRL_COLOR(30,30,30,255); 
GG::Clr     ClientUI::TEXT_COLOR(255,255,255,255);

int         ClientUI::BUTTON_WIDTH = 7;

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

ClientUI::ClientUI(const GG::XMLElement& elem)
{
    using namespace GG;
    
    the_UI = this;
    
    if(elem.Tag() != "ClientUI")
        throw std::invalid_argument("Tried to construct a 'ClientUI' object from an XML tag that was not 'ClientUI'.");
    
    const XMLElement* current = &elem.Child("BORDER_COLOR");
    BORDER_COLOR = Clr(current->Child("GG::Clr"));
    
    current = &elem.Child("CTRL_COLOR");
    CTRL_COLOR = Clr(current->Child("GG::Clr"));
    
    current = &elem.Child("INNER_BORDER_COLOR");
    INNER_BORDER_COLOR = Clr(current->Child("GG::Clr"));
    
    current = &elem.Child("OUTER_BORDER_COLOR");
    OUTER_BORDER_COLOR = Clr(current->Child("GG::Clr"));
    
    current = &elem.Child("TEXT_COLOR");
    TEXT_COLOR = Clr(current->Child("GG::Clr"));
    
    current = &elem.Child("WND_COLOR");
    WND_COLOR = Clr(current->Child("GG::Clr"));
    
    current = &elem.Child("PTS");
    PTS = lexical_cast<int>(current->Attribute("value"));
    
    current = &elem.Child("TITLE_PTS");
    TITLE_PTS = lexical_cast<int>(current->Attribute("value"));
    
    current = &elem.Child("DIR");
    DIR = current->Attribute("value");
    
    current = &elem.Child("FONT");
    FONT = current->Attribute("value");
    
    current = &elem.Child("TITLE_FONT");
    TITLE_FONT = current->Attribute("value");
    
    current = &elem.Child("ART_DIR");
    ART_DIR = DIR + "art/" + current->Attribute("value") + "/";

    //call initialize with stringtable filename
    current = &elem.Child("STRINGTABLE_FILENAME");
    Initialize(DIR + current->Attribute("value"));
    
}//ClientUI(XMLElement)
 
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

const std::string& ClientUI::Language() const 
{
    return m_string_table->Language();
}

GG::XMLElement ClientUI::XMLEncode() const
{
    using namespace GG;
    XMLElement retval("ClientUI"), temp;
    
    //static constants
    
    //config the directory
    temp = XMLElement("DIR");
    temp.SetAttribute("value", DIR);
    retval.AppendChild(temp);
    //only large or small is saved, so art dir is created from the DIR
    temp = XMLElement("ART_DIR");
    temp.SetAttribute("value", (ART_DIR.find("small") != std::string::npos ? "small" : "large") );
    retval.AppendChild(temp);
          
    temp = XMLElement("FONT");
    temp.SetAttribute("value", FONT);
    retval.AppendChild(temp);
    
    temp = XMLElement("PTS");
    temp.SetAttribute("value", lexical_cast<std::string>(PTS));
    retval.AppendChild(temp);
    
    temp = XMLElement("TITLE_FONT");
    temp.SetAttribute("value", TITLE_FONT);
    retval.AppendChild(temp);
    
    temp = XMLElement("TITLE_PTS");
    temp.SetAttribute("value", lexical_cast<std::string>(TITLE_PTS));
    retval.AppendChild(temp);
    
    temp = XMLElement("WND_COLOR");
    temp.AppendChild(WND_COLOR.XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("TEXT_COLOR");
    temp.AppendChild(TEXT_COLOR.XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("CTRL_COLOR");
    temp.AppendChild(CTRL_COLOR.XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("OUTER_BORDER_COLOR");
    temp.AppendChild(OUTER_BORDER_COLOR.XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("BORDER_COLOR");
    temp.AppendChild(BORDER_COLOR.XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("INNER_BORDER_COLOR");
    temp.AppendChild(INNER_BORDER_COLOR.XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("TOOLTIP_DELAY");
    temp.SetAttribute("value", lexical_cast<std::string>(TOOLTIP_DELAY));
    retval.AppendChild(temp);
    
    temp = XMLElement("STRINGTABLE_FILENAME");
    temp.SetAttribute("value", m_string_table->Filename());
    retval.AppendChild(temp);
    
    //other values are initialized automatically
    
    //return the element
    return retval;
    
}//XMLEncode()

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

///////////////////////////////////////////////////

//Zoom Functions///////////////////////////////////
bool ClientUI::ZoomToPlanet(int id)
{
    //TODO: Zooming code
    
    return false;
}

bool ClientUI::ZoomToSystem(int id)
{ 
    //TODO: Zooming code
    
    return false;
}

bool ClientUI::ZoomToFleet(int id)
{
    //TODO: Zooming code
    
    return false;
}

bool ClientUI::ZoomToShip(int id)
{
    //TODO: Zooming code
    
    return false;
}

bool ClientUI::ZoomToTech(int id)
{ 
    //TODO: Zooming code
    
    return false;
}

bool ClientUI::ZoomToEncyclopediaEntry(const std::string& str)
{ 
    //TODO: Zooming code
    
    return false;
}

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

void ClientUI::MessageBox(const std::string& message)
{
 //   std::string dbg_msg = "MessageBox( \"" + message + "\" )";
 //   s_logger.debug(dbg_msg);    //write message to log
    
    GG::ThreeButtonDlg dlg(320,200,message,FONT,PTS+2,WND_COLOR, BORDER_COLOR, CTRL_COLOR, TEXT_COLOR, 1,
        new GG::Button((320-75)/2, 170, 75, 25, "OK", FONT, PTS, CTRL_COLOR, TEXT_COLOR));
    
    dlg.Run();    
}//MessageBox()

void ClientUI::LogMessage(const std::string& msg)
{
    s_logger.debug(msg);
}

const std::string& ClientUI::String(const std::string& index)
{
    return the_UI->m_string_table->String(index);
}

////////////////////////////////////////////////////
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

