//MapWnd.cpp

#include "MapWnd.h"

#include "ClientUI.h"
#include "../universe/ClientUniverse.h"
#include "GGDrawUtil.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "SidePanel.h"
#include "../universe/System.h"
#include "SystemIcon.h"
#include "../universe/UniverseObject.h"

#include <vector>

namespace {
const int SIDE_PANEL_WIDTH = 300;
}

const int MapWnd::MAX_SCALE_FACTOR = 8;

MapWnd::MapWnd() :
    GG::Wnd(0, 0, 5, 5, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE),
    m_drag_offset(-1, -1),
    m_dragged(false)
{
    //resize to the exact size of the universe....this will eventually change
    SizeMove(0, 0, static_cast<int>(ClientUniverse::UNIVERSE_WIDTH * MAX_SCALE_FACTOR), 
             static_cast<int>(ClientUniverse::UNIVERSE_WIDTH * MAX_SCALE_FACTOR));

    SetText("MapWnd");

    m_side_panel = new SidePanel(GG::App::GetApp()->AppWidth() - SIDE_PANEL_WIDTH, 0, SIDE_PANEL_WIDTH, GG::App::GetApp()->AppHeight());
    AttachChild(m_side_panel);
    Connect(SelectedSystemSignal(), &SidePanel::SetSystem, m_side_panel);

    //set up background images
    m_background[0] = boost::shared_ptr<GG::Texture>(new GG::Texture());
    m_background[0]->Load(ClientUI::ART_DIR + "starfield1.png");
    m_bg_position_X[0] = 10.0;
    m_bg_position_Y[0] = 10.0;
    m_bg_scroll_rate[0] = 0.5;
    
    m_background[1] = boost::shared_ptr<GG::Texture>(new GG::Texture());
    m_background[1]->Load(ClientUI::ART_DIR + "starfield2.png");
    m_bg_position_X[1] = 10.0;
    m_bg_position_Y[1] = 10.0;
    m_bg_scroll_rate[1] = 0.25;
    
    m_background[2] = boost::shared_ptr<GG::Texture>(new GG::Texture());
    m_background[2]->Load(ClientUI::ART_DIR + "starfield3.png");
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
    MoveChildUp(m_side_panel);
    m_side_panel->Hide();
}


int MapWnd::Render()
{
    RenderBackgrounds();
    glDisable(GL_TEXTURE_2D);
    GG::FlatRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, GG::CLR_ZERO, GG::CLR_WHITE, 2);
    glEnable(GL_TEXTURE_2D);
    return 1;
}

int MapWnd::LButtonDown (const GG::Pt &pt, Uint32 keys)
{
    m_drag_offset = pt - UpperLeft();
    return 1;
}

int MapWnd::LDrag (const GG::Pt &pt, const GG::Pt &move, Uint32 keys)
{
    GG::Pt move_to_pt = pt - m_drag_offset;
    GG::Pt wnd_sz = WindowDimensions();

    if (0 < move_to_pt.x)
        move_to_pt.x = 0;
    if (0 < move_to_pt.y)
        move_to_pt.y = 0;

    if (move_to_pt.x < GG::App::GetApp()->AppWidth() - wnd_sz.x)
        move_to_pt.x = GG::App::GetApp()->AppWidth() - wnd_sz.x;
    if (move_to_pt.y < GG::App::GetApp()->AppHeight() - wnd_sz.y)
        move_to_pt.y = GG::App::GetApp()->AppHeight() - wnd_sz.y;

    GG::Pt final_move = move_to_pt - UpperLeft();
    m_side_panel->OffsetMove(-final_move);
    MoveBackgrounds(final_move);
    MoveTo(move_to_pt);

    m_dragged = true;

    return 1;
}

int MapWnd::LButtonUp (const GG::Pt &pt, Uint32 keys)
{
    m_drag_offset = GG::Pt(-1, -1);
    m_dragged = false;
    return 1;
}

int MapWnd::LClick (const GG::Pt &pt, Uint32 keys)
{
    m_drag_offset = GG::Pt(-1, -1);
    if (!m_dragged)
        m_selected_system_signal(UniverseObject::INVALID_OBJECT_ID);
    m_dragged = false;
    return 1;
}

void MapWnd::RenderBackgrounds()
{
    float x, y;
    for (int i = 0; i < GMAP_NUM_BACKGROUNDS; ++i) {
        glColor3f(1.0, 1.0, 1.0);
        x = std::fmod(m_bg_position_X[i], m_background[i]->Width());
        while (x < GG::App::GetApp()->AppWidth()) {
            y = std::fmod(m_bg_position_Y[i], m_background[i]->Height());
            while (y < GG::App::GetApp()->AppHeight()) {
                m_background[i]->OrthoBlit(static_cast<int>(x), static_cast<int>(y));
                y += m_background[i]->Height();
            }
            x += m_background[i]->Width();
        }
    }
}

void MapWnd::MoveBackgrounds(const GG::Pt& move)
{   
    float move_x, move_y;
    
    for (int i = 0; i < GMAP_NUM_BACKGROUNDS; ++i) {
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
