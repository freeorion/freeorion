#ifdef FREEORION_WIN32
#include <GL/glew.h>
#endif

#include "MapWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../universe/Fleet.h"
#include "FleetButton.h"
#include "FleetWnd.h"
#include "../client/human/HumanClientApp.h"
#include "InGameMenu.h"
#include "../network/Message.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../util/Random.h"
#include "DesignWnd.h"
#include "ProductionWnd.h"
#include "ResearchWnd.h"
#include "SidePanel.h"
#include "SitRepPanel.h"
#include "../universe/System.h"
#include "SystemIcon.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "TurnProgressWnd.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>
#include <GG/MultiEdit.h>
#include <GG/WndEvent.h>

#include <vector>
#include <deque>

#define TEST_BROWSE_INFO 0
#if TEST_BROWSE_INFO
#include <GG/BrowseInfoWnd.h>
class BrowseFoo : public GG::TextBoxBrowseInfoWnd
{
public:
    BrowseFoo() :
        TextBoxBrowseInfoWnd(200, ClientUI::Font(), 12, GG::CLR_SHADOW, GG::CLR_WHITE, GG::CLR_WHITE)
        {}

    void Update(int mode, const GG::Wnd* target)
        { SetText("mode=" + boost::lexical_cast<std::string>(mode) + " wnd=" + target->WindowText()); }
};
#endif

namespace {
    const double ZOOM_STEP_SIZE = 1.25;
    const int MIN_NEBULAE = 3; // this min and max are for a 1000.0-width galaxy
    const int MAX_NEBULAE = 6;
    const int END_TURN_BTN_WIDTH = 60;
    const int SITREP_PANEL_WIDTH = 400;
    const int SITREP_PANEL_HEIGHT = 300;
    const int MIN_SYSTEM_NAME_SIZE = 10;
    const int LAYOUT_MARGIN = 5;
    const int CHAT_WIDTH = 400;
    const int CHAT_HEIGHT = 400;
    const int CHAT_EDIT_HEIGHT = 30;

    int g_chat_display_show_time = 0;
    std::deque<std::string> g_chat_edit_history;
    int g_history_position = 0; // the current edit contents are in history position 0

    struct BoolToVoidAdapter
    {
        BoolToVoidAdapter(const boost::function<bool ()>& fn) : m_fn(fn) {}
        void operator()() { m_fn(); }
        boost::function<bool ()> m_fn;
    };

    void AddOptions(OptionsDB& db)
    {
        db.Add("UI.chat-hide-interval", "OPTIONS_DB_UI_CHAT_HIDE_INTERVAL", 10, RangedValidator<int>(0, 3600));
        db.Add("UI.chat-edit-history", "OPTIONS_DB_UI_CHAT_EDIT_HISTORY", 50, RangedValidator<int>(0, 1000));
        db.Add("UI.galaxy-gas-background", "OPTIONS_DB_GALAXY_MAP_GAS", true, Validator<bool>());
        db.Add("UI.optimized-system-rendering", "OPTIONS_DB_OPTIMIZED_SYSTEM_RENDERING", true, Validator<bool>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

#ifndef FREEORION_RELEASE
    bool RequestRegressionTestDump()
    {
        ClientNetworking& networking = HumanClientApp::GetApp()->Networking();
        Message msg(Message::DEBUG, HumanClientApp::GetApp()->PlayerID(), -1, "EffectsRegressionTest");
        networking.SendMessage(msg);
        return true;
    }
#endif

    // returns an int-int pair that doesn't depend on the order of parameters
    std::pair<int, int> UnorderedIntPair(int one, int two) {
        return std::make_pair(std::min(one, two), std::max(one, two));
    }

    // disambiguate overloaded function with a function pointer
    void (MapWnd::*SetFleetMovementLineFunc)(const Fleet*) = &MapWnd::SetFleetMovementLine;

    const float STARLANE_GRAY = 127.0f / 255.0f;
    const float STARLANE_ALPHA = 0.7f;
    const double STARLANE_WIDTH = 2.5;

    const GG::Clr UNOWNED_STARLANE_GRAY_CLR = GG::Clr(128, 128, 128, 255);
}

////////////////////////////////////////////////////////////
// MapWnd::GLBuffer
////////////////////////////////////////////////////////////
MapWnd::GLBuffer::GLBuffer() :
    m_name(0),
    m_size(0)
{}

////////////////////////////////////////////////////////////
// MapWndPopup
////////////////////////////////////////////////////////////
MapWndPopup::MapWndPopup(const std::string& t, int x, int y, int h, int w, GG::Flags<GG::WndFlag> flags) :
    CUIWnd( t, x, y, h, w, flags)
{ ClientUI::GetClientUI()->GetMapWnd()->RegisterPopup(this); }

MapWndPopup::~MapWndPopup()
{ ClientUI::GetClientUI()->GetMapWnd()->RemovePopup(this);}

void MapWndPopup::CloseClicked()
{
    CUIWnd::CloseClicked();
    delete this;
}

void MapWndPopup::Close()
{ CloseClicked(); }

////////////////////////////////////////////////
// MapWnd
////////////////////////////////////////////////
// MapWnd::MovementLineData
MapWnd::MovementLineData::MovementLineData() : 
    m_colour(GG::CLR_ZERO), 
    m_path(),
    m_button(0),
    m_x(-100000.0),   // UniverseObject::INVALID_POSITION value respecified here to avoid unnecessary include dependency
    m_y(-100000.0)
{}

MapWnd::MovementLineData::MovementLineData(double x, double y, const std::list<MovePathNode>& path, GG::Clr colour/* = GG::CLR_WHITE*/) :
    m_colour(colour),
    m_path(path),
    m_button(0),
    m_x(x),
    m_y(y)
{}

MapWnd::MovementLineData::MovementLineData(const FleetButton* button, const std::list<MovePathNode>& path, GG::Clr colour/* = GG::CLR_WHITE*/) :
    m_colour(colour),
    m_path(path),
    m_button(button),
    m_x(-100000.0),
    m_y(-100000.0)
{ assert(button); }

GG::Clr MapWnd::MovementLineData::Colour() const
{ return m_colour; }

const std::list<MovePathNode>& MapWnd::MovementLineData::Path() const
{ return m_path; }

std::pair<double, double> MapWnd::MovementLineData::Start() const
{
    if (m_button) {
        GG::Pt sz = m_button->Size();
        GG::Pt fleet_screen_centre = m_button->UpperLeft() + GG::Pt(sz.x / 2, sz.y / 2);
        return ClientUI::GetClientUI()->GetMapWnd()->UniversePositionFromScreenCoords(fleet_screen_centre);
    } else {
        return std::make_pair(m_x, m_y);
    }
}

// MapWnd::FleetETAMapIndicator
class MapWnd::FleetETAMapIndicator : public GG::Wnd
{
public:
    FleetETAMapIndicator(double x, double y, int eta);
    virtual void    Render();
private:
    double              m_x, m_y;
    GG::TextControl*    m_text;
};

MapWnd::FleetETAMapIndicator::FleetETAMapIndicator(double x, double y, int eta) :
    GG::Wnd(0, 0, 1, 1, GG::Flags<GG::WndFlag>()),
    m_x(x),
    m_y(y),
    m_text(0)
{
    std::string eta_text;
    if (eta == Fleet::ETA_UNKNOWN)
        eta_text = UserString("FW_FLEET_ETA_UNKNOWN");
    else if (eta == Fleet::ETA_NEVER)
        eta_text = UserString("FW_FLEET_ETA_NEVER");
    else if (eta == Fleet::ETA_OUT_OF_RANGE)
        eta_text = UserString("FW_FLEET_ETA_OUT_OF_RANGE");
    else
        eta_text = boost::lexical_cast<std::string>(eta);

    m_text = new GG::TextControl(0, 0, eta_text, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()),
                                 ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
    Resize(m_text->Size());
    AttachChild(m_text);
}

void MapWnd::FleetETAMapIndicator::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_BLACK, ClientUI::WndInnerBorderColor(), 1);
}

// MapWnd
// static(s)
double    MapWnd::s_min_scale_factor = 0.35;
double    MapWnd::s_max_scale_factor = 8.0;
const int MapWnd::SIDE_PANEL_WIDTH = 360;

MapWnd::MapWnd() :
    GG::Wnd(-GG::GUI::GetGUI()->AppWidth(), -GG::GUI::GetGUI()->AppHeight(),
            static_cast<int>(Universe::UniverseWidth() * s_max_scale_factor + GG::GUI::GetGUI()->AppWidth() * 1.5),
            static_cast<int>(Universe::UniverseWidth() * s_max_scale_factor + GG::GUI::GetGUI()->AppHeight() * 1.5),
            GG::CLICKABLE | GG::DRAGABLE),
    m_disabled_accels_list(),
    m_backgrounds(),
    m_bg_scroll_rate(),
    m_previously_selected_system(UniverseObject::INVALID_OBJECT_ID),
    m_zoom_factor(1.0),
    m_star_texture_coords(),
    m_starlane_vertices(),
    m_starlane_colors(),
    m_starlane_fleet_supply_vertices(),
    m_starlane_fleet_supply_colors(),
    m_drag_offset(-1, -1),
    m_dragged(false),
    m_current_owned_system(UniverseObject::INVALID_OBJECT_ID),
    m_current_fleet(UniverseObject::INVALID_OBJECT_ID),
    m_in_production_view_mode(false)
{
    SetText("MapWnd");

    Connect(GetUniverse().UniverseObjectDeleteSignal, &MapWnd::UniverseObjectDeleted, this);

    // toolbar
    m_toolbar = new CUIToolBar(0,0,GG::GUI::GetGUI()->AppWidth(),30);
    GG::GUI::GetGUI()->Register(m_toolbar);
    m_toolbar->Hide();

    // system-view side panel
    m_side_panel = new SidePanel(GG::GUI::GetGUI()->AppWidth() - SIDE_PANEL_WIDTH, m_toolbar->LowerRight().y, SIDE_PANEL_WIDTH, GG::GUI::GetGUI()->AppHeight());
    GG::Connect(m_side_panel->SystemSelectedSignal, &MapWnd::SelectSystem, this);                                               // sidepanel requests system selection change -> select it
    GG::Connect(m_side_panel->ResourceCenterChangedSignal, &MapWnd::UpdateSidePanelSystemObjectMetersAndResourcePools, this);   // something in sidepanel changed resource pool(s), so need to recalculate and update meteres and resource pools and refresh their indicators
    //GG::Connect(m_side_panel->ResourceCenterChangedSignal, &MapWnd::UpdateMetersAndResourcePools, this);                        // something in sidepanel changed resource pool(s), so need to recalculate and update meteres and resource pools and refresh their indicators

    m_sitrep_panel = new SitRepPanel( (GG::GUI::GetGUI()->AppWidth()-SITREP_PANEL_WIDTH)/2, (GG::GUI::GetGUI()->AppHeight()-SITREP_PANEL_HEIGHT)/2, SITREP_PANEL_WIDTH, SITREP_PANEL_HEIGHT );
    GG::Connect(m_sitrep_panel->ClosingSignal, BoolToVoidAdapter(boost::bind(&MapWnd::ToggleSitRep, this)));    // sitrep panel is manually closed by user

    m_research_wnd = new ResearchWnd(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight() - m_toolbar->Height());
    m_research_wnd->MoveTo(GG::Pt(0, m_toolbar->Height()));
    GG::GUI::GetGUI()->Register(m_research_wnd);
    m_research_wnd->Hide();

    m_production_wnd = new ProductionWnd(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight() - m_toolbar->Height());
    m_production_wnd->MoveTo(GG::Pt(0, m_toolbar->Height()));
    GG::GUI::GetGUI()->Register(m_production_wnd);
    m_production_wnd->Hide();
    GG::Connect(m_production_wnd->SystemSelectedSignal, &MapWnd::SelectSystem, this); // productionwnd requests system selection change -> select it

    m_design_wnd = new DesignWnd(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight() - m_toolbar->Height());
    m_design_wnd->MoveTo(GG::Pt(0, m_toolbar->Height()));
    GG::GUI::GetGUI()->Register(m_design_wnd);
    m_design_wnd->Hide();

    // turn button
    m_turn_update = new CUITurnButton(LAYOUT_MARGIN, LAYOUT_MARGIN, END_TURN_BTN_WIDTH, "" );
    m_toolbar->AttachChild(m_turn_update);
    GG::Connect(m_turn_update->ClickedSignal, BoolToVoidAdapter(boost::bind(&MapWnd::EndTurn, this)));

    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());
    const int BUTTON_TOTAL_MARGIN = 8;

    // FPS indicator
    m_FPS = new FPSIndicator(m_turn_update->LowerRight().x + LAYOUT_MARGIN, m_turn_update->UpperLeft().y);
    m_toolbar->AttachChild(m_FPS);

    // Subscreen / Menu buttons
    int button_width = font->TextExtent(UserString("MAP_BTN_MENU")).x + BUTTON_TOTAL_MARGIN;
    m_btn_menu = new CUIButton(m_toolbar->LowerRight().x-button_width, 0, button_width, UserString("MAP_BTN_MENU") );
    m_toolbar->AttachChild(m_btn_menu);
    GG::Connect(m_btn_menu->ClickedSignal, BoolToVoidAdapter(boost::bind(&MapWnd::ShowMenu, this)));

    button_width = font->TextExtent(UserString("MAP_BTN_DESIGN")).x + BUTTON_TOTAL_MARGIN;
    m_btn_design = new CUIButton(m_btn_menu->UpperLeft().x-LAYOUT_MARGIN-button_width, 0, button_width, UserString("MAP_BTN_DESIGN") );
    m_toolbar->AttachChild(m_btn_design);
    GG::Connect(m_btn_design->ClickedSignal, BoolToVoidAdapter(boost::bind(&MapWnd::ToggleDesign, this)));

    button_width = font->TextExtent(UserString("MAP_BTN_PRODUCTION")).x + BUTTON_TOTAL_MARGIN;
    m_btn_production = new CUIButton(m_btn_design->UpperLeft().x-LAYOUT_MARGIN-button_width, 0, button_width, UserString("MAP_BTN_PRODUCTION") );
    m_toolbar->AttachChild(m_btn_production);
    GG::Connect(m_btn_production->ClickedSignal, BoolToVoidAdapter(boost::bind(&MapWnd::ToggleProduction, this)));

    button_width = font->TextExtent(UserString("MAP_BTN_RESEARCH")).x + BUTTON_TOTAL_MARGIN;
    m_btn_research = new CUIButton(m_btn_production->UpperLeft().x-LAYOUT_MARGIN-button_width, 0, button_width, UserString("MAP_BTN_RESEARCH") );
    m_toolbar->AttachChild(m_btn_research);
    GG::Connect(m_btn_research->ClickedSignal, BoolToVoidAdapter(boost::bind(&MapWnd::ToggleResearch, this)));

    button_width = font->TextExtent(UserString("MAP_BTN_SITREP")).x + BUTTON_TOTAL_MARGIN;
    m_btn_siterep = new CUIButton(m_btn_research->UpperLeft().x-LAYOUT_MARGIN-button_width, 0, button_width, UserString("MAP_BTN_SITREP") );
    m_toolbar->AttachChild(m_btn_siterep);
    GG::Connect(m_btn_siterep->ClickedSignal, BoolToVoidAdapter(boost::bind(&MapWnd::ToggleSitRep, this)));

    // resources
    const int ICON_DUAL_WIDTH = 100;
    const int ICON_WIDTH = ICON_DUAL_WIDTH - 30;
    m_population = new StatisticIcon(m_btn_siterep->UpperLeft().x-LAYOUT_MARGIN-ICON_DUAL_WIDTH,LAYOUT_MARGIN,ICON_DUAL_WIDTH,m_turn_update->Height(),
                                     ClientUI::MeterIcon(METER_POPULATION),
                                     0,0,3,3,true,true,false,true);
    m_toolbar->AttachChild(m_population);

    m_industry = new StatisticIcon(m_population->UpperLeft().x-LAYOUT_MARGIN-ICON_WIDTH,LAYOUT_MARGIN,ICON_WIDTH,m_turn_update->Height(),
                                   ClientUI::MeterIcon(METER_INDUSTRY),
                                   0,3,true,false);
    m_toolbar->AttachChild(m_industry);

    m_research = new StatisticIcon(m_industry->UpperLeft().x-LAYOUT_MARGIN-ICON_WIDTH,LAYOUT_MARGIN,ICON_WIDTH,m_turn_update->Height(),
                                   ClientUI::MeterIcon(METER_RESEARCH),
                                   0,3,true,false);
    m_toolbar->AttachChild(m_research);

    m_trade = new StatisticIcon(m_research->UpperLeft().x-LAYOUT_MARGIN-ICON_DUAL_WIDTH,LAYOUT_MARGIN,ICON_DUAL_WIDTH,m_turn_update->Height(),
                                ClientUI::MeterIcon(METER_TRADE),
                                0,0,3,3,true,true,false,true);
    m_toolbar->AttachChild(m_trade);

    m_mineral = new StatisticIcon(m_trade->UpperLeft().x-LAYOUT_MARGIN-ICON_DUAL_WIDTH,LAYOUT_MARGIN,ICON_DUAL_WIDTH,m_turn_update->Height(),
                                  ClientUI::MeterIcon(METER_MINING),
                                  0,0,3,3,true,true,false,true);
    m_toolbar->AttachChild(m_mineral);

    m_food = new StatisticIcon(m_mineral->UpperLeft().x-LAYOUT_MARGIN-ICON_DUAL_WIDTH,LAYOUT_MARGIN,ICON_DUAL_WIDTH,m_turn_update->Height(),
                               ClientUI::MeterIcon(METER_FARMING),
                               0,0,3,3,true,true,false,true);
    m_toolbar->AttachChild(m_food);

    // chat display and chat input box
    m_chat_display = new GG::MultiEdit(LAYOUT_MARGIN, m_turn_update->LowerRight().y + LAYOUT_MARGIN, CHAT_WIDTH, CHAT_HEIGHT, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), GG::CLR_ZERO, 
                                       GG::MULTI_WORDBREAK | GG::MULTI_READ_ONLY | GG::MULTI_TERMINAL_STYLE | GG::MULTI_INTEGRAL_HEIGHT | GG::MULTI_NO_VSCROLL, 
                                       ClientUI::TextColor(), GG::CLR_ZERO, GG::Flags<GG::WndFlag>());
    AttachChild(m_chat_display);
    m_chat_display->SetMaxLinesOfHistory(100);
    m_chat_display->Hide();

    m_chat_edit = new CUIEdit(LAYOUT_MARGIN, GG::GUI::GetGUI()->AppHeight() - CHAT_EDIT_HEIGHT - LAYOUT_MARGIN, CHAT_WIDTH, "", 
                              GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::CtrlBorderColor(), ClientUI::TextColor(), GG::CLR_ZERO);
    AttachChild(m_chat_edit);
    m_chat_edit->Hide();
    EnableAlphaNumAccels();

    m_menu_showing = false;

    //clear background images
    m_backgrounds.clear();
    m_bg_scroll_rate.clear();


    // connect keyboard accelerators
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_ESCAPE), &MapWnd::ReturnToMap, this);

    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_RETURN), &MapWnd::OpenChatWindow, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_KP_ENTER), &MapWnd::OpenChatWindow, this);

    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_RETURN, GG::MOD_KEY_CTRL), &MapWnd::EndTurn, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_KP_ENTER, GG::MOD_KEY_CTRL), &MapWnd::EndTurn, this);

    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_F2), &MapWnd::ToggleSitRep, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_F3), &MapWnd::ToggleResearch, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_F4), &MapWnd::ToggleProduction, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_F5), &MapWnd::ToggleDesign, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_F10), &MapWnd::ShowMenu, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_s), &MapWnd::CloseSystemView, this);

    // Keys for zooming
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_e), &MapWnd::KeyboardZoomIn, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_KP_PLUS), &MapWnd::KeyboardZoomIn, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_r), &MapWnd::KeyboardZoomOut, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_KP_MINUS), &MapWnd::KeyboardZoomOut, this);

    // Keys for showing systems
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_d), &MapWnd::ZoomToHomeSystem, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_x), &MapWnd::ZoomToPrevOwnedSystem, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_c), &MapWnd::ZoomToNextOwnedSystem, this);

    // Keys for showing fleets
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_f), &MapWnd::ZoomToPrevIdleFleet, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_g), &MapWnd::ZoomToNextIdleFleet, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_v), &MapWnd::ZoomToPrevFleet, this);
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_b), &MapWnd::ZoomToNextFleet, this);

#ifndef FREEORION_RELEASE
    // special development-only key combo that dumps ValueRef, Condition, and Effect regression tests using the current
    // Universe
    GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_r, GG::MOD_KEY_CTRL), &RequestRegressionTestDump);
#endif

    g_chat_edit_history.push_front("");

#if TEST_BROWSE_INFO
    boost::shared_ptr<GG::BrowseInfoWnd> browser_wnd(new BrowseFoo());
    GG::Wnd::SetDefaultBrowseInfoWnd(browser_wnd);
#endif

    Connect(ClientApp::GetApp()->EmpireEliminatedSignal, &MapWnd::HandleEmpireElimination, this);
}

MapWnd::~MapWnd()
{
    delete m_toolbar;
    delete m_research_wnd;
    delete m_production_wnd;
    delete m_design_wnd;
    RemoveAccelerators();
}

GG::Pt MapWnd::ClientUpperLeft() const
{
    return UpperLeft() + GG::Pt(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight());
}

double MapWnd::ZoomFactor() const
{
    return m_zoom_factor;
}

GG::Pt MapWnd::ScreenCoordsFromUniversePosition(double universe_x, double universe_y) const
{
    GG::Pt cl_ul = ClientUpperLeft();
    int x = static_cast<int>((universe_x * m_zoom_factor) + cl_ul.x);
    int y = static_cast<int>((universe_y * m_zoom_factor) + cl_ul.y);
    return GG::Pt(x, y);
}

std::pair<double, double> MapWnd::UniversePositionFromScreenCoords(GG::Pt screen_coords) const
{
    GG::Pt cl_ul = ClientUpperLeft();
    double x = (screen_coords - cl_ul).x / m_zoom_factor;
    double y = (screen_coords - cl_ul).y / m_zoom_factor;
    return std::pair<double, double>(x, y);
}

SidePanel* MapWnd::GetSidePanel() const
{
    return m_side_panel;
}

void MapWnd::GetSaveGameUIData(SaveGameUIData& data) const
{
    data.map_left = UpperLeft().x;
    data.map_top = UpperLeft().y;
    data.map_zoom_factor = m_zoom_factor;
}

bool MapWnd::InProductionViewMode() const
{ return m_in_production_view_mode; }

void MapWnd::Render()
{
    // HACK! This is placed here so we can be sure it is executed frequently
    // (every time we render), and before we render any of the
    // FleetWnds/FleetDetailWnds.  It doesn't necessarily belong in MapWnd at
    // all.
    FleetUIManager::GetFleetUIManager().CullEmptyWnds();

    RenderStarfields();

    int interval = GetOptionsDB().Get<int>("UI.chat-hide-interval");
    if (!m_chat_edit->Visible() && g_chat_display_show_time && interval && 
        (interval < (GG::GUI::GetGUI()->Ticks() - g_chat_display_show_time) / 1000)) {
        m_chat_display->Hide();
        g_chat_display_show_time = 0;
    }

    GG::Pt origin_offset = UpperLeft() + GG::Pt(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight());
    glPushMatrix();
    glLoadIdentity();
    glScalef(m_zoom_factor, m_zoom_factor, 1.0);
    glTranslatef(origin_offset.x / m_zoom_factor, origin_offset.y / m_zoom_factor, 0.0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    RenderGalaxyGas();
    RenderNebulae();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_LINE_STIPPLE);
    RenderStarlanes();
    RenderFleetMovementLines();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_LINE_STIPPLE);
    glLineWidth(1.0);

    RenderSystems();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();
}

void MapWnd::KeyPress(GG::Key key, GG::Flags<GG::ModKey> mod_keys)
{
    switch (key) {
    case GG::GGK_TAB: { // auto-complete current chat edit word
        if (m_chat_edit->Visible()) {
            std::string text = m_chat_edit->WindowText();
            std::pair<int, int> cursor_pos = m_chat_edit->CursorPosn();
            if (cursor_pos.first == cursor_pos.second && 0 < cursor_pos.first && cursor_pos.first <= static_cast<int>(text.size())) {
                std::string::size_type word_start = text.substr(0, cursor_pos.first).find_last_of(" :");
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
                HumanClientApp::GetApp()->Networking().SendMessage(GlobalChatMessage(HumanClientApp::GetApp()->PlayerID(), edit_text));
            }
            m_chat_edit->Clear();
            m_chat_edit->Hide();
            EnableAlphaNumAccels();

            GG::GUI::GetGUI()->SetFocusWnd(this);
            g_chat_display_show_time = GG::GUI::GetGUI()->Ticks();
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

void MapWnd::LButtonDown(const GG::Pt &pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_drag_offset = pt - ClientUpperLeft();
}

void MapWnd::LDrag(const GG::Pt &pt, const GG::Pt &move, GG::Flags<GG::ModKey> mod_keys)
{
    GG::Pt move_to_pt = pt - m_drag_offset;
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ClientUpperLeft();
    m_side_panel->OffsetMove(-final_move);
    m_chat_display->OffsetMove(-final_move);
    m_chat_edit->OffsetMove(-final_move);
    m_sitrep_panel->OffsetMove(-final_move);

    MoveTo(move_to_pt - GG::Pt(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight()));
    m_dragged = true;
}

void MapWnd::LButtonUp(const GG::Pt &pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_drag_offset = GG::Pt(-1, -1);
    m_dragged = false;
}

void MapWnd::LClick(const GG::Pt &pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_drag_offset = GG::Pt(-1, -1);
    if (!m_dragged && !m_in_production_view_mode) {
        SelectSystem(UniverseObject::INVALID_OBJECT_ID);
        m_side_panel->Hide();
        DetachChild(m_side_panel);
    }
    m_dragged = false;
}

void MapWnd::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    // Attempt to close open fleet windows (if any are open and this is allowed), then attempt to close the SidePanel (if open);
    // if these fail, go ahead with the context-sensitive popup menu . Note that this enforces a one-close-per-click policy.

    if (GetOptionsDB().Get<bool>("UI.window-quickclose")) {
        if (FleetUIManager::GetFleetUIManager().CloseAll())
            return;

        if (m_side_panel->Visible()) {
            m_side_panel->Hide();
            DetachChild(m_side_panel);
            return;
        }
    }
}

void MapWnd::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{
    if (move)
        Zoom(move);
}

void MapWnd::InitTurn(int turn_number)
{
    SetAccelerators();

    Universe& universe = GetUniverse();

    //Logger().debugStream() << "MapWnd::InitTurn universe objects:";
    //for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it)
    //    Logger().debugStream() << " ... " << it->second->Name() << " with id " << it->first << " and systemID: " << it->second->SystemID();

    //Logger().debugStream() << "MapWnd::InitTurn universe destroyed objects:";
    //for (Universe::const_iterator it = universe.beginDestroyed(); it != universe.endDestroyed(); ++it)
    //    Logger().debugStream() << " ... " << it->second->Name() << " with id " << it->first;

    EmpireManager& manager = HumanClientApp::GetApp()->Empires();
    Empire* empire = manager.Lookup(HumanClientApp::GetApp()->EmpireID());
    if (!empire) {
        Logger().errorStream() << "MapWnd::InitTurn couldn't get an empire!";
        return;
    }


    // update effect accounting and meter estimates
    universe.InitMeterEstimatesAndDiscrepancies();

    // redo meter estimates with unowned planets marked as owned by player, so accurate predictions of planet
    // population is available for currently uncolonized planets
    UpdateMeterEstimates();

    const std::set<int>& this_player_explored_systems = empire->ExploredSystems();
    const std::map<int, std::set<int> > this_player_known_starlanes = empire->KnownStarlanes();

    // determine sytems where fleets can deliver supply, and groups of systems that can exchange resources
    for (EmpireManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        Empire* cur_empire = it->second;

        // use systems this client's player has explored for all empires, so that this client's player can
        // see where other empires can probably propegate supply, even if this client's empire / player
        // doesn't know what systems the other player has actually explored
        cur_empire->UpdateSupplyUnobstructedSystems(this_player_explored_systems);  

        cur_empire->UpdateSystemSupplyRanges();

        // similarly, use this client's player's known starlanes to propegate all empires' supply
        cur_empire->UpdateFleetSupply(this_player_known_starlanes);
        cur_empire->UpdateResourceSupply(this_player_known_starlanes);

        cur_empire->InitResourcePools();
    }


    Resize(GG::Pt(static_cast<int>(Universe::UniverseWidth() * s_max_scale_factor + GG::GUI::GetGUI()->AppWidth() * 1.5),
                  static_cast<int>(Universe::UniverseWidth() * s_max_scale_factor + GG::GUI::GetGUI()->AppHeight() * 1.5)));


    // set up backgrounds on first turn
    if (m_backgrounds.empty()) {
        std::vector<boost::shared_ptr<GG::Texture> > starfield_textures = ClientUI::GetClientUI()->GetPrefixedTextures(ClientUI::ArtDir(), "starfield", false);
        double scroll_rate = 1.0;
        for (std::vector<boost::shared_ptr<GG::Texture> >::const_iterator it = starfield_textures.begin(); it != starfield_textures.end(); ++it) {
            scroll_rate *= 0.5;
            m_backgrounds.push_back(*it);
            m_bg_scroll_rate.push_back(scroll_rate);
            glBindTexture(GL_TEXTURE_2D, (*it)->OpenGLId());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
    }

    // this gets cleared here instead of with the movement line stuff because that would clear some movement lines that come from the SystemIcons below
    m_fleet_lines.clear();
    ClearProjectedFleetMovementLines();

    // systems and starlanes
    for (std::map<int, SystemIcon*>::iterator it = m_system_icons.begin(); it != m_system_icons.end(); ++it)
        DeleteChild(it->second);
    m_system_icons.clear();

    std::map<boost::shared_ptr<GG::Texture>, std::vector<float> > raw_star_core_quad_vertices;
    std::map<boost::shared_ptr<GG::Texture>, std::vector<float> > raw_star_halo_quad_vertices;
    std::map<boost::shared_ptr<GG::Texture>, std::vector<float> > raw_galaxy_gas_quad_vertices;
    std::vector<float> raw_star_texture_coords;
    std::vector<float> raw_starlane_vertices;
    std::vector<unsigned char> raw_starlane_colors;
    std::vector<float> raw_starlane_supply_vertices;
    std::vector<unsigned char> raw_starlane_supply_colors;

    std::set<std::pair<int, int> > rendered_starlanes;      // stored by inserting return value of UnorderedIntPair so different orders of system ids don't create duplicates
    std::set<std::pair<int, int> > rendered_half_starlanes; // stored as unaltered pairs, so that a each direction of traversal can be shown separately

    Logger().debugStream() << "====ADDING STARLANES====";

    std::vector<System*> systems = universe.FindObjects<System>();
    for (unsigned int i = 0; i < systems.size(); ++i) {
        // system
        const System* start_system = systems[i];
        SystemIcon* icon = new SystemIcon(this, 0, 0, 10, start_system->ID());
        {
            // See note above texture coords for why we're making coordinate
            // sets that are 2x too big.
            const System& system = icon->GetSystem();
            double icon_size = ClientUI::SystemIconSize();
            double icon_ul_x = system.X() - icon_size;
            double icon_ul_y = system.Y() - icon_size;
            double icon_lr_x = system.X() + icon_size;
            double icon_lr_y = system.Y() + icon_size;
            if (icon->DiscTexture()) {
                glBindTexture(GL_TEXTURE_2D, icon->DiscTexture()->OpenGLId());
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                std::vector<float>& core_vertices = raw_star_core_quad_vertices[icon->DiscTexture()];
                core_vertices.push_back(icon_lr_x);
                core_vertices.push_back(icon_ul_y);
                core_vertices.push_back(icon_ul_x);
                core_vertices.push_back(icon_ul_y);
                core_vertices.push_back(icon_ul_x);
                core_vertices.push_back(icon_lr_y);
                core_vertices.push_back(icon_lr_x);
                core_vertices.push_back(icon_lr_y);
            }

            if (icon->HaloTexture()) {
                glBindTexture(GL_TEXTURE_2D, icon->HaloTexture()->OpenGLId());
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                std::vector<float>& halo_vertices = raw_star_halo_quad_vertices[icon->HaloTexture()];
                halo_vertices.push_back(icon_lr_x);
                halo_vertices.push_back(icon_ul_y);
                halo_vertices.push_back(icon_ul_x);
                halo_vertices.push_back(icon_ul_y);
                halo_vertices.push_back(icon_ul_x);
                halo_vertices.push_back(icon_lr_y);
                halo_vertices.push_back(icon_lr_x);
                halo_vertices.push_back(icon_lr_y);
            }
        }
        m_system_icons[start_system->ID()] = icon;
        icon->InstallEventFilter(this);
        AttachChild(icon);
        GG::Connect(icon->LeftClickedSignal,        &MapWnd::SystemLeftClicked,         this);
        GG::Connect(icon->RightClickedSignal,       &MapWnd::SystemRightClicked,        this);
        GG::Connect(icon->LeftDoubleClickedSignal,  &MapWnd::SystemDoubleClicked,       this);
        GG::Connect(icon->MouseEnteringSignal,      &MapWnd::MouseEnteringSystem,       this);
        GG::Connect(icon->MouseLeavingSignal,       &MapWnd::MouseLeavingSystem,        this);
        GG::Connect(icon->FleetButtonClickedSignal, &MapWnd::FleetButtonLeftClicked,    this);


        Logger().debugStream() << " considering lanes from " << start_system->Name() << " (id: " << start_system->ID() << ")";

            // gaseous substance around system
        if (boost::shared_ptr<GG::Texture> gaseous_texture = ClientUI::GetClientUI()->GetModuloTexture(ClientUI::ArtDir() / "galaxy_decoration", "gaseous", start_system->ID())) {
            glBindTexture(GL_TEXTURE_2D, gaseous_texture->OpenGLId());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            const double GAS_SIZE = ClientUI::SystemIconSize() * 12.0;
            const double ROTATION = start_system->ID() * 27.0; // arbitrary rotation in radians ("27.0" is just a number that produces pleasing results)
            const double COS_THETA = std::cos(ROTATION);
            const double SIN_THETA = std::sin(ROTATION);
            // This rotates the upper left and lower right corners CCW ROTATION
            // radians, around start_system.  The initial upper left and lower
            // right corners are (-GAS_SIZE, -GAS_SIZE) and (GAS_SIZE,
            // GAS_SIZE), in map coordinates respectively.  See note above
            // texture coords for why we're making coordinate sets that are 2x
            // too big.
            double gas_ul_x = start_system->X() + COS_THETA * -GAS_SIZE + SIN_THETA * GAS_SIZE;
            double gas_ul_y = start_system->Y() - -SIN_THETA * -GAS_SIZE + COS_THETA * GAS_SIZE;
            double gas_lr_x = start_system->X() + COS_THETA * GAS_SIZE + SIN_THETA * -GAS_SIZE;
            double gas_lr_y = start_system->Y() - -SIN_THETA * GAS_SIZE + COS_THETA * -GAS_SIZE;
            std::vector<float>& gas_vertices = raw_galaxy_gas_quad_vertices[gaseous_texture];
            gas_vertices.push_back(gas_lr_x);
            gas_vertices.push_back(gas_ul_y);
            gas_vertices.push_back(gas_ul_x);
            gas_vertices.push_back(gas_ul_y);
            gas_vertices.push_back(gas_ul_x);
            gas_vertices.push_back(gas_lr_y);
            gas_vertices.push_back(gas_lr_x);
            gas_vertices.push_back(gas_lr_y);
        }

        // system's starlanes
        for (System::const_lane_iterator lane_it = start_system->begin_lanes(); lane_it != start_system->end_lanes(); ++lane_it) {
            bool lane_is_wormhole = lane_it->second;
            if (lane_is_wormhole) continue; // at present, not rendering wormholes

            const System* dest_system = universe.Object<System>(lane_it->first);


            Logger().debugStream() << " considering lanes to " << dest_system->Name() << " (id: " << dest_system->ID() << ")";

            Logger().debugStream() << "added starlanes:";
            for (std::set<std::pair<int, int> >::const_iterator it = rendered_starlanes.begin(); it != rendered_starlanes.end(); ++it)
                Logger().debugStream() << " ... " << GetUniverse().Object(it->first)->Name() << " to " << GetUniverse().Object(it->second)->Name();

            Logger().debugStream() << "looking for " << start_system->Name() << " to " << dest_system->Name();

            // render starlane between start and dest systems?

            // check that this lane isn't already going to be rendered.  skip it if it is.
            if (rendered_starlanes.find(UnorderedIntPair(start_system->ID(), dest_system->ID())) == rendered_starlanes.end()) {
                Logger().debugStream() << " ... lane not found.";
                rendered_starlanes.insert(UnorderedIntPair(start_system->ID(), dest_system->ID()));

                raw_starlane_vertices.push_back(start_system->X());
                raw_starlane_vertices.push_back(start_system->Y());
                raw_starlane_vertices.push_back(dest_system->X());
                raw_starlane_vertices.push_back(dest_system->Y());

                // determine colour(s) for lane based on which empire(s) can transfer resources along the lane.
                // todo: multiple rendered lanes (one for each empire) when multiple empires use the same lane.

                GG::Clr lane_colour = UNOWNED_STARLANE_GRAY_CLR;    // default colour if no empires transfer

                for (EmpireManager::iterator empire_it = manager.begin(); empire_it != manager.end(); ++empire_it) {
                    empire = empire_it->second;
                    const std::set<std::pair<int, int> >& resource_supply_lanes = empire->ResourceSupplyStarlaneTraversals();

                    std::pair<int, int> lane_forward = std::make_pair(start_system->ID(), dest_system->ID());
                    std::pair<int, int> lane_backward = std::make_pair(dest_system->ID(), start_system->ID());

                    // see if this lane exists in this empire's supply propegation lanes set.  either direction accepted.
                    if (resource_supply_lanes.find(lane_forward) != resource_supply_lanes.end() || resource_supply_lanes.find(lane_backward) != resource_supply_lanes.end()) {
                        lane_colour = empire->Color();
                        Logger().debugStream() << "selected colour of empire " << empire->Name() << " for this full lane";
                        break;
                    }
                }

                if (lane_colour == UNOWNED_STARLANE_GRAY_CLR)
                    Logger().debugStream() << "selected unowned gray colour for this full lane";

                raw_starlane_colors.push_back(lane_colour.r);
                raw_starlane_colors.push_back(lane_colour.g);
                raw_starlane_colors.push_back(lane_colour.b);
                raw_starlane_colors.push_back(lane_colour.a);
                raw_starlane_colors.push_back(lane_colour.r);
                raw_starlane_colors.push_back(lane_colour.g);
                raw_starlane_colors.push_back(lane_colour.b);
                raw_starlane_colors.push_back(lane_colour.a);

                Logger().debugStream() << "adding full lane from " << start_system->Name() << " to " << dest_system->Name();
            }



            // render half-starlane from the current start_system to the current dest_system?

            // check that this lane isn't already going to be rendered.  skip it if it is.
            if (rendered_half_starlanes.find(std::make_pair(start_system->ID(), dest_system->ID())) == rendered_half_starlanes.end()) {
                // NOTE: this will never find a preexisting half lane
                Logger().debugStream() << "half lane not found... considering possible half lanes to add";

                // scan through possible empires to have a half-lane here and add a half-lane if one is found

                for (EmpireManager::iterator empire_it = manager.begin(); empire_it != manager.end(); ++empire_it) {
                    empire = empire_it->second;
                    const std::set<std::pair<int, int> >& resource_obstructed_supply_lanes = empire->ResourceSupplyOstructedStarlaneTraversals();

                    std::pair<int, int> lane_forward = std::make_pair(start_system->ID(), dest_system->ID());

                    // see if this lane exists in this empire's supply propegation lanes set.  either direction accepted.
                    if (resource_obstructed_supply_lanes.find(lane_forward) != resource_obstructed_supply_lanes.end()) {
                        // found an empire that has a half lane here, so add it.
                        rendered_half_starlanes.insert(std::make_pair(start_system->ID(), dest_system->ID()));  // inserted as ordered pair, so both directions can have different half-lanes

                        raw_starlane_vertices.push_back(start_system->X());
                        raw_starlane_vertices.push_back(start_system->Y());
                        raw_starlane_vertices.push_back((start_system->X() + dest_system->X()) * 0.5);  // half way along starlane
                        raw_starlane_vertices.push_back((start_system->Y() + dest_system->Y()) * 0.5);

                        const GG::Clr& lane_colour = empire->Color();
                        raw_starlane_colors.push_back(lane_colour.r);
                        raw_starlane_colors.push_back(lane_colour.g);
                        raw_starlane_colors.push_back(lane_colour.b);
                        raw_starlane_colors.push_back(lane_colour.a);
                        raw_starlane_colors.push_back(lane_colour.r);
                        raw_starlane_colors.push_back(lane_colour.g);
                        raw_starlane_colors.push_back(lane_colour.b);
                        raw_starlane_colors.push_back(lane_colour.a);

                        Logger().debugStream() << "Adding half lane between " << start_system->Name() << " to " << dest_system->Name() << " with colour of empire " << empire->Name();

                        break;
                    }
                }
            }
        }
    }


    // Note these coordinates assume the texture is twice as large as it should
    // be.  This allows us to use one set of texture coords for everything, even
    // though the star-halo textures must be rendered at sizes as much as twice
    // as large as the star-disc textures.
    for (std::size_t i = 0; i < systems.size(); ++i) {
        raw_star_texture_coords.push_back(1.5);
        raw_star_texture_coords.push_back(-0.5);
        raw_star_texture_coords.push_back(-0.5);
        raw_star_texture_coords.push_back(-0.5);
        raw_star_texture_coords.push_back(-0.5);
        raw_star_texture_coords.push_back(1.5);
        raw_star_texture_coords.push_back(1.5);
        raw_star_texture_coords.push_back(1.5);
    }

    DoSystemIconsLayout();


    // create animated lines indicating fleet supply flow
    for (EmpireManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        empire = it->second;
        const std::set<std::pair<int, int> >& fleet_supply_lanes = empire->FleetSupplyStarlaneTraversals();
        for (std::set<std::pair<int, int> >::const_iterator lane_it = fleet_supply_lanes.begin(); lane_it != fleet_supply_lanes.end(); ++lane_it) {
            const System* start_sys = universe.Object<System>(lane_it->first);
            const System* end_sys = universe.Object<System>(lane_it->second);
            raw_starlane_supply_vertices.push_back(start_sys->X());
            raw_starlane_supply_vertices.push_back(start_sys->Y());
            raw_starlane_supply_vertices.push_back(end_sys->X());
            raw_starlane_supply_vertices.push_back(end_sys->Y());
            raw_starlane_supply_colors.push_back(empire->Color().r);
            raw_starlane_supply_colors.push_back(empire->Color().g);
            raw_starlane_supply_colors.push_back(empire->Color().b);
            raw_starlane_supply_colors.push_back(empire->Color().a);
            raw_starlane_supply_colors.push_back(empire->Color().r);
            raw_starlane_supply_colors.push_back(empire->Color().g);
            raw_starlane_supply_colors.push_back(empire->Color().b);
            raw_starlane_supply_colors.push_back(empire->Color().a);
        }
    }


    // remove old fleet buttons for fleets not in systems
    for (unsigned int i = 0; i < m_moving_fleet_buttons.size(); ++i)
        DeleteChild(m_moving_fleet_buttons[i]);
    m_moving_fleet_buttons.clear();

    // disconnect old moving fleet statechangedsignal connections
    for (std::vector<boost::signals::connection>::iterator it = m_fleet_state_change_signals.begin(); it != m_fleet_state_change_signals.end(); ++it)
        it->disconnect();
    m_fleet_state_change_signals.clear();

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

        for (; it != local_end_it; ++it)
            IDs_by_empire_color[*it->second->Owners().begin()].push_back(it->second->ID());

        // create new fleetbutton for this cluster of fleets
        for (std::map<int, std::vector<int> >::iterator ID_it = IDs_by_empire_color.begin(); ID_it != IDs_by_empire_color.end(); ++ID_it) {
            FleetButton* fb = new FleetButton(Empires().Lookup(ID_it->first)->Color(), ID_it->second);
            m_moving_fleet_buttons.push_back(fb);
            AttachChild(fb);
            GG::Connect(fb->ClickedSignal, FleetButtonClickedFunctor(*fb, *this));
        }
    }
    // position fleetbuttons
    DoMovingFleetButtonsLayout();
    // create movement lines (after positioning buttons, so lines will originate from button location)
    for (std::vector<FleetButton*>::iterator it = m_moving_fleet_buttons.begin(); it != m_moving_fleet_buttons.end(); ++it)
        SetFleetMovementLine(*it);

    // connect fleet change signals to update moving fleet movement lines, so that ordering moving fleets to move
    // updates their displayed path
    for (Universe::ObjectVec::const_iterator it = fleets.begin(); it != fleets.end(); ++it) {
        const Fleet* moving_fleet = universe_object_cast<const Fleet*>(*it);
        if (!moving_fleet) {
            Logger().errorStream() << "MapWnd::InitTurn couldn't cast a (supposed) moving fleet pointer to a Fleet*";
            continue;
        }
        m_fleet_state_change_signals.push_back(GG::Connect(moving_fleet->StateChangedSignal, boost::bind(SetFleetMovementLineFunc, this, moving_fleet)));
    }

    MoveChildUp(m_side_panel);

    // set turn button to current turn
    m_turn_update->SetText(boost::io::str(FlexibleFormat(UserString("MAP_BTN_TURN_UPDATE")) %
                                          boost::lexical_cast<std::string>(turn_number)));
    MoveChildUp(m_turn_update);

    // are there any sitreps to show?
    m_sitrep_panel->Update();
    // HACK! The first time this SitRepPanel gets an update, the report row(s) are misaligned.  I have no idea why, and
    // I am sick of dealing with it, so I'm forcing another update in order to force it to behave.
    m_sitrep_panel->Update();
    empire = manager.Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire && empire->NumSitRepEntries()) {
        AttachChild(m_sitrep_panel);
        MoveChildUp(m_sitrep_panel);
        m_sitrep_panel->Show();
    } else {
        DetachChild(m_sitrep_panel);
        m_sitrep_panel->Hide();
    }

    m_research_wnd->Hide();
    m_production_wnd->Hide();
    m_design_wnd->Hide();
    m_in_production_view_mode = false;

    m_chat_edit->Hide();
    EnableAlphaNumAccels();


    if (m_zoom_factor * ClientUI::Pts() < MIN_SYSTEM_NAME_SIZE)
        HideSystemNames();
    else
        ShowSystemNames();

    // if we're at the default start position, the odds are very good that this is a fresh game
    if (ClientUpperLeft() == GG::Pt()) {
        // center the map on player's home system at the start of the game
        int capitol_id = empire->CapitolID();
        UniverseObject *obj = universe.Object(capitol_id);
        if (obj) {
            CenterOnMapCoord(obj->X(), obj->Y());
        } else {
            // default to centred on whole universe if there is no capitol
            CenterOnMapCoord(Universe::UniverseWidth() / 2, Universe::UniverseWidth() / 2);
        }

        // default the tech tree to be centred on something interesting
        m_research_wnd->Reset();
    }

    // empire is recreated each turn based on turn update from server, so connections of signals emitted from
    // the empire must be remade each turn (unlike connections to signals from the sidepanel)
    GG::Connect(empire->GetResourcePool(RE_FOOD)->ChangedSignal,            &MapWnd::RefreshFoodResourceIndicator,      this, 0);
    GG::Connect(empire->GetResourcePool(RE_MINERALS)->ChangedSignal,        &MapWnd::RefreshMineralsResourceIndicator,  this, 0);
    GG::Connect(empire->GetResourcePool(RE_TRADE)->ChangedSignal,           &MapWnd::RefreshTradeResourceIndicator,     this, 0);
    GG::Connect(empire->GetResourcePool(RE_RESEARCH)->ChangedSignal,        &MapWnd::RefreshResearchResourceIndicator,  this, 0);
    GG::Connect(empire->GetResourcePool(RE_INDUSTRY)->ChangedSignal,        &MapWnd::RefreshIndustryResourceIndicator,  this, 0);

    GG::Connect(empire->GetPopulationPool().ChangedSignal,                  &MapWnd::RefreshPopulationIndicator,        this, 1);

    GG::Connect(empire->GetProductionQueue().ProductionQueueChangedSignal,  &SidePanel::Refresh);


    m_toolbar->Show();
    m_FPS->Show();
    m_side_panel->Hide();   // prevents sidepanel from appearing if previous turn was ended without sidepanel open.  also ensures sidepanel UI updates properly, which it did not otherwise for unknown reasons.
    DetachChild(m_side_panel);
    SelectSystem(m_side_panel->SystemID());

    for (EmpireManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        empire = it->second;
        empire->UpdateResourcePools();
    }

    m_research_wnd->Update();
    m_production_wnd->Update();

    // clear out all the old buffers
    for (std::map<boost::shared_ptr<GG::Texture>, GLBuffer>::const_iterator it = m_star_core_quad_vertices.begin();
         it != m_star_core_quad_vertices.end();
         ++it) {
#ifdef FREEORION_WIN32
        glDeleteBuffersARB(1, &it->second.m_name);
#else
        glDeleteBuffers(1, &it->second.m_name);
#endif
    }
    m_star_core_quad_vertices.clear();
    for (std::map<boost::shared_ptr<GG::Texture>, GLBuffer>::const_iterator it =
             m_star_halo_quad_vertices.begin();
         it != m_star_halo_quad_vertices.end();
         ++it) {
#ifdef FREEORION_WIN32
        glDeleteBuffersARB(1, &it->second.m_name);
#else
        glDeleteBuffers(1, &it->second.m_name);
#endif
    }
    m_star_halo_quad_vertices.clear();
    for (std::map<boost::shared_ptr<GG::Texture>, GLBuffer>::const_iterator it =
             m_galaxy_gas_quad_vertices.begin();
         it != m_galaxy_gas_quad_vertices.end();
         ++it) {
#ifdef FREEORION_WIN32
        glDeleteBuffersARB(1, &it->second.m_name);
#else
        glDeleteBuffers(1, &it->second.m_name);
#endif
    }
    m_galaxy_gas_quad_vertices.clear();

    if (m_star_texture_coords.m_name) {
#ifdef FREEORION_WIN32
        glDeleteBuffersARB(1, &m_star_texture_coords.m_name);
#else
        glDeleteBuffers(1, &m_star_texture_coords.m_name);
#endif
        m_star_texture_coords.m_name = 0;
    }

    if (m_starlane_vertices.m_name) {
#ifdef FREEORION_WIN32
        glDeleteBuffersARB(1, &m_starlane_vertices.m_name);
#else
        glDeleteBuffers(1, &m_starlane_vertices.m_name);
#endif
        m_starlane_vertices.m_name = 0;
    }

    if (m_starlane_colors.m_name) {
#ifdef FREEORION_WIN32
        glDeleteBuffersARB(1, &m_starlane_colors.m_name);
#else
        glDeleteBuffers(1, &m_starlane_colors.m_name);
#endif
        m_starlane_colors.m_name = 0;
    }

    if (m_starlane_fleet_supply_vertices.m_name) {
#ifdef FREEORION_WIN32
        glDeleteBuffersARB(1, &m_starlane_fleet_supply_vertices.m_name);
#else
        glDeleteBuffers(1, &m_starlane_fleet_supply_vertices.m_name);
#endif
        m_starlane_fleet_supply_vertices.m_name = 0;
    }

    if (m_starlane_fleet_supply_colors.m_name) {
#ifdef FREEORION_WIN32
        glDeleteBuffersARB(1, &m_starlane_fleet_supply_colors.m_name);
#else
        glDeleteBuffers(1, &m_starlane_fleet_supply_colors.m_name);
#endif
        m_starlane_fleet_supply_colors.m_name = 0;
    }


    // create new buffers
    for (std::map<boost::shared_ptr<GG::Texture>, std::vector<float> >::const_iterator it =
             raw_star_core_quad_vertices.begin();
         it != raw_star_core_quad_vertices.end();
         ++it) {
        GLuint& name = m_star_core_quad_vertices[it->first].m_name;
#ifdef FREEORION_WIN32
        glGenBuffersARB(1, &name);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, name);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        it->second.size() * sizeof(float),
                        &it->second[0],
                        GL_STATIC_DRAW_ARB);
#else
        glGenBuffers(1, &name);
        glBindBuffer(GL_ARRAY_BUFFER, name);
        glBufferData(GL_ARRAY_BUFFER,
                     it->second.size() * sizeof(float),
                     &it->second[0],
                     GL_STATIC_DRAW);
#endif
        m_star_core_quad_vertices[it->first].m_size = it->second.size() / 2;
    }
    for (std::map<boost::shared_ptr<GG::Texture>, std::vector<float> >::const_iterator it =
             raw_star_halo_quad_vertices.begin();
         it != raw_star_halo_quad_vertices.end();
         ++it) {
        GLuint& name = m_star_halo_quad_vertices[it->first].m_name;
#ifdef FREEORION_WIN32
        glGenBuffersARB(1, &name);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, name);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        it->second.size() * sizeof(float),
                        &it->second[0],
                        GL_STATIC_DRAW_ARB);
#else
        glGenBuffers(1, &name);
        glBindBuffer(GL_ARRAY_BUFFER, name);
        glBufferData(GL_ARRAY_BUFFER,
                     it->second.size() * sizeof(float),
                     &it->second[0],
                     GL_STATIC_DRAW);
#endif
        m_star_halo_quad_vertices[it->first].m_size = it->second.size() / 2;
    }
    for (std::map<boost::shared_ptr<GG::Texture>, std::vector<float> >::const_iterator it =
             raw_galaxy_gas_quad_vertices.begin();
         it != raw_galaxy_gas_quad_vertices.end();
         ++it) {
        GLuint& name = m_galaxy_gas_quad_vertices[it->first].m_name;
#ifdef FREEORION_WIN32
        glGenBuffersARB(1, &name);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, name);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        it->second.size() * sizeof(float),
                        &it->second[0],
                        GL_STATIC_DRAW_ARB);
#else
        glGenBuffers(1, &name);
        glBindBuffer(GL_ARRAY_BUFFER, name);
        glBufferData(GL_ARRAY_BUFFER,
                     it->second.size() * sizeof(float),
                     &it->second[0],
                     GL_STATIC_DRAW);
#endif
        m_galaxy_gas_quad_vertices[it->first].m_size = it->second.size() / 2;
    }

    if (!raw_star_texture_coords.empty()) {
#ifdef FREEORION_WIN32
        glGenBuffersARB(1, &m_star_texture_coords.m_name);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_star_texture_coords.m_name);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        raw_star_texture_coords.size() * sizeof(float),
                        &raw_star_texture_coords[0],
                        GL_STATIC_DRAW_ARB);
#else
        glGenBuffers(1, &m_star_texture_coords.m_name);
        glBindBuffer(GL_ARRAY_BUFFER, m_star_texture_coords.m_name);
        glBufferData(GL_ARRAY_BUFFER,
                     raw_star_texture_coords.size() * sizeof(float),
                     &raw_star_texture_coords[0],
                     GL_STATIC_DRAW);
#endif
        m_star_texture_coords.m_size = raw_star_texture_coords.size() / 2;
    }
    if (!raw_starlane_vertices.empty()) {
#ifdef FREEORION_WIN32
        glGenBuffersARB(1, &m_starlane_vertices.m_name);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_starlane_vertices.m_name);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        raw_starlane_vertices.size() * sizeof(float),
                        &raw_starlane_vertices[0],
                        GL_STATIC_DRAW_ARB);
#else
        glGenBuffers(1, &m_starlane_vertices.m_name);
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_vertices.m_name);
        glBufferData(GL_ARRAY_BUFFER,
                     raw_starlane_vertices.size() * sizeof(float),
                     &raw_starlane_vertices[0],
                     GL_STATIC_DRAW);
#endif
        m_starlane_vertices.m_size = raw_starlane_vertices.size() / 2;
    }
    if (!raw_starlane_colors.empty()) {
#ifdef FREEORION_WIN32
        glGenBuffersARB(1, &m_starlane_colors.m_name);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_starlane_colors.m_name);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        raw_starlane_colors.size() * sizeof(unsigned char),
                        &raw_starlane_colors[0],
                        GL_STATIC_DRAW_ARB);
#else
        glGenBuffers(1, &m_starlane_colors.m_name);
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_colors.m_name);
        glBufferData(GL_ARRAY_BUFFER,
                     raw_starlane_colors.size() * sizeof(unsigned char),
                     &raw_starlane_colors[0],
                     GL_STATIC_DRAW);
#endif
        m_starlane_colors.m_size = raw_starlane_colors.size() / 4;
    }
    if (!raw_starlane_supply_vertices.empty()) {
#ifdef FREEORION_WIN32
        glGenBuffersARB(1, &m_starlane_fleet_supply_vertices.m_name);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_starlane_fleet_supply_vertices.m_name);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        raw_starlane_supply_vertices.size() * sizeof(float),
                        &raw_starlane_supply_vertices[0],
                        GL_STATIC_DRAW_ARB);
#else
        glGenBuffers(1, &m_starlane_fleet_supply_vertices.m_name);
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_fleet_supply_vertices.m_name);
        glBufferData(GL_ARRAY_BUFFER,
                     raw_starlane_supply_vertices.size() * sizeof(float),
                     &raw_starlane_supply_vertices[0],
                     GL_STATIC_DRAW);
#endif
        m_starlane_fleet_supply_vertices.m_size = raw_starlane_supply_vertices.size() / 2;
    }
    if (!raw_starlane_supply_colors.empty()) {
#ifdef FREEORION_WIN32
        glGenBuffersARB(1, &m_starlane_fleet_supply_colors.m_name);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_starlane_fleet_supply_colors.m_name);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        raw_starlane_supply_colors.size() * sizeof(unsigned char),
                        &raw_starlane_supply_colors[0],
                        GL_STATIC_DRAW_ARB);
#else
        glGenBuffers(1, &m_starlane_fleet_supply_colors.m_name);
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_fleet_supply_colors.m_name);
        glBufferData(GL_ARRAY_BUFFER,
                     raw_starlane_supply_colors.size() * sizeof(unsigned char),
                     &raw_starlane_supply_colors[0],
                     GL_STATIC_DRAW);
#endif
        m_starlane_fleet_supply_colors.m_size = raw_starlane_supply_colors.size() / 4;
    }

#ifdef FREEORION_WIN32
    glBindBufferARB(GL_ARRAY_BUFFER, 0);
#else
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}

void MapWnd::RestoreFromSaveData(const SaveGameUIData& data)
{
    m_zoom_factor = data.map_zoom_factor;

    DoSystemIconsLayout();
    DoMovingFleetButtonsLayout();

    GG::Pt ul = UpperLeft();
    GG::Pt map_ul = GG::Pt(data.map_left, data.map_top);
    GG::Pt map_move = map_ul - ul;
    OffsetMove(map_move);
    m_side_panel->OffsetMove(-map_move);
    m_chat_display->OffsetMove(-map_move);
    m_chat_edit->OffsetMove(-map_move);
    m_sitrep_panel->OffsetMove(-map_move);

    // this correction ensures that zooming in doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ul;
    m_side_panel->OffsetMove(-final_move);
    m_chat_display->OffsetMove(-final_move);
    m_chat_edit->OffsetMove(-final_move);
    m_sitrep_panel->OffsetMove(-final_move);

    MoveTo(move_to_pt - GG::Pt(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight()));
}

void MapWnd::ShowSystemNames()
{
    for (std::map<int, SystemIcon*>::iterator it = m_system_icons.begin(); it != m_system_icons.end(); ++it) {
        it->second->ShowName();
    }
}

void MapWnd::HideSystemNames()
{
    for (std::map<int, SystemIcon*>::iterator it = m_system_icons.begin(); it != m_system_icons.end(); ++it) {
        it->second->HideName();
    }
}

void MapWnd::HandlePlayerChatMessage(const std::string& msg)
{
    *m_chat_display += msg;
    m_chat_display->Show();
    g_chat_display_show_time = GG::GUI::GetGUI()->Ticks();
}

void MapWnd::CenterOnMapCoord(double x, double y)
{
    GG::Pt ul = ClientUpperLeft();
    double current_x = (GG::GUI::GetGUI()->AppWidth() / 2 - ul.x) / m_zoom_factor;
    double current_y = (GG::GUI::GetGUI()->AppHeight() / 2 - ul.y) / m_zoom_factor;
    GG::Pt map_move = GG::Pt(static_cast<int>((current_x - x) * m_zoom_factor), 
                             static_cast<int>((current_y - y) * m_zoom_factor));
    OffsetMove(map_move);
    m_side_panel->OffsetMove(-map_move);
    m_chat_display->OffsetMove(-map_move);
    m_chat_edit->OffsetMove(-map_move);
    m_sitrep_panel->OffsetMove(-map_move);

    // this correction ensures that the centering doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ul;
    m_side_panel->OffsetMove(-final_move);
    m_chat_display->OffsetMove(-final_move);
    m_chat_edit->OffsetMove(-final_move);
    m_sitrep_panel->OffsetMove(-final_move);

    MoveTo(move_to_pt - GG::Pt(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight()));
}

void MapWnd::ShowTech(const std::string& tech_name)
{
    if (!m_research_wnd->Visible())
        ToggleResearch();
    m_research_wnd->CenterOnTech(tech_name);
}

void MapWnd::ShowBuildingType(const std::string& building_type_name)
{
    //if (!m_production_wnd->Visible())
    //    ToggleProduction();
    //m_production_wnd->building_type_name);
}

void MapWnd::CenterOnObject(int id)
{
    if (UniverseObject* obj = GetUniverse().Object(id))
        CenterOnMapCoord(obj->X(), obj->Y());
}

void MapWnd::CenterOnObject(const UniverseObject* obj)
{
    if (!obj) return;
    CenterOnMapCoord(obj->X(), obj->Y());
}

void MapWnd::ReselectLastSystem()
{
    SelectSystem(m_previously_selected_system);
}

void MapWnd::SelectSystem(int system_id)
{
    // remove selection indicator from previously selected system
    int prev_system_id = m_side_panel->SystemID();
    if (prev_system_id != UniverseObject::INVALID_OBJECT_ID)
        m_system_icons[prev_system_id]->SetSelected(false);
    
    // place indicator on newly selected system
    if (system_id != UniverseObject::INVALID_OBJECT_ID)
        m_system_icons[system_id]->SetSelected(true);

    // show selected system in sidepanel(s)
    if (m_in_production_view_mode) {
        if (system_id != m_side_panel->SystemID()) {
            // only set selected system if newly selected system is different from before, otherwise planet rotation phase resets
            m_side_panel->SetSystem(system_id);
            m_production_wnd->SelectSystem(system_id);            
        }
        m_side_panel->Hide();   // only show ProductionWnd's sidepanel when ProductionWnd is open
        DetachChild(m_side_panel);
    } else {    
        if (!m_side_panel->Visible() || system_id != m_side_panel->SystemID()) {
            m_side_panel->SetSystem(system_id);
            
            // if selected an invalid system, hide sidepanel
            if (system_id == UniverseObject::INVALID_OBJECT_ID) {
                m_side_panel->Hide();
                DetachChild(m_side_panel);
            } else {
                AttachChild(m_side_panel);
                MoveChildUp(m_side_panel);
                MoveChildUp(m_sitrep_panel);
                m_side_panel->Show();
            }
        }
    }
}

void MapWnd::SelectFleet(int fleet_id)
{
    SelectFleet(GetUniverse().Object<Fleet>(fleet_id));
}

void MapWnd::SelectFleet(Fleet* fleet)
{
    if (!fleet) return;
    if (System* system = fleet->GetSystem()) {
        std::map<int, SystemIcon*>::iterator it = m_system_icons.find(system->ID());
        if (it != m_system_icons.end())
            it->second->ClickFleetButton(fleet);
    } else {
        for (unsigned int i = 0; i < m_moving_fleet_buttons.size(); ++i) {
            if (std::find(m_moving_fleet_buttons[i]->Fleets().begin(), m_moving_fleet_buttons[i]->Fleets().end(), fleet) != m_moving_fleet_buttons[i]->Fleets().end()) {
                m_moving_fleet_buttons[i]->LClick(GG::Pt(), GG::MOD_KEY_NONE);
                break;
            }
        }
    }
}

void MapWnd::SetFleetMovementLine(const FleetButton* fleet_button)
{
    assert(fleet_button);
    for (std::vector<Fleet*>::const_iterator it = fleet_button->Fleets().begin(); it != fleet_button->Fleets().end(); ++it) {
        Fleet* fleet = *it;
        m_fleet_lines[fleet] = MovementLineData(fleet_button, fleet->MovePath());
    }
}

void MapWnd::SetFleetMovementLine(const Fleet* fleet)
{
    if (!fleet) {
        Logger().errorStream() << "MapWnd::SetFleetMovementLine was passed a null fleet pointer";
        return;
    }

    if (const System* system = fleet->GetSystem()) {
        // if fleet is in a system, draw movement line from fleet button near system icon.

        // get system icon
        std::map<int, SystemIcon*>::const_iterator icon_it = m_system_icons.find(system->ID());
        if (icon_it != m_system_icons.end()) {
            const SystemIcon* icon = icon_it->second;
            // get fleet button
            const FleetButton* fleet_button = icon->GetFleetButton(fleet);

            if (!fleet_button) {
                Logger().errorStream() << "MapWnd::SetFleetMovement couldn't get a fleet button for a fleet in a system";
                return;
            }

            m_fleet_lines[fleet] = MovementLineData(fleet_button, fleet->MovePath());
        }
    } else {
        // fleet is not in a system, so fleet button is located at fleet's actual location, so can just
        // create movement line starting at fleet's actual universe position
        m_fleet_lines[fleet] = MovementLineData(fleet->X(), fleet->Y(), fleet->MovePath());
    }
}

void MapWnd::SetProjectedFleetMovementLine(const Fleet* fleet, const std::list<System*>& travel_route)
{
    if (!fleet)
        return;

    if (travel_route.empty()) {
        RemoveProjectedFleetMovementLine(fleet);
        return;
    }

    std::list<MovePathNode> path = fleet->MovePath(travel_route);

    if (path.empty()) {
        // no route to display
        RemoveProjectedFleetMovementLine(fleet);
        return;
    }

    GG::Clr line_colour = Empires().Lookup(*fleet->Owners().begin())->Color();

    // get starting location for fleet line
    std::pair<double, double> universe_position;

    // check if this MapWnd already has MovementLineData for this fleet
    std::map<const Fleet*, MovementLineData>::iterator it = m_fleet_lines.find(fleet);
    if (it != m_fleet_lines.end()) {
        // there is a fleet line already.  Its x and y are useful for the projected line, so it can be copied and tweaked a bit
        std::pair<double, double> start = it->second.Start();
        m_projected_fleet_lines[fleet] = MovementLineData(start.first, start.second, path, line_colour);
    } else {
        // there is no preexisting fleet line.  need to make one from scratch

        // -> need fleet position on screen

        // attempt to get system icon
        std::map<int, SystemIcon*>::const_iterator it = m_system_icons.find(fleet->SystemID());
        if (it != m_system_icons.end()) {
            // get fleet button
            const FleetButton* fleet_button = it->second->GetFleetButton(fleet);
            assert(fleet_button);
            m_projected_fleet_lines[fleet] = MovementLineData(fleet_button, path, line_colour);
        } else {
            // couldn't get a fleet button, so instead use fleet's own position
            m_projected_fleet_lines[fleet] = MovementLineData(fleet->X(), fleet->Y(), path, line_colour);
        }
    }
}

void MapWnd::SetProjectedFleetMovementLines(const std::vector<const Fleet*>& fleets, const std::list<System*>& travel_route)
{
    for (std::vector<const Fleet*>::const_iterator it = fleets.begin(); it != fleets.end(); ++it)
        SetProjectedFleetMovementLine(*it, travel_route);
}

void MapWnd::RemoveProjectedFleetMovementLine(const Fleet* fleet)
{
    std::map<const Fleet*, MovementLineData>::iterator it = m_projected_fleet_lines.find(fleet);
    if (it != m_projected_fleet_lines.end())
        m_projected_fleet_lines.erase(it);
}

void MapWnd::ClearProjectedFleetMovementLines()
{
    m_projected_fleet_lines.clear();
    for (std::map<const Fleet*, std::vector<FleetETAMapIndicator*> >::iterator map_it = m_projected_fleet_eta_map_indicators.begin();
         map_it != m_projected_fleet_eta_map_indicators.end(); ++map_it)
    {
        for (std::vector<FleetETAMapIndicator*>::iterator vec_it = map_it->second.begin(); vec_it != map_it->second.end(); ++vec_it)
            DeleteChild(*vec_it);
    }
    m_projected_fleet_eta_map_indicators.clear();
}

bool MapWnd::EventFilter(GG::Wnd* w, const GG::WndEvent& event)
{
    if (event.Type() == GG::WndEvent::RClick && FleetUIManager::GetFleetUIManager().empty()) {
        // Attempt to close the SidePanel (if open); if this fails, just let Wnd w handle it.  
        // Note that this enforces a one-close-per-click policy.

        if (GetOptionsDB().Get<bool>("UI.window-quickclose")) {
            if (m_side_panel->Visible()) {
                m_side_panel->Hide();
                DetachChild(m_side_panel);
                return true;
            }
        }
    }
    return false;
}

void MapWnd::DoSystemIconsLayout()
{
    // position and resize system icons and gaseous substance
    const int SYSTEM_ICON_SIZE = SystemIconSize();
    for (std::map<int, SystemIcon*>::iterator it = m_system_icons.begin(); it != m_system_icons.end(); ++it) {
        const System& system = it->second->GetSystem();

        GG::Pt icon_ul(static_cast<int>(system.X()*m_zoom_factor - SYSTEM_ICON_SIZE / 2.0),
                       static_cast<int>(system.Y()*m_zoom_factor - SYSTEM_ICON_SIZE / 2.0));
        it->second->SizeMove(icon_ul, icon_ul + GG::Pt(SYSTEM_ICON_SIZE, SYSTEM_ICON_SIZE));
    }
}

void MapWnd::DoMovingFleetButtonsLayout()
{
    const int FLEET_BUTTON_SIZE = FleetButtonSize();
    // position and resize unattached (to system icons) fleet icons
    for (unsigned int i = 0; i < m_moving_fleet_buttons.size(); ++i) {
        Fleet* fleet = *m_moving_fleet_buttons[i]->Fleets().begin();

        GG::Pt button_ul(static_cast<int>(fleet->X()*m_zoom_factor - FLEET_BUTTON_SIZE / 2.0), 
                         static_cast<int>(fleet->Y()*m_zoom_factor - FLEET_BUTTON_SIZE / 2.0));
        m_moving_fleet_buttons[i]->SizeMove(button_ul, button_ul + GG::Pt(FLEET_BUTTON_SIZE, FLEET_BUTTON_SIZE));
    }
}

int MapWnd::SystemIconSize() const
{
    return static_cast<int>(ClientUI::SystemIconSize() * m_zoom_factor);
}

int MapWnd::FleetButtonSize() const
{
    return static_cast<int>(SystemIconSize() * ClientUI::FleetButtonSize());
}

void MapWnd::Zoom(int delta)
{
    GG::Pt ul = ClientUpperLeft();
    double ul_x = ul.x, ul_y = ul.y;
    double center_x = GG::GUI::GetGUI()->AppWidth() / 2.0, center_y = GG::GUI::GetGUI()->AppHeight() / 2.0;
    double ul_offset_x = ul_x - center_x, ul_offset_y = ul_y - center_y;
    if (delta > 0) {
        if (m_zoom_factor * ZOOM_STEP_SIZE < s_max_scale_factor) {
            ul_offset_x *= ZOOM_STEP_SIZE;
            ul_offset_y *= ZOOM_STEP_SIZE;
            m_zoom_factor *= ZOOM_STEP_SIZE;
        } else {
            ul_offset_x *= s_max_scale_factor / m_zoom_factor;
            ul_offset_y *= s_max_scale_factor / m_zoom_factor;
            m_zoom_factor = s_max_scale_factor;
        }
    } else if (delta < 0) {
        if (s_min_scale_factor < m_zoom_factor / ZOOM_STEP_SIZE) {
            ul_offset_x /= ZOOM_STEP_SIZE;
            ul_offset_y /= ZOOM_STEP_SIZE;
            m_zoom_factor /= ZOOM_STEP_SIZE;
        } else {
            ul_offset_x *= s_min_scale_factor / m_zoom_factor;
            ul_offset_y *= s_min_scale_factor / m_zoom_factor;
            m_zoom_factor = s_min_scale_factor;
        }
    } else {
        return; // If delta == 0, no change
    }

    if (m_zoom_factor * ClientUI::Pts() < MIN_SYSTEM_NAME_SIZE)
        HideSystemNames();
    else
        ShowSystemNames();

    DoSystemIconsLayout();
    DoMovingFleetButtonsLayout();

    // translate map and UI widgets to account for the change in upper left due to zooming
    GG::Pt map_move(static_cast<int>((center_x + ul_offset_x) - ul_x),
                    static_cast<int>((center_y + ul_offset_y) - ul_y));
    OffsetMove(map_move);
    m_side_panel->OffsetMove(-map_move);
    m_chat_display->OffsetMove(-map_move);
    m_chat_edit->OffsetMove(-map_move);
    m_sitrep_panel->OffsetMove(-map_move);

    // this correction ensures that zooming in doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ul;
    m_side_panel->OffsetMove(-final_move);
    m_chat_display->OffsetMove(-final_move);
    m_chat_edit->OffsetMove(-final_move);
    m_sitrep_panel->OffsetMove(-final_move);

    MoveTo(move_to_pt - GG::Pt(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight()));
}

void MapWnd::RenderStarfields()
{
    glColor3d(1.0, 1.0, 1.0);

    GG::Pt origin_offset =
        UpperLeft() + GG::Pt(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight());
    glMatrixMode(GL_TEXTURE);

    for (unsigned int i = 0; i < m_backgrounds.size(); ++i) {
        float texture_coords_per_pixel_x = 1.0 / m_backgrounds[i]->Width();
        float texture_coords_per_pixel_y = 1.0 / m_backgrounds[i]->Height();
        glScalef(texture_coords_per_pixel_x * Width(),
                 texture_coords_per_pixel_y * Height(),
                 1.0);
        glTranslatef(-texture_coords_per_pixel_x * origin_offset.x / 16.0 * m_bg_scroll_rate[i],
                     -texture_coords_per_pixel_y * origin_offset.y / 16.0 * m_bg_scroll_rate[i],
                     0.0);
        glBindTexture(GL_TEXTURE_2D, m_backgrounds[i]->OpenGLId());
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex2i(0, 0);
        glTexCoord2f(0.0, 1.0);
        glVertex2i(0, Height());
        glTexCoord2f(1.0, 1.0);
        glVertex2i(Width(), Height());
        glTexCoord2f(1.0, 0.0);
        glVertex2i(Width(), 0);
        glEnd();
        glLoadIdentity();
    }

    glMatrixMode(GL_MODELVIEW);
}

void MapWnd::RenderNebulae()
{
    // nebula rendering disabled until we add nebulae worth rendering, which likely
    // means for them to have some gameplay purpose and artist-approved way to
    // specify what colours or specific nebula images to use

    //glColor4f(1.0, 1.0, 1.0, 1.0);
    //glPushMatrix();
    //glLoadIdentity();
    //for (unsigned int i = 0; i < m_nebulae.size(); ++i) {
    //    int nebula_width = m_nebulae[i]->Width() / 3;   // factor of 3 chosen to give ok-seeming nebula sizes for images in use at time of this writing
    //    int nebula_height = m_nebulae[i]->Height() / 3;

    //    GG::Pt ul = 
    //        ClientUpperLeft() + 
    //        GG::Pt(static_cast<int>((m_nebula_centers[i].x - nebula_width / 2.0) * m_zoom_factor),
    //               static_cast<int>((m_nebula_centers[i].y - nebula_height / 2.0) * m_zoom_factor));
    //    m_nebulae[i]->OrthoBlit(ul, 
    //                            ul + GG::Pt(static_cast<int>(nebula_width * m_zoom_factor), 
    //                                        static_cast<int>(nebula_height * m_zoom_factor)));
    //}
    //glPopMatrix();
}

void MapWnd::RenderGalaxyGas()
{
    if (!GetOptionsDB().Get<bool>("UI.galaxy-gas-background"))
        return;
    glColor4f(1.0, 1.0, 1.0, 1.0);
    for (std::map<boost::shared_ptr<GG::Texture>, GLBuffer>::const_iterator it = m_galaxy_gas_quad_vertices.begin();
         it != m_galaxy_gas_quad_vertices.end();
         ++it) {
        glBindTexture(GL_TEXTURE_2D, it->first->OpenGLId());
        // This is provided here to ensure maximum backwards compatability with
        // older hardware and GL drivers.  It can only work on a temporary
        // basis, however, since GL 2.0 will soon be required for the use of
        // Ogre, shaders, etc.  This note applies to all such uses of the ARB
        // versions of GL functions and macros.
#ifdef FREEORION_WIN32
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, it->second.m_name);
#else
        glBindBuffer(GL_ARRAY_BUFFER, it->second.m_name);
#endif
        glVertexPointer(2, GL_FLOAT, 0, 0);
#ifdef FREEORION_WIN32
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_star_texture_coords.m_name);
#else
        glBindBuffer(GL_ARRAY_BUFFER, m_star_texture_coords.m_name);
#endif
        glTexCoordPointer(2, GL_FLOAT, 0, 0);
        glDrawArrays(GL_QUADS, 0, it->second.m_size);
    }
}

void MapWnd::RenderSystems()
{
    const double HALO_SCALE_FACTOR = 1.0 + log10(m_zoom_factor);

    if (GetOptionsDB().Get<bool>("UI.optimized-system-rendering")) {
        glColor4f(1.0, 1.0, 1.0, 1.0);

        if (0.5 < HALO_SCALE_FACTOR) {
            glMatrixMode(GL_TEXTURE);
            glTranslatef(0.5, 0.5, 0.0);
            glScalef(1.0 / HALO_SCALE_FACTOR, 1.0 / HALO_SCALE_FACTOR, 1.0);
            glTranslatef(-0.5, -0.5, 0.0);
            for (std::map<boost::shared_ptr<GG::Texture>, GLBuffer>::const_iterator it = m_star_halo_quad_vertices.begin();
                 it != m_star_halo_quad_vertices.end();
                 ++it) {
                glBindTexture(GL_TEXTURE_2D, it->first->OpenGLId());
#ifdef FREEORION_WIN32
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, it->second.m_name);
#else
                glBindBuffer(GL_ARRAY_BUFFER, it->second.m_name);
#endif
                glVertexPointer(2, GL_FLOAT, 0, 0);
#ifdef FREEORION_WIN32
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_star_texture_coords.m_name);
#else
                glBindBuffer(GL_ARRAY_BUFFER, m_star_texture_coords.m_name);
#endif
                glTexCoordPointer(2, GL_FLOAT, 0, 0);
                glDrawArrays(GL_QUADS, 0, it->second.m_size);
            }
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
        }

        if (SystemIcon::TINY_SIZE < m_zoom_factor * ClientUI::SystemIconSize()) {
            for (std::map<boost::shared_ptr<GG::Texture>, GLBuffer>::const_iterator it = m_star_core_quad_vertices.begin();
                 it != m_star_core_quad_vertices.end();
                 ++it) {
                glBindTexture(GL_TEXTURE_2D, it->first->OpenGLId());
#ifdef FREEORION_WIN32
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, it->second.m_name);
#else
                glBindBuffer(GL_ARRAY_BUFFER, it->second.m_name);
#endif
                glVertexPointer(2, GL_FLOAT, 0, 0);
#ifdef FREEORION_WIN32
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_star_texture_coords.m_name);
#else
                glBindBuffer(GL_ARRAY_BUFFER, m_star_texture_coords.m_name);
#endif
                glTexCoordPointer(2, GL_FLOAT, 0, 0);
                glDrawArrays(GL_QUADS, 0, it->second.m_size);
            }
        }


#ifdef FREEORION_WIN32
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
#else
        glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
    } else {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glPushMatrix();
        glLoadIdentity();
        for (std::map<int, SystemIcon*>::const_iterator it = m_system_icons.begin(); it != m_system_icons.end(); ++it)
            it->second->ManualRender(HALO_SCALE_FACTOR);
        glPopMatrix();
    }
}

void MapWnd::RenderStarlanes()
{
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    if (m_starlane_vertices.m_name && m_starlane_colors.m_name) {
        glLineStipple(1, 0xffff);
        glLineWidth(STARLANE_WIDTH);
        glEnableClientState(GL_COLOR_ARRAY);
#ifdef FREEORION_WIN32
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_starlane_vertices.m_name);
#else
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_vertices.m_name);
#endif
        glVertexPointer(2, GL_FLOAT, 0, 0);
#ifdef FREEORION_WIN32
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_starlane_colors.m_name);
#else
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_colors.m_name);
#endif
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
        glDrawArrays(GL_LINES, 0, m_starlane_vertices.m_size);
        glDisableClientState(GL_COLOR_ARRAY);
    }

    if (m_starlane_fleet_supply_vertices.m_name && m_starlane_fleet_supply_colors.m_name) {
        // render fleet supply lines
        const GLushort PATTERN = 0x8080;    // = 1000000010000000  -> widely space small dots
        const int GLUSHORT_BIT_LENGTH = sizeof(GLushort) * 8;
        const double RATE = 0.1;            // slow crawl
        const int SHIFT = static_cast<int>(GG::GUI::GetGUI()->Ticks() * RATE / GLUSHORT_BIT_LENGTH) % GLUSHORT_BIT_LENGTH;
        const unsigned int STIPPLE = (PATTERN << SHIFT) | (PATTERN >> (GLUSHORT_BIT_LENGTH - SHIFT));
        glLineStipple(static_cast<int>(STARLANE_WIDTH), STIPPLE);
        glLineWidth(STARLANE_WIDTH);
        glEnableClientState(GL_COLOR_ARRAY);
#ifdef FREEORION_WIN32
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_starlane_fleet_supply_vertices.m_name);
#else
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_fleet_supply_vertices.m_name);
#endif
        glVertexPointer(2, GL_FLOAT, 0, 0);
#ifdef FREEORION_WIN32
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_starlane_fleet_supply_colors.m_name);
#else
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_fleet_supply_colors.m_name);
#endif
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
        glDrawArrays(GL_LINES, 0, m_starlane_fleet_supply_vertices.m_size);
        glDisableClientState(GL_COLOR_ARRAY);
    }

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void MapWnd::RenderFleetMovementLines()
{
    const double STARLANE_WIDTH = 2.5;
    glLineWidth(STARLANE_WIDTH);

    // standard movement line stipple
    const GLushort PATTERN = 0xF0F0;
    const int GLUSHORT_BIT_LENGTH = sizeof(GLushort) * 8;
    const double RATE = 0.25;
    const int SHIFT = static_cast<int>(GG::GUI::GetGUI()->Ticks() * RATE / GLUSHORT_BIT_LENGTH) % GLUSHORT_BIT_LENGTH;
    const unsigned int STIPPLE = (PATTERN << SHIFT) | (PATTERN >> (GLUSHORT_BIT_LENGTH - SHIFT));

    // render standard movement lines
    glLineStipple(static_cast<int>(STARLANE_WIDTH), STIPPLE);
    for (std::map<const Fleet*, MovementLineData>::const_iterator it = m_fleet_lines.begin(); it != m_fleet_lines.end(); ++it)
        RenderMovementLine(it->second);


    // projected movement line stipple
    const double PROJECTED_PATH_RATE = 0.35;
    const int PROJECTED_PATH_SHIFT =
        static_cast<int>(GG::GUI::GetGUI()->Ticks() * PROJECTED_PATH_RATE / GLUSHORT_BIT_LENGTH) % GLUSHORT_BIT_LENGTH;
    const unsigned int PROJECTED_PATH_STIPPLE =
        (PATTERN << PROJECTED_PATH_SHIFT) | (PATTERN >> (GLUSHORT_BIT_LENGTH - PROJECTED_PATH_SHIFT));

    //// render projected move liens
    glLineStipple(static_cast<int>(STARLANE_WIDTH), PROJECTED_PATH_STIPPLE);
    for (std::map<const Fleet*, MovementLineData>::const_iterator it = m_projected_fleet_lines.begin(); it != m_projected_fleet_lines.end(); ++it)
        RenderMovementLine(it->second);
}

void MapWnd::RenderMovementLine(const MapWnd::MovementLineData& move_line) {
    if (move_line.Path().empty() || move_line.Path().size() == 1)
        return;

    // get starting vertex
    std::pair<double, double> start = move_line.Start();
    double prev_vertex_x = start.first, prev_vertex_y = start.second;

    bool started = false;

    // draw lines connecting starting to second vertex, second to third, etc, 
    std::list<MovePathNode>::const_iterator path_it = move_line.Path().begin();
    ++path_it;
    for (; path_it != move_line.Path().end(); ++path_it) {
        // if this vertex can be reached, add vertices for line from previous to this vertex
        if (path_it->eta == Fleet::ETA_NEVER || path_it->eta == Fleet::ETA_NEVER || path_it->eta == Fleet::ETA_OUT_OF_RANGE)
            break;  // don't render additional legs of path that aren't reachable

        double cur_vertex_x = path_it->x, cur_vertex_y = path_it->y;

        // skip zero-length line segments since they seem to cause problems...
        if (cur_vertex_x == prev_vertex_x && cur_vertex_y == prev_vertex_y)
            continue;

        if (!started) {
            // this is obviously less efficient than using GL_LINE_STRIP, but GL_LINE_STRIP sometimes produces nasty artifacts 
            // when the begining of a line segment starts offscreen
            glBegin(GL_LINES);
            glColor(move_line.Colour());
            started = true;
        }

        glVertex2d(prev_vertex_x,   prev_vertex_y);
        glVertex2d(cur_vertex_x,    cur_vertex_y);

        // and update previous vertex for next iteration
        prev_vertex_x = cur_vertex_x;
        prev_vertex_y = cur_vertex_y;
    }

    if (started)
        glEnd();
}

void MapWnd::CorrectMapPosition(GG::Pt &move_to_pt)
{
    int contents_width = static_cast<int>(m_zoom_factor * Universe::UniverseWidth());
    int app_width =  GG::GUI::GetGUI()->AppWidth();
    int app_height = GG::GUI::GetGUI()->AppHeight();
    int map_margin_width = static_cast<int>(app_width / 2.0);

    if (app_width - map_margin_width < contents_width || app_height - map_margin_width < contents_width) {
        if (map_margin_width < move_to_pt.x)
            move_to_pt.x = map_margin_width;
        if (move_to_pt.x + contents_width < app_width - map_margin_width)
            move_to_pt.x = app_width - map_margin_width - contents_width;
        if (map_margin_width < move_to_pt.y)
            move_to_pt.y = map_margin_width;
        if (move_to_pt.y + contents_width < app_height - map_margin_width)
            move_to_pt.y = app_height - map_margin_width - contents_width;
    } else {
        if (move_to_pt.x < 0)
            move_to_pt.x = 0;
        if (app_width < move_to_pt.x + contents_width)
            move_to_pt.x = app_width - contents_width;
        if (move_to_pt.y < 0)
            move_to_pt.y = 0;
        if (app_height < move_to_pt.y + contents_width)
            move_to_pt.y = app_height - contents_width;
    }
}

void MapWnd::SystemDoubleClicked(int system_id)
{
    if (!m_in_production_view_mode) {
        if (!m_production_wnd->Visible())
            ToggleProduction();
        CenterOnObject(system_id);
        m_production_wnd->SelectSystem(system_id);
    }
}

void MapWnd::SystemLeftClicked(int system_id)
{
    SelectSystem(system_id);
    SystemLeftClickedSignal(system_id);
}

void MapWnd::SystemRightClicked(int system_id)
{
    if (!m_in_production_view_mode && FleetUIManager::GetFleetUIManager().ActiveFleetWnd()) {
        if (system_id == UniverseObject::INVALID_OBJECT_ID)
            ClearProjectedFleetMovementLines();
        else
            PlotFleetMovement(system_id, true);
    }
    SystemRightClickedSignal(system_id);
}

void MapWnd::MouseEnteringSystem(int system_id)
{
    if (!m_in_production_view_mode && FleetUIManager::GetFleetUIManager().ActiveFleetWnd())
        PlotFleetMovement(system_id, false);
    SystemBrowsedSignal(system_id);
}

void MapWnd::MouseLeavingSystem(int system_id)
{
    MouseEnteringSystem(UniverseObject::INVALID_OBJECT_ID);
}

void MapWnd::PlotFleetMovement(int system_id, bool execute_move)
{
    if (!FleetUIManager::GetFleetUIManager().ActiveFleetWnd())
        return;

    int empire_id = HumanClientApp::GetApp()->EmpireID();

    std::set<Fleet*> fleets = FleetUIManager::GetFleetUIManager().ActiveFleetWnd()->SelectedFleets();

    // apply to all this-player-owned fleets in currently-active FleetWnd
    for (std::set<Fleet*>::iterator it = fleets.begin(); it != fleets.end(); ++it) {
        Fleet* fleet = *it;
        // only give orders / plot prospective move paths of fleets owned by player
        if (!(fleet->OwnedBy(empire_id)) || !(fleet->NumShips()))
            continue;

        // plot empty move pathes if destination is not a known system
        if (system_id == UniverseObject::INVALID_OBJECT_ID) {
            RemoveProjectedFleetMovementLine(fleet);
            continue;
        }

        int fleet_sys_id = fleet->SystemID();

        int start_system = fleet_sys_id;
        if (fleet_sys_id == UniverseObject::INVALID_OBJECT_ID)
            start_system = fleet->NextSystemID();

        // get path to destination...
        std::list<System*> route = GetUniverse().ShortestPath(start_system, system_id, empire_id).first;

        // disallow "offroad" (direct non-starlane non-wormhole) travel
        if (route.size() == 2 && *route.begin() != *route.rbegin() &&
            !(*route.begin())->HasStarlaneTo((*route.rbegin())->ID()) && !(*route.begin())->HasWormholeTo((*route.rbegin())->ID()) &&
            !(*route.rbegin())->HasStarlaneTo((*route.begin())->ID()) && !(*route.rbegin())->HasWormholeTo((*route.begin())->ID())) {
            continue;
        }

        // if actually ordering fleet movement, not just prospectively previewing, ... do so
        if (execute_move && !route.empty())
            HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new FleetMoveOrder(empire_id, fleet->ID(), start_system, system_id)));

        // show route on map
        SetProjectedFleetMovementLine(fleet, route);
    }
}

void MapWnd::FleetButtonLeftClicked(FleetButton& fleet_btn, bool fleet_departing)
{
    if (m_in_production_view_mode) return;

    const std::vector<Fleet*>& btn_fleets = fleet_btn.Fleets();
    if (btn_fleets.empty())
        throw std::runtime_error("caught clicked signal for empty fleet button");

    Fleet* fleet = btn_fleets[0];

    System* system = fleet->GetSystem();
    int owner = *(fleet->Owners().begin());

    // find if a FleetWnd for this FleetButton's fleet(s) is already open
    FleetWnd* wnd_for_button = FleetUIManager::GetFleetUIManager().WndForFleet(fleet);

    if (!wnd_for_button) {
        // get all fleets at this location.  may be in a system, in which case fleets are separated into
        // departing or stationary; or may be away from any system, moving
        std::vector<Fleet*> fleets;
        if (system) {
            const System::ObjectVec owned_fleets = system->FindObjects(OwnedVisitor<Fleet>(owner));
            for (System::ObjectVec::const_iterator it = owned_fleets.begin(); it != owned_fleets.end(); ++it) {
                Fleet* owned_fleet = universe_object_cast<Fleet*>(*it);
                if (owned_fleet)
                    fleets.push_back(owned_fleet);
            }
        } else {
            std::copy(btn_fleets.begin(), btn_fleets.end(), std::back_inserter(fleets));
        }
        
        // determine whether this FleetWnd can't be manipulated by the users: can't manipulate other 
        // empires FleetWnds, and can't give orders to your fleets while they're en-route.
        bool read_only = false;
        if (owner != HumanClientApp::GetApp()->EmpireID() || !system)
            read_only = true;

        wnd_for_button = FleetUIManager::GetFleetUIManager().NewFleetWnd(fleets, 0, read_only);

        // position new FleetWnd.  default to last user-set position...
        GG::Pt wnd_position = FleetWnd::LastPosition();
        // unless the user hasn't opened and closed a FleetWnd yet, in which case use the lower-right
        if (wnd_position == GG::Pt())
            wnd_position = GG::Pt(5, GG::GUI::GetGUI()->AppHeight() - wnd_for_button->Height() - 5);

        wnd_for_button->MoveTo(wnd_position);

        // safety check to ensure window is on screen... may be redundant
        if (GG::GUI::GetGUI()->AppWidth() - 5 < wnd_for_button->LowerRight().x)
            wnd_for_button->OffsetMove(GG::Pt(GG::GUI::GetGUI()->AppWidth() - 5 - wnd_for_button->LowerRight().x, 0));
        if (GG::GUI::GetGUI()->AppHeight() - 5 < wnd_for_button->LowerRight().y)
            wnd_for_button->OffsetMove(GG::Pt(0, GG::GUI::GetGUI()->AppHeight() - 5 - wnd_for_button->LowerRight().y));
     }


    // if active fleet wnd hasn't changed, cycle through fleets
    if (FleetUIManager::GetFleetUIManager().ActiveFleetWnd() == wnd_for_button) {
        std::set<Fleet*> selected_fleets = FleetUIManager::GetFleetUIManager().ActiveFleetWnd()->SelectedFleets();

        const UniverseObject* selected_fleet = 0;

        if (selected_fleets.empty()) {
            // do nothing
        } else if (selected_fleets.size() > 1) {
            return; // don't mess up user's carefully selected fleets
        } else {
            selected_fleet = universe_object_cast<UniverseObject*>(*(selected_fleets.begin()));
        }

        if (system) {
            System::ObjectVec departing_fleets = system->FindObjects(OrderedMovingFleetVisitor(owner));
            System::ObjectVec stationary_fleets = system->FindObjects(StationaryFleetVisitor(owner));

            if (departing_fleets.empty() && stationary_fleets.empty()) return;

            if ((fleet_departing && !departing_fleets.empty()) || stationary_fleets.empty()) {
                // are assured there is at least one departing fleet

                // attempt to find already-selected fleet in departing fleets
                System::ObjectVec::iterator it;
                if (selected_fleet)
                    it = std::find(departing_fleets.begin(), departing_fleets.end(), selected_fleet);
                else
                    it = departing_fleets.end();

                if (it == departing_fleets.end() || it == departing_fleets.end() - 1) {
                    // selected fleet wasn't found, or it was found at the end, so select the first departing fleet

                    FleetUIManager::GetFleetUIManager().ActiveFleetWnd()->SelectFleet(universe_object_cast<Fleet*>(departing_fleets.front()));
                } else {
                    // it was found, and wasn't at the end, so select the next fleet after it
                    ++it;
                    FleetUIManager::GetFleetUIManager().ActiveFleetWnd()->SelectFleet(universe_object_cast<Fleet*>(*it));
                }
            } else {
                // are assured there is at least one stationary fleet

                // attempt to find already-selected fleet in departing fleets
                System::ObjectVec::iterator it;
                if (selected_fleet)
                    it = std::find(stationary_fleets.begin(), stationary_fleets.end(), selected_fleet);
                else
                    it = stationary_fleets.end();

                if (it == stationary_fleets.end() || it == stationary_fleets.end() - 1) {
                    // it wasn't found, or it was found at the end, so select the first stationary fleet
                    FleetUIManager::GetFleetUIManager().ActiveFleetWnd()->SelectFleet(universe_object_cast<Fleet*>(stationary_fleets.front()));
                } else {
                    // it was found, and wasn't at the end, so select the next fleet after it
                    ++it;
                    FleetUIManager::GetFleetUIManager().ActiveFleetWnd()->SelectFleet(universe_object_cast<Fleet*>(*it));
                }
            }
        } else {
            if (btn_fleets.empty()) return;
            // are assured there is at least one moving fleet

            // attempt to find already-selected fleet in moving fleets
            std::vector<Fleet*>::const_iterator it;
            if (selected_fleet)
                it = std::find(btn_fleets.begin(), btn_fleets.end(), selected_fleet);
            else
                it == btn_fleets.end();

            if (it == btn_fleets.end() || it == btn_fleets.end() - 1) {
                // it wasn't found, or it was found at the end, so select the first moving fleet
                FleetUIManager::GetFleetUIManager().ActiveFleetWnd()->SelectFleet(universe_object_cast<Fleet*>(btn_fleets.front()));
            } else {
                // it was found, and wasn't at the end, so select the next fleet after it
                ++it;
                FleetUIManager::GetFleetUIManager().ActiveFleetWnd()->SelectFleet(universe_object_cast<Fleet*>(*it));
            }
        }
    } else {
        FleetUIManager::GetFleetUIManager().SetActiveFleetWnd(wnd_for_button);
    }

}

void MapWnd::HandleEmpireElimination(int empire_id)
{}

void MapWnd::UniverseObjectDeleted(const UniverseObject *obj)
{
    const Fleet* fleet = universe_object_cast<const Fleet*>(obj);
    if (fleet) {
        std::map<const Fleet*, MovementLineData>::iterator it1 = m_fleet_lines.find(fleet);
        if (it1 != m_fleet_lines.end())
            m_fleet_lines.erase(it1);

        std::map<const Fleet*, std::vector<FleetETAMapIndicator*> >::iterator it2 = m_fleet_eta_map_indicators.find(fleet);
        if (it2 != m_fleet_eta_map_indicators.end()) {
            // clear all ETA indicators
            m_fleet_eta_map_indicators.erase(it2);
        }

        std::map<const Fleet*, MovementLineData>::iterator it3 = m_projected_fleet_lines.find(fleet);
        if (it3 != m_projected_fleet_lines.end())
            m_projected_fleet_lines.erase(it3);

        std::map<const Fleet*, std::vector<FleetETAMapIndicator*> >::iterator it4 = m_projected_fleet_eta_map_indicators.find(fleet);
        if (it4 != m_projected_fleet_eta_map_indicators.end()) {
            // clear all ETA indicators
            m_projected_fleet_eta_map_indicators.erase(it4);
        }
    }
}

void MapWnd::RegisterPopup(MapWndPopup* popup)
{
    if (popup)
        m_popups.push_back(popup);
}

void MapWnd::RemovePopup(MapWndPopup* popup)
{
    if (popup) {
        std::list<MapWndPopup*>::iterator it = std::find(m_popups.begin(), m_popups.end(), popup);
        if (it != m_popups.end())
            m_popups.erase(it);
    }
}

void MapWnd::Cleanup()
{
    CloseAllPopups();
    RemoveAccelerators();
    m_research_wnd->Hide();
    m_production_wnd->Hide();
    m_design_wnd->Hide();
    m_in_production_view_mode = false;
    m_toolbar->Hide();
    m_FPS->Hide();
}

void MapWnd::Sanitize()
{
    Cleanup();
    m_side_panel->MoveTo(GG::Pt(GG::GUI::GetGUI()->AppWidth() - SIDE_PANEL_WIDTH, m_toolbar->LowerRight().y));
    m_chat_display->MoveTo(GG::Pt(LAYOUT_MARGIN, m_turn_update->LowerRight().y + LAYOUT_MARGIN));
    m_chat_display->Clear();
    m_chat_edit->MoveTo(GG::Pt(LAYOUT_MARGIN, GG::GUI::GetGUI()->AppHeight() - CHAT_EDIT_HEIGHT - LAYOUT_MARGIN));
    m_sitrep_panel->MoveTo(GG::Pt((GG::GUI::GetGUI()->AppWidth() - SITREP_PANEL_WIDTH) / 2, (GG::GUI::GetGUI()->AppHeight() - SITREP_PANEL_HEIGHT) / 2));
    m_sitrep_panel->Resize(GG::Pt(SITREP_PANEL_WIDTH, SITREP_PANEL_HEIGHT));
    MoveTo(GG::Pt(-GG::GUI::GetGUI()->AppWidth(), -GG::GUI::GetGUI()->AppHeight()));
    m_zoom_factor = 1.0;
    m_research_wnd->Sanitize();
    m_production_wnd->Sanitize();
    m_design_wnd->Sanitize();
    m_previously_selected_system = UniverseObject::INVALID_OBJECT_ID;
}

bool MapWnd::ReturnToMap()
{
    if (m_sitrep_panel->Visible())
        m_sitrep_panel->Hide();
    if (m_research_wnd->Visible()) {
        m_research_wnd->Hide();
        HumanClientApp::GetApp()->MoveDown(m_research_wnd);
    }
    if (m_design_wnd->Visible()) {
        m_design_wnd->Hide();
        HumanClientApp::GetApp()->MoveDown(m_design_wnd);
        EnableAlphaNumAccels();
    }
    if (m_production_wnd->Visible()) {
        m_production_wnd->Hide();
        if (m_in_production_view_mode) {
            m_in_production_view_mode = false;
            ShowAllPopups();
            if (!m_side_panel->Visible())
                m_side_panel->SetSystem(m_side_panel->SystemID());
        }
        HumanClientApp::GetApp()->MoveDown(m_production_wnd);
    }
    return true;
}

bool MapWnd::OpenChatWindow()
{
    if (!m_chat_display->Visible() || !m_chat_edit->Visible()) {
        EnableAlphaNumAccels();
        m_chat_display->Show();
        DisableAlphaNumAccels();
        m_chat_edit->Show();
        GG::GUI::GetGUI()->SetFocusWnd(m_chat_edit);
        g_chat_display_show_time = GG::GUI::GetGUI()->Ticks();
        return true;
    }
    return false;
}

bool MapWnd::EndTurn()
{
    Cleanup();
    HumanClientApp::GetApp()->StartTurn();
    return true;
}

bool MapWnd::ToggleSitRep()
{
    ClearProjectedFleetMovementLines();
    if (m_sitrep_panel->Visible()) {
        DetachChild(m_sitrep_panel);
        m_sitrep_panel->Hide(); // necessary so it won't be visible when next toggled
    } else {
        // hide other "competing" windows
        m_research_wnd->Hide();
        HumanClientApp::GetApp()->MoveDown(m_research_wnd);
        if (m_design_wnd->Visible()) {
            m_design_wnd->Hide();
            HumanClientApp::GetApp()->MoveDown(m_design_wnd);
            EnableAlphaNumAccels();
        }
        m_production_wnd->Hide();
        if (m_in_production_view_mode) {
            m_in_production_view_mode = false;
            ShowAllPopups();
            if (!m_side_panel->Visible())
                m_side_panel->SetSystem(m_side_panel->SystemID());
        }
        HumanClientApp::GetApp()->MoveDown(m_production_wnd);

        // show the sitrep window
        AttachChild(m_sitrep_panel);
        MoveChildUp(m_sitrep_panel);
        m_sitrep_panel->Show();
    }
    return true;
}

bool MapWnd::ToggleResearch()
{
    ClearProjectedFleetMovementLines();
    if (m_research_wnd->Visible()) {
        m_research_wnd->Hide();
    } else {
        // hide other "competing" windows
        m_sitrep_panel->Hide();
        if (m_design_wnd->Visible()) {
            m_design_wnd->Hide();
            EnableAlphaNumAccels();
        }
        m_production_wnd->Hide();
        if (m_in_production_view_mode) {
            m_in_production_view_mode = false;
            ShowAllPopups();
            if (!m_side_panel->Visible())
                m_side_panel->SetSystem(m_side_panel->SystemID());
        }

        // show the research window
        m_research_wnd->Show();
        GG::GUI::GetGUI()->MoveUp(m_research_wnd);
    }
    return true;
}

bool MapWnd::ToggleProduction()
{
    ClearProjectedFleetMovementLines();
    if (m_production_wnd->Visible()) {
        m_production_wnd->Hide();
        m_in_production_view_mode = false;
        ShowAllPopups();
    } else {
        // hide other "competing" windows
        m_sitrep_panel->Hide();
        DetachChild(m_sitrep_panel);
        m_research_wnd->Hide();
        if (m_design_wnd->Visible()) {
            m_design_wnd->Hide();
            EnableAlphaNumAccels();
        }

        // show the production window
        m_production_wnd->Show();
        m_in_production_view_mode = true;
        HideAllPopups();

        m_side_panel->Hide();
        DetachChild(m_side_panel);

        GG::GUI::GetGUI()->MoveUp(m_production_wnd);

        // if no system is currently shown in sidepanel, default to this empire's home system (ie. where the capitol is)
        if (m_side_panel->SystemID() == UniverseObject::INVALID_OBJECT_ID)
            if (const Empire* empire = HumanClientApp::GetApp()->Empires().Lookup(HumanClientApp::GetApp()->EmpireID()))
                if (const UniverseObject* obj = GetUniverse().Object(empire->CapitolID()))
                    m_production_wnd->SelectSystem(obj->SystemID());
    }
    return true;
}

bool MapWnd::ToggleDesign()
{
    ClearProjectedFleetMovementLines();
    if (m_design_wnd->Visible()) {
        m_design_wnd->Hide();
        EnableAlphaNumAccels();
    } else {
        // hide other "competing" windows
        m_sitrep_panel->Hide();
        m_research_wnd->Hide();
        m_production_wnd->Hide();
        if (m_in_production_view_mode) {
            m_in_production_view_mode = false;
            ShowAllPopups();
            if (!m_side_panel->Visible())
                m_side_panel->SetSystem(m_side_panel->SystemID());
        }

        // show the design window
        m_design_wnd->Show();
        GG::GUI::GetGUI()->MoveUp(m_design_wnd);
        GG::GUI::GetGUI()->SetFocusWnd(m_design_wnd);
        DisableAlphaNumAccels();
        m_design_wnd->Reset();
    }
    return true;
}

bool MapWnd::ShowMenu()
{
    if (!m_menu_showing) {
        ClearProjectedFleetMovementLines();
        m_menu_showing = true;
        InGameMenu menu;
        menu.Run();
        m_menu_showing = false;
    }
    return true;
}

bool MapWnd::CloseSystemView()
{
    SelectSystem(UniverseObject::INVALID_OBJECT_ID);
    m_side_panel->Hide();   // redundant, but safer to keep in case the behavior of SelectSystem changes
    DetachChild(m_side_panel);
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

void MapWnd::RefreshFoodResourceIndicator()
{
    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );

    m_food->SetValue(empire->ResourceStockpile(RE_FOOD)); // set first value to stockpiled food

    double production = empire->ResourceProduction(RE_FOOD);
    double spent = empire->TotalFoodDistributed();

    m_food->SetValue(production - spent, 1);    // set second (bracketed) value to predicted stockpile change
}

void MapWnd::RefreshMineralsResourceIndicator()
{
    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );

    m_mineral->SetValue(empire->ResourceStockpile(RE_MINERALS));

    double production = empire->ResourceProduction(RE_MINERALS);
    double spent = empire->GetProductionQueue().TotalPPsSpent();

    m_mineral->SetValue(production - spent, 1);
}

void MapWnd::RefreshTradeResourceIndicator()
{
    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );

    m_trade->SetValue(empire->ResourceStockpile(RE_TRADE));

    double production = empire->ResourceProduction(RE_TRADE);
    double spent = empire->TotalTradeSpending();

    m_trade->SetValue(production - spent, 1);
}

void MapWnd::RefreshResearchResourceIndicator()
{
    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );
    m_research->SetValue(empire->ResourceProduction(RE_RESEARCH));
}

void MapWnd::RefreshIndustryResourceIndicator()
{
    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );
    m_industry->SetValue(empire->ResourceProduction(RE_INDUSTRY));
}

void MapWnd::RefreshPopulationIndicator()
{
    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );
    m_population->SetValue(empire->GetPopulationPool().Population());
    m_population->SetValue(empire->GetPopulationPool().Growth(), 1);
}

void MapWnd::UpdateMetersAndResourcePools()
{
    UpdateMeterEstimates();
    UpdateEmpireResourcePools();
}

void MapWnd::UpdateMetersAndResourcePools(const std::vector<int>& objects_vec)
{
    UpdateMeterEstimates(objects_vec);
    UpdateEmpireResourcePools();
}

void MapWnd::UpdateMetersAndResourcePools(int object_id, bool update_contained_objects)
{
    UpdateMeterEstimates(object_id, update_contained_objects);
    UpdateEmpireResourcePools();
}

void MapWnd::UpdateSidePanelSystemObjectMetersAndResourcePools()
{
    UpdateMetersAndResourcePools(m_side_panel->SystemID(), true);
}

void MapWnd::UpdateMeterEstimates()
{
    UpdateMeterEstimates(UniverseObject::INVALID_OBJECT_ID, false);
}

void MapWnd::UpdateMeterEstimates(int object_id, bool update_contained_objects)
{
    //Logger().debugStream() << "MapWnd::UpdateMeterEstimates";

    if (object_id == UniverseObject::INVALID_OBJECT_ID) {
        // update meters for all objects.  Value of updated_contained_objects is irrelivant and is ignored in this case.
        std::vector<int> object_ids;
        const Universe& universe = GetUniverse();
        for (Universe::const_iterator obj_it = universe.begin(); obj_it != universe.end(); ++obj_it)
            object_ids.push_back(obj_it->first);

        UpdateMeterEstimates(object_ids);
        return;
    }

    // collect objects to update meter for.  this may be a single object, a group of related objects, or all objects
    // in the (known) universe.  also clear effect accounting for meters that are to be updated.
    std::set<int> objects_set;
    std::list<int> objects_list;
    objects_list.push_back(object_id);

    for (std::list<int>::iterator list_it = objects_list.begin(); list_it !=  objects_list.end(); ++list_it) {
        int cur_object_id = *list_it;

        UniverseObject* cur_object = GetUniverse().Object(cur_object_id);
        if (!cur_object) {
            Logger().errorStream() << "MapWnd::UpdateMeterEstimates tried to get an invalid object...";
            return;
        }

        // add current object to list
        objects_set.insert(cur_object_id);


        // add contained objects within current object to list of objects to process, if requested.  assumes no objects contain themselves (which could cause infinite loops)
        if (update_contained_objects) {
            const std::vector<UniverseObject*> contained_objects = cur_object->FindObjects(); // get all contained objects
            for (std::vector<UniverseObject*>::const_iterator cont_it = contained_objects.begin(); cont_it != contained_objects.end(); ++cont_it)
                objects_list.push_back((*cont_it)->ID());
        }
    }
    std::vector<int> objects_vec;
    std::copy(objects_set.begin(), objects_set.end(), std::back_inserter(objects_vec));
    UpdateMeterEstimates(objects_vec);
}

void MapWnd::UpdateMeterEstimates(const std::vector<int>& objects_vec) {
    // add this player ownership to all planets in the objects_vec that aren't currently colonized.
    // this way, any effects the player knows about that would act on those planets if the player colonized them
    // include those planets in their scope.  This lets effects from techs the player knows alter the max
    // population of planet that is displayed to the player, even if those effects have a condition that causes
    // them to only act on planets the player owns (so as to not improve enemy planets if a player reseraches a
    // tech that should only benefit him/herself)

    int player_id = HumanClientApp::GetApp()->PlayerID();

    // get all planets the player knows about that aren't yet colonized (aren't owned by anyone).  Add this
    // the current player's ownership to all, while remembering which planets this is done to
    std::set<Planet*> unowned_planets;
    Universe::InhibitUniverseObjectSignals(true);
    for (std::vector<int>::const_iterator it = objects_vec.begin(); it != objects_vec.end(); ++it) {
         Planet* planet = GetUniverse().Object<Planet>(*it);
         if (!planet)
             continue;
         if (planet->Owners().empty()) {
             unowned_planets.insert(planet);
             planet->AddOwner(player_id);
         }
    }

    // update meter estimates with temporary ownership
    GetUniverse().UpdateMeterEstimates(objects_vec);

    // remove temporary ownership added above
    for (std::set<Planet*>::iterator it = unowned_planets.begin(); it != unowned_planets.end(); ++it)
        (*it)->RemoveOwner(player_id);
    Universe::InhibitUniverseObjectSignals(false);
}

void MapWnd::UpdateEmpireResourcePools()
{
    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );
    /* Recalculate stockpile, available, production, predicted change of resources.  When resourcepools
       update, they emit ChangeSignal, which is connected to MapWnd::RefreshFoodResourceIndicator, which
       updates the empire resource pool indicators of the MapWnd. */
    empire->UpdateResourcePools();

    // Update indicators on sidepanel, which are not directly connected to from the ResourcePool ChangedSignal
    m_side_panel->Refresh();
}

bool MapWnd::ZoomToHomeSystem()
{
    int id = Empires().Lookup(HumanClientApp::GetApp()->EmpireID())->HomeworldID();

    if (id != UniverseObject::INVALID_OBJECT_ID) {
        UniverseObject *object = GetUniverse().Object(id);
        if (!object) return false;
        CenterOnObject(object->SystemID());
        SelectSystem(object->SystemID());
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
        CenterOnObject(m_current_owned_system);
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
        CenterOnObject(m_current_owned_system);
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
        CenterOnObject(m_current_fleet);
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
        CenterOnObject(m_current_fleet);
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
        CenterOnObject(m_current_fleet);
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
        CenterOnObject(m_current_fleet);
        SelectFleet(m_current_fleet);
    }

    return true;
}

void MapWnd::SetAccelerators()
{
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_ESCAPE);

    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_RETURN);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_KP_ENTER);

    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_RETURN, GG::MOD_KEY_CTRL);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_KP_ENTER, GG::MOD_KEY_CTRL);

    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_F2);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_F3);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_F4);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_F10);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_s);

    // Keys for zooming
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_e);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_r);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_KP_PLUS);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_KP_MINUS);

    // Keys for showing systems
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_d);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_x);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_c);

    // Keys for showing fleets
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_f);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_g);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_v);
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_b);

#ifndef FREEORION_RELEASE
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_r, GG::MOD_KEY_CTRL);
#endif
}

void MapWnd::RemoveAccelerators()
{
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_ESCAPE);

    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_RETURN);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_KP_ENTER);
    
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_RETURN, GG::MOD_KEY_CTRL);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_KP_ENTER, GG::MOD_KEY_CTRL);

    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_F2);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_F3);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_F4);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_F10);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_s);

    // Zoom keys
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_e);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_r);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_KP_PLUS);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_KP_MINUS);

    // Keys for showing systems
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_d);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_x);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_c);

    // Keys for showing fleets
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_f);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_g);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_v);
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_b);

#ifndef FREEORION_RELEASE
    GG::GUI::GetGUI()->RemoveAccelerator(GG::GGK_r, GG::MOD_KEY_CTRL);
#endif
}

void MapWnd::DisableAlphaNumAccels()
{
    for (GG::GUI::const_accel_iterator i = GG::GUI::GetGUI()->accel_begin();
         i != GG::GUI::GetGUI()->accel_end(); ++i) {
        if (i->second != 0) // we only want to disable mod_keys without modifiers
            continue; 
        GG::Key key = i->first;
        if ((key >= GG::GGK_a && key <= GG::GGK_z) || 
            (key >= GG::GGK_0 && key <= GG::GGK_9)) {
            m_disabled_accels_list.insert(key);
        }
    }
    for (std::set<GG::Key>::iterator i = m_disabled_accels_list.begin();
         i != m_disabled_accels_list.end(); ++i) {
        GG::GUI::GetGUI()->RemoveAccelerator(*i);
    }
}

void MapWnd::EnableAlphaNumAccels()
{
    for (std::set<GG::Key>::iterator i = m_disabled_accels_list.begin();
         i != m_disabled_accels_list.end(); ++i) {
        GG::GUI::GetGUI()->SetAccelerator(*i);
    }
    m_disabled_accels_list.clear();
}

void MapWnd::CloseAllPopups()
{
    for (std::list<MapWndPopup*>::iterator it = m_popups.begin(); it != m_popups.end(); ) {
        // get popup and increment iterator first since closing the popup will change this list by removing the popup
        MapWndPopup* popup = *it++;
        popup->Close();
    }   
    // clear list
    m_popups.clear();
}

void MapWnd::HideAllPopups()
{
    for (std::list<MapWndPopup*>::iterator it = m_popups.begin(); it != m_popups.end(); ++it) {
        (*it)->Hide();
    }
}

void MapWnd::ShowAllPopups()
{
    for (std::list<MapWndPopup*>::iterator it = m_popups.begin(); it != m_popups.end(); ++it) {
        (*it)->Show();
    }
}

MapWnd::FleetButtonClickedFunctor::FleetButtonClickedFunctor(FleetButton& fleet_btn, MapWnd& map_wnd) :
    m_fleet_btn(fleet_btn),
    m_map_wnd(map_wnd)
{}

void MapWnd::FleetButtonClickedFunctor::operator()()
{
    m_map_wnd.FleetButtonLeftClicked(m_fleet_btn, false);
}

