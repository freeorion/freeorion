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

#include "../network/Message.h"

#include <stdlib.h>
#include <string>

#include "../network/Message.h"

IntroScreen::IntroScreen():
    CUI_Wnd("FreeOrion Main Menu", 600, 100, 300, 600, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE)
{

    //get a texture to fill the background with
    
    m_background = new GG::Texture();
    m_background->Load(ClientUI::ART_DIR + "splash01.png");
  
    //create staticgraphic from the image
    GG::StaticGraphic* bg_graphic = new GG::StaticGraphic(0,0,GG::App::GetApp()->AppWidth(),GG::App::GetApp()->AppHeight(),m_background);
    GG::App::GetApp()->Register(bg_graphic);
    
    //create buttons
    m_start_game = new GG::Button(20, 30, 260, 30, "Start New Game", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR,ClientUI::TEXT_COLOR);
    m_quick_start = new GG::Button(20, 80, 260, 30, "Quick Start", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR,ClientUI::TEXT_COLOR);
    m_exit_game = new GG::Button(20, 130, 260, 30, "Exit", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR,ClientUI::TEXT_COLOR);
    
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
    CUI_Wnd(elem.Child("CUI_Wnd"))
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
/*
int IntroScreen::Render()
{
    //draw it
     
//    GG::BeveledRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y,ClientUI::WND_COLOR,ClientUI::BORDER_COLOR,true);
//    ClientUI::DrawWindow(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, "FreeOrion Main Menu");

//    return GG::Wnd::Render();
    return 1;
}//Render()
*/
///////////////////////
//    Message Maps   //
/////////////////////////////////////////////////////////////

void IntroScreen::OnStartGame()
{
    //Open up the server connect screen
    //        XML Read begin
//        ifstream dlg_file("servercnct.xml");
//        assert(dlg_file);
//        GG::XMLDoc doc;
//        doc.ReadDoc(dlg_file);
//        dlg_file.close();
//        ServerConnectWnd* sc = new ServerConnectWnd(doc.root_node.Child("ServerConnectWnd"));
//        XML Read end
   
    /*
            Here ClientCore should perform a preliminary scan to
            determine the available servers, at least "localhost".
            Servers can be added via the AddServer call.
    */

    // Auto Constructor begin
    ServerConnectWnd wnd(400,100,450,400);
    // Auto Constructor end
    
    // Add some servers
    wnd.AddServer("127.0.0.1","localhost");
    wnd.AddServer("194.23.24.21","LAN Server");
        
    wnd.Run();
    
    if (wnd.IsServerSelected())
    {
            HumanClientApp::GetApp()->Logger().debugStream() << "Selected server: " << wnd.GetSelectedServer();
    }
    else
    {
            HumanClientApp::GetApp()->Logger().debugStream() << "No server was selected.";       
    }
    
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
            
            //TEMP
                //display the chosen settings

            // connect to server
#define SERVER_CONNECT_TIMEOUT 30000         //TODO: Move to config file
                
               int timeout_count=GG::App::GetApp()->Ticks();
               while (!HumanClientApp::GetApp()->NetworkCore().ConnectToLocalhostServer())
               {
                   if((timeout_count + GG::App::GetApp()->Ticks()) >= (timeout_count + SERVER_CONNECT_TIMEOUT))
                   {
                       ClientUI::MessageBox("Unable to connect to server." );
                       break;
                   }               
               }

		      //sleep(1);

                // start a game on the server
                GG::XMLDoc game_parameters;
                // use (host) human player named "Zach"
                GG::XMLElement elem("host_player_name");
                elem.SetText("Zach");
                game_parameters.root_node.AppendChild(elem);
                // 5 players total
                elem = GG::XMLElement("num_players");
                elem.SetAttribute("value", "5");
                    game_parameters.root_node.AppendChild(elem);
                // 1 AI client
                elem = GG::XMLElement("AI_client");
                game_parameters.root_node.AppendChild(elem);
		// Universe setup
                char tmp_universe_size[255];
                char tmp_universe_shape[255];

		// Add universe size and shape to the XML doc
                sprintf(tmp_universe_size, "%d", galaxy_wnd.GalaxySize());
                sprintf(tmp_universe_shape, "%d", galaxy_wnd.GalaxyShape());
		elem = GG::XMLElement("universe_params");
                elem.SetAttribute("size", tmp_universe_size);
                elem.SetAttribute("shape", tmp_universe_shape);
		if (boost::lexical_cast<int>(tmp_universe_shape) == 4)
                  elem.SetAttribute("file", galaxy_wnd.GalaxyFile().c_str());
                game_parameters.root_node.AppendChild(elem);
		
                HumanClientApp::GetApp()->NetworkCore().SendMessage(HostGameMessage(game_parameters));

                char tmp[255];
                sprintf(tmp, "Size: %d\nShape: %d\nFilename: %s",
                         galaxy_wnd.GalaxySize(), galaxy_wnd.GalaxyShape(), galaxy_wnd.GalaxyFile().c_str());
                ClientUI::MessageBox(tmp);
            // \TEMP
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

