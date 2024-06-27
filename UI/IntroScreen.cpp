#include "IntroScreen.h"

#include "../client/human/GGHumanClientApp.h"

#include "About.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "OptionsWnd.h"
#include "EncyclopediaDetailPanel.h"
#include "../network/Message.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/OptionsDB.h"
#include "../util/Version.h"
#include "../util/XMLDoc.h"

#include <GG/GUI.h>
#include <GG/StaticGraphic.h>
#include <GG/Texture.h>
#include <GG/Layout.h>

#include <boost/filesystem/fstream.hpp>

#include <cstdlib>
#include <string>

namespace {
    constexpr GG::X MAIN_MENU_WIDTH{200};
    constexpr GG::Y MAIN_MENU_HEIGHT{450};

    void Options(OptionsDB& db) {
        db.AddFlag("network.server.external.force", UserStringNop("OPTIONS_DB_FORCE_EXTERNAL_SERVER"),  false);
        db.Add<std::string>("network.server.uri",   UserStringNop("OPTIONS_DB_EXTERNAL_SERVER_ADDRESS"),"localhost");
        db.Add("ui.intro.menu.center.x",            UserStringNop("OPTIONS_DB_UI_MAIN_MENU_X"),         0.75,           RangedStepValidator<double>(0.01, 0.0, 1.0));
        db.Add("ui.intro.menu.center.y",            UserStringNop("OPTIONS_DB_UI_MAIN_MENU_Y"),         0.5,            RangedStepValidator<double>(0.01, 0.0, 1.0));
        db.Add("version.gl.check.done",             UserStringNop("OPTIONS_DB_CHECKED_GL_VERSION"),     false);
    }
    bool foo_bool = RegisterOptions(&Options);
}

/////////////////////////////////
// CreditsWnd
/////////////////////////////////

/** Displays scrolling credits. */
class CreditsWnd : public GG::Wnd {
public:
    CreditsWnd(GG::X x, GG::Y y, GG::X w, GG::Y h, int cx, int cy, int cw, int ch, int co);
    ~CreditsWnd();

    void Render() override;

    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override
    { OnExit(); }
    void MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) override
    { m_scroll_offset -= move * 2000; }

    void KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override {
        if (key == GG::Key::GGK_ESCAPE)
            OnExit();
    }

private:
    void DrawCredits(GG::X x1, GG::Y y1, GG::X x2, GG::Y y2);
    void OnExit();

    std::string                 m_credits;
    int                         m_cx, m_cy, m_cw, m_ch, m_co;
    int                         m_start_time;
    int                         m_scroll_offset = 0;
    int                         m_render = true;
    int                         m_display_list_id = 0;
    int                         m_credits_height = 0;
    std::shared_ptr<GG::Font>   m_font;
};

CreditsWnd::CreditsWnd(GG::X x, GG::Y y, GG::X w, GG::Y h, int cx, int cy, int cw, int ch, int co) :
    GG::Wnd(x, y, w, h, GG::INTERACTIVE | GG::MODAL),
    m_cx(cx),
    m_cy(cy),
    m_cw(cw),
    m_ch(ch),
    m_co(co),
    m_start_time(GG::GUI::GetGUI()->Ticks())
{
    m_font = ClientUI::GetFont(static_cast<int>(ClientUI::Pts()*1.3));

    /** Handle app resizing by closing the credits window. */
    GG::GUI::GetGUI()->WindowResizedSignal.connect(
        boost::bind(&CreditsWnd::OnExit, this));

    XMLDoc doc;
    boost::filesystem::ifstream ifs(GetResourceDir() / "credits.xml");
    doc.ReadDoc(ifs);
    ifs.close();

    if (!doc.root_node.ContainsChild("CREDITS"))
        return;

    const auto& credits_node = doc.root_node.Child("CREDITS");

    std::ostringstream credits;

    auto group_format = boost::format("%1%\n\n");
    auto nick_format = boost::format(" <rgba 153 153 153 255>(%1%)</rgba>");
    auto task_format = boost::format(" - <rgba 204 204 204 255>%1%</rgba>");
    auto resource_format = boost::format("%1% - <rgba 153 153 153 255>%2%</rgba>\n%3% %4%%5%\n");
    auto source_format = boost::format(" - <rgba 153 153 153 255>%1%</rgba>");
    auto note_format = boost::format("<rgba 204 204 204 255>(%1%)</rgba>\n");

    for (const XMLElement& group : credits_node.Children()) {
        if (0 == group.Tag().compare("GROUP")) {
            credits << group_format % group.Attribute("name");
            for (const XMLElement& item : group.Children()) {
                if (0 == item.Tag().compare("PERSON")) {
                    if (item.HasAttribute("name"))
                        credits << item.Attribute("name");
                    if (item.HasAttribute("nick") && !item.Attribute("nick").empty())
                        credits << nick_format % item.Attribute("nick");
                    if (item.HasAttribute("task"))
                        credits << task_format % item.Attribute("task");
                }

                if (0 == item.Tag().compare("RESOURCE")) {
                    credits << resource_format
                        % item.Attribute("author")
                        % item.Attribute("title")
                        % UserString("INTRO_CREDITS_LICENSE")
                        % item.Attribute("license")
                        % ((item.HasAttribute("source"))
                            ? boost::str(source_format % item.Attribute("source"))
                            : std::string{});
                    if (item.HasAttribute("notes"))
                        credits << note_format % item.Attribute("notes");
                }

                credits << "\n";
            }

            credits << "\n\n";
        }
    }

    m_credits = credits.str();
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
    m_modal_done.store(true);
}

void CreditsWnd::DrawCredits(GG::X x1, GG::Y y1, GG::X x2, GG::Y y2) {
    static constexpr auto format = GG::FORMAT_CENTER | GG::FORMAT_TOP;

    //start color is white (name), this color is valid outside the rgba tags

    GG::Font::RenderState rs{GG::CLR_WHITE};
    const auto text_elements = m_font->ExpensiveParseFromTextToTextElements(m_credits, format);
    const auto lines = m_font->DetermineLines(m_credits, format, x2 - x1, text_elements);
    m_font->RenderText(GG::Pt(x1, y1), GG::Pt(x2, y2), m_credits, format, lines, rs);
    const auto offset = m_font->TextExtent(lines).y;
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
        DrawCredits(ul.x + m_cx, ul.y + m_cy, ul.x + m_cx + m_cw, ul.y + m_cy + m_ch);
        glEndList();
    }
    //time passed
    int ticks_delta = GG::GUI::GetGUI()->Ticks() - m_start_time + m_scroll_offset;

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
        DrawCredits(ul.x + m_cx, ul.y + m_cy, ul.x + m_cx + m_cw, ul.y + m_cy + m_ch);
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
    GG::Wnd(GG::X0, GG::Y0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight(), GG::NO_WND_FLAGS)
{}

void IntroScreen::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    m_menu = GG::Wnd::Create<CUIWnd>(UserString("INTRO_WINDOW_TITLE"), GG::X1, GG::Y1,
                                     MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT, GG::ONTOP | GG::INTERACTIVE);

    auto today = boost::gregorian::day_clock::local_day();
    if (today.month() == 4 && today.day() == 1) {
        m_splash = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::GetTexture(ClientUI::ArtDir() / "splash0104.png"),
                                                      GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE,
                                                      GG::INTERACTIVE);
        m_logo = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::GetTexture(ClientUI::ArtDir() / "logo0104.png"),
                                                    GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

    } else if (today.month() == 12 && today.day() == 25) {
        m_splash = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::GetTexture(ClientUI::ArtDir() / "splash2512.png"),
                                                      GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE,
                                                      GG::INTERACTIVE);
        m_logo = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::GetTexture(ClientUI::ArtDir() / "logo2512.png"),
                                                    GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

    } else if (today.month() == 10 && today.day() == 31) {
            m_splash = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::GetTexture(ClientUI::ArtDir() / "splash3110.png"),
                                                          GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE,
                                                          GG::INTERACTIVE);
            m_logo = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::GetTexture(ClientUI::ArtDir() / "logo3110.png"),
                                                        GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

    } else {
        m_splash = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::GetTexture(ClientUI::ArtDir() / "splash.png"),
                                                      GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE,
                                                      GG::INTERACTIVE);

        auto now{boost::posix_time::second_clock::local_time()};
        if (now.time_of_day().seconds() == 42)
            m_logo = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::GetTexture(ClientUI::ArtDir() / "logo0104.png"),
                                                        GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        else
            m_logo = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::GetTexture(ClientUI::ArtDir() / "logo.png"),
                                                        GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    }

    m_version = GG::Wnd::Create<CUILabel>(FreeOrionVersionString(), GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    m_version->MoveTo(GG::Pt(Width() - m_version->Width(), Height() - m_version->Height()));

    AttachChild(m_splash);
    m_splash->AttachChild(m_logo);
    m_splash->AttachChild(m_menu);
    m_splash->AttachChild(m_version);

    //create buttons
    m_continue =      Wnd::Create<CUIButton>(UserString("INTRO_BTN_CONTINUE"));
    m_single_player = Wnd::Create<CUIButton>(UserString("INTRO_BTN_SINGLE_PLAYER"));
    m_quick_start =   Wnd::Create<CUIButton>(UserString("INTRO_BTN_QUICK_START"));
    m_multi_player =  Wnd::Create<CUIButton>(UserString("INTRO_BTN_MULTI_PLAYER"));
    m_load_game =     Wnd::Create<CUIButton>(UserString("INTRO_BTN_LOAD_GAME"));
    m_options =       Wnd::Create<CUIButton>(UserString("INTRO_BTN_OPTIONS"));
    m_pedia =         Wnd::Create<CUIButton>(UserString("INTRO_BTN_PEDIA"));
    m_about =         Wnd::Create<CUIButton>(UserString("INTRO_BTN_ABOUT"));
    m_website =       Wnd::Create<CUIButton>(UserString("INTRO_BTN_WEBSITE"));
    m_credits =       Wnd::Create<CUIButton>(UserString("INTRO_BTN_CREDITS"));
    m_exit_game =     Wnd::Create<CUIButton>(UserString("INTRO_BTN_EXIT"));

    //connect signals and slots
    m_continue->LeftClickedSignal.connect(      boost::bind(&IntroScreen::OnContinue, this));
    m_single_player->LeftClickedSignal.connect( boost::bind(&IntroScreen::OnSinglePlayer, this));
    m_quick_start->LeftClickedSignal.connect(   boost::bind(&IntroScreen::OnQuickStart, this));
    m_multi_player->LeftClickedSignal.connect(  boost::bind(&IntroScreen::OnMultiPlayer, this));
    m_load_game->LeftClickedSignal.connect(     boost::bind(&IntroScreen::OnLoadGame, this));
    m_options->LeftClickedSignal.connect(       boost::bind(&IntroScreen::OnOptions, this));
    m_pedia->LeftClickedSignal.connect(         boost::bind(&IntroScreen::OnPedia, this));
    m_about->LeftClickedSignal.connect(         boost::bind(&IntroScreen::OnAbout, this));
    m_website->LeftClickedSignal.connect(       boost::bind(&IntroScreen::OnWebsite, this));
    m_credits->LeftClickedSignal.connect(       boost::bind(&IntroScreen::OnCredits, this));
    m_exit_game->LeftClickedSignal.connect(     boost::bind(&IntroScreen::OnExitGame, this));

    auto button_list = {m_continue, m_single_player, m_quick_start, m_multi_player, m_load_game,
                        m_options, m_pedia, m_about, m_website, m_credits, std::shared_ptr<GG::Button>(), m_exit_game};
    GG::X needed_width = GG::X1;
    GG::Y needed_height = GG::Y1;
    for (auto& b : button_list) {
        if (!b)
            continue;
        needed_width = std::max(needed_width, b->MinUsableSize().x);
        needed_height = std::max(needed_height, b->MinUsableSize().y);
    }
    int PAD = Value(needed_width) / 16;
    bool have_load = GGHumanClientApp::GetApp()->IsLoadGameAvailable();

    //Layout(X x, Y y, X w, Y h, std::size_t rows, std::size_t columns,
    //       unsigned int border_margin = 0, unsigned int cell_margin = INVALID_CELL_MARGIN);
    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, GG::X1, GG::Y1, 12, 1, PAD);
    layout->SetMinimumColumnWidth(0, needed_width + PAD*2);
    std::size_t row_idx = 0;
    for (auto& b : button_list) {
        if (!b) {   // empty row to separate exit from other buttons
            layout->SetMinimumRowHeight(row_idx, needed_height);

        } else if (b == m_continue || b == m_load_game) {
            if (have_load) {            // standard spacing when buttons shown
                b->Resize(GG::Pt(needed_width + PAD, needed_height));
                layout->SetMinimumRowHeight(row_idx, needed_height + PAD);
                layout->Add(b, row_idx, 0, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
            } else if (row_idx > 0) {   // smaller gap for hidden buttons, except don't need extra gap if at top
                layout->SetMinimumRowHeight(row_idx, GG::Y(PAD));
            }

        } else {    // standard spacing for most buttons
            b->Resize(GG::Pt(needed_width + PAD, needed_height));
            layout->SetMinimumRowHeight(row_idx, needed_height + PAD);
            layout->Add(b, row_idx, 0, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
        }

        row_idx++;
    }
    m_menu->SetLayout(layout);

    RequirePreRender();
}

void IntroScreen::OnContinue()
{ GGHumanClientApp::GetApp()->ContinueSinglePlayerGame(); }

void IntroScreen::OnSinglePlayer()
{ GGHumanClientApp::GetApp()->NewSinglePlayerGame(); }

void IntroScreen::OnQuickStart()
{ GGHumanClientApp::GetApp()->NewSinglePlayerGame(true); }

void IntroScreen::OnMultiPlayer()
{ GGHumanClientApp::GetApp()->MultiPlayerGame(); }

void IntroScreen::OnLoadGame()
{ GGHumanClientApp::GetApp()->LoadSinglePlayerGame(); }

void IntroScreen::OnOptions() {
    auto options_wnd = GG::Wnd::Create<OptionsWnd>(false);
    options_wnd->Run();
}

void IntroScreen::OnPedia() {
    static constexpr std::string_view INTRO_PEDIA_WND_NAME = "intro.pedia";
    auto enc_panel = GG::Wnd::Create<EncyclopediaDetailPanel>(
        GG::MODAL | GG::INTERACTIVE | GG::DRAGABLE |
        GG::RESIZABLE | CLOSABLE | PINABLE, INTRO_PEDIA_WND_NAME);
    enc_panel->InitSizeMove(GG::Pt(GG::X(100), GG::Y(100)), Size() - GG::Pt(GG::X(100), GG::Y(100)));
    enc_panel->ClearItems();
    enc_panel->SetIndex();
    enc_panel->ValidatePosition();

    enc_panel->ClosingSignal.connect(boost::bind(&EncyclopediaDetailPanel::EndRun, enc_panel));

    enc_panel->Run();
}

void IntroScreen::OnAbout() {
    auto about_wnd = GG::Wnd::Create<About>();
    about_wnd->Run();
}

void IntroScreen::OnWebsite()
{ GGHumanClientApp::GetApp()->OpenURL("https://freeorion.org"); }

void IntroScreen::OnCredits() {
    // only the area between the upper and lower line of the splash screen should be darkend
    // if we use another splash screen we have the change the following values
    GG::Y nUpperLine = ( 79 * GG::GUI::GetGUI()->AppHeight()) / 768;
    GG::Y nLowerLine = (692 * GG::GUI::GetGUI()->AppHeight()) / 768;

    static constexpr int credit_side_pad(30);

    auto credits_wnd = GG::Wnd::Create<CreditsWnd>(
        GG::X0, nUpperLine, GG::GUI::GetGUI()->AppWidth(), nLowerLine-nUpperLine,
        credit_side_pad, 0, Value(m_menu->Left()) - credit_side_pad,
        Value(nLowerLine-nUpperLine), Value((nLowerLine-nUpperLine))/2);

    credits_wnd->Run();
}

void IntroScreen::OnExitGame() {
    DebugLogger() << "IntroScreen::OnExitGame";
    GG::GUI::GetGUI()->ExitApp(0);
}

void IntroScreen::KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if (key == GG::Key::GGK_ESCAPE)
        OnExitGame();
}

void IntroScreen::Close()
{ OnExitGame(); }

void IntroScreen::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        RequirePreRender();
}

void IntroScreen::Render()
{}

void IntroScreen::PreRender() {
    GG::Wnd::PreRender();

    m_splash->Resize(this->Size());
    m_logo->Resize(GG::Pt(this->Width(), this->Height() / 8));
    m_version->MoveTo(GG::Pt(this->Width() - m_version->Width(), this->Height() - m_version->Height()));

    auto layout = m_menu->GetLayout();
    if (!layout)
        return;

    // position menu window
    GG::Pt ul(GG::ToX(Width()  * GetOptionsDB().Get<double>("ui.intro.menu.center.x")) - layout->MinUsableSize().x / 2,
              GG::ToY(Height() * GetOptionsDB().Get<double>("ui.intro.menu.center.y")) - layout->MinUsableSize().y / 2);
    GG::Pt lr(GG::ToX(Width()  * GetOptionsDB().Get<double>("ui.intro.menu.center.x")) + layout->MinUsableSize().x / 2,
              GG::ToY(Height() * GetOptionsDB().Get<double>("ui.intro.menu.center.y")) + layout->MinUsableSize().y / 2);

    m_menu->InitSizeMove(ul, lr);
}
