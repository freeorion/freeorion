//SystemIcon.cpp

#include "SystemIcon.h"

#include "ClientUI.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "GGTextControl.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/System.h"

// static(s)
const int SystemIcon::ICON_WIDTH = 32;
const int SystemIcon::ICON_HEIGHT = 32;

SystemIcon::SystemIcon(int id, double zoom) :
    GG::Control(0, 0, 1, 1, GG::Wnd::CLICKABLE),
    m_systemID(id),
    m_static_graphic(0),
    m_name(0)
{
    const System* sys = dynamic_cast<const System*>(ClientApp::Universe().Object(id));

    SetText(sys->Name());

    //resize to the proper size
    SizeMove((int)(sys->X() * zoom - ICON_WIDTH / 2),
             (int)(sys->Y() * zoom - ICON_HEIGHT / 2),
             (int)(sys->X() * zoom - ICON_WIDTH / 2 + ICON_WIDTH),
             (int)(sys->Y() * zoom - ICON_HEIGHT / 2 + ICON_HEIGHT)); //to position of this universe object

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
    m_static_graphic = new GG::StaticGraphic(0, 0, ICON_WIDTH, ICON_HEIGHT, graphic, GG::SG_FITGRAPHIC);
    AttachChild(m_static_graphic);

    //set up the name text control
    m_name = new GG::TextControl(0, 0, sys->Name(), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
    PositionSystemName();
    AttachChild(m_name);

    Show();
}


SystemIcon::~SystemIcon()
{
}

void SystemIcon::SizeMove(int x1, int y1, int x2, int y2)
{
    Wnd::SizeMove(x1, y1, x2, y2);
    if (m_static_graphic)
        m_static_graphic->SizeMove(0, 0, x2 - x1, y2 - y1);
    PositionSystemName();
}

void SystemIcon::ShowName()
{
    m_name->Show();
}

void SystemIcon::HideName()
{
    m_name->Hide();
}

void SystemIcon::PositionSystemName()
{
    if (m_name)
        m_name->MoveTo(Width() / 2 - m_name->Width() / 2, Height());
}
