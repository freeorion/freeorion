//SystemIcon.cpp

#include "SystemIcon.h"

#include "../client/human/HumanClientApp.h"
#include "../universe/System.h"
#include "GGDrawUtil.h"
#include "GalaxyMapScreen.h"
#include "GGApp.h"
#include "ClientUI.h"
#include "GGTextControl.h"
#include "MapWnd.h"

const int SystemIcon::ICON_WIDTH = 64;
const int SystemIcon::ICON_HEIGHT = 64;

SystemIcon::SystemIcon(int id) :
    GG::Control(0,0,1,1, GG::Wnd::CLICKABLE)
{
    m_systemID = id;

    const System* sys = dynamic_cast<const System*>(ClientApp::Universe().Object(id));

    //resize to the proper size
    //TEMP: this will be read from the universe
    SizeMove((int)(sys->X() * MapWnd::MAX_SCALE_FACTOR),
             (int)(sys->Y() * MapWnd::MAX_SCALE_FACTOR),
             (int)(sys->X() * MapWnd::MAX_SCALE_FACTOR + ICON_WIDTH),
             (int)(sys->Y() * MapWnd::MAX_SCALE_FACTOR + ICON_HEIGHT)); //to position of this universe object

    //load the proper graphic for the color of the star
    boost::shared_ptr<GG::Texture> graphic;
    switch (sys->Star()) {
    case System::YELLOW:       
        graphic = HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "/stars/yellow1.png");
        break;
    case System::RED_GIANT:
        graphic = HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "/stars/red3.png");
        break;
    case System::RED_DWARF:
        graphic = HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "/stars/red4.png");
        break;
    default:
    //just load a blue star for now
        graphic = HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "/stars/blue2.png");
        break;
    }//end switch

    //setup static graphic
    m_static_graphic = new GG::StaticGraphic(0, 0, ICON_WIDTH, ICON_HEIGHT, graphic);
    AttachChild(m_static_graphic);

    //set up the name text control
    m_name = new GG::TextControl(0, Height(), Width(), 12,sys->Name(),ClientUI::FONT, ClientUI::PTS, GG::TF_CENTER,GG::CLR_WHITE);
    AttachChild(m_name);

    Show();
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
