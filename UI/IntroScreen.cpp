//IntroScreen.cpp

#ifndef _IntroScreen_h_
#include "IntroScreen.h"
#endif

#ifndef _GGDrawUtil_h_
#include "GGDrawUtil.h"
#endif

#ifndef _GGStaticGraphic_h_
#include "GGStaticGraphic.h"
#endif

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

#ifndef _GalaxySetupWnd_h_
#include "GalaxySetupWnd.h"
#endif

#ifndef _ServerConnectWnd_h_
#include "ServerConnectWnd.h"
#endif

IntroScreen::IntroScreen():
    GG::Wnd(600, 100, 300, 600)
{

    //get a texture to fill the background with
    
    m_background = new GG::Texture();
    m_background->Load(ClientUI::ART_DIR + "splash01.png");
  
    //create staticgraphic from the image
    GG::StaticGraphic* bg_graphic = new GG::StaticGraphic(0,0,GG::App::GetApp()->AppWidth(),GG::App::GetApp()->AppHeight(),m_background);
    GG::App::GetApp()->Register(bg_graphic);
    
    //create buttons
    m_start_game = new GG::Button(20, 20, 260, 30, "Start New Game", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR,ClientUI::TEXT_COLOR);
    m_quick_start = new GG::Button(20, 70, 260, 30, "Quick Start", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR,ClientUI::TEXT_COLOR);
    m_exit_game = new GG::Button(20, 120, 260, 30, "Exit", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR,ClientUI::TEXT_COLOR);
    
    //attach buttons
    AttachChild(m_start_game);
    AttachChild(m_quick_start);
    AttachChild(m_exit_game);
    
    //connect signals and slots
    GG::Connect(m_start_game->ClickedSignal(), &IntroScreen::OnStartGame, this);
    GG::Connect(m_quick_start->ClickedSignal(), &IntroScreen::OnQuickStart, this);
    GG::Connect(m_exit_game->ClickedSignal(), &IntroScreen::OnExitGame, this);
   
}//IntroScreen()

IntroScreen::IntroScreen(const GG::XMLElement &elem):
    GG::Wnd(elem.Child("GG::Wnd"))
{
    //TODO: load from XML
    
}//IntroScreen(XMLElement)

IntroScreen::~IntroScreen()
{
    //TODO: destruction code
    
}// ~IntroScreen

GG::XMLElement IntroScreen::XMLEncode() const
{
    //TODO: encode to XML
}

int IntroScreen::Render()
{
    //draw it
     
    GG::BeveledRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y,ClientUI::WND_COLOR,ClientUI::BORDER_COLOR,true);

//    return GG::Wnd::Render();
    return 1;
}//Render()

///////////////////////
//    Message Maps   //
/////////////////////////////////////////////////////////////

void IntroScreen::OnStartGame()
{
    //Open up the server connect screen
    ServerConnectWnd wnd(10,10,800,600);
    wnd.Run();
    
    //TODO: add server init code here
    
    //If user clicked OK, open the galaxy screen    
    if(wnd.IsServerSelected())
    {
        GalaxySetupWnd galaxy_wnd;    
        galaxy_wnd.Run();
        
        //TODO: Do universe setup
        
        if(galaxy_wnd.m_end_with_ok)
        {
            //TODO: Open Empire selection window if user clicked OK
        }
    }

}//OnStartGame()

void IntroScreen::OnExitGame()
{
    //exit the application
    GG::App::GetApp()->Exit(0); //exit with 0, good error code
}//OnExitGame()

void IntroScreen:: OnQuickStart()
{
    //TODO: Start a quick game.
    // choose localhost, universe, and empire all for player


}//OnQuickStart()
