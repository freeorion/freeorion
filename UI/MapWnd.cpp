//MapWnd.cpp
#include <vector>

#ifndef _MapWnd_h_
#include "MapWnd.h"
#endif

#ifndef _HumanClientApp_h_
#include "../client/human/HumanClientApp.h"
#endif

#ifndef _UniverseObject_h_
#include "../util/UniverseObject.h"
#endif

#ifndef _System_h_
#include "../universe/System.h"
#endif

#ifndef _Planet_h_
#include "../universe/Planet.h"
#endif

#ifndef _ClientUniverse_h_
#include "../universe/ClientUniverse.h"
#endif

#ifndef _SystemIcon_h_
#include "SystemIcon.h"
#endif

#include "GGDrawUtil.h"

const int MapWnd::MAX_SCALE_FACTOR = 32;

namespace
{

    //predicate for finding systems
    bool IsSystem(UniverseObject* obj)
    {
        if(obj->GetSystem() == obj)
            return true;
        return false;
    }
    
}

std::vector<UniverseObject*> g_temp_objects;

MapWnd::MapWnd() :
    GG::Wnd(0,0,5,5,GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE)
{
    const ClientUniverse u = HumanClientApp::GetApp()->Universe();
    
    //resize to the exact size of the universe....this will eventually change
    SizeMove(0,0,UNIVERSE_X_SIZE*MAX_SCALE_FACTOR, UNIVERSE_Y_SIZE*MAX_SCALE_FACTOR);
    
    
    //TEMP: set up the temporary universe objects
    //REMOVE THIS WHEN THE SERVER STARTS GIVING US UNIVERSE DATA!!
    UniverseObject* temp_object = new System(System::YELLOW, 0, "Utilae", 10.0, 10.0);   
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::RED_DWARF, 0, "Tsev", 1.0, 1.0);   
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::YELLOW, 0, "Aquitaine", 3.0, 5.0);   
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::RED_GIANT, 0, "Nightfish", 3.0, 2.0);   
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::YELLOW, 0, "Tzlaine", 69.0, 70.0);   
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::RED_GIANT, 0, "Drektopia", 8.0, 12.0);   
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::RED_DWARF, 0, "Burndaddy", 10.0, 13.0);   
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::RED_GIANT, 0, "Ocean Machine", 150.0, 190.0);   
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::YELLOW, 0, "Tyreth", 900.0, 900.0);
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::RED_DWARF, 0, "PK", 900.0, 10.0);
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::RED_DWARF, 0, "Jbarcz1", 3.0, 900.0);
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::YELLOW, 0, "Orion", 500.0, 500.0);
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::YELLOW, 0, "Neuromancer", 350.0, 10.0);
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::YELLOW, 0, "Mr. Ed", 735.0, 6.0);
    g_temp_objects.push_back(temp_object);
    
    temp_object = new System(System::YELLOW, 0, "Yoghurt", 600.0, 13.0);
    g_temp_objects.push_back(temp_object);
    
    //set up background images
    m_background[0] = new GG::Texture();
    m_background[0]->Load(ClientUI::ART_DIR+"starfield1.png");
    m_bg_position_X[0] = 10.0;
    m_bg_position_Y[0] = 10.0;
    m_bg_scroll_rate[0] = 0.5;
    
    m_background[1] = new GG::Texture();
    m_background[1]->Load(ClientUI::ART_DIR+"starfield2.png");
    m_bg_position_X[1] = 10.0;
    m_bg_position_Y[1] = 10.0;
    m_bg_scroll_rate[1] = 0.25;
    
    m_background[2] = new GG::Texture();
    m_background[2]->Load(ClientUI::ART_DIR+"starfield3.png");
    m_bg_position_X[2] = 10.0;
    m_bg_position_Y[2] = 10.0;
    m_bg_scroll_rate[2] = 0.12;
}

MapWnd::~MapWnd()
{
//TEMP: de-allocate temporary objects
    std::vector<UniverseObject*>::iterator pos;
    for(pos=g_temp_objects.begin(); pos != g_temp_objects.end(); pos = g_temp_objects.erase(pos))
    {
        if(*pos)
            delete(*pos);
    }
}

void MapWnd::InitTurn()
{
    //should create all of the system children
    std::vector<UniverseObject*>::iterator pos;
    int count=0;
    for(pos=g_temp_objects.begin(); pos != g_temp_objects.end(); ++pos)
    {
        AttachChild(new SystemIcon(count));
        ++count;
    }
    
}


int MapWnd::Render()
{
    HumanClientApp::GetApp()->Enter2DMode();
    const ClientUniverse u = HumanClientApp::GetApp()->Universe();
    
    glScissor(GalaxyMapScreen::s_scissor_rect.Left(),
                GalaxyMapScreen::s_scissor_rect.Top(),
                GalaxyMapScreen::s_scissor_rect.Right(),
                GalaxyMapScreen::s_scissor_rect.Bottom());
    glEnable(GL_SCISSOR_TEST);
    
    //render the background images
    RenderBackgrounds();
    
    
    glDisable(GL_TEXTURE_2D);
    
    //ClientUniverse::ConstObjectVec objects = u.FindObjects(IsSystem);
    //ClientUniverse::ConstObjectVec::iterator pos;
    //ClientUniverse::const_iterator pos;
    //plot each system as a point 
/*
    glColor3f(1.0,1.0,1.0);
    glBegin(GL_POINTS);
        for(pos = u.begin(); pos!=u.end(); ++pos)
        {
            const UniverseObject* sys = pos->second;
            HumanClientApp::GetApp()->Logger().debug("Plotting pixel");
            glVertex2d(sys->X(), sys->Y());
            glVertex2d(sys->X()+1, sys->Y());
            glVertex2d(sys->X()+1, sys->Y()+1);
            glVertex2d(sys->X(), sys->Y()+1);
            
        }
        
    glEnd();
*/
/*
        std::vector<UniverseObject*>::iterator pos;
        glDisable(GL_TEXTURE_2D);
        glColor3f(1.0,1.0,1.0);
        glBegin(GL_POINTS);
            for(pos=g_temp_objects.begin(); pos != g_temp_objects.end(); ++pos)
            {
                HumanClientApp::GetApp()->Logger().debug("Drawing an object");
                UniverseObject* obj = *pos;
                glVertex2d(obj->X(), obj->Y());
                glVertex2d(obj->X()+1, obj->Y()+1);
                glVertex2d(obj->X()+2, obj->Y()+2);
            }
        glEnd();
*/
    GG::FlatRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, GG::CLR_ZERO, GG::CLR_WHITE, 2);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_SCISSOR_TEST);    
            
    HumanClientApp::GetApp()->Exit2DMode();
}

int MapWnd::LDrag (const GG::Pt &pt, const GG::Pt &move, Uint32 keys)
{
    //make sure that <move> doesn't take the corders off the screen
    GG::Pt new_move(move);
    
    //check top-left side
    if(UpperLeft().x + new_move.x > GalaxyMapScreen::s_scissor_rect.Left())
    {
        //MoveTo(GalaxyMapScreen::s_scissor_rect.Left(), UpperLeft().y);
        new_move.x = UpperLeft().x - GalaxyMapScreen::s_scissor_rect.Left();
    }
    if(UpperLeft().y + new_move.y > GalaxyMapScreen::s_scissor_rect.Top())
    {
        //MoveTo(UpperLeft().x, GalaxyMapScreen::s_scissor_rect.Top());
        new_move.y = UpperLeft().y - GalaxyMapScreen::s_scissor_rect.Top();
    }
    
    //check bottom-right
    if(LowerRight().x + new_move.x < GalaxyMapScreen::s_scissor_rect.Right())
    {
       new_move.x = GalaxyMapScreen::s_scissor_rect.Right() - LowerRight().x;
    }
    if(LowerRight().y + new_move.y < GalaxyMapScreen::s_scissor_rect.Bottom())
    {    
       new_move.y = LowerRight().y - GalaxyMapScreen::s_scissor_rect.Bottom();
    }
    
    //scroll the backgrounds
    MoveBackgrounds(new_move);
    
    return GG::Wnd::LDrag(pt, new_move, keys);;
}

void MapWnd::RenderBackgrounds()
{
float x, y;
for(int i=0; i<GMAP_NUM_BACKGROUNDS; ++i)
{
    x = m_bg_position_X[i];
    y = m_bg_position_Y[i];
    
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0,1.0,1.0);
    while(x < GalaxyMapScreen::s_scissor_rect.Right())
    {
        while(y < GalaxyMapScreen::s_scissor_rect.Bottom())
        {
            m_background[i]->OrthoBlit((int)x, (int)y);
            y += m_background[i]->Height();
        }
        x += m_background[i]->Width();
        y = m_bg_position_Y[i];
    }
}
}

void MapWnd::MoveBackgrounds(const GG::Pt& move)
{   
    float move_x, move_y;
    
    for(int i=0; i< GMAP_NUM_BACKGROUNDS; ++i)
    {
    
        move_x = move.x * m_bg_scroll_rate[i];
        move_y = move.y * m_bg_scroll_rate[i];
    
        m_bg_position_X[i] += move_x;
        m_bg_position_Y[i] += move_y;    
    }
}
