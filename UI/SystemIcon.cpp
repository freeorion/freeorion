//SystemIcon.cpp

#include "SystemIcon.h"

#include "../client/human/HumanClientApp.h"
#include "../universe/System.h"
#include "GGDrawUtil.h"
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

    SetText(sys->Name());

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
    m_name = new GG::TextControl(0, Height(), Width(), 12, sys->Name(), ClientUI::FONT, ClientUI::PTS, GG::TF_CENTER, ClientUI::TEXT_COLOR);
    AttachChild(m_name);

    Show();
}


SystemIcon::~SystemIcon()
{
    // TODO: destruction code
}
