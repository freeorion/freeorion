//GalaxyMapScreen.cpp

#ifndef _GalaxyMapScreen_h_
#include "GalaxyMapScreen.h"
#endif

#ifndef _GGDrawUtil_h_
#include "GGDrawUtil.h"
#endif

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

#ifndef _GGApp_h_
#include "GGApp.h"
#endif

#ifndef _FleetWindow_h_
#include "FleetWindow.h"
#endif

#include <vector>

GG::Rect GalaxyMapScreen::s_scissor_rect;
double GalaxyMapScreen::s_scale_factor;


GalaxyMapScreen::GalaxyMapScreen() :
    GG::Wnd(0, 0, GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight(), GG::Wnd::DRAG_KEEPER),
    m_selected_index(-1),
    m_orders(0),
    m_map_wnd(0)
{
    //TODO: one-time initialization
    //set the scissor rectangle
    s_scissor_rect = GG::Rect(0, 0, GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight());
    //set the size
    SizeMove(0, 0, GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight());
    
    //set the scale factor
    s_scale_factor = 1.0;    //just draw everything at max zoom
    
    //add the map windows
    m_map_wnd = new MapWnd();
    
    AttachChild(m_map_wnd);
    
    // ***********************************************************************
    // TEMP TEMP TEMP TEMP -- create fleetwindow for testing/demo purposes
    // ***********************************************************************
 
    std::vector<int> fleets;
    
    // causes memory leak, but thats ok, this is only a test....
    FleetWindow* wnd = new FleetWindow(40, 40, 0, fleets);
    
    AttachChild(wnd);
    GG::App::GetApp()->Register(wnd);
}

GalaxyMapScreen::~GalaxyMapScreen()
{
    //de-initialize
    if(m_orders)
        delete(m_orders);
    m_orders = 0;
    
}

void GalaxyMapScreen::InitTurn()
{
    //use ClientApp::Empire() and ClientApp::Universe() to get those objects
    
    //update the data on the screen to reflect the data in the
    
    //reset the old orderset
    if(m_orders)
        delete(m_orders);
    
    m_orders = new OrderSet();
    //selected index should still be valid....we don't want to reset the
    //  user's selection just because ite a new turn :)
    
    GG::App::GetApp()->Logger().debug("Initializing GalaxyMapScreen");
    
    //call InitTurn() on the map window
    m_map_wnd->InitTurn();
    

}

int GalaxyMapScreen::Render()
{
    //rendering code
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    
    glScissor(s_scissor_rect.Left(),s_scissor_rect.Top(),s_scissor_rect.Right(),s_scissor_rect.Bottom());
    glEnable(GL_SCISSOR_TEST);
    GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_BLACK, GG::CLR_BLACK, 0);
    glDisable(GL_SCISSOR_TEST);
}
