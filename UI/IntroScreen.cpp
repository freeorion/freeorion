//IntroScreen.cpp
#include "IntroScreen.h"

#include "../client/human/HumanClientApp.h"

#include "About.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "../util/Directories.h"
#include "../network/Message.h"
#include "../util/MultiplayerCommon.h"
#include "OptionsWnd.h"
#include "../util/OptionsDB.h"
#include "../util/Serialize.h"
#include "../util/Version.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/Texture.h>

#include <boost/filesystem/fstream.hpp>

#include <cstdlib>
#include <fstream>
#include <string>

namespace {
    int MAIN_MENU_WIDTH = 200;
    int MAIN_MENU_HEIGHT = 340;

    void Options(OptionsDB& db)
    {
        db.AddFlag("force-external-server",  "OPTIONS_DB_FORCE_EXTERNAL_SERVER", false);
        db.Add("UI.main-menu.x", "OPTIONS_DB_UI_MAIN_MENU_X", 0.75, RangedValidator<double>(0.0, 1.0));
        db.Add("UI.main-menu.y", "OPTIONS_DB_UI_MAIN_MENU_Y", 0.35, RangedValidator<double>(0.0, 1.0));
    }

    bool foo_bool = RegisterOptions(&Options);

}

//****************************************************************************************************
class CreditsWnd : public GG::Wnd
{
public:
    CreditsWnd(int x, int y, int w, int h,const XMLElement &credits,int cx, int cy, int cw, int ch,int co);
        
    virtual void Render();
    virtual void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {m_bRender=false;}

private:
    XMLElement m_credits;
    int m_cx,m_cy,m_cw,m_ch,m_co;
    int m_start_time;
    int m_bRender,m_bFadeIn;
};

CreditsWnd::CreditsWnd(int x, int y, int w, int h,const XMLElement &credits,int cx, int cy, int cw, int ch,int co) :
    GG::Wnd(x, y, w, h,GG::CLICKABLE),m_credits(credits),m_cx(cx),m_cy(cy),m_cw(cw),m_ch(ch),m_co(co),
    m_start_time(GG::GUI::GetGUI()->Ticks()),
    m_bRender(true),
    m_bFadeIn(true)
{}

void CreditsWnd::Render()
{
    if(!m_bRender)
        return;

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    boost::shared_ptr<GG::Font> font=HumanClientApp::GetApp()->GetFont(ClientUI::Font(), static_cast<int>(ClientUI::Pts()*1.3));;
    GG::Flags<GG::TextFormat> format = GG::FORMAT_CENTER | GG::FORMAT_TOP;

    GG::FlatRectangle(ul.x,ul.y,lr.x,lr.y,GG::Clr(0.0,0.0,0.0,0.5),GG::CLR_ZERO,0);
    glColor(GG::CLR_WHITE);

    int offset=m_co;

    offset -= (GG::GUI::GetGUI()->Ticks() - m_start_time)/40;

    int transparency = 255;

    if (m_bFadeIn)
    {
        double fade_in = (GG::GUI::GetGUI()->Ticks() - m_start_time)/2000.0;
        if(fade_in>1.0)
            m_bFadeIn=false;
        else
            transparency = static_cast<int>(255*fade_in);
    }

    glColor(GG::Clr(transparency,transparency,transparency,255));

    GG::BeginScissorClipping(ul.x+m_cx,ul.y+m_cy,ul.x+m_cx+m_cw,ul.y+m_cy+m_ch);

    std::string credit;
    for(int i = 0; i<m_credits.NumChildren();i++) {
        if(0==m_credits.Child(i).Tag().compare("GROUP"))
        {
            XMLElement group = m_credits.Child(i);
            for(int j = 0; j<group.NumChildren();j++) {
                if(0==group.Child(j).Tag().compare("PERSON"))
                {
                    XMLElement person = group.Child(j);
                    credit = "";
                    if(person.ContainsAttribute("name"))
                        credit+=person.Attribute("name");
                    if(person.ContainsAttribute("nick") && person.Attribute("nick").length()>0)
                    {
                        credit+=" <rgba 153 153 153 " + boost::lexical_cast<std::string>(transparency) +">(";
                        credit+=person.Attribute("nick");
                        credit+=")</rgba>";
                    }
                    if(person.ContainsAttribute("task") && person.Attribute("task").length()>0)
                    {
                        credit+=" - <rgba 204 204 204 " + boost::lexical_cast<std::string>(transparency) +">";
                        credit+=person.Attribute("task");
                        credit+="</rgba>";
                    }
                    font->RenderText(ul.x+m_cx,ul.y+m_cy+offset,ul.x+m_cx+m_cw,ul.y+m_cy+m_ch,credit, format, 0);
                    offset+=font->TextExtent(credit, format).y+2;
                }
            }
            offset+=font->Lineskip()+2;
        }
    }
    GG::EndScissorClipping();
    if(offset<0)
    {
        m_co = 0;
        m_start_time = GG::GUI::GetGUI()->Ticks()+m_ch*40;
    }
}
//****************************************************************************************************

IntroScreen::IntroScreen() :
    CUIWnd(UserString("INTRO_WINDOW_TITLE"), 
           static_cast<int>(GG::GUI::GetGUI()->AppWidth() * GetOptionsDB().Get<double>("UI.main-menu.x") - MAIN_MENU_WIDTH / 2),
           static_cast<int>(GG::GUI::GetGUI()->AppWidth() * GetOptionsDB().Get<double>("UI.main-menu.y") - MAIN_MENU_HEIGHT / 2),
           MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT, GG::ONTOP | GG::CLICKABLE),
    m_credits_wnd(0),
    m_splash(new GG::StaticGraphic(0, 0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight(),
                                   ClientUI::GetTexture(ClientUI::ArtDir() / "splash.png"),
                                   GG::GRAPHIC_FITGRAPHIC, GG::CLICKABLE)),
    m_logo(new GG::StaticGraphic(0, 0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight() / 10,
                                 ClientUI::GetTexture(ClientUI::ArtDir() / "logo.png"),
                                 GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE))
{
    m_splash->AttachChild(m_logo);
    GG::GUI::GetGUI()->Register(m_splash);

    //create buttons
    m_single_player = new CUIButton(15, 12, 160, UserString("INTRO_BTN_SINGLE_PLAYER"));
    m_multi_player = new CUIButton(15, 52, 160, UserString("INTRO_BTN_MULTI_PLAYER"));
    m_load_game = new CUIButton(15, 92, 160, UserString("INTRO_BTN_LOAD_GAME"));
    m_options = new CUIButton(15, 132, 160, UserString("INTRO_BTN_OPTIONS"));
    m_about = new CUIButton(15, 172, 160, UserString("INTRO_BTN_ABOUT"));
    m_credits = new CUIButton(15, 212, 160, UserString("INTRO_BTN_CREDITS"));
    m_exit_game = new CUIButton(15, 282, 160, UserString("INTRO_BTN_EXIT"));
    
    //attach buttons
    AttachChild(m_single_player);
    AttachChild(m_multi_player);
    AttachChild(m_load_game);
    AttachChild(m_options);
    AttachChild(m_about);
    AttachChild(m_credits);
    AttachChild(m_exit_game);

    //connect signals and slots
    GG::Connect(m_single_player->ClickedSignal, &IntroScreen::OnSinglePlayer, this);
    GG::Connect(m_multi_player->ClickedSignal, &IntroScreen::OnMultiPlayer, this);
    GG::Connect(m_load_game->ClickedSignal, &IntroScreen::OnLoadGame, this);
    GG::Connect(m_options->ClickedSignal, &IntroScreen::OnOptions, this);
    GG::Connect(m_about->ClickedSignal, &IntroScreen::OnAbout, this);
    GG::Connect(m_credits->ClickedSignal, &IntroScreen::OnCredits, this);
    GG::Connect(m_exit_game->ClickedSignal, &IntroScreen::OnExitGame, this);
}

IntroScreen::~IntroScreen()
{
    delete m_credits_wnd;
    delete m_logo;
    delete m_splash;
}

void IntroScreen::OnSinglePlayer()
{
    delete m_credits_wnd;
    m_credits_wnd = 0;
    HumanClientApp::GetApp()->NewSinglePlayerGame();
}

void IntroScreen::OnMultiPlayer()
{
    delete m_credits_wnd;
    m_credits_wnd = 0;
    HumanClientApp::GetApp()->MulitplayerGame();
}

void IntroScreen::OnLoadGame()
{  
    delete m_credits_wnd;
    m_credits_wnd = 0;
    HumanClientApp::GetApp()->LoadSinglePlayerGame();
}

void IntroScreen::OnOptions()
{
    delete m_credits_wnd;
    m_credits_wnd = 0;

    OptionsWnd options_wnd;
    options_wnd.Run();
}

void IntroScreen::OnAbout()
{
    delete m_credits_wnd;
    m_credits_wnd = 0;

    About about_wnd;
    about_wnd.Run();
}

void IntroScreen::OnCredits()
{
    delete m_credits_wnd;
    m_credits_wnd = 0;


    XMLDoc doc;
    boost::filesystem::ifstream ifs(GetSettingsDir() / "credits.xml");
    doc.ReadDoc(ifs);
    ifs.close();


    if (!doc.root_node.ContainsChild("CREDITS"))
        return;

    XMLElement credits = doc.root_node.Child("CREDITS");
    // only the area between the upper and lower line of the splash screen should be darkend
    // if we use another splash screen we have the change the following values
    int nUpperLine = ( 79 * GG::GUI::GetGUI()->AppHeight()) / 768,
        nLowerLine = (692 * GG::GUI::GetGUI()->AppHeight()) / 768;

    m_credits_wnd = new CreditsWnd(0,nUpperLine,
                                   GG::GUI::GetGUI()->AppWidth(),nLowerLine-nUpperLine,
                                   credits,
                                   60,0,600,(nLowerLine-nUpperLine),((nLowerLine-nUpperLine))/2);

    GG::GUI::GetGUI()->Register(m_credits_wnd);
}

void IntroScreen::OnExitGame()
{
    delete m_credits_wnd;
    m_credits_wnd = 0;

    GG::GUI::GetGUI()->Exit(0);
}

void IntroScreen::KeyPress (GG::Key key, GG::Flags<GG::ModKey> mod_keys)
{
    if (key == GG::GGK_ESCAPE)
        OnExitGame();
}

void IntroScreen::Close()
{ OnExitGame(); }

void IntroScreen::Render()
{
    boost::shared_ptr<GG::Font> font = HumanClientApp::GetApp()->GetFont(ClientUI::Font(), ClientUI::Pts());
    CUIWnd::Render();
    GG::Pt size = font->TextExtent(FreeOrionVersionString());
    font->RenderText(GG::GUI::GetGUI()->AppWidth()-size.x,
                     GG::GUI::GetGUI()->AppHeight()-size.y,
                     FreeOrionVersionString());
}
