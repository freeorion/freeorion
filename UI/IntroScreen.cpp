//IntroScreen.cpp
#include "IntroScreen.h"

#include "../client/human/HumanClientApp.h"

#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "ClientUI.h"
#include "GalaxySetupWnd.h"
#include "EmpireSelect.h"
#include "About.h"
#include "ServerConnectWnd.h"
#include "../network/Message.h"
#include "CUIControls.h"

#include <stdlib.h>
#include <string>

IntroScreen::IntroScreen():
    CUI_Wnd(ClientUI::String("INTRO_WINDOW_TITLE"), (1024+300)/2, 300, 200, 400, 
            GG::Wnd::ONTOP | GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::RESIZABLE | 
            CUI_Wnd::MINIMIZABLE | CUI_Wnd::CLOSABLE)
{
    //get the background texture
    m_background = new GG::Texture();
    m_background->Load(ClientUI::ART_DIR + "splash01.png");
    
    //create staticgraphic from the image
    GG::StaticGraphic* bg_graphic = new GG::StaticGraphic(0,0,GG::App::GetApp()->AppWidth(),GG::App::GetApp()->AppHeight(),m_background);
    GG::App::GetApp()->Register(bg_graphic);
    
    //create buttons
    m_single_player = new CUIButton(20, 30, 160, ClientUI::String("INTRO_BTN_SINGLE_PLAYER"));
    m_multi_player = new CUIButton(20, 70, 160, ClientUI::String("INTRO_BTN_MULTI_PLAYER"));
    m_options = new CUIButton(20, 110, 160, ClientUI::String("INTRO_BTN_OPTIONS"));
    m_about = new CUIButton(20, 150, 160, ClientUI::String("INTRO_BTN_ABOUT"));
    m_exit_game = new CUIButton(20, 190, 160, ClientUI::String("INTRO_BTN_EXIT"));
    
    //attach buttons
    AttachChild(m_single_player);
    AttachChild(m_multi_player);
    AttachChild(m_options);
    AttachChild(m_about);
    AttachChild(m_exit_game);

    //connect signals and slots
    GG::Connect(m_single_player->ClickedSignal(), &IntroScreen::OnSinglePlayer, this);
    GG::Connect(m_multi_player->ClickedSignal(), &IntroScreen::OnMultiPlayer, this);
    GG::Connect(m_options->ClickedSignal(), &IntroScreen::OnOptions, this);
    GG::Connect(m_about->ClickedSignal(), &IntroScreen::OnAbout, this);
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
     GG::XMLElement retval;
     return retval;
    //TODO: encode to XML
}


///////////////////////
//    Message Maps   //
/////////////////////////////////////////////////////////////

void IntroScreen::OnSinglePlayer()
{
    //TODO: Start the server and some AI's and go to galaxy setup screen
}//OnSinglePlayer()

void IntroScreen::OnMultiPlayer()
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
                       ClientUI::MessageBox(ClientUI::String("ERR_UNABLE_TO_CONNECT") );
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

            if (HumanClientApp::GetApp()->PlayerID() == -1)
            {
              ClientUI::MessageBox(ClientUI::String("Game already hosted or unknown error") );
            }
            else
            {
                EmpireSelect empire_wnd;
                empire_wnd.Run();
    
                if(empire_wnd.m_end_with_ok)
                {
                     // send the empire selection info
                     GG::XMLDoc empire_setup;

                     //char tmp_empire_name[255];
                     //sprintf(tmp_empire_name, "%d", empire_wnd.GalaxySize());

                     // add the empire name
                     GG::XMLElement elem("empire_name");
                     elem.SetText(empire_wnd.EmpireName().c_str());
                     empire_setup.root_node.AppendChild(elem);

                     // add the empire color
                     char tmp_empire_color[255];
                     sprintf(tmp_empire_color, "%d", empire_wnd.EmpireColor());

                     elem = GG::XMLElement("empire_color");
                     elem.SetAttribute("value", tmp_empire_color);
                     empire_setup.root_node.AppendChild(elem);

		             // send the empire setup choices to the server
                     HumanClientApp::GetApp()->NetworkCore().SendMessage(JoinGameSetup(empire_setup));
                }
                else
                    ClientUI::MessageBox(ClientUI::String("Failed empire setup!") );
                   
            }//else
                        
            //if we got this far, we can actually start up the map screen YAY!
            GG::App::GetApp()->Logger().debug("About to call ScreenMap");
            ClientUI::GetClientUI()->ScreenMap();
            GG::App::GetApp()->Logger().debug("After ScreenMap");
        }//if end with ok
    }//if
}//OnMultiPlayer()

void IntroScreen::OnOptions()
{

}//OnOptions

void IntroScreen::OnAbout()
{
   About about_wnd;
   about_wnd.Run();

}//OnAbout()

void IntroScreen::OnExitGame()
{
    //exit the application
    GG::App::GetApp()->Exit(0); //exit with 0, good error code
}//OnExitGame()
