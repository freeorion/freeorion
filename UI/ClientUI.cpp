//ClientUI.cpp

#include "ClientUI.h"

#include "CUIControls.h"
#include "GGApp.h"
#include "GGClr.h"
#include "GGDrawUtil.h"
#include "IntroScreen.h"
#include "TurnProgressWnd.h"
#include "dialogs/GGThreeButtonDlg.h"
#include "MapWnd.h"
#include "ToolContainer.h"
#include "../util/AppInterface.h"


#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <string>
#include <fstream>

//static members
std::string ClientUI::FONT          = "arial.ttf";
int         ClientUI::PTS           = 12;
std::string ClientUI::TITLE_FONT    = "arial.ttf";
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
    "SITREP_BASE_BUILT"
  };
}



//Init and Cleanup//////////////////////////////////////
ClientUI::ClientUI(const std::string& string_table_file /* = StringTable::S_DEFAULT_FILENAME */) :
    TOOLTIP_DELAY(1000), // 1 second delay for tooltips to appear
    m_tooltips(0),
    m_state(STATE_STARTUP),
    m_string_table(0),
    m_intro_screen(0),
    m_map_wnd(0),
    m_turn_progress_wnd(0)
{
    s_the_UI = this;    
    Initialize(string_table_file);
}//ClientUI()

ClientUI::ClientUI(const GG::XMLElement& elem) :
    TOOLTIP_DELAY(1000),
    m_tooltips(0),
    m_state(STATE_STARTUP),
    m_string_table(0),
    m_intro_screen(0),
    m_map_wnd(0),
    m_turn_progress_wnd(0)
{
    using namespace GG;
    
    s_the_UI = this;
    
    if(elem.Tag() != "ClientUI")
        throw std::invalid_argument("Tried to construct a 'ClientUI' object from an XML tag that was not 'ClientUI'.");
    
    const XMLElement* current = &elem.Child("WND_BORDER_COLOR");
    WND_BORDER_COLOR = Clr(current->Child("GG::Clr"));
    
    current = &elem.Child("CTRL_COLOR");
    CTRL_COLOR = Clr(current->Child("GG::Clr"));
    
    current = &elem.Child("WND_INNER_BORDER_COLOR");
    WND_INNER_BORDER_COLOR = Clr(current->Child("GG::Clr"));
    
    current = &elem.Child("WND_OUTER_BORDER_COLOR");
    WND_OUTER_BORDER_COLOR = Clr(current->Child("GG::Clr"));
    
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
    
    temp = XMLElement("WND_OUTER_BORDER_COLOR");
    temp.AppendChild(WND_OUTER_BORDER_COLOR.XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("WND_BORDER_COLOR");
    temp.AppendChild(WND_BORDER_COLOR.XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("WND_INNER_BORDER_COLOR");
    temp.AppendChild(WND_INNER_BORDER_COLOR.XMLEncode());
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

bool ClientUI::AttachToolWnd(GG::Wnd* parent, ToolWnd* tool) 
{
    return (m_tooltips ? m_tooltips->AttachToolWnd(parent, tool) : false);
}


//////////////////////////////////////////////////////////

//Utilities////////////////////////////////////////////////
bool ClientUI::ChangeResolution(int width, int height)
{
    //TODO: Determine ability to reinitialize SDL with OpenGL.

    return false;
}//ChangeResolution()


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

    // clean up prvious windows, based on previous state
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


void ClientUI::UpdateTurnProgress( const std::string& phase_str, const int empire_id )
{
  m_turn_progress_wnd->UpdateTurnProgress( phase_str, empire_id );
}


void ClientUI::GenerateSitRepText( SitRepEntry *p_sit_rep )
{
  // get template string
  std::string template_str( String( g_string_id_lut[ p_sit_rep->m_type ] ) );

  // parse string
  p_sit_rep->GenerateVarText( template_str );

}
