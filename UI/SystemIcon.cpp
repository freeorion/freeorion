//SystemIcon.cpp

#ifndef _SystemIcon_h_
#include "SystemIcon.h"
#endif

#ifndef _System_h_
#include "../universe/System.h"
#endif

#ifndef _GGDrawUtil_h_
#include "GGDrawUtil.h"
#endif

#ifndef _GalaxyMapScreen_h_
#include "GalaxyMapScreen.h"
#endif

#ifndef _GGApp_h_
#include "GGApp.h"
#endif

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

#ifndef _GGTextControl_h_
#include "GGTextControl.h"
#endif

#ifndef _MapWnd_h_
#include "MapWnd.h"
#endif

// TEMP from MapWnd, a global that defines our temporary working universe
extern std::vector<UniverseObject*> g_temp_objects;

const int SystemIcon::ICON_WIDTH = 64;
const int SystemIcon::ICON_HEIGHT = 64;

SystemIcon::SystemIcon(int id) :
    GG::Control(0,0,1,1, GG::Wnd::CLICKABLE)
{
    m_systemID = id;
    
    //resize to the proper size
    //TEMP: this will be read from the universe
    SizeMove((int)(g_temp_objects[id]->X()*MapWnd::MAX_SCALE_FACTOR),
             (int)(g_temp_objects[id]->Y()*MapWnd::MAX_SCALE_FACTOR),
             (int)(g_temp_objects[id]->X()*MapWnd::MAX_SCALE_FACTOR+(double)ICON_WIDTH),
             (int)(g_temp_objects[id]->Y()*MapWnd::MAX_SCALE_FACTOR+(double)ICON_HEIGHT)); //to position of this universe object
             
            
    //load the texture
    m_graphic = new GG::Texture(); 
    
    System* sys = dynamic_cast<System*>(g_temp_objects[id]);
    
    //load the proper graphic for the color of the star
    switch(sys->Star())
    {   
    case System::YELLOW:       
        m_graphic->Load(ClientUI::ART_DIR+"/stars/yellow1.png");
        break;
    case System::RED_GIANT:
        m_graphic->Load(ClientUI::ART_DIR+"/stars/red3.png");
        break;
    case System::RED_DWARF:
        m_graphic->Load(ClientUI::ART_DIR+"/stars/red4.png");
        break;
    default:
    //just load a blue star for now
        m_graphic->Load(ClientUI::ART_DIR+"/stars/blue2.png");
        break;
    }//end switch
    
    //setup static graphic
    m_static_graphic = new GG::StaticGraphic(0,0,ICON_WIDTH,ICON_HEIGHT,m_graphic);
    AttachChild(m_static_graphic);
    
    
    
    //set up the name text control
    m_name = new GG::TextControl(0, Height(), Width(), 12,sys->Name(),ClientUI::FONT, ClientUI::PTS, GG::TF_CENTER,GG::CLR_WHITE);
    AttachChild(m_name);
    
    Show();
    //TODO: set up connection of signals and slots            

}


SystemIcon::~SystemIcon()
{
    // TODO: destruction code
}

int SystemIcon::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    //GG::App::GetApp()->Logger().debug("Drawing a SystemIcon");
    
    
    glScissor(GalaxyMapScreen::s_scissor_rect.Left(),
                GalaxyMapScreen::s_scissor_rect.Top(),
                GalaxyMapScreen::s_scissor_rect.Right(),
                GalaxyMapScreen::s_scissor_rect.Bottom());
    glEnable(GL_SCISSOR_TEST);
    
    //for now, render a white rectangle
    //GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_WHITE, GG::CLR_RED, 2);
    //glColor3f(1.0,1.0,1.0);
    glEnable(GL_TEXTURE_2D);
    //m_graphic->OrthoBlit(Parent()->UpperLeft().x + ul.x, Parent()->UpperLeft().y + ul.y);
    
    //render the name of the system
    //  center the name under the star
       
    //glDisable(GL_TEXTURE_2D);
    glDisable(GL_SCISSOR_TEST);
}
