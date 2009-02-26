#ifdef FREEORION_WIN32
#include <GL/glew.h>
#endif

#include "MapWnd.h"

#include "ChatWnd.h"
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


namespace {
    const double    ZOOM_STEP_SIZE = std::pow(2.0, 1.0/3.0);
    const double    ZOOM_IN_MAX_STEPS = 9.0;
    const double    ZOOM_IN_MIN_STEPS = -4.0;   // negative zoom steps indicates zooming out
    const int       ZOOM_TOTAL_STEPS = ZOOM_IN_MAX_STEPS + 1 + ZOOM_IN_MIN_STEPS;
    const double    ZOOM_MAX = std::pow(ZOOM_STEP_SIZE, ZOOM_IN_MAX_STEPS);
    const double    ZOOM_MIN = std::pow(ZOOM_STEP_SIZE, ZOOM_IN_MIN_STEPS);
    const int       MIN_NEBULAE = 3; // this min and max are for a 1000.0-width galaxy
    const int       MAX_NEBULAE = 6;
    const GG::X     END_TURN_BTN_WIDTH(60);
    const GG::X     SITREP_PANEL_WIDTH(400);
    const GG::Y     SITREP_PANEL_HEIGHT(300);
    const GG::Y     ZOOM_SLIDER_HEIGHT(200);
    const int       MIN_SYSTEM_NAME_SIZE = 10;
    const int       LAYOUT_MARGIN = 5;

    double  ZoomScaleFactor(double steps_in) {
        if (steps_in > ZOOM_IN_MAX_STEPS) {
            Logger().errorStream() << "ZoomScaleFactor passed steps in (" << steps_in << ") higher than max (" << ZOOM_IN_MAX_STEPS << "), so using max";
            steps_in = ZOOM_IN_MAX_STEPS;
        } else if (steps_in < ZOOM_IN_MIN_STEPS) {
            Logger().errorStream() << "ZoomScaleFactor passed steps in (" << steps_in << ") lower than minimum (" << ZOOM_IN_MIN_STEPS << "), so using min";
            steps_in = ZOOM_IN_MIN_STEPS;
        }
        return std::pow(ZOOM_STEP_SIZE, steps_in);
    }

    struct BoolToVoidAdapter
    {
        BoolToVoidAdapter(const boost::function<bool ()>& fn) : m_fn(fn) {}
        void operator()() { m_fn(); }
        boost::function<bool ()> m_fn;
    };

    void AddOptions(OptionsDB& db)
    {
        db.Add("UI.galaxy-gas-background",          "OPTIONS_DB_GALAXY_MAP_GAS",                    true,   Validator<bool>());
        db.Add("UI.galaxy-starfields",              "OPTIONS_DB_GALAXY_MAP_STARFIELDS",             true,   Validator<bool>());
        db.Add("UI.optimized-system-rendering",     "OPTIONS_DB_OPTIMIZED_SYSTEM_RENDERING",        true,   Validator<bool>());
        db.Add("UI.starlane-thickness",             "OPTIONS_DB_STARLANE_THICKNESS",                2.5,    RangedStepValidator<double>(0.25, 0.25, 10.0));
        db.Add("UI.resource-starlane-colouring",    "OPTIONS_DB_RESOURCE_STARLANE_COLOURING",       true,   Validator<bool>());
        db.Add("UI.fleet-supply-lines",             "OPTIONS_DB_FLEET_SUPPLY_LINES",                true,   Validator<bool>());
        db.Add("UI.fleet-supply-line-width",        "OPTIONS_DB_FLEET_SUPPLY_LINE_WIDTH",           3.0,    RangedStepValidator<double>(0.25, 0.25, 10.0));
        db.Add("UI.unowned-starlane-colour",        "OPTIONS_DB_UNOWNED_STARLANE_COLOUR",           StreamableColor(GG::Clr(72,  72,  72,  255)),   Validator<StreamableColor>());

        db.Add("UI.system-circles",                 "OPTIONS_DB_UI_SYSTEM_CIRCLES",                 true,       Validator<bool>());
        db.Add("UI.system-circle-size",             "OPTIONS_DB_UI_SYSTEM_CIRCLE_SIZE",             1.0,        RangedStepValidator<double>(0.1, 1.0, 2.5));
        db.Add("UI.system-icon-size",               "OPTIONS_DB_UI_SYSTEM_ICON_SIZE",               14,         RangedValidator<int>(8, 50));
        db.Add("UI.system-name-unowned-color",      "OPTIONS_DB_UI_SYSTEM_NAME_UNOWNED_COLOR",      StreamableColor(GG::Clr(160, 160, 160, 255)),   Validator<StreamableColor>());
        db.Add("UI.system-selection-indicator-size","OPTIONS_DB_UI_SYSTEM_SELECTION_INDICATOR_SIZE",1.6,        RangedStepValidator<double>(0.1, 0.5, 5));
        db.Add("UI.tiny-fleet-button-minimum-zoom", "OPTIONS_DB_UI_TINY_FLEET_BUTTON_MIN_ZOOM",     0.75,       RangedStepValidator<double>(0.125, 0.125, 4.0));
        db.Add("UI.small-fleet-button-minimum-zoom","OPTIONS_DB_UI_SMALL_FLEET_BUTTON_MIN_ZOOM",    1.50,       RangedStepValidator<double>(0.125, 0.125, 4.0));
        db.Add("UI.medium-fleet-button-minimum-zoom","OPTIONS_DB_UI_MEDIUM_FLEET_BUTTON_MIN_ZOOM",  4.00,       RangedStepValidator<double>(0.125, 0.125, 4.0));
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


    static bool checked_gl_version_already = false;
    void CheckGLVersion() {
        // only execute once
        if (checked_gl_version_already)
            return;
        else
            checked_gl_version_already = true;

        // get OpenGL version string and parse to get version number
        const GLubyte* gl_version = glGetString(GL_VERSION);
        std::string gl_version_string = boost::lexical_cast<std::string>(gl_version);
        Logger().debugStream() << "OpenGL version string: " << boost::lexical_cast<std::string>(gl_version);

        float version_number = 0.0;
        std::istringstream iss(gl_version_string);
        iss >> version_number;
        version_number += 0.05f;    // ensures proper rounding of 1.1 digit number

        Logger().debugStream() << "...extracted version number: " << DoubleToString(version_number, 2, false, false);    // combination of floating point precision and DoubleToString preferring to round down means the +0.05 is needed to round properly

        if (version_number < 2.0)
            Logger().debugStream() << "OpenGL version number less than 2.0.  FreeOrion requires OpenGL version 2.0 or greater, so you may have problems on this system.";
    }


    // disambiguate overloaded function with a function pointer
    void (MapWnd::*SetFleetMovementLineFunc)(const Fleet*) = &MapWnd::SetFleetMovementLine;
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
MapWndPopup::MapWndPopup(const std::string& t, GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Flags<GG::WndFlag> flags) :
    CUIWnd(t, x, y, w, h, flags)
{ ClientUI::GetClientUI()->GetMapWnd()->RegisterPopup(this); }

MapWndPopup::~MapWndPopup()
{ ClientUI::GetClientUI()->GetMapWnd()->RemovePopup(this); }

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


//////////////////////////////////
// MapWnd::FleetETAMapIndicator
//////////////////////////////////
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
    GG::Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::Flags<GG::WndFlag>()),
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

    m_text = new GG::TextControl(GG::X0, GG::Y0, eta_text, ClientUI::GetFont(),
                                 ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
    Resize(m_text->Size());
    AttachChild(m_text);
}

void MapWnd::FleetETAMapIndicator::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::FlatRectangle(ul, lr, GG::CLR_BLACK, ClientUI::WndInnerBorderColor(), 1);
}


///////////////////////////
// MapWnd
///////////////////////////
MapWnd::MapWnd() :
    GG::Wnd(-GG::GUI::GetGUI()->AppWidth(), -GG::GUI::GetGUI()->AppHeight(),
            static_cast<GG::X>(Universe::UniverseWidth() * ZOOM_MAX + GG::GUI::GetGUI()->AppWidth() * 1.5),
            static_cast<GG::Y>(Universe::UniverseWidth() * ZOOM_MAX + GG::GUI::GetGUI()->AppHeight() * 1.5),
            GG::CLICKABLE | GG::DRAGABLE),
    m_backgrounds(),
    m_bg_scroll_rate(),
    m_previously_selected_system(UniverseObject::INVALID_OBJECT_ID),
    m_zoom_steps_in(0.0),
    m_side_panel(NULL),
    m_system_icons(),
    m_sitrep_panel(NULL),
    m_research_wnd(NULL),
    m_production_wnd(NULL),
    m_design_wnd(NULL),
    m_moving_fleet_buttons(),
    m_system_fleet_buttons(),
    m_fleet_state_change_signals(),
    m_system_fleet_state_change_signals(),
    m_keyboard_accelerator_signals(),
    m_fleet_lines(),
    m_fleet_eta_map_indicators(),
    m_projected_fleet_lines(),
    m_projected_fleet_eta_map_indicators(),
    m_star_core_quad_vertices(),
    m_star_halo_quad_vertices(),
    m_galaxy_gas_quad_vertices(),
    m_star_texture_coords(),
    m_starlane_vertices(),
    m_starlane_colors(),
    m_starlane_fleet_supply_vertices(),
    m_starlane_fleet_supply_colors(),
    m_drag_offset(-GG::X1, -GG::Y1),
    m_dragged(false),
    m_turn_update(NULL),
    m_popups(),
    m_menu_showing(false),
    m_current_owned_system(UniverseObject::INVALID_OBJECT_ID),
    m_current_fleet(UniverseObject::INVALID_OBJECT_ID),
    m_in_production_view_mode(false),
    m_toolbar(NULL),
    m_food(NULL),
    m_mineral(NULL),
    m_trade(NULL),
    m_population(NULL),
    m_research(NULL),
    m_industry(NULL),
    m_btn_siterep(NULL),
    m_btn_research(NULL),
    m_btn_production(NULL),
    m_btn_design(NULL),
    m_btn_menu(NULL),
    m_FPS(NULL),
    m_zoom_slider(NULL)
{
    SetName("MapWnd");

    Connect(GetUniverse().UniverseObjectDeleteSignal, &MapWnd::UniverseObjectDeleted, this);

    // toolbar
    m_toolbar = new CUIToolBar(GG::X0,GG::Y0,GG::GUI::GetGUI()->AppWidth(),GG::Y(30));
    GG::GUI::GetGUI()->Register(m_toolbar);
    m_toolbar->Hide();

    // system-view side panel
    const GG::X SIDEPANEL_WIDTH(GetOptionsDB().Get<int>("UI.sidepanel-width"));
    const GG::X APP_WIDTH(GG::GUI::GetGUI()->AppWidth());
    const GG::Y APP_HEIGHT(GG::GUI::GetGUI()->AppHeight());

    m_side_panel = new SidePanel(APP_WIDTH - SIDEPANEL_WIDTH, m_toolbar->LowerRight().y, APP_HEIGHT);
    GG::Connect(m_side_panel->SystemSelectedSignal,         &MapWnd::SelectSystem, this);                                               // sidepanel requests system selection change -> select it
    GG::Connect(m_side_panel->ResourceCenterChangedSignal,  &MapWnd::UpdateSidePanelSystemObjectMetersAndResourcePools, this);   // something in sidepanel changed resource pool(s), so need to recalculate and update meteres and resource pools and refresh their indicators

    m_sitrep_panel = new SitRepPanel( (APP_WIDTH-SITREP_PANEL_WIDTH)/2, (APP_HEIGHT-SITREP_PANEL_HEIGHT)/2, SITREP_PANEL_WIDTH, SITREP_PANEL_HEIGHT );
    GG::Connect(m_sitrep_panel->ClosingSignal, BoolToVoidAdapter(boost::bind(&MapWnd::ToggleSitRep, this)));    // sitrep panel is manually closed by user

    m_research_wnd = new ResearchWnd(APP_WIDTH, APP_HEIGHT - m_toolbar->Height());
    m_research_wnd->MoveTo(GG::Pt(GG::X0, m_toolbar->Height()));
    GG::GUI::GetGUI()->Register(m_research_wnd);
    m_research_wnd->Hide();

    m_production_wnd = new ProductionWnd(APP_WIDTH, APP_HEIGHT - m_toolbar->Height());
    m_production_wnd->MoveTo(GG::Pt(GG::X0, m_toolbar->Height()));
    GG::GUI::GetGUI()->Register(m_production_wnd);
    m_production_wnd->Hide();
    GG::Connect(m_production_wnd->SystemSelectedSignal, &MapWnd::SelectSystem, this); // productionwnd requests system selection change -> select it

    m_design_wnd = new DesignWnd(APP_WIDTH, APP_HEIGHT - m_toolbar->Height());
    m_design_wnd->MoveTo(GG::Pt(GG::X0, m_toolbar->Height()));
    GG::GUI::GetGUI()->Register(m_design_wnd);
    m_design_wnd->Hide();

    // turn button
    m_turn_update = new CUITurnButton(GG::X(LAYOUT_MARGIN), GG::Y(LAYOUT_MARGIN), END_TURN_BTN_WIDTH, "");
    m_toolbar->AttachChild(m_turn_update);
    GG::Connect(m_turn_update->ClickedSignal, BoolToVoidAdapter(boost::bind(&MapWnd::EndTurn, this)));

    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
    const GG::X BUTTON_TOTAL_MARGIN(8);

    // FPS indicator
    m_FPS = new FPSIndicator(m_turn_update->LowerRight().x + LAYOUT_MARGIN, m_turn_update->UpperLeft().y);
    m_toolbar->AttachChild(m_FPS);

    // Zoom slider
    //const int ZOOM_SLIDER_MIN = static_cast<int>(ZOOM_IN_MIN_STEPS),
    //          ZOOM_SLIDER_MAX = static_cast<int>(ZOOM_IN_MAX_STEPS);
    //m_zoom_slider = new CUISlider(m_turn_update->UpperLeft().x, m_turn_update->LowerRight().y + GG::Y(LAYOUT_MARGIN),
    //                              GG::X(ClientUI::ScrollWidth()), ZOOM_SLIDER_HEIGHT,
    //                              ZOOM_SLIDER_MIN, ZOOM_SLIDER_MAX, GG::VERTICAL);
    ////m_zoom_slider->SizeSlider(ZOOM_SLIDER_MIN, ZOOM_SLIDER_MAX);
    //m_toolbar->AttachChild(m_zoom_slider);
    //GG::Connect(m_zoom_slider->SlidSignal, &MapWnd::ZoomSlid, this);

    // Subscreen / Menu buttons
    GG::X button_width = font->TextExtent(UserString("MAP_BTN_MENU")).x + BUTTON_TOTAL_MARGIN;
    m_btn_menu = new CUIButton(m_toolbar->LowerRight().x-button_width, GG::Y0, button_width, UserString("MAP_BTN_MENU") );
    m_toolbar->AttachChild(m_btn_menu);
    GG::Connect(m_btn_menu->ClickedSignal, BoolToVoidAdapter(boost::bind(&MapWnd::ShowMenu, this)));

    button_width = font->TextExtent(UserString("MAP_BTN_DESIGN")).x + BUTTON_TOTAL_MARGIN;
    m_btn_design = new CUIButton(m_btn_menu->UpperLeft().x-LAYOUT_MARGIN-button_width, GG::Y0, button_width, UserString("MAP_BTN_DESIGN") );
    m_toolbar->AttachChild(m_btn_design);
    GG::Connect(m_btn_design->ClickedSignal, BoolToVoidAdapter(boost::bind(&MapWnd::ToggleDesign, this)));

    button_width = font->TextExtent(UserString("MAP_BTN_PRODUCTION")).x + BUTTON_TOTAL_MARGIN;
    m_btn_production = new CUIButton(m_btn_design->UpperLeft().x-LAYOUT_MARGIN-button_width, GG::Y0, button_width, UserString("MAP_BTN_PRODUCTION") );
    m_toolbar->AttachChild(m_btn_production);
    GG::Connect(m_btn_production->ClickedSignal, BoolToVoidAdapter(boost::bind(&MapWnd::ToggleProduction, this)));

    button_width = font->TextExtent(UserString("MAP_BTN_RESEARCH")).x + BUTTON_TOTAL_MARGIN;
    m_btn_research = new CUIButton(m_btn_production->UpperLeft().x-LAYOUT_MARGIN-button_width, GG::Y0, button_width, UserString("MAP_BTN_RESEARCH") );
    m_toolbar->AttachChild(m_btn_research);
    GG::Connect(m_btn_research->ClickedSignal, BoolToVoidAdapter(boost::bind(&MapWnd::ToggleResearch, this)));

    button_width = font->TextExtent(UserString("MAP_BTN_SITREP")).x + BUTTON_TOTAL_MARGIN;
    m_btn_siterep = new CUIButton(m_btn_research->UpperLeft().x-LAYOUT_MARGIN-button_width, GG::Y0, button_width, UserString("MAP_BTN_SITREP") );
    m_toolbar->AttachChild(m_btn_siterep);
    GG::Connect(m_btn_siterep->ClickedSignal, BoolToVoidAdapter(boost::bind(&MapWnd::ToggleSitRep, this)));

    // resources
    const GG::X ICON_DUAL_WIDTH(100);
    const GG::X ICON_WIDTH(ICON_DUAL_WIDTH - 30);
    m_population = new StatisticIcon(m_btn_siterep->UpperLeft().x-LAYOUT_MARGIN-ICON_DUAL_WIDTH,GG::Y(LAYOUT_MARGIN),ICON_DUAL_WIDTH,m_turn_update->Height(),
                                     ClientUI::MeterIcon(METER_POPULATION),
                                     0,0,3,3,true,true,false,true);
    m_toolbar->AttachChild(m_population);

    m_industry = new StatisticIcon(m_population->UpperLeft().x-LAYOUT_MARGIN-ICON_WIDTH,GG::Y(LAYOUT_MARGIN),ICON_WIDTH,m_turn_update->Height(),
                                   ClientUI::MeterIcon(METER_INDUSTRY),
                                   0,3,true,false);
    m_toolbar->AttachChild(m_industry);

    m_research = new StatisticIcon(m_industry->UpperLeft().x-LAYOUT_MARGIN-ICON_WIDTH,GG::Y(LAYOUT_MARGIN),ICON_WIDTH,m_turn_update->Height(),
                                   ClientUI::MeterIcon(METER_RESEARCH),
                                   0,3,true,false);
    m_toolbar->AttachChild(m_research);

    m_trade = new StatisticIcon(m_research->UpperLeft().x-LAYOUT_MARGIN-ICON_DUAL_WIDTH,GG::Y(LAYOUT_MARGIN),ICON_DUAL_WIDTH,m_turn_update->Height(),
                                ClientUI::MeterIcon(METER_TRADE),
                                0,0,3,3,true,true,false,true);
    m_toolbar->AttachChild(m_trade);

    m_mineral = new StatisticIcon(m_trade->UpperLeft().x-LAYOUT_MARGIN-ICON_DUAL_WIDTH,GG::Y(LAYOUT_MARGIN),ICON_DUAL_WIDTH,m_turn_update->Height(),
                                  ClientUI::MeterIcon(METER_MINING),
                                  0,0,3,3,true,true,false,true);
    m_toolbar->AttachChild(m_mineral);

    m_food = new StatisticIcon(m_mineral->UpperLeft().x-LAYOUT_MARGIN-ICON_DUAL_WIDTH,GG::Y(LAYOUT_MARGIN),ICON_DUAL_WIDTH,m_turn_update->Height(),
                               ClientUI::MeterIcon(METER_FARMING),
                               0,0,3,3,true,true,false,true);
    m_toolbar->AttachChild(m_food);

    m_menu_showing = false;

    //clear background images
    m_backgrounds.clear();
    m_bg_scroll_rate.clear();

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
    return ZoomScaleFactor(m_zoom_steps_in);
}

GG::Pt MapWnd::ScreenCoordsFromUniversePosition(double universe_x, double universe_y) const
{
    GG::Pt cl_ul = ClientUpperLeft();
    GG::X x((universe_x * ZoomFactor()) + cl_ul.x);
    GG::Y y((universe_y * ZoomFactor()) + cl_ul.y);
    return GG::Pt(x, y);
}

std::pair<double, double> MapWnd::UniversePositionFromScreenCoords(GG::Pt screen_coords) const
{
    GG::Pt cl_ul = ClientUpperLeft();
    double x = Value((screen_coords - cl_ul).x / ZoomFactor());
    double y = Value((screen_coords - cl_ul).y / ZoomFactor());
    return std::pair<double, double>(x, y);
}

SidePanel* MapWnd::GetSidePanel() const
{
    return m_side_panel;
}

void MapWnd::GetSaveGameUIData(SaveGameUIData& data) const
{
    data.map_left = Value(UpperLeft().x);
    data.map_top = Value(UpperLeft().y);
    data.map_zoom_steps_in = m_zoom_steps_in;
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

    GG::Pt origin_offset = UpperLeft() + GG::Pt(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight());
    glPushMatrix();
    glLoadIdentity();
    glScalef(ZoomFactor(), ZoomFactor(), 1.0);
    glTranslatef(Value(origin_offset.x / ZoomFactor()), Value(origin_offset.y / ZoomFactor()), 0.0);

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
    m_sitrep_panel->OffsetMove(-final_move);

    MoveTo(move_to_pt - GG::Pt(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight()));
    m_dragged = true;
}

void MapWnd::LButtonUp(const GG::Pt &pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_drag_offset = GG::Pt(-GG::X1, -GG::Y1);
    m_dragged = false;
}

void MapWnd::LClick(const GG::Pt &pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_drag_offset = GG::Pt(-GG::X1, -GG::Y1);
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
    Logger().debugStream() << "Initializing turn " << turn_number;

    SetAccelerators();

    Universe& universe = GetUniverse();

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


    // set up system icons, starlanes, galaxy gas rendering
    InitTurnRendering();



    // disconnect old moving fleet statechangedsignal connections
    for (std::map<int, boost::signals::connection>::iterator it = m_fleet_state_change_signals.begin(); it != m_fleet_state_change_signals.end(); ++it)
        it->second.disconnect();
    m_fleet_state_change_signals.clear();

    // connect fleet change signals to update moving fleet movement lines, so that ordering
    // moving fleets to move updates their displayed path
    Universe::ObjectVec fleets = universe.FindObjects(MovingFleetVisitor());
    for (Universe::ObjectVec::const_iterator it = fleets.begin(); it != fleets.end(); ++it) {
        const Fleet* moving_fleet = universe_object_cast<const Fleet*>(*it);
        if (!moving_fleet) {
            Logger().errorStream() << "MapWnd::InitTurn couldn't cast a (supposed) moving fleet pointer to a Fleet*";
            continue;
        }
        m_fleet_state_change_signals[moving_fleet->ID()] = GG::Connect(moving_fleet->StateChangedSignal, boost::bind(SetFleetMovementLineFunc, this, moving_fleet));
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
    if (empire && empire->NumSitRepEntries())
        ShowSitRep();


    GetChatWnd()->HideEdit();
    EnableAlphaNumAccels();


    // show or hide system names, depending on zoom.  replicates code in MapWnd::Zoom
    if (ZoomFactor() * ClientUI::Pts() < MIN_SYSTEM_NAME_SIZE)
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

    for (EmpireManager::iterator it = manager.begin(); it != manager.end(); ++it)
        it->second->UpdateResourcePools();

    m_research_wnd->Update();
    m_production_wnd->Update();
}

void MapWnd::InitTurnRendering()
{
    Logger().debugStream() << "MapWnd::InitTurnRendering";
    CheckGLVersion();
    Universe& universe = GetUniverse();
    EmpireManager& manager = HumanClientApp::GetApp()->Empires();


    // temp storage
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
    const GG::Clr UNOWNED_LANE_COLOUR = GetOptionsDB().Get<StreamableColor>("UI.unowned-starlane-colour").ToClr();


    // adjust size of map window for universe and application size
    Resize(GG::Pt(static_cast<GG::X>(Universe::UniverseWidth() * ZOOM_MAX + GG::GUI::GetGUI()->AppWidth() * 1.5),
                  static_cast<GG::Y>(Universe::UniverseWidth() * ZOOM_MAX + GG::GUI::GetGUI()->AppHeight() * 1.5)));


    std::vector<System*> systems = universe.FindObjects<System>();


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


    // remove any existing fleet movement lines or projected movement lines.  this gets cleared
    // here instead of with the movement line stuff because that would clear some movement lines
    // that come from the SystemIcons
    m_fleet_lines.clear();
    ClearProjectedFleetMovementLines();


    // create fleet buttons for moving fleets
    RefreshFleetButtons();


    // remove old system icons
    for (std::map<int, SystemIcon*>::iterator it = m_system_icons.begin(); it != m_system_icons.end(); ++it)
        DeleteChild(it->second);
    m_system_icons.clear();


    // create system icons
    for (unsigned int i = 0; i < systems.size(); ++i) {
        // create new system icon
        const System* start_system = systems[i];
        SystemIcon* icon = new SystemIcon(this, GG::X0, GG::Y0, GG::X(10), start_system->ID());
        m_system_icons[start_system->ID()] = icon;
        icon->InstallEventFilter(this);
        AttachChild(icon);


        // connect UI response signals.  TODO: Make these configurable in GUI?
        GG::Connect(icon->LeftClickedSignal,        &MapWnd::SystemLeftClicked,         this);
        GG::Connect(icon->RightClickedSignal,       &MapWnd::SystemRightClicked,        this);
        GG::Connect(icon->LeftDoubleClickedSignal,  &MapWnd::SystemDoubleClicked,       this);
        GG::Connect(icon->MouseEnteringSignal,      &MapWnd::MouseEnteringSystem,       this);
        GG::Connect(icon->MouseLeavingSignal,       &MapWnd::MouseLeavingSystem,        this);
        GG::Connect(icon->FleetButtonClickedSignal, &MapWnd::FleetButtonLeftClicked,    this);
    }


    // position system icons
    DoSystemIconsLayout();


    // Generate texture coordinates to be used for subsequent vertex buffer creation.
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


    // create various buffers used for map contents rendering
    for (std::map<int, SystemIcon*>::const_iterator it = m_system_icons.begin(); it != m_system_icons.end(); ++it) {
        const SystemIcon* icon = it->second;
        const System& system = icon->GetSystem();

        // add disc and halo textures for system icon
        {
            // See note above texture coords for why we're making coordinate sets that are 2x too big.
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


        // add gaseous substance around system
        if (boost::shared_ptr<GG::Texture> gaseous_texture = ClientUI::GetClientUI()->GetModuloTexture(ClientUI::ArtDir() / "galaxy_decoration", "gaseous", system.ID())) {
            glBindTexture(GL_TEXTURE_2D, gaseous_texture->OpenGLId());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            const double GAS_SIZE = ClientUI::SystemIconSize() * 12.0;
            const double ROTATION = system.ID() * 27.0; // arbitrary rotation in radians ("27.0" is just a number that produces pleasing results)
            const double COS_THETA = std::cos(ROTATION);
            const double SIN_THETA = std::sin(ROTATION);

            // Components of corner points of a quad
            const double X1 =  1.0, Y1 =  1.0;  // upper right corner (X1, Y1)
            const double X2 = -1.0, Y2 =  1.0;  // upper left corner  (X2, Y2)
            const double X3 = -1.0, Y3 = -1.0;  // lower left corner  (X3, Y3)
            const double X4 =  1.0, Y4 = -1.0;  // lower right corner (X4, Y4)

            // Calculate rotated corner point components after CCW ROTATION radians around origin.
            const double X1r =  COS_THETA*X1 + SIN_THETA*Y1;
            const double Y1r = -SIN_THETA*X1 + COS_THETA*Y1;
            const double X2r =  COS_THETA*X2 + SIN_THETA*Y2;
            const double Y2r = -SIN_THETA*X2 + COS_THETA*Y2;
            const double X3r =  COS_THETA*X3 + SIN_THETA*Y3;
            const double Y3r = -SIN_THETA*X3 + COS_THETA*Y3;
            const double X4r =  COS_THETA*X4 + SIN_THETA*Y4;
            const double Y4r = -SIN_THETA*X4 + COS_THETA*Y4;

            // Multiply all coords by GAS_SIZE to get relative scaled rotated quad corner components
            // See note above texture coords for why we're making coordinate sets that are 2x too big.

            // add to system position to get translated scaled rotated quad corner
            const double GAS_X1 = system.X() + (X1r * GAS_SIZE);
            const double GAS_Y1 = system.Y() + (Y1r * GAS_SIZE);
            const double GAS_X2 = system.X() + (X2r * GAS_SIZE);
            const double GAS_Y2 = system.Y() + (Y2r * GAS_SIZE);
            const double GAS_X3 = system.X() + (X3r * GAS_SIZE);
            const double GAS_Y3 = system.Y() + (Y3r * GAS_SIZE);
            const double GAS_X4 = system.X() + (X4r * GAS_SIZE);
            const double GAS_Y4 = system.Y() + (Y4r * GAS_SIZE);

            std::vector<float>& gas_vertices = raw_galaxy_gas_quad_vertices[gaseous_texture];
            // rotated upper right
            gas_vertices.push_back(GAS_X1);
            gas_vertices.push_back(GAS_Y1);
            // rotated upper left
            gas_vertices.push_back(GAS_X2);
            gas_vertices.push_back(GAS_Y2);
            // rotated lower left
            gas_vertices.push_back(GAS_X3);
            gas_vertices.push_back(GAS_Y3);
            // rotated lower right
            gas_vertices.push_back(GAS_X4);
            gas_vertices.push_back(GAS_Y4);
        }


        // add system's starlanes
        for (System::const_lane_iterator lane_it = system.begin_lanes(); lane_it != system.end_lanes(); ++lane_it) {
            bool lane_is_wormhole = lane_it->second;
            if (lane_is_wormhole) continue; // at present, not rendering wormholes

            const System* start_system = &system;
            const System* dest_system = universe.Object<System>(lane_it->first);

            // render starlane between start and dest systems?

            // check that this lane isn't already going to be rendered.  skip it if it is.
            if (rendered_starlanes.find(UnorderedIntPair(start_system->ID(), dest_system->ID())) == rendered_starlanes.end()) {
                //Logger().debugStream() << " ... lane not found.";
                rendered_starlanes.insert(UnorderedIntPair(start_system->ID(), dest_system->ID()));

                raw_starlane_vertices.push_back(start_system->X());
                raw_starlane_vertices.push_back(start_system->Y());
                raw_starlane_vertices.push_back(dest_system->X());
                raw_starlane_vertices.push_back(dest_system->Y());

                // determine colour(s) for lane based on which empire(s) can transfer resources along the lane.
                // todo: multiple rendered lanes (one for each empire) when multiple empires use the same lane.
                GG::Clr lane_colour = UNOWNED_LANE_COLOUR;    // default colour if no empires transfer resources along starlane

                for (EmpireManager::iterator empire_it = manager.begin(); empire_it != manager.end(); ++empire_it) {
                    Empire* empire = empire_it->second;
                    const std::set<std::pair<int, int> >& resource_supply_lanes = empire->ResourceSupplyStarlaneTraversals();

                    std::pair<int, int> lane_forward = std::make_pair(start_system->ID(), dest_system->ID());
                    std::pair<int, int> lane_backward = std::make_pair(dest_system->ID(), start_system->ID());

                    // see if this lane exists in this empire's supply propegation lanes set.  either direction accepted.
                    if (resource_supply_lanes.find(lane_forward) != resource_supply_lanes.end() || resource_supply_lanes.find(lane_backward) != resource_supply_lanes.end()) {
                        lane_colour = empire->Color();
                        //Logger().debugStream() << "selected colour of empire " << empire->Name() << " for this full lane";
                        break;
                    }
                }

                raw_starlane_colors.push_back(lane_colour.r);
                raw_starlane_colors.push_back(lane_colour.g);
                raw_starlane_colors.push_back(lane_colour.b);
                raw_starlane_colors.push_back(lane_colour.a);
                raw_starlane_colors.push_back(lane_colour.r);
                raw_starlane_colors.push_back(lane_colour.g);
                raw_starlane_colors.push_back(lane_colour.b);
                raw_starlane_colors.push_back(lane_colour.a);

                //Logger().debugStream() << "adding full lane from " << start_system->Name() << " to " << dest_system->Name();
            }


            // render half-starlane from the current start_system to the current dest_system?

            // check that this lane isn't already going to be rendered.  skip it if it is.
            if (rendered_half_starlanes.find(std::make_pair(start_system->ID(), dest_system->ID())) == rendered_half_starlanes.end()) {
                // NOTE: this will never find a preexisting half lane
                //Logger().debugStream() << "half lane not found... considering possible half lanes to add";

                // scan through possible empires to have a half-lane here and add a half-lane if one is found

                for (EmpireManager::iterator empire_it = manager.begin(); empire_it != manager.end(); ++empire_it) {
                    Empire* empire = empire_it->second;
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

                        //Logger().debugStream() << "Adding half lane between " << start_system->Name() << " to " << dest_system->Name() << " with colour of empire " << empire->Name();

                        break;
                    }
                }
            }
        }
    }


    // create animated lines indicating fleet supply flow
    for (EmpireManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        Empire* empire = it->second;
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



    // clear out all the old buffers
    for (std::map<boost::shared_ptr<GG::Texture>, GLBuffer>::const_iterator it = m_star_core_quad_vertices.begin();
         it != m_star_core_quad_vertices.end();
         ++it) {
        glDeleteBuffers(1, &it->second.m_name);
    }
    m_star_core_quad_vertices.clear();
    for (std::map<boost::shared_ptr<GG::Texture>, GLBuffer>::const_iterator it =
             m_star_halo_quad_vertices.begin();
         it != m_star_halo_quad_vertices.end();
         ++it) {
        glDeleteBuffers(1, &it->second.m_name);
    }
    m_star_halo_quad_vertices.clear();
    for (std::map<boost::shared_ptr<GG::Texture>, GLBuffer>::const_iterator it =
             m_galaxy_gas_quad_vertices.begin();
         it != m_galaxy_gas_quad_vertices.end();
         ++it) {
        glDeleteBuffers(1, &it->second.m_name);
    }
    m_galaxy_gas_quad_vertices.clear();

    if (m_star_texture_coords.m_name) {
        glDeleteBuffers(1, &m_star_texture_coords.m_name);
        m_star_texture_coords.m_name = 0;
    }

    if (m_starlane_vertices.m_name) {
        glDeleteBuffers(1, &m_starlane_vertices.m_name);
        m_starlane_vertices.m_name = 0;
    }

    if (m_starlane_colors.m_name) {
        glDeleteBuffers(1, &m_starlane_colors.m_name);
        m_starlane_colors.m_name = 0;
    }

    if (m_starlane_fleet_supply_vertices.m_name) {
        glDeleteBuffers(1, &m_starlane_fleet_supply_vertices.m_name);
        m_starlane_fleet_supply_vertices.m_name = 0;
    }

    if (m_starlane_fleet_supply_colors.m_name) {
        glDeleteBuffers(1, &m_starlane_fleet_supply_colors.m_name);
        m_starlane_fleet_supply_colors.m_name = 0;
    }


    Logger().debugStream() << "Turn initialization new graphic buffer creation";
    // create new buffers
    for (std::map<boost::shared_ptr<GG::Texture>, std::vector<float> >::const_iterator it =
             raw_star_core_quad_vertices.begin();
         it != raw_star_core_quad_vertices.end();
         ++it) {
        GLuint& name = m_star_core_quad_vertices[it->first].m_name;
        glGenBuffers(1, &name);
        glBindBuffer(GL_ARRAY_BUFFER, name);
        glBufferData(GL_ARRAY_BUFFER,
                     it->second.size() * sizeof(float),
                     &it->second[0],
                     GL_STATIC_DRAW);
        m_star_core_quad_vertices[it->first].m_size = it->second.size() / 2;
    }
    for (std::map<boost::shared_ptr<GG::Texture>, std::vector<float> >::const_iterator it =
             raw_star_halo_quad_vertices.begin();
         it != raw_star_halo_quad_vertices.end();
         ++it) {
        GLuint& name = m_star_halo_quad_vertices[it->first].m_name;
        glGenBuffers(1, &name);
        glBindBuffer(GL_ARRAY_BUFFER, name);
        glBufferData(GL_ARRAY_BUFFER,
                     it->second.size() * sizeof(float),
                     &it->second[0],
                     GL_STATIC_DRAW);
        m_star_halo_quad_vertices[it->first].m_size = it->second.size() / 2;
    }
    for (std::map<boost::shared_ptr<GG::Texture>, std::vector<float> >::const_iterator it =
             raw_galaxy_gas_quad_vertices.begin();
         it != raw_galaxy_gas_quad_vertices.end();
         ++it) {
        GLuint& name = m_galaxy_gas_quad_vertices[it->first].m_name;
        glGenBuffers(1, &name);
        glBindBuffer(GL_ARRAY_BUFFER, name);
        glBufferData(GL_ARRAY_BUFFER,
                     it->second.size() * sizeof(float),
                     &it->second[0],
                     GL_STATIC_DRAW);
        m_galaxy_gas_quad_vertices[it->first].m_size = it->second.size() / 2;
    }

    if (!raw_star_texture_coords.empty()) {
        glGenBuffers(1, &m_star_texture_coords.m_name);
        glBindBuffer(GL_ARRAY_BUFFER, m_star_texture_coords.m_name);
        glBufferData(GL_ARRAY_BUFFER,
                     raw_star_texture_coords.size() * sizeof(float),
                     &raw_star_texture_coords[0],
                     GL_STATIC_DRAW);
        m_star_texture_coords.m_size = raw_star_texture_coords.size() / 2;
    }
    if (!raw_starlane_vertices.empty()) {
        glGenBuffers(1, &m_starlane_vertices.m_name);
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_vertices.m_name);
        glBufferData(GL_ARRAY_BUFFER,
                     raw_starlane_vertices.size() * sizeof(float),
                     &raw_starlane_vertices[0],
                     GL_STATIC_DRAW);
        m_starlane_vertices.m_size = raw_starlane_vertices.size() / 2;
    }
    if (!raw_starlane_colors.empty()) {
        glGenBuffers(1, &m_starlane_colors.m_name);
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_colors.m_name);
        glBufferData(GL_ARRAY_BUFFER,
                     raw_starlane_colors.size() * sizeof(unsigned char),
                     &raw_starlane_colors[0],
                     GL_STATIC_DRAW);
        m_starlane_colors.m_size = raw_starlane_colors.size() / 4;
    }
    if (!raw_starlane_supply_vertices.empty()) {
        glGenBuffers(1, &m_starlane_fleet_supply_vertices.m_name);
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_fleet_supply_vertices.m_name);
        glBufferData(GL_ARRAY_BUFFER,
                     raw_starlane_supply_vertices.size() * sizeof(float),
                     &raw_starlane_supply_vertices[0],
                     GL_STATIC_DRAW);
        m_starlane_fleet_supply_vertices.m_size = raw_starlane_supply_vertices.size() / 2;
    }
    if (!raw_starlane_supply_colors.empty()) {
        glGenBuffers(1, &m_starlane_fleet_supply_colors.m_name);
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_fleet_supply_colors.m_name);
        glBufferData(GL_ARRAY_BUFFER,
                     raw_starlane_supply_colors.size() * sizeof(unsigned char),
                     &raw_starlane_supply_colors[0],
                     GL_STATIC_DRAW);
        m_starlane_fleet_supply_colors.m_size = raw_starlane_supply_colors.size() / 4;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MapWnd::RestoreFromSaveData(const SaveGameUIData& data)
{
    m_zoom_steps_in = data.map_zoom_steps_in;

    GG::Pt ul = UpperLeft();
    GG::Pt map_ul = GG::Pt(GG::X(data.map_left), GG::Y(data.map_top));
    GG::Pt map_move = map_ul - ul;
    OffsetMove(map_move);
    m_side_panel->OffsetMove(-map_move);
    m_sitrep_panel->OffsetMove(-map_move);

    // this correction ensures that zooming in doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ul;
    m_side_panel->OffsetMove(-final_move);
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

void MapWnd::CenterOnMapCoord(double x, double y)
{
    GG::Pt ul = ClientUpperLeft();
    GG::X_d current_x = (GG::GUI::GetGUI()->AppWidth() / 2 - ul.x) / ZoomFactor();
    GG::Y_d current_y = (GG::GUI::GetGUI()->AppHeight() / 2 - ul.y) / ZoomFactor();
    GG::Pt map_move = GG::Pt(static_cast<GG::X>((current_x - x) * ZoomFactor()), 
                             static_cast<GG::Y>((current_y - y) * ZoomFactor()));
    OffsetMove(map_move);
    m_side_panel->OffsetMove(-map_move);
    m_sitrep_panel->OffsetMove(-map_move);

    // this correction ensures that the centering doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ul;
    m_side_panel->OffsetMove(-final_move);
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

        GG::Pt icon_ul(GG::X(static_cast<int>(system.X()*ZoomFactor() - SYSTEM_ICON_SIZE / 2.0)),
                       GG::Y(static_cast<int>(system.Y()*ZoomFactor() - SYSTEM_ICON_SIZE / 2.0)));
        it->second->SizeMove(icon_ul, icon_ul + GG::Pt(GG::X(SYSTEM_ICON_SIZE), GG::Y(SYSTEM_ICON_SIZE)));
    }
}

void MapWnd::DoFleetButtonsLayout()
{
    // reposition unattached (to system icons) fleet icons
    for (std::vector<FleetButton*>::iterator it = m_moving_fleet_buttons.begin(); it != m_moving_fleet_buttons.end(); ++it) {
        FleetButton* fb = *it;

        const GG::Pt FLEET_BUTTON_SIZE = fb->Size();
        const Fleet* fleet = *(fb->Fleets().begin());

        GG::Pt button_ul(fleet->X()*ZoomFactor() - FLEET_BUTTON_SIZE.x / 2.0,
                         fleet->Y()*ZoomFactor() - FLEET_BUTTON_SIZE.y / 2.0);

        fb->MoveTo(button_ul);
    }
}

void MapWnd::RefreshFleetButtons()
{
    // determine fleets that need buttons, sorted by position, so that all fleets at the same location can
    // be grouped together to be passed to the button
    Universe::ObjectVec fleets = GetUniverse().FindObjects(MovingFleetVisitor());
    typedef std::multimap<std::pair<double, double>, UniverseObject*> SortedFleetMap;
    SortedFleetMap position_sorted_fleets;
    for (unsigned int i = 0; i < fleets.size(); ++i) {
        position_sorted_fleets.insert(std::make_pair(std::make_pair(fleets[i]->X(), fleets[i]->Y()), fleets[i]));
    }


    // clear old fleet buttons
    for (std::vector<FleetButton*>::iterator it = m_moving_fleet_buttons.begin(); it != m_moving_fleet_buttons.end(); ++it)
        delete *it;
    m_moving_fleet_buttons.clear();



    // create new fleet buttons for fleets at each unique location
    const FleetButton::SizeType FLEETBUTTON_SIZE = FleetButtonSizeType();
    SortedFleetMap::iterator it = position_sorted_fleets.begin();
    SortedFleetMap::iterator end_it = position_sorted_fleets.end();
    while (it != end_it) {
        // get end of range of fleets at this location
        SortedFleetMap::iterator local_end_it = position_sorted_fleets.upper_bound(it->first);

        // put into a vector to be passed to the fleetbutton
        std::vector<int> fleet_ids;
        for (; it != local_end_it; ++it)
            fleet_ids.push_back(it->second->ID());

        // create new fleetbutton for this cluster of fleets
        FleetButton* fb = new FleetButton(fleet_ids, FLEETBUTTON_SIZE);
        m_moving_fleet_buttons.push_back(fb);
        AttachChild(fb);
        GG::Connect(fb->ClickedSignal, FleetButtonClickedFunctor(*fb, *this));
    }

    // position fleetbuttons
    DoFleetButtonsLayout();

    // create movement lines (after positioning buttons, so lines will originate from button location)
    for (std::vector<FleetButton*>::iterator it = m_moving_fleet_buttons.begin(); it != m_moving_fleet_buttons.end(); ++it)
        SetFleetMovementLine(*it);
}

int MapWnd::SystemIconSize() const
{
    return static_cast<int>(ClientUI::SystemIconSize() * ZoomFactor());
}

double MapWnd::SystemHaloScaleFactor() const
{
    return 1.0 + log10(ZoomFactor());
}

FleetButton::SizeType MapWnd::FleetButtonSizeType() const
{
    // no FLEET_BUTTON_LARGE as these icons are too big for the map.  (they can be used in the FleetWnd, however)
    if      (ZoomFactor() > ClientUI::MediumFleetButtonZoomThreshold())
        return FleetButton::FLEET_BUTTON_MEDIUM;

    else if (ZoomFactor() > ClientUI::SmallFleetButtonZoomThreshold())
        return FleetButton::FLEET_BUTTON_SMALL;

    else if (ZoomFactor() > ClientUI::TinyFleetButtonZoomThreshold())
        return FleetButton::FLEET_BUTTON_TINY;

    else
        return FleetButton::FLEET_BUTTON_NONE;
}

void MapWnd::Zoom(int delta)
{
    if (delta == 0)
        return;

    // increment zoom steps in by delta steps
    double new_zoom_steps_in = m_zoom_steps_in + static_cast<double>(delta);
    SetZoom(new_zoom_steps_in);
}

void MapWnd::SetZoom(double steps_in)
{
    // impose range limits on zoom steps
    double new_steps_in = std::max(std::min(steps_in, ZOOM_IN_MAX_STEPS), ZOOM_IN_MIN_STEPS);

    // abort if no change
    if (new_steps_in == m_zoom_steps_in)
        return;


    // save position offsets and old zoom factors
    GG::Pt                      ul =                    ClientUpperLeft();
    const GG::X_d               center_x =              GG::GUI::GetGUI()->AppWidth() / 2.0;
    const GG::Y_d               center_y =              GG::GUI::GetGUI()->AppHeight() / 2.0;
    GG::X_d                     ul_offset_x =           ul.x - center_x;
    GG::Y_d                     ul_offset_y =           ul.y - center_y;
    const double                OLD_ZOOM =              ZoomFactor();
    const FleetButton::SizeType OLD_FLEETBUTTON_SIZE =  FleetButtonSizeType();


    // set new zoom level
    m_zoom_steps_in = new_steps_in;


    // correct map offsets for zoom changes
    ul_offset_x *= (ZoomFactor() / OLD_ZOOM);
    ul_offset_y *= (ZoomFactor() / OLD_ZOOM);


    // show / hide system names after zooming
    if (ZoomFactor() < ClientUI::TinyFleetButtonZoomThreshold())
        HideSystemNames();
    else
        ShowSystemNames();


    DoSystemIconsLayout();


    // if fleet buttons need to change size, need to fully refresh them (clear and recreate).  If they are the
    // same size as before the zoom, then can just reposition them without recreating
    const FleetButton::SizeType NEW_FLEETBUTTON_SIZE = FleetButtonSizeType();
    if (OLD_FLEETBUTTON_SIZE != NEW_FLEETBUTTON_SIZE)
        RefreshFleetButtons();
    else
        DoFleetButtonsLayout();


    // translate map and UI widgets to account for the change in upper left due to zooming
    GG::Pt map_move(static_cast<GG::X>((center_x + ul_offset_x) - ul.x),
                    static_cast<GG::Y>((center_y + ul_offset_y) - ul.y));
    OffsetMove(map_move);
    m_side_panel->OffsetMove(-map_move);
    m_sitrep_panel->OffsetMove(-map_move);

    // this correction ensures that zooming in doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    GG::Pt final_move = move_to_pt - ul;
    m_side_panel->OffsetMove(-final_move);
    m_sitrep_panel->OffsetMove(-final_move);

    MoveTo(move_to_pt - GG::Pt(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight()));
}

void MapWnd::ZoomSlid(int pos, int low, int high)
{
    SetZoom(static_cast<double>(pos));
}

void MapWnd::RenderStarfields()
{
    if (!GetOptionsDB().Get<bool>("UI.galaxy-starfields"))
        return;

    glColor3d(1.0, 1.0, 1.0);

    GG::Pt origin_offset =
        UpperLeft() + GG::Pt(GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight());
    glMatrixMode(GL_TEXTURE);

    for (unsigned int i = 0; i < m_backgrounds.size(); ++i) {
        float texture_coords_per_pixel_x = 1.0 / Value(m_backgrounds[i]->Width());
        float texture_coords_per_pixel_y = 1.0 / Value(m_backgrounds[i]->Height());
        glScalef(Value(texture_coords_per_pixel_x * Width()),
                 Value(texture_coords_per_pixel_y * Height()),
                 1.0);
        glTranslatef(Value(-texture_coords_per_pixel_x * origin_offset.x / 16.0 * m_bg_scroll_rate[i]),
                     Value(-texture_coords_per_pixel_y * origin_offset.y / 16.0 * m_bg_scroll_rate[i]),
                     0.0);
        glBindTexture(GL_TEXTURE_2D, m_backgrounds[i]->OpenGLId());
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex2i(0, 0);
        glTexCoord2f(0.0, 1.0);
        glVertex(GG::X0, Height());
        glTexCoord2f(1.0, 1.0);
        glVertex(Width(), Height());
        glTexCoord2f(1.0, 0.0);
        glVertex(Width(), GG::Y0);
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
    //        GG::Pt(static_cast<int>((m_nebula_centers[i].x - nebula_width / 2.0) * ZoomFactor()),
    //               static_cast<int>((m_nebula_centers[i].y - nebula_height / 2.0) * ZoomFactor()));
    //    m_nebulae[i]->OrthoBlit(ul, 
    //                            ul + GG::Pt(static_cast<int>(nebula_width * ZoomFactor()), 
    //                                        static_cast<int>(nebula_height * ZoomFactor())));
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
        glBindBuffer(GL_ARRAY_BUFFER, it->second.m_name);
        glVertexPointer(2, GL_FLOAT, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, m_star_texture_coords.m_name);
        glTexCoordPointer(2, GL_FLOAT, 0, 0);
        glDrawArrays(GL_QUADS, 0, it->second.m_size);
    }
}

void MapWnd::RenderSystems()
{
    const double HALO_SCALE_FACTOR = SystemHaloScaleFactor();

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
                glBindBuffer(GL_ARRAY_BUFFER, it->second.m_name);
                glVertexPointer(2, GL_FLOAT, 0, 0);
                glBindBuffer(GL_ARRAY_BUFFER, m_star_texture_coords.m_name);
                glTexCoordPointer(2, GL_FLOAT, 0, 0);
                glDrawArrays(GL_QUADS, 0, it->second.m_size);
            }
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
        }

        if (SystemIcon::TINY_SIZE < ZoomFactor() * ClientUI::SystemIconSize()) {
            for (std::map<boost::shared_ptr<GG::Texture>, GLBuffer>::const_iterator it = m_star_core_quad_vertices.begin();
                 it != m_star_core_quad_vertices.end();
                 ++it) {
                glBindTexture(GL_TEXTURE_2D, it->first->OpenGLId());
                glBindBuffer(GL_ARRAY_BUFFER, it->second.m_name);
                glVertexPointer(2, GL_FLOAT, 0, 0);
                glBindBuffer(GL_ARRAY_BUFFER, m_star_texture_coords.m_name);
                glTexCoordPointer(2, GL_FLOAT, 0, 0);
                glDrawArrays(GL_QUADS, 0, it->second.m_size);
            }
        }


        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glPushMatrix();
        glLoadIdentity();
        for (std::map<int, SystemIcon*>::const_iterator it = m_system_icons.begin(); it != m_system_icons.end(); ++it)
            it->second->ManualRender(HALO_SCALE_FACTOR);
        glPopMatrix();
    }

    // circles around system icons
    if (GetOptionsDB().Get<bool>("UI.system-circles")) {
        glPushMatrix();
        glLoadIdentity();
        const double TWO_PI = 2.0*3.14159;
        glDisable(GL_TEXTURE_2D);
        glLineWidth(1.5);
        glColor(GetOptionsDB().Get<StreamableColor>("UI.unowned-starlane-colour").ToClr());

        for (std::map<int, SystemIcon*>::const_iterator it = m_system_icons.begin(); it != m_system_icons.end(); ++it) {
            const SystemIcon* icon = it->second;

            const int ARC_SIZE = icon->EnclosingCircleDiameter();

            GG::Pt ul = icon->UpperLeft(), lr = icon->LowerRight();
            GG::Pt size = lr - ul;
            GG::Pt half_size = GG::Pt(size.x / 2, size.y / 2);
            GG::Pt middle = ul + half_size;

            GG::Pt circle_size = GG::Pt(static_cast<GG::X>(ARC_SIZE),
                                        static_cast<GG::Y>(ARC_SIZE));
            GG::Pt circle_half_size = GG::Pt(circle_size.x / 2, circle_size.y / 2);
            GG::Pt circle_ul = middle - circle_half_size;
            GG::Pt circle_lr = circle_ul + circle_size;

            glBegin(GL_LINE_STRIP);
            CircleArc(circle_ul, circle_lr, 0.0, TWO_PI, false);
            glEnd();
        }

        glEnable(GL_TEXTURE_2D);
        glPopMatrix();
    }
}

void MapWnd::RenderStarlanes()
{
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    bool coloured = GetOptionsDB().Get<bool>("UI.resource-starlane-colouring");
    const GG::Clr UNOWNED_LANE_COLOUR = GetOptionsDB().Get<StreamableColor>("UI.unowned-starlane-colour").ToClr();


    if (m_starlane_vertices.m_name && (m_starlane_colors.m_name || !coloured)) {
        glLineStipple(1, 0xffff);   // solid line / no stipple
        glLineWidth(GetOptionsDB().Get<double>("UI.starlane-thickness"));

        if (coloured)
            glEnableClientState(GL_COLOR_ARRAY);
        else
            glColor(UNOWNED_LANE_COLOUR);

        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_vertices.m_name);
        glVertexPointer(2, GL_FLOAT, 0, 0);

        if (coloured) {
            glBindBuffer(GL_ARRAY_BUFFER, m_starlane_colors.m_name);
            glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
        }

        glDrawArrays(GL_LINES, 0, m_starlane_vertices.m_size);

        if (coloured)
            glDisableClientState(GL_COLOR_ARRAY);
    }

    if (m_starlane_fleet_supply_vertices.m_name && m_starlane_fleet_supply_colors.m_name && GetOptionsDB().Get<bool>("UI.fleet-supply-lines")) {
        // render fleet supply lines
        const GLushort PATTERN = 0x8080;    // = 1000000010000000  -> widely space small dots
        const int GLUSHORT_BIT_LENGTH = sizeof(GLushort) * 8;
        const double RATE = 0.1;            // slow crawl
        const int SHIFT = static_cast<int>(GG::GUI::GetGUI()->Ticks() * RATE / GLUSHORT_BIT_LENGTH) % GLUSHORT_BIT_LENGTH;
        const unsigned int STIPPLE = (PATTERN << SHIFT) | (PATTERN >> (GLUSHORT_BIT_LENGTH - SHIFT));
        glLineStipple(static_cast<int>(GetOptionsDB().Get<double>("UI.fleet-supply-line-width")), STIPPLE);
        glLineWidth(GetOptionsDB().Get<double>("UI.fleet-supply-line-width"));
        glEnableClientState(GL_COLOR_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_fleet_supply_vertices.m_name);
        glVertexPointer(2, GL_FLOAT, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, m_starlane_fleet_supply_colors.m_name);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
        glDrawArrays(GL_LINES, 0, m_starlane_fleet_supply_vertices.m_size);
        glDisableClientState(GL_COLOR_ARRAY);
    }

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void MapWnd::RenderFleetMovementLines()
{
    glLineWidth(GetOptionsDB().Get<double>("UI.starlane-thickness"));

    // standard movement line stipple
    const GLushort PATTERN = 0xF0F0;
    const int GLUSHORT_BIT_LENGTH = sizeof(GLushort) * 8;
    const double RATE = 0.25;
    const int SHIFT = static_cast<int>(GG::GUI::GetGUI()->Ticks() * RATE / GLUSHORT_BIT_LENGTH) % GLUSHORT_BIT_LENGTH;
    const unsigned int STIPPLE = (PATTERN << SHIFT) | (PATTERN >> (GLUSHORT_BIT_LENGTH - SHIFT));

    // render standard movement lines
    glLineStipple(static_cast<int>(GetOptionsDB().Get<double>("UI.starlane-thickness")), STIPPLE);
    for (std::map<const Fleet*, MovementLineData>::const_iterator it = m_fleet_lines.begin(); it != m_fleet_lines.end(); ++it)
        RenderMovementLine(it->second);


    // projected movement line stipple
    const double PROJECTED_PATH_RATE = 0.35;
    const int PROJECTED_PATH_SHIFT =
        static_cast<int>(GG::GUI::GetGUI()->Ticks() * PROJECTED_PATH_RATE / GLUSHORT_BIT_LENGTH) % GLUSHORT_BIT_LENGTH;
    const unsigned int PROJECTED_PATH_STIPPLE =
        (PATTERN << PROJECTED_PATH_SHIFT) | (PATTERN >> (GLUSHORT_BIT_LENGTH - PROJECTED_PATH_SHIFT));

    //// render projected move liens
    glLineStipple(static_cast<int>(GetOptionsDB().Get<double>("UI.starlane-thickness")), PROJECTED_PATH_STIPPLE);
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
    GG::X contents_width(static_cast<int>(ZoomFactor() * Universe::UniverseWidth()));
    GG::X app_width =  GG::GUI::GetGUI()->AppWidth();
    GG::Y app_height = GG::GUI::GetGUI()->AppHeight();
    GG::X map_margin_width(app_width / 2.0);

    if (app_width - map_margin_width < contents_width || Value(app_height) - map_margin_width < contents_width) {
        if (map_margin_width < move_to_pt.x)
            move_to_pt.x = map_margin_width;
        if (move_to_pt.x + contents_width < app_width - map_margin_width)
            move_to_pt.x = app_width - map_margin_width - contents_width;
        if (map_margin_width < Value(move_to_pt.y))
            move_to_pt.y = GG::Y(Value(map_margin_width));
        if (Value(move_to_pt.y) + contents_width < Value(app_height) - map_margin_width)
            move_to_pt.y = app_height - Value(map_margin_width - contents_width);
    } else {
        if (move_to_pt.x < 0)
            move_to_pt.x = GG::X0;
        if (app_width < move_to_pt.x + contents_width)
            move_to_pt.x = app_width - contents_width;
        if (move_to_pt.y < GG::Y0)
            move_to_pt.y = GG::Y0;
        if (app_height < move_to_pt.y + Value(contents_width))
            move_to_pt.y = app_height - Value(contents_width);
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
            wnd_position = GG::Pt(GG::X(5), GG::GUI::GetGUI()->AppHeight() - wnd_for_button->Height() - 5);

        wnd_for_button->MoveTo(wnd_position);

        // safety check to ensure window is on screen... may be redundant
        if (GG::GUI::GetGUI()->AppWidth() - 5 < wnd_for_button->LowerRight().x)
            wnd_for_button->OffsetMove(GG::Pt(GG::GUI::GetGUI()->AppWidth() - 5 - wnd_for_button->LowerRight().x, GG::Y0));
        if (GG::GUI::GetGUI()->AppHeight() - 5 < wnd_for_button->LowerRight().y)
            wnd_for_button->OffsetMove(GG::Pt(GG::X0, GG::GUI::GetGUI()->AppHeight() - 5 - wnd_for_button->LowerRight().y));
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
    HideResearch();
    HideProduction();
    HideDesign();
    HideSitRep();
    m_toolbar->Hide();
    m_FPS->Hide();
}

void MapWnd::Sanitize()
{
    Cleanup();

    const GG::X SIDEPANEL_WIDTH = GG::X(GetOptionsDB().Get<int>("UI.sidepanel-width"));
    const GG::X APP_WIDTH = GG::GUI::GetGUI()->AppWidth();
    const GG::Y APP_HEIGHT = GG::GUI::GetGUI()->AppHeight();

    GG::Pt sp_ul = GG::Pt(APP_WIDTH - SIDEPANEL_WIDTH, m_toolbar->LowerRight().y);
    GG::Pt sp_lr = sp_ul + GG::Pt(SIDEPANEL_WIDTH, m_side_panel->Height());

    m_side_panel->SizeMove(sp_ul, sp_lr);
    m_sitrep_panel->MoveTo(GG::Pt((APP_WIDTH - SITREP_PANEL_WIDTH) / 2, (APP_HEIGHT - SITREP_PANEL_HEIGHT) / 2));
    m_sitrep_panel->Resize(GG::Pt(SITREP_PANEL_WIDTH, SITREP_PANEL_HEIGHT));
    MoveTo(GG::Pt(-APP_WIDTH, -APP_HEIGHT));
    m_zoom_steps_in = 0.0;
    m_research_wnd->Sanitize();
    m_production_wnd->Sanitize();
    m_design_wnd->Sanitize();
    m_previously_selected_system = UniverseObject::INVALID_OBJECT_ID;
}

bool MapWnd::ReturnToMap()
{
    if (m_sitrep_panel->Visible())
        ToggleSitRep();

    if (m_research_wnd->Visible())
        ToggleResearch();

    if (m_design_wnd->Visible())
        ToggleDesign();

    if (m_production_wnd->Visible())
        ToggleProduction();

    return true;
}

bool MapWnd::OpenChatWindow()
{
    bool retval = true;
    if (GetChatWnd()->OpenForInput())
        DisableAlphaNumAccels();
    else
        retval = false;
    return retval;
}

bool MapWnd::EndTurn()
{
    Cleanup();
    HumanClientApp::GetApp()->StartTurn();
    return true;
}

void MapWnd::ShowSitRep()
{
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideResearch();
    HideProduction();
    HideDesign();

    // show the sitrep window
    AttachChild(m_sitrep_panel);
    MoveChildUp(m_sitrep_panel);
    m_sitrep_panel->Show();

    // indicate selection on button
    m_btn_siterep->MarkSelectedGray();
}

void MapWnd::HideSitRep()
{
    DetachChild(m_sitrep_panel);
    m_sitrep_panel->Hide(); // necessary so it won't be visible when next toggled
    m_btn_siterep->MarkNotSelected();
}

bool MapWnd::ToggleSitRep()
{
    if (m_sitrep_panel->Visible())
        HideSitRep();
    else
        ShowSitRep();
    return true;
}

void MapWnd::ShowResearch()
{
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideSitRep();
    HideProduction();
    HideDesign();

    // show the research window
    m_research_wnd->Show();
    GG::GUI::GetGUI()->MoveUp(m_research_wnd);

    // indicate selection on button
    m_btn_research->MarkSelectedGray();
}

void MapWnd::HideResearch()
{
    m_research_wnd->Hide();
    m_btn_research->MarkNotSelected();
    ShowAllPopups();
}

bool MapWnd::ToggleResearch()
{
    if (m_research_wnd->Visible())
        HideResearch();
    else
        ShowResearch();
    return true;
}

void MapWnd::ShowProduction()
{
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideSitRep();
    HideResearch();
    HideDesign();
    m_side_panel->Hide();
    DetachChild(m_side_panel);

    // show the production window
    m_production_wnd->Show();
    m_in_production_view_mode = true;
    HideAllPopups();
    GG::GUI::GetGUI()->MoveUp(m_production_wnd);

    // indicate selection on button
    m_btn_production->MarkSelectedGray();

    // if no system is currently shown in sidepanel, default to this empire's home system (ie. where the capitol is)
    if (m_side_panel->SystemID() == UniverseObject::INVALID_OBJECT_ID)
        if (const Empire* empire = HumanClientApp::GetApp()->Empires().Lookup(HumanClientApp::GetApp()->EmpireID()))
            if (const UniverseObject* obj = GetUniverse().Object(empire->CapitolID()))
                m_production_wnd->SelectSystem(obj->SystemID());
}

void MapWnd::HideProduction()
{
    m_production_wnd->Hide();
    m_in_production_view_mode = false;
    m_btn_production->MarkNotSelected();
    ShowAllPopups();
    //if (!m_side_panel->Visible())
    //        m_side_panel->SetSystem(m_side_panel->SystemID());
}

bool MapWnd::ToggleProduction()
{
    if (m_production_wnd->Visible())
        HideProduction();
    else
        ShowProduction();
    return true;
}

void MapWnd::ShowDesign()
{
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideSitRep();
    HideResearch();
    HideProduction();

    // show the design window
    m_design_wnd->Show();
    GG::GUI::GetGUI()->MoveUp(m_design_wnd);
    //GG::GUI::GetGUI()->SetFocusWnd(m_design_wnd);
    DisableAlphaNumAccels();
    m_design_wnd->Reset();

    // indicate selection on button
    m_btn_design->MarkSelectedGray();
}

void MapWnd::HideDesign()
{
    m_design_wnd->Hide();
    m_btn_design->MarkNotSelected();
    EnableAlphaNumAccels();
}

bool MapWnd::ToggleDesign()
{
    if (m_design_wnd->Visible())
        HideDesign();
    else
        ShowDesign();
    return true;
}

bool MapWnd::ShowMenu()
{
    if (!m_menu_showing) {
        ClearProjectedFleetMovementLines();
        m_menu_showing = true;
        m_btn_menu->MarkSelectedGray();
        InGameMenu menu;
        menu.Run();
        m_menu_showing = false;
        m_btn_menu->MarkNotSelected();
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
    Empire* empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );
    if (!empire) {
        Logger().errorStream() << "MapWnd::RefreshFoodResourceIndicator couldn't get an empire";
        m_mineral->SetValue(0.0);
        m_mineral->SetValue(0.0, 1);
        return;
    }

    const ResourcePool* pool = empire->GetResourcePool(RE_FOOD);
    if (!pool) {
        Logger().errorStream() << "MapWnd::RefreshFoodResourceIndicator couldn't get a food resourepool";
        m_food->SetValue(0.0);
        m_food->SetValue(0.0, 1);
        return;
    }

    const PopulationPool& pop_pool = empire->GetPopulationPool();

    int stockpile_system_id = pool->StockpileSystemID();

    if (stockpile_system_id == UniverseObject::INVALID_OBJECT_ID) {
        // empire has nowhere to stockpile food, so has no stockpile.  Instead of showing stockpile, show production
        m_food->SetValue(pool->TotalAvailable());   // no stockpile means available is equal to production
        m_food->SetValue(0.0, 1);                   // TODO: Make StatisticIcon able to change number of numbers shown, and remove second number here
        return;
    }

    // empire has a stockpile. Show stockpiled amount for first number

    m_food->SetValue(pool->Stockpile()); // set first value to stockpiled food

    // for second number, show predicted change in stockpile for next turn compared to this turn.


    // find total food allocated to group that has access to stockpile
    std::map<std::set<int>, double> food_sharing_groups = pool->Available();
    std::set<int> stockpile_group_systems;
    Logger().debugStream() << "trying to find stockpile system group...  stockpile system has id: " << stockpile_system_id;
    for (std::map<std::set<int>, double>::const_iterator it = food_sharing_groups.begin(); it != food_sharing_groups.end(); ++it) {
        const std::set<int>& group = it->first;                     // get group
        Logger().debugStream() << "potential group:";
        for (std::set<int>::const_iterator qit = group.begin(); qit != group.end(); ++qit)
            Logger().debugStream() << "...." << *qit;

        if (group.find(stockpile_system_id) != group.end()) {       // check for stockpile system
            stockpile_group_systems = group;
            Logger().debugStream() << "MapWnd::RefreshFoodResourceIndicator found group of systems for stockpile system.  size: " << stockpile_group_systems.size();
            break;
        }

        Logger().debugStream() << "didn't find in group... trying next.";
    }


    const std::vector<PopCenter*>& pop_centers = pop_pool.PopCenters();


    double stockpile_group_food_allocation = 0.0;


    // go through population pools, adding up food allocation of those that are in one of the systems
    // in the group of systems that can access the stockpile
    for (std::vector<PopCenter*>::const_iterator it = pop_centers.begin(); it != pop_centers.end(); ++it) {
        const PopCenter* pop = *it;
        const UniverseObject* obj = dynamic_cast<const UniverseObject*>(pop);
        if (!obj) {
            Logger().debugStream() << "MapWnd::RefreshFoodResourceIndicator couldn't cast a PopCenter* to an UniverseObject*";
            continue;
        }
        int center_system_id = obj->SystemID();

        if (stockpile_group_systems.find(center_system_id) != stockpile_group_systems.end()) {
            stockpile_group_food_allocation += pop->AllocatedFood();    // finally add allocation for this PopCenter
            Logger().debugStream() << "object " << obj->Name() << " is in stockpile system group has " << pop->AllocatedFood() << " food allocated to it";
        }
    }

    double stockpile_system_group_available = pool->GroupAvailable(stockpile_system_id);
    Logger().debugStream() << "food available in stockpile group is:  " << stockpile_system_group_available;
    Logger().debugStream() << "food allocation in stockpile group is: " << stockpile_group_food_allocation;

    double new_stockpile = stockpile_system_group_available - stockpile_group_food_allocation;
    Logger().debugStream() << "Predicted stockpile is: " << new_stockpile;

    Logger().debugStream() << "Old stockpile is " << pool->Stockpile();

    double stockpile_change = new_stockpile - pool->Stockpile();
    Logger().debugStream() << "Stockpile change is: " << stockpile_change;

    m_food->SetValue(stockpile_change, 1);
}

void MapWnd::RefreshMineralsResourceIndicator()
{
    Empire* empire = HumanClientApp::GetApp()->Empires().Lookup( HumanClientApp::GetApp()->EmpireID() );
    if (!empire) {
        Logger().errorStream() << "MapWnd::RefreshFoodResourceIndicator couldn't get an empire";
        m_mineral->SetValue(0.0);
        m_mineral->SetValue(0.0, 1);
        return;
    }

    const ResourcePool* pool = empire->GetResourcePool(RE_MINERALS);
    if (!pool) {
        Logger().errorStream() << "MapWnd::RefreshFoodResourceIndicator couldn't get a minerals resourepool";
        m_mineral->SetValue(0.0);
        m_mineral->SetValue(0.0, 1);
        return;
    }

    int stockpile_system_id = pool->StockpileSystemID();

    if (stockpile_system_id == UniverseObject::INVALID_OBJECT_ID) {
        // empire has nowhere to stockpile food, so has no stockpile.
        m_mineral->SetValue(0.0);
        m_mineral->SetValue(0.0, 1);        // TODO: Make StatisticIcon able to change number of numbers shown, and remove second number here
        return;
    }

    // empire has a stockpile. Show stockpiled amount for first number

    m_mineral->SetValue(pool->Stockpile()); // set first value to stockpiled food


    // find minerals (PP) allocated to production elements located in systems in the group of
    // resource-sharing systems that has access to stockpile
    double stockpile_group_pp_allocation = 0.0;

    // find the set of systems that contains the stopile system, from the map of PP allocated within each group
    const ProductionQueue& queue = empire->GetProductionQueue();
    std::map<std::set<int>, double> allocated_pp = queue.AllocatedPP();

    Logger().debugStream() << "trying to find stockpile system group...  stockpile system has id: " << stockpile_system_id;
    for (std::map<std::set<int>, double>::const_iterator it = allocated_pp.begin(); it != allocated_pp.end(); ++it) {
        const std::set<int>& group = it->first;                     // get group
        Logger().debugStream() << "potential group:";
        for (std::set<int>::const_iterator qit = group.begin(); qit != group.end(); ++qit)
            Logger().debugStream() << "...." << *qit;

        if (group.find(stockpile_system_id) != group.end()) {       // check for stockpile system
            stockpile_group_pp_allocation = it->second;        // record allocation for this group
            Logger().debugStream() << "MapWnd::RefreshMineralsResourceIndicator found group of systems for stockpile system.  size: " << it->first.size();
            break;
        }

        Logger().debugStream() << "didn't find in group... trying next.";
    }
    // if the stockpile system is not found in any group of systems with allocated pp, assuming this is fine and that the
    // stockpile system's group of systems didn't have any allocated pp...


    double stockpile_system_group_available = pool->GroupAvailable(stockpile_system_id);
    Logger().debugStream() << "minerals available in stockpile group is:  " << stockpile_system_group_available;
    Logger().debugStream() << "minerals allocation in stockpile group is: " << stockpile_group_pp_allocation;       // as of this writing, PP consume one mineral and one industry point, so PP allocation is equal to minerals allocation

    double new_stockpile = stockpile_system_group_available - stockpile_group_pp_allocation;
    Logger().debugStream() << "Predicted stockpile is: " << new_stockpile;

    Logger().debugStream() << "Old stockpile is " << pool->Stockpile();

    double stockpile_change = new_stockpile - pool->Stockpile();
    Logger().debugStream() << "Stockpile change is: " << stockpile_change;

    m_mineral->SetValue(stockpile_change, 1);
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

void MapWnd::ConnectKeyboardAcceleratorSignals()
{
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_ESCAPE),
                    &MapWnd::ReturnToMap, this));

    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_RETURN),
                    &MapWnd::OpenChatWindow, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_KP_ENTER),
                    &MapWnd::OpenChatWindow, this));

    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_RETURN,   GG::MOD_KEY_CTRL),
                    &MapWnd::EndTurn, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_KP_ENTER, GG::MOD_KEY_CTRL),
                    &MapWnd::EndTurn, this));

    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_F2),
                    &MapWnd::ToggleSitRep, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_F3),
                    &MapWnd::ToggleResearch, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_F4),
                    &MapWnd::ToggleProduction, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_F5),
                    &MapWnd::ToggleDesign, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_F10),
                    &MapWnd::ShowMenu, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_s),
                    &MapWnd::CloseSystemView, this));

    // Keys for zooming
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_e),
                    &MapWnd::KeyboardZoomIn, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_KP_PLUS),
                    &MapWnd::KeyboardZoomIn, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_r),
                    &MapWnd::KeyboardZoomOut, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_KP_MINUS),
                    &MapWnd::KeyboardZoomOut, this));

    // Keys for showing systems
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_d),
                    &MapWnd::ZoomToHomeSystem, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_x),
                    &MapWnd::ZoomToPrevOwnedSystem, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_c),
                    &MapWnd::ZoomToNextOwnedSystem, this));

    // Keys for showing fleets
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_f),
                    &MapWnd::ZoomToPrevIdleFleet, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_g),
                    &MapWnd::ZoomToNextIdleFleet, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_v),
                    &MapWnd::ZoomToPrevFleet, this));
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_b),
                    &MapWnd::ZoomToNextFleet, this));

#ifndef FREEORION_RELEASE
    // Previously-used but presently ignored development-only key combo for dumping
    // ValueRef, Condition, and Effect regression tests using the current Universe
    m_keyboard_accelerator_signals.insert(
        GG::Connect(GG::GUI::GetGUI()->AcceleratorSignal(GG::GGK_r, GG::MOD_KEY_CTRL),
                    &RequestRegressionTestDump));
#endif
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
    GG::GUI::GetGUI()->SetAccelerator(GG::GGK_F5);
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

    ConnectKeyboardAcceleratorSignals();
}

void MapWnd::RemoveAccelerators()
{
    GG::GUI::accel_iterator i = GG::GUI::GetGUI()->accel_begin();
    while (i != GG::GUI::GetGUI()->accel_end()) {
        GG::GUI::GetGUI()->RemoveAccelerator(i);
        i = GG::GUI::GetGUI()->accel_begin();
    }
    m_disabled_accels_list.clear();

    for (std::set<boost::signals::connection>::iterator it =
             m_keyboard_accelerator_signals.begin();
         it != m_keyboard_accelerator_signals.end();
         ++it) {
        it->disconnect();
    }
    m_keyboard_accelerator_signals.clear();
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

void MapWnd::ChatMessageSentSlot()
{
    if (!m_disabled_accels_list.empty()) {
        EnableAlphaNumAccels();
        GG::GUI::GetGUI()->SetFocusWnd(this);
    }
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

