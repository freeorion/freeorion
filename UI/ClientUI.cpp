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
//Utilities
/////////////////////////////////////////////////////

void ClientUI::MessageBox(const std::string& message)
{
 //   std::string dbg_msg = "MessageBox( \"" + message + "\" )";
 //   s_logger.debug(dbg_msg);    //write message to log
    
    GG::MessageDlg dlg(320,200,message,FONT,PTS+2,WND_COLOR,BORDER_COLOR,TEXT_COLOR,
        new GG::Button( (320-75)/2, 170, 75, 25, "OK", FONT, PTS, CTRL_COLOR, TEXT_COLOR));
    
    dlg.Run();    
}//MessageBox()

void ClientUI::DrawWindow(int x1, int y1, int x2, int y2, const std::string& title, bool close, bool min, bool resize, const std::string& font, int pts)
{
    const int o_x1=x1, o_y1=y1, o_x2=x2, o_y2=y2; //stores original values of these
    //draws a rectangle like the burndaddy prototype
    //first draw a flat rectangle of the current window color
    GG::App::GetApp()->Enter2DMode();
    GG::FlatRectangle(x1, y1, x2, y2, WND_COLOR, OUTER_BORDER_COLOR, 1);
    
    //draw a wire rectangle
    // 15 gap on top, 5 gap around
    //start at upper left
    x1 += 5;
    y1 += 15;
    x2 -= 5;
    y2 -= 5; 
    
    //use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    
    if(resize)
    {
        //LogMessage("Resizable window!");
        //if it is resizable, draw a line strip that has a diagonal at lower right corner
        
        glBegin(GL_LINE_STRIP);
            glColor4ubv(INNER_BORDER_COLOR.v);
            glVertex2i(x1, y1);
            glVertex2i(x2, y1);
            
            //line down to 15 px from total bottom
            glVertex2i(x2, y2-10);
            //line diagonal to bottom
            glVertex2i(x2-10, y2);
            //line back to lower left
            glVertex2i(x1, y2);
            glVertex2i(x1, y1-1);
        glEnd();
    }//end if resize
    else
    {
        //LogMessage("NOT A Resizable window!");
        //...otherwise, make a clear rectangle with a thin border
//        GG::FlatRectangle(x1, y1, x2, y2, GG::CLR_ZERO, INNER_BORDER_COLOR, 1);
        glBegin(GL_LINE_STRIP);
            glColor4ubv(INNER_BORDER_COLOR.v);
            glVertex2i(x1,y1);
            glVertex2i(x2,y1);
            glVertex2i(x2,y2);
            glVertex2i(x1,y2);
            glVertex2i(x1,y1);
        glEnd();
    }//end else

    //draw extra details
    
        
    if(resize)
    {
        LogMessage("Resizable window! Part II.");
        
        glBegin(GL_LINES);
            //draw the extra lines if its a resizable window
            glColor4ubv(INNER_BORDER_COLOR.v);
            glVertex2i(x2, y2-6);
            glVertex2i(x2-6, y2);
            
            glVertex2i(x2, y2-2);
            glVertex2i(x2-2, y2);
        glEnd();
    }
    if(min)
    {    
        LogMessage("Minimizable window!");
        //draw dash if minimizable
        //draw the lines for the dash and the x
        //dash
        
        glBegin(GL_LINES);
            glColor4ubv(INNER_BORDER_COLOR.v);
            glVertex2i(o_x2-30, o_y1+7);
            glVertex2i(o_x2-23, o_y1+7);
        glEnd();
    }
    if(close)
    {
        //LogMessage("Closable Window!");
        //if closable, draw the "X"
        //draw cross
        
        glBegin(GL_LINES);
            glColor4ubv(INNER_BORDER_COLOR.v);
            glVertex2i(o_x2-15, o_y1+3);
            glVertex2i(o_x2-8, o_y1+10);
            
            glVertex2i(o_x2-15, o_y1+10);
            glVertex2i(o_x2-8, o_y1+3); 
        glEnd();
    }
    
//    glFlush();
    
    glEnable(GL_TEXTURE_2D);
    //now draw the text title
    GG::TextImage title_img(title, font, pts);
    title_img.OrthoBlit(o_x1+3, o_y1+2);
    
    
    GG::App::GetApp()->Exit2DMode();
}

