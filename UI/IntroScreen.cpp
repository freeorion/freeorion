//IntroScreen.cpp
#include "IntroScreen.h"

#include "../client/human/HumanClientApp.h"

#include "About.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "OptionsWnd.h"
#include "EncyclopediaDetailPanel.h"
#include "../network/Message.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/OptionsDB.h"
#include "../util/Serialize.h"
#include "../util/Version.h"
#include "../util/XMLDoc.h"

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/Texture.h>

#include <boost/filesystem/fstream.hpp>

#include <cstdlib>
#include <fstream>
#include <string>

namespace {
    const GG::X MAIN_MENU_WIDTH(200);
    const GG::Y MAIN_MENU_HEIGHT(450);

    void Options(OptionsDB& db) {
        db.AddFlag("force-external-server",             UserStringNop("OPTIONS_DB_FORCE_EXTERNAL_SERVER"),     false);
        db.Add<std::string>("external-server-address",  UserStringNop("OPTIONS_DB_EXTERNAL_SERVER_ADDRESS"),   "localhost");
        db.Add("UI.main-menu.x",                        UserStringNop("OPTIONS_DB_UI_MAIN_MENU_X"),            0.75,   RangedStepValidator<double>(0.01, 0.0, 1.0));
        db.Add("UI.main-menu.y",                        UserStringNop("OPTIONS_DB_UI_MAIN_MENU_Y"),            0.5,    RangedStepValidator<double>(0.01, 0.0, 1.0));
        db.Add("checked-gl-version",                    UserStringNop("OPTIONS_DB_CHECKED_GL_VERSION"),        false);
    }
    bool foo_bool = RegisterOptions(&Options);
}

/////////////////////////////////
// CreditsWnd
/////////////////////////////////

/** Displays scrolling credits. */
class CreditsWnd : public GG::Wnd {
public:
    CreditsWnd(GG::X x, GG::Y y, GG::X w, GG::Y h, const XMLElement &credits, int cx, int cy, int cw, int ch, int co);
    ~CreditsWnd();

    virtual void    Render();
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
    { OnExit(); }
    virtual void    KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
    {
        if (key == GG::GGK_ESCAPE)
            OnExit();
    }


private:
    void            DrawCredits(GG::X x1, GG::Y y1, GG::X x2, GG::Y y2, int transparency);
    void            OnExit();

    XMLElement                  m_credits;
    int                         m_cx, m_cy, m_cw, m_ch, m_co;
    int                         m_start_time;
    int                         m_render;
    int                         m_display_list_id;
    int                         m_credits_height;
    boost::shared_ptr<GG::Font> m_font;
};

CreditsWnd::CreditsWnd(GG::X x, GG::Y y, GG::X w, GG::Y h, const XMLElement &credits, int cx, int cy, int cw, int ch, int co) :
    GG::Wnd(x, y, w, h, GG::INTERACTIVE | GG::MODAL),
    m_credits(credits),
    m_cx(cx),
    m_cy(cy),
    m_cw(cw),
    m_ch(ch),
    m_co(co),
    m_start_time(GG::GUI::GetGUI()->Ticks()),
    m_render(true),
    m_display_list_id(0),
    m_credits_height(0)
{
    m_font = ClientUI::GetFont(static_cast<int>(ClientUI::Pts()*1.3));
}

CreditsWnd::~CreditsWnd() {
    if (m_display_list_id != 0)
        glDeleteLists(m_display_list_id, 1);
}

void CreditsWnd::OnExit() {
    m_render = false;
    if (m_display_list_id != 0) {
        glDeleteLists(m_display_list_id, 1);
        m_display_list_id = 0;
    }
    m_done = true;
}

void CreditsWnd::DrawCredits(GG::X x1, GG::Y y1, GG::X x2, GG::Y y2, int transparency) {
    GG::Flags<GG::TextFormat> format = GG::FORMAT_CENTER | GG::FORMAT_TOP;

    //offset starts with 0, credits are place by transforming the viewport
    GG::Y offset(0);

    //start color is white (name), this color is valid outside the rgba tags
    glColor(GG::Clr(transparency, transparency, transparency, 255));

    std::string credit;
    for (const XMLElement& group : m_credits.children) {
        if (0 == group.Tag().compare("GROUP")) {
            for (const XMLElement& person : group.children) {
                if (0 == person.Tag().compare("PERSON")) {
                    credit = "";
                    if (person.attributes.count("name"))
                        credit += person.attributes.at("name");
                    if (person.attributes.count("nick") && person.attributes.at("nick").length() > 0) {
                        credit += " <rgba 153 153 153 " + boost::lexical_cast<std::string>(transparency) +">(";
                        credit += person.attributes.at("nick");
                        credit += ")</rgba>";
                    }
                    if (person.attributes.count("task") && person.attributes.at("task").length() > 0) {
                        credit += " - <rgba 204 204 204 " + boost::lexical_cast<std::string>(transparency) +">";
                        credit += person.attributes.at("task");
                        credit += "</rgba>";
                    }
                    std::vector<boost::shared_ptr<GG::Font::TextElement> > text_elements =
                        m_font->ExpensiveParseFromTextToTextElements(credit, format);
                    std::vector<GG::Font::LineData> lines =
                        m_font->DetermineLines(credit, format, x2 - x1, text_elements);
                    m_font->RenderText(GG::Pt(x1, y1 + offset), GG::Pt(x2, y2), credit, format, lines);
                    offset += m_font->TextExtent(lines).y + 2;
                }
            }
            offset += m_font->Lineskip() + 2;
        }
    }
    //store complete height for self destruction
    m_credits_height = Value(offset);
}

void CreditsWnd::Render() {
    if (!m_render)
        return;
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (m_display_list_id == 0) {
        // compile credits
        m_display_list_id = glGenLists(1);
        glNewList(m_display_list_id, GL_COMPILE);
        DrawCredits(ul.x + m_cx, ul.y + m_cy, ul.x + m_cx + m_cw, ul.y + m_cy + m_ch, 255);
        glEndList();
    }
    //time passed
    int ticks_delta = GG::GUI::GetGUI()->Ticks() - m_start_time;

    //draw background
    GG::FlatRectangle(ul, lr, GG::FloatClr(0.0f, 0.0f, 0.0f, 0.5f), GG::CLR_ZERO, 0);

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();

    // define clip area
    glEnable(GL_SCISSOR_TEST);
    glScissor(Value(ul.x + m_cx), Value(GG::GUI::GetGUI()->AppHeight() - lr.y), m_cw, m_ch);

    // move credits
    glTranslatef(0, m_co - ticks_delta/40, 0);

    if (m_display_list_id != 0) {
        // draw credits using prepared display list
        // !!! in order for the display list to be valid, the font object (m_font) may not be destroyed !!!
        glCallList(m_display_list_id);
    } else {
        // draw credits directly
        DrawCredits(ul.x + m_cx, ul.y + m_cy, ul.x + m_cx + m_cw, ul.y + m_cy + m_ch, 255);
    }

    glPopMatrix();
    glPopAttrib();

    //check if we are done
    if (m_credits_height + m_ch < m_co + ticks_delta/40)
        OnExit();
}


/////////////////////////////////
// IntroScreen
/////////////////////////////////
IntroScreen::IntroScreen() :
    GG::Wnd(GG::X0, GG::Y0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight(), GG::NO_WND_FLAGS),
    m_single_player(0),
    m_quick_start(0),
    m_multi_player(0),
    m_load_game(0),
    m_options(0),
    m_pedia(0),
    m_about(0),
    m_website(0),
    m_credits(0),
    m_exit_game(0),
    m_menu(0),
    m_splash(0),
    m_logo(0),
    m_version(0)
{
    m_menu = new CUIWnd(UserString("INTRO_WINDOW_TITLE"), GG::X1, GG::Y1,
                        MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT, GG::ONTOP | GG::INTERACTIVE);

    m_splash = new GG::StaticGraphic(ClientUI::GetTexture(ClientUI::ArtDir() / "splash.png"), GG::GRAPHIC_FITGRAPHIC, GG::INTERACTIVE);

    m_logo = new GG::StaticGraphic(ClientUI::GetTexture(ClientUI::ArtDir() / "logo.png"), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

    m_version = new CUILabel(FreeOrionVersionString(), GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    m_version->MoveTo(GG::Pt(Width() - m_version->Width(), Height() - m_version->Height()));

    AttachChild(m_splash);
    m_splash->AttachChild(m_logo);
    m_splash->AttachChild(m_menu);
    m_splash->AttachChild(m_version);

    //create buttons
    m_single_player = new CUIButton(UserString("INTRO_BTN_SINGLE_PLAYER"));
    m_quick_start =   new CUIButton(UserString("INTRO_BTN_QUICK_START"));
    m_multi_player =  new CUIButton(UserString("INTRO_BTN_MULTI_PLAYER"));
    m_load_game =     new CUIButton(UserString("INTRO_BTN_LOAD_GAME"));
    m_options =       new CUIButton(UserString("INTRO_BTN_OPTIONS"));
    m_pedia =         new CUIButton(UserString("INTRO_BTN_PEDIA"));
    m_about =         new CUIButton(UserString("INTRO_BTN_ABOUT"));
    m_website =       new CUIButton(UserString("INTRO_BTN_WEBSITE"));
    m_credits =       new CUIButton(UserString("INTRO_BTN_CREDITS"));
    m_exit_game =     new CUIButton(UserString("INTRO_BTN_EXIT"));

    //attach buttons
    m_menu->AttachChild(m_single_player);
    m_menu->AttachChild(m_quick_start);
    m_menu->AttachChild(m_multi_player);
    m_menu->AttachChild(m_load_game);
    m_menu->AttachChild(m_options);
    m_menu->AttachChild(m_pedia);
    m_menu->AttachChild(m_about);
    m_menu->AttachChild(m_website);
    m_menu->AttachChild(m_credits);
    m_menu->AttachChild(m_exit_game);

    //connect signals and slots
    GG::Connect(m_single_player->LeftClickedSignal, &IntroScreen::OnSinglePlayer,   this);
    GG::Connect(m_quick_start->LeftClickedSignal,   &IntroScreen::OnQuickStart,     this);
    GG::Connect(m_multi_player->LeftClickedSignal,  &IntroScreen::OnMultiPlayer,    this);
    GG::Connect(m_load_game->LeftClickedSignal,     &IntroScreen::OnLoadGame,       this);
    GG::Connect(m_options->LeftClickedSignal,       &IntroScreen::OnOptions,        this);
    GG::Connect(m_pedia->LeftClickedSignal,         &IntroScreen::OnPedia,          this);
    GG::Connect(m_about->LeftClickedSignal,         &IntroScreen::OnAbout,          this);
    GG::Connect(m_website->LeftClickedSignal,       &IntroScreen::OnWebsite,        this);
    GG::Connect(m_credits->LeftClickedSignal,       &IntroScreen::OnCredits,        this);
    GG::Connect(m_exit_game->LeftClickedSignal,     &IntroScreen::OnExitGame,       this);

    DoLayout();
}

IntroScreen::~IntroScreen() {
    delete m_splash;
    // m_menu, m_version, m_logo were childs of m_splash, so don't need to be deleted here
}

void IntroScreen::OnSinglePlayer() {
    HumanClientApp::GetApp()->NewSinglePlayerGame();
}

void IntroScreen::OnQuickStart() {
    HumanClientApp::GetApp()->NewSinglePlayerGame(true);
}

void IntroScreen::OnMultiPlayer() {
    HumanClientApp::GetApp()->MultiPlayerGame();
}

void IntroScreen::OnLoadGame() {
    HumanClientApp::GetApp()->LoadSinglePlayerGame();
}

void IntroScreen::OnOptions() {
    OptionsWnd options_wnd;
    options_wnd.Run();
}

void IntroScreen::OnPedia() {
    static const std::string INTRO_PEDIA_WND_NAME = "introscreen.pedia";
    EncyclopediaDetailPanel enc_panel(GG::MODAL | GG::INTERACTIVE | GG::DRAGABLE |
                                      GG::RESIZABLE | CLOSABLE | PINABLE, INTRO_PEDIA_WND_NAME);
    enc_panel.SizeMove(GG::Pt(GG::X(100), GG::Y(100)), Size() - GG::Pt(GG::X(100), GG::Y(100)));
    enc_panel.ClearItems();
    enc_panel.SetIndex();
    enc_panel.ValidatePosition();

    GG::Connect(enc_panel.ClosingSignal, &EncyclopediaDetailPanel::EndRun, &enc_panel);

    enc_panel.Run();
}

void IntroScreen::OnAbout() {
    About about_wnd;
    about_wnd.Run();
}

void IntroScreen::OnWebsite()
{ HumanClientApp::GetApp()->OpenURL("http://freeorion.org"); }

void IntroScreen::OnCredits() {
    XMLDoc doc;
    boost::filesystem::ifstream ifs(GetResourceDir() / "credits.xml");
    doc.ReadDoc(ifs);
    ifs.close();


    if (!doc.root_node.ContainsChild("CREDITS"))
        return;

    XMLElement credits = doc.root_node.Child("CREDITS");
    // only the area between the upper and lower line of the splash screen should be darkend
    // if we use another splash screen we have the change the following values
    GG::Y nUpperLine = ( 79 * GG::GUI::GetGUI()->AppHeight()) / 768;
    GG::Y nLowerLine = (692 * GG::GUI::GetGUI()->AppHeight()) / 768;

    int credit_side_pad(30);

    CreditsWnd credits_wnd(GG::X0, nUpperLine, GG::GUI::GetGUI()->AppWidth(), nLowerLine-nUpperLine,
                           credits,
                           credit_side_pad, 0, Value(m_menu->Left()) - credit_side_pad,
                           Value(nLowerLine-nUpperLine), Value((nLowerLine-nUpperLine))/2);

    credits_wnd.Run();
}

void IntroScreen::OnExitGame() {
    GG::GUI::GetGUI()->Exit(0);
}

void IntroScreen::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if (key == GG::GGK_ESCAPE)
        OnExitGame();
}

void IntroScreen::Close()
{ OnExitGame(); }

void IntroScreen::Render()
{}

void IntroScreen::DoLayout() {
    m_splash->Resize(this->Size());
    m_logo->Resize(GG::Pt(this->Width(), this->Height() / 10));
    m_version->MoveTo(GG::Pt(this->Width() - m_version->Width(), this->Height() - m_version->Height()));

    //size calculation consts and variables
    const GG::X MIN_BUTTON_WIDTH(160);
    const GG::Y MIN_BUTTON_HEIGHT(40);
    GG::X button_width(0);              //width of the buttons
    GG::Y button_cell_height(0);        //height of the buttons
    const GG::X H_MAINMENU_MARGIN(40);  //horizontal empty space
    const GG::Y V_MAINMENU_MARGIN(40);  //vertical empty space
    GG::X mainmenu_width(0);            //width of the mainmenu
    GG::Y mainmenu_height(0);           //height of the mainmenu

    //calculate necessary button width
    button_width = std::max(button_width, m_single_player->MinUsableSize().x);
    button_width = std::max(button_width, m_quick_start->MinUsableSize().x);
    button_width = std::max(button_width, m_multi_player->MinUsableSize().x);
    button_width = std::max(button_width, m_load_game->MinUsableSize().x);
    button_width = std::max(button_width, m_options->MinUsableSize().x);
    button_width = std::max(button_width, m_pedia->MinUsableSize().x);
    button_width = std::max(button_width, m_about->MinUsableSize().x);
    button_width = std::max(button_width, m_website->MinUsableSize().x);
    button_width = std::max(button_width, m_credits->MinUsableSize().x);
    button_width = std::max(button_width, m_exit_game->MinUsableSize().x);
    button_width = std::max(MIN_BUTTON_WIDTH, button_width);

    //calculate  necessary button height
    button_cell_height = std::max(MIN_BUTTON_HEIGHT, m_exit_game->MinUsableSize().y);
    //culate window width and height
    mainmenu_width  =         button_width  + H_MAINMENU_MARGIN;
    mainmenu_height = 10.75 * button_cell_height + V_MAINMENU_MARGIN; // 10 rows + 0.75 before exit button

    // place buttons
    GG::Pt button_ul(GG::X(15), GG::Y(12));
    GG::Pt button_lr(button_width, m_exit_game->MinUsableSize().y);

    button_lr += button_ul;

    m_single_player->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height);
    button_lr.y += GG::Y(button_cell_height);
    m_quick_start->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height);
    button_lr.y += GG::Y(button_cell_height);
    m_multi_player->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height);
    button_lr.y += GG::Y(button_cell_height);
    m_load_game->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height);
    button_lr.y += GG::Y(button_cell_height);
    m_options->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height);
    button_lr.y += GG::Y(button_cell_height);
    m_pedia->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height);
    button_lr.y += GG::Y(button_cell_height);
    m_about->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height);
    button_lr.y += GG::Y(button_cell_height);
    m_website->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height);
    button_lr.y += GG::Y(button_cell_height);
    m_credits->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height) * 1.75;
    button_lr.y += GG::Y(button_cell_height) * 1.75;
    m_exit_game->SizeMove(button_ul, button_lr);

    // position menu window
    GG::Pt ul(Width()  * GetOptionsDB().Get<double>("UI.main-menu.x") - mainmenu_width/2,
              Height() * GetOptionsDB().Get<double>("UI.main-menu.y") - mainmenu_height/2);
    GG::Pt lr(Width()  * GetOptionsDB().Get<double>("UI.main-menu.x") + mainmenu_width/2,
              Height() * GetOptionsDB().Get<double>("UI.main-menu.y") + mainmenu_height/2);

    m_menu->SizeMove(ul, lr);
}
