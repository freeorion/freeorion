//MapWnd.cpp

#include "MapWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../universe/Fleet.h"
#include "FleetButton.h"
#include "FleetWindow.h"
#include "GGDrawUtil.h"
#include "GGMultiEdit.h"
#include "../client/human/HumanClientApp.h"
#include "../network/Message.h"
#include "../util/OptionsDB.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../util/Random.h"
#include "SidePanel.h"
#include "SitRepPanel.h"
#include "../universe/System.h"
#include "SystemIcon.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "TurnProgressWnd.h"

#include <vector>
#include <deque>

namespace {
    const double ZOOM_STEP_SIZE = 1.25;
    const int NUM_NEBULA_TEXTURES = 5;
    const int MIN_NEBULAE = 3; // this min and max are for a 1000.0-width galaxy
    const int MAX_NEBULAE = 6;
    const int END_TURN_BTN_WIDTH = 60;
    const int SITREP_PANEL_WIDTH = 400;
    const int SITREP_PANEL_HEIGHT = 300;
    const int MIN_SYSTEM_NAME_SIZE = 10;
    int g_chat_display_show_time = 0;
    std::deque<std::string> g_chat_edit_history;
    int g_history_position = 0; // the current edit contents are in history position 0
    void AddOptions(OptionsDB& db)
    {
        db.Add("UI.chat-hide-interval", "Time interval, in seconds, after which the multiplayer chat window will disappear if "
               "nothing is added to it.  A value of 0 indicates that the window should never diappear.", 10, RangedValidator<int>(0, 3600));
        db.Add("UI.chat-edit-history", "The number of outgoing messages to keep in the chat edit box history.", 50, RangedValidator<int>(0, 1000));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    bool temp_header_bool = RecordHeaderFile(MapWndRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


////////////////////////////////////////////////
// MapWnd::StarlaneData
////////////////////////////////////////////////
/* Note that an order is imposed on the two systems the starlane spans.  The "source" system is the one with the lower pointer.
    This is so StarlaneDatas can be stored in a std::set and duplicates will not be a problem. */
struct MapWnd::StarlaneData
{
    StarlaneData() {}
    StarlaneData(const System* src_system, const System* dst_system) : 
        src(std::min(src_system, dst_system)), 
        dst(std::max(src_system, dst_system)) 
        {}
    bool operator<(const StarlaneData& rhs) const 
    {
        if (src != rhs.src) 
            return src < rhs.src; 
        if (dst != rhs.dst) 
            return dst < rhs.dst;
        return false;
    }
    void SetSystems(const System* src_system, const System* dst_system) 
    {
        src = std::max(src_system, dst_system); 
        dst = std::min(src_system, dst_system);
    }

    const System* Src() const {return src;}
    const System* Dst() const {return dst;}

private:
    const System* src;
    const System* dst;
};


////////////////////////////////////////////////
// MapWnd::MovementLineData
////////////////////////////////////////////////
struct MapWnd::MovementLineData
{
    MovementLineData() {}
    MovementLineData(double x_, double y_, const std::list<System*>& dest) : x(x_), y(y_), destinations(dest) {}
    double x;
    double y;
    std::list<System*> destinations;
    // TODO : color, other properties(?), based on moving empire and destination, etc.
};


////////////////////////////////////////////////////////////
// MapWndPopup
////////////////////////////////////////////////////////////

MapWndPopup::MapWndPopup( const std::string& t, int x, int y, int h, int w, Uint32 flags ):
    CUI_Wnd( t, x, y, h, w, flags )
{
    // register with map wnd
    HumanClientApp::GetUI()->GetMapWnd()->RegisterPopup( this );
}

MapWndPopup::MapWndPopup(const GG::XMLElement& elem):
    CUI_Wnd( elem )
{
}

MapWndPopup::~MapWndPopup( )
{
    // remove from map wnd
    HumanClientApp::GetUI()->GetMapWnd()->RemovePopup( this );
}

void MapWndPopup::Close( )
{
    // close window as though it's been clicked closed by the user.
    CloseClicked( );
}

////////////////////////////////////////////////
// MapWnd
////////////////////////////////////////////////
// static(s)
const int MapWnd::NUM_BACKGROUNDS = 3;
double    MapWnd::s_min_scale_factor = 0.5;
double    MapWnd::s_max_scale_factor = 8.0;
const int MapWnd::SIDE_PANEL_WIDTH = 300;

namespace {
    const int MAP_MARGIN_WIDTH = MapWnd::SIDE_PANEL_WIDTH; // the number of pixels of system-less space around all four sides of the starfield
}

MapWnd::MapWnd() :
    GG::Wnd(-GG::App::GetApp()->AppWidth(), -GG::App::GetApp()->AppHeight(),
            static_cast<int>(Universe::UniverseWidth() * s_max_scale_factor) + GG::App::GetApp()->AppWidth() + MAP_MARGIN_WIDTH, 
            static_cast<int>(Universe::UniverseWidth() * s_max_scale_factor) + GG::App::GetApp()->AppHeight() + MAP_MARGIN_WIDTH, 
            GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE),
    m_disabled_accels_list(),
    m_backgrounds(NUM_BACKGROUNDS),
    m_bg_scroll_rate(NUM_BACKGROUNDS),
    m_bg_position_X(NUM_BACKGROUNDS),
    m_bg_position_Y(NUM_BACKGROUNDS),
    m_zoom_factor(1.0),
    m_drag_offset(-1, -1),
    m_dragged(false),
    m_current_owned_system(UniverseObject::INVALID_OBJECT_ID),
    m_current_fleet(UniverseObject::INVALID_OBJECT_ID)
{
    SetText("MapWnd");

    Connect(GetUniverse().UniverseObjectDeleteSignal(), &MapWnd::UniverseObjectDeleted, this);

    // toolbar
    m_toolbar = new CUIToolBar(0,0,GG::App::GetApp()->AppWidth(),30);
    AttachChild(m_toolbar);

    // system-view side panel
    m_side_panel = new SidePanel(GG::App::GetApp()->AppWidth() - SIDE_PANEL_WIDTH, m_toolbar->LowerRight().y, SIDE_PANEL_WIDTH, GG::App::GetApp()->AppHeight());
    AttachChild(m_side_panel);
    Connect(m_left_clicked_system_signal, &SidePanel::SetSystem, m_side_panel);

    m_sitrep_panel = new SitRepPanel( (GG::App::GetApp()->AppWidth()-SITREP_PANEL_WIDTH)/2, (GG::App::GetApp()->AppHeight()-SITREP_PANEL_HEIGHT)/2, SITREP_PANEL_WIDTH, SITREP_PANEL_HEIGHT );
    AttachChild(m_sitrep_panel);


    // turn button
    m_turn_update = new CUIButton(5, 5, END_TURN_BTN_WIDTH, "" );
    m_toolbar->AttachChild(m_turn_update);


    m_btn_menu = new CUIButton(m_toolbar->LowerRight().x-5-40, 5, 40, ClientUI::String("MAP_BTN_MENU") );
    m_toolbar->AttachChild(m_btn_menu);
    GG::Connect(m_btn_menu->ClickedSignal(), &MapWnd::MenuBtnClicked, this);
    
    m_btn_siterep = new CUIButton(m_btn_menu->UpperLeft().x-5-50, 5, 50, ClientUI::String("MAP_BTN_SITREP") );
    m_toolbar->AttachChild(m_btn_siterep);
    GG::Connect(m_btn_siterep->ClickedSignal(), &MapWnd::SiteRepBtnClicked, this);
    
    m_population= new StatisticIconDualValue(m_btn_siterep->UpperLeft().x-5-80,5,80,m_turn_update->Height(),ClientUI::ART_DIR+"icons/pop.png",GG::CLR_WHITE,0,0,0,2,false,false);
    m_population->SetPositiveColor(GG::CLR_GREEN); m_population->SetNegativeColor(GG::CLR_RED);
    m_toolbar->AttachChild(m_population);
   
    m_industry= new StatisticIcon(m_population->UpperLeft().x-5-50,5,50,m_turn_update->Height(),ClientUI::ART_DIR+"icons/industry.png",GG::CLR_WHITE,0);
    m_toolbar->AttachChild(m_industry);

    m_research= new StatisticIcon(m_industry->UpperLeft().x-5-50,5,50,m_turn_update->Height(),ClientUI::ART_DIR+"icons/research.png",GG::CLR_WHITE,0);
    m_toolbar->AttachChild(m_research);

    m_mineral = new StatisticIconDualValue(m_research->UpperLeft().x-5-80,5,80,m_turn_update->Height(),ClientUI::ART_DIR+"icons/mining.png",GG::CLR_WHITE,0,0,0,0,false,false);
    m_mineral->SetPositiveColor(GG::CLR_GREEN); m_mineral->SetNegativeColor(GG::CLR_RED);
    m_toolbar->AttachChild(m_mineral);

    m_food = new StatisticIconDualValue(m_mineral->UpperLeft().x-5-80,5,80,m_turn_update->Height(),ClientUI::ART_DIR+"icons/farming.png",GG::CLR_WHITE,0,0,0,0,false,false);
    m_food->SetPositiveColor(GG::CLR_GREEN); m_food->SetNegativeColor(GG::CLR_RED);
    m_toolbar->AttachChild(m_food);

    // chat display and chat input box
    const int CHAT_WIDTH = 400;
    const int CHAT_HEIGHT = 400;
    m_chat_display = new GG::MultiEdit(5, m_turn_update->LowerRight().y + 5, CHAT_WIDTH, CHAT_HEIGHT, "", ClientUI::FONT, ClientUI::PTS, GG::CLR_ZERO, 
                                       GG::TF_WORDBREAK | GG::MultiEdit::READ_ONLY | GG::MultiEdit::TERMINAL_STYLE | GG::MultiEdit::INTEGRAL_HEIGHT | GG::MultiEdit::NO_VSCROLL, 
                                       ClientUI::TEXT_COLOR, GG::CLR_ZERO, 0);
    AttachChild(m_chat_display);
    m_chat_display->SetMaxLinesOfHistory(100);
    m_chat_display->Hide();

    const int CHAT_EDIT_HEIGHT = 30;
    m_chat_edit = new CUIEdit(5, GG::App::GetApp()->AppHeight() - CHAT_EDIT_HEIGHT - 5, CHAT_WIDTH, CHAT_EDIT_HEIGHT, "", 
                              ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_BORDER_COLOR, ClientUI::TEXT_COLOR, GG::CLR_ZERO);
    AttachChild(m_chat_edit);
    m_chat_edit->Hide();
    EnableAlphaNumAccels();

	m_options_showing = false;

    //set up background images
    m_backgrounds[0].reset(new GG::Texture());
    m_backgrounds[0]->Load(ClientUI::ART_DIR + "starfield1.png");
    m_bg_position_X[0] = 10.0;
    m_bg_position_Y[0] = 10.0;
    m_bg_scroll_rate[0] = 0.125;

    m_backgrounds[1].reset(new GG::Texture());
    m_backgrounds[1]->Load(ClientUI::ART_DIR + "starfield2.png");
    m_bg_position_X[1] = 10.0;
    m_bg_position_Y[1] = 10.0;
    m_bg_scroll_rate[1] = 0.25;

    m_backgrounds[2].reset(new GG::Texture());
    m_backgrounds[2]->Load(ClientUI::ART_DIR + "starfield3.png");
    m_bg_position_X[2] = 10.0;
    m_bg_position_Y[2] = 10.0;
    m_bg_scroll_rate[2] = 0.5;

    // connect signals and slots
    GG::Connect(m_turn_update->ClickedSignal(), &MapWnd::OnTurnUpdate, this);

    // connect keyboard accelerators
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_RETURN, 0), &MapWnd::OpenChatWindow, this);
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_KP_ENTER, 0), &MapWnd::OpenChatWindow, this);

    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_RETURN, GG::GGKMOD_CTRL), &MapWnd::EndTurn, this);
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_KP_ENTER, GG::GGKMOD_CTRL), &MapWnd::EndTurn, this);

    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_F2, 0), &MapWnd::ToggleSitRep, this);
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_F10, 0), &MapWnd::ShowOptions, this);
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_s, 0), &MapWnd::CloseSystemView, this);

    // Keys for zooming
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_e, 0), &MapWnd::KeyboardZoomIn, this);
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_KP_PLUS, 0), &MapWnd::KeyboardZoomIn, this);
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_r, 0), &MapWnd::KeyboardZoomOut, this);
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_KP_MINUS, 0), &MapWnd::KeyboardZoomOut, this);

    // Keys for showing systems
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_d, 0), &MapWnd::ZoomToHomeSystem, this);
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_x, 0), &MapWnd::ZoomToPrevOwnedSystem, this);
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_c, 0), &MapWnd::ZoomToNextOwnedSystem, this);

    // Keys for showing fleets
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_f, 0), &MapWnd::ZoomToPrevIdleFleet, this);
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_g, 0), &MapWnd::ZoomToNextIdleFleet, this);
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_v, 0), &MapWnd::ZoomToPrevFleet, this);
    GG::Connect(GG::App::GetApp()->AcceleratorSignal(GG::GGK_b, 0), &MapWnd::ZoomToNextFleet, this);

    g_chat_edit_history.push_front("");
}

MapWnd::~MapWnd()
{
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_RETURN, 0);
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_KP_ENTER, 0);

    GG::App::GetApp()->RemoveAccelerator(GG::GGK_RETURN, GG::GGKMOD_CTRL);
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_KP_ENTER, GG::GGKMOD_CTRL);

    GG::App::GetApp()->RemoveAccelerator(GG::GGK_F2, 0);
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_F10, 0);
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_s, 0);

    // Zoom keys
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_e, 0);
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_r, 0);
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_KP_PLUS, 0);
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_KP_MINUS, 0);

    // Keys for showing systems
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_d, 0);
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_x, 0);
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_c, 0);

    // Keys for showing fleets
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_f, 0);
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_g, 0);
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_v, 0);
    GG::App::GetApp()->RemoveAccelerator(GG::GGK_b, 0);
}

GG::Pt MapWnd::ClientUpperLeft() const
{
    return UpperLeft() + GG::Pt(GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight());
}

GG::XMLElement MapWnd::SaveGameData() const
{
    GG::XMLElement retval("MapWnd");
    retval.AppendChild(GG::XMLElement("upper_left", UpperLeft().XMLEncode()));
    retval.AppendChild(GG::XMLElement("m_zoom_factor", boost::lexical_cast<std::string>(m_zoom_factor)));
    retval.AppendChild(GG::XMLElement("m_nebulae"));
    for (unsigned int i = 0; i < m_nebulae.size(); ++i) {
        retval.LastChild().AppendChild(GG::XMLElement("nebula" + boost::lexical_cast<std::string>(i)));
        retval.LastChild().LastChild().AppendChild(GG::XMLElement("filename", m_nebulae[i]->Filename()));
        retval.LastChild().LastChild().AppendChild(GG::XMLElement("position", m_nebula_centers[i].XMLEncode()));
    }
    return retval;
}

bool MapWnd::Render()
{
    RenderBackgrounds();
    RenderStarlanes();
    RenderFleetMovementLines();

    int interval = GetOptionsDB().Get<int>("UI.chat-hide-interval");
    if (!m_chat_edit->Visible() && g_chat_display_show_time && interval && 
        (interval < (GG::App::GetApp()->Ticks() - g_chat_display_show_time) / 1000)) {
        m_chat_display->Hide();
        g_chat_display_show_time = 0;
    }

    return true;
}

void MapWnd::Keypress (GG::Key key, Uint32 key_mods)
{
    switch (key) {
    case GG::GGK_TAB: { // auto-complete current chat edit word
        if (m_chat_edit->Visible()) {
            std::string text = m_chat_edit->WindowText();
            std::pair<int, int> cursor_pos = m_chat_edit->CursorPosn();
            if (cursor_pos.first == cursor_pos.second && 0 < cursor_pos.first && cursor_pos.first <= static_cast<int>(text.size())) {
                unsigned int word_start = text.substr(0, cursor_pos.first).find_last_of(" :");
                if (word_start == std::string::npos)
                    word_start = 0;
                else
                    ++word_start;
                std::string partial_word = text.substr(word_start, cursor_pos.first - word_start);
                if (partial_word == "")
                    return;
                std::set<std::string> names;
                // add player and empire names
                for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
                    names.insert(it->second->Name());
                    names.insert(it->second->PlayerName());
                }
                // add system names
                std::vector<System*> systems = GetUniverse().FindObjects<System>();
                for (unsigned int i = 0; i < systems.size(); ++i) {
                    if (systems[i]->Name() != "")
                        names.insert(systems[i]->Name());
                }

                if (names.find(partial_word) != names.end()) { // if there's an exact match, just add a space
                    text.insert(cursor_pos.first, " ");
                    m_chat_edit->SetText(text);
                    m_chat_edit->SelectRange(cursor_pos.first + 1, cursor_pos.first + 1);
                } else { // no exact match; look for possible completions
                    // find the range of strings in names that is at least partially matched by partial_word
                    std::set<std::string>::iterator lower_bound = names.lower_bound(partial_word);
                    std::set<std::string>::iterator upper_bound = lower_bound;
                    while (upper_bound != names.end() && upper_bound->find(partial_word) == 0)
                        ++upper_bound;
                    if (lower_bound == upper_bound)
                        return;

                    // find the common portion of the strings in (upper_bound, lower_bound)
                    unsigned int common_end = partial_word.size();
                    for ( ; common_end < lower_bound->size(); ++common_end) {
                        std::set<std::string>::iterator it = lower_bound;
                        char ch = (*it++)[common_end];
                        bool match = true;
                        for ( ; it != upper_bound; ++it) {
                            if ((*it)[common_end] != ch) {
                                match = false;
                                break;
                            }
                        }
                        if (!match)
                            break;
                    }
                    unsigned int chars_to_add = common_end - partial_word.size();
                    bool full_completion = common_end == lower_bound->size();
                    text.insert(cursor_pos.first, lower_bound->substr(partial_word.size(), chars_to_add) + (full_completion ? " " : ""));
                    m_chat_edit->SetText(text);
                    int move_cursor_to = cursor_pos.first + chars_to_add + (full_completion ? 1 : 0);
                    m_chat_edit->SelectRange(move_cursor_to, move_cursor_to);
                }
            }
        }
        break;
    }
    case GG::GGK_RETURN:
    case GG::GGK_KP_ENTER: { // send chat message
        if (m_chat_edit->Visible()) {
            std::string edit_text = m_chat_edit->WindowText();
            if (edit_text != "") {
                if (g_chat_edit_history.size() == 1 || g_chat_edit_history[1] != edit_text) {
                    g_chat_edit_history[0] = edit_text;
                    g_chat_edit_history.push_front("");
                } else {
                    g_chat_edit_history[0] = "";
                }
                while (GetOptionsDB().Get<int>("UI.chat-edit-history") < static_cast<int>(g_chat_edit_history.size()) + 1)
                    g_chat_edit_history.pop_back();
                g_history_position = 0;
                HumanClientApp::GetApp()->NetworkCore().SendMessage(ChatMessage(HumanClientApp::GetApp()->PlayerID(), edit_text));
            }
            m_chat_edit->Clear();
            m_chat_edit->Hide();
	    EnableAlphaNumAccels();

            GG::App::GetApp()->SetFocusWnd(this);
            g_chat_display_show_time = GG::App::GetApp()->Ticks();
        }
        break;
    }

    case GG::GGK_UP: {
        if (m_chat_edit->Visible() && g_history_position < static_cast<int>(g_chat_edit_history.size()) - 1) {
            g_chat_edit_history[g_history_position] = m_chat_edit->WindowText();
            ++g_history_position;
            m_chat_edit->SetText(g_chat_edit_history[g_history_position]);
        }
        break;
    }

    case GG::GGK_DOWN: {
        if (m_chat_edit->Visible() && 0 < g_history_position) {
            g_chat_edit_history[g_history_position] = m_chat_edit->WindowText();
            --g_history_position;
            m_chat_edit->SetText(g_chat_edit_history[g_history_position]);
        }
        break;
    }

    default:
        break;
    }
}

void MapWnd::LButtonDown (const GG::Pt &pt, Uint32 keys)
{
    m_drag_offset = pt - ClientUpperLeft();
}

void MapWnd::LDrag (const GG::Pt &pt, const GG::Pt &move, Uint32 keys)
{
    GG::Pt move_to_pt = pt - m_drag_offset;
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ClientUpperLeft();
    m_side_panel->OffsetMove(-final_move);
    m_chat_display->OffsetMove(-final_move);
    m_chat_edit->OffsetMove(-final_move);
    m_sitrep_panel->OffsetMove(-final_move);
    m_toolbar->OffsetMove(-final_move);

    MoveBackgrounds(final_move);
    MoveTo(move_to_pt - GG::Pt(GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight()));
    m_dragged = true;
}

void MapWnd::LButtonUp (const GG::Pt &pt, Uint32 keys)
{
    m_drag_offset = GG::Pt(-1, -1);
    m_dragged = false;
}

void MapWnd::LClick (const GG::Pt &pt, Uint32 keys)
{
    m_drag_offset = GG::Pt(-1, -1);
    if (!m_dragged)
        m_left_clicked_system_signal(UniverseObject::INVALID_OBJECT_ID);
    m_dragged = false;
}

void MapWnd::RClick(const GG::Pt& pt, Uint32 keys)
{
    // Attempt to close open fleet windows (if any are open and this is allowed), then attempt to close the SidePanel (if open);
    // if these fail, go ahead with the context-sensitive popup menu . Note that this enforces a one-close-per-click policy.

    if (GetOptionsDB().Get<bool>("UI.window-quickclose")) {
        if (FleetWnd::CloseAllFleetWnds())
            return;

        if (m_side_panel->Visible()) {
            m_side_panel->Hide();
            return;
        }
    }

    // TODO : provide a context-sensitive menu for the main map, if needed
}

void MapWnd::MouseWheel(const GG::Pt& pt, int move, Uint32 keys)
{
    if (move != 0) {
	Zoom(move);
    }
}

void MapWnd::InitTurn(int turn_number)
{ 
    GG::App::GetApp()->SetAccelerator(GG::GGK_RETURN, 0);
    GG::App::GetApp()->SetAccelerator(GG::GGK_KP_ENTER, 0);

    GG::App::GetApp()->SetAccelerator(GG::GGK_RETURN, GG::GGKMOD_CTRL);
    GG::App::GetApp()->SetAccelerator(GG::GGK_KP_ENTER, GG::GGKMOD_CTRL);

    GG::App::GetApp()->SetAccelerator(GG::GGK_F2, 0);
    GG::App::GetApp()->SetAccelerator(GG::GGK_F10, 0);
    GG::App::GetApp()->SetAccelerator(GG::GGK_s, 0);

    // Keys for zooming
    GG::App::GetApp()->SetAccelerator(GG::GGK_e, 0);
    GG::App::GetApp()->SetAccelerator(GG::GGK_r, 0);
    GG::App::GetApp()->SetAccelerator(GG::GGK_KP_PLUS, 0);
    GG::App::GetApp()->SetAccelerator(GG::GGK_KP_MINUS, 0);

    // Keys for showing systems
    GG::App::GetApp()->SetAccelerator(GG::GGK_d, 0);
    GG::App::GetApp()->SetAccelerator(GG::GGK_x, 0);
    GG::App::GetApp()->SetAccelerator(GG::GGK_c, 0);

    // Keys for showing fleets
    GG::App::GetApp()->SetAccelerator(GG::GGK_f, 0);
    GG::App::GetApp()->SetAccelerator(GG::GGK_g, 0);
    GG::App::GetApp()->SetAccelerator(GG::GGK_v, 0);
    GG::App::GetApp()->SetAccelerator(GG::GGK_b, 0);

    Universe& universe = ClientApp::GetUniverse();

    // assumes the app is wider than it is tall, and so if it fits in the height it will fit in the width
    if (GG::App::GetApp()->AppHeight() - 2.0 * MAP_MARGIN_WIDTH < Universe::UniverseWidth() * s_min_scale_factor)
        s_min_scale_factor = std::max(0.05, (GG::App::GetApp()->AppHeight() - 2.0 * MAP_MARGIN_WIDTH) / Universe::UniverseWidth());

    Resize(static_cast<int>(Universe::UniverseWidth() * s_max_scale_factor) + GG::App::GetApp()->AppWidth() + MAP_MARGIN_WIDTH,
	   static_cast<int>(Universe::UniverseWidth() * s_max_scale_factor) + GG::App::GetApp()->AppHeight() + MAP_MARGIN_WIDTH);

    // set up nebulae on the first turn
    if (m_nebulae.empty()) {
	    // chosen so that the density of nebulae will be about MIN_NEBULAE to MAX_NEBULAE for a 1000.0-width galaxy
	    const double DENSITY_SCALE_FACTOR = (Universe::UniverseWidth() * Universe::UniverseWidth()) / (1000.0 * 1000.0);
	    int num_nebulae = RandSmallInt(static_cast<int>(MIN_NEBULAE * DENSITY_SCALE_FACTOR), 
				                       static_cast<int>(MAX_NEBULAE * DENSITY_SCALE_FACTOR));
	    m_nebulae.resize(num_nebulae);
	    m_nebula_centers.resize(num_nebulae);
	    SmallIntDistType universe_placement = SmallIntDist(0, static_cast<int>(Universe::UniverseWidth()));
	    SmallIntDistType nebula_type = SmallIntDist(1, NUM_NEBULA_TEXTURES);
	    for (int i = 0; i < num_nebulae; ++i) {
	        std::string nebula_filename = "nebula" + boost::lexical_cast<std::string>(nebula_type()) + ".png";
	        m_nebulae[i].reset(new GG::Texture());
	        m_nebulae[i]->Load(ClientUI::ART_DIR + nebula_filename);
	        m_nebula_centers[i] = GG::Pt(universe_placement(), universe_placement());
	    }
    }

    // this gets cleared here instead of with the movement line stuff because that would clear some movement lines that come from the SystemIcons below
    m_fleet_lines.clear();

    // systems and starlanes
    for (unsigned int i = 0; i < m_system_icons.size(); ++i) {
        DeleteChild(m_system_icons[i]);
    }
    m_system_icons.clear();
    m_starlanes.clear();

    std::vector<System*> systems = universe.FindObjects<System>();
    for (unsigned int i = 0; i < systems.size(); ++i) {
        // system
        SystemIcon* icon = new SystemIcon(systems[i]->ID(), m_zoom_factor);
        m_system_icons.push_back(icon);
        icon->InstallEventFilter(this);
        AttachChild(icon);
        GG::Connect(icon->LeftClickedSignal(), &MapWnd::SelectSystem, this);
        GG::Connect(icon->RightClickedSignal(), &MapWnd::SystemRightClicked, this);

        // system's starlanes
        for (System::lane_iterator it = systems[i]->begin_lanes(); it != systems[i]->end_lanes(); ++it) {
            if (!it->second) {
                System* dest_system = universe.Object<System>(it->first);
                m_starlanes.insert(StarlaneData(systems[i], dest_system));
            }
        }
    }        

    // fleets not in systems
    for (unsigned int i = 0; i < m_moving_fleet_buttons.size(); ++i) {
        DeleteChild(m_moving_fleet_buttons[i]);
    }
    m_moving_fleet_buttons.clear();

    Universe::ObjectVec fleets = universe.FindObjects(MovingFleetVisitor());
    typedef std::multimap<std::pair<double, double>, UniverseObject*> SortedFleetMap;
    SortedFleetMap position_sorted_fleets;
    for (unsigned int i = 0; i < fleets.size(); ++i) {
        position_sorted_fleets.insert(std::make_pair(std::make_pair(fleets[i]->X(), fleets[i]->Y()), fleets[i]));
    }

    SortedFleetMap::iterator it = position_sorted_fleets.begin();
    SortedFleetMap::iterator end_it = position_sorted_fleets.end();
    while (it != end_it) {
        SortedFleetMap::iterator local_end_it = position_sorted_fleets.upper_bound(it->first);
        std::map<int, std::vector<int> > IDs_by_empire_color;
        for (; it != local_end_it; ++it) {
            IDs_by_empire_color[*it->second->Owners().begin()].push_back(it->second->ID());
        }
        for (std::map<int, std::vector<int> >::iterator ID_it = IDs_by_empire_color.begin(); ID_it != IDs_by_empire_color.end(); ++ID_it) {
            FleetButton* fb = new FleetButton(Empires().Lookup(ID_it->first)->Color(), ID_it->second, m_zoom_factor);
            m_moving_fleet_buttons.push_back(fb);
            AttachChild(fb);
            SetFleetMovement(fb);
        }
    }

    MoveChildUp(m_side_panel);
    if (m_side_panel->SystemID() == UniverseObject::INVALID_OBJECT_ID)
        m_side_panel->Hide();

    // set turn button to current turn
    m_turn_update->SetText( ClientUI::String("MAP_BTN_TURN_UPDATE") + boost::lexical_cast<std::string>(turn_number ) );    
    MoveChildUp( m_turn_update );

    MoveChildUp(m_sitrep_panel);
    // are there any sitreps to show?
    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );
    m_sitrep_panel->Update();
    if ( empire->NumSitRepEntries( ) )
        m_sitrep_panel->Show();
    else
        m_sitrep_panel->Hide();

    m_chat_edit->Hide();
    EnableAlphaNumAccels();


    if (m_zoom_factor * ClientUI::PTS < MIN_SYSTEM_NAME_SIZE)
        HideSystemNames();
    else
        ShowSystemNames();

    // center the map at the start of the game (if we're at the default start position, the ods are very good that this is a fresh game)
    if (ClientUpperLeft() == GG::Pt())
        CenterOnMapCoord(Universe::UniverseWidth() / 2, Universe::UniverseWidth() / 2);

    GG::Connect(empire->FoodResPool       ().ChangedSignal(),&MapWnd::FoodResourcePoolChanged      ,this,0);FoodResourcePoolChanged();
    GG::Connect(empire->MineralResPool    ().ChangedSignal(),&MapWnd::MineralResourcePoolChanged   ,this,0);MineralResourcePoolChanged();
    GG::Connect(empire->ResearchResPool   ().ChangedSignal(),&MapWnd::ResearchResourcePoolChanged  ,this,0);ResearchResourcePoolChanged();
    GG::Connect(empire->PopulationResPool ().ChangedSignal(),&MapWnd::PopulationResourcePoolChanged,this,1);PopulationResourcePoolChanged();
    GG::Connect(empire->IndustryResPool   ().ChangedSignal(),&MapWnd::IndustryResourcePoolChanged  ,this,0);IndustryResourcePoolChanged();
    
    MoveChildUp(m_toolbar);
}

void MapWnd::RestoreFromSaveData(const GG::XMLElement& elem)
{
    m_zoom_factor = boost::lexical_cast<double>(elem.Child("m_zoom_factor").Text());

    for (unsigned int i = 0; i < m_system_icons.size(); ++i) {
        const System& system = m_system_icons[i]->GetSystem();
        GG::Pt icon_ul(static_cast<int>((system.X() - ClientUI::SYSTEM_ICON_SIZE / 2) * m_zoom_factor), 
                       static_cast<int>((system.Y() - ClientUI::SYSTEM_ICON_SIZE / 2) * m_zoom_factor));
        m_system_icons[i]->SizeMove(icon_ul.x, icon_ul.y, 
                             static_cast<int>(icon_ul.x + ClientUI::SYSTEM_ICON_SIZE * m_zoom_factor + 0.5), 
                             static_cast<int>(icon_ul.y + ClientUI::SYSTEM_ICON_SIZE * m_zoom_factor + 0.5));
    }

    for (unsigned int i = 0; i < m_moving_fleet_buttons.size(); ++i) {
        Fleet* fleet = *m_moving_fleet_buttons[i]->Fleets().begin();
        double x = fleet->X();
        double y = fleet->Y();
        GG::Pt button_ul(static_cast<int>((x - ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE / 2) * m_zoom_factor), 
                         static_cast<int>((y - ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE / 2) * m_zoom_factor));
        m_moving_fleet_buttons[i]->SizeMove(button_ul.x, button_ul.y, 
                                     static_cast<int>(button_ul.x + ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE * m_zoom_factor + 0.5), 
                                     static_cast<int>(button_ul.y + ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE * m_zoom_factor + 0.5));
    }

    GG::Pt ul = UpperLeft();
    GG::Pt map_move = GG::Pt(elem.Child("upper_left").Child("GG::Pt")) - ul;
    OffsetMove(map_move);
    MoveBackgrounds(map_move);
    m_side_panel->OffsetMove(-map_move);
    m_chat_display->OffsetMove(-map_move);
    m_chat_edit->OffsetMove(-map_move);
    m_sitrep_panel->OffsetMove(-map_move);
    m_toolbar->OffsetMove(-map_move);

    // this correction ensures that zooming in doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ul;
    m_side_panel->OffsetMove(-final_move);
    m_chat_display->OffsetMove(-final_move);
    m_chat_edit->OffsetMove(-final_move);
    m_sitrep_panel->OffsetMove(-final_move);
    m_toolbar->OffsetMove(-final_move);

    MoveBackgrounds(final_move);
    MoveTo(move_to_pt - GG::Pt(GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight()));

    m_nebulae.clear();
    m_nebula_centers.clear();
    const GG::XMLElement& nebulae = elem.Child("m_nebulae");
    for (int i = 0; i < nebulae.NumChildren(); ++i) {
        const GG::XMLElement& curr_nebula = nebulae.Child(i);
        m_nebulae.push_back(GG::App::GetApp()->GetTexture(curr_nebula.Child("filename").Text()));
        m_nebula_centers.push_back(GG::Pt(curr_nebula.Child("position").Child("GG::Pt")));
    }
}

void MapWnd::ShowSystemNames()
{
    for (unsigned int i = 0; i < m_system_icons.size(); ++i)
        m_system_icons[i]->ShowName();
}

void MapWnd::HideSystemNames()
{
    for (unsigned int i = 0; i < m_system_icons.size(); ++i)
        m_system_icons[i]->HideName();
}

void MapWnd::HandlePlayerChatMessage(const std::string& msg)
{
    *m_chat_display += msg;
    m_chat_display->Show();
    g_chat_display_show_time = GG::App::GetApp()->Ticks();
}

void MapWnd::CenterOnMapCoord(double x, double y)
{
    GG::Pt ul = ClientUpperLeft();
    double current_x = (GG::App::GetApp()->AppWidth() / 2 - ul.x) / m_zoom_factor;
    double current_y = (GG::App::GetApp()->AppHeight() / 2 - ul.y) / m_zoom_factor;
    GG::Pt map_move = GG::Pt(static_cast<int>((current_x - x) * m_zoom_factor), 
                             static_cast<int>((current_y - y) * m_zoom_factor));
    OffsetMove(map_move);
    MoveBackgrounds(map_move);
    m_side_panel->OffsetMove(-map_move);
    m_chat_display->OffsetMove(-map_move);
    m_chat_edit->OffsetMove(-map_move);
    m_sitrep_panel->OffsetMove(-map_move);
    m_toolbar->OffsetMove(-map_move);

    // this correction ensures that the centering doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ul;
    m_side_panel->OffsetMove(-final_move);
    m_chat_display->OffsetMove(-final_move);
    m_chat_edit->OffsetMove(-final_move);
    m_sitrep_panel->OffsetMove(-final_move);
    m_toolbar->OffsetMove(-final_move);

    MoveBackgrounds(final_move);
    MoveTo(move_to_pt - GG::Pt(GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight()));
}

void MapWnd::CenterOnSystem(int systemID)
{
    if (System* system = GetUniverse().Object<System>(systemID))
        CenterOnSystem(system);
}

void MapWnd::CenterOnFleet(int fleetID)
{
    if (Fleet* fleet = GetUniverse().Object<Fleet>(fleetID))
        CenterOnFleet(fleet);
}

void MapWnd::CenterOnSystem(System* system)
{
    CenterOnMapCoord(system->X(), system->Y());
}

void MapWnd::CenterOnFleet(Fleet* fleet)
{
    CenterOnMapCoord(fleet->X(), fleet->Y());
}

void MapWnd::SelectSystem(int systemID)
{
    m_left_clicked_system_signal(systemID);
}

void MapWnd::SelectFleet(int fleetID)
{
    if (Fleet* fleet = GetUniverse().Object<Fleet>(fleetID))
        SelectFleet(fleet);
}

void MapWnd::SelectSystem(System* system)
{
    SelectSystem(system->ID());
}

void MapWnd::SelectFleet(Fleet* fleet)
{
    if (System* system = fleet->GetSystem()) {
        for (unsigned int i = 0; i < m_system_icons.size(); ++i) {
            if (&m_system_icons[i]->GetSystem() == system) {
                m_system_icons[i]->ClickFleetButton(fleet);
                break;
            }
        }
    } else {
        for (unsigned int i = 0; i < m_moving_fleet_buttons.size(); ++i) {
            if (std::find(m_moving_fleet_buttons[i]->Fleets().begin(), m_moving_fleet_buttons[i]->Fleets().end(), fleet) != m_moving_fleet_buttons[i]->Fleets().end()) {
                m_moving_fleet_buttons[i]->LClick(GG::Pt(), 0);
                break;
            }
        }
    }
}

void MapWnd::SetFleetMovement(FleetButton* fleet_button)
{
    std::set<int> destinations;
    for (unsigned int i = 0; i < fleet_button->Fleets().size(); ++i) {
        Fleet* fleet = fleet_button->Fleets()[i];
        GG::Pt ul = ClientUpperLeft();
        if (fleet->FinalDestinationID() != UniverseObject::INVALID_OBJECT_ID &&
            fleet->FinalDestinationID() != fleet->SystemID() &&
            destinations.find(fleet->FinalDestinationID()) == destinations.end()) {
            destinations.insert(fleet->FinalDestinationID());
            GG::Pt fleet_ul = fleet_button->UpperLeft();
            GG::Pt sz = fleet_button->Size();
            m_fleet_lines[fleet] = MovementLineData((fleet_ul.x + sz.x / 2 - ul.x) / m_zoom_factor, 
                                                    (fleet_ul.y + sz.y / 2 - ul.y) / m_zoom_factor, 
                                                    fleet->TravelRoute());
        } else {
            m_fleet_lines.erase(fleet);
        }
    }
}

void MapWnd::SetFleetMovement(Fleet* fleet)
{
    GG::Pt ul = ClientUpperLeft();
    std::map<Fleet*, MovementLineData>::iterator it = m_fleet_lines.find(fleet);
    if (it != m_fleet_lines.end()) {
        m_fleet_lines[fleet].destinations = fleet->TravelRoute();
    }
}

void MapWnd::OnTurnUpdate()
{
    // delete app popups
    CloseAllPopups( );

    HumanClientApp::GetApp()->StartTurn();
}

bool MapWnd::EventFilter(GG::Wnd* w, const GG::Wnd::Event& event)
{
    if (event.Type() == GG::Wnd::Event::RClick && !FleetWnd::FleetWndsOpen()) {
        // Attempt to close the SidePanel (if open); if this fails, just let Wnd w handle it.  
        // Note that this enforces a one-close-per-click policy.

        if (GetOptionsDB().Get<bool>("UI.window-quickclose")) {
            if (m_side_panel->Visible()) {
                m_side_panel->Hide();
                return true;
            }
        }
    }
    return false;
}

void MapWnd::Zoom(int delta)
{
    GG::Pt ul = ClientUpperLeft();
    GG::Pt center = GG::Pt( GG::App::GetApp()->AppWidth() / 2,  GG::App::GetApp()->AppHeight() / 2);
    GG::Pt ul_offset = ul - center;
    if (delta > 0) {
        if (m_zoom_factor * ZOOM_STEP_SIZE < s_max_scale_factor) {
            ul_offset.x = static_cast<int>(ul_offset.x * ZOOM_STEP_SIZE);
            ul_offset.y = static_cast<int>(ul_offset.y * ZOOM_STEP_SIZE);
            m_zoom_factor *= ZOOM_STEP_SIZE;
        } else {
            ul_offset.x = static_cast<int>(ul_offset.x * s_max_scale_factor / m_zoom_factor);
            ul_offset.y = static_cast<int>(ul_offset.y * s_max_scale_factor / m_zoom_factor);
            m_zoom_factor = s_max_scale_factor;
        }
    } else if ( delta < 0) {
        if (s_min_scale_factor < m_zoom_factor / ZOOM_STEP_SIZE) {
            ul_offset.x = static_cast<int>(ul_offset.x / ZOOM_STEP_SIZE);
            ul_offset.y = static_cast<int>(ul_offset.y / ZOOM_STEP_SIZE);
            m_zoom_factor /= ZOOM_STEP_SIZE;
        } else {
            ul_offset.x = static_cast<int>(ul_offset.x * s_min_scale_factor / m_zoom_factor);
            ul_offset.y = static_cast<int>(ul_offset.y * s_min_scale_factor / m_zoom_factor);
            m_zoom_factor = s_min_scale_factor;
        }
    } else {
        return; // If delta == 0, no change
    }

    if (m_zoom_factor * ClientUI::PTS < MIN_SYSTEM_NAME_SIZE)
        HideSystemNames();
    else
        ShowSystemNames();

    for (unsigned int i = 0; i < m_system_icons.size(); ++i) {
        const System& system = m_system_icons[i]->GetSystem();
        GG::Pt icon_ul(static_cast<int>((system.X() - ClientUI::SYSTEM_ICON_SIZE / 2) * m_zoom_factor), 
                       static_cast<int>((system.Y() - ClientUI::SYSTEM_ICON_SIZE / 2) * m_zoom_factor));
        m_system_icons[i]->SizeMove(icon_ul.x, icon_ul.y, 
                             static_cast<int>(icon_ul.x + ClientUI::SYSTEM_ICON_SIZE * m_zoom_factor + 0.5), 
                             static_cast<int>(icon_ul.y + ClientUI::SYSTEM_ICON_SIZE * m_zoom_factor + 0.5));
    }

    for (unsigned int i = 0; i < m_moving_fleet_buttons.size(); ++i) {
        Fleet* fleet = *m_moving_fleet_buttons[i]->Fleets().begin();
        double x = fleet->X();
        double y = fleet->Y();
        GG::Pt button_ul(static_cast<int>((x - ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE / 2) * m_zoom_factor), 
                         static_cast<int>((y - ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE / 2) * m_zoom_factor));
        m_moving_fleet_buttons[i]->SizeMove(button_ul.x, button_ul.y, 
                                     static_cast<int>(button_ul.x + ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE * m_zoom_factor + 0.5), 
                                     static_cast<int>(button_ul.y + ClientUI::SYSTEM_ICON_SIZE * ClientUI::FLEET_BUTTON_SIZE * m_zoom_factor + 0.5));
    }

    GG::Pt map_move = ul_offset + center - ul;
    OffsetMove(map_move);
    MoveBackgrounds(map_move);
    m_side_panel->OffsetMove(-map_move);
    m_chat_display->OffsetMove(-map_move);
    m_chat_edit->OffsetMove(-map_move);
    m_sitrep_panel->OffsetMove(-map_move);
    m_toolbar->OffsetMove(-map_move);

    // this correction ensures that zooming in doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ul;
    m_side_panel->OffsetMove(-final_move);
    m_chat_display->OffsetMove(-final_move);
    m_chat_edit->OffsetMove(-final_move);
    m_sitrep_panel->OffsetMove(-final_move);
    m_toolbar->OffsetMove(-final_move);
    MoveBackgrounds(final_move);
    MoveTo(move_to_pt - GG::Pt(GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight()));
}

void MapWnd::RenderBackgrounds()
{
    double x, y;
    glColor3d(1.0, 1.0, 1.0);
    for (int i = 0; i < NUM_BACKGROUNDS; ++i) {
        int bg_width = m_backgrounds[i]->Width();
        int bg_height = m_backgrounds[i]->Height();
        int app_width = GG::App::GetApp()->AppWidth();
        int app_height = GG::App::GetApp()->AppHeight();
        x = std::fmod(m_bg_position_X[i], bg_width);
        while (x < app_width + bg_width) {
            y = std::fmod(m_bg_position_Y[i], bg_height);
            while (y < app_height + bg_height) {
                m_backgrounds[i]->OrthoBlit(static_cast<int>(x - bg_width), static_cast<int>(y - bg_height), false);
                y += m_backgrounds[i]->Height();
            }
            x += m_backgrounds[i]->Width();
        }
    }

    for (unsigned int i = 0; i < m_nebulae.size(); ++i) {
        GG::Pt ul = 
            ClientUpperLeft() + 
            GG::Pt(static_cast<int>((m_nebula_centers[i].x - m_nebulae[i]->Width() / 2.0) * m_zoom_factor),
                   static_cast<int>((m_nebula_centers[i].y - m_nebulae[i]->Height() / 2.0) * m_zoom_factor));
        m_nebulae[i]->OrthoBlit(ul, 
                                ul + GG::Pt(static_cast<int>(m_nebulae[i]->Width() * m_zoom_factor), 
                                            static_cast<int>(m_nebulae[i]->Width() * m_zoom_factor)), 
                                0,
                                false);
    }
}

void MapWnd::RenderStarlanes()
{
    double LINE_SCALE = std::max(1.0, 0.666 * m_zoom_factor);
    double INNER_LINE_PORTION = 0.3;
    double INNER_LINE_WIDTH = (LINE_SCALE / 2.0) * INNER_LINE_PORTION; // these are actually half-widths in either direction
    double OUTER_LINE_WIDTH = (LINE_SCALE / 2.0);
    double CENTER_ALPHA = 0.7;
    double INNER_LINE_EDGE_ALPHA = 0.4;
    double OUTER_LINE_EDGE_ALPHA = 0.0;

    glDisable(GL_TEXTURE_2D);

    GG::Pt ul = ClientUpperLeft();
    for (std::set<StarlaneData>::iterator it = m_starlanes.begin(); it != m_starlanes.end(); ++it) {
        double center1[2] = {ul.x + it->Src()->X() * m_zoom_factor, ul.y + it->Src()->Y() * m_zoom_factor};
        double center2[2] = {ul.x + it->Dst()->X() * m_zoom_factor, ul.y + it->Dst()->Y() * m_zoom_factor};
        double lane_length = GetUniverse().LinearDistance(it->Src()->ID(), it->Dst()->ID());
        double left_vec[2] = {-(center2[1] - center1[1]) / lane_length, (center2[0] - center1[0]) / lane_length};
        double far_left1[2] = {center1[0] + OUTER_LINE_WIDTH * left_vec[0], center1[1] + OUTER_LINE_WIDTH * left_vec[1]};
        double far_left2[2] = {center2[0] + OUTER_LINE_WIDTH * left_vec[0], center2[1] + OUTER_LINE_WIDTH * left_vec[1]};
        double left1[2] = {center1[0] + INNER_LINE_WIDTH * left_vec[0], center1[1] + INNER_LINE_WIDTH * left_vec[1]};
        double left2[2] = {center2[0] + INNER_LINE_WIDTH * left_vec[0], center2[1] + INNER_LINE_WIDTH * left_vec[1]};
        double right1[2] = {center1[0] - INNER_LINE_WIDTH * left_vec[0], center1[1] - INNER_LINE_WIDTH * left_vec[1]};
        double right2[2] = {center2[0] - INNER_LINE_WIDTH * left_vec[0], center2[1] - INNER_LINE_WIDTH * left_vec[1]};
        double far_right1[2] = {center1[0] - OUTER_LINE_WIDTH * left_vec[0], center1[1] - OUTER_LINE_WIDTH * left_vec[1]};
        double far_right2[2] = {center2[0] - OUTER_LINE_WIDTH * left_vec[0], center2[1] - OUTER_LINE_WIDTH * left_vec[1]};

        GG::Clr color = GG::CLR_WHITE;
        if (it->Src()->Owners().size() == 1 && it->Dst()->Owners().size() == 1 &&
            *it->Src()->Owners().begin() == *it->Dst()->Owners().begin()) {
            color = Empires().Lookup(*it->Src()->Owners().begin())->Color();
        }

        glBegin(GL_TRIANGLE_STRIP);
        color.a = static_cast<unsigned char>(255 * OUTER_LINE_EDGE_ALPHA);
        glColor4ubv(color.v);
        glVertex2dv(far_left2);
        glVertex2dv(far_left1);
        color.a = static_cast<unsigned char>(255 * INNER_LINE_EDGE_ALPHA);
        glColor4ubv(color.v);
        glVertex2dv(left2);
        glVertex2dv(left1);
        color.a = static_cast<unsigned char>(255 * CENTER_ALPHA);
        glColor4ubv(color.v);
        glVertex2dv(center2);
        glVertex2dv(center1);
        color.a = static_cast<unsigned char>(255 * INNER_LINE_EDGE_ALPHA);
        glColor4ubv(color.v);
        glVertex2dv(right2);
        glVertex2dv(right1);
        color.a = static_cast<unsigned char>(255 * OUTER_LINE_EDGE_ALPHA);
        glColor4ubv(color.v);
        glVertex2dv(far_right2);
        glVertex2dv(far_right1);
        glEnd();
    }

    glEnable(GL_TEXTURE_2D);
}

void MapWnd::RenderFleetMovementLines()
{
    const GLushort PATTERN = 0xF0F0;
    const int GLUSHORT_BIT_LENGTH = sizeof(GLushort) * 8;
    const double RATE = 0.25;
    const int SHIFT = static_cast<int>(GG::App::GetApp()->Ticks() * RATE / GLUSHORT_BIT_LENGTH) % GLUSHORT_BIT_LENGTH;
    const unsigned int STIPPLE = (PATTERN << SHIFT) | (PATTERN >> (GLUSHORT_BIT_LENGTH - SHIFT));
    double LINE_SCALE = std::max(1.0, 1.333 * m_zoom_factor);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(static_cast<int>(LINE_SCALE), STIPPLE);
    glLineWidth(LINE_SCALE);
    glColor4d(1.0, 1.0, 1.0, 1.0);

    GG::Pt ul = ClientUpperLeft();
    for (std::map<Fleet*, MovementLineData>::iterator it = m_fleet_lines.begin(); it != m_fleet_lines.end(); ++it) {
        // this is obviously less efficient than using GL_LINE_STRIP, but GL_LINE_STRIP sometimes produces nasty artifacts 
        // when the begining of a line segment starts offscreen
        glBegin(GL_LINES);
        const std::list<System*>& destinations = it->second.destinations;
        glVertex2d(ul.x + it->second.x * m_zoom_factor, ul.y + it->second.y * m_zoom_factor);
        for (std::list<System*>::const_iterator dest_it = destinations.begin(); dest_it != destinations.end(); ++dest_it) {
            if (it->first->SystemID() == (*dest_it)->ID())
                continue;
            glVertex2d(ul.x + (*dest_it)->X() * m_zoom_factor, ul.y + (*dest_it)->Y() * m_zoom_factor);
            std::list<System*>::const_iterator temp_it = dest_it;
            if (++temp_it != destinations.end())
                glVertex2d(ul.x + (*dest_it)->X() * m_zoom_factor, ul.y + (*dest_it)->Y() * m_zoom_factor);
        }
        glEnd();
    }

    glLineWidth(1.0);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_LINE_STIPPLE);
    glEnable(GL_TEXTURE_2D);
}

void MapWnd::MoveBackgrounds(const GG::Pt& move)
{   
    double move_x, move_y;

    for (int i = 0; i < NUM_BACKGROUNDS; ++i) {
        move_x = move.x * m_bg_scroll_rate[i];
        move_y = move.y * m_bg_scroll_rate[i];
        m_bg_position_X[i] += move_x;
        m_bg_position_Y[i] += move_y;    
    }
}

void MapWnd::CorrectMapPosition(GG::Pt &move_to_pt)
{
    int contents_width = static_cast<int>(m_zoom_factor * Universe::UniverseWidth());
    int app_width =  GG::App::GetApp()->AppWidth();
    int app_height = GG::App::GetApp()->AppHeight();
    if (app_width < contents_width * 1.25) {
        if (MAP_MARGIN_WIDTH < move_to_pt.x)
            move_to_pt.x = MAP_MARGIN_WIDTH;
        if (move_to_pt.x < app_width - contents_width - 2 * MAP_MARGIN_WIDTH)
            move_to_pt.x = app_width - contents_width - 2 * MAP_MARGIN_WIDTH;
    } else {
        if (move_to_pt.x < 0)
            move_to_pt.x = 0;
        if (app_width - contents_width < move_to_pt.x)
            move_to_pt.x = app_width - contents_width;
    }

    if (app_height < contents_width * 1.25) {
        if (MAP_MARGIN_WIDTH < move_to_pt.y)
            move_to_pt.y = MAP_MARGIN_WIDTH;
        if (move_to_pt.y < app_height - contents_width - 2 * MAP_MARGIN_WIDTH)
            move_to_pt.y = app_height - contents_width - 2 * MAP_MARGIN_WIDTH;
    } else {
        if (move_to_pt.y < 0)
            move_to_pt.y = 0;
        if (app_height - contents_width < move_to_pt.y)
            move_to_pt.y = app_height - contents_width;
    }
}

void MapWnd::SystemRightClicked(int system_id)
{
    m_right_clicked_system_signal(system_id);
}

void MapWnd::UniverseObjectDeleted(const UniverseObject *obj)
{
    m_fleet_lines.erase(const_cast<Fleet*>(universe_object_cast<const Fleet*>(obj)));
}

void MapWnd::RegisterPopup( MapWndPopup* popup )
{
    if (popup) {
        m_popups.push_back(popup);
    }
}

void MapWnd::RemovePopup( MapWndPopup* popup )
{
    if (popup) {
        std::list<MapWndPopup*>::iterator it = std::find( m_popups.begin(), m_popups.end(), popup );
        if (it != m_popups.end())
            m_popups.erase(it);
    }
}

void MapWnd::CloseAllPopups( )
{
    for (std::list<MapWndPopup*>::iterator it = m_popups.begin(); it != m_popups.end(); ) {
        // get popup and increment iterator first since closing the popup will change this list by removing the popup
        MapWndPopup* popup = *it++;
        popup->Close( );
    }   
    // clear list
    m_popups.clear( );
}

bool MapWnd::OpenChatWindow()
{
    if (!m_chat_display->Visible() || !m_chat_edit->Visible()) {
        m_chat_display->Show();
        DisableAlphaNumAccels();
        m_chat_edit->Show();
        GG::App::GetApp()->SetFocusWnd(m_chat_edit);
        g_chat_display_show_time = GG::App::GetApp()->Ticks();
        return true;
    }
    return false;
}

bool MapWnd::EndTurn()
{
    HumanClientApp::GetApp()->StartTurn();
    return true;
}

bool MapWnd::ToggleSitRep()
{
    if (m_sitrep_panel->Visible())
        m_sitrep_panel->Hide();
    else
        m_sitrep_panel->Show();
    return true;
}

bool MapWnd::ShowOptions()
{
    if (!m_options_showing) {
        m_options_showing = true;
        InGameOptions options;
        options.Run();
        m_options_showing = false;
    }
    return true;
}

bool MapWnd::CloseSystemView()
{
    m_left_clicked_system_signal(UniverseObject::INVALID_OBJECT_ID);
	return true;
}

bool MapWnd::KeyboardZoomIn()
{
    Zoom(1);
    return true;
}
bool MapWnd::KeyboardZoomOut()
{
    Zoom(-1);
    return true;
}

void MapWnd::FoodResourcePoolChanged()
{
  Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );

  m_food->SetValue      (empire->FoodResPool   ().Available());
  m_food->SetValueSecond(empire->FoodResPool().Available()-empire->FoodResPool().Needed());
}

void MapWnd::MineralResourcePoolChanged()
{
  Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );

  m_mineral->SetValue      (empire->MineralResPool   ().Available());
  m_mineral->SetValueSecond(empire->MineralResPool   ().Available()-empire->MineralResPool().Needed());
}

void MapWnd::ResearchResourcePoolChanged()
{
  Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );

  m_research->SetValue(empire->ResearchResPool().Available());
}

void MapWnd::IndustryResourcePoolChanged()
{
  Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );

  m_industry->SetValue(empire->IndustryResPool().Available());
}


void MapWnd::PopulationResourcePoolChanged()
{
  Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );

  m_population->SetValue      (empire->PopulationResPool().Available());
  m_population->SetValueSecond(empire->PopulationResPool().Growth   ());
}

bool MapWnd::ZoomToHomeSystem()
{
    int id = Empires().Lookup(HumanClientApp::GetApp()->EmpireID())->HomeworldID();

    if (id != UniverseObject::INVALID_OBJECT_ID) {
        CenterOnSystem(GetUniverse().Object(id)->SystemID());
        SelectSystem(GetUniverse().Object(id)->SystemID());
    }

    return true;
}

bool MapWnd::ZoomToPrevOwnedSystem()
{
    // TODO: go through these in some sorted order (the sort method used in the SidePanel system name drop-list)
    Universe::ObjectIDVec vec = GetUniverse().FindObjectIDs(OwnedVisitor<System>(HumanClientApp::GetApp()->EmpireID()));
    Universe::ObjectIDVec::iterator it = std::find(vec.begin(), vec.end(), m_current_owned_system);
    if (it == vec.end()) {
        m_current_owned_system = vec.empty() ? UniverseObject::INVALID_OBJECT_ID : vec.back();
    } else {
        m_current_owned_system = it == vec.begin() ? vec.back() : *--it;
    }

    if (m_current_owned_system != UniverseObject::INVALID_OBJECT_ID) {
        CenterOnSystem(m_current_owned_system);
        SelectSystem(m_current_owned_system);
    }

    return true;
}

bool MapWnd::ZoomToNextOwnedSystem()
{
    // TODO: go through these in some sorted order (the sort method used in the SidePanel system name drop-list)
    Universe::ObjectIDVec vec = GetUniverse().FindObjectIDs(OwnedVisitor<System>(HumanClientApp::GetApp()->EmpireID()));
    Universe::ObjectIDVec::iterator it = std::find(vec.begin(), vec.end(), m_current_owned_system);
    if (it == vec.end()) {
        m_current_owned_system = vec.empty() ? UniverseObject::INVALID_OBJECT_ID : vec.front();
    } else {
        Universe::ObjectIDVec::iterator next_it = it;
        ++next_it;
        m_current_owned_system = next_it == vec.end() ? vec.front() : *next_it;
    }

    if (m_current_owned_system != UniverseObject::INVALID_OBJECT_ID) {
        CenterOnSystem(m_current_owned_system);
        SelectSystem(m_current_owned_system);
    }

    return true;
}

bool MapWnd::ZoomToPrevIdleFleet()
{
    Universe::ObjectIDVec vec = GetUniverse().FindObjectIDs(StationaryFleetVisitor(HumanClientApp::GetApp()->EmpireID()));
    Universe::ObjectIDVec::iterator it = std::find(vec.begin(), vec.end(), m_current_fleet);
    if (it == vec.end()) {
        m_current_fleet = vec.empty() ? UniverseObject::INVALID_OBJECT_ID : vec.back();
    } else {
        m_current_fleet = it == vec.begin() ? vec.back() : *--it;
    }

    if (m_current_fleet != UniverseObject::INVALID_OBJECT_ID) {
        CenterOnFleet(m_current_fleet);
        SelectFleet(m_current_fleet);
    }

    return true;
}

bool MapWnd::ZoomToNextIdleFleet()
{
    Universe::ObjectIDVec vec = GetUniverse().FindObjectIDs(StationaryFleetVisitor(HumanClientApp::GetApp()->EmpireID()));
    Universe::ObjectIDVec::iterator it = std::find(vec.begin(), vec.end(), m_current_fleet);
    if (it == vec.end()) {
        m_current_fleet = vec.empty() ? UniverseObject::INVALID_OBJECT_ID : vec.front();
    } else {
        Universe::ObjectIDVec::iterator next_it = it;
        ++next_it;
        m_current_fleet = next_it == vec.end() ? vec.front() : *next_it;
    }

    if (m_current_fleet != UniverseObject::INVALID_OBJECT_ID) {
        CenterOnFleet(m_current_fleet);
        SelectFleet(m_current_fleet);
    }

    return true;
}

bool MapWnd::ZoomToPrevFleet()
{
    Universe::ObjectIDVec vec = GetUniverse().FindObjectIDs(OwnedVisitor<Fleet>(HumanClientApp::GetApp()->EmpireID()));
    Universe::ObjectIDVec::iterator it = std::find(vec.begin(), vec.end(), m_current_fleet);
    if (it == vec.end()) {
        m_current_fleet = vec.empty() ? UniverseObject::INVALID_OBJECT_ID : vec.back();
    } else {
        m_current_fleet = it == vec.begin() ? vec.back() : *--it;
    }

    if (m_current_fleet != UniverseObject::INVALID_OBJECT_ID) {
        CenterOnFleet(m_current_fleet);
        SelectFleet(m_current_fleet);
    }

    return true;
}

bool MapWnd::ZoomToNextFleet()
{
    Universe::ObjectIDVec vec = GetUniverse().FindObjectIDs(OwnedVisitor<Fleet>(HumanClientApp::GetApp()->EmpireID()));
    Universe::ObjectIDVec::iterator it = std::find(vec.begin(), vec.end(), m_current_fleet);
    if (it == vec.end()) {
        m_current_fleet = vec.empty() ? UniverseObject::INVALID_OBJECT_ID : vec.front();
    } else {
        Universe::ObjectIDVec::iterator next_it = it;
        ++next_it;
        m_current_fleet = next_it == vec.end() ? vec.front() : *next_it;
    }

    if (m_current_fleet != UniverseObject::INVALID_OBJECT_ID) {
        CenterOnFleet(m_current_fleet);
        SelectFleet(m_current_fleet);
    }

    return true;
}

/* Disables keyboard accelerators that use an alphanumeric key
   without modifiers. This is useful if a keyboard input is required,
   so that the keys aren't interpreted as an accelerator.
   @note Repeated calls of DisableAlphaNumAccels have to be followed by the
   same number of calls to EnableAlphaNumAccels to re-enable the
   accelerators.
*/
void MapWnd::DisableAlphaNumAccels()
{
    for (GG::App::const_accel_iterator i = GG::App::GetApp()->accel_begin();
         i != GG::App::GetApp()->accel_end(); ++i) {
        if (i->second != 0) // we only want to disable keys without modifiers
            continue; 
        GG::Key key = i->first;
        if ((key >= GG::GGK_a && key <= GG::GGK_z) || 
            (key >= GG::GGK_0 && key <= GG::GGK_9)) {
            m_disabled_accels_list.insert(key);
        }
    }
    for (std::set<GG::Key>::iterator i = m_disabled_accels_list.begin();
         i != m_disabled_accels_list.end(); ++i) {
        GG::App::GetApp()->RemoveAccelerator(*i, 0);
    }
}

// Re-enable accelerators disabled by DisableAlphaNumAccels
void MapWnd::EnableAlphaNumAccels()
{
    for (std::set<GG::Key>::iterator i = m_disabled_accels_list.begin();
         i != m_disabled_accels_list.end(); ++i) {
        GG::App::GetApp()->SetAccelerator(*i, 0);
    }
    m_disabled_accels_list.clear();
}
