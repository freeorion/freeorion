//MapWnd.cpp

#include "MapWnd.h"

#include "../client/human/HumanClientApp.h"
#include "../universe/ClientUniverse.h"
#include "../universe/UniverseObject.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "SystemIcon.h"
#include "GGDrawUtil.h"
#include "../universe/Predicates.h"

#include <vector>

const int MapWnd::MAX_SCALE_FACTOR = 32;

#if 0
std::vector<UniverseObject*> g_temp_objects;
#endif

MapWnd::MapWnd() :
    GG::Wnd(0,0,5,5,GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE)
{
    //resize to the exact size of the universe....this will eventually change
    SizeMove(0, 0, static_cast<int>(ClientUniverse::UNIVERSE_WIDTH * MAX_SCALE_FACTOR), 
             static_cast<int>(ClientUniverse::UNIVERSE_WIDTH * MAX_SCALE_FACTOR));
    
    //set up background images
    m_background[0] = boost::shared_ptr<GG::Texture>(new GG::Texture());
    m_background[0]->Load(ClientUI::ART_DIR+"starfield1.png");
    m_bg_position_X[0] = 10.0;
    m_bg_position_Y[0] = 10.0;
    m_bg_scroll_rate[0] = 0.5;
    
    m_background[1] = boost::shared_ptr<GG::Texture>(new GG::Texture());
    m_background[1]->Load(ClientUI::ART_DIR+"starfield2.png");
    m_bg_position_X[1] = 10.0;
    m_bg_position_Y[1] = 10.0;
    m_bg_scroll_rate[1] = 0.25;
    
    m_background[2] = boost::shared_ptr<GG::Texture>(new GG::Texture());
    m_background[2]->Load(ClientUI::ART_DIR+"starfield3.png");
    m_bg_position_X[2] = 10.0;
    m_bg_position_Y[2] = 10.0;
    m_bg_scroll_rate[2] = 0.12;
}

MapWnd::~MapWnd()
{
}

void MapWnd::InitTurn()
{
    ClientUniverse::ObjectIDVec system_IDs = ClientApp::Universe().FindObjectIDs(IsSystem);
    for (unsigned int i = 0; i < system_IDs.size(); ++i) {
        SystemIcon* icon = new SystemIcon(system_IDs[i]);
        AttachChild(icon);
        GG::Connect(icon->LeftClickedSignal(), &MapWnd::SelectSystem, this);
    }
}


int MapWnd::Render()
{
    glScissor(GalaxyMapScreen::s_scissor_rect.Left(),
              GalaxyMapScreen::s_scissor_rect.Top(),
              GalaxyMapScreen::s_scissor_rect.Right(),
              GalaxyMapScreen::s_scissor_rect.Bottom());
    glEnable(GL_SCISSOR_TEST);

    //render the background images
    RenderBackgrounds();

    glDisable(GL_TEXTURE_2D);
    
    GG::FlatRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, GG::CLR_ZERO, GG::CLR_WHITE, 2);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_SCISSOR_TEST);    
     
    return 1;
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

void MapWnd::SelectSystem(int systemID)
{
    m_selected_system_signal(systemID);
}
