#include "MapWnd.h"

#include "CensusBrowseWnd.h"
#include "ResourceBrowseWnd.h"
#include "ChatWnd.h"
#include "PlayerListWnd.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"
#include "CombatReport/CombatReportWnd.h"
#include "FleetButton.h"
#include "FleetWnd.h"
#include "InGameMenu.h"
#include "DesignWnd.h"
#include "ProductionWnd.h"
#include "ResearchWnd.h"
#include "EncyclopediaDetailPanel.h"
#include "ObjectListWnd.h"
#include "ModeratorActionsWnd.h"
#include "SidePanel.h"
#include "SitRepPanel.h"
#include "SystemIcon.h"
#include "FieldIcon.h"
#include "ShaderProgram.h"
#include "Hotkeys.h"
#include "Sound.h"
#include "TextBrowseWnd.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/GameRules.h"
#include "../util/OptionsDB.h"
#include "../util/Order.h"
#include "../util/Random.h"
#include "../util/ModeratorAction.h"
#include "../util/ScopedTimer.h"
#include "../universe/Enums.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/Field.h"
#include "../universe/Pathfinder.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../Empire/Empire.h"
#include "../network/Message.h"
#include "../network/ClientNetworking.h"
#include "../client/human/HumanClientApp.h"

#include <boost/timer.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/unordered_map.hpp>
#include <boost/optional/optional.hpp>
#include <boost/range/numeric.hpp>
#include <boost/range/adaptor/map.hpp>

#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/MultiEdit.h>
#include <GG/PtRect.h>
#include <GG/WndEvent.h>

#include <deque>
#include <unordered_set>
#include <valarray>
#include <vector>
#include <unordered_map>

namespace {
    const double    ZOOM_STEP_SIZE = std::pow(2.0, 1.0/4.0);
    const double    ZOOM_IN_MAX_STEPS = 12.0;
    const double    ZOOM_IN_MIN_STEPS = -10.0;//-7.0;   // negative zoom steps indicates zooming out
    const double    ZOOM_MAX = std::pow(ZOOM_STEP_SIZE, ZOOM_IN_MAX_STEPS);
    const double    ZOOM_MIN = std::pow(ZOOM_STEP_SIZE, ZOOM_IN_MIN_STEPS);

    const GG::X     SITREP_PANEL_WIDTH(400);
    const GG::Y     SITREP_PANEL_HEIGHT(200);

    const std::string SITREP_WND_NAME = "map.sitrep";
    const std::string MAP_PEDIA_WND_NAME = "map.pedia";
    const std::string OBJECT_WND_NAME = "map.object-list";
    const std::string MODERATOR_WND_NAME = "map.moderator";
    const std::string COMBAT_REPORT_WND_NAME = "combat.summary";
    const std::string MAP_SIDEPANEL_WND_NAME = "map.sidepanel";

    const GG::Y     ZOOM_SLIDER_HEIGHT(200);
    const GG::Y     SCALE_LINE_HEIGHT(20);
    const GG::X     SCALE_LINE_MAX_WIDTH(240);
    const int       MIN_SYSTEM_NAME_SIZE = 10;
    const int       LAYOUT_MARGIN = 5;
    const GG::Y     TOOLBAR_HEIGHT(32);

    const double    TWO_PI = 2.0*3.1415926536;

    DeclareThreadSafeLogger(effects);

    double ZoomScaleFactor(double steps_in) {
        if (steps_in > ZOOM_IN_MAX_STEPS) {
            ErrorLogger() << "ZoomScaleFactor passed steps in (" << steps_in << ") higher than max (" << ZOOM_IN_MAX_STEPS << "), so using max";
            steps_in = ZOOM_IN_MAX_STEPS;
        } else if (steps_in < ZOOM_IN_MIN_STEPS) {
            ErrorLogger() << "ZoomScaleFactor passed steps in (" << steps_in << ") lower than minimum (" << ZOOM_IN_MIN_STEPS << "), so using min";
            steps_in = ZOOM_IN_MIN_STEPS;
        }
        return std::pow(ZOOM_STEP_SIZE, steps_in);
    }

    void AddOptions(OptionsDB& db) {
        db.Add("ui.map.background.gas.shown",               UserStringNop("OPTIONS_DB_GALAXY_MAP_GAS"),                         true,                           Validator<bool>());
        db.Add("ui.map.background.starfields.shown",        UserStringNop("OPTIONS_DB_GALAXY_MAP_STARFIELDS"),                  true,                           Validator<bool>());

        db.Add("ui.map.scale.legend.shown",                 UserStringNop("OPTIONS_DB_GALAXY_MAP_SCALE_LINE"),                  true,                           Validator<bool>());
        db.Add("ui.map.scale.circle.shown",                 UserStringNop("OPTIONS_DB_GALAXY_MAP_SCALE_CIRCLE"),                false,                          Validator<bool>());

        db.Add("ui.map.zoom.slider.shown",                  UserStringNop("OPTIONS_DB_GALAXY_MAP_ZOOM_SLIDER"),                 false,                          Validator<bool>());

        db.Add("ui.map.starlane.thickness",                 UserStringNop("OPTIONS_DB_STARLANE_THICKNESS"),                     2.0,                            RangedStepValidator<double>(0.25, 0.25, 10.0));
        db.Add("ui.map.starlane.thickness.factor",          UserStringNop("OPTIONS_DB_STARLANE_CORE"),                          2.0,                            RangedStepValidator<double>(1.0, 1.0, 10.0));
        db.Add("ui.map.starlane.empire.color.shown",        UserStringNop("OPTIONS_DB_RESOURCE_STARLANE_COLOURING"),            true,                           Validator<bool>());

        db.Add("ui.map.fleet.supply.shown",                 UserStringNop("OPTIONS_DB_FLEET_SUPPLY_LINES"),                     true,                           Validator<bool>());
        db.Add("ui.map.fleet.supply.width",                 UserStringNop("OPTIONS_DB_FLEET_SUPPLY_LINE_WIDTH"),                3.0,                            RangedStepValidator<double>(0.25, 0.25, 10.0));
        db.Add("ui.map.fleet.supply.dot.spacing",           UserStringNop("OPTIONS_DB_FLEET_SUPPLY_LINE_DOT_SPACING"),          20,                             RangedStepValidator<int>(1, 3, 40));
        db.Add("ui.map.fleet.supply.dot.rate",              UserStringNop("OPTIONS_DB_FLEET_SUPPLY_LINE_DOT_RATE"),             0.02,                           RangedStepValidator<double>(0.01, 0.01, 0.1));

        db.Add("ui.fleet.explore.hostile.ignored",          UserStringNop("OPTIONS_DB_FLEET_EXPLORE_IGNORE_HOSTILE"),           false,                          Validator<bool>());
        db.Add("ui.fleet.explore.system.route.limit",       UserStringNop("OPTIONS_DB_FLEET_EXPLORE_SYSTEM_ROUTE_LIMIT"),       25,                             StepValidator<int>(1, -1));
        db.Add("ui.fleet.explore.system.known.multiplier",  UserStringNop("OPTIONS_DB_FLEET_EXPLORE_SYSTEM_KNOWN_MULTIPLIER"),  10.0f,                          Validator<float>());

        db.Add("ui.map.starlane.color",                     UserStringNop("OPTIONS_DB_UNOWNED_STARLANE_COLOUR"),                GG::Clr(72,  72,  72,  255),    Validator<GG::Clr>());

        db.Add("ui.map.detection.range.shown",              UserStringNop("OPTIONS_DB_GALAXY_MAP_DETECTION_RANGE"),             true,                           Validator<bool>());

        db.Add("ui.map.scanlines.shown",                    UserStringNop("OPTIONS_DB_UI_SYSTEM_FOG"),                          true,                           Validator<bool>());
        db.Add("ui.map.system.scanlines.spacing",           UserStringNop("OPTIONS_DB_UI_SYSTEM_FOG_SPACING"),                  4.0,                            RangedStepValidator<double>(0.25, 1.5, 8.0));
        db.Add("ui.map.system.scanlines.color",             UserStringNop("OPTIONS_DB_UI_SYSTEM_FOG_CLR"),                      GG::Clr(36, 36, 36, 192),       Validator<GG::Clr>());
        db.Add("ui.map.field.scanlines.color",              UserStringNop("OPTIONS_DB_UI_FIELD_FOG_CLR"),                       GG::Clr(0, 0, 0, 64),           Validator<GG::Clr>());

        db.Add("ui.map.system.icon.size",                   UserStringNop("OPTIONS_DB_UI_SYSTEM_ICON_SIZE"),                    14,                             RangedValidator<int>(8, 50));

        db.Add("ui.map.system.circle.shown",                UserStringNop("OPTIONS_DB_UI_SYSTEM_CIRCLES"),                      true,                           Validator<bool>());
        db.Add("ui.map.system.circle.size",                 UserStringNop("OPTIONS_DB_UI_SYSTEM_CIRCLE_SIZE"),                  1.5,                            RangedStepValidator<double>(0.125, 1.0, 2.5));
        db.Add("ui.map.system.circle.inner.width",          UserStringNop("OPTIONS_DB_UI_SYSTEM_INNER_CIRCLE_WIDTH"),           2.0,                            RangedStepValidator<double>(0.5, 1.0, 8.0));
        db.Add("ui.map.system.circle.outer.width",          UserStringNop("OPTIONS_DB_UI_SYSTEM_OUTER_CIRCLE_WIDTH"),           2.0,                            RangedStepValidator<double>(0.5, 1.0, 8.0));
        db.Add("ui.map.system.circle.inner.max.width",      UserStringNop("OPTIONS_DB_UI_SYSTEM_INNER_CIRCLE_MAX_WIDTH"),       5.0,                            RangedStepValidator<double>(0.5, 1.0, 12.0));
        db.Add("ui.map.system.circle.distance",             UserStringNop("OPTIONS_DB_UI_SYSTEM_CIRCLE_DISTANCE"),              2.0,                            RangedStepValidator<double>(0.5, 1.0, 8.0));

        db.Add("ui.map.system.unexplored.rollover.enabled", UserStringNop("OPTIONS_DB_UI_SYSTEM_UNEXPLORED_OVERLAY"),           true,                           Validator<bool>());

        db.Add("ui.map.system.icon.tiny.threshold",         UserStringNop("OPTIONS_DB_UI_SYSTEM_TINY_ICON_SIZE_THRESHOLD"),     10,                             RangedValidator<int>(1, 16));

        db.Add("ui.map.system.select.indicator.size",       UserStringNop("OPTIONS_DB_UI_SYSTEM_SELECTION_INDICATOR_SIZE"),     1.625,                          RangedStepValidator<double>(0.125, 0.5, 5));
        db.Add("ui.map.system.select.indicator.rpm",        UserStringNop("OPTIONS_DB_UI_SYSTEM_SELECTION_INDICATOR_FPS"),      12,                             RangedValidator<int>(1, 60));

        db.Add("ui.map.system.unowned.name.color",          UserStringNop("OPTIONS_DB_UI_SYSTEM_NAME_UNOWNED_COLOR"),           GG::Clr(160, 160, 160, 255),    Validator<GG::Clr>());

        db.Add("ui.map.fleet.button.tiny.zoom.threshold",   UserStringNop("OPTIONS_DB_UI_TINY_FLEET_BUTTON_MIN_ZOOM"),          0.75,                           RangedStepValidator<double>(0.125, 0.125, 4.0));
        db.Add("ui.map.fleet.button.small.zoom.threshold",  UserStringNop("OPTIONS_DB_UI_SMALL_FLEET_BUTTON_MIN_ZOOM"),         1.50,                           RangedStepValidator<double>(0.125, 0.125, 4.0));
        db.Add("ui.map.fleet.button.medium.zoom.threshold", UserStringNop("OPTIONS_DB_UI_MEDIUM_FLEET_BUTTON_MIN_ZOOM"),        4.00,                           RangedStepValidator<double>(0.125, 0.125, 4.0));

        db.Add("ui.map.detection.range.opacity",            UserStringNop("OPTIONS_DB_GALAXY_MAP_DETECTION_RANGE_OPACITY"),     3,                              RangedValidator<int>(0, 8));

        db.Add("ui.map.menu.enabled",                       UserStringNop("OPTIONS_DB_UI_GALAXY_MAP_POPUP"),                    false,                          Validator<bool>());

        db.Add("ui.production.mappanels.removed",           UserStringNop("OPTIONS_DB_UI_HIDE_MAP_PANELS"),                     false,                          Validator<bool>());

        db.Add("ui.map.sidepanel.width",                    UserStringNop("OPTIONS_DB_UI_SIDEPANEL_WIDTH"),                     512,                            Validator<int>());

        // Register hotkey names/default values for the context "map".
        Hotkey::AddHotkey("ui.map.open",                    UserStringNop("HOTKEY_MAP_RETURN_TO_MAP"),                          GG::GGK_ESCAPE);
        Hotkey::AddHotkey("ui.turn.end",                    UserStringNop("HOTKEY_MAP_END_TURN"),                               GG::GGK_RETURN,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.sitrep",                  UserStringNop("HOTKEY_MAP_SIT_REP"),                                GG::GGK_n,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.research",                    UserStringNop("HOTKEY_MAP_RESEARCH"),                               GG::GGK_r,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.production",                  UserStringNop("HOTKEY_MAP_PRODUCTION"),                             GG::GGK_p,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.design",                      UserStringNop("HOTKEY_MAP_DESIGN"),                                 GG::GGK_d,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.objects",                 UserStringNop("HOTKEY_MAP_OBJECTS"),                                GG::GGK_o,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.messages",                UserStringNop("HOTKEY_MAP_MESSAGES"),                               GG::GGK_t,                      GG::MOD_KEY_ALT);
        Hotkey::AddHotkey("ui.map.empires",                 UserStringNop("HOTKEY_MAP_EMPIRES"),                                GG::GGK_e,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.pedia",                       UserStringNop("HOTKEY_MAP_PEDIA"),                                  GG::GGK_F1);
        Hotkey::AddHotkey("ui.map.graphs",                  UserStringNop("HOTKEY_MAP_GRAPHS"),                                 GG::GGK_NONE);
        Hotkey::AddHotkey("ui.gamemenu",                    UserStringNop("HOTKEY_MAP_MENU"),                                   GG::GGK_F10);
        Hotkey::AddHotkey("ui.zoom.in",                     UserStringNop("HOTKEY_MAP_ZOOM_IN"),                                GG::GGK_z,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.zoom.in.alt",                 UserStringNop("HOTKEY_MAP_ZOOM_IN_ALT"),                            GG::GGK_KP_PLUS,                GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.zoom.out",                    UserStringNop("HOTKEY_MAP_ZOOM_OUT"),                               GG::GGK_x,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.zoom.out.alt",                UserStringNop("HOTKEY_MAP_ZOOM_OUT_ALT"),                           GG::GGK_KP_MINUS,               GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.system.zoom.home",        UserStringNop("HOTKEY_MAP_ZOOM_HOME_SYSTEM"),                       GG::GGK_h,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.system.zoom.prev",        UserStringNop("HOTKEY_MAP_ZOOM_PREV_SYSTEM"),                       GG::GGK_COMMA,                  GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.system.zoom.next",        UserStringNop("HOTKEY_MAP_ZOOM_NEXT_SYSTEM"),                       GG::GGK_PERIOD,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.system.owned.zoom.prev",  UserStringNop("HOTKEY_MAP_ZOOM_PREV_OWNED_SYSTEM"),                 GG::GGK_LESS,                   GG::MOD_KEY_CTRL | GG::MOD_KEY_SHIFT);
        Hotkey::AddHotkey("ui.map.system.owned.zoom.next",  UserStringNop("HOTKEY_MAP_ZOOM_NEXT_OWNED_SYSTEM"),                 GG::GGK_GREATER,                GG::MOD_KEY_CTRL | GG::MOD_KEY_SHIFT);
        Hotkey::AddHotkey("ui.map.fleet.zoom.prev",         UserStringNop("HOTKEY_MAP_ZOOM_PREV_FLEET"),                        GG::GGK_f,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.fleet.zoom.next",         UserStringNop("HOTKEY_MAP_ZOOM_NEXT_FLEET"),                        GG::GGK_g,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.fleet.idle.zoom.prev",    UserStringNop("HOTKEY_MAP_ZOOM_PREV_IDLE_FLEET"),                   GG::GGK_f,                      GG::MOD_KEY_ALT);
        Hotkey::AddHotkey("ui.map.fleet.idle.zoom.next",    UserStringNop("HOTKEY_MAP_ZOOM_NEXT_IDLE_FLEET"),                   GG::GGK_g,                      GG::MOD_KEY_ALT);

        Hotkey::AddHotkey("ui.pan.right",                   UserStringNop("HOTKEY_MAP_PAN_RIGHT"),                              GG::GGK_RIGHT,                  GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.pan.left",                    UserStringNop("HOTKEY_MAP_PAN_LEFT"),                               GG::GGK_LEFT,                   GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.pan.up",                      UserStringNop("HOTKEY_MAP_PAN_UP"),                                 GG::GGK_UP,                     GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.pan.down",                    UserStringNop("HOTKEY_MAP_PAN_DOWN"),                               GG::GGK_DOWN,                   GG::MOD_KEY_CTRL);

        Hotkey::AddHotkey("ui.map.scale.legend",            UserStringNop("HOTKEY_MAP_TOGGLE_SCALE_LINE"),                      GG::GGK_l,                      GG::MOD_KEY_ALT);
        Hotkey::AddHotkey("ui.map.scale.circle",            UserStringNop("HOTKEY_MAP_TOGGLE_SCALE_CIRCLE"),                    GG::GGK_c,                      GG::MOD_KEY_ALT);

        Hotkey::AddHotkey("ui.cut",                         UserStringNop("HOTKEY_CUT"),                                        GG::GGK_x,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.copy",                        UserStringNop("HOTKEY_COPY"),                                       GG::GGK_c,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.paste",                       UserStringNop("HOTKEY_PASTE"),                                      GG::GGK_v,                      GG::MOD_KEY_CTRL);

        Hotkey::AddHotkey("ui.select.all",                  UserStringNop("HOTKEY_SELECT_ALL"),                                 GG::GGK_a,                      GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.select.none",                 UserStringNop("HOTKEY_DESELECT"),                                   GG::GGK_d,                      GG::MOD_KEY_CTRL);

        Hotkey::AddHotkey("ui.focus.prev",                  UserStringNop("HOTKEY_FOCUS_PREV_WND"),                             GG::GGK_TAB,                    GG::MOD_KEY_SHIFT);
        Hotkey::AddHotkey("ui.focus.next",                  UserStringNop("HOTKEY_FOCUS_NEXT_WND"),                             GG::GGK_TAB);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    /* Returns fractional distance along line segment between two points that a
     * third point between them is.assumes the "mid" point is between the
     * "start" and "end" points, in which case the returned fraction is between
     * 0.0 and 1.0 */
    double FractionalDistanceBetweenPoints(double startX, double startY, double midX, double midY, double endX, double endY) {
        // get magnitudes of vectors
        double full_deltaX = endX - startX, full_deltaY = endY - startY;
        double mid_deltaX = midX - startX, mid_deltaY = midY - startY;
        double full_length = std::sqrt(full_deltaX*full_deltaX + full_deltaY*full_deltaY);
        if (full_length == 0.0) // safety check
            full_length = 1.0;
        double mid_length = std::sqrt(mid_deltaX*mid_deltaX + mid_deltaY*mid_deltaY);
        return mid_length / full_length;
    }

    /* Returns point that is dist ditance away from (X1, Y1) in the direction
     * of (X2, Y2) */
    std::pair<double, double> PositionFractionalAtDistanceBetweenPoints(double X1, double Y1, double X2, double Y2, double dist) {
        double newX = X1 + (X2 - X1) * dist;
        double newY = Y1 + (Y2 - Y1) * dist;
        return {newX, newY};
    }

    /* Returns apparent map X and Y position of an object at universe position
     * \a X and \a Y for an object that is located on a starlane between
     * systems with ids \a lane_start_sys_id and \a lane_end_sys_id
     * The apparent position of an object depends on its actual position, the
     * actual positions of the systems at the ends of the lane, and the
     * apparent positions of the ends of the lanes.  The apparent position of
     * objects on the lane is compressed into the space between the apparent
     * ends of the lane, but is proportional to the distance of the actual
     * position along the lane. */
    boost::optional<std::pair<double, double>> ScreenPosOnStarlane(double X, double Y, int lane_start_sys_id, int lane_end_sys_id, const LaneEndpoints& screen_lane_endpoints) {
        // get endpoints of lane in universe.  may be different because on-
        // screen lanes are drawn between system circles, not system centres
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        auto prev = GetEmpireKnownObject(lane_start_sys_id, empire_id);
        auto next = GetEmpireKnownObject(lane_end_sys_id, empire_id);
        if (!next || !prev) {
            ErrorLogger() << "ScreenPosOnStarlane couldn't find next system " << lane_start_sys_id << " or prev system " << lane_end_sys_id;
            return boost::none;
        }

        // get fractional distance along lane that fleet's universe position is
        double dist_along_lane = FractionalDistanceBetweenPoints(prev->X(), prev->Y(), X, Y, next->X(), next->Y());

        return PositionFractionalAtDistanceBetweenPoints(screen_lane_endpoints.X1, screen_lane_endpoints.Y1,
                                                         screen_lane_endpoints.X2, screen_lane_endpoints.Y2,
                                                         dist_along_lane);
    }

    GG::X WndLeft(const GG::Wnd* wnd) { return wnd ? wnd->Left() : GG::X0; }
    GG::X WndRight(const GG::Wnd* wnd) { return wnd ? wnd->Right() : GG::X0; }
    GG::Y WndTop(const GG::Wnd* wnd) { return wnd ? wnd->Top() : GG::Y0; }
    GG::Y WndBottom(const GG::Wnd* wnd) { return wnd ? wnd->Bottom() : GG::Y0; }
    bool InRect(GG::X left, GG::Y top, GG::X right, GG::Y bottom, const GG::Pt& pt)
    { return pt.x >= left && pt.y >= top && pt.x < right && pt.y < bottom; } //pt >= ul && pt < lr;

    GG::X AppWidth() {
        if (HumanClientApp* app = HumanClientApp::GetApp())
            return app->AppWidth();
        return GG::X0;
    }

    GG::Y AppHeight() {
        if (HumanClientApp* app = HumanClientApp::GetApp())
            return app->AppHeight();
        return GG::Y0;
    }

    bool ClientPlayerIsModerator()
    { return HumanClientApp::GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR; }

    void PlayTurnButtonClickSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.button.turn.press.sound.path"), true); }

    bool ToggleBoolOption(const std::string& option_name) {
        bool initially_enabled = GetOptionsDB().Get<bool>(option_name);
        GetOptionsDB().Set(option_name, !initially_enabled);
        return !initially_enabled;
    }

    const std::string FLEET_DETAIL_SHIP_COUNT{UserStringNop("MAP_FLEET_SHIP_COUNT")};
    const std::string FLEET_DETAIL_ARMED_COUNT{UserStringNop("MAP_FLEET_ARMED_COUNT")};
    const std::string FLEET_DETAIL_SLOT_COUNT{UserStringNop("MAP_FLEET_SLOT_COUNT")};
    const std::string FLEET_DETAIL_PART_COUNT{UserStringNop("MAP_FLEET_PART_COUNT")};
    const std::string FLEET_DETAIL_UNARMED_COUNT{UserStringNop("MAP_FLEET_UNARMED_COUNT")};
    const std::string FLEET_DETAIL_COLONY_COUNT{UserStringNop("MAP_FLEET_COLONY_COUNT")};
    const std::string FLEET_DETAIL_CARRIER_COUNT{UserStringNop("MAP_FLEET_CARRIER_COUNT")};
    const std::string FLEET_DETAIL_TROOP_COUNT{UserStringNop("MAP_FLEET_TROOP_COUNT")};


    /** BrowseInfoWnd for the fleet icon tooltip */
    class FleetDetailBrowseWnd : public GG::BrowseInfoWnd {
    public:
        FleetDetailBrowseWnd(int empire_id, GG::X width) :
            GG::BrowseInfoWnd(GG::X0, GG::Y0, width, GG::Y(ClientUI::Pts())),
            m_empire_id(empire_id),
            m_margin(5)
        {
            GG::X value_col_width{(m_margin * 3) + (ClientUI::Pts() * 3)};
            m_col_widths = {width - value_col_width, value_col_width};

            RequirePreRender();
        }

        void PreRender() override {
            SetChildClippingMode(ClipToClient);

            NewLabelValue(FLEET_DETAIL_SHIP_COUNT, true);
            NewLabelValue(FLEET_DETAIL_ARMED_COUNT);
            NewLabelValue(FLEET_DETAIL_SLOT_COUNT);
            NewLabelValue(FLEET_DETAIL_PART_COUNT);
            NewLabelValue(FLEET_DETAIL_UNARMED_COUNT);
            NewLabelValue(FLEET_DETAIL_COLONY_COUNT);
            NewLabelValue(FLEET_DETAIL_CARRIER_COUNT);
            NewLabelValue(FLEET_DETAIL_TROOP_COUNT);

            UpdateLabels();
            ResetShipDesignLabels();
            DoLayout();
        }

        typedef std::pair<std::shared_ptr<CUILabel>,
                          std::shared_ptr<CUILabel>> LabelValueType;

        bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override {
            assert(mode <= wnd->BrowseModes().size());
            return true;
        }

        void Render() override {
            const GG::Y row_height{ClientUI::Pts() + (m_margin * 2)};
            const GG::Y offset{32};
            const GG::Clr& BG_CLR = ClientUI::WndColor();
            const GG::Clr& BORDER_CLR = ClientUI::WndOuterBorderColor();
            const GG::Pt& UL = GG::Pt(UpperLeft().x, UpperLeft().y + offset);
            const GG::Pt& LR = LowerRight();

            // main background
            GG::FlatRectangle(UL, LR, BG_CLR, BORDER_CLR);

            // summary text background
            GG::FlatRectangle(UL, GG::Pt(LR.x, row_height + offset), BORDER_CLR, BORDER_CLR);

            // seperation line between armed/unarmed and utility ships
            GG::Y line_ht(UL.y + (row_height * 2) + (row_height * 5 / 4));
            GG::Pt line_ul(UL.x + (m_margin * 2), line_ht);
            GG::Pt line_lr(LR.x - (m_margin * 2), line_ht);
            GG::Line(line_ul, line_lr, BORDER_CLR);

            // seperation line between ships and parts
            line_ht = {UL.y + (row_height * 5) + (row_height * 6 / 4)};
            line_ul = {UL.x + (m_margin * 2), line_ht};
            line_lr = {LR.x - (m_margin * 2), line_ht};
            GG::Line(line_ul, line_lr, BORDER_CLR);

            // seperation line between parts and designs
            line_ht = { UL.y + (row_height * 7) + (row_height * 7 / 4) };
            line_ul = { UL.x + (m_margin * 2), line_ht };
            line_lr = { LR.x - (m_margin * 2), line_ht };
            GG::Line(line_ul, line_lr, BORDER_CLR);
        }

        void DoLayout() {
            const GG::Y row_height{ClientUI::Pts()};
            const GG::Y offset{32};
            const GG::X descr_width{m_col_widths.at(0) - (m_margin * 2)};
            const GG::X value_width{m_col_widths.at(1) - (m_margin * 3)};

            GG::Pt descr_ul{GG::X{m_margin}, offset + m_margin};
            GG::Pt descr_lr{descr_ul.x + descr_width, offset + row_height};
            GG::Pt value_ul{descr_lr.x + m_margin, descr_ul.y};
            GG::Pt value_lr{value_ul.x + value_width, descr_lr.y};

            const GG::Pt next_row{GG::X0, row_height + (m_margin * 2)};
            const GG::Pt space_row{next_row * 5 / 4};

            LayoutRow(FLEET_DETAIL_SHIP_COUNT,
                      descr_ul, descr_lr, value_ul, value_lr, space_row);
            LayoutRow(FLEET_DETAIL_ARMED_COUNT,
                      descr_ul, descr_lr, value_ul, value_lr, next_row);
            LayoutRow(FLEET_DETAIL_UNARMED_COUNT,
                      descr_ul, descr_lr, value_ul, value_lr, space_row);
            LayoutRow(FLEET_DETAIL_CARRIER_COUNT,
                      descr_ul, descr_lr, value_ul, value_lr, next_row);
            LayoutRow(FLEET_DETAIL_TROOP_COUNT,
                      descr_ul, descr_lr, value_ul, value_lr, next_row);
            LayoutRow(FLEET_DETAIL_COLONY_COUNT,
                      descr_ul, descr_lr, value_ul, value_lr, space_row);
            LayoutRow(FLEET_DETAIL_PART_COUNT,
                      descr_ul, descr_lr, value_ul, value_lr, next_row);
            LayoutRow(FLEET_DETAIL_SLOT_COUNT,
                      descr_ul, descr_lr, value_ul, value_lr,
                      m_ship_design_labels.empty() ? GG::Pt(GG::X0, GG::Y0) : space_row);

            for (auto it = m_ship_design_labels.begin(); it != m_ship_design_labels.end(); ++it) {
                LayoutRow(*it, descr_ul, descr_lr, value_ul, value_lr,
                          std::next(it) == m_ship_design_labels.end()? GG::Pt(GG::X0, GG::Y0) : next_row);
            }

            Resize(GG::Pt(value_lr.x + (m_margin * 3), value_lr.y + (m_margin * 3)));
        }

        /** Constructs and attaches new description and value labels
         *  for the given description row @p descr. */
        void NewLabelValue(const std::string& descr, bool title = false) {
            if (m_labels.count(descr))
                return;

            GG::Y height{ClientUI::Pts()};
            // center format for title label
            m_labels.emplace(descr, std::make_pair(
                GG::Wnd::Create<CUILabel>(UserString(descr),
                                          title ? GG::FORMAT_CENTER : GG::FORMAT_RIGHT,
                                          GG::NO_WND_FLAGS, GG::X0, GG::Y0,
                                          m_col_widths.at(0) - (m_margin * 2), height
                ),
                GG::Wnd::Create<CUILabel>("0", GG::FORMAT_RIGHT,
                                          GG::NO_WND_FLAGS, GG::X0, GG::Y0,
                                          m_col_widths.at(1) - (m_margin * 2), height
                )
            ));

            if (title) {  // utilize bold font for title label and value
                m_labels.at(descr).first->SetFont(ClientUI::GetBoldFont());
                m_labels.at(descr).second->SetFont(ClientUI::GetBoldFont());
            }

            AttachChild(m_labels.at(descr).first);
            AttachChild(m_labels.at(descr).second);
        }

        /** Updates the text displayed for the value of each label */
        void UpdateLabels() {
            UpdateValues();
            for (const auto& value : m_values) {
                auto label_it = m_labels.find(value.first);
                if (label_it == m_labels.end())
                    continue;
                label_it->second.second->ChangeTemplatedText(std::to_string(value.second), 0);
            }
        }

    protected:
        /** Updates the value for display with each label */
        void UpdateValues() {
            m_values.clear();
            m_values[FLEET_DETAIL_SHIP_COUNT] = 0;
            m_ship_design_counts.clear();
            if (m_empire_id == ALL_EMPIRES)
                return;

            const auto& destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(m_empire_id);
            for (auto& ship : Objects().FindObjects<Ship>()) {
                if (!ship->OwnedBy(m_empire_id) || destroyed_objects.count(ship->ID()))
                    continue;
                m_values[FLEET_DETAIL_SHIP_COUNT]++;

                if (ship->IsArmed())
                    m_values[FLEET_DETAIL_ARMED_COUNT]++;
                else
                    m_values[FLEET_DETAIL_UNARMED_COUNT]++;

                if (ship->CanColonize())
                    m_values[FLEET_DETAIL_COLONY_COUNT]++;

                if (ship->HasTroops())
                    m_values[FLEET_DETAIL_TROOP_COUNT]++;

                if (ship->HasFighters())
                    m_values[FLEET_DETAIL_CARRIER_COUNT]++;

                const ShipDesign* design = ship->Design();
                if (!design)
                    continue;
                m_ship_design_counts[design->ID()]++;
                for (const std::string& part : design->Parts()) {
                    m_values[FLEET_DETAIL_SLOT_COUNT] ++;
                    if (!part.empty())
                        m_values[FLEET_DETAIL_PART_COUNT] ++;
                }
            }

        }

        /** Resize/Move controls for row @p descr
         *  and then advance sizes by @p row_advance */
        void LayoutRow(const std::string& descr,
                       GG::Pt& descr_ul, GG::Pt& descr_lr,
                       GG::Pt& value_ul, GG::Pt& value_lr,
                       const GG::Pt& row_advance)
        {
            if (!m_labels.count(descr)) {
                ErrorLogger() << "Unable to find expected label key " << descr;
                return;
            }

            LayoutRow(m_labels.at(descr), descr_ul, descr_lr, value_ul, value_lr, row_advance);
        }

        void LayoutRow(LabelValueType& row,
            GG::Pt& descr_ul, GG::Pt& descr_lr,
            GG::Pt& value_ul, GG::Pt& value_lr,
            const GG::Pt& row_advance)
        {
            row.first->SizeMove(descr_ul, descr_lr);
            if (row.second) {
                row.second->SizeMove(value_ul, value_lr);
            }
            descr_ul += row_advance;
            descr_lr += row_advance;
            value_ul += row_advance;
            value_lr += row_advance;
        }

        /** Remove the old labels for ship design counts, and add the new ones.
            The stats themselves are updated in UpdateValues. */
        void ResetShipDesignLabels() {
            for (auto& labels : m_ship_design_labels) {
                DetachChild(labels.first);
                DetachChild(labels.second);
            }
            m_ship_design_labels.clear();
            for (const auto& entry : m_ship_design_counts) {
                GG::Y height{ ClientUI::Pts() };
                // center format for title label
                m_ship_design_labels.emplace_back(
                    GG::Wnd::Create<CUILabel>(GetShipDesign(entry.first)->Name(),
                        GG::FORMAT_RIGHT,
                        GG::NO_WND_FLAGS, GG::X0, GG::Y0,
                        m_col_widths.at(0) - (m_margin * 2), height
                        ),
                    GG::Wnd::Create<CUILabel>(std::to_string(entry.second),
                        GG::FORMAT_RIGHT,
                        GG::NO_WND_FLAGS, GG::X0, GG::Y0,
                        m_col_widths.at(1) - (m_margin * 2), height
                        )
                );
            }
            std::sort(m_ship_design_labels.begin(), m_ship_design_labels.end(),
                [](LabelValueType a, LabelValueType b) {
                    return a.first->Text() < b.first->Text();
                }
            );
            for (auto& labels : m_ship_design_labels) {
                AttachChild(labels.first);
                AttachChild(labels.second);
            }
        }

    private:
        void UpdateImpl(size_t mode, const Wnd* target) override {
            UpdateLabels();
            ResetShipDesignLabels();
            DoLayout();
        }

        std::unordered_map<std::string, int>            m_values;             ///< Internal value for display with a description
        std::unordered_map<std::string, LabelValueType> m_labels;             ///< Label controls mapped to the description key
        std::unordered_map<int, int>                    m_ship_design_counts; ///< Map of ship design ids to the number of ships with that id
        std::vector<LabelValueType>                     m_ship_design_labels; ///< Label controls for ship designs, sorted by disply name
        int                                             m_empire_id;          ///< ID of the viewing empire
        std::vector<GG::X>                              m_col_widths;         ///< widths of each column
        int                                             m_margin;             ///< margin between controls
    };
}


////////////////////////////////////////////////////////////
// MapWnd::MapScaleLine
////////////////////////////////////////////////////////////
/** Displays a notched line with number labels to indicate Universe distance on the map. */
class MapWnd::MapScaleLine : public GG::Control {
public:
    MapScaleLine(GG::X x, GG::Y y, GG::X w, GG::Y h) :
        GG::Control(x, y, w, h, GG::ONTOP),
        m_scale_factor(1.0),
        m_line_length(GG::X1),
        m_label(nullptr),
        m_enabled(false)
    {
        m_label = GG::Wnd::Create<GG::TextControl>(GG::X0, GG::Y0, GG::X1, GG::Y1, "", ClientUI::GetFont(), ClientUI::TextColor());
    }

    void CompleteConstruction() override {
        GG::Control::CompleteConstruction();
        AttachChild(m_label);
        std::set<int> dummy = std::set<int>();
        Update(1.0, dummy, INVALID_OBJECT_ID);
        GetOptionsDB().OptionChangedSignal("ui.map.scale.legend.shown").connect(
            boost::bind(&MapScaleLine::UpdateEnabled, this));
        UpdateEnabled();
    }

    virtual ~MapScaleLine()
    {}

    void Render() override {
        if (!m_enabled)
            return;

        // use GL to draw line and ticks and labels to indicte a length on the map
        GG::Pt ul = UpperLeft();
        GG::Pt lr = ul + GG::Pt(m_line_length, Height());

        GG::GL2DVertexBuffer verts;
        verts.reserve(6);
        // left border
        verts.store(Value(ul.x), Value(ul.y));
        verts.store(Value(ul.x), Value(lr.y));
        // right border
        verts.store(Value(lr.x), Value(ul.y));
        verts.store(Value(lr.x), Value(lr.y));
        // bottom line
        verts.store(Value(ul.x), Value(lr.y));
        verts.store(Value(lr.x), Value(lr.y));

        glColor(GG::CLR_WHITE);
        glLineWidth(2.0);

        glPushAttrib(GL_ENABLE_BIT | GL_LINE_WIDTH);
        glDisable(GL_TEXTURE_2D);

        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        verts.activate();
        glDrawArrays(GL_LINES, 0, verts.size());

        glPopClientAttrib();
        glPopAttrib();
    }

    GG::X GetLength() const
    { return m_line_length; }

    double GetScaleFactor() const
    { return m_scale_factor; }

    void Update(double zoom_factor, std::set<int>& fleet_ids, int sel_system_id) {
        // The uu length of the map scale line is generally adjusted in this routine up or down by factors of two or five as
        // the zoom_factor changes, so that it's pixel length on the screen is kept to a reasonable distance.  We also add
        // additional stopping points for the map scale to augment the usefulness of the linked map scale circle (whose
        // radius is the same as the map scale length).  These additional stopping points include the speeds and detection
        // ranges of any selected fleets, and the detection ranges of all planets in the currently selected system,
        // provided such values are at least 20 uu.

        // get selected fleet speeds and detection ranges
        std::set<double> fixed_distances;
        for (int fleet_id : fleet_ids) {
            if (auto fleet = GetFleet(fleet_id)) {
                if (fleet->Speed() > 20)
                    fixed_distances.insert(fleet->Speed());
                for (int ship_id : fleet->ShipIDs()) {
                    if (auto ship = GetShip(ship_id)) {
                        const float ship_range = ship->InitialMeterValue(METER_DETECTION);
                        if (ship_range > 20)
                            fixed_distances.insert(ship_range);
                        const float ship_speed = ship->Speed();
                        if (ship_speed > 20)
                            fixed_distances.insert(ship_speed);
                    }
                }
            }
        }
        // get detection ranges for planets in the selected system (if any)
        if (const auto system = GetSystem(sel_system_id)) {
            for (int planet_id : system->PlanetIDs()) {
                if (const auto planet = GetPlanet(planet_id)) {
                    const float planet_range = planet->InitialMeterValue(METER_DETECTION);
                    if (planet_range > 20)
                        fixed_distances.insert(planet_range);
                }
            }
        }

        zoom_factor = std::min(std::max(zoom_factor, ZOOM_MIN), ZOOM_MAX);  // sanity range limits to prevent divide by zero
        m_scale_factor = zoom_factor;

        // determine length of line to draw and how long that is in universe units
        double AVAILABLE_WIDTH = static_cast<double>(std::max(Value(Width()), 1));

        // length in universe units that could be shown if full AVAILABLE_WIDTH was used
        double max_shown_length = AVAILABLE_WIDTH / m_scale_factor;

        // select an actual shown length in universe units by reducing max_shown_length to a nice round number,
        // where nice round numbers are numbers beginning with 1, 2 or 5
        // We start by getting the highest power of 10 that is less than the max_shown_length,
        // and then we step that initial value up, either by a factor of 2 or 5 if that
        // will still fit, or to one of the values taken from fleets and planets
        // if they are at least as reat as the standard value but not larger than the max_shown_length

        // get appropriate power of 10
        double pow10 = floor(log10(max_shown_length));
        double init_length = std::pow(10.0, pow10);
        double shown_length = init_length;

        // determin a preliminary shown length
        // see if next higher multiples of 5 or 2 can be used
        if (shown_length * 5.0 <= max_shown_length)
            shown_length *= 5.0;
        else if (shown_length * 2.0 <= max_shown_length)
            shown_length *= 2.0;

        // DebugLogger()  << "MapScaleLine::Update maxLen: " << max_shown_length
        //                        << "; init_length: " << init_length
        //                        << "; shown_length: " << shown_length;

        // for (double distance : fixed_distances) {
        //     DebugLogger()  << " MapScaleLine::Update fleet speed: " << distance;
        // }

        // if there are additional scale steps to check (from fleets & planets)
        if (!fixed_distances.empty()) {
            // check if the set of fixed distances includes a value that is in between the max_shown_length
            // and one zoom step smaller than the max shown length.  If so, use that for the shown_length;
            // otherwise, check if the set of fixed distances includes a value that is greater than the
            // shown_length determined above but still less than the max_shown_length, and if so then use that value.
            // Note: if the ZOOM_STEP_SIZE is too large, then potential values in the set of fixed distances
            // might get skipped over.
            std::set<double>::iterator distance_ub = fixed_distances.upper_bound(max_shown_length/ZOOM_STEP_SIZE);
            if (distance_ub != fixed_distances.end() && *distance_ub <= max_shown_length) {
                DebugLogger()  << " MapScaleLine::Update distance_ub: " << *distance_ub;
                shown_length = *distance_ub;
            } else {
                distance_ub = fixed_distances.upper_bound(shown_length);
                if (distance_ub != fixed_distances.end() && *distance_ub <= max_shown_length) {
                    DebugLogger()  << " MapScaleLine::Update distance_ub: " << *distance_ub;
                    shown_length = *distance_ub;
                }
            }
        }

        // determine end of drawn scale line
        m_line_length = GG::X(static_cast<int>(shown_length * m_scale_factor));

        // update text
        std::string label_text = boost::io::str(FlexibleFormat(UserString("MAP_SCALE_INDICATOR")) %
                                                std::to_string(static_cast<int>(shown_length)));
        m_label->SetText(label_text);
        m_label->Resize(GG::Pt(GG::X(m_line_length), Height()));
    }

private:
    void UpdateEnabled() {
        m_enabled = GetOptionsDB().Get<bool>("ui.map.scale.legend.shown");
        if (m_enabled)
            AttachChild(m_label);
        else
            DetachChild(m_label);
    }

    double              m_scale_factor;
    GG::X               m_line_length;
    std::shared_ptr<GG::TextControl>    m_label;
    bool                m_enabled;
};

////////////////////////////////////////////////////////////
// MapWndPopup
////////////////////////////////////////////////////////////
MapWndPopup::MapWndPopup(const std::string& t, GG::X default_x, GG::Y default_y, GG::X default_w, GG::Y default_h,
                         GG::Flags<GG::WndFlag> flags, const std::string& config_name) :
    CUIWnd(t, default_x, default_y, default_w, default_h, flags, config_name)
{}

MapWndPopup::MapWndPopup(const std::string& t, GG::Flags<GG::WndFlag> flags, const std::string& config_name) :
    CUIWnd(t, flags, config_name)
{}

void MapWndPopup::CompleteConstruction() {
    CUIWnd::CompleteConstruction();

    // MapWndPopupWnd is registered as a top level window, the same as ClientUI and MapWnd.
    // Consequently, when the GUI shutsdown either could be destroyed before this Wnd
    if (auto client = ClientUI::GetClientUI())
        if (auto mapwnd = client->GetMapWnd())
            mapwnd->RegisterPopup(std::static_pointer_cast<MapWndPopup>(shared_from_this()));
}

MapWndPopup::~MapWndPopup()
{
    if (!Visible()) {
        // Make sure it doesn't save visible = 0 to the config (doesn't
        // make sense for windows that are created/destroyed repeatedly),
        // try/catch because this is called from a dtor and OptionsDB
        // functions can throw.
        try {
            Show();
        } catch (const std::exception& e) {
            ErrorLogger() << "~MapWndPopup() : caught exception cleaning up a popup: " << e.what();
        }
    }

    // MapWndPopupWnd is registered as a top level window, the same as ClientUI and MapWnd.
    // Consequently, when the GUI shutsdown either could be destroyed before this Wnd
    if (auto client = ClientUI::GetClientUI())
        if (auto mapwnd = client->GetMapWnd())
            mapwnd->RemovePopup(this);
}

void MapWndPopup::CloseClicked()
{ CUIWnd::CloseClicked(); }

void MapWndPopup::Close()
{ CloseClicked(); }


//////////////////////////////////
//LaneEndpoints
//////////////////////////////////
LaneEndpoints::LaneEndpoints() :
    X1(static_cast<float>(UniverseObject::INVALID_POSITION)),
    Y1(static_cast<float>(UniverseObject::INVALID_POSITION)),
    X2(static_cast<float>(UniverseObject::INVALID_POSITION)),
    Y2(static_cast<float>(UniverseObject::INVALID_POSITION))
{}


////////////////////////////////////////////////
// MapWnd::MovementLineData::Vertex
////////////////////////////////////////////////
struct MapWnd::MovementLineData::Vertex {
    Vertex(double x_, double y_, int eta_, bool show_eta_, bool flag_blockade_ = false, bool flag_supply_block_ = false) :
    x(x_), y(y_), eta(eta_), show_eta(show_eta_), flag_blockade(flag_blockade_), flag_supply_block(flag_supply_block_)
    {}
    double  x, y;       // apparent in-universe position of a point on move line.  not actual universe positions, but rather where the move line vertices are drawn
    int     eta;        // turns taken to reach point by object travelling along move line
    bool    show_eta;   // should an ETA indicator / number be shown over this vertex?
    bool    flag_blockade;
    bool    flag_supply_block;
};

////////////////////////////////////////////////
// MapWnd::MovementLineData
////////////////////////////////////////////////
MapWnd::MovementLineData::MovementLineData() :
    path(),
    colour(GG::CLR_ZERO)
{}

MapWnd::MovementLineData::MovementLineData(const std::list<MovePathNode>& path_,
                                           const std::map<std::pair<int, int>, LaneEndpoints>& lane_end_points_map,
                                           GG::Clr colour_/*= GG::CLR_WHITE*/, int empireID /*= ALL_EMPIRES*/) :
    path(path_),
    colour(colour_)
{
    // generate screen position vertices

    if (path.empty() || path.size() == 1)
        return; // nothing to draw.  need at least two nodes at different locations to draw a line

    //DebugLogger() << "move_line path: ";
    //for (const MovePathNode& node : path)
    //    DebugLogger() << " ... " << node.object_id << " (" << node.x << ", " << node.y << ") eta: " << node.eta << " turn_end: " << node.turn_end;


    // draw lines connecting points of interest along path.  only draw a line if the previous and
    // current positions of the ends of the line are known.
    const   MovePathNode& first_node =  *(path.begin());
    double  prev_node_x =               first_node.x;
    double  prev_node_y =               first_node.y;
    int     prev_sys_id =               first_node.object_id;
    int     prev_eta =                  first_node.eta;
    int     next_sys_id =               INVALID_OBJECT_ID;

    const Empire* empire = GetEmpire(empireID);
    std::set<int> unobstructed;
    bool s_flag = false;
    bool calc_s_flag = false;
    if (empire) {
        unobstructed = empire->SupplyUnobstructedSystems();
        calc_s_flag = true;
        //s_flag = ((first_node.object_id != INVALID_OBJECT_ID) && !unobstructed.count(first_node.object_id));
    }

    for (const MovePathNode& node : path) {
        // stop rendering if end of path is indicated
        if (node.eta == Fleet::ETA_UNKNOWN || node.eta == Fleet::ETA_NEVER || node.eta == Fleet::ETA_OUT_OF_RANGE)
            break;

        // 1) Get systems at ends of lane on which current node is located.

        if (node.object_id == INVALID_OBJECT_ID) {
            // node is in open space.
            // node should have valid prev_sys_id and node.lane_end_id to get info about starlane this node is on
            prev_sys_id = node.lane_start_id;
            next_sys_id = node.lane_end_id;

        } else {
            // node is at a system.
            // node should / may not have a valid lane_end_id, but this system's id is the end of a lane approaching it
            next_sys_id = node.object_id;
            // if this is the first node of the path, prev_sys_id should be set to node.object_id from pre-loop initialization.
            // if this node is later in the path, prev_sys_id should have been set in previous loop iteration
        }

        // skip invalid line segments
        if (prev_sys_id == next_sys_id || next_sys_id == INVALID_OBJECT_ID || prev_sys_id == INVALID_OBJECT_ID)
            continue;


        // 2) Get apparent universe positions of nodes, which depend on endpoints of lane and actual universe position of nodes

        // get lane end points
        auto ends_it = lane_end_points_map.find({prev_sys_id, next_sys_id});
        if (ends_it == lane_end_points_map.end()) {
            ErrorLogger() << "couldn't get endpoints of lane for move line";
            break;
        }
        const LaneEndpoints& lane_endpoints = ends_it->second;

        // get on-screen positions of nodes shifted to fit on starlane
        auto start_xy = ScreenPosOnStarlane(prev_node_x, prev_node_y, prev_sys_id, next_sys_id, lane_endpoints);
        auto end_xy =   ScreenPosOnStarlane(node.x,      node.y,      prev_sys_id, next_sys_id, lane_endpoints);

        if (!start_xy) {
            ErrorLogger() << "System " << prev_sys_id << " has invalid screen coordinates.";
            continue;
        }
        if (!end_xy) {
            ErrorLogger() << "System " << next_sys_id << " has invalid screen coordinates.";
            continue;
        }

        // 3) Add points for line segment to list of Vertices
        bool b_flag = node.post_blockade;
        s_flag = s_flag || (calc_s_flag &&
            ((node.object_id != INVALID_OBJECT_ID) && !unobstructed.count(node.object_id)));
        vertices.push_back(Vertex(start_xy->first,   start_xy->second,    prev_eta,   false,          b_flag, s_flag));
        vertices.push_back(Vertex(end_xy->first,     end_xy->second,      node.eta,   node.turn_end,  b_flag, s_flag));


        // 4) prep for next iteration
        prev_node_x = node.x;
        prev_node_y = node.y;
        prev_eta = node.eta;
        if (node.object_id != INVALID_OBJECT_ID) {  // only need to update previous and next sys ids if current node is at a system
            prev_sys_id = node.object_id;                       // to be used in next iteration
            next_sys_id = INVALID_OBJECT_ID;    // to be set in next iteration
        }
    }
}


///////////////////////////
// MapWnd
///////////////////////////
MapWnd::MapWnd() :
    GG::Wnd(-AppWidth(), -AppHeight(),
            static_cast<GG::X>(GetUniverse().UniverseWidth() * ZOOM_MAX + AppWidth() * 1.5),
            static_cast<GG::Y>(GetUniverse().UniverseWidth() * ZOOM_MAX + AppHeight() * 1.5),
            GG::INTERACTIVE | GG::DRAGABLE),
    m_selected_fleet_ids(),
    m_selected_ship_ids(),
    m_system_icons(),
    m_wnd_stack(),
    m_starlane_endpoints(),
    m_stationary_fleet_buttons(),
    m_departing_fleet_buttons(),
    m_moving_fleet_buttons(),
    m_offroad_fleet_buttons(),
    m_fleet_buttons(),
    m_fleet_state_change_signals(),
    m_system_fleet_insert_remove_signals(),
    m_fleet_lines(),
    m_projected_fleet_lines(),
    m_line_between_systems{INVALID_OBJECT_ID, INVALID_OBJECT_ID},
    m_star_core_quad_vertices(),
    m_star_halo_quad_vertices(),
    m_galaxy_gas_quad_vertices(),
    m_galaxy_gas_texture_coords(),
    m_star_texture_coords(),
    m_star_circle_vertices(),
    m_starlane_vertices(),
    m_starlane_colors(),
    m_RC_starlane_vertices(),
    m_RC_starlane_colors(),
    m_field_vertices(),
    m_field_scanline_circles(),
    m_field_texture_coords(),
    m_visibility_radii_vertices(),
    m_visibility_radii_colors(),
    m_visibility_radii_border_vertices(),
    m_visibility_radii_border_colors(),
    m_radii_radii_vertices_indices_runs(),
    m_scale_circle_vertices(),
    m_starfield_verts(),
    m_starfield_colours(),
    m_popups(),
    m_current_owned_system(INVALID_OBJECT_ID),
    m_current_fleet_id(INVALID_OBJECT_ID)
{}

void MapWnd::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    SetName("MapWnd");

    GetUniverse().UniverseObjectDeleteSignal.connect(
        boost::bind(&MapWnd::UniverseObjectDeleted, this, _1));

    // toolbar
    m_toolbar = GG::Wnd::Create<CUIToolBar>();
    m_toolbar->SetName("MapWnd Toolbar");
    GG::GUI::GetGUI()->Register(m_toolbar);
    m_toolbar->Hide();

    auto layout = GG::Wnd::Create<GG::Layout>(m_toolbar->ClientUpperLeft().x, m_toolbar->ClientUpperLeft().y,
                                              m_toolbar->ClientWidth(),       m_toolbar->ClientHeight(),
                                              1, 22);
    layout->SetName("Toolbar Layout");
    m_toolbar->SetLayout(layout);

    //////////////////////////////
    // Toolbar buttons and icons
    //////////////////////////////

    // turn button
    // determine size from the text that will go into the button, using a test year string
    std::string turn_button_longest_reasonable_text =  boost::io::str(FlexibleFormat(UserString("MAP_BTN_TURN_UPDATE")) % "99999"); // it is unlikely a game will go over 100000 turns
    std::string unready_button_longest_reasonable_text =  boost::io::str(FlexibleFormat(UserString("MAP_BTN_TURN_UNREADY")) % "99999");
    m_btn_turn = Wnd::Create<CUIButton>(turn_button_longest_reasonable_text.size() > unready_button_longest_reasonable_text.size() ?
                                        turn_button_longest_reasonable_text :
                                        unready_button_longest_reasonable_text);
    m_btn_turn->Resize(m_btn_turn->MinUsableSize());
    m_btn_turn->LeftClickedSignal.connect(
        boost::bind(&MapWnd::EndTurn, this));
    m_btn_turn->LeftClickedSignal.connect(
        &PlayTurnButtonClickSound);

    boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

    // auto turn button
    m_btn_auto_turn = Wnd::Create<CUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "manual_turn.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "auto_turn.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "manual_turn_mouseover.png")));

    m_btn_auto_turn->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ToggleAutoEndTurn, this));
    m_btn_auto_turn->Resize(GG::Pt(GG::X(24), GG::Y(24)));
    m_btn_auto_turn->SetMinSize(GG::Pt(GG::X(24), GG::Y(24)));
    ToggleAutoEndTurn();    // toggle twice to set textures without changing default setting state
    ToggleAutoEndTurn();


    // FPS indicator
    m_FPS = GG::Wnd::Create<FPSIndicator>();
    m_FPS->Hide();

    // create custom InWindow function for Menu button that extends its
    // clickable area to the adjacent edges of the toolbar containing it
    boost::function<bool(const SettableInWindowCUIButton*, const GG::Pt&)> in_window_func =
        boost::bind(&InRect, boost::bind(&WndLeft, _1), boost::bind(&WndTop, m_toolbar.get()),
                             boost::bind(&WndRight, _1), boost::bind(&WndBottom, _1),
                    _2);
    // Menu button
    m_btn_menu = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "menu.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "menu_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "menu_mouseover.png")),
        in_window_func);
    m_btn_menu->SetMinSize(GG::Pt(GG::X(32), GG::Y(32)));
    m_btn_menu->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ShowMenu, this));
    m_btn_menu->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_menu->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_MENU"), UserString("MAP_BTN_MENU_DESC")));

    in_window_func =
        boost::bind(&InRect, boost::bind(&WndLeft, _1),   boost::bind(&WndTop, m_toolbar.get()),
                             boost::bind(&WndRight, _1),  boost::bind(&WndBottom, _1),
                    _2);
    // Encyclo"pedia" button
    m_btn_pedia = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "pedia.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "pedia_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "pedia_mouseover.png")),
        in_window_func);
    m_btn_pedia->SetMinSize(GG::Pt(GG::X(32), GG::Y(32)));
    m_btn_pedia->LeftClickedSignal.connect(
        boost::bind(&MapWnd::TogglePedia, this));
    m_btn_pedia->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_pedia->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_PEDIA"), UserString("MAP_BTN_PEDIA_DESC")));

    in_window_func =
        boost::bind(&InRect, boost::bind(&WndLeft, _1),   boost::bind(&WndTop, m_toolbar.get()),
                             boost::bind(&WndRight, _1),  boost::bind(&WndBottom, _1),
                    _2);
    // Graphs button
    m_btn_graphs = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "charts.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "charts_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "charts_mouseover.png")),
        in_window_func);
    m_btn_graphs->SetMinSize(GG::Pt(GG::X(32), GG::Y(32)));
    m_btn_graphs->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ShowGraphs, this));
    m_btn_graphs->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_graphs->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_GRAPH"), UserString("MAP_BTN_GRAPH_DESC")));

    in_window_func =
        boost::bind(&InRect, boost::bind(&WndLeft, _1),   boost::bind(&WndTop, m_toolbar.get()),
                             boost::bind(&WndRight, _1),  boost::bind(&WndBottom, _1),
                    _2);
    // Design button
    m_btn_design = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "design.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "design_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "design_mouseover.png")),
        in_window_func);
    m_btn_design->SetMinSize(GG::Pt(GG::X(32), GG::Y(32)));
    m_btn_design->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ToggleDesign, this));
    m_btn_design->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_design->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_DESIGN"), UserString("MAP_BTN_DESIGN_DESC")));

    in_window_func =
        boost::bind(&InRect, boost::bind(&WndLeft, _1),   boost::bind(&WndTop, m_toolbar.get()),
                             boost::bind(&WndRight, _1),  boost::bind(&WndBottom, _1),
                    _2);
    // Production button
    m_btn_production = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "production.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "production_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "production_mouseover.png")),
        in_window_func);
    m_btn_production->SetMinSize(GG::Pt(GG::X(32), GG::Y(32)));
    m_btn_production->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ToggleProduction, this));
    m_btn_production->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_production->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_PRODUCTION"), UserString("MAP_BTN_PRODUCTION_DESC")));

    in_window_func =
        boost::bind(&InRect, boost::bind(&WndLeft, _1),   boost::bind(&WndTop, m_toolbar.get()),
                             boost::bind(&WndRight, _1),  boost::bind(&WndBottom, _1),
                    _2);
    // Research button
    m_btn_research = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "research.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "research_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "research_mouseover.png")),
        in_window_func);
    m_btn_research->SetMinSize(GG::Pt(GG::X(32), GG::Y(32)));
    m_btn_research->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ToggleResearch, this));
    m_btn_research->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_research->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_RESEARCH"), UserString("MAP_BTN_RESEARCH_DESC")));

    in_window_func =
        boost::bind(&InRect, boost::bind(&WndLeft, _1),   boost::bind(&WndTop, m_toolbar.get()),
                             boost::bind(&WndRight, _1),  boost::bind(&WndBottom, _1),
                    _2);
    // Objects button
    m_btn_objects = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "objects.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "objects_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "objects_mouseover.png")),
        in_window_func);
    m_btn_objects->SetMinSize(GG::Pt(GG::X(32), GG::Y(32)));
    m_btn_objects->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ToggleObjects, this));
    m_btn_objects->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_objects->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_OBJECTS"), UserString("MAP_BTN_OBJECTS_DESC")));

    in_window_func =
        boost::bind(&InRect, boost::bind(&WndLeft, _1),   boost::bind(&WndTop, m_toolbar.get()),
                             boost::bind(&WndRight, _1),  boost::bind(&WndBottom, _1),
                    _2);
    // Empires button
    m_btn_empires = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "empires.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "empires_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "empires_mouseover.png")),
        in_window_func);
    m_btn_empires->SetMinSize(GG::Pt(GG::X(32), GG::Y(32)));
    m_btn_empires->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ToggleEmpires, this));
    m_btn_empires->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_empires->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_EMPIRES"), UserString("MAP_BTN_EMPIRES_DESC")));

    in_window_func =
        boost::bind(&InRect, boost::bind(&WndLeft, _1),  boost::bind(&WndTop, m_toolbar.get()),
                             boost::bind(&WndRight, _1), boost::bind(&WndBottom, _1),
                    _2);
    // SitRep button
    m_btn_siterep = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "sitrep.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "sitrep_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "sitrep_mouseover.png")),
        in_window_func);
    m_btn_siterep->SetMinSize(GG::Pt(GG::X(32), GG::Y(32)));
    m_btn_siterep->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ToggleSitRep, this));
    m_btn_siterep->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_siterep->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_SITREP"), UserString("MAP_BTN_SITREP_DESC")));

    in_window_func =
        boost::bind(&InRect, boost::bind(&WndLeft, _1),  boost::bind(&WndTop, m_toolbar.get()),
                             boost::bind(&WndRight, _1), boost::bind(&WndBottom, _1),
                    _2);
    // Messages button
    m_btn_messages = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "messages.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "messages_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "messages_mouseover.png")),
        in_window_func);
    m_btn_messages->SetMinSize(GG::Pt(GG::X(32), GG::Y(32)));
    m_btn_messages->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ToggleMessages, this));
    m_btn_messages->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_messages->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_MESSAGES"), UserString("MAP_BTN_MESSAGES_DESC")));

    in_window_func =
        boost::bind(&InRect, boost::bind(&WndLeft, _1),  boost::bind(&WndTop, m_toolbar.get()),
                             boost::bind(&WndRight, _1), boost::bind(&WndBottom, _1),
                    _2);
    // Moderator button
    m_btn_moderator = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "moderator.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "moderator_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "moderator_mouseover.png")),
        in_window_func);
    m_btn_moderator->SetMinSize(GG::Pt(GG::X(32), GG::Y(32)));
    m_btn_moderator->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ToggleModeratorActions, this));
    m_btn_moderator->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_moderator->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_MODERATOR"), UserString("MAP_BTN_MODERATOR_DESC")));


    // resources
    const GG::X ICON_DUAL_WIDTH(100);
    const GG::X ICON_WIDTH(24);
    m_population = GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(METER_POPULATION), 0, 3, false,
                                                  ICON_DUAL_WIDTH, m_btn_turn->Height());
    m_population->SetName("Population StatisticIcon");

    m_industry = GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(METER_INDUSTRY), 0, 3, false,
                                                ICON_DUAL_WIDTH, m_btn_turn->Height());
    m_industry->SetName("Industry StatisticIcon");
    m_industry->LeftClickedSignal.connect(boost::bind(&MapWnd::ToggleProduction, this));

    m_stockpile = GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(METER_STOCKPILE), 0, 3, false,
                                                 ICON_DUAL_WIDTH, m_btn_turn->Height());
    m_stockpile->SetName("Stockpile StatisticIcon");

    m_research = GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(METER_RESEARCH), 0, 3, false,
                                                ICON_DUAL_WIDTH, m_btn_turn->Height());
    m_research->SetName("Research StatisticIcon");
    m_research->LeftClickedSignal.connect(boost::bind(&MapWnd::ToggleResearch, this));

    m_trade = GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(METER_TRADE), 0, 3, false,
                                             ICON_DUAL_WIDTH, m_btn_turn->Height());
    m_trade->SetName("Trade StatisticIcon");

    m_fleet = GG::Wnd::Create<StatisticIcon>(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "sitrep" / "fleet_arrived.png"),
                                             0, 3, false,
                                             ICON_DUAL_WIDTH, m_btn_turn->Height());
    m_fleet->SetName("Fleet StatisticIcon");

    m_detection = GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(METER_DETECTION), 0, 3, false,
                                                 ICON_DUAL_WIDTH, m_btn_turn->Height());
    m_detection->SetName("Detection StatisticIcon");

    GG::SubTexture wasted_ressource_subtexture = GG::SubTexture(ClientUI::GetTexture(button_texture_dir /
                                                                "wasted_resource.png", false));
    GG::SubTexture wasted_ressource_mouseover_subtexture = GG::SubTexture(ClientUI::GetTexture(button_texture_dir /
                                                                "wasted_resource_mouseover.png", false));
    GG::SubTexture wasted_ressource_clicked_subtexture = GG::SubTexture(ClientUI::GetTexture(button_texture_dir /
                                                                "wasted_resource_clicked.png", false));

    m_industry_wasted = Wnd::Create<CUIButton>(
        wasted_ressource_subtexture,
        wasted_ressource_clicked_subtexture,
        wasted_ressource_mouseover_subtexture);

    m_research_wasted = Wnd::Create<CUIButton>(
        wasted_ressource_subtexture,
        wasted_ressource_clicked_subtexture,
        wasted_ressource_mouseover_subtexture);

    m_industry_wasted->Resize(GG::Pt(ICON_WIDTH, GG::Y(Value(ICON_WIDTH))));
    m_industry_wasted->SetMinSize(GG::Pt(ICON_WIDTH, GG::Y(Value(ICON_WIDTH))));
    m_research_wasted->Resize(GG::Pt(ICON_WIDTH, GG::Y(Value(ICON_WIDTH))));
    m_research_wasted->SetMinSize(GG::Pt(ICON_WIDTH, GG::Y(Value(ICON_WIDTH))));

    m_industry_wasted->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_research_wasted->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    m_industry_wasted->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ZoomToSystemWithWastedPP, this));
    m_research_wasted->LeftClickedSignal.connect(
        boost::bind(&MapWnd::ToggleResearch, this));

    //Set the correct tooltips
    RefreshIndustryResourceIndicator();
    RefreshResearchResourceIndicator();



    /////////////////////////////////////
    // place buttons / icons on toolbar
    /////////////////////////////////////
    int layout_column(0);

    layout->SetMinimumColumnWidth(layout_column, m_btn_turn->Width());
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_turn,         0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, ICON_WIDTH);
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_auto_turn,    0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, GG::X(ClientUI::Pts()*4));
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_FPS,              0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, ICON_WIDTH);
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_industry_wasted,  0, layout_column, GG::ALIGN_RIGHT | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetColumnStretch(layout_column, 1.0);
    layout->Add(m_industry, 0, layout_column, GG::ALIGN_LEFT | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetColumnStretch(layout_column, 1.2);
    layout->Add(m_stockpile, 0, layout_column, GG::ALIGN_LEFT | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, ICON_WIDTH);
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_research_wasted,  0, layout_column, GG::ALIGN_RIGHT | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetColumnStretch(layout_column, 1.0);
    layout->Add(m_research,         0, layout_column, GG::ALIGN_LEFT | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetColumnStretch(layout_column, 1.0);
    layout->Add(m_fleet,            0, layout_column, GG::ALIGN_LEFT | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetColumnStretch(layout_column, 1.0);
    layout->Add(m_population,       0, layout_column, GG::ALIGN_LEFT | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetColumnStretch(layout_column, 1.0);
    layout->Add(m_detection,        0, layout_column, GG::ALIGN_LEFT | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, m_btn_moderator->Width());
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_moderator,    0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, m_btn_messages->Width());
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_messages,    0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, m_btn_siterep->Width());
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_siterep,      0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, m_btn_empires->Width());
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_empires,      0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, m_btn_objects->Width());
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_objects,      0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, m_btn_research->Width());
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_research,     0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, m_btn_production->Width());
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_production,   0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, m_btn_design->Width());
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_design,       0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, m_btn_graphs->Width());
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_graphs,       0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, m_btn_pedia->Width());
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_pedia,        0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetMinimumColumnWidth(layout_column, m_btn_menu->Width());
    layout->SetColumnStretch(layout_column, 0.0);
    layout->Add(m_btn_menu,         0, layout_column, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    ++layout_column;

    layout->SetCellMargin(5);
    layout->SetBorderMargin(5);


    ///////////////////
    // Misc widgets on map screen
    ///////////////////

    // scale line
    m_scale_line = GG::Wnd::Create<MapScaleLine>(GG::X(LAYOUT_MARGIN),   GG::Y(LAYOUT_MARGIN) + m_toolbar->Height(),
                                                 SCALE_LINE_MAX_WIDTH,   SCALE_LINE_HEIGHT);
    GG::GUI::GetGUI()->Register(m_scale_line);
    int sel_system_id = SidePanel::SystemID();
    m_scale_line->Update(ZoomFactor(), m_selected_fleet_ids, sel_system_id);
    m_scale_line->Hide();

    // Zoom slider
    const int ZOOM_SLIDER_MIN = static_cast<int>(ZOOM_IN_MIN_STEPS),
              ZOOM_SLIDER_MAX = static_cast<int>(ZOOM_IN_MAX_STEPS);
    m_zoom_slider = GG::Wnd::Create<CUISlider<double>>(ZOOM_SLIDER_MIN, ZOOM_SLIDER_MAX, GG::VERTICAL, GG::INTERACTIVE | GG::ONTOP);
    m_zoom_slider->MoveTo(GG::Pt(m_btn_turn->Left(), m_scale_line->Bottom() + GG::Y(LAYOUT_MARGIN)));
    m_zoom_slider->Resize(GG::Pt(GG::X(ClientUI::ScrollWidth()), ZOOM_SLIDER_HEIGHT));
    m_zoom_slider->SlideTo(m_zoom_steps_in);
    GG::GUI::GetGUI()->Register(m_zoom_slider);
    m_zoom_slider->Hide();
    m_zoom_slider->SlidSignal.connect(
        boost::bind(&MapWnd::SetZoom, this, _1, false));
    GetOptionsDB().OptionChangedSignal("ui.map.zoom.slider.shown").connect(
        boost::bind(&MapWnd::RefreshSliders, this));

    ///////////////////
    // Map sub-windows
    ///////////////////

    // system-view side panel
    m_side_panel = GG::Wnd::Create<SidePanel>(MAP_SIDEPANEL_WND_NAME);
    GG::GUI::GetGUI()->Register(m_side_panel);

    SidePanel::SystemSelectedSignal.connect(
        boost::bind(&MapWnd::SelectSystem, this, _1));
    SidePanel::PlanetSelectedSignal.connect(
        boost::bind(&MapWnd::SelectPlanet, this, _1));
    SidePanel::PlanetDoubleClickedSignal.connect(
        boost::bind(&MapWnd::PlanetDoubleClicked, this, _1));
    SidePanel::PlanetRightClickedSignal.connect(
        boost::bind(&MapWnd::PlanetRightClicked, this, _1));
    SidePanel::BuildingRightClickedSignal.connect(
        boost::bind(&MapWnd::BuildingRightClicked, this, _1));

    // not strictly necessary, as in principle whenever any ResourceCenter
    // changes, all meter estimates and resource pools should / could be
    // updated.  however, this is a convenience to limit the updates to
    // what is actually being shown in the sidepanel right now, which is
    // useful since most ResourceCenter changes will be due to focus
    // changes on the sidepanel, and most differences in meter estimates
    // and resource pools due to this will be in the same system
    SidePanel::ResourceCenterChangedSignal.connect(
        boost::bind(&MapWnd::UpdateSidePanelSystemObjectMetersAndResourcePools, this));

    // situation report window
    m_sitrep_panel = GG::Wnd::Create<SitRepPanel>(SITREP_WND_NAME);
    // Wnd is manually closed by user
    m_sitrep_panel->ClosingSignal.connect(
        boost::bind(&MapWnd::HideSitRep, this));
    if (m_sitrep_panel->Visible()) {
        PushWndStack(m_sitrep_panel);
        m_btn_siterep->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "sitrep_mouseover.png")));
        m_btn_siterep->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "sitrep.png")));
    }

    // encyclopedia panel
    m_pedia_panel = GG::Wnd::Create<EncyclopediaDetailPanel>(GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE | PINABLE, MAP_PEDIA_WND_NAME);
    // Wnd is manually closed by user
    m_pedia_panel->ClosingSignal.connect(
        boost::bind(&MapWnd::HidePedia, this));
    if (m_pedia_panel->Visible()) {
        PushWndStack(m_pedia_panel);
        m_btn_pedia->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "pedia_mouseover.png")));
        m_btn_pedia->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "pedia.png")));
    }

    // objects list
    m_object_list_wnd = GG::Wnd::Create<ObjectListWnd>(OBJECT_WND_NAME);
    // Wnd is manually closed by user
    m_object_list_wnd->ClosingSignal.connect(
        boost::bind(&MapWnd::HideObjects, this));
    m_object_list_wnd->ObjectDumpSignal.connect(
        boost::bind(&ClientUI::DumpObject, ClientUI::GetClientUI(), _1));
    if (m_object_list_wnd->Visible()) {
        PushWndStack(m_object_list_wnd);
        m_btn_objects->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "objects_mouseover.png")));
        m_btn_objects->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "objects.png")));
    }

    // moderator actions
    m_moderator_wnd = GG::Wnd::Create<ModeratorActionsWnd>(MODERATOR_WND_NAME);
    m_moderator_wnd->ClosingSignal.connect(
        boost::bind(&MapWnd::HideModeratorActions, this));
    if (m_moderator_wnd->Visible()) {
        PushWndStack(m_moderator_wnd);
        m_btn_moderator->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "moderator_mouseover.png")));
        m_btn_moderator->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "moderator.png")));
    }

    // Combat report
    m_combat_report_wnd = GG::Wnd::Create<CombatReportWnd>(COMBAT_REPORT_WND_NAME);

    // position CUIWnds owned by the MapWnd
    InitializeWindows();

    // messages and empires windows
    if (ClientUI* cui = ClientUI::GetClientUI()) {
        if (const auto& msg_wnd = cui->GetMessageWnd()) {
            // Wnd is manually closed by user
            msg_wnd->ClosingSignal.connect(
                boost::bind(&MapWnd::HideMessages, this));
            if (msg_wnd->Visible()) {
                PushWndStack(msg_wnd);
                m_btn_messages->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "messages_mouseover.png")));
                m_btn_messages->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "messages.png")));
            }
        }
        if (const auto& plr_wnd = cui->GetPlayerListWnd()) {
            // Wnd is manually closed by user
            plr_wnd->ClosingSignal.connect(
                boost::bind(&MapWnd::HideEmpires, this));
            if (plr_wnd->Visible()) {
                PushWndStack(plr_wnd);
                m_btn_empires->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "empires_mouseover.png")));
                m_btn_empires->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "empires.png")));
            }
        }
    }

    HumanClientApp::GetApp()->RepositionWindowsSignal.connect(
        boost::bind(&MapWnd::InitializeWindows, this));

    // research window
    m_research_wnd = GG::Wnd::Create<ResearchWnd>(AppWidth(), AppHeight() - m_toolbar->Height());
    m_research_wnd->MoveTo(GG::Pt(GG::X0, m_toolbar->Height()));
    GG::GUI::GetGUI()->Register(m_research_wnd);
    m_research_wnd->Hide();

    // production window
    m_production_wnd = GG::Wnd::Create<ProductionWnd>(AppWidth(), AppHeight() - m_toolbar->Height());
    m_production_wnd->MoveTo(GG::Pt(GG::X0, m_toolbar->Height()));
    GG::GUI::GetGUI()->Register(m_production_wnd);
    m_production_wnd->Hide();
    m_production_wnd->SystemSelectedSignal.connect(
        boost::bind(&MapWnd::SelectSystem, this, _1));
    m_production_wnd->PlanetSelectedSignal.connect(
        boost::bind(&MapWnd::SelectPlanet, this, _1));

    // design window
    m_design_wnd = GG::Wnd::Create<DesignWnd>(AppWidth(), AppHeight() - m_toolbar->Height());
    m_design_wnd->MoveTo(GG::Pt(GG::X0, m_toolbar->Height()));
    GG::GUI::GetGUI()->Register(m_design_wnd);
    m_design_wnd->Hide();



    //////////////////
    // General Gamestate response signals
    //////////////////
    FleetUIManager& fm = FleetUIManager::GetFleetUIManager();
    fm.ActiveFleetWndChangedSignal.connect(boost::bind(&MapWnd::SelectedFleetsChanged, this));
    fm.ActiveFleetWndSelectedFleetsChangedSignal.connect(boost::bind(&MapWnd::SelectedFleetsChanged, this));
    fm.ActiveFleetWndSelectedShipsChangedSignal.connect(boost::bind(&MapWnd::SelectedShipsChanged, this));
    fm.FleetRightClickedSignal.connect(boost::bind(&MapWnd::FleetRightClicked, this, _1));
    fm.ShipRightClickedSignal.connect(boost::bind(&MapWnd::ShipRightClicked, this, _1));

    DoLayout();

    // Connect keyboard accelerators for map
    ConnectKeyboardAcceleratorSignals();
}

MapWnd::~MapWnd()
{}

void MapWnd::DoLayout() {
    m_toolbar->Resize(GG::Pt(AppWidth(), TOOLBAR_HEIGHT));
    m_research_wnd->Resize(GG::Pt(AppWidth(), AppHeight() - m_toolbar->Height()));
    m_production_wnd->Resize(GG::Pt(AppWidth(), AppHeight() - m_toolbar->Height()));
    m_design_wnd->Resize(GG::Pt(AppWidth(), AppHeight() - m_toolbar->Height()));
    m_sitrep_panel->ValidatePosition();
    m_object_list_wnd->ValidatePosition();
    m_pedia_panel->ValidatePosition();
    m_side_panel->ValidatePosition();
    m_combat_report_wnd->ValidatePosition();
    m_moderator_wnd->ValidatePosition();

    if (ClientUI* cui = ClientUI::GetClientUI()) {
        if (const auto& msg_wnd = cui->GetMessageWnd())
            msg_wnd->ValidatePosition();
        if (const auto& plr_wnd = cui->GetPlayerListWnd())
            plr_wnd->ValidatePosition();
    }

    FleetUIManager::GetFleetUIManager().CullEmptyWnds();
    for (auto& fwnd : FleetUIManager::GetFleetUIManager()) {
        if (auto wnd = fwnd.lock()) {
            wnd->ValidatePosition();
        } else {
            ErrorLogger() << "MapWnd::DoLayout(): null FleetWnd* found in the FleetUIManager::iterator.";
        }
    }
}

void MapWnd::InitializeWindows() {
    const GG::X SIDEPANEL_WIDTH(GetOptionsDB().Get<int>("ui.map.sidepanel.width"));

    // system-view side panel
    const GG::Pt sidepanel_ul(AppWidth() - SIDEPANEL_WIDTH, m_toolbar->Bottom());
    const GG::Pt sidepanel_wh(SIDEPANEL_WIDTH, AppHeight() - m_toolbar->Height());

    // situation report window
    const GG::Pt sitrep_ul(SCALE_LINE_MAX_WIDTH + LAYOUT_MARGIN, m_toolbar->Bottom());
    const GG::Pt sitrep_wh(SITREP_PANEL_WIDTH, SITREP_PANEL_HEIGHT);

    // encyclopedia panel
    const GG::Pt pedia_ul(SCALE_LINE_MAX_WIDTH + LAYOUT_MARGIN, m_toolbar->Bottom() + SITREP_PANEL_HEIGHT);
    const GG::Pt pedia_wh(SITREP_PANEL_WIDTH, SITREP_PANEL_HEIGHT);

    // objects list
    const GG::Pt object_list_ul(GG::X0, m_scale_line->Bottom() + GG::Y(LAYOUT_MARGIN));
    const GG::Pt object_list_wh(SITREP_PANEL_WIDTH, SITREP_PANEL_HEIGHT);

    // moderator actions
    const GG::Pt moderator_ul(GG::X0, m_scale_line->Bottom() + GG::Y(LAYOUT_MARGIN));
    const GG::Pt moderator_wh(SITREP_PANEL_WIDTH, SITREP_PANEL_HEIGHT);

    // Combat report
    // These values were formerly in UI/CombatReport/CombatReportWnd.cpp
    const GG::Pt combat_log_ul(GG::X(150), GG::Y(50));
    const GG::Pt combat_log_wh(GG::X(400), GG::Y(300));

    m_side_panel->       InitSizeMove(sidepanel_ul,   sidepanel_ul + sidepanel_wh);
    m_sitrep_panel->     InitSizeMove(sitrep_ul,      sitrep_ul + sitrep_wh);
    m_pedia_panel->      InitSizeMove(pedia_ul,       pedia_ul + pedia_wh);
    m_object_list_wnd->  InitSizeMove(object_list_ul, object_list_ul + object_list_wh);
    m_moderator_wnd->    InitSizeMove(moderator_ul,   moderator_ul + moderator_wh);
    m_combat_report_wnd->InitSizeMove(combat_log_ul,  combat_log_ul + combat_log_wh);
}

GG::Pt MapWnd::ClientUpperLeft() const
{ return UpperLeft() + GG::Pt(AppWidth(), AppHeight()); }

double MapWnd::ZoomFactor() const
{ return ZoomScaleFactor(m_zoom_steps_in); }

GG::Pt MapWnd::ScreenCoordsFromUniversePosition(double universe_x, double universe_y) const {
    GG::Pt cl_ul = ClientUpperLeft();
    GG::X x((universe_x * ZoomFactor()) + cl_ul.x);
    GG::Y y((universe_y * ZoomFactor()) + cl_ul.y);
    return GG::Pt(x, y);
}

std::pair<double, double> MapWnd::UniversePositionFromScreenCoords(GG::Pt screen_coords) const {
    GG::Pt cl_ul = ClientUpperLeft();
    double x = Value((screen_coords - cl_ul).x / ZoomFactor());
    double y = Value((screen_coords - cl_ul).y / ZoomFactor());
    return std::pair<double, double>(x, y);
}

int MapWnd::SelectedPlanetID() const
{ return m_production_wnd->SelectedPlanetID(); }

void MapWnd::GetSaveGameUIData(SaveGameUIData& data) const {
    data.map_left = Value(Left());
    data.map_top = Value(Top());
    data.map_zoom_steps_in = m_zoom_steps_in;
    data.fleets_exploring = m_fleets_exploring;
}

bool MapWnd::InProductionViewMode() const
{ return m_in_production_view_mode; }

bool MapWnd::InResearchViewMode() const
{ return m_research_wnd->Visible(); }

bool MapWnd::InDesignViewMode() const
{ return m_design_wnd->Visible(); }

ModeratorActionSetting MapWnd::GetModeratorActionSetting() const
{ return m_moderator_wnd->SelectedAction(); }

bool MapWnd::AutoEndTurnEnabled() const
{ return m_auto_end_turn; }

void MapWnd::PreRender() {
    // Save CPU / GPU activity by skipping rendering when it's not needed
    // As of this writing, the design and research screens have fully opaque backgrounds.
    if (m_design_wnd->Visible())
        return;
    if (m_research_wnd->Visible())
        return;

    GG::Wnd::PreRender();
    DeferredRefreshFleetButtons();
}

void MapWnd::Render() {
    // HACK! This is placed here so we can be sure it is executed frequently
    // (every time we render), and before we render any of the
    // FleetWnds.  It doesn't necessarily belong in MapWnd at all.
    FleetUIManager::GetFleetUIManager().CullEmptyWnds();

    // Save CPU / GPU activity by skipping rendering when it's not needed
    // As of this writing, the design and research screens have fully opaque backgrounds.
    if (m_design_wnd->Visible())
        return;
    if (m_research_wnd->Visible())
        return;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    GG::Pt origin_offset = UpperLeft() + GG::Pt(AppWidth(), AppHeight());
    glTranslatef(Value(origin_offset.x), Value(origin_offset.y), 0.0f);
    glScalef(ZoomFactor(), ZoomFactor(), 1.0f);

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    RenderStarfields();
    RenderGalaxyGas();
    RenderVisibilityRadii();
    RenderFields();
    RenderSystemOverlays();
    RenderStarlanes();
    RenderSystems();
    RenderFleetMovementLines();

    glPopClientAttrib();
    glPopMatrix();
}

void MapWnd::RenderStarfields() {
    if (!GetOptionsDB().Get<bool>("ui.map.background.starfields.shown"))
        return;

    double starfield_width = GetUniverse().UniverseWidth();
    if (m_starfield_verts.empty()) {
        ClearStarfieldRenderingBuffers();
        Seed(static_cast<int>(starfield_width));
        m_starfield_colours.clear();
        std::size_t NUM_STARS = std::pow(2.0, 12.0);

        m_starfield_verts.reserve(NUM_STARS);
        m_starfield_colours.reserve(NUM_STARS);
        for (std::size_t i = 0; i < NUM_STARS; ++i) {
            float x = RandGaussian(starfield_width/2, starfield_width/3);   // RandDouble(0, starfield_width);
            float y = RandGaussian(starfield_width/2, starfield_width/3);   // RandDouble(0, starfield_width);
            float r2 = (x - starfield_width/2)*(x - starfield_width/2) + (y-starfield_width/2)*(y-starfield_width/2);
            float z = RandDouble(-100, 100)*std::exp(-r2/(starfield_width*starfield_width/4));
            m_starfield_verts.store(x, y, z);

            float brightness = 1.0f - std::pow(RandZeroToOne(), 2);
            m_starfield_colours.store(GG::CLR_WHITE * brightness);
        }
        m_starfield_verts.createServerBuffer();
        m_starfield_colours.createServerBuffer();
    }


    GLfloat window_width = Value(AppWidth());
    GLfloat window_height = std::max(1, Value(AppHeight()));
    glViewport(0, 0, window_width, window_height);

    bool perspective_starfield = true;
    GLfloat zpos_1to1 = 0.0f;
    if (perspective_starfield) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        GLfloat aspect_ratio = window_width / window_height;
        GLfloat zclip_near = window_height/4;
        GLfloat zclip_far = 3*zclip_near;
        GLfloat fov_height = zclip_near;
        GLfloat fov_width = fov_height * aspect_ratio;
        zpos_1to1 = -2*zclip_near;  // stars rendered at -viewport_size units should be 1:1 with ortho projected stuff

        glFrustum(-fov_width,   fov_width,
                   fov_height, -fov_height,
                   zclip_near,  zclip_far);
        glTranslatef(-window_width/2, -window_height/2, 0.0f);
    }


    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // last: standard map panning
    GG::Pt origin_offset = UpperLeft() + GG::Pt(AppWidth(), AppHeight());
    glTranslatef(Value(origin_offset.x), Value(origin_offset.y), 0.0f);
    glScalef(ZoomFactor(), ZoomFactor(), 1.0f);
    glTranslatef(0.0, 0.0, zpos_1to1);
    glScalef(1.0f, 1.0f, ZoomFactor());

    // first: starfield manipulations
    bool rotate_starfield = false;
    if (rotate_starfield) {
        float ticks = GG::GUI::GetGUI()->Ticks();
        glTranslatef(starfield_width/2, starfield_width/2, 0.0f);   // move back to original position
        glRotatef(ticks/10, 0.0f, 0.0f, 1.0f);                      // rotate about centre of starfield
        glTranslatef(-starfield_width/2, -starfield_width/2, 0.0f); // move centre of starfield to origin
    }


    glPointSize(std::min(5.0, 0.5 * ZoomFactor()));
    glEnable(GL_POINT_SMOOTH);
    glDisable(GL_TEXTURE_2D);

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    m_starfield_verts.activate();
    m_starfield_colours.activate();
    glDrawArrays(GL_POINTS, 0, m_starfield_verts.size());

    glPopClientAttrib();

    glEnable(GL_TEXTURE_2D);
    glPointSize(1.0f);


    if (perspective_starfield) {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }


    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glViewport(0, 0, window_width, window_height);
}

void MapWnd::RenderFields() {
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // render not visible fields
    for (auto& field_buffer : m_field_vertices) {
        if (field_buffer.second.second.empty())
            continue;

        glBindTexture(GL_TEXTURE_2D, field_buffer.first->OpenGLId());
        field_buffer.second.second.activate();
        m_field_texture_coords.activate();
        glDrawArrays(GL_QUADS, 0, field_buffer.second.second.size());
    }

    // if any, render scanlines over not-visible fields
    if (!m_field_scanline_circles.empty()
        && HumanClientApp::GetApp()->EmpireID() != ALL_EMPIRES
        && GetOptionsDB().Get<bool>("ui.map.scanlines.shown"))
    {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        m_field_scanline_circles.activate();
        glBindTexture(GL_TEXTURE_2D, 0);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        m_scanline_shader.SetColor(GetOptionsDB().Get<GG::Clr>("ui.map.field.scanlines.color"));
        m_scanline_shader.StartUsing();

        glDrawArrays(GL_TRIANGLES, 0, m_field_scanline_circles.size());

        m_scanline_shader.StopUsing();
        /*glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }


    // render visible fields
    for (auto& field_buffer : m_field_vertices) {
        if (field_buffer.second.first.empty())
            continue;

        glBindTexture(GL_TEXTURE_2D, field_buffer.first->OpenGLId());
        field_buffer.second.first.activate();
        m_field_texture_coords.activate();
        glDrawArrays(GL_QUADS, 0, field_buffer.second.first.size());
    }


    glPopClientAttrib();
}

namespace {
    std::shared_ptr<GG::Texture> GetGasTexture() {
        static std::shared_ptr<GG::Texture> gas_texture;
        if (!gas_texture) {
            gas_texture = ClientUI::GetClientUI()->GetTexture(ClientUI::ArtDir() / "galaxy_decoration" / "gaseous_array.png");
            gas_texture->SetFilters(GL_NEAREST, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, gas_texture->OpenGLId());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        }
        return gas_texture;
    }
}

void MapWnd::RenderGalaxyGas() {
    if (!GetOptionsDB().Get<bool>("ui.map.background.gas.shown"))
        return;
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    if (m_galaxy_gas_quad_vertices.empty())
        return;

    glEnable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    m_galaxy_gas_quad_vertices.activate();
    m_galaxy_gas_texture_coords.activate();

    glBindTexture(GL_TEXTURE_2D, GetGasTexture()->OpenGLId());
    glDrawArrays(GL_QUADS, 0, m_galaxy_gas_quad_vertices.size());

    glPopClientAttrib();
}

void MapWnd::RenderSystemOverlays() {
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glLoadIdentity();
    for (auto& system_icon : m_system_icons)
    { system_icon.second->RenderOverlay(ZoomFactor()); }
    glPopMatrix();
}

void MapWnd::RenderSystems() {
    const float HALO_SCALE_FACTOR = static_cast<float>(SystemHaloScaleFactor());
    int empire_id = HumanClientApp::GetApp()->EmpireID();

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    if (0.5f < HALO_SCALE_FACTOR && m_star_texture_coords.size()) {
        glMatrixMode(GL_TEXTURE);
        glTranslatef(0.5f, 0.5f, 0.0f);
        glScalef(1.0f / HALO_SCALE_FACTOR, 1.0f / HALO_SCALE_FACTOR, 1.0f);
        glTranslatef(-0.5f, -0.5f, 0.0f);
        m_star_texture_coords.activate();
        for (auto& star_halo_buffer : m_star_halo_quad_vertices) {
            if (!star_halo_buffer.second.size())
                continue;
            glBindTexture(GL_TEXTURE_2D, star_halo_buffer.first->OpenGLId());
            star_halo_buffer.second.activate();
            glDrawArrays(GL_QUADS, 0, star_halo_buffer.second.size());
        }
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
    }

    if (m_star_texture_coords.size() &&
        ClientUI::SystemTinyIconSizeThreshold() < ZoomFactor() * ClientUI::SystemIconSize())
    {
        m_star_texture_coords.activate();
        for (auto& star_core_buffer : m_star_core_quad_vertices) {
            if (!star_core_buffer.second.size())
                continue;
            glBindTexture(GL_TEXTURE_2D, star_core_buffer.first->OpenGLId());
            star_core_buffer.second.activate();
            glDrawArrays(GL_QUADS, 0, star_core_buffer.second.size());
        }
    }

    // circles around system icons and fog over unexplored systems
    bool circles = GetOptionsDB().Get<bool>("ui.map.system.circle.shown");
    bool fog_scanlines = false;
    Universe& universe = GetUniverse();

    if (empire_id != ALL_EMPIRES && GetOptionsDB().Get<bool>("ui.map.scanlines.shown"))
        fog_scanlines = true;

    RenderScaleCircle();

    if (fog_scanlines || circles) {
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_LINE_SMOOTH);

        // distance between inner and outer system circle
        const double circle_distance = GetOptionsDB().Get<double>("ui.map.system.circle.distance");
        // width of outer...
        const double outer_circle_width = GetOptionsDB().Get<double>("ui.map.system.circle.outer.width");
        // ... and inner circle line at close zoom
        const double inner_circle_width = GetOptionsDB().Get<double>("ui.map.system.circle.inner.width");
        // width of inner circle line when map is zoomed out
        const double max_inner_circle_width = GetOptionsDB().Get<double>("ui.map.system.circle.inner.max.width");

        for (const auto& system_icon : m_system_icons) {
            const auto& icon = system_icon.second;

            GG::Pt icon_size = icon->LowerRight() - icon->UpperLeft();
            GG::Pt icon_middle = icon->UpperLeft() + (icon_size / 2);

            GG::Pt circle_size = GG::Pt(static_cast<GG::X>(icon->EnclosingCircleDiameter()),
                                        static_cast<GG::Y>(icon->EnclosingCircleDiameter()));

            GG::Pt circle_ul = icon_middle - (circle_size / 2);
            GG::Pt circle_lr = circle_ul + circle_size;

            GG::Pt circle_distance_pt = GG::Pt(GG::X1, GG::Y1) * circle_distance;

            GG::Pt inner_circle_ul = circle_ul + (circle_distance_pt * ZoomFactor());
            GG::Pt inner_circle_lr = circle_lr - (circle_distance_pt * ZoomFactor());

            if (fog_scanlines
                && (universe.GetObjectVisibilityByEmpire(system_icon.first, empire_id) <= VIS_BASIC_VISIBILITY))
            {
                m_scanline_shader.SetColor(GetOptionsDB().Get<GG::Clr>("ui.map.system.scanlines.color"));
                m_scanline_shader.RenderCircle(circle_ul, circle_lr);
            }

            // render circles around systems that have at least one starlane, if they are enabled
            if (!circles) continue;

            if (auto system = GetSystem(system_icon.first)) {
                if (system->NumStarlanes() > 0) {
                    bool has_empire_planet = false;
                    bool has_neutrals = false;
                    std::map<int, int> colony_count_by_empire_id;
                    const std::set<int>& known_destroyed_object_ids = GetUniverse().EmpireKnownDestroyedObjectIDs(HumanClientApp::GetApp()->EmpireID());

                    for (auto& planet : Objects().FindObjects<const Planet>(system->PlanetIDs())) {
                        if (known_destroyed_object_ids.count(planet->ID()) > 0)
                            continue;

                        // remember if this system has a player-owned planet, count # of colonies for each empire
                        if (!planet->Unowned()) {
                            has_empire_planet = true;

                            std::map<int, int>::iterator it = colony_count_by_empire_id.find(planet->Owner()) ;
                            if (it != colony_count_by_empire_id.end())
                                it->second++;
                            else
                                colony_count_by_empire_id.insert({planet->Owner(), 1});
                        }

                        // remember if this system has neutrals
                        if (planet->Unowned() && !planet->SpeciesName().empty() && planet->InitialMeterValue(METER_POPULATION) > 0.0) {
                            has_neutrals = true;

                            std::map<int, int>::iterator it = colony_count_by_empire_id.find(ALL_EMPIRES);
                            if (it != colony_count_by_empire_id.end())
                                it->second++;
                            else
                                colony_count_by_empire_id.insert({ALL_EMPIRES, 1});
                        }
                    }

                    // draw outer circle in color of supplying empire
                    int supply_empire_id = GetSupplyManager().EmpireThatCanSupplyAt(system_icon.first);
                    if (supply_empire_id != ALL_EMPIRES) {
                        if (const Empire* empire = GetEmpire(supply_empire_id))
                            glColor(empire->Color());
                        else
                            ErrorLogger() << "MapWnd::RenderSystems(): could not load empire with id " << supply_empire_id;
                    } else
                        glColor(GetOptionsDB().Get<GG::Clr>("ui.map.starlane.color"));

                    glLineWidth(outer_circle_width);
                    CircleArc(circle_ul, circle_lr, 0.0, TWO_PI, false);

                    // systems with neutrals and no empire have a segmented inner circle
                    if (has_neutrals && !(has_empire_planet)) {
                        float line_width = std::max(std::min(2 / ZoomFactor(), max_inner_circle_width), inner_circle_width);
                        glLineWidth(line_width);
                        glColor(ClientUI::TextColor());

                        float segment = static_cast<float>(TWO_PI) / 24.0f;
                        for (int n = 0; n < 24; n = n + 2)
                            CircleArc(inner_circle_ul, inner_circle_lr, n * segment, (n+1) * segment, false);
                    }

                    // systems with empire planets have an unbroken inner circle; color segments for each empire present
                    if (!has_empire_planet) continue;

                    float line_width = std::max(std::min(2 / ZoomFactor(), max_inner_circle_width), inner_circle_width);
                    glLineWidth(line_width);

                    int colonised_planets = 0;
                    int position = 0;

                    for (std::pair<int, int> it : colony_count_by_empire_id)
                        colonised_planets += it.second;

                    float segment = static_cast<float>(TWO_PI) / colonised_planets;

                    for (std::pair<int, int> it : colony_count_by_empire_id) {
                        if (const Empire* empire = GetEmpire(it.first))
                            glColor(empire->Color());
                        else
                            glColor(ClientUI::TextColor());

                        CircleArc(inner_circle_ul, inner_circle_lr, position * segment, (it.second + position) * segment, false);
                        position += it.second;
                    }
                }
            }
        }

        glDisable(GL_LINE_SMOOTH);
        glEnable(GL_TEXTURE_2D);
        glPopMatrix();
        glLineWidth(1.0f);
    }

    glPopClientAttrib();
}

void MapWnd::RenderStarlanes() {
    // if lanes everywhere, don't render the lanes
    if (GetGameRules().Get<bool>("RULE_STARLANES_EVERYWHERE"))
        return;

    bool coloured = GetOptionsDB().Get<bool>("ui.map.starlane.empire.color.shown");
    float core_multiplier = static_cast<float>(GetOptionsDB().Get<double>("ui.map.starlane.thickness.factor"));
    RenderStarlanes(m_RC_starlane_vertices, m_RC_starlane_colors, core_multiplier * ZoomFactor(), coloured, false);
    RenderStarlanes(m_starlane_vertices, m_starlane_colors, 1.0, coloured, true);
}

void MapWnd::RenderStarlanes(GG::GL2DVertexBuffer& vertices, GG::GLRGBAColorBuffer& colours,
                             double thickness, bool coloured, bool do_base_render) {
    if (vertices.size() && (colours.size() || !coloured) && (coloured || do_base_render)) {
        // render starlanes with vertex buffer (and possibly colour buffer)
        const GG::Clr UNOWNED_LANE_COLOUR = GetOptionsDB().Get<GG::Clr>("ui.map.starlane.color");

        glDisable(GL_TEXTURE_2D);
        glEnable(GL_LINE_SMOOTH);

        glLineWidth(static_cast<GLfloat>(thickness * GetOptionsDB().Get<double>("ui.map.starlane.thickness")));

        glPushAttrib(GL_COLOR_BUFFER_BIT);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        if (coloured) {
            glEnableClientState(GL_COLOR_ARRAY);
            colours.activate();
        } else {
            glDisableClientState(GL_COLOR_ARRAY);
            glColor(UNOWNED_LANE_COLOUR);
        }
        vertices.activate();

        glDrawArrays(GL_LINES, 0, vertices.size());

        glLineWidth(1.0);

        glPopClientAttrib();
        glPopAttrib();

        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LINE_SMOOTH);
    }

    glLineWidth(1.0);
}

namespace {
    GG::GL2DVertexBuffer dot_vertices_buffer;
    GG::GLTexCoordBuffer dot_star_texture_coords;
    const unsigned int BUFFER_CAPACITY(512);    // should be long enough for most plausible fleet move lines

    std::shared_ptr<GG::Texture> MoveLineDotTexture() {
        auto retval = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "move_line_dot.png");
        return retval;
    }
}

void MapWnd::RenderFleetMovementLines() {
    if (ZoomFactor() < ClientUI::TinyFleetButtonZoomThreshold())
        return;

    // determine animation shift for move lines
    int dot_spacing = GetOptionsDB().Get<int>("ui.map.fleet.supply.dot.spacing");
    float rate = static_cast<float>(GetOptionsDB().Get<double>("ui.map.fleet.supply.dot.rate"));
    int ticks = GG::GUI::GetGUI()->Ticks();
    /* Updated each frame to shift rendered posistion of dots that are drawn to
     * show fleet move lines. */
    float move_line_animation_shift = static_cast<int>(ticks * rate) % dot_spacing;

    // texture for dots
    auto move_line_dot_texture = MoveLineDotTexture();
    float dot_size = Value(move_line_dot_texture->DefaultWidth());
    //std::cout << "dot size: " << dot_size << std::endl;

    // texture coords
    if (dot_star_texture_coords.empty()) {
        dot_star_texture_coords.reserve(BUFFER_CAPACITY*4);
        for (std::size_t i = 0; i < BUFFER_CAPACITY; ++i) {
            dot_star_texture_coords.store(0.0f, 0.0f);
            dot_star_texture_coords.store(0.0f, 1.0f);
            dot_star_texture_coords.store(1.0f, 1.0f);
            dot_star_texture_coords.store(1.0f, 0.0f);
        }
    }


    // dots rendered same size for all zoom levels, so do positioning in screen
    // space instead of universe space
    glPushMatrix();
    glLoadIdentity();

    // render movement lines for all fleets
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindTexture(GL_TEXTURE_2D, move_line_dot_texture->OpenGLId());
    for (const auto& fleet_line : m_fleet_lines)
    { RenderMovementLine(fleet_line.second, dot_size, dot_spacing, move_line_animation_shift); }

    // re-render selected fleets' movement lines in white
    for (int fleet_id : m_selected_fleet_ids) {
        auto line_it = m_fleet_lines.find(fleet_id);
        if (line_it != m_fleet_lines.end())
            RenderMovementLine(line_it->second, dot_size, dot_spacing, move_line_animation_shift, GG::CLR_WHITE);
    }

    // render move line ETA indicators for selected fleets
    for (int fleet_id : m_selected_fleet_ids) {
        auto line_it = m_fleet_lines.find(fleet_id);
        if (line_it != m_fleet_lines.end())
            RenderMovementLineETAIndicators(line_it->second);
    }

    // render projected move lines
    glBindTexture(GL_TEXTURE_2D, move_line_dot_texture->OpenGLId());
    for (const auto& fleet_line : m_projected_fleet_lines)
    { RenderMovementLine(fleet_line.second, dot_size, dot_spacing, move_line_animation_shift, GG::CLR_WHITE); }

    // render projected move line ETA indicators
    for (const auto& eta_indicator : m_projected_fleet_lines)
    { RenderMovementLineETAIndicators(eta_indicator.second, GG::CLR_WHITE); }

    glPopClientAttrib();
    glPopMatrix();
}

void MapWnd::RenderMovementLine(const MapWnd::MovementLineData& move_line, float dot_size,
                                float dot_spacing, float dot_shift, GG::Clr clr)
{
    // assumes:
    // - dot texture has already been bound
    // - identity matrix has been loaded
    // - vertex array and texture coord array client states ahve been enabled

    const auto& vertices = move_line.vertices;
    if (vertices.empty())
        return; // nothing to draw.  need at least two nodes at different locations to draw a line
    if (vertices.size() % 2 == 1) {
        //ErrorLogger() << "RenderMovementLine given an odd number of vertices (" << vertices.size() << ") to render?!";
        return;
    }

    // if no override colour specified, use line's own colour info
    if (clr == GG::CLR_ZERO)
        glColor(move_line.colour);
    else
        glColor(clr);

    float dot_half_sz = dot_size / 2.0f;
    float offset = dot_shift;  // step along line in by move_line_animation_shift to get position of first dot


    // movement line data changes every frame, so no use for a server buffer...
    // so fill a client buffer each frame with latest vertex data for this line.
    // however, want to avoid extra reallocations of buffer, so reserve space.
    dot_vertices_buffer.clear();
    dot_vertices_buffer.reserve(BUFFER_CAPACITY*4);

    unsigned int dots_added_to_buffer = 0;

    // set vertex positions to outline a quad for each move line vertex
    for (auto verts_it = vertices.begin(); verts_it != vertices.end(); ++verts_it) {
        if (dots_added_to_buffer >= BUFFER_CAPACITY)
            break; // can't fit any more!

        // get next two vertices
        const auto& vert1 = *verts_it;
        ++verts_it;
        const auto& vert2 = *verts_it;

        // find centres of dots on screen
        GG::Pt vert1Pt = ScreenCoordsFromUniversePosition(vert1.x, vert1.y);
        GG::Pt vert2Pt = ScreenCoordsFromUniversePosition(vert2.x, vert2.y);

        // get unit vector along line connecting vertices
        float deltaX = Value(vert2Pt.x - vert1Pt.x);
        float deltaY = Value(vert2Pt.y - vert1Pt.y);
        float length = std::sqrt(deltaX*deltaX + deltaY*deltaY);
        if (length == 0.0f) // safety check
            length = 1.0f;
        float uVecX = deltaX / length;
        float uVecY = deltaY / length;

        // increment along line, adding dots to buffers, until end of line segment is passed
        while (offset < length && dots_added_to_buffer < BUFFER_CAPACITY) {
            ++dots_added_to_buffer;

            // don't know why the dot needs to be shifted half a dot size down/right and
            // rendered 2 x dot size on each axis, but apparently it does...

            // find position of dot from initial vertex position, offset length and unit vectors
            std::pair<float, float> ul(Value(vert1Pt.x) + offset * uVecX + dot_half_sz,
                                       Value(vert1Pt.y) + offset * uVecY + dot_half_sz);

            dot_vertices_buffer.store(ul.first - dot_size,   ul.second - dot_size);
            dot_vertices_buffer.store(ul.first - dot_size,   ul.second + dot_size);
            dot_vertices_buffer.store(ul.first + dot_size,   ul.second + dot_size);
            dot_vertices_buffer.store(ul.first + dot_size,   ul.second - dot_size);

            // move offset to that for next dot
            offset += dot_spacing;
        }

        offset -= length;   // so next segment's dots meld smoothly into this segment's
    }

    // after adding all dots to buffer, render in one call
    dot_vertices_buffer.activate();
    dot_star_texture_coords.activate();
    glDrawArrays(GL_QUADS, 0, dot_vertices_buffer.size());
    //std::cout << "dot verts buffer size: " << dot_vertices_buffer.size() << std::endl;
}

void MapWnd::RenderMovementLineETAIndicators(const MapWnd::MovementLineData& move_line,
                                             GG::Clr clr)
{
    const auto& vertices = move_line.vertices;
    if (vertices.empty())
        return; // nothing to draw.


    const double MARKER_HALF_SIZE = 9;
    const int MARKER_PTS = ClientUI::Pts();
    auto font = ClientUI::GetBoldFont(MARKER_PTS);
    auto flags = GG::FORMAT_CENTER | GG::FORMAT_VCENTER;

    glPushMatrix();
    glLoadIdentity();
    int flag_border = 5;

    for (const auto& vert : vertices) {
        if (!vert.show_eta)
            continue;

        // draw background disc in empire colour, or passed-in colour
        GG::Pt marker_centre = ScreenCoordsFromUniversePosition(vert.x, vert.y);
        GG::Pt ul = marker_centre - GG::Pt(GG::X(static_cast<int>(MARKER_HALF_SIZE)),
                                           GG::Y(static_cast<int>(MARKER_HALF_SIZE)));
        GG::Pt lr = marker_centre + GG::Pt(GG::X(static_cast<int>(MARKER_HALF_SIZE)),
                                           GG::Y(static_cast<int>(MARKER_HALF_SIZE)));

        glDisable(GL_TEXTURE_2D);

        // segmented circle of wedges to indicate blockades
        if (vert.flag_blockade) {
            float wedge = static_cast<float>(TWO_PI)/12.0f;
            for (int n = 0; n < 12; n = n + 2) {
                glColor(GG::CLR_BLACK);
                CircleArc(ul + GG::Pt(-flag_border*GG::X1,      -flag_border*GG::Y1),   lr + GG::Pt(flag_border*GG::X1,     flag_border*GG::Y1),    n*wedge,        (n+1)*wedge, true);
                glColor(GG::CLR_RED);
                CircleArc(ul + GG::Pt(-(flag_border)*GG::X1,    -(flag_border)*GG::Y1), lr + GG::Pt((flag_border)*GG::X1,   (flag_border)*GG::Y1),  (n+1)*wedge,    (n+2)*wedge, true);
            }
        } else if (vert.flag_supply_block) {
            float wedge = static_cast<float>(TWO_PI)/12.0f;
            for (int n = 0; n < 12; n = n + 2) {
                glColor(GG::CLR_BLACK);
                CircleArc(ul + GG::Pt(-flag_border*GG::X1,      -flag_border*GG::Y1),   lr + GG::Pt(flag_border*GG::X1,     flag_border*GG::Y1),    n*wedge,        (n+1)*wedge, true);
                glColor(GG::CLR_YELLOW);
                CircleArc(ul + GG::Pt(-(flag_border)*GG::X1,    -(flag_border)*GG::Y1), lr + GG::Pt((flag_border)*GG::X1,   (flag_border)*GG::Y1),  (n+1)*wedge,    (n+2)*wedge, true);
            }
        }


        // empire-coloured central fill within wedged outer ring
        if (clr == GG::CLR_ZERO)
            glColor(move_line.colour);
        else
            glColor(clr);

        CircleArc(ul, lr, 0.0, TWO_PI, true);
        glEnable(GL_TEXTURE_2D);


        // render ETA number in white with black shadows
        std::string text = "<s>" + std::to_string(vert.eta) + "</s>";
        glColor(GG::CLR_WHITE);
        // TODO cache the text_elements
        auto text_elements = font->ExpensiveParseFromTextToTextElements(text, flags);
        auto lines = font->DetermineLines(text, flags, lr.x - ul.x, text_elements);
        font->RenderText(ul, lr, text, flags, lines);
    }
    glPopMatrix();
}

void MapWnd::RenderVisibilityRadii() {
    if (!GetOptionsDB().Get<bool>("ui.map.detection.range.shown"))
        return;

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glPushAttrib(GL_ENABLE_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);

    glEnable(GL_LINE_SMOOTH);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.5f);

    // render each colour's radii separately, so they can consistently blend
    // when overlapping other colours, but be stenciled to avoid blending
    // when overlapping within a colour
    for (unsigned int i = 0; i < m_radii_radii_vertices_indices_runs.size(); ++i) {
        const auto& radii_start_run = m_radii_radii_vertices_indices_runs[i].first;
        const auto& border_start_run = m_radii_radii_vertices_indices_runs[i].second;

        glClear(GL_STENCIL_BUFFER_BIT);
        glStencilOp(GL_INCR, GL_INCR, GL_INCR);
        glStencilFunc(GL_EQUAL, 0x0, 0xff);

        m_visibility_radii_vertices.activate();
        m_visibility_radii_colors.activate();
        glDrawArrays(GL_TRIANGLES, radii_start_run.first, radii_start_run.second);

        glStencilFunc(GL_GREATER, 0x2, 0xff);
        glStencilOp(GL_DECR, GL_KEEP, GL_KEEP);

        m_visibility_radii_border_vertices.activate();
        m_visibility_radii_border_colors.activate();

        glDrawArrays(GL_LINES, border_start_run.first, border_start_run.second);
    }

    glEnable(GL_TEXTURE_2D);
    glLineWidth(1.0f);
    glPopAttrib();
    glPopClientAttrib();
}

void MapWnd::RenderScaleCircle() {
    if (SidePanel::SystemID() == INVALID_OBJECT_ID)
        return;
    if (!GetOptionsDB().Get<bool>("ui.map.scale.legend.shown") || !GetOptionsDB().Get<bool>("ui.map.scale.circle.shown"))
        return;
    if (m_scale_circle_vertices.empty())
        InitScaleCircleRenderingBuffer();

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnable(GL_LINE_SMOOTH);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(2.0f);

    GG::Clr circle_colour = GG::CLR_WHITE;
    circle_colour.a = 128;
    glColor(circle_colour);

    m_scale_circle_vertices.activate();
    glDrawArrays(GL_LINE_STRIP, 0, m_scale_circle_vertices.size());

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);

    glPopClientAttrib();
}

void MapWnd::RegisterWindows() {
    // TODO: move these wnds into a GG::Wnd and call parent_wnd->Show(false) to
    //       hide all windows instead of unregistering them all.
    // Actually register these CUIWnds so that the Visible() ones are rendered.
    if (HumanClientApp* app = HumanClientApp::GetApp()) {
        app->Register(m_sitrep_panel);
        app->Register(m_object_list_wnd);
        app->Register(m_pedia_panel);
        app->Register(m_side_panel);
        app->Register(m_combat_report_wnd);
        app->Register(m_moderator_wnd);
        // message and player list wnds are managed by the HumanClientFSM
    }
}

void MapWnd::RemoveWindows() {
    // Hide windows by unregistering them which works regardless of their
    // m_visible attribute.
    if (HumanClientApp* app = HumanClientApp::GetApp()) {
        app->Remove(m_sitrep_panel);
        app->Remove(m_object_list_wnd);
        app->Remove(m_pedia_panel);
        app->Remove(m_side_panel);
        app->Remove(m_combat_report_wnd);
        app->Remove(m_moderator_wnd);
        // message and player list wnds are managed by the HumanClientFSM
    }
}

void MapWnd::Pan(const GG::Pt& delta) {
    GG::Pt move_to_pt = ClientUpperLeft() + delta;
    CorrectMapPosition(move_to_pt);
    MoveTo(move_to_pt - GG::Pt(AppWidth(), AppHeight()));
}

bool MapWnd::PanX(GG::X x) {
    Pan(GG::Pt(x, GG::Y0));
    return true;
}

bool MapWnd::PanY(GG::Y y) {
    Pan(GG::Pt(GG::X0, y));
    return true;
}

void MapWnd::LButtonDown(const GG::Pt &pt, GG::Flags<GG::ModKey> mod_keys)
{ m_drag_offset = pt - ClientUpperLeft(); }

void MapWnd::LDrag(const GG::Pt &pt, const GG::Pt &move, GG::Flags<GG::ModKey> mod_keys) {
    GG::Pt move_to_pt = pt - m_drag_offset;
    CorrectMapPosition(move_to_pt);

    MoveTo(move_to_pt - GG::Pt(AppWidth(), AppHeight()));
    m_dragged = true;
}

void MapWnd::LButtonUp(const GG::Pt &pt, GG::Flags<GG::ModKey> mod_keys) {
    m_drag_offset = GG::Pt(-GG::X1, -GG::Y1);
    m_dragged = false;
}

void MapWnd::LClick(const GG::Pt &pt, GG::Flags<GG::ModKey> mod_keys) {
    m_drag_offset = GG::Pt(-GG::X1, -GG::Y1);
    FleetUIManager& manager = FleetUIManager::GetFleetUIManager();
    const auto fleet_wnd = manager.ActiveFleetWnd();
    bool quick_close_wnds = GetOptionsDB().Get<bool>("ui.quickclose.enabled");

    // if a fleet window is visible, hide it and deselect fleet; if not, hide sidepanel
    if (!m_dragged && !m_in_production_view_mode && fleet_wnd && quick_close_wnds) {
        manager.CloseAll();
    } else if (!m_dragged && !m_in_production_view_mode) {
        SelectSystem(INVALID_OBJECT_ID);
        m_side_panel->Hide();
    }
    m_dragged = false;
}

void MapWnd::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    // if in moderator mode, treat as moderator action click
    if (ClientPlayerIsModerator()) {
        // only supported action on empty map location at present is creating a system
        if (m_moderator_wnd->SelectedAction() == MAS_CreateSystem) {
            ClientNetworking& net = HumanClientApp::GetApp()->Networking();
            auto u_pos = this->UniversePositionFromScreenCoords(pt);
            StarType star_type = m_moderator_wnd->SelectedStarType();
            net.SendMessage(ModeratorActionMessage(
                Moderator::CreateSystem(u_pos.first, u_pos.second, star_type)));
            return;
        }
    }

    if (GetOptionsDB().Get<bool>("ui.map.menu.enabled")) {
        // create popup menu with map options in it.
        bool fps            = GetOptionsDB().Get<bool>("video.fps.shown");
        bool showPlanets    = GetOptionsDB().Get<bool>("ui.map.sidepanel.planet.shown");
        bool systemCircles  = GetOptionsDB().Get<bool>("ui.map.system.circle.shown");
        bool resourceColor  = GetOptionsDB().Get<bool>("ui.map.starlane.empire.color.shown");
        bool fleetSupply    = GetOptionsDB().Get<bool>("ui.map.fleet.supply.shown");
        bool gas            = GetOptionsDB().Get<bool>("ui.map.background.gas.shown");
        bool starfields     = GetOptionsDB().Get<bool>("ui.map.background.starfields.shown");
        bool scale          = GetOptionsDB().Get<bool>("ui.map.scale.legend.shown");
        bool scaleCircle    = GetOptionsDB().Get<bool>("ui.map.scale.circle.shown");
        bool zoomSlider     = GetOptionsDB().Get<bool>("ui.map.zoom.slider.shown");
        bool detectionRange = GetOptionsDB().Get<bool>("ui.map.detection.range.shown");

        auto show_fps_action        = [&fps]()            { GetOptionsDB().Set<bool>("video.fps.shown",                !fps);         };
        auto show_planets_action    = [&showPlanets]()    { GetOptionsDB().Set<bool>("ui.map.sidepanel.planet.shown",       !showPlanets);      };
        auto system_circles_action  = [&systemCircles]()  { GetOptionsDB().Set<bool>("ui.map.system.circle.shown",          !systemCircles);    };
        auto resource_color_action  = [&resourceColor]()  { GetOptionsDB().Set<bool>("ui.map.starlane.empire.color.shown",  !resourceColor);    };
        auto fleet_supply_action    = [&fleetSupply]()    { GetOptionsDB().Set<bool>("ui.map.fleet.supply.shown",      !fleetSupply);      };
        auto gas_action             = [&gas]()            { GetOptionsDB().Set<bool>("ui.map.background.gas.shown",         !gas);              };
        auto starfield_action       = [&starfields]()     { GetOptionsDB().Set<bool>("ui.map.background.starfields.shown",  !starfields); };
        auto map_scale_action       = [&scale]()          { GetOptionsDB().Set<bool>("ui.map.scale.legend.shown",           !scale);            };
        auto scale_circle_action    = [&scaleCircle]()    { GetOptionsDB().Set<bool>("ui.map.scale.circle.shown",           !scaleCircle);      };
        auto zoom_slider_action     = [&zoomSlider]()     { GetOptionsDB().Set<bool>("ui.map.zoom.slider.shown",            !zoomSlider);       };
        auto detection_range_action = [&detectionRange]() { GetOptionsDB().Set<bool>("ui.map.detection.range.shown",        !detectionRange);   };

        auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
        popup->AddMenuItem(GG::MenuItem(UserString("OPTIONS_SHOW_FPS"),                     false, fps,            show_fps_action));
        popup->AddMenuItem(GG::MenuItem(UserString("OPTIONS_SHOW_SIDEPANEL_PLANETS"),       false, showPlanets,    show_planets_action));
        popup->AddMenuItem(GG::MenuItem(UserString("OPTIONS_UI_SYSTEM_CIRCLES"),            false, systemCircles,  system_circles_action));
        popup->AddMenuItem(GG::MenuItem(UserString("OPTIONS_RESOURCE_STARLANE_COLOURING"),  false, resourceColor,  resource_color_action));
        popup->AddMenuItem(GG::MenuItem(UserString("OPTIONS_FLEET_SUPPLY_LINES"),           false, fleetSupply,    fleet_supply_action));
        popup->AddMenuItem(GG::MenuItem(UserString("OPTIONS_GALAXY_MAP_GAS"),               false, gas,            gas_action));
        popup->AddMenuItem(GG::MenuItem(UserString("OPTIONS_GALAXY_MAP_STARFIELDS"),        false, starfields,     starfield_action));
        popup->AddMenuItem(GG::MenuItem(UserString("OPTIONS_GALAXY_MAP_SCALE_LINE"),        false, scale,          map_scale_action));
        popup->AddMenuItem(GG::MenuItem(UserString("OPTIONS_GALAXY_MAP_SCALE_CIRCLE"),      false, scaleCircle,    scale_circle_action));
        popup->AddMenuItem(GG::MenuItem(UserString("OPTIONS_GALAXY_MAP_ZOOM_SLIDER"),       false, zoomSlider,     zoom_slider_action));
        popup->AddMenuItem(GG::MenuItem(UserString("OPTIONS_GALAXY_MAP_DETECTION_RANGE"),   false, detectionRange, detection_range_action));
        // display popup menu
        popup->Run();

    }
}

void MapWnd::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) {
    if (move)
        Zoom(move, pt);
}

void MapWnd::KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if (key == GG::GGK_LSHIFT || key == GG::GGK_RSHIFT) {
        ReplotProjectedFleetMovement(mod_keys & GG::MOD_KEY_SHIFT);
    }
}

void MapWnd::KeyRelease(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if (key == GG::GGK_LSHIFT || key == GG::GGK_RSHIFT) {
        ReplotProjectedFleetMovement(mod_keys & GG::MOD_KEY_SHIFT);
    }
}

void MapWnd::EnableOrderIssuing(bool enable/* = true*/) {
    // disallow order enabling if this client does not have an empire
    // and is not a moderator
    HumanClientApp* app = HumanClientApp::GetApp();
    bool moderator = false;
    m_btn_turn->Disable(HumanClientApp::GetApp()->SinglePlayerGame() && !enable);
    if (!app) {
        enable = false;
        m_btn_turn->Disable(true);
    } else {
        bool have_empire = (app->EmpireID() != ALL_EMPIRES);
        moderator = (app->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR);
        if (!have_empire && !moderator) {
            enable = false;
            m_btn_turn->Disable(true);
        }
    }

    m_moderator_wnd->EnableActions(enable && moderator);
    m_ready_turn = !enable;
    m_btn_turn->SetText(boost::io::str(FlexibleFormat(m_ready_turn && !HumanClientApp::GetApp()->SinglePlayerGame() ?
                                                      UserString("MAP_BTN_TURN_UNREADY") :
                                                      UserString("MAP_BTN_TURN_UPDATE")) %
                                       std::to_string(CurrentTurn())));
    m_side_panel->EnableOrderIssuing(enable);
    m_production_wnd->EnableOrderIssuing(enable);
    m_research_wnd->EnableOrderIssuing(enable);
    m_design_wnd->EnableOrderIssuing(enable);
    FleetUIManager::GetFleetUIManager().EnableOrderIssuing(enable);
}

void MapWnd::InitTurn() {
    int turn_number = CurrentTurn();
    DebugLogger() << "Initializing turn " << turn_number;
    SectionedScopedTimer timer("MapWnd::InitTurn", std::chrono::milliseconds(1));
    timer.EnterSection("init");

    //DebugLogger() << GetSupplyManager().Dump();

    Universe& universe = GetUniverse();
    ObjectMap& objects = Objects();

    TraceLogger(effects) << "MapWnd::InitTurn initial:";
    for (auto obj : objects)
        TraceLogger(effects) << obj->Dump();

    timer.EnterSection("system graph");
    // FIXME: this is actually only needed when there was no mid-turn update
    universe.InitializeSystemGraph(HumanClientApp::GetApp()->EmpireID());

    timer.EnterSection("meter estimates");
    // update effect accounting and meter estimates
    universe.InitMeterEstimatesAndDiscrepancies();

    // if we've just loaded the game there may be some unexecuted orders, we
    // should reapply them now, so they are reflected in the UI, but do not
    // influence current meters or their discrepancies for this turn
    HumanClientApp::GetApp()->Orders().ApplyOrders();

    // redo meter estimates with unowned planets marked as owned by player, so accurate predictions of planet
    // population is available for currently uncolonized planets
    GetUniverse().UpdateMeterEstimates();

    GetUniverse().ApplyAppearanceEffects();

    timer.EnterSection("rendering");
    // set up system icons, starlanes, galaxy gas rendering
    InitTurnRendering();

    timer.EnterSection("fleet signals");
    // connect system fleet add and remove signals
    for (auto& system : objects.FindObjects<System>()) {
        m_system_fleet_insert_remove_signals[system->ID()].push_back(system->FleetsInsertedSignal.connect(
            boost::bind(&MapWnd::FleetsInsertedSignalHandler, this, _1)));
        m_system_fleet_insert_remove_signals[system->ID()].push_back(system->FleetsRemovedSignal.connect(
            boost::bind(&MapWnd::FleetsRemovedSignalHandler, this, _1)));
    }

    RefreshFleetSignals();


    // set turn button to current turn
    m_btn_turn->SetText(boost::io::str(FlexibleFormat(UserString("MAP_BTN_TURN_UPDATE")) %
                                       std::to_string(turn_number)));
    m_ready_turn = false;
    MoveChildUp(m_btn_turn);


    timer.EnterSection("sitreps");
    // are there any sitreps to show?
    bool show_intro_sitreps = CurrentTurn() == 1 &&
        GetOptionsDB().Get<Aggression>("setup.ai.aggression") <= TYPICAL;
    DebugLogger() << "showing intro sitreps : " << show_intro_sitreps;
    if (show_intro_sitreps || m_sitrep_panel->NumVisibleSitrepsThisTurn() > 0) {
        m_sitrep_panel->ShowSitRepsForTurn(CurrentTurn());
        if (!m_design_wnd->Visible() && !m_research_wnd->Visible() && !m_production_wnd->Visible())
            ShowSitRep();
    }

    if (m_sitrep_panel->Visible()) {
        // Ensure that the panel is at least updated if it's visible because it
        // can now set itself to be visible (from the config) before a game is
        // loaded, and it can be visible while the production/research/design
        // windows are open.
        m_sitrep_panel->Update();
    }

    m_combat_report_wnd->Hide();

    if (m_object_list_wnd->Visible())
        m_object_list_wnd->Refresh();

    m_moderator_wnd->Refresh();
    m_pedia_panel->Refresh();


    // show or hide system names, depending on zoom.  replicates code in MapWnd::Zoom
    if (ZoomFactor() * ClientUI::Pts() < MIN_SYSTEM_NAME_SIZE)
        HideSystemNames();
    else
        ShowSystemNames();


    // empire is recreated each turn based on turn update from server, so
    // connections of signals emitted from the empire must be remade each turn
    // (unlike connections to signals from the sidepanel)
    Empire* this_client_empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
    if (this_client_empire) {
        this_client_empire->GetResourcePool(RE_TRADE)->ChangedSignal.connect(
            boost::bind(&MapWnd::RefreshTradeResourceIndicator, this));
        this_client_empire->GetResourcePool(RE_RESEARCH)->ChangedSignal.connect(
            boost::bind(&MapWnd::RefreshResearchResourceIndicator, this));
        this_client_empire->GetResourcePool(RE_INDUSTRY)->ChangedSignal.connect(
            boost::bind(&MapWnd::RefreshIndustryResourceIndicator, this));
        this_client_empire->GetPopulationPool().ChangedSignal.connect(
            boost::bind(&MapWnd::RefreshPopulationIndicator, this));
        this_client_empire->GetProductionQueue().ProductionQueueChangedSignal.connect(
            boost::bind(&MapWnd::RefreshIndustryResourceIndicator, this));
        // so lane colouring to indicate wasted PP is updated
        this_client_empire->GetProductionQueue().ProductionQueueChangedSignal.connect(
            boost::bind(&MapWnd::InitStarlaneRenderingBuffers, this));
        this_client_empire->GetResearchQueue().ResearchQueueChangedSignal.connect(
            boost::bind(&MapWnd::RefreshResearchResourceIndicator, this));
    }

    m_toolbar->Show();
    m_FPS->Show();
    m_scale_line->Show();
    RefreshSliders();


    timer.EnterSection("update resource pools");
    for (auto& entry : Empires())
        entry.second->UpdateResourcePools();


    timer.EnterSection("refresh research");
    m_research_wnd->Refresh();


    timer.EnterSection("refresh sidepanel");
    SidePanel::Refresh();       // recreate contents of all SidePanels.  ensures previous turn's objects and signals are disposed of


    timer.EnterSection("refresh production wnd");
    m_production_wnd->Refresh();


    if (turn_number == 1 && this_client_empire) {
        // start first turn with player's system selected
        if (auto obj = objects.Object(this_client_empire->CapitalID())) {
            SelectSystem(obj->SystemID());
            CenterOnMapCoord(obj->X(), obj->Y());
        }

        // default the tech tree to be centred on something interesting
        m_research_wnd->Reset();
    } else if (turn_number == 1 && !this_client_empire) {
        CenterOnMapCoord(0.0, 0.0);
    }

    timer.EnterSection("refresh indicators");
    RefreshIndustryResourceIndicator();
    RefreshResearchResourceIndicator();
    RefreshTradeResourceIndicator();
    RefreshFleetResourceIndicator();
    RefreshPopulationIndicator();
    RefreshDetectionIndicator();

    timer.EnterSection("dispatch exploring");
    FleetUIManager::GetFleetUIManager().RefreshAll();
    DispatchFleetsExploring();

    timer.EnterSection("enable observers");
    HumanClientApp* app = HumanClientApp::GetApp();
    if (app->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR) {
        // this client is a moderator
        m_btn_moderator->Disable(false);
        m_btn_moderator->Show();
    } else {
        HideModeratorActions();
        m_btn_moderator->Disable();
        m_btn_moderator->Hide();
    }
    if (app->GetClientType() == Networking::CLIENT_TYPE_HUMAN_OBSERVER) {
        m_btn_auto_turn->Disable();
        m_btn_auto_turn->Hide();
    } else {
        m_btn_auto_turn->Disable(false);
        m_btn_auto_turn->Show();
    }

    if (GetOptionsDB().Get<bool>("ui.turn.start.sound.enabled"))
        Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.turn.start.sound.path"), true);
}

void MapWnd::MidTurnUpdate() {
    DebugLogger() << "MapWnd::MidTurnUpdate";
    ScopedTimer timer("MapWnd::MidTurnUpdate", true);

    GetUniverse().InitializeSystemGraph(HumanClientApp::GetApp()->EmpireID());

    // set up system icons, starlanes, galaxy gas rendering
    InitTurnRendering();

    FleetUIManager::GetFleetUIManager().RefreshAll();
    SidePanel::Refresh();

    // show or hide system names, depending on zoom.  replicates code in MapWnd::Zoom
    if (ZoomFactor() * ClientUI::Pts() < MIN_SYSTEM_NAME_SIZE)
        HideSystemNames();
    else
        ShowSystemNames();
}

void MapWnd::InitTurnRendering() {
    DebugLogger() << "MapWnd::InitTurnRendering";
    ScopedTimer timer("MapWnd::InitTurnRendering", true);

    // adjust size of map window for universe and application size
    Resize(GG::Pt(static_cast<GG::X>(GetUniverse().UniverseWidth() * ZOOM_MAX + AppWidth() * 1.5),
                  static_cast<GG::Y>(GetUniverse().UniverseWidth() * ZOOM_MAX + AppHeight() * 1.5)));


    // remove any existing fleet movement lines or projected movement lines.  this gets cleared
    // here instead of with the movement line stuff because that would clear some movement lines
    // that come from the SystemIcons
    m_fleet_lines.clear();
    ClearProjectedFleetMovementLines();

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    const auto& this_client_known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id);
    const auto& this_client_stale_object_info = GetUniverse().EmpireStaleKnowledgeObjectIDs(client_empire_id);
    const ObjectMap& objects = Objects();

    // remove old system icons
    for (const auto& system_icon : m_system_icons)
        DetachChild(system_icon.second);
    m_system_icons.clear();

    // create system icons
    for (auto& sys : objects.FindObjects<System>()) {
        int sys_id = sys->ID();

        // skip known destroyed objects
        if (this_client_known_destroyed_objects.count(sys_id))
            continue;

        // create new system icon
        auto icon = GG::Wnd::Create<SystemIcon>(GG::X0, GG::Y0, GG::X(10), sys_id);
        m_system_icons[sys_id] = icon;
        icon->InstallEventFilter(shared_from_this());
        if (SidePanel::SystemID() == sys_id)
            icon->SetSelected(true);
        AttachChild(icon);

        // connect UI response signals.  TODO: Make these configurable in GUI?
        icon->LeftClickedSignal.connect(
            boost::bind(&MapWnd::SystemLeftClicked, this, _1));
        icon->RightClickedSignal.connect(
            boost::bind(&MapWnd::SystemRightClicked, this, _1, _2));
        icon->LeftDoubleClickedSignal.connect(
            boost::bind(&MapWnd::SystemDoubleClicked, this, _1));
        icon->MouseEnteringSignal.connect(
            boost::bind(&MapWnd::MouseEnteringSystem, this, _1, _2));
        icon->MouseLeavingSignal.connect(
            boost::bind(&MapWnd::MouseLeavingSystem, this, _1));
    }

    // temp: reset starfield each turn
    m_starfield_verts.clear();
    m_starfield_colours.clear();
    // end temp

    // create buffers for system icon and galaxy gas rendering, and starlane rendering
    InitSystemRenderingBuffers();
    InitStarlaneRenderingBuffers();

    // position system icons
    DoSystemIconsLayout();


    // remove old field icons
    for (const auto& field_icon : m_field_icons)
        DetachChild(field_icon.second);
    m_field_icons.clear();

    // create field icons
    for (auto& field : objects.FindObjects<Field>()) {
        int fld_id = field->ID();

        // skip known destroyed and stale fields
        if (this_client_known_destroyed_objects.count(fld_id))
            continue;
        if (this_client_stale_object_info.count(fld_id))
            continue;
        // don't skip not visible but not stale fields; still expect these to be where last seen, or near there
        //if (field->GetVisibility(client_empire_id) <= VIS_NO_VISIBILITY)
        //    continue;

        // create new system icon
        auto icon = GG::Wnd::Create<FieldIcon>(fld_id);
        m_field_icons[fld_id] = icon;
        icon->InstallEventFilter(shared_from_this());

        AttachChild(icon);

        icon->RightClickedSignal.connect(
            boost::bind(&MapWnd::FieldRightClicked, this, _1));
    }

    // position field icons
    DoFieldIconsLayout();
    InitFieldRenderingBuffers();

    InitVisibilityRadiiRenderingBuffers();

    // create fleet buttons and move lines.  needs to be after InitStarlaneRenderingBuffers so that m_starlane_endpoints is populated
    RefreshFleetButtons();


    // move field icons to bottom of child stack so that other icons can be moused over with a field
    for (const auto& field_icon : m_field_icons)
        MoveChildDown(field_icon.second);
}

void MapWnd::InitSystemRenderingBuffers() {
    DebugLogger() << "MapWnd::InitSystemRenderingBuffers";
    ScopedTimer timer("MapWnd::InitSystemRenderingBuffers", true);

    // clear out all the old buffers
    ClearSystemRenderingBuffers();

    // Generate texture coordinates to be used for subsequent vertex buffer creation.
    // Note these coordinates assume the texture is twice as large as it should
    // be.  This allows us to use one set of texture coords for everything, even
    // though the star-halo textures must be rendered at sizes as much as twice
    // as large as the star-disc textures.
    for (std::size_t i = 0; i < m_system_icons.size(); ++i) {
        m_star_texture_coords.store( 1.5,-0.5);
        m_star_texture_coords.store(-0.5,-0.5);
        m_star_texture_coords.store(-0.5, 1.5);
        m_star_texture_coords.store( 1.5, 1.5);
    }


    for (const auto& system_icon : m_system_icons) {
        const auto& icon = system_icon.second;
        int system_id = system_icon.first;
        auto system = GetSystem(system_id);
        if (!system) {
            ErrorLogger() << "MapWnd::InitSystemRenderingBuffers couldn't get system with id " << system_id;
            continue;
        }

        // Add disc and halo textures for system icon
        // See note above texture coords for why we're making coordinate sets that are 2x too big.
        double icon_size = ClientUI::SystemIconSize();
        float icon_ul_x = static_cast<float>(system->X() - icon_size);
        float icon_ul_y = static_cast<float>(system->Y() - icon_size);
        float icon_lr_x = static_cast<float>(system->X() + icon_size);
        float icon_lr_y = static_cast<float>(system->Y() + icon_size);

        if (icon->DiscTexture()) {
            auto& core_vertices = m_star_core_quad_vertices[icon->DiscTexture()];
            core_vertices.store(icon_lr_x,icon_ul_y);
            core_vertices.store(icon_ul_x,icon_ul_y);
            core_vertices.store(icon_ul_x,icon_lr_y);
            core_vertices.store(icon_lr_x,icon_lr_y);
        }

        if (icon->HaloTexture()) {
            auto& halo_vertices = m_star_halo_quad_vertices[icon->HaloTexture()];
            halo_vertices.store(icon_lr_x,icon_ul_y);
            halo_vertices.store(icon_ul_x,icon_ul_y);
            halo_vertices.store(icon_ul_x,icon_lr_y);
            halo_vertices.store(icon_lr_x,icon_lr_y);
        }


        // add (rotated) gaseous substance around system
        if (auto gas_texture = GetGasTexture()) {
            const float GAS_SIZE = ClientUI::SystemIconSize() * 6.0;
            const float ROTATION = system_id * 27.0; // arbitrary rotation in radians ("27.0" is just a number that produces pleasing results)
            const float COS_THETA = std::cos(ROTATION);
            const float SIN_THETA = std::sin(ROTATION);

            // Components of corner points of a quad
            const float X1 =  GAS_SIZE, Y1 =  GAS_SIZE;  // upper right corner (X1, Y1)
            const float X2 = -GAS_SIZE, Y2 =  GAS_SIZE;  // upper left corner  (X2, Y2)
            const float X3 = -GAS_SIZE, Y3 = -GAS_SIZE;  // lower left corner  (X3, Y3)
            const float X4 =  GAS_SIZE, Y4 = -GAS_SIZE;  // lower right corner (X4, Y4)

            // Calculate rotated corner point components after CCW ROTATION radians around origin.
            const float X1r =  COS_THETA*X1 + SIN_THETA*Y1;
            const float Y1r = -SIN_THETA*X1 + COS_THETA*Y1;
            const float X2r =  COS_THETA*X2 + SIN_THETA*Y2;
            const float Y2r = -SIN_THETA*X2 + COS_THETA*Y2;
            const float X3r =  COS_THETA*X3 + SIN_THETA*Y3;
            const float Y3r = -SIN_THETA*X3 + COS_THETA*Y3;
            const float X4r =  COS_THETA*X4 + SIN_THETA*Y4;
            const float Y4r = -SIN_THETA*X4 + COS_THETA*Y4;

            // See note above texture coords for why we're making coordinate sets that are 2x too big.

            // add to system position to get translated scaled rotated quad corner
            const float GAS_X1 = system->X() + X1r;
            const float GAS_Y1 = system->Y() + Y1r;
            const float GAS_X2 = system->X() + X2r;
            const float GAS_Y2 = system->Y() + Y2r;
            const float GAS_X3 = system->X() + X3r;
            const float GAS_Y3 = system->Y() + Y3r;
            const float GAS_X4 = system->X() + X4r;
            const float GAS_Y4 = system->Y() + Y4r;

            m_galaxy_gas_quad_vertices.store(GAS_X1,GAS_Y1); // rotated upper right
            m_galaxy_gas_quad_vertices.store(GAS_X2,GAS_Y2); // rotated upper left
            m_galaxy_gas_quad_vertices.store(GAS_X3,GAS_Y3); // rotated lower left
            m_galaxy_gas_quad_vertices.store(GAS_X4,GAS_Y4); // rotated lower right

            unsigned int subtexture_index = system_id % 12;                             //  0  1  2  3  4  5  6  7  8  9 10 11
            unsigned int subtexture_x_index = subtexture_index / 3;                     //  0  0  0  0  1  1  1  1  2  2  2  2
            unsigned int subtexture_y_index = subtexture_index - 4*subtexture_x_index;  //  0  1  2  3  0  1  2  3  0  1  2  3

            const GLfloat* default_tex_coords = gas_texture->DefaultTexCoords();
            const GLfloat tex_coord_min_x = default_tex_coords[0];
            const GLfloat tex_coord_min_y = default_tex_coords[1];
            const GLfloat tex_coord_max_x = default_tex_coords[2];
            const GLfloat tex_coord_max_y = default_tex_coords[3];

            // gas texture is expected to be a 4 wide by 3 high grid
            // also add a bit of padding to hopefully avoid artifacts of texture edges
            const GLfloat tx_low_x = tex_coord_min_x  + (subtexture_x_index + 0)*(tex_coord_max_x - tex_coord_min_x)/4;
            const GLfloat tx_high_x = tex_coord_min_x + (subtexture_x_index + 1)*(tex_coord_max_x - tex_coord_min_x)/4;
            const GLfloat tx_low_y = tex_coord_min_y  + (subtexture_y_index + 0)*(tex_coord_max_y - tex_coord_min_y)/3;
            const GLfloat tx_high_y = tex_coord_min_y + (subtexture_y_index + 1)*(tex_coord_max_y - tex_coord_min_y)/3;

            m_galaxy_gas_texture_coords.store(tx_high_x, tx_low_y);
            m_galaxy_gas_texture_coords.store(tx_low_x,  tx_low_y);
            m_galaxy_gas_texture_coords.store(tx_low_x,  tx_high_y);
            m_galaxy_gas_texture_coords.store(tx_high_x, tx_high_y);
        }
    }

    // create new buffers

    // star cores
    for (auto& star_core_buffer : m_star_core_quad_vertices) {
        glBindTexture(GL_TEXTURE_2D, star_core_buffer.first->OpenGLId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        star_core_buffer.second.createServerBuffer();
    }

    // star halos
    for (auto& star_halo_buffer : m_star_halo_quad_vertices) {
        glBindTexture(GL_TEXTURE_2D, star_halo_buffer.first->OpenGLId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        star_halo_buffer.second.createServerBuffer();
    }

    m_star_texture_coords.createServerBuffer();
    m_galaxy_gas_quad_vertices.createServerBuffer();
    m_galaxy_gas_texture_coords.createServerBuffer();
}

void MapWnd::ClearSystemRenderingBuffers() {
    m_star_core_quad_vertices.clear();
    m_star_halo_quad_vertices.clear();
    m_galaxy_gas_quad_vertices.clear();
    m_galaxy_gas_texture_coords.clear();
    m_star_texture_coords.clear();
    m_star_circle_vertices.clear();
}

namespace GetPathsThroughSupplyLanes {
    // SupplyLaneMap map keyed by system containing all systems
    // corresponding to valid supply lane destinations
    typedef boost::unordered_multimap<int,int> SupplyLaneMMap;


    /**
       GetPathsThroughSupplyLanes starts with:

       \p terminal_points are system ids of systems that contain either
       a resource source or a resource sink.

       \p supply_lanes are pairs of system ids at the end of supply
       lanes.

       GetPathsThroughSupplyLanes returns a \p good_path.

       The \p good_path is all system ids of systems connecting any \p
       terminal_point to any other reachable \p terminal_point along the
       \p supply_lanes. The \p good_path is all systems on a path that
       could transport a resource from a source to a sink along a
       starlane that is part of the starlanes through which supply can
       flow (See Empire/Supply.h for details.). The \p good_path
       includes the terminal point system ids that are part of the
       path.  The \p good_path will exclude terminal_points not
       connected to a supply lane, islands of supply lane not connected
       to at least two terminal points, and dead-end lengths of supply
       lane that don't connect between two terminal points.


       Algorithm Descrition:

       The algorithm starts with terminal points and supply lanes.  It
       finds all paths from any terminal point to any other terminal
       point connected only by supply lanes.

       The algorithm finds and returns all system ids on the \p
       good_path in two steps:
       1) find mid points on paths along supply lanes between terminal points,
       2) return the system ids collected by tracing the paths from
          mid points to terminal points of the found paths.


       In the first part, it starts a breadth first search from every
       terminal point at once.  It tracks which terminal point each path
       started from.

       When two paths from different terminal points meet it records
       both points that met as mid points on a good path between
       terminal points.

       When two paths meet from the same terminal points it merges them
       into one path.


       In the second part, it starts from the mid points and works its
       way back to the terminal points, recording every system along the
       path as part of the good path.  It stops when it reaches a system
       already in the good path.


       The algorithm is fast because neither the first nor the second
       part visits any system more than once.

       The first part uses visited to track already visited systems.

       The second part stops back tracking along paths when it reaches
       systems already on the good path.

     */
    void GetPathsThroughSupplyLanes(
        std::unordered_set<int>& good_path,
        const std::unordered_set<int>& terminal_points,
        const SupplyLaneMMap& supply_lanes);


    // PathInfo stores the \p ids of systems one hop back on a path
    // toward an \p o originating terminal system.
    struct PathInfo {
        PathInfo(int a, int o) : one_hop_back(1, a), single_origin(o) {}
        PathInfo(int o) : one_hop_back(), single_origin(o) {}
        // system(s) one hop back on the path.
        // The terminal point has no preceding system.
        // Merged paths are indicated with multiple preceding systems.
        std::vector<int> one_hop_back;
        // The originating terminal point.
        // If single origin is boost::none then two paths with at least
        // two different terminal points merged.
        boost::optional<int> single_origin;
    };

    struct PrevCurrInfo {
        PrevCurrInfo(int p, int n, int o) : prev(p), curr(n), origin(o) {}
        int prev, curr, origin;
    };

    void GetPathsThroughSupplyLanes(
        std::unordered_set<int> & good_path,
        const std::unordered_set<int> & terminal_points,
        const SupplyLaneMMap& supply_lanes)
    {
        good_path.clear();

        // No terminal points, so all paths lead nowhere.
        if (terminal_points.empty())
            return;

        // Part One:  Find all reachable mid points between two different
        // terminal points.

        // try_next holds systems reached in the breadth first search
        // that have not had the supply lanes leaving them explored.
        std::deque<PrevCurrInfo> try_next;

        // visited holds systems already reached by the breadth first search.
        boost::unordered_map<int, PathInfo> visited;

        // reachable_midpoints holds all systems reachable from at least
        // two different terminal points.
        std::vector<int> reachable_midpoints;

        // Initialize with all the terminal points, by adding all
        // terminal points to the queue, and to visited.
        for (int terminal_point : terminal_points) {
            try_next.push_back(PrevCurrInfo(terminal_point, terminal_point, terminal_point));
            visited.insert({terminal_point, PathInfo(terminal_point)});
        }

        // Find all reachable midpoints where paths from two different
        // terminal points meet.
        while (!try_next.empty() ) {
            // Try the next system from the queue.
            const PrevCurrInfo& curr = try_next.front();

            // Check each supply lane that exits this sytem.
            auto supplylane_endpoints = supply_lanes.equal_range(curr.curr);
            for (auto sup_it = supplylane_endpoints.first;
                 sup_it != supplylane_endpoints.second; ++sup_it)
            {
                int next = sup_it->second;

                // Skip the system if it back tracks.
                if (next == curr.prev)
                    continue;

                auto previous = visited.find(next);

                // next has no previous so it is an unvisited
                // system. Create a new previous from curr->next with
                // the same originating terminal point as the current
                // system.
                if (previous == visited.end()) {
                    visited.insert({next, PathInfo(curr.curr, curr.origin)});
                    try_next.push_back(PrevCurrInfo(curr.curr, next, curr.origin));

                // next has an ancester so it was visited. Modify the
                // older previous to merge paths/create mid points.
                } else {
                    // curr and the previous have the same origin so add
                    // curr to the systems one hop back along the path
                    // to previous.
                    if (previous->second.single_origin
                        && previous->second.single_origin == curr.origin)
                    {
                        previous->second.one_hop_back.push_back(curr.curr);

                    // curr and the previous have different origins so
                    // mark both as reachable midpoints along a good path.
                    } else if (previous->second.single_origin
                        && previous->second.single_origin != curr.origin)
                    {
                        // Single origin becomes multi-origin and these
                        // points are both marked as midpoints.
                        previous->second.single_origin = boost::none;
                        reachable_midpoints.push_back(curr.curr);
                        reachable_midpoints.push_back(next);

                    // previous is multi-origin so it is already a mid
                    // point on a good path to multiple terminal
                    // points.  Add curr to the systems one hop back
                    // along the path to previous.
                    } else
                        previous->second.one_hop_back.push_back(curr.curr);
                }
            }
            try_next.pop_front();
        }

        // Queue is exhausted.

        // Part Two: Starting from the mid points find all systems on
        // good paths between terminal points.

        // No terminal point has a path to any other terminal point.
        if (reachable_midpoints.empty())
            return;

        // Return all systems on any path back to a terminal point.
        // Start from every mid point and back track along all paths
        // from that mid point adding each system to the good path.
        // Stop back tracking when you hit a system already on the good
        // path.

        // All visited systems on the path(s) from this midpoint not yet processed.
        std::unordered_set<int> unprocessed;

        for (int reachable_midpoint : reachable_midpoints) {
            boost::unordered_map<int, PathInfo>::const_iterator previous_ii_sys;
            int ii_sys;

            // Add the mid point to unprocessed, and while there
            // are more unprocessed keep checking if the next system is
            // in the good_path.
            unprocessed.insert(reachable_midpoint);
            while (!unprocessed.empty()) {
                ii_sys = *unprocessed.begin();
                unprocessed.erase(unprocessed.begin());

                // If ii_sys is not in the good_path, then add it to the
                // good_path and add all of its visited to the unprocessed.
                if ((previous_ii_sys = visited.find(ii_sys)) != visited.end()
                    && (good_path.count(ii_sys) == 0))
                {
                    good_path.insert(ii_sys);
                    unprocessed.insert(previous_ii_sys->second.one_hop_back.begin(),
                                       previous_ii_sys->second.one_hop_back.end());
                }
            }
        }
        return;
    }
}

namespace {
    /** Look a \p kkey in \p mmap and if not found allocate a new
        shared_ptr with the default constructor.*/
    template <typename Map>
    typename Map::mapped_type& lookup_or_make_shared(Map& mmap, typename Map::key_type const& kkey) {
        auto map_it = mmap.find(kkey);
        if (map_it == mmap.end()) {
            map_it = mmap.insert(map_it, {kkey, std::make_shared<typename Map::mapped_type::element_type>()});
            if (map_it == mmap.end())
                ErrorLogger() << "Unable to insert new empty set into map.";
        }
        return map_it->second;
    }


    /* Takes X and Y coordinates of a pair of systems and moves these points inwards along the vector
     * between them by the radius of a system on screen (at zoom 1.0) and return result */
    LaneEndpoints StarlaneEndPointsFromSystemPositions(double X1, double Y1, double X2, double Y2) {
        // get unit vector
        double deltaX = X2 - X1, deltaY = Y2 - Y1;
        double mag = std::sqrt(deltaX*deltaX + deltaY*deltaY);

        double ring_radius = ClientUI::SystemCircleSize() / 2.0 + 0.5;

        // safety check.  don't modify original coordinates if they're too close togther
        if (mag > 2*ring_radius) {
            // rescale vector to length of ring radius
            double offsetX = deltaX / mag * ring_radius;
            double offsetY = deltaY / mag * ring_radius;

            // move start and end points inwards by rescaled vector
            X1 += offsetX;
            Y1 += offsetY;
            X2 -= offsetX;
            Y2 -= offsetY;
        }

        LaneEndpoints retval(static_cast<float>(X1), static_cast<float>(Y1), static_cast<float>(X2), static_cast<float>(Y2));
        return retval;
    }


    void GetResPoolLaneInfo(int empire_id,
                            boost::unordered_map<std::set<int>, std::shared_ptr<std::set<int>>>& res_pool_systems,
                            boost::unordered_map<std::set<int>, std::shared_ptr<std::set<int>>>& res_group_cores,
                            std::unordered_set<int>& res_group_core_members,
                            boost::unordered_map<int, std::shared_ptr<std::set<int>>>& member_to_core,
                            std::shared_ptr<std::unordered_set<int>>& under_alloc_res_grp_core_members)
    {
        res_pool_systems.clear();
        res_group_cores.clear();
        res_group_core_members.clear();
        member_to_core.clear();
        under_alloc_res_grp_core_members.reset();
        if (empire_id == ALL_EMPIRES)
            return;
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return;

        const ProductionQueue& queue = empire->GetProductionQueue();
        const auto& allocated_pp(queue.AllocatedPP());
        const auto available_pp(empire->GetResourcePool(RE_INDUSTRY)->Output());
        // For each industry set,
        // add all planet's systems to res_pool_systems[industry set]
        for (const auto& available_pp_group : available_pp) {
            float group_pp = available_pp_group.second;
            if (group_pp < 1e-4f)
                continue;

            // std::string this_pool = "( ";
            for (int object_id : available_pp_group.first) {
                // this_pool += std::to_string(object_id) +", ";

                auto planet = GetPlanet(object_id);
                if (!planet)
                    continue;

                //DebugLogger() << "Empire " << empire_id << "; Planet (" << object_id << ") is named " << planet->Name();

                int system_id = planet->SystemID();
                auto system = GetSystem(system_id);
                if (!system)
                    continue;

                lookup_or_make_shared(res_pool_systems, available_pp_group.first)->insert(system_id);
            }
            // this_pool += ")";
            //DebugLogger() << "Empire " << empire_id << "; ResourcePool[RE_INDUSTRY] resourceGroup (" << this_pool << ") has (" << available_pp_group.second << " PP available";
            //DebugLogger() << "Empire " << empire_id << "; ResourcePool[RE_INDUSTRY] resourceGroup (" << this_pool << ") has (" << allocated_pp[available_pp_group.first] << " PP allocated";
        }


        // Convert supply starlanes to non-directional.  This saves half
        // of the lookups.
        GetPathsThroughSupplyLanes::SupplyLaneMMap resource_supply_lanes_undirected;
        const auto resource_supply_lanes_directed =
            GetSupplyManager().SupplyStarlaneTraversals(empire_id);

        for (const auto& supply_lane : resource_supply_lanes_directed) {
            resource_supply_lanes_undirected.insert({supply_lane.first, supply_lane.second});
            resource_supply_lanes_undirected.insert({supply_lane.second, supply_lane.first});
        }

        // For each pool of resources find all paths available through
        // the supply network.

        for (auto& res_pool_system : res_pool_systems) {
            auto& group_core = lookup_or_make_shared(res_group_cores, res_pool_system.first);

            // All individual resource system are included in the
            // network on their own.
            for (int system_id : *(res_pool_system.second)) {
                group_core->insert(system_id);
                res_group_core_members.insert(system_id);
            }

            // Convert res_pool_system.second from set<int> to
            // unordered_set<int> to improve lookup speed.
            std::unordered_set<int> terminal_points;
            for (int system_id : *(res_pool_system.second)) {
                terminal_points.insert(system_id);
            }

            std::unordered_set<int> paths;
            GetPathsThroughSupplyLanes::GetPathsThroughSupplyLanes(
                paths, terminal_points, resource_supply_lanes_undirected);

            // All systems on the paths are valid end points so they are
            // added to the core group of systems that will be rendered
            // with thick lines.
            for (int waypoint : paths) {
                group_core->insert(waypoint);
                res_group_core_members.insert(waypoint);
                member_to_core[waypoint] = group_core;
            }
        }

        // Take note of all systems of under allocated resource groups.
        for (const auto& available_pp_group : available_pp) {
            float group_pp = available_pp_group.second;
            if (group_pp < 1e-4f)
                continue;

            auto allocated_it = allocated_pp.find(available_pp_group.first);
            if (allocated_it == allocated_pp.end() || (group_pp > allocated_it->second + 0.05)) {
                auto group_core_it = res_group_cores.find(available_pp_group.first);
                if (group_core_it != res_group_cores.end()) {
                    if (!under_alloc_res_grp_core_members)
                        under_alloc_res_grp_core_members = std::make_shared<std::unordered_set<int>>();
                    under_alloc_res_grp_core_members->insert(group_core_it->second->begin(), group_core_it->second->end());
                }
            }
        }
    }


    void PrepFullLanesToRender(const boost::unordered_map<int, std::shared_ptr<SystemIcon>>& sys_icons,
                               GG::GL2DVertexBuffer& starlane_vertices,
                               GG::GLRGBAColorBuffer& starlane_colors)
    {
        const auto& this_client_known_destroyed_objects =
            GetUniverse().EmpireKnownDestroyedObjectIDs(HumanClientApp::GetApp()->EmpireID());
        const GG::Clr UNOWNED_LANE_COLOUR = GetOptionsDB().Get<GG::Clr>("ui.map.starlane.color");

        std::set<std::pair<int, int>> already_rendered_full_lanes;

        for (const auto& id_icon : sys_icons) {
            int system_id = id_icon.first;

            // skip systems that don't actually exist
            if (this_client_known_destroyed_objects.count(system_id))
                continue;

            auto start_system = GetSystem(system_id);
            if (!start_system) {
                ErrorLogger() << "GetFullLanesToRender couldn't get system with id " << system_id;
                continue;
            }

            // add system's starlanes
            for (const auto& render_lane : start_system->StarlanesWormholes()) {
                bool lane_is_wormhole = render_lane.second;
                if (lane_is_wormhole) continue; // at present, not rendering wormholes

                int lane_end_sys_id = render_lane.first;

                // skip lanes to systems that don't actually exist
                if (this_client_known_destroyed_objects.count(lane_end_sys_id))
                    continue;

                auto dest_system = GetSystem(render_lane.first);
                if (!dest_system)
                    continue;
                //std::cout << "colouring lanes between " << start_system->Name() << " and " << dest_system->Name() << std::endl;


                // check that this lane isn't already in map / being rendered.
                if (already_rendered_full_lanes.count({start_system->ID(), dest_system->ID()}))
                    continue;
                already_rendered_full_lanes.insert({start_system->ID(), dest_system->ID()});
                already_rendered_full_lanes.insert({dest_system->ID(), start_system->ID()});


                //std::cout << "adding full length lane" << std::endl;
                // add vertices for this full-length starlane
                LaneEndpoints lane_endpoints = StarlaneEndPointsFromSystemPositions(start_system->X(), start_system->Y(), dest_system->X(), dest_system->Y());
                starlane_vertices.store(lane_endpoints.X1, lane_endpoints.Y1);
                starlane_vertices.store(lane_endpoints.X2, lane_endpoints.Y2);


                // determine colour(s) for lane based on which empire(s) can transfer resources along the lane.
                // todo: multiple rendered lanes (one for each empire) when multiple empires use the same lane.
                GG::Clr lane_colour = UNOWNED_LANE_COLOUR;    // default colour if no empires transfer resources along starlane
                for (auto& entry : Empires()) {
                    Empire* empire = entry.second;
                    const auto& resource_supply_lanes = GetSupplyManager().SupplyStarlaneTraversals(entry.first);

                    //std::cout << "resource supply starlane traversals for empire " << empire->Name() << ": " << resource_supply_lanes.size() << std::endl;

                    std::pair<int, int> lane_forward{start_system->ID(), dest_system->ID()};
                    std::pair<int, int> lane_backward{dest_system->ID(), start_system->ID()};

                    // see if this lane exists in this empire's supply propagation lanes set.  either direction accepted.
                    if (resource_supply_lanes.count(lane_forward) || resource_supply_lanes.count(lane_backward)) {
                        lane_colour = empire->Color();
                        //std::cout << "selected colour of empire " << empire->Name() << " for this full lane" << std::endl;
                        break;
                    }
                }

                // vertex colours for starlane
                starlane_colors.store(lane_colour);
                starlane_colors.store(lane_colour);

                //DebugLogger() << "adding full lane from " << start_system->Name() << " to " << dest_system->Name();
            }
        }
    }

    void PrepResourceConnectionLanesToRender(const boost::unordered_map<int, std::shared_ptr<SystemIcon>>& sys_icons,
                                             int empire_id,
                                             std::set<std::pair<int, int>>& rendered_half_starlanes,
                                             GG::GL2DVertexBuffer& rc_starlane_vertices,
                                             GG::GLRGBAColorBuffer& rc_starlane_colors)
    {
        rendered_half_starlanes.clear();

        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return;
        GG::Clr lane_colour = empire->Color();

        // map keyed by ResourcePool (set of objects) to the corresponding set of system ids
        boost::unordered_map<std::set<int>, std::shared_ptr<std::set<int>>> res_pool_systems;
        // map keyed by ResourcePool to the set of systems considered the core of the corresponding ResGroup
        boost::unordered_map<std::set<int>, std::shared_ptr<std::set<int>>> res_group_cores;
        std::unordered_set<int> res_group_core_members;
        boost::unordered_map<int, std::shared_ptr<std::set<int>>> member_to_core;
        std::shared_ptr<std::unordered_set<int>> under_alloc_res_grp_core_members;
        GetResPoolLaneInfo(empire_id, res_pool_systems,
                           res_group_cores, res_group_core_members,
                           member_to_core, under_alloc_res_grp_core_members);

        const std::set<int>& this_client_known_destroyed_objects =
            GetUniverse().EmpireKnownDestroyedObjectIDs(HumanClientApp::GetApp()->EmpireID());
        //unused variable const GG::Clr UNOWNED_LANE_COLOUR = GetOptionsDB().Get<GG::Clr>("ui.map.starlane.color");


        for (const auto& id_icon : sys_icons) {
            int system_id = id_icon.first;

            // skip systems that don't actually exist
            if (this_client_known_destroyed_objects.count(system_id))
                continue;

            auto start_system = GetSystem(system_id);
            if (!start_system) {
                ErrorLogger() << "GetFullLanesToRender couldn't get system with id " << system_id;
                continue;
            }

            // add system's starlanes
            for (const auto& render_lane : start_system->StarlanesWormholes()) {
                bool lane_is_wormhole = render_lane.second;
                if (lane_is_wormhole) continue; // at present, not rendering wormholes

                int lane_end_sys_id = render_lane.first;

                // skip lanes to systems that don't actually exist
                if (this_client_known_destroyed_objects.count(lane_end_sys_id))
                    continue;

                auto dest_system = GetSystem(render_lane.first);
                if (!dest_system)
                    continue;
                //std::cout << "colouring lanes between " << start_system->Name() << " and " << dest_system->Name() << std::endl;


                // check that this lane isn't already going to be rendered.  skip it if it is.
                if (rendered_half_starlanes.count({start_system->ID(), dest_system->ID()}))
                    continue;


                // add resource connection highlight lanes
                //std::pair<int, int> lane_forward{start_system->ID(), dest_system->ID()};
                //std::pair<int, int> lane_backward{dest_system->ID(), start_system->ID()};
                LaneEndpoints lane_endpoints = StarlaneEndPointsFromSystemPositions(start_system->X(), start_system->Y(), dest_system->X(), dest_system->Y());

                if (!res_group_core_members.count(start_system->ID()))
                    continue;

                //start system is a res Grp core member for empire -- highlight
                float indicator_extent = 0.5f;
                GG::Clr lane_colour_to_use = lane_colour;
                if (under_alloc_res_grp_core_members
                    && under_alloc_res_grp_core_members->count(start_system->ID()))
                {
                    lane_colour_to_use = GG::DarkColor(GG::Clr(255-lane_colour.r, 255-lane_colour.g, 255-lane_colour.b, lane_colour.a));
                }

                auto start_core = member_to_core.find(start_system->ID());
                auto dest_core = member_to_core.find(dest_system->ID());
                if (start_core != member_to_core.end() && dest_core != member_to_core.end()
                    && (start_core->second != dest_core->second)
                    && (*(start_core->second) != *(dest_core->second)))
                {
                    indicator_extent = 0.2f;
                }
                rc_starlane_vertices.store(lane_endpoints.X1, lane_endpoints.Y1);
                rc_starlane_vertices.store((lane_endpoints.X2 - lane_endpoints.X1) * indicator_extent + lane_endpoints.X1,  // part way along starlane
                                           (lane_endpoints.Y2 - lane_endpoints.Y1) * indicator_extent + lane_endpoints.Y1);

                rc_starlane_colors.store(lane_colour_to_use);
                rc_starlane_colors.store(lane_colour_to_use);
            }
        }
    }

    void PrepObstructedLaneTraversalsToRender(const boost::unordered_map<int, std::shared_ptr<SystemIcon>>& sys_icons,
                                              int empire_id,
                                              std::set<std::pair<int, int>>& rendered_half_starlanes,
                                              GG::GL2DVertexBuffer& starlane_vertices,
                                              GG::GLRGBAColorBuffer& starlane_colors)
    {
        auto this_empire = GetEmpire(empire_id);
        if (!this_empire)
            return;

        const auto& this_client_known_destroyed_objects =
            GetUniverse().EmpireKnownDestroyedObjectIDs(HumanClientApp::GetApp()->EmpireID());


        for (const auto& id_icon : sys_icons) {
            int system_id = id_icon.first;

            // skip systems that don't actually exist
            if (this_client_known_destroyed_objects.count(system_id))
                continue;

            // skip systems that don't actually exist
            if (this_client_known_destroyed_objects.count(system_id))
                continue;

            auto start_system = GetSystem(system_id);
            if (!start_system) {
                ErrorLogger() << "MapWnd::InitStarlaneRenderingBuffers couldn't get system with id " << system_id;
                continue;
            }

            // add system's starlanes
            for (const auto& render_lane : start_system->StarlanesWormholes()) {
                bool lane_is_wormhole = render_lane.second;
                if (lane_is_wormhole) continue; // at present, not rendering wormholes

                int lane_end_sys_id = render_lane.first;

                // skip lanes to systems that don't actually exist
                if (this_client_known_destroyed_objects.count(lane_end_sys_id))
                    continue;

                auto dest_system = GetSystem(render_lane.first);
                if (!dest_system)
                    continue;
                //std::cout << "colouring lanes between " << start_system->Name() << " and " << dest_system->Name() << std::endl;


                // check that this lane isn't already going to be rendered.  skip it if it is.
                if (rendered_half_starlanes.count({start_system->ID(), dest_system->ID()}))
                    continue;


                // add obstructed lane traversals as half lanes
                for (auto& entry : Empires()) {
                    Empire* empire = entry.second;
                    const auto& resource_obstructed_supply_lanes =
                        GetSupplyManager().SupplyObstructedStarlaneTraversals(entry.first);

                    // see if this lane exists in this empire's obstructed supply propagation lanes set.  either direction accepted.
                    if (!resource_obstructed_supply_lanes.count({start_system->ID(), dest_system->ID()}))
                        continue;

                    // found an empire that has a half lane here, so add it.
                    rendered_half_starlanes.insert({start_system->ID(), dest_system->ID()});  // inserted as ordered pair, so both directions can have different half-lanes

                    LaneEndpoints lane_endpoints = StarlaneEndPointsFromSystemPositions(start_system->X(), start_system->Y(), dest_system->X(), dest_system->Y());
                    starlane_vertices.store(lane_endpoints.X1, lane_endpoints.Y1);
                    starlane_vertices.store((lane_endpoints.X1 + lane_endpoints.X2) * 0.5f,   // half way along starlane
                                            (lane_endpoints.Y1 + lane_endpoints.Y2) * 0.5f);

                    GG::Clr lane_colour = empire->Color();
                    starlane_colors.store(lane_colour);
                    starlane_colors.store(lane_colour);

                    //std::cout << "Adding half lane between " << start_system->Name() << " to " << dest_system->Name() << " with colour of empire " << empire->Name() << std::endl;

                    break;
                }
            }
        }
    }

    std::map<std::pair<int, int>, LaneEndpoints> CalculateStarlaneEndpoints(
        const boost::unordered_map<int, std::shared_ptr<SystemIcon>>& sys_icons)
    {

        std::map<std::pair<int, int>, LaneEndpoints> retval;

        const std::set<int>& this_client_known_destroyed_objects =
            GetUniverse().EmpireKnownDestroyedObjectIDs(HumanClientApp::GetApp()->EmpireID());

        for (auto const& id_icon : sys_icons) {
            int system_id = id_icon.first;

            // skip systems that don't actually exist
            if (this_client_known_destroyed_objects.count(system_id))
                continue;

            auto start_system = GetSystem(system_id);
            if (!start_system) {
                ErrorLogger() << "GetFullLanesToRender couldn't get system with id " << system_id;
                continue;
            }

            // add system's starlanes
            for (const auto& render_lane : start_system->StarlanesWormholes()) {
                bool lane_is_wormhole = render_lane.second;
                if (lane_is_wormhole) continue; // at present, not rendering wormholes

                int lane_end_sys_id = render_lane.first;

                // skip lanes to systems that don't actually exist
                if (this_client_known_destroyed_objects.count(lane_end_sys_id))
                    continue;

                auto dest_system = GetSystem(render_lane.first);
                if (!dest_system)
                    continue;

                retval[{system_id, lane_end_sys_id}] =
                    StarlaneEndPointsFromSystemPositions(start_system->X(), start_system->Y(),
                                                         dest_system->X(), dest_system->Y());
                retval[{lane_end_sys_id, system_id}] =
                    StarlaneEndPointsFromSystemPositions(dest_system->X(), dest_system->Y(),
                                                         start_system->X(), start_system->Y());
            }
        }

        return retval;
    }
}

void MapWnd::InitStarlaneRenderingBuffers() {
    DebugLogger() << "MapWnd::InitStarlaneRenderingBuffers";
    ScopedTimer timer("MapWnd::InitStarlaneRenderingBuffers", true);

    ClearStarlaneRenderingBuffers();

    // todo: move this somewhere better... fill in starlane endpoint cache
    m_starlane_endpoints = CalculateStarlaneEndpoints(m_system_icons);


    // temp storage
    std::set<std::pair<int, int>> rendered_half_starlanes;  // stored as unaltered pairs, so that a each direction of traversal can be shown separately


    // add vertices and colours to lane rendering buffers
    PrepFullLanesToRender(m_system_icons, m_starlane_vertices, m_starlane_colors);
    PrepResourceConnectionLanesToRender(m_system_icons, HumanClientApp::GetApp()->EmpireID(),
                                        rendered_half_starlanes,
                                        m_RC_starlane_vertices, m_RC_starlane_colors);
    PrepObstructedLaneTraversalsToRender(m_system_icons, HumanClientApp::GetApp()->EmpireID(),
                                         rendered_half_starlanes,
                                         m_starlane_vertices, m_starlane_colors);


    // fill new buffers
    m_starlane_vertices.createServerBuffer();
    m_starlane_colors.createServerBuffer();
    m_starlane_vertices.harmonizeBufferType(m_starlane_colors);
    m_RC_starlane_vertices.createServerBuffer();
    m_RC_starlane_colors.createServerBuffer();
    m_RC_starlane_vertices.harmonizeBufferType(m_RC_starlane_colors);
}

void MapWnd::ClearStarlaneRenderingBuffers() {
    m_starlane_vertices.clear();
    m_starlane_colors.clear();
    m_RC_starlane_vertices.clear();
    m_RC_starlane_colors.clear();
}

void MapWnd::InitFieldRenderingBuffers() {
    DebugLogger() << "MapWnd::InitFieldRenderingBuffers";
    ScopedTimer timer("MapWnd::InitFieldRenderingBuffers", true);

    ClearFieldRenderingBuffers();

    const Universe& universe = GetUniverse();
    int empire_id = HumanClientApp::GetApp()->EmpireID();


    for (auto& field_icon : m_field_icons) {
        bool current_field_visible = universe.GetObjectVisibilityByEmpire(field_icon.first, empire_id) > VIS_BASIC_VISIBILITY;
        auto field = GetField(field_icon.first);
        if (!field)
            continue;
        const float FIELD_SIZE = field->InitialMeterValue(METER_SIZE);  // field size is its radius
        if (FIELD_SIZE <= 0)
            continue;
        auto field_texture = field_icon.second->FieldTexture();
        if (!field_texture)
            continue;

        auto& field_both_vertex_buffers = m_field_vertices[field_texture];
        GG::GL2DVertexBuffer& current_field_vertex_buffer =
            current_field_visible ? field_both_vertex_buffers.first : field_both_vertex_buffers.second;

        // determine field rotation angle...
        float rotation_angle = field->ID() * 27.0f; // arbitrary rotation in radians ("27.0" is just a number that produces pleasing results)
        // per-turn rotation of texture. TODO: make depend on something scriptable
        float rotation_speed = 0.03f;               // arbitrary rotation rate in radians
        if (rotation_speed != 0.0f)
            rotation_angle += CurrentTurn() * rotation_speed;

        const float COS_THETA = std::cos(rotation_angle);
        const float SIN_THETA = std::sin(rotation_angle);

        // Components of corner points of a quad
        const float X1 =  FIELD_SIZE, Y1 =  FIELD_SIZE; // upper right corner (X1, Y1)
        const float X2 = -FIELD_SIZE, Y2 =  FIELD_SIZE; // upper left corner  (X2, Y2)
        const float X3 = -FIELD_SIZE, Y3 = -FIELD_SIZE; // lower left corner  (X3, Y3)
        const float X4 =  FIELD_SIZE, Y4 = -FIELD_SIZE; // lower right corner (X4, Y4)

        // Calculate rotated corner point components after CCW ROTATION radians around origin.
        const float X1r =  COS_THETA*X1 + SIN_THETA*Y1;
        const float Y1r = -SIN_THETA*X1 + COS_THETA*Y1;
        const float X2r =  COS_THETA*X2 + SIN_THETA*Y2;
        const float Y2r = -SIN_THETA*X2 + COS_THETA*Y2;
        const float X3r =  COS_THETA*X3 + SIN_THETA*Y3;
        const float Y3r = -SIN_THETA*X3 + COS_THETA*Y3;
        const float X4r =  COS_THETA*X4 + SIN_THETA*Y4;
        const float Y4r = -SIN_THETA*X4 + COS_THETA*Y4;

        // add to system position to get translated scaled rotated quad corners
        const float FIELD_X1 = field->X() + X1r;
        const float FIELD_Y1 = field->Y() + Y1r;
        const float FIELD_X2 = field->X() + X2r;
        const float FIELD_Y2 = field->Y() + Y2r;
        const float FIELD_X3 = field->X() + X3r;
        const float FIELD_Y3 = field->Y() + Y3r;
        const float FIELD_X4 = field->X() + X4r;
        const float FIELD_Y4 = field->Y() + Y4r;

        current_field_vertex_buffer.store(FIELD_X1, FIELD_Y1);  // rotated upper right
        current_field_vertex_buffer.store(FIELD_X2, FIELD_Y2);  // rotated upper left
        current_field_vertex_buffer.store(FIELD_X3, FIELD_Y3);  // rotated lower left
        current_field_vertex_buffer.store(FIELD_X4, FIELD_Y4);  // rotated lower right

        // also add circles to render scanlines for not-visible fields
        if (!current_field_visible) {
            GG::Pt circle_ul = GG::Pt(GG::X(field->X() - FIELD_SIZE), GG::Y(field->Y() - FIELD_SIZE));
            GG::Pt circle_lr = GG::Pt(GG::X(field->X() + FIELD_SIZE), GG::Y(field->Y() + FIELD_SIZE));
            BufferStoreCircleArcVertices(m_field_scanline_circles, circle_ul, circle_lr, 0, TWO_PI, true, 0, false);
        }
    }
    m_field_scanline_circles.createServerBuffer();

    for (auto& field_buffer : m_field_vertices) {
        auto field_texture = field_buffer.first;
        if (!field_texture)
            continue;

        // todo: why the binding here?
        glBindTexture(GL_TEXTURE_2D, field_texture->OpenGLId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        field_buffer.second.first.createServerBuffer();
        field_buffer.second.second.createServerBuffer();
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // this buffer should only need to be as big as the largest number of
    // visible or not visisble fields for any single texture, but
    // this is simpler to prepare and should be more than big enough
    for (std::size_t i = 0; i < m_field_icons.size(); ++i) {
        m_field_texture_coords.store(1.0f, 0.0f);
        m_field_texture_coords.store(0.0f, 0.0f);
        m_field_texture_coords.store(0.0f, 1.0f);
        m_field_texture_coords.store(1.0f, 1.0f);
    }
    m_field_texture_coords.createServerBuffer();
}

void MapWnd::ClearFieldRenderingBuffers() {
    m_field_vertices.clear();
    m_field_texture_coords.clear();
    m_field_scanline_circles.clear();
}

void MapWnd::InitVisibilityRadiiRenderingBuffers() {
    DebugLogger() << "MapWnd::InitVisibilityRadiiRenderingBuffers";
    //std::cout << "InitVisibilityRadiiRenderingBuffers" << std::endl;
    ScopedTimer timer("MapWnd::InitVisibilityRadiiRenderingBuffers", true);

    ClearVisibilityRadiiRenderingBuffers();

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    const auto& destroyed_object_ids = GetUniverse().DestroyedObjectIds();
    const auto& stale_object_ids = GetUniverse().EmpireStaleKnowledgeObjectIDs(client_empire_id);
    const auto& objects = GetUniverse().Objects();

    // for each map position and empire, find max value of detection range at that position
    std::map<std::pair<int, std::pair<float, float>>, float> empire_position_max_detection_ranges;

    for (auto& obj : objects.FindObjects<UniverseObject>()) {
        int object_id = obj->ID();
        // skip destroyed objects
        if (destroyed_object_ids.count(object_id))
            continue;
        // skip stale objects
        if (stale_object_ids.count(object_id))
            continue;

        // skip unowned objects
        if (obj->Unowned())
            continue;

        // skip objects not at least partially visible this turn
        if (obj->GetVisibility(client_empire_id) <= VIS_BASIC_VISIBILITY)
            continue;

        // don't show radii for fleets or moving ships
        if (obj->ObjectType() == OBJ_FLEET)
            continue;
        if (obj->ObjectType() == OBJ_SHIP) {
            auto ship = std::dynamic_pointer_cast<const Ship>(obj);
            if (!ship)
                continue;
            auto fleet = objects.Object<Fleet>(ship->FleetID());
            if (!fleet)
                continue;
            int cur_id = fleet->SystemID();
            if (cur_id == INVALID_OBJECT_ID)
                continue;
        }

        const Meter* detection_meter = obj->GetMeter(METER_DETECTION);
        if (!detection_meter)
            continue;

        // if this object has the largest yet checked visibility range at this location, update the location's range
        float X = static_cast<float>(obj->X());
        float Y = static_cast<float>(obj->Y());
        float D = detection_meter->Current();
        // skip objects that don't contribute detection
        if (D <= 0.0f)
            continue;

        // find this empires entry for this location, if any
        std::pair<int, std::pair<float, float>> key{obj->Owner(), {X, Y}};
        auto range_it = empire_position_max_detection_ranges.find(key);
        if (range_it != empire_position_max_detection_ranges.end()) {
            if (range_it->second < D) range_it->second = D; // update existing entry
        } else {
            empire_position_max_detection_ranges[key] = D;  // add new entry to map
        }
    }

    std::map<GG::Clr, std::vector<std::pair<GG::Pt, GG::Pt>>> circles;
    for (const auto& detection_circle : empire_position_max_detection_ranges) {
        const Empire* empire = GetEmpire(detection_circle.first.first);
        if (!empire) {
            ErrorLogger() << "InitVisibilityRadiiRenderingBuffers couldn't find empire with id: " << detection_circle.first.first;
            continue;
        }

        float radius = detection_circle.second;
        if (radius < 5.0f || radius > 2048.0f)  // hide uselessly small and ridiculously large circles. the latter so super-testers don't have an empire-coloured haze over the whole map.
            continue;

        GG::Clr circle_colour = empire->Color();
        circle_colour.a = 8*GetOptionsDB().Get<int>("ui.map.detection.range.opacity");

        GG::Pt circle_centre = GG::Pt(GG::X(detection_circle.first.second.first), GG::Y(detection_circle.first.second.second));
        GG::Pt ul = circle_centre - GG::Pt(GG::X(static_cast<int>(radius)), GG::Y(static_cast<int>(radius)));
        GG::Pt lr = circle_centre + GG::Pt(GG::X(static_cast<int>(radius)), GG::Y(static_cast<int>(radius)));

        circles[circle_colour].push_back({ul, lr});

        //std::cout << "adding radii circle at: " << circle_centre << " for empire: " << it->first.first << std::endl;
    }


    const GG::Pt BORDER_INSET(GG::X(1.0f), GG::Y(1.0f));

    // loop over colours / empires, adding a batch of triangles to buffers for
    // each's visibilty circles and outlines
    for (const auto& circle_group : circles) {
        // get empire colour and calculate brighter radii outline colour
        GG::Clr circle_colour = circle_group.first;
        GG::Clr border_colour = circle_colour;
        border_colour.a = std::min(255, border_colour.a + 80);
        AdjustBrightness(border_colour, 2.0, true);

        std::size_t radii_start_index = m_visibility_radii_vertices.size();
        std::size_t border_start_index = m_visibility_radii_border_vertices.size();

        for (const auto& circle : circle_group.second) {
            const GG::Pt& ul = circle.first;
            const GG::Pt& lr = circle.second;

            unsigned int initial_size = m_visibility_radii_vertices.size();
            // store triangles for filled / transparent part of radii
            BufferStoreCircleArcVertices(m_visibility_radii_vertices, ul, lr, 0.0, TWO_PI, true, 0, false);

            // store colours for triangles
            unsigned int size_increment = m_visibility_radii_vertices.size() - initial_size;
            for (unsigned int count = 0; count < size_increment; ++count)
                 m_visibility_radii_colors.store(circle_colour);

            // store line segments for border lines of radii
            initial_size = m_visibility_radii_border_vertices.size();
            BufferStoreCircleArcVertices(m_visibility_radii_border_vertices, ul + BORDER_INSET, lr - BORDER_INSET,
                                         0.0, TWO_PI, false, 0, false);

            // store colours for line segments
            size_increment = m_visibility_radii_border_vertices.size() - initial_size;
            for (unsigned int count = 0; count < size_increment; ++count)
                 m_visibility_radii_border_colors.store(border_colour);
        }

        // store how many vertices to render for this colour
        std::size_t radii_end_index = m_visibility_radii_vertices.size();
        std::size_t border_end_index = m_visibility_radii_border_vertices.size();

        m_radii_radii_vertices_indices_runs.push_back({
            {radii_start_index, radii_end_index - radii_start_index},
            {border_start_index, border_end_index - border_start_index}});
    }

    m_visibility_radii_border_vertices.createServerBuffer();
    m_visibility_radii_border_colors.createServerBuffer();
    m_visibility_radii_border_vertices.harmonizeBufferType(m_visibility_radii_border_colors);
    m_visibility_radii_vertices.createServerBuffer();
    m_visibility_radii_colors.createServerBuffer();
    m_visibility_radii_vertices.harmonizeBufferType(m_visibility_radii_colors);
}

void MapWnd::ClearVisibilityRadiiRenderingBuffers() {
    m_visibility_radii_vertices.clear();
    m_visibility_radii_colors.clear();
    m_visibility_radii_border_vertices.clear();
    m_visibility_radii_border_colors.clear();
    m_radii_radii_vertices_indices_runs.clear();
}

void MapWnd::InitScaleCircleRenderingBuffer() {
    ClearScaleCircleRenderingBuffer();

    if (!m_scale_line)
        return;

    int radius = static_cast<int>(Value(m_scale_line->GetLength()) / std::max(0.001, m_scale_line->GetScaleFactor()));
    if (radius < 5)
        return;

    auto selected_system = GetSystem(SidePanel::SystemID());
    if (!selected_system)
        return;

    GG::Pt circle_centre = GG::Pt(GG::X(selected_system->X()), GG::Y(selected_system->Y()));
    GG::Pt ul = circle_centre - GG::Pt(GG::X(radius), GG::Y(radius));
    GG::Pt lr = circle_centre + GG::Pt(GG::X(radius), GG::Y(radius));

    BufferStoreCircleArcVertices(m_scale_circle_vertices, ul, lr, 0, TWO_PI, false, 0, true);

    m_scale_circle_vertices.createServerBuffer();
}

void MapWnd::ClearScaleCircleRenderingBuffer()
{ m_scale_circle_vertices.clear(); }

void MapWnd::ClearStarfieldRenderingBuffers() {
    m_starfield_verts.clear();
    m_starfield_colours.clear();
}

void MapWnd::RestoreFromSaveData(const SaveGameUIData& data) {
    m_zoom_steps_in = data.map_zoom_steps_in;

    GG::Pt ul = UpperLeft();
    GG::Pt map_ul = GG::Pt(GG::X(data.map_left), GG::Y(data.map_top));
    GG::Pt map_move = map_ul - ul;
    OffsetMove(map_move);

    // this correction ensures that zooming in doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);
    //GG::Pt final_move = move_to_pt - ul;

    MoveTo(move_to_pt - GG::Pt(AppWidth(), AppHeight()));

    m_fleets_exploring = data.fleets_exploring;
}

void MapWnd::ShowSystemNames() {
    for (auto& system_icon : m_system_icons) {
        system_icon.second->ShowName();
    }
}

void MapWnd::HideSystemNames() {
    for (auto& system_icon : m_system_icons) {
        system_icon.second->HideName();
    }
}

void MapWnd::CenterOnMapCoord(double x, double y) {
    GG::Pt ul = ClientUpperLeft();
    GG::X_d current_x = (AppWidth() / 2 - ul.x) / ZoomFactor();
    GG::Y_d current_y = (AppHeight() / 2 - ul.y) / ZoomFactor();
    GG::Pt map_move = GG::Pt(static_cast<GG::X>((current_x - x) * ZoomFactor()),
                             static_cast<GG::Y>((current_y - y) * ZoomFactor()));
    OffsetMove(map_move);

    // this correction ensures that the centering doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);

    MoveTo(move_to_pt - GG::Pt(AppWidth(), AppHeight()));
}

void MapWnd::ShowPlanet(int planet_id) {
    if (!m_in_production_view_mode) {
        ShowPedia();
        m_pedia_panel->SetPlanet(planet_id);
    }
    if (m_in_production_view_mode) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowPlanetInEncyclopedia(planet_id);
    }
}

void MapWnd::ShowCombatLog(int log_id) {
    m_combat_report_wnd->SetLog( log_id );
    m_combat_report_wnd->Show();
    GG::GUI::GetGUI()->MoveUp(m_combat_report_wnd);
    PushWndStack(m_combat_report_wnd);
}

void MapWnd::ShowTech(const std::string& tech_name) {
    if (m_research_wnd->Visible())
        m_research_wnd->ShowTech(tech_name);
    if (m_in_production_view_mode) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowTechInEncyclopedia(tech_name);
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetTech(tech_name);
    }
}

void MapWnd::ShowBuildingType(const std::string& building_type_name) {
    if (m_production_wnd->Visible()) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowBuildingTypeInEncyclopedia(building_type_name);
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetBuildingType(building_type_name);
    }
}

void MapWnd::ShowPartType(const std::string& part_type_name) {
    if (m_design_wnd->Visible())
        m_design_wnd->ShowPartTypeInEncyclopedia(part_type_name);
    if (m_in_production_view_mode) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowPartTypeInEncyclopedia(part_type_name);
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetPartType(part_type_name);
    }
}

void MapWnd::ShowHullType(const std::string& hull_type_name) {
    if (m_design_wnd->Visible()) {
        m_design_wnd->ShowHullTypeInEncyclopedia(hull_type_name);
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetHullType(hull_type_name);
    }
}

void MapWnd::ShowShipDesign(int design_id) {
    if (m_production_wnd->Visible()) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowShipDesignInEncyclopedia(design_id);
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetDesign(design_id);
    }
}

void MapWnd::ShowSpecial(const std::string& special_name) {
    if (m_production_wnd->Visible()) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowSpecialInEncyclopedia(special_name);
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetSpecial(special_name);
    }
}

void MapWnd::ShowSpecies(const std::string& species_name) {
    if (m_production_wnd->Visible()) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowSpeciesInEncyclopedia(species_name);
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetSpecies(species_name);
    }
}

void MapWnd::ShowFieldType(const std::string& field_type_name) {
    if (m_production_wnd->Visible()) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowFieldTypeInEncyclopedia(field_type_name);
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetFieldType(field_type_name);
    }
}

void MapWnd::ShowEmpire(int empire_id) {
    if (m_in_production_view_mode) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowEmpireInEncyclopedia(empire_id);
    }
    else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetEmpire(empire_id);
    }
}

void MapWnd::ShowMeterTypeArticle(const std::string& meter_string) {
    ShowPedia();
    m_pedia_panel->SetMeterType(meter_string);
}

void MapWnd::ShowEncyclopediaEntry(const std::string& str) {
    if (!m_pedia_panel->Visible())
        TogglePedia();
    m_pedia_panel->SetEncyclopediaArticle(str);
}

void MapWnd::CenterOnObject(int id) {
    if (auto obj = GetUniverseObject(id))
        CenterOnMapCoord(obj->X(), obj->Y());
}

void MapWnd::CenterOnObject(std::shared_ptr<const UniverseObject> obj) {
    if (!obj) return;
    CenterOnMapCoord(obj->X(), obj->Y());
}

void MapWnd::ReselectLastSystem() {
    if (SidePanel::SystemID() != INVALID_OBJECT_ID)
        SelectSystem(SidePanel::SystemID());
}

void MapWnd::SelectSystem(int system_id) {
    //std::cout << "MapWnd::SelectSystem(" << system_id << ")" << std::endl;
    auto system = GetSystem(system_id);
    if (!system && system_id != INVALID_OBJECT_ID) {
        ErrorLogger() << "MapWnd::SelectSystem couldn't find system with id " << system_id << " so is selected no system instead";
        system_id = INVALID_OBJECT_ID;
    }


    if (system) {
        // ensure meter estimates are up to date, particularly for which ship is selected
        GetUniverse().UpdateMeterEstimates(system_id, true);
    }


    if (SidePanel::SystemID() != system_id) {
        // remove map selection indicator from previously selected system
        if (SidePanel::SystemID() != INVALID_OBJECT_ID) {
            const auto& it = m_system_icons.find(SidePanel::SystemID());
            if (it != m_system_icons.end())
                it->second->SetSelected(false);
        }

        // set selected system on sidepanel and production screen, as appropriate
        if (m_in_production_view_mode)
            m_production_wnd->SelectSystem(system_id);  // calls SidePanel::SetSystem
        else
            SidePanel::SetSystem(system_id);

        // place map selection indicator on newly selected system
        if (SidePanel::SystemID() != INVALID_OBJECT_ID) {
            const auto& it = m_system_icons.find(SidePanel::SystemID());
            if (it != m_system_icons.end())
                it->second->SetSelected(true);
        }
    }

    InitScaleCircleRenderingBuffer();

    if (m_in_production_view_mode) {
        // don't need to do anything to ensure this->m_side_panel is visible,
        // since it should be hidden if in production view mode.
        return;
    }

    // even if selected system hasn't changed, it may be nessary to show or
    // hide this mapwnd's sidepanel, in case it was hidden at some point and
    // should be visible, or is visible and should be hidden.

    if (SidePanel::SystemID() == INVALID_OBJECT_ID) {
        // no selected system.  hide sidepanel.
        m_side_panel->Hide();
        RemoveFromWndStack(m_side_panel);
    } else {
        // selected a valid system, show sidepanel
        m_side_panel->Show();
        GG::GUI::GetGUI()->MoveUp(m_side_panel);
        PushWndStack(m_side_panel);
    }
}

void MapWnd::ReselectLastFleet() {
    //// DEBUG
    //std::cout << "MapWnd::ReselectLastFleet m_selected_fleet_ids: " << std::endl;
    //for (int fleet_id : m_selected_fleet_ids) {
    //    auto obj = GetUniverse().Object(fleet_id);
    //    if (obj)
    //        std::cout << "    " << obj->Name() << "(" << fleet_id << ")" << std::endl;
    //    else
    //        std::cout << "    [missing object] (" << fleet_id << ")" << std::endl;
    //}

    const ObjectMap& objects = GetUniverse().Objects();

    // search through stored selected fleets' ids and remove ids of missing fleets
    std::set<int> missing_fleets;
    for (int fleet_id : m_selected_fleet_ids) {
        auto fleet = objects.Object<Fleet>(fleet_id);
        if (!fleet)
            missing_fleets.insert(fleet_id);
    }
    for (int fleet_id : missing_fleets)
        m_selected_fleet_ids.erase(fleet_id);


    // select a not-missing fleet, if any
    for (int fleet_id : m_selected_fleet_ids) {
        SelectFleet(fleet_id);
        break;              // abort after first fleet selected... don't need to do more
    }
}

void MapWnd::SelectPlanet(int planetID)
{ m_production_wnd->SelectPlanet(planetID); }   // calls SidePanel::SelectPlanet()

void MapWnd::SelectFleet(int fleet_id)
{ SelectFleet(GetFleet(fleet_id)); }

void MapWnd::SelectFleet(std::shared_ptr<Fleet> fleet) {
    FleetUIManager& manager = FleetUIManager::GetFleetUIManager();

    if (!fleet) {
        //std::cout << "MapWnd::SelectFleet selecting no fleet: deselecting all selected fleets." << std::endl;

        // first deselect any selected fleets in non-active fleet wnd.  this should
        // not emit any signals about the active fleet wnd's fleets changing
        const auto& active_fleet_wnd = manager.ActiveFleetWnd();

        for (const auto& fwnd : manager) {
            auto wnd = fwnd.lock();
            if (wnd && wnd.get() != active_fleet_wnd)
                wnd->DeselectAllFleets();
        }

        // and finally deselect active fleet wnd fleets.  this might emit a signal
        // which will update this->m_selected_Fleets
        if (active_fleet_wnd)
            active_fleet_wnd->DeselectAllFleets();

        return;
    }

    //std::cout << "MapWnd::SelectFleet " << fleet->ID() << std::endl;

    // find if there is a FleetWnd for this fleet already open.
    auto fleet_wnd = manager.WndForFleetID(fleet->ID());
    
    // if there isn't a FleetWnd for this fleet open, need to open one
    if (!fleet_wnd) {
        // Add any overlapping fleet buttons for moving or offroad fleets.
        const auto wnd_fleet_ids = FleetIDsOfFleetButtonsOverlapping(fleet->ID());

        // A leeway, scaled to the button size, around a group of moving fleets so the fleetwnd
        // tracks moving fleets together
        const auto& any_fleet_button = m_fleet_buttons.empty() ? nullptr : m_fleet_buttons.begin()->second;
        double leeway_around_moving_fleets = any_fleet_button ?
            2.0 * (Value(any_fleet_button->Width()) + Value(any_fleet_button->Height())) / ZoomFactor() : 0.0;

        // create new fleetwnd in which to show selected fleet
        fleet_wnd = manager.NewFleetWnd(wnd_fleet_ids, leeway_around_moving_fleets);

        // opened a new FleetWnd, so play sound
        FleetButton::PlayFleetButtonOpenSound();
    }

    // make sure selected fleet's FleetWnd is active
    manager.SetActiveFleetWnd(fleet_wnd);
    GG::GUI::GetGUI()->MoveUp(fleet_wnd);
    PushWndStack(fleet_wnd);

    // if indicated fleet is already the only selected fleet in the active FleetWnd, nothing to do.
    if (m_selected_fleet_ids.size() == 1 && m_selected_fleet_ids.count(fleet->ID()))
        return;
    
    // select fleet in FleetWnd.  this deselects all other fleets in the FleetWnd.
    // this->m_selected_fleet_ids will be updated by ActiveFleetWndSelectedFleetsChanged or ActiveFleetWndChanged
    // signals being emitted and connected to MapWnd::SelectedFleetsChanged
    fleet_wnd->SelectFleet(fleet->ID());
}

void MapWnd::RemoveFleet(int fleet_id) {
    m_fleet_lines.erase(fleet_id);
    m_projected_fleet_lines.erase(fleet_id);
    m_selected_fleet_ids.erase(fleet_id);
    RefreshFleetButtons();
}

void MapWnd::SetFleetMovementLine(int fleet_id) {
    if (fleet_id == INVALID_OBJECT_ID)
        return;

    auto fleet = GetFleet(fleet_id);
    if (!fleet) {
        ErrorLogger() << "MapWnd::SetFleetMovementLine was passed invalid fleet id " << fleet_id;
        return;
    }
    //std::cout << "creating fleet movement line for fleet at (" << fleet->X() << ", " << fleet->Y() << ")" << std::endl;

    // get colour: empire colour, or white if no single empire applicable
    GG::Clr line_colour = GG::CLR_WHITE;
    const Empire* empire = GetEmpire(fleet->Owner());
    if (empire)
        line_colour = empire->Color();
    else if (fleet->Unowned() && fleet->HasMonsters())
        line_colour = GG::CLR_RED;

    // create and store line
    auto route(fleet->TravelRoute());
    auto path = fleet->MovePath(route, true);
    auto route_it = route.begin();
    if (!route.empty() && (++route_it) != route.end()) {
        //DebugLogger() << "MapWnd::SetFleetMovementLine fleet id " << fleet_id<<" checking for blockade at system "<< route.front() <<
        //    " with m_arrival_lane "<< fleet->ArrivalStarlane()<<" and next destination "<<*route_it;
        if (fleet->SystemID() == route.front() && fleet->BlockadedAtSystem(route.front(), *route_it)) { //adjust ETAs if necessary
            //if (!route.empty() && fleet->SystemID()==route.front() && (++(path.begin()))->post_blockade) {
            //DebugLogger() << "MapWnd::SetFleetMovementLine fleet id " << fleet_id<<" blockaded at system "<< route.front() <<
            //    " with m_arrival_lane "<< fleet->ArrivalStarlane()<<" and next destination "<<*route_it;
            if (route_it != route.end() && !( (*route_it == fleet->ArrivalStarlane())  ||
                (empire && empire->PreservedLaneTravel(fleet->SystemID(), *route_it)) ) )
            {
                for (MovePathNode& node : path) {
                    //DebugLogger() <<   "MapWnd::SetFleetMovementLine fleet id " << fleet_id<<" node obj " << node.object_id <<
                    //                            ", node lane end " << node.lane_end_id << ", is post-blockade (" << node.post_blockade << ")";
                    node.eta++;
                }
            } else {
                //DebugLogger() << "MapWnd::SetFleetMovementLine fleet id " << fleet_id<<" slips through second block check";
            }
        }
    }
    m_fleet_lines[fleet_id] = MovementLineData(path, m_starlane_endpoints, line_colour, fleet->Owner());
}

void MapWnd::SetProjectedFleetMovementLine(int fleet_id, const std::list<int>& travel_route) {
    if (fleet_id == INVALID_OBJECT_ID)
        return;

    // ensure passed fleet exists
    auto fleet = GetFleet(fleet_id);
    if (!fleet) {
        ErrorLogger() << "MapWnd::SetProjectedFleetMovementLine was passed invalid fleet id " << fleet_id;
        return;
    }

    //std::cout << "creating projected fleet movement line for fleet at (" << fleet->X() << ", " << fleet->Y() << ")" << std::endl;
    const Empire* empire = GetEmpire(fleet->Owner());

    // get move path to show.  if there isn't one, show nothing
    auto path = fleet->MovePath(travel_route, true);



    // We need the route to contain the current system
    // even when it is empty to switch between non appending
    // and appending projections on shift changes
    if (path.empty()) {
        path.push_back(MovePathNode(fleet->X(), fleet->Y(), true, 0, fleet->SystemID(), INVALID_OBJECT_ID, INVALID_OBJECT_ID));
    }

    auto route_it = travel_route.begin();
    if (!travel_route.empty() && (++route_it) != travel_route.end()) {
        if (fleet->SystemID() == travel_route.front() && fleet->BlockadedAtSystem(travel_route.front(), *route_it)) { //adjust ETAs if necessary
            //if (!route.empty() && fleet->SystemID()==route.front() && (++(path.begin()))->post_blockade) {
            //DebugLogger() << "MapWnd::SetFleetMovementLine fleet id " << fleet_id<<" blockaded at system "<< route.front() <<
            //" with m_arrival_lane "<< fleet->ArrivalStarlane()<<" and next destination "<<*route_it;
            if (route_it != travel_route.end() && !((*route_it == fleet->ArrivalStarlane()) ||
                (empire && empire->PreservedLaneTravel(fleet->SystemID(), *route_it))))
            {
                for (MovePathNode& node : path) {
                    //DebugLogger() <<   "MapWnd::SetFleetMovementLine fleet id " << fleet_id << " node obj " << node.object_id <<
                    //                            ", node lane end " << node.lane_end_id << ", is post-blockade (" << node.post_blockade << ")";
                    node.eta++;
                }
            }
        }
    }

    // get colour: empire colour, or white if no single empire applicable
    GG::Clr line_colour = GG::CLR_WHITE;
    if (empire)
        line_colour = empire->Color();

    // create and store line
    m_projected_fleet_lines[fleet_id] = MovementLineData(path, m_starlane_endpoints, line_colour, fleet->Owner());
}

void MapWnd::SetProjectedFleetMovementLines(const std::vector<int>& fleet_ids,
                                            const std::list<int>& travel_route)
{
    for (int fleet_id : fleet_ids)
        SetProjectedFleetMovementLine(fleet_id, travel_route);
}

void MapWnd::ClearProjectedFleetMovementLines()
{ m_projected_fleet_lines.clear(); }

void MapWnd::ForgetObject(int id) {
    // Remove visibility information for this object from
    // the empire's visibility table.
    // The object is usually a ghost ship or fleet.
    // Server changes cause a permanent effect
    // Tell the server to change what the empire wants to know
    // in future so that the server doesn't keep resending this
    // object information.
    auto obj = GetUniverseObject(id);
    if (!obj)
        return;

    // If there is only 1 ship in a fleet, forget the fleet
    auto ship = std::dynamic_pointer_cast<const Ship>(obj);
    if (ship) {
        if (auto ship_s_fleet = GetUniverse().Objects().Object<const Fleet>(ship->FleetID())) {
            bool only_ship_in_fleet = ship_s_fleet->NumShips() == 1;
            if (only_ship_in_fleet)
                return ForgetObject(ship->FleetID());
        }
    }

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    HumanClientApp::GetApp()->Orders().IssueOrder(
        std::make_shared<ForgetOrder>(client_empire_id, obj->ID()));

    // Client changes for immediate effect
    // Force the client to change immediately.
    GetUniverse().ForgetKnownObject(ALL_EMPIRES, obj->ID());

    // Force a refresh
    RequirePreRender();

    // Update fleet wnd if needed
    if (auto fleet = std::dynamic_pointer_cast<const Fleet>(obj)) {
        RemoveFleet(fleet->ID());
        fleet->StateChangedSignal();
    }

    if (ship)
        ship->StateChangedSignal();
}

void MapWnd::DoSystemIconsLayout() {
    // position and resize system icons and gaseous substance
    const int SYSTEM_ICON_SIZE = SystemIconSize();
    for (auto& system_icon : m_system_icons) {
        auto system = GetSystem(system_icon.first);
        if (!system) {
            ErrorLogger() << "MapWnd::DoSystemIconsLayout couldn't get system with id " << system_icon.first;
            continue;
        }

        GG::Pt icon_ul(GG::X(static_cast<int>(system->X()*ZoomFactor() - SYSTEM_ICON_SIZE / 2.0)),
                       GG::Y(static_cast<int>(system->Y()*ZoomFactor() - SYSTEM_ICON_SIZE / 2.0)));
        system_icon.second->SizeMove(icon_ul, icon_ul + GG::Pt(GG::X(SYSTEM_ICON_SIZE), GG::Y(SYSTEM_ICON_SIZE)));
    }
}

void MapWnd::DoFieldIconsLayout() {
    // position and resize field icons
    for (auto& field_icon : m_field_icons) {
        auto field = GetField(field_icon.first);
        if (!field) {
            ErrorLogger() << "MapWnd::DoFieldIconsLayout couldn't get field with id " << field_icon.first;
            continue;
        }

        double RADIUS = ZoomFactor() * field->InitialMeterValue(METER_SIZE);    // Field's METER_SIZE gives the radius of the field

        GG::Pt icon_ul(GG::X(static_cast<int>(field->X()*ZoomFactor() - RADIUS)),
                       GG::Y(static_cast<int>(field->Y()*ZoomFactor() - RADIUS)));
        field_icon.second->SizeMove(icon_ul, icon_ul + GG::Pt(GG::X(2*RADIUS), GG::Y(2*RADIUS)));
    }
}

void MapWnd::DoFleetButtonsLayout() {
    const ObjectMap& objects = GetUniverse().Objects();

    auto place_system_fleet_btn = [this](const std::unordered_map<int, std::unordered_set<std::shared_ptr<FleetButton>>>::value_type& system_and_btns, bool is_departing) {
        // calculate system icon position
        auto system = GetSystem(system_and_btns.first);
        if (!system) {
            ErrorLogger() << "MapWnd::DoFleetButtonsLayout couldn't find system with id " << system_and_btns.first;
            return;
        }

        const int SYSTEM_ICON_SIZE = SystemIconSize();
        GG::Pt icon_ul(GG::X(static_cast<int>(system->X()*ZoomFactor() - SYSTEM_ICON_SIZE / 2.0)),
                       GG::Y(static_cast<int>(system->Y()*ZoomFactor() - SYSTEM_ICON_SIZE / 2.0)));

        // get system icon itself.  can't use the system icon's UpperLeft to position fleet button due to weirdness that results that I don't want to figure out
        const auto& sys_it = m_system_icons.find(system->ID());
        if (sys_it == m_system_icons.end()) {
            ErrorLogger() << "couldn't find system icon for fleet button in DoFleetButtonsLayout";
            return;
        }
        const auto& system_icon = sys_it->second;

        // place all buttons
        int n = 1;
        for (auto& button : system_and_btns.second) {
            GG::Pt ul = system_icon->NthFleetButtonUpperLeft(n, is_departing);
            ++n;
            button->MoveTo(ul + icon_ul);
        }
    };

    // position departing fleet buttons
    for (const auto& system_and_btns : m_departing_fleet_buttons)
        place_system_fleet_btn(system_and_btns, true);

    // position stationary fleet buttons
    for (const auto& system_and_btns : m_stationary_fleet_buttons)
        place_system_fleet_btn(system_and_btns, false);

    // position moving fleet buttons
    for (auto& lane_and_fbs : m_moving_fleet_buttons) {
        for (auto& fb : lane_and_fbs.second) {
            const GG::Pt FLEET_BUTTON_SIZE = fb->Size();
            std::shared_ptr<const Fleet> fleet;

            // skip button if it has no fleets (somehow...?) or if the first fleet in the button is 0
            if (fb->Fleets().empty() || !(fleet = objects.Object<Fleet>(*fb->Fleets().begin()))) {
                ErrorLogger() << "DoFleetButtonsLayout couldn't get first fleet for button";
                continue;
            }

            auto button_pos = MovingFleetMapPositionOnLane(fleet);
            if (!button_pos)
                continue;

            // position button
            GG::Pt button_ul(button_pos->first  * ZoomFactor() - FLEET_BUTTON_SIZE.x / 2.0,
                             button_pos->second * ZoomFactor() - FLEET_BUTTON_SIZE.y / 2.0);

            fb->MoveTo(button_ul);
        }
    }

    // position offroad fleet buttons
    for (auto& pos_and_fbs : m_offroad_fleet_buttons) {
        for (auto& fb : pos_and_fbs.second) {
            const GG::Pt FLEET_BUTTON_SIZE = fb->Size();
            std::shared_ptr<const Fleet> fleet;

            // skip button if it has no fleets (somehow...?) or if the first fleet in the button is 0
            if (fb->Fleets().empty() || !(fleet = objects.Object<Fleet>(*fb->Fleets().begin()))) {
                ErrorLogger() << "DoFleetButtonsLayout couldn't get first fleet for button";
                continue;
            }

            // position button
            auto& button_pos = pos_and_fbs.first;
            GG::Pt button_ul(button_pos.first  * ZoomFactor() - FLEET_BUTTON_SIZE.x / 2.0,
                             button_pos.second * ZoomFactor() - FLEET_BUTTON_SIZE.y / 2.0);

            fb->MoveTo(button_ul);
        }
    }

}

boost::optional<std::pair<double, double>> MapWnd::MovingFleetMapPositionOnLane(
    std::shared_ptr<const Fleet> fleet) const
{
    if (!fleet)
        return boost::none;

    // get endpoints of lane on screen
    int sys1_id = fleet->PreviousSystemID();
    int sys2_id = fleet->NextSystemID();

    // get apparent positions of endpoints for this lane that have been pre-calculated
    auto endpoints_it = m_starlane_endpoints.find({sys1_id, sys2_id});
    if (endpoints_it == m_starlane_endpoints.end()) {
        // couldn't find an entry for the lane this fleet is one, so just
        // return actual position of fleet on starlane - ignore the distance
        // away from the star centre at which starlane endpoints should appear
        return std::make_pair(fleet->X(), fleet->Y());
    }

    // return apparent position of fleet on starlane
    const LaneEndpoints& screen_lane_endpoints = endpoints_it->second;
    return ScreenPosOnStarlane(fleet->X(), fleet->Y(), sys1_id, sys2_id,
                              screen_lane_endpoints);
}

namespace {
    template <typename Key>
    using KeyToFleetsMap = std::unordered_map<Key, std::vector<int>, boost::hash<Key>>;
    using SystemXEmpireToFleetsMap = KeyToFleetsMap<std::pair<int, int>>;
    using LocationXEmpireToFleetsMap = KeyToFleetsMap<std::pair<std::pair<double, double>, int>>;
    using StarlaneToFleetsMap = KeyToFleetsMap<std::pair<int, int>>;

    /** Return fleet if \p obj is not destroyed, not stale, a fleet and not empty.*/
    std::shared_ptr<const Fleet> IsQualifiedFleet(const std::shared_ptr<const UniverseObject>& obj,
                                                  int empire_id,
                                                  const std::set<int>& known_destroyed_objects,
                                                  const std::set<int>& stale_object_info)
    {
        int object_id = obj->ID();
        auto fleet = std::dynamic_pointer_cast<const Fleet>(obj);

        if (fleet
            && !fleet->Empty()
            && (known_destroyed_objects.count(object_id) == 0)
            && (stale_object_info.count(object_id) == 0))
        {
            return fleet;
        }
        return nullptr;
    }

    /** If the \p fleet has orders and is departing from a valid system, return the system*/
    std::shared_ptr<const System> IsDepartingFromSystem(
        const std::shared_ptr<const Fleet>& fleet)
    {
        if (fleet->FinalDestinationID() != INVALID_OBJECT_ID
            && !fleet->TravelRoute().empty()
            && fleet->SystemID() != INVALID_OBJECT_ID)
        {
            auto system = GetSystem(fleet->SystemID());
            if (system)
                return system;
            ErrorLogger() << "Couldn't get system with id " << fleet->SystemID()
                          << " of a departing fleet named " << fleet->Name();
        }
        return nullptr;
    }

    /** If the \p fleet is stationary in a valid system, return the system*/
    std::shared_ptr<const System> IsStationaryInSystem(
        const std::shared_ptr<const Fleet>& fleet)
    {
        if ((fleet->FinalDestinationID() == INVALID_OBJECT_ID
             || fleet->TravelRoute().empty())
            && fleet->SystemID() != INVALID_OBJECT_ID)
        {
            auto system = GetSystem(fleet->SystemID());
            if (system)
                return system;
            ErrorLogger() << "Couldn't get system with id " << fleet->SystemID()
                          << " of a stationary fleet named " << fleet->Name();
        }
        return nullptr;
    }

    /** If the \p fleet has a valid destination and it not at a system, return true*/
    bool IsMoving(const std::shared_ptr<const Fleet>& fleet) {
        return (fleet->FinalDestinationID() != INVALID_OBJECT_ID
                && fleet->SystemID() == INVALID_OBJECT_ID);
    }

    /** Return the starlane's endpoints if the \p fleet is on a starlane. */
    boost::optional<std::pair<int, int>> IsOnStarlane(const std::shared_ptr<const Fleet>& fleet) {
        // get endpoints of lane on screen
        int sys1_id = fleet->PreviousSystemID();
        int sys2_id = fleet->NextSystemID();

        auto sys1 = GetSystem(sys1_id);
        if (!sys1)
            return boost::none;
        if (sys1->HasStarlaneTo(sys2_id))
            return std::make_pair(std::min(sys1_id, sys2_id), std::max(sys1_id, sys2_id));

        return boost::none;
    }

    /** If the \p fleet has a valid destination, is not at a system and is
        on a starlane, return the starlane's endpoint system ids */
    boost::optional<std::pair<int, int>> IsMovingOnStarlane(const std::shared_ptr<const Fleet>& fleet) {
        if (!IsMoving(fleet))
            return boost::none;

        return IsOnStarlane(fleet);
    }

    /** If the \p fleet has a valid destination and it not on a starlane, return true*/
    bool IsOffRoad(const std::shared_ptr<const Fleet>& fleet)
    { return (IsMoving(fleet) && !IsOnStarlane(fleet)); }
}

void MapWnd::RefreshFleetButtons() {
    RequirePreRender();
    m_deferred_refresh_fleet_buttons = true;
}

void MapWnd::DeferredRefreshFleetButtons() {
    if (!m_deferred_refresh_fleet_buttons)
        return;
    m_deferred_refresh_fleet_buttons = false;

    ScopedTimer timer("RefreshFleetButtons()");

    // determine fleets that need buttons so that fleets at the same location can
    // be grouped by empire owner and buttons created

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    const auto& this_client_known_destroyed_objects =
        GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id);
    const auto& this_client_stale_object_info =
        GetUniverse().EmpireStaleKnowledgeObjectIDs(client_empire_id);

    SystemXEmpireToFleetsMap   departing_fleets;
    SystemXEmpireToFleetsMap   stationary_fleets;
    m_moving_fleets.clear();
    LocationXEmpireToFleetsMap moving_fleets;
    LocationXEmpireToFleetsMap offroad_fleets;

    for (const auto& entry : Objects().ExistingFleets()) {
        auto fleet = IsQualifiedFleet(entry.second, client_empire_id,
                                      this_client_known_destroyed_objects,
                                      this_client_stale_object_info);

        if (!fleet)
            continue;

        // Collect fleets with a travel route just departing.
        if (auto departure_system = IsDepartingFromSystem(fleet)) {
            departing_fleets[{departure_system->ID(), fleet->Owner()}].push_back(fleet->ID());

            // Collect stationary fleets by system.
        } else if (auto stationary_system = IsStationaryInSystem(fleet)) {
            // DebugLogger() << fleet->Name() << " is Stationary." ;
            stationary_fleets[{stationary_system->ID(), fleet->Owner()}].push_back(fleet->ID());

            // Collect traveling fleets between systems by which starlane they
            // are on (ignoring location on lane and direction of travel)
        } else if (auto starlane_end_systems = IsMovingOnStarlane(fleet)) {
            moving_fleets[{*starlane_end_systems, fleet->Owner()}].push_back(fleet->ID());
            m_moving_fleets[*starlane_end_systems].push_back(fleet->ID());

        } else if (IsOffRoad(fleet)) {
            offroad_fleets[{{fleet->X(), fleet->Y()}, fleet->Owner()}].push_back(fleet->ID());

        } else {
            ErrorLogger() << "Fleet "<< fleet->Name() <<"(" << fleet->ID()
                          << ") is not stationary, departing from a system or in transit."
                          << " final dest id is " << fleet->FinalDestinationID()
                          << " travel routes is of length = " << fleet->TravelRoute().size()
                          << " system id is " << fleet->SystemID()
                          << " location is (" << fleet->X() << "," <<fleet->Y() << ")";
        }
    }

    DeleteFleetButtons();

    // create new fleet buttons for fleets...
    const auto FLEETBUTTON_SIZE = FleetButtonSizeType();
    CreateFleetButtonsOfType(m_departing_fleet_buttons,  departing_fleets,  FLEETBUTTON_SIZE);
    CreateFleetButtonsOfType(m_stationary_fleet_buttons, stationary_fleets, FLEETBUTTON_SIZE);
    CreateFleetButtonsOfType(m_moving_fleet_buttons,     moving_fleets,     FLEETBUTTON_SIZE);
    CreateFleetButtonsOfType(m_offroad_fleet_buttons,    offroad_fleets,    FLEETBUTTON_SIZE);

    // position fleetbuttons
    DoFleetButtonsLayout();

    // add selection indicators to fleetbuttons
    RefreshFleetButtonSelectionIndicators();

    // create movement lines (after positioning buttons, so lines will originate from button location)
    for (const auto& fleet_button : m_fleet_buttons)
        SetFleetMovementLine(fleet_button.first);
}

template <typename FleetButtonMap, typename FleetsMap>
void MapWnd::CreateFleetButtonsOfType(FleetButtonMap& type_fleet_buttons,
                                      const FleetsMap& fleets_map,
                                      const FleetButton::SizeType& fleet_button_size)
{
    for (const auto& fleets : fleets_map) {
        const auto& key = fleets.first.first;

        // buttons need fleet IDs
        const auto& fleet_IDs = fleets.second;
        if (fleet_IDs.empty())
            continue;

        // sort fleets by position
        std::map<std::pair<double, double>, std::vector<int>> fleet_positions_ids;
        for (int id : fleet_IDs) {
            const auto fleet = GetFleet(id);
            if (!fleet)
                continue;
            fleet_positions_ids[{fleet->X(), fleet->Y()}].push_back(id);
        }

        // create separate FleetButton for each cluster of fleets
        for (const auto& cluster : fleet_positions_ids) {
            const auto& ids_in_cluster = cluster.second;

            // create new fleetbutton for this cluster of fleets
            auto fb = GG::Wnd::Create<FleetButton>(ids_in_cluster, fleet_button_size);

            // store per type of fleet button.
            type_fleet_buttons[key].insert(fb);

            // store FleetButton for fleets in current cluster
            for (int fleet_id : ids_in_cluster)
                m_fleet_buttons[fleet_id] = fb;

            fb->LeftClickedSignal.connect(
                boost::bind(&MapWnd::FleetButtonLeftClicked, this, fb.get()));
            fb->RightClickedSignal.connect(
                boost::bind(&MapWnd::FleetButtonRightClicked, this, fb.get()));
            AttachChild(std::move(fb));
        }
    }
}

void MapWnd::DeleteFleetButtons() {
    for (auto& id_and_fb : m_fleet_buttons)
        DetachChild(id_and_fb.second);
    m_fleet_buttons.clear();

    // The pointers in the following containers duplicate those in
    // m_fleet_buttons and therefore don't need to be detached.
    m_stationary_fleet_buttons.clear();
    m_departing_fleet_buttons.clear();
    m_moving_fleet_buttons.clear();
    m_offroad_fleet_buttons.clear();
}

void MapWnd::RemoveFleetsStateChangedSignal(const std::vector<std::shared_ptr<Fleet>>& fleets) {
    ScopedTimer timer("RemoveFleetsStateChangedSignal()", true);
    for (auto& fleet : fleets) {
        auto found_signal = m_fleet_state_change_signals.find(fleet->ID());
        if (found_signal != m_fleet_state_change_signals.end()) {
            found_signal->second.disconnect();
            m_fleet_state_change_signals.erase(found_signal);
        }
    }
}

void MapWnd::AddFleetsStateChangedSignal(const std::vector<std::shared_ptr<Fleet>>& fleets) {
    ScopedTimer timer("AddFleetsStateChangedSignal()", true);
    for (auto& fleet : fleets) {
        m_fleet_state_change_signals[fleet->ID()] = fleet->StateChangedSignal.connect(
            boost::bind(&MapWnd::RefreshFleetButtons, this));
    }
}

void MapWnd::FleetsInsertedSignalHandler(const std::vector<std::shared_ptr<Fleet>>& fleets) {
    ScopedTimer timer("FleetsInsertedSignalHandler()", true);
    RefreshFleetButtons();
    RemoveFleetsStateChangedSignal(fleets);
    AddFleetsStateChangedSignal(fleets);
}

void MapWnd::FleetsRemovedSignalHandler(const std::vector<std::shared_ptr<Fleet>>& fleets) {
    ScopedTimer timer("FleetsRemovedSignalHandler()", true);
    RefreshFleetButtons();
    RemoveFleetsStateChangedSignal(fleets);
}

void MapWnd::RefreshFleetSignals() {
    ScopedTimer timer("RefreshFleetSignals()", true);
    // disconnect old fleet statechangedsignal connections
    for (auto& con : m_fleet_state_change_signals)
    { con.second.disconnect(); }
    m_fleet_state_change_signals.clear();


    // connect fleet change signals to update fleet movement lines, so that ordering
    // fleets to move updates their displayed path and rearranges fleet buttons (if necessary)
    auto fleets = Objects().FindObjects<Fleet>();
    AddFleetsStateChangedSignal(fleets);
}

void MapWnd::RefreshSliders() {
    if (m_zoom_slider) {
        if (GetOptionsDB().Get<bool>("ui.map.zoom.slider.shown") && Visible())
            m_zoom_slider->Show();
        else
            m_zoom_slider->Hide();
    }
}

int MapWnd::SystemIconSize() const
{ return static_cast<int>(ClientUI::SystemIconSize() * ZoomFactor()); }

int MapWnd::SystemNamePts() const {
    const int       SYSTEM_NAME_MINIMUM_PTS = 6;    // limit to absolute minimum point size
    const double    MAX_NAME_ZOOM_FACTOR = 1.5;     // limit to relative max above standard UI font size
    const double    NAME_ZOOM_FACTOR = std::min(MAX_NAME_ZOOM_FACTOR, ZoomFactor());
    const int       ZOOMED_PTS = static_cast<int>(ClientUI::Pts() * NAME_ZOOM_FACTOR);
    return std::max(ZOOMED_PTS, SYSTEM_NAME_MINIMUM_PTS);
}

double MapWnd::SystemHaloScaleFactor() const
{ return 1.0 + log10(ZoomFactor()); }

FleetButton::SizeType MapWnd::FleetButtonSizeType() const {
    // no SizeType::LARGE as these icons are too big for the map.  (they can be used in the FleetWnd, however)
    if      (ZoomFactor() > ClientUI::MediumFleetButtonZoomThreshold())
        return FleetButton::SizeType::MEDIUM;

    else if (ZoomFactor() > ClientUI::SmallFleetButtonZoomThreshold())
        return FleetButton::SizeType::SMALL;

    else if (ZoomFactor() > ClientUI::TinyFleetButtonZoomThreshold())
        return FleetButton::SizeType::TINY;

    else
        return FleetButton::SizeType::NONE;
}

void MapWnd::Zoom(int delta) {
    GG::Pt center = GG::Pt(AppWidth() / 2.0, AppHeight() / 2.0);
    Zoom(delta, center);
}

void MapWnd::Zoom(int delta, const GG::Pt& position) {
    if (delta == 0)
        return;

    // increment zoom steps in by delta steps
    double new_zoom_steps_in = m_zoom_steps_in + static_cast<double>(delta);
    SetZoom(new_zoom_steps_in, true, position);
}

void MapWnd::SetZoom(double steps_in, bool update_slide) {
    GG::Pt center = GG::Pt(AppWidth() / 2.0, AppHeight() / 2.0);
    SetZoom(steps_in, update_slide, center);
}

void MapWnd::SetZoom(double steps_in, bool update_slide, const GG::Pt& position) {
    // impose range limits on zoom steps
    double new_steps_in = std::max(std::min(steps_in, ZOOM_IN_MAX_STEPS), ZOOM_IN_MIN_STEPS);

    // abort if no change
    if (new_steps_in == m_zoom_steps_in)
        return;


    // save position offsets and old zoom factors
    GG::Pt                      ul =                    ClientUpperLeft();
    const GG::X_d               center_x =              AppWidth() / 2.0;
    const GG::Y_d               center_y =              AppHeight() / 2.0;
    GG::X_d                     ul_offset_x =           ul.x - center_x;
    GG::Y_d                     ul_offset_y =           ul.y - center_y;
    const double                OLD_ZOOM =              ZoomFactor();
    const FleetButton::SizeType OLD_FLEETBUTTON_SIZE =  FleetButtonSizeType();


    // set new zoom level
    m_zoom_steps_in = new_steps_in;


    // keeps position the same after zooming
    // used to keep the mouse at the same position when doing mouse wheel zoom
    const GG::Pt position_center_delta = GG::Pt(position.x - center_x, position.y - center_y);
    ul_offset_x -= position_center_delta.x;
    ul_offset_y -= position_center_delta.y;

    // correct map offsets for zoom changes
    ul_offset_x *= (ZoomFactor() / OLD_ZOOM);
    ul_offset_y *= (ZoomFactor() / OLD_ZOOM);

    // now add the zoom position offset at the new zoom level
    ul_offset_x += position_center_delta.x;
    ul_offset_y += position_center_delta.y;

    // show or hide system names, depending on zoom.  replicates code in MapWnd::Zoom
    if (ZoomFactor() * ClientUI::Pts() < MIN_SYSTEM_NAME_SIZE)
        HideSystemNames();
    else
        ShowSystemNames();


    DoSystemIconsLayout();
    DoFieldIconsLayout();


    // if fleet buttons need to change size, need to fully refresh them (clear
    // and recreate).  If they are the same size as before the zoom, then can
    // just reposition them without recreating
    const FleetButton::SizeType NEW_FLEETBUTTON_SIZE = FleetButtonSizeType();
    if (OLD_FLEETBUTTON_SIZE != NEW_FLEETBUTTON_SIZE)
        RefreshFleetButtons();
    else
        DoFleetButtonsLayout();


    // move field icons to bottom of child stack so that other icons can be moused over with a field
    for (const auto& field_icon : m_field_icons)
        MoveChildDown(field_icon.second);


    // translate map and UI widgets to account for the change in upper left due to zooming
    GG::Pt map_move(static_cast<GG::X>((center_x + ul_offset_x) - ul.x),
                    static_cast<GG::Y>((center_y + ul_offset_y) - ul.y));
    OffsetMove(map_move);

    // this correction ensures that zooming in doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);

    MoveTo(move_to_pt - GG::Pt(AppWidth(), AppHeight()));

    int sel_system_id = SidePanel::SystemID();
    if (m_scale_line)
        m_scale_line->Update(ZoomFactor(), m_selected_fleet_ids, sel_system_id);
    if (update_slide && m_zoom_slider)
        m_zoom_slider->SlideTo(m_zoom_steps_in);

    InitScaleCircleRenderingBuffer();

    ZoomedSignal(ZoomFactor());
}

void MapWnd::CorrectMapPosition(GG::Pt& move_to_pt) {
    GG::X contents_width(static_cast<int>(ZoomFactor() * GetUniverse().UniverseWidth()));
    GG::X app_width =  AppWidth();
    GG::Y app_height = AppHeight();
    GG::X map_margin_width(app_width);

    //std::cout << "MapWnd::CorrectMapPosition appwidth: " << Value(app_width) << " appheight: " << Value(app_height)
    //          << " to_x: " << Value(move_to_pt.x) << " to_y: " << Value(move_to_pt.y) << std::endl;;

    // restrict map positions to prevent map from being dragged too far off screen.
    // add extra padding to restrictions when universe to be shown is larger than
    // the screen area in which to show it.
    if (app_width - map_margin_width < contents_width || Value(app_height) - map_margin_width < contents_width) {
        if (map_margin_width < move_to_pt.x)
            move_to_pt.x = map_margin_width;
        if (move_to_pt.x + contents_width < app_width - map_margin_width)
            move_to_pt.x = app_width - map_margin_width - contents_width;
        if (map_margin_width < Value(move_to_pt.y))
            move_to_pt.y = GG::Y(Value(map_margin_width));
        if (Value(move_to_pt.y) + contents_width < Value(app_height) - map_margin_width)
            move_to_pt.y = app_height - Value(map_margin_width) - Value(contents_width);
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

void MapWnd::FieldRightClicked(int field_id) {
    if (ClientPlayerIsModerator()) {
        ModeratorActionSetting mas = m_moderator_wnd->SelectedAction();
        ClientNetworking& net = HumanClientApp::GetApp()->Networking();

        if (mas == MAS_Destroy) {
            net.SendMessage(ModeratorActionMessage(Moderator::DestroyUniverseObject(field_id)));
        }
        return;
    }
}

void MapWnd::SystemDoubleClicked(int system_id) {
    if (!m_in_production_view_mode) {
        if (!m_production_wnd->Visible())
            ToggleProduction();
        CenterOnObject(system_id);
        m_production_wnd->SelectSystem(system_id);
    }
}

void MapWnd::SystemLeftClicked(int system_id) {
    SelectSystem(system_id);
    SystemLeftClickedSignal(system_id);
}

void MapWnd::SystemRightClicked(int system_id, GG::Flags<GG::ModKey> mod_keys) {
    if (ClientPlayerIsModerator()) {
        ModeratorActionSetting mas = m_moderator_wnd->SelectedAction();
        ClientNetworking& net = HumanClientApp::GetApp()->Networking();

        if (mas == MAS_Destroy) {
            net.SendMessage(ModeratorActionMessage(
                Moderator::DestroyUniverseObject(system_id)));

        } else if (mas == MAS_CreatePlanet) {
            net.SendMessage(ModeratorActionMessage(
                Moderator::CreatePlanet(system_id, m_moderator_wnd->SelectedPlanetType(),
                                        m_moderator_wnd->SelectedPlanetSize())));

        } else if (mas == MAS_AddStarlane) {
            int selected_system_id = SidePanel::SystemID();
            if (GetSystem(selected_system_id)) {
                net.SendMessage(ModeratorActionMessage(
                    Moderator::AddStarlane(system_id, selected_system_id)));
            }

        } else if (mas == MAS_RemoveStarlane) {
            int selected_system_id = SidePanel::SystemID();
            if (GetSystem(selected_system_id)) {
                net.SendMessage(ModeratorActionMessage(
                    Moderator::RemoveStarlane(system_id, selected_system_id)));
            }
        } else if (mas == MAS_SetOwner) {
            int empire_id = m_moderator_wnd->SelectedEmpire();
            auto system = GetSystem(system_id);
            if (!system)
                return;

            for (auto& obj : Objects().FindObjects<const UniverseObject>(system->ContainedObjectIDs())) {
                UniverseObjectType obj_type = obj->ObjectType();
                if (obj_type >= OBJ_BUILDING && obj_type < OBJ_SYSTEM) {
                    net.SendMessage(ModeratorActionMessage(
                    Moderator::SetOwner(obj->ID(), empire_id)));
                }
            }
        }
    }

    if (!m_in_production_view_mode && FleetUIManager::GetFleetUIManager().ActiveFleetWnd()) {
        if (system_id == INVALID_OBJECT_ID)
            ClearProjectedFleetMovementLines();
        else
            PlotFleetMovement(system_id, true, mod_keys &  GG::MOD_KEY_SHIFT);
        SystemRightClickedSignal(system_id);
    }
}

void MapWnd::MouseEnteringSystem(int system_id, GG::Flags<GG::ModKey> mod_keys) {
    if (ClientPlayerIsModerator()) {
        //
    } else {
        if (!m_in_production_view_mode)
            PlotFleetMovement(system_id, false, mod_keys & GG::MOD_KEY_SHIFT);
    }
    SystemBrowsedSignal(system_id);
}

void MapWnd::MouseLeavingSystem(int system_id)
{ MouseEnteringSystem(INVALID_OBJECT_ID, GG::Flags<GG::ModKey>()); }

void MapWnd::PlanetDoubleClicked(int planet_id) {
    if (planet_id == INVALID_OBJECT_ID)
        return;

    // retrieve system_id from planet_id
    auto planet = GetPlanet(planet_id);
    if (!planet)
        return;

    // open production screen
    if (!m_in_production_view_mode) {
        if (!m_production_wnd->Visible())
            ToggleProduction();
        CenterOnObject(planet->SystemID());
        m_production_wnd->SelectSystem(planet->SystemID());
        m_production_wnd->SelectPlanet(planet_id);
    }
}

void MapWnd::PlanetRightClicked(int planet_id) {
    if (planet_id == INVALID_OBJECT_ID)
        return;
    if (!ClientPlayerIsModerator())
        return;

    ModeratorActionSetting mas = m_moderator_wnd->SelectedAction();
    ClientNetworking& net = HumanClientApp::GetApp()->Networking();

    if (mas == MAS_Destroy) {
        net.SendMessage(ModeratorActionMessage(
            Moderator::DestroyUniverseObject(planet_id)));
    } else if (mas == MAS_SetOwner) {
        int empire_id = m_moderator_wnd->SelectedEmpire();
        net.SendMessage(ModeratorActionMessage(
            Moderator::SetOwner(planet_id, empire_id)));
    }
}

void MapWnd::BuildingRightClicked(int building_id) {
    if (building_id == INVALID_OBJECT_ID)
        return;
    if (!ClientPlayerIsModerator())
        return;

    ModeratorActionSetting mas = m_moderator_wnd->SelectedAction();
    ClientNetworking& net = HumanClientApp::GetApp()->Networking();

    if (mas == MAS_Destroy) {
        net.SendMessage(ModeratorActionMessage(
            Moderator::DestroyUniverseObject(building_id)));
    } else if (mas == MAS_SetOwner) {
        int empire_id = m_moderator_wnd->SelectedEmpire();
        net.SendMessage(ModeratorActionMessage(
            Moderator::SetOwner(building_id, empire_id)));
    }
}

void MapWnd::ReplotProjectedFleetMovement(bool append) {
    DebugLogger() << "MapWnd::ReplotProjectedFleetMovement" << (append?" append":"");
    for (const auto& fleet_line : m_projected_fleet_lines) {
        const MovementLineData& data = fleet_line.second;
        if (!data.path.empty()) {
            int target = data.path.back().object_id;
            if (target != INVALID_OBJECT_ID) {
                PlotFleetMovement(target, false, append);
            }
        }
    }
}

void MapWnd::PlotFleetMovement(int system_id, bool execute_move, bool append) {
    if (!FleetUIManager::GetFleetUIManager().ActiveFleetWnd())
        return;

    DebugLogger() << "PlotFleetMovement " << (execute_move?" execute":"") << (append?" append":"");

    int empire_id = HumanClientApp::GetApp()->EmpireID();

    auto fleet_ids = FleetUIManager::GetFleetUIManager().ActiveFleetWnd()->SelectedFleetIDs();

    // apply to all selected this-player-owned fleets in currently-active FleetWnd
    for (int fleet_id : fleet_ids) {
        auto fleet = GetFleet(fleet_id);
        if (!fleet) {
            ErrorLogger() << "MapWnd::PlotFleetMovementLine couldn't get fleet with id " << fleet_id;
            continue;
        }

        // only give orders / plot prospective move paths of fleets owned by player
        if (!(fleet->OwnedBy(empire_id)) || !(fleet->NumShips()))
            continue;

        // plot empty move pathes if destination is not a known system
        if (system_id == INVALID_OBJECT_ID) {
            m_projected_fleet_lines.erase(fleet_id);
            continue;
        }

        int fleet_sys_id = fleet->SystemID();
        if (append && !fleet->TravelRoute().empty()) {
            fleet_sys_id = fleet->TravelRoute().back();
        }

        int start_system = fleet_sys_id;
        if (fleet_sys_id == INVALID_OBJECT_ID)
            start_system = fleet->NextSystemID();

        // get path to destination...
        std::list<int> route = GetPathfinder()->ShortestPath(start_system, system_id, empire_id).first;
        // Prepend a non-empty old_route to the beginning of route.
        if (append && !fleet->TravelRoute().empty()) {
            std::list<int> old_route(fleet->TravelRoute());
            old_route.erase(--old_route.end()); //end of old is begin of new
            route.splice(route.begin(), old_route);
        }

        // disallow "offroad" (direct non-starlane non-wormhole) travel
        if (route.size() == 2 && *route.begin() != *route.rbegin()) {
            int begin_id = *route.begin();
            auto begin_sys = GetSystem(begin_id);
            int end_id = *route.rbegin();
            auto end_sys = GetSystem(end_id);

            if (!begin_sys->HasStarlaneTo(end_id) && !begin_sys->HasWormholeTo(end_id) &&
                !end_sys->HasStarlaneTo(begin_id) && !end_sys->HasWormholeTo(begin_id))
            {
                continue;
            }
        }

        // if actually ordering fleet movement, not just prospectively previewing, ... do so
        if (execute_move && !route.empty()){
            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<FleetMoveOrder>(empire_id, fleet_id, system_id, append));
            StopFleetExploring(fleet_id);
        }

        // show route on map
        SetProjectedFleetMovementLine(fleet_id, route);
    }
}

std::vector<int> MapWnd::FleetIDsOfFleetButtonsOverlapping(int fleet_id) const {
    std::vector<int> fleet_ids;

    auto fleet = GetFleet(fleet_id);
    if (!fleet) {
        ErrorLogger() << "MapWnd::FleetIDsOfFleetButtonsOverlapping: Fleet id "
                      << fleet_id << " does not exist.";
        return fleet_ids;
    }

    const auto& it = m_fleet_buttons.find(fleet_id);
    if (it == m_fleet_buttons.end()) {
        // Log that a FleetButton could not be found for the requested fleet, and include when the fleet was last seen
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        auto vis_turn_map = GetUniverse().GetObjectVisibilityTurnMapByEmpire(fleet_id, empire_id);
        int vis_turn = -1;
        if (vis_turn_map.find(VIS_BASIC_VISIBILITY) != vis_turn_map.end())
            vis_turn = vis_turn_map[VIS_BASIC_VISIBILITY];
        ErrorLogger() << "Couldn't find a FleetButton for fleet " << fleet_id
                      << " with last basic vis turn " << vis_turn;
        return fleet_ids;
    }
    const auto& fleet_btn = it->second;

    // A check if a button overlaps the fleet button
    auto overlaps_fleet_btn = [&fleet_btn](const FleetButton& test_fb) {
        GG::Pt center = GG::Pt((fleet_btn->Left() + fleet_btn->Right()) / 2,
                               (fleet_btn->Top() + fleet_btn->Bottom()) /2);

        bool retval = test_fb.InWindow(center)
                   || test_fb.InWindow(fleet_btn->UpperLeft())
                   || test_fb.InWindow(GG::Pt(fleet_btn->Right(), fleet_btn->Top()))
                   || test_fb.InWindow(GG::Pt(fleet_btn->Left(), fleet_btn->Bottom()))
                   || test_fb.InWindow(fleet_btn->LowerRight());

        //std::cout << "FleetButton with fleets: ";
        //for (const auto entry : test_fb.Fleets())
        //    std::cout << entry << " ";
        //if (retval)
        //    std::cout << "  overlaps FleetButton with fleets: ";
        //else
        //    std::cout << "  does not overlap FleetButton with fleets: ";
        //for (const auto entry : fleet_btn->Fleets())
        //    std::cout << entry << " ";
        //std::cout << std::endl;

        return retval;
    };

    // There are 4 types of fleet buttons: moving on a starlane, offroad,
    // and stationary or departing from a system.

    // Moving fleet buttons only overlap with fleet buttons on the same starlane
    if (const auto starlane_end_systems = IsMovingOnStarlane(fleet)) {
        const auto& lane_btns_it = m_moving_fleet_buttons.find(*starlane_end_systems);
        if (lane_btns_it == m_moving_fleet_buttons.end())
            return fleet_ids;

        // Add all fleets for each overlapping button on the starlane
        for (const auto& test_fb : lane_btns_it->second)
            if (overlaps_fleet_btn(*test_fb))
                std::copy(test_fb->Fleets().begin(), test_fb->Fleets().end(), std::back_inserter(fleet_ids));

        return fleet_ids;
    }

    // Offroad fleet buttons only overlap other offroad fleet buttons.
    if (IsOffRoad(fleet)) {
        // This scales poorly (linearly) with increasing universe size if
        // offroading is common.
        for (const auto& pos_and_fbs: m_offroad_fleet_buttons) {
            const auto& fbs = pos_and_fbs.second;
            if (fbs.empty())
                continue;

            // Since all buttons are at the same position, only check if the first
            // button overlaps fleet_btn
            if (!overlaps_fleet_btn(**fbs.begin()))
                continue;

            // Add all fleets for all fleet buttons to btn_fleet
            for (const auto& overlapped_fb: fbs) {
                std::copy(overlapped_fb->Fleets().begin(), overlapped_fb->Fleets().end(),
                          std::back_inserter(fleet_ids));
            }
        }

        return fleet_ids;
    }

    // Stationary and departing fleet buttons should not overlap with each
    // other because of their offset placement for each empire.
    return fleet_btn->Fleets();
}

std::vector<int> MapWnd::FleetIDsOfFleetButtonsOverlapping(const FleetButton& fleet_btn) const {
    // get possible fleets to select from, and a pointer to one of those fleets
    if (fleet_btn.Fleets().empty()) {
        ErrorLogger() << "Clicked FleetButton contained no fleets!";
        return std::vector<int>();
    }

    // Add any overlapping fleet buttons for moving or offroad fleets.
    const auto overlapped_fleets = FleetIDsOfFleetButtonsOverlapping(fleet_btn.Fleets()[0]);

    if (overlapped_fleets.empty())
        ErrorLogger() << "Clicked FleetButton and overlapping buttons contained no fleets!";
    return overlapped_fleets;
}

void MapWnd::FleetButtonLeftClicked(const FleetButton* fleet_btn) {
    if (!fleet_btn)
        return;

    // allow switching to fleetView even when in production mode
    if (m_in_production_view_mode)
        HideProduction();

    // Add any overlapping fleet buttons for moving or offroad fleets.
    const auto fleet_ids_to_include_in_fleet_wnd = FleetIDsOfFleetButtonsOverlapping(*fleet_btn);
    if (fleet_ids_to_include_in_fleet_wnd.empty())
        return;

    int already_selected_fleet_id = INVALID_OBJECT_ID;

    // Find if a FleetWnd for these fleet(s) is already open, and if so, if there
    // is a single selected fleet in the window, and if so, what fleet that is

    // Note: The shared_ptr<FleetWnd> scope is confined to this if block, so that
    // SelectFleet below can delete the FleetWnd and re-use the CUIWnd config from
    // OptionsDB if needed.
    if (const auto& wnd_for_button = FleetUIManager::GetFleetUIManager().WndForFleetIDs(fleet_ids_to_include_in_fleet_wnd)) {
        // check which fleet(s) is/are selected in the button's FleetWnd
        auto selected_fleet_ids = wnd_for_button->SelectedFleetIDs();
        //std::cout << "Initially selected fleets: " << selected_fleet_ids.size() << " : ";
        //for (auto id : selected_fleet_ids)
        //    std::cout << id << " ";
        //std::cout << std::endl;

        // record selected fleet if just one fleet is selected.  otherwise, keep default
        // INVALID_OBJECT_ID to indicate that no single fleet is selected
        if (selected_fleet_ids.size() == 1)
            already_selected_fleet_id = *(selected_fleet_ids.begin());
    }


    // pick fleet to select from fleets represented by the clicked FleetButton.
    int fleet_to_select_id = INVALID_OBJECT_ID;


    // fleet_ids are the ids of the clicked and nearby buttons, but when
    // clicking a button, only the fleets in that button should be cycled
    // through.
    const auto& selectable_fleet_ids = fleet_btn->Fleets();

    if (already_selected_fleet_id == INVALID_OBJECT_ID || selectable_fleet_ids.size() == 1) {
        // no (single) fleet is already selected, or there is only one selectable fleet,
        // so select first fleet in button
        fleet_to_select_id = *selectable_fleet_ids.begin();

    } else {
        // select next fleet after already-selected fleet, or first fleet if already-selected
        // fleet is the last fleet in the button.

        // to do this, scan through button's fleets to find already_selected_fleet
        bool found_already_selected_fleet = false;
        for (auto it = selectable_fleet_ids.begin(); it != selectable_fleet_ids.end(); ++it) {
            if (*it == already_selected_fleet_id) {
                // found already selected fleet.  get NEXT fleet.  don't need to worry about
                // there not being enough fleets to do this because if above checks for case
                // of there being only one fleet in this button
                ++it;
                // if next fleet iterator is past end of fleets, loop around to first fleet
                if (it == selectable_fleet_ids.end())
                    it = selectable_fleet_ids.begin();
                // get fleet to select out of iterator
                fleet_to_select_id = *it;
                found_already_selected_fleet = true;
                break;
            }
        }

        if (!found_already_selected_fleet) {
            // didn't find already-selected fleet.  the selected fleet might have been moving when the
            // click button was for stationary fleets, or vice versa.  regardless, just default back
            // to selecting the first fleet for this button
            fleet_to_select_id = *selectable_fleet_ids.begin();
        }
    }


    // select chosen fleet
    if (fleet_to_select_id != INVALID_OBJECT_ID)
        SelectFleet(fleet_to_select_id);
}

void MapWnd::FleetButtonRightClicked(const FleetButton* fleet_btn) {
    if (!fleet_btn)
        return;

    // Add any overlapping fleet buttons for moving or offroad fleets.
    const auto fleet_ids = FleetIDsOfFleetButtonsOverlapping(*fleet_btn);
    if (fleet_ids.empty())
        return;

    // if fleetbutton holds currently not visible fleets, offer to dismiss them
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    std::vector<int> sensor_ghosts;

    // find sensor ghosts
    for (int fleet_id : fleet_ids) {
        auto fleet = GetFleet(fleet_id);
        if (!fleet)
            continue;
        if (fleet->OwnedBy(empire_id))
            continue;
        if (GetUniverse().GetObjectVisibilityByEmpire(fleet_id, empire_id) >= VIS_BASIC_VISIBILITY)
            continue;
        sensor_ghosts.push_back(fleet_id);
    }

    // should there be sensor ghosts, offer to dismiss them
    if (sensor_ghosts.size() > 0) {
        auto popup = GG::Wnd::Create<CUIPopupMenu>(fleet_btn->LowerRight().x, fleet_btn->LowerRight().y);

        auto forget_fleet_actions = [this, empire_id, sensor_ghosts]() {
            for (auto fleet_id : sensor_ghosts) {
                ForgetObject(fleet_id);
            }
        };

        popup->AddMenuItem(GG::MenuItem(UserString("FW_ORDER_DISMISS_SENSOR_GHOST_ALL"), false, false, forget_fleet_actions));
        popup->Run();

        // Force a redraw
        RequirePreRender();
        auto fleet_wnd = FleetUIManager::GetFleetUIManager().ActiveFleetWnd();
        if (fleet_wnd)
            fleet_wnd->RequirePreRender();
    }

    FleetsRightClicked(fleet_ids);
}

void MapWnd::FleetRightClicked(int fleet_id) {
    if (fleet_id == INVALID_OBJECT_ID)
        return;
    std::vector<int> fleet_ids;
    fleet_ids.push_back(fleet_id);
    FleetsRightClicked(fleet_ids);
}

void MapWnd::FleetsRightClicked(const std::vector<int>& fleet_ids) {
    if (fleet_ids.empty())
        return;
    if (!ClientPlayerIsModerator())
        return;

    ModeratorActionSetting mas = m_moderator_wnd->SelectedAction();
    ClientNetworking& net = HumanClientApp::GetApp()->Networking();

    if (mas == MAS_Destroy) {
        for (int fleet_id : fleet_ids) {
            net.SendMessage(ModeratorActionMessage(
                Moderator::DestroyUniverseObject(fleet_id)));
        }
    } else if (mas == MAS_SetOwner) {
        int empire_id = m_moderator_wnd->SelectedEmpire();
        for (int fleet_id : fleet_ids) {
            net.SendMessage(ModeratorActionMessage(
                Moderator::SetOwner(fleet_id, empire_id)));
        }
    }
}

void MapWnd::ShipRightClicked(int ship_id) {
    if (ship_id == INVALID_OBJECT_ID)
        return;
    std::vector<int> ship_ids;
    ship_ids.push_back(ship_id);
    ShipsRightClicked(ship_ids);
}

void MapWnd::ShipsRightClicked(const std::vector<int>& ship_ids) {
    if (ship_ids.empty())
        return;
    if (!ClientPlayerIsModerator())
        return;

    ModeratorActionSetting mas = m_moderator_wnd->SelectedAction();
    ClientNetworking& net = HumanClientApp::GetApp()->Networking();

    if (mas == MAS_Destroy) {
        for (int ship_id : ship_ids) {
            net.SendMessage(ModeratorActionMessage(
                Moderator::DestroyUniverseObject(ship_id)));
        }
    } else if (mas == MAS_SetOwner) {
        int empire_id = m_moderator_wnd->SelectedEmpire();
        for (int ship_id : ship_ids) {
            net.SendMessage(ModeratorActionMessage(
                Moderator::SetOwner(ship_id, empire_id)));
        }
    }
}

void MapWnd::SelectedFleetsChanged() {
    // get selected fleets
    std::set<int> selected_fleet_ids;
    if (const FleetWnd* fleet_wnd = FleetUIManager::GetFleetUIManager().ActiveFleetWnd())
        selected_fleet_ids = fleet_wnd->SelectedFleetIDs();

    // if old and new sets of selected fleets are the same, don't need to change anything
    if (selected_fleet_ids == m_selected_fleet_ids)
        return;

    // set new selected fleets
    m_selected_fleet_ids = selected_fleet_ids;

    // update fleetbutton selection indicators
    RefreshFleetButtonSelectionIndicators();
}

void MapWnd::SelectedShipsChanged() {
    ScopedTimer timer("MapWnd::SelectedShipsChanged", true);

    // get selected ships
    std::set<int> selected_ship_ids;
    if (const FleetWnd* fleet_wnd = FleetUIManager::GetFleetUIManager().ActiveFleetWnd())
        selected_ship_ids = fleet_wnd->SelectedShipIDs();

    // if old and new sets of selected fleets are the same, don't need to change anything
    if (selected_ship_ids == m_selected_ship_ids)
        return;

    // set new selected fleets
    m_selected_ship_ids = selected_ship_ids;


    // refresh meters of planets in currently selected system, as changing selected fleets
    // may have changed which species a planet should have population estimates shown for

    SidePanel::Update();
}

void MapWnd::RefreshFleetButtonSelectionIndicators() {
    //std::cout << "MapWnd::RefreshFleetButtonSelectionIndicators()" << std::endl;

    // clear old selection indicators
    for (auto& stationary_fleet_button : m_stationary_fleet_buttons) {
        for (auto& button : stationary_fleet_button.second)
            button->SetSelected(false);
    }

    for (auto& departing_fleet_button : m_departing_fleet_buttons) {
        for (auto& button : departing_fleet_button.second)
            button->SetSelected(false);
    }

    for (auto& moving_fleet_button : m_moving_fleet_buttons) {
        for (auto& button : moving_fleet_button.second)
            button->SetSelected(false);
    }

    // add new selection indicators
    for (int fleet_id : m_selected_fleet_ids) {
        const auto& button_it = m_fleet_buttons.find(fleet_id);
        if (button_it != m_fleet_buttons.end())
            button_it->second->SetSelected(true);
    }
}

void MapWnd::UniverseObjectDeleted(std::shared_ptr<const UniverseObject> obj) {
    if (obj)
        DebugLogger() << "MapWnd::UniverseObjectDeleted: " << obj->ID();
    else
        DebugLogger() << "MapWnd::UniverseObjectDeleted: NO OBJECT";
    if (auto fleet = std::dynamic_pointer_cast<const Fleet>(obj))
        RemoveFleet(fleet->ID());
}

void MapWnd::RegisterPopup(const std::shared_ptr<MapWndPopup>& popup) {
    if (popup)
        m_popups.push_back(std::weak_ptr<MapWndPopup>(popup));
}

void MapWnd::RemovePopup(MapWndPopup* popup) {
    if (!popup)
        return;

    const auto& it = std::find_if(m_popups.begin(), m_popups.end(),
                                  [&popup](const std::weak_ptr<Wnd>& xx){ return xx.lock().get() == popup;});
    if (it != m_popups.end())
        m_popups.erase(it);
}

void MapWnd::ResetEmpireShown() {
    m_production_wnd->SetEmpireShown(HumanClientApp::GetApp()->EmpireID());
    m_research_wnd->SetEmpireShown(HumanClientApp::GetApp()->EmpireID());
    // TODO: Design?
}

void MapWnd::Sanitize() {
    ShowAllPopups(); // make sure popups don't save visible = 0 to the config
    CloseAllPopups();
    HideResearch();
    HideProduction();
    HideDesign();
    RemoveWindows();
    m_pedia_panel->ClearItems();    // deletes all pedia items in the memory
    m_toolbar->Hide();
    m_FPS->Hide();
    m_scale_line->Hide();
    m_zoom_slider->Hide();
    m_combat_report_wnd->Hide();
    m_sitrep_panel->ShowSitRepsForTurn(INVALID_GAME_TURN);
    if (m_auto_end_turn)
        ToggleAutoEndTurn();

    ResetEmpireShown();

    SelectSystem(INVALID_OBJECT_ID);

    // temp
    m_starfield_verts.clear();
    m_starfield_colours.clear();
    // end temp

    ClearSystemRenderingBuffers();
    ClearStarlaneRenderingBuffers();
    ClearFieldRenderingBuffers();
    ClearVisibilityRadiiRenderingBuffers();
    ClearScaleCircleRenderingBuffer();
    ClearStarfieldRenderingBuffers();


    if (ClientUI* cui = ClientUI::GetClientUI()) {
        // clearing of message window commented out because scrollbar has quirks
        // after doing so until enough messages are added
        //if (MessageWnd* msg_wnd = cui->GetMessageWnd())
        //    msg_wnd->Clear();
        if (const auto& plr_wnd = cui->GetPlayerListWnd())
            plr_wnd->Clear();
    }

    MoveTo(GG::Pt(-AppWidth(), -AppHeight()));
    m_zoom_steps_in = 1.0;

    m_research_wnd->Sanitize();
    m_production_wnd->Sanitize();
    m_design_wnd->Sanitize();

    m_selected_fleet_ids.clear();
    m_selected_ship_ids.clear();

    m_starlane_endpoints.clear();

    DeleteFleetButtons();

    for (auto& signal : m_fleet_state_change_signals)
        signal.second.disconnect();
    m_fleet_state_change_signals.clear();

    for (auto& entry : m_system_fleet_insert_remove_signals) {
        for (auto& connection : entry.second)
            connection.disconnect();
        entry.second.clear();
    }
    m_system_fleet_insert_remove_signals.clear();
    m_fleet_lines.clear();
    m_projected_fleet_lines.clear();
    m_system_icons.clear();
    m_fleets_exploring.clear();
    m_line_between_systems = {INVALID_OBJECT_ID, INVALID_OBJECT_ID};

    DetachChildren();
}

void MapWnd::PushWndStack(std::shared_ptr<GG::Wnd> wnd) {
    if (!wnd)
        return;
    // First remove it from its current location in the stack (if any), to prevent it from being
    // present in two locations at once.
    RemoveFromWndStack(wnd);
    m_wnd_stack.push_back(wnd);
}

void MapWnd::RemoveFromWndStack(std::shared_ptr<GG::Wnd> wnd) {
    // Recreate the stack, but without the Wnd to be removed or any null/expired weak_ptrs
    std::vector<std::weak_ptr<GG::Wnd>> new_stack;
    for (auto& weak_wnd : m_wnd_stack) {
        // skip adding to the new stack if it's null/expired
        if (auto shared_wnd = weak_wnd.lock()) {
            // skip adding to the new stack if it's the one to be removed
            if (shared_wnd != wnd) {
                // Swap them to avoid another reference count check
                new_stack.emplace_back();
                new_stack.back().swap(weak_wnd);
            }
        }
    }
    m_wnd_stack.swap(new_stack);
}

bool MapWnd::ReturnToMap() {
    std::shared_ptr<GG::Wnd> wnd;
    // Pop the top Wnd from the stack, and repeat until we find a non-null and visible one (or
    // until the stack runs out).
    // Need to check that it's visible, in case it was closed without being removed from the stack;
    // if we didn't reject such a Wnd, we might close no window, or even open a window.
    // Either way, the Wnd is removed from the stack, since it is no longer of any use.
    while (!m_wnd_stack.empty() && !(wnd && wnd->Visible())) {
        wnd = m_wnd_stack.back().lock();
        m_wnd_stack.pop_back();
    }
    // If no non-null and visible Wnd was found, then there's nothing to do.
    if (!(wnd && wnd->Visible())) {
        return true;
    }

    auto cui = ClientUI::GetClientUI();

    if (wnd == m_sitrep_panel) {
        ToggleSitRep();
    } else if (wnd == m_research_wnd) {
        ToggleResearch();
    } else if (wnd == m_design_wnd) {
        ToggleDesign();
    } else if (wnd == m_production_wnd) {
        ToggleProduction();
    } else if (wnd == m_pedia_panel) {
        TogglePedia();
    } else if (wnd == m_object_list_wnd) {
        ToggleObjects();
    } else if (wnd == m_moderator_wnd) {
        ToggleModeratorActions();
    } else if (wnd == m_combat_report_wnd) {
        m_combat_report_wnd->Hide();
    } else if (wnd == m_side_panel) {
        SelectSystem(INVALID_OBJECT_ID);
    } else if (dynamic_cast<FleetWnd*>(wnd.get())) {
        // if it is any fleet window at all, go ahead and close all fleet windows.
        FleetUIManager::GetFleetUIManager().CloseAll();
    } else if (cui && wnd == cui->GetPlayerListWnd()) {
        HideEmpires();
    } else if (cui && wnd == cui->GetMessageWnd()) {
        HideMessages();
    } else {
        ErrorLogger() << "Unknown GG::Wnd " << wnd->Name() << " found in MapWnd::m_wnd_stack";
    }

    return true;
}

bool MapWnd::EndTurn() {
    if (m_ready_turn) {
        HumanClientApp::GetApp()->UnreadyTurn();
    } else {
        ClientUI* cui = ClientUI::GetClientUI();
        if (!cui) {
            ErrorLogger() << "MapWnd::EndTurn: No client UI available";
            return false;
        }
        SaveGameUIData ui_data;
        cui->GetSaveGameUIData(ui_data);
        HumanClientApp::GetApp()->StartTurn(ui_data);
    }
    return true;
}

void MapWnd::ToggleAutoEndTurn() {
    if (!m_btn_auto_turn)
        return;

     if (m_auto_end_turn) {
        m_auto_end_turn = false;
        m_btn_auto_turn->SetUnpressedGraphic(   GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "manual_turn.png")));
        m_btn_auto_turn->SetPressedGraphic(     GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "auto_turn.png")));
        m_btn_auto_turn->SetRolloverGraphic(    GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "manual_turn_mouseover.png")));

        m_btn_auto_turn->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
        m_btn_auto_turn->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("MAP_BTN_MANUAL_TURN_ADVANCE"),
            UserString("MAP_BTN_MANUAL_TURN_ADVANCE_DESC")
        ));
    } else {
        m_auto_end_turn = true;
        m_btn_auto_turn->SetUnpressedGraphic(   GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "auto_turn.png")));
        m_btn_auto_turn->SetPressedGraphic(     GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "manual_turn.png")));
        m_btn_auto_turn->SetRolloverGraphic(    GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "auto_turn_mouseover.png")));

        m_btn_auto_turn->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
        m_btn_auto_turn->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("MAP_BTN_AUTO_ADVANCE_ENABLED"),
            UserString("MAP_BTN_AUTO_ADVANCE_ENABLED_DESC")
        ));
    }
}

void MapWnd::ShowModeratorActions() {
    // hide other "competing" windows
    HideResearch();
    HideProduction();
    HideDesign();

    // update moderator window
    m_moderator_wnd->Refresh();

    // show the moderator window
    m_moderator_wnd->Show();
    GG::GUI::GetGUI()->MoveUp(m_moderator_wnd);
    PushWndStack(m_moderator_wnd);

    m_btn_moderator->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "moderator_mouseover.png")));
    m_btn_moderator->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "moderator.png")));
}

void MapWnd::HideModeratorActions() {
    m_moderator_wnd->Hide();
    RemoveFromWndStack(m_moderator_wnd);
    m_btn_moderator->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "moderator.png")));
    m_btn_moderator->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "moderator_mouseover.png")));
}

bool MapWnd::ToggleModeratorActions() {
    if (!m_moderator_wnd->Visible() || m_production_wnd->Visible() || m_research_wnd->Visible() || m_design_wnd->Visible()) {
        ShowModeratorActions();
    } else {
        HideModeratorActions();
    }
    return true;
}

void MapWnd::ShowObjects() {
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideResearch();
    HideProduction();
    HideDesign();

    // update objects window
    m_object_list_wnd->Refresh();

    // show the objects window
    m_object_list_wnd->Show();
    GG::GUI::GetGUI()->MoveUp(m_object_list_wnd);
    PushWndStack(m_object_list_wnd);

    // indicate selection on button
    m_btn_objects->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "objects_mouseover.png")));
    m_btn_objects->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "objects.png")));
}

void MapWnd::HideObjects() {
    m_object_list_wnd->Hide(); // necessary so it won't be visible when next toggled
    RemoveFromWndStack(m_object_list_wnd);
    m_btn_objects->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "objects.png")));
    m_btn_objects->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "objects_mouseover.png")));
}

bool MapWnd::ToggleObjects() {
    if (!m_object_list_wnd->Visible() || m_production_wnd->Visible() || m_research_wnd->Visible() || m_design_wnd->Visible()) {
        ShowObjects();
    } else {
        HideObjects();
    }
    return true;
}

void MapWnd::ShowSitRep() {
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideResearch();
    HideProduction();
    HideDesign();

    // show the sitrep window
    m_sitrep_panel->Show();
    GG::GUI::GetGUI()->MoveUp(m_sitrep_panel);
    PushWndStack(m_sitrep_panel);

    // indicate selection on button
    m_btn_siterep->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "sitrep_mouseover.png")));
    m_btn_siterep->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "sitrep.png")));
}

void MapWnd::HideSitRep() {
    m_sitrep_panel->Hide(); // necessary so it won't be visible when next toggled
    RemoveFromWndStack(m_sitrep_panel);
    m_btn_siterep->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "sitrep.png")));
    m_btn_siterep->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "sitrep_mouseover.png")));
}

bool MapWnd::ToggleSitRep() {
    if (!m_sitrep_panel->Visible() || m_production_wnd->Visible() || m_research_wnd->Visible() || m_design_wnd->Visible()) {
        ShowSitRep();
    } else {
        HideSitRep();
    }
    return true;
}

void MapWnd::ShowMessages() {
    // hide other "competing" windows
    HideResearch();
    HideProduction();
    HideDesign();

    ClientUI* cui = ClientUI::GetClientUI();
    if (!cui)
        return;
    const auto& msg_wnd = cui->GetMessageWnd();
    if (!msg_wnd)
        return;
    GG::GUI* gui = GG::GUI::GetGUI();
    if (!gui)
        return;
    msg_wnd->Show();
    msg_wnd->OpenForInput();
    gui->MoveUp(msg_wnd);
    PushWndStack(msg_wnd);

    // indicate selection on button
    m_btn_messages->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "messages_mouseover.png")));
    m_btn_messages->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "messages.png")));
}

void MapWnd::HideMessages() {
    if (ClientUI* cui = ClientUI::GetClientUI()) {
        cui->GetMessageWnd()->Hide();
        RemoveFromWndStack(cui->GetMessageWnd());
    }
    m_btn_messages->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "messages.png")));
    m_btn_messages->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "messages_mouseover.png")));
}

bool MapWnd::ToggleMessages() {
    ClientUI* cui = ClientUI::GetClientUI();
    if (!cui)
        return false;
    const auto& msg_wnd = cui->GetMessageWnd();
    if (!msg_wnd)
        return false;
    if (!msg_wnd->Visible() || m_production_wnd->Visible() || m_research_wnd->Visible() || m_design_wnd->Visible()) {
        ShowMessages();
    } else {
        HideMessages();
    }
    return true;
}

void MapWnd::ShowEmpires() {
    // hide other "competing" windows
    HideResearch();
    HideProduction();
    HideDesign();

    ClientUI* cui = ClientUI::GetClientUI();
    if (!cui)
        return;
    const auto& plr_wnd = cui->GetPlayerListWnd();
    if (!plr_wnd)
        return;
    GG::GUI* gui = GG::GUI::GetGUI();
    if (!gui)
        return;
    plr_wnd->Show();
    gui->MoveUp(plr_wnd);
    PushWndStack(plr_wnd);

    // indicate selection on button
    m_btn_empires->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "empires_mouseover.png")));
    m_btn_empires->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "empires.png")));
}

void MapWnd::HideEmpires() {
    if (ClientUI* cui = ClientUI::GetClientUI()) {
        cui->GetPlayerListWnd()->Hide();
        RemoveFromWndStack(cui->GetPlayerListWnd());
    }
    m_btn_empires->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "empires.png")));
    m_btn_empires->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "empires_mouseover.png")));
}

bool MapWnd::ToggleEmpires() {
    ClientUI* cui = ClientUI::GetClientUI();
    if (!cui)
        return false;
    const auto& plr_wnd = cui->GetPlayerListWnd();
    if (!plr_wnd)
        return false;
    if (!plr_wnd->Visible() || m_production_wnd->Visible() || m_research_wnd->Visible() || m_design_wnd->Visible()) {
        ShowEmpires();
    } else {
        HideEmpires();
    }
    return true;
}

void MapWnd::ShowPedia() {
    // if production screen is visible, toggle the production screen's pedia, not the one of the map screen
    if (m_in_production_view_mode) {
        m_production_wnd->TogglePedia();
        return;
    }

    if (m_research_wnd->Visible()) {
        m_research_wnd->TogglePedia();
        return;
    }

    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideResearch();
    HideProduction();
    HideDesign();

    if (m_pedia_panel->GetItemsSize() == 0)
        m_pedia_panel->SetIndex();

    // update pedia window
    m_pedia_panel->Refresh();

    // show the pedia window
    m_pedia_panel->Show();
    GG::GUI::GetGUI()->MoveUp(m_pedia_panel);
    PushWndStack(m_pedia_panel);

    // indicate selection on button
    m_btn_pedia->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "pedia_mouseover.png")));
    m_btn_pedia->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "pedia.png")));
}

void MapWnd::HidePedia() {
    m_pedia_panel->Hide();
    RemoveFromWndStack(m_pedia_panel);
    m_btn_pedia->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "pedia.png")));
    m_btn_pedia->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "pedia_mouseover.png")));
}

bool MapWnd::TogglePedia() {
    if (!m_pedia_panel->Visible() || m_production_wnd->Visible() || m_research_wnd->Visible() || m_design_wnd->Visible()) {
        ShowPedia();
    } else {
        HidePedia();
    }
    return true;
}

bool MapWnd::ShowGraphs() {
    ShowPedia();
    m_pedia_panel->AddItem(TextLinker::ENCYCLOPEDIA_TAG, "ENC_GRAPH");
    return true;
}

void MapWnd::HideSidePanel() {
    m_sidepanel_open_before_showing_other = m_side_panel->Visible();   // a kludge, so the sidepanel will reappear after opening and closing a full screen wnd
    m_side_panel->Hide();
}

void MapWnd::RestoreSidePanel() {
    if (m_sidepanel_open_before_showing_other)
        ReselectLastSystem();
    // send order changes could be made in research, production or other windows
    HumanClientApp::GetApp()->SendPartialOrders();
}

void MapWnd::ShowResearch() {
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideProduction();
    HideDesign();
    HideSidePanel();

    // show the research window
    m_research_wnd->Show();
    GG::GUI::GetGUI()->MoveUp(m_research_wnd);
    PushWndStack(m_research_wnd);

    // hide pedia again if it is supposed to be hidden persistently
    if (GetOptionsDB().Get<bool>("ui.research.pedia.hidden.enabled"))
        m_research_wnd->HidePedia();

    // indicate selection on button
    m_btn_research->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "research_mouseover.png")));
    m_btn_research->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "research.png")));
}

void MapWnd::HideResearch() {
    m_research_wnd->Hide();
    RemoveFromWndStack(m_research_wnd);
    m_btn_research->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "research.png")));
    m_btn_research->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "research_mouseover.png")));

    RestoreSidePanel();
}

bool MapWnd::ToggleResearch() {
    if (m_research_wnd->Visible())
        HideResearch();
    else
        ShowResearch();
    return true;
}

void MapWnd::ShowProduction() {
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideResearch();
    HideDesign();
    HideSidePanel();
    HidePedia();
    if (GetOptionsDB().Get<bool>("ui.production.mappanels.removed")) {
        RemoveWindows();
        GG::GUI::GetGUI()->Remove(ClientUI::GetClientUI()->GetMessageWnd());
        GG::GUI::GetGUI()->Remove(ClientUI::GetClientUI()->GetPlayerListWnd());
    }

    m_in_production_view_mode = true;
    HideAllPopups();
    GG::GUI::GetGUI()->MoveUp(m_production_wnd);
    PushWndStack(m_production_wnd);

    // indicate selection on button
    m_btn_production->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "production_mouseover.png")));
    m_btn_production->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "production.png")));

    // if no system is currently shown in sidepanel, default to this empire's
    // home system (ie. where the capital is)
    if (SidePanel::SystemID() == INVALID_OBJECT_ID) {
        if (const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID()))
            if (auto obj = GetUniverseObject(empire->CapitalID()))
                SelectSystem(obj->SystemID());
    } else {
        // if a system is already shown, make sure a planet gets selected by
        // default when the production screen opens up
        m_production_wnd->SelectDefaultPlanet();
    }
    m_production_wnd->Update();
    m_production_wnd->Show();

    // hide pedia again if it is supposed to be hidden persistently
    if (GetOptionsDB().Get<bool>("ui.production.pedia.hidden.enabled"))
        m_production_wnd->TogglePedia();
}

void MapWnd::HideProduction() {
    m_production_wnd->Hide();
    RemoveFromWndStack(m_production_wnd);
    m_in_production_view_mode = false;
    m_btn_production->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "production.png")));
    m_btn_production->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "production_mouseover.png")));

    // Don't check ui.production.mappanels.removed to avoid a
    // situation where the option is changed and the panels aren't re-registered.
    RegisterWindows();
    GG::GUI::GetGUI()->Register(ClientUI::GetClientUI()->GetMessageWnd());
    GG::GUI::GetGUI()->Register(ClientUI::GetClientUI()->GetPlayerListWnd());

    ShowAllPopups();
    RestoreSidePanel();
}

bool MapWnd::ToggleProduction() {
    if (m_in_production_view_mode)
        HideProduction();
    else
        ShowProduction();

    // make info panels in production/map window's side panel update their expand-collapse state
    m_side_panel->Update();

    return true;
}

void MapWnd::ShowDesign() {
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideResearch();
    HideProduction();
    HideSidePanel();

    // show the design window
    m_design_wnd->Show();
    GG::GUI::GetGUI()->MoveUp(m_design_wnd);
    PushWndStack(m_design_wnd);
    m_design_wnd->Reset();

    // indicate selection on button
    m_btn_design->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "design_mouseover.png")));
    m_btn_design->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "design.png")));
}

void MapWnd::HideDesign() {
    m_design_wnd->Hide();
    RemoveFromWndStack(m_design_wnd);
    m_btn_design->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "design.png")));
    m_btn_design->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "design_mouseover.png")));

    RestoreSidePanel();
}

bool MapWnd::ToggleDesign() {
    if (m_design_wnd->Visible())
        HideDesign();
    else
        ShowDesign();
    return true;
}

bool MapWnd::ShowMenu() {
    if (m_menu_showing)
        return true;

    ClearProjectedFleetMovementLines();
    m_menu_showing = true;

    m_btn_menu->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "menu_mouseover.png")));
    m_btn_menu->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "menu.png")));

    auto menu = GG::Wnd::Create<InGameMenu>();
    menu->Run();
    m_menu_showing = false;

    m_btn_menu->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "menu.png")));
    m_btn_menu->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "menu_mouseover.png")));

    return true;
}

bool MapWnd::CloseSystemView() {
    SelectSystem(INVALID_OBJECT_ID);
    m_side_panel->Hide();   // redundant, but safer to keep in case the behavior of SelectSystem changes
    return true;
}

bool MapWnd::KeyboardZoomIn() {
    Zoom(1);
    return true;
}

bool MapWnd::KeyboardZoomOut() {
    Zoom(-1);
    return true;
}

void MapWnd::RefreshTradeResourceIndicator() {
    Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
    if (!empire) {
        m_trade->SetValue(0.0);
        return;
    }
    m_trade->SetValue(empire->ResourceStockpile(RE_TRADE));
    m_trade->ClearBrowseInfoWnd();
    m_trade->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_trade->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_TRADE_TITLE"), UserString("MAP_TRADE_TEXT")));
}

void MapWnd::RefreshFleetResourceIndicator() {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    Empire* empire = GetEmpire(empire_id);
    if (!empire) {
        m_fleet->SetValue(0.0);
        return;
    }

    const auto& this_client_known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(empire_id);

    int total_fleet_count = 0;
    for (auto& ship : Objects().FindObjects<Ship>()) {
        if (ship->OwnedBy(empire_id) && !this_client_known_destroyed_objects.count(ship->ID()))
            total_fleet_count++;
    }

    m_fleet->SetValue(total_fleet_count);
    m_fleet->ClearBrowseInfoWnd();
    m_fleet->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_fleet->SetBrowseInfoWnd(GG::Wnd::Create<FleetDetailBrowseWnd>(
        empire_id, GG::X(FontBasedUpscale(250))));
}

void MapWnd::RefreshResearchResourceIndicator() {
    const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
    if (!empire) {
        m_research->SetValue(0.0);
        m_research_wasted->Hide();
        return;
    }
    m_research->SetValue(empire->ResourceOutput(RE_RESEARCH));
    m_research->ClearBrowseInfoWnd();
    m_research->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    float total_RP_spent = empire->GetResearchQueue().TotalRPsSpent();
    float total_RP_output = empire->GetResourcePool(RE_RESEARCH)->TotalOutput();
    float total_RP_wasted = total_RP_output - total_RP_spent;
    float total_RP_target_output = empire->GetResourcePool(RE_RESEARCH)->TargetOutput();

    m_research->SetBrowseInfoWnd(GG::Wnd::Create<ResourceBrowseWnd>(
        UserString("MAP_RESEARCH_TITLE"), UserString("RESEARCH_INFO_RP"),
        total_RP_spent, total_RP_output, total_RP_target_output
    ));

    if (total_RP_wasted > 0.05) {
        DebugLogger()  << "MapWnd::RefreshResearchResourceIndicator: Showing Research Wasted Icon with RP spent: "
                       << total_RP_spent << " and RP Production: " << total_RP_output << ", wasting " << total_RP_wasted;
        m_research_wasted->Show();
        m_research_wasted->ClearBrowseInfoWnd();
        m_research_wasted->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

        m_research_wasted->SetBrowseInfoWnd(GG::Wnd::Create<WastedStockpiledResourceBrowseWnd>(
            UserString("MAP_RESEARCH_WASTED_TITLE"), UserString("RESEARCH_INFO_RP"),
            total_RP_output, total_RP_wasted, false, 0.0f, 0.0f, total_RP_wasted,
            UserString("MAP_RES_CLICK_TO_OPEN")));

    } else {
        m_research_wasted->Hide();
    }
}

void MapWnd::RefreshDetectionIndicator() {
    const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return;
    m_detection->SetValue(empire->GetMeter("METER_DETECTION_STRENGTH")->Current());
    m_detection->ClearBrowseInfoWnd();
    m_detection->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_detection->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_DETECTION_TITLE"), UserString("MAP_DETECTION_TEXT")));
}

void MapWnd::RefreshIndustryResourceIndicator() {
    const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
    if (!empire) {
        m_industry->SetValue(0.0);
        m_industry_wasted->Hide();
        m_stockpile->SetValue(0.0);
        return;
    }
    m_industry->SetValue(empire->ResourceOutput(RE_INDUSTRY));
    m_industry->ClearBrowseInfoWnd();
    m_industry->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    double total_PP_spent = empire->GetProductionQueue().TotalPPsSpent();
    double total_PP_output = empire->GetResourcePool(RE_INDUSTRY)->TotalOutput();
    double total_PP_target_output = empire->GetResourcePool(RE_INDUSTRY)->TargetOutput();
    float  stockpile = empire->GetResourcePool(RE_INDUSTRY)->Stockpile();
    float  stockpile_used = boost::accumulate(empire->GetProductionQueue().AllocatedStockpilePP() | boost::adaptors::map_values, 0.0f);
    float  stockpile_use_capacity = empire->GetProductionQueue().StockpileCapacity();
    float  expected_stockpile = empire->GetProductionQueue().ExpectedNewStockpileAmount();

    float  stockpile_plusminus_next_turn = expected_stockpile - stockpile;
    double total_PP_for_stockpile_projects = empire->GetProductionQueue().ExpectedProjectTransferToStockpile();
    double total_PP_to_stockpile = expected_stockpile - stockpile + stockpile_used;
    double total_PP_excess = total_PP_output - total_PP_spent;
    double total_PP_wasted = total_PP_output - total_PP_spent - total_PP_to_stockpile + total_PP_for_stockpile_projects;

    m_industry->SetBrowseInfoWnd(GG::Wnd::Create<ResourceBrowseWnd>(
        UserString("MAP_PRODUCTION_TITLE"), UserString("PRODUCTION_INFO_PP"),
        total_PP_spent, total_PP_output, total_PP_target_output,
        true, stockpile_used, stockpile, expected_stockpile));

    m_stockpile->SetValue(stockpile);
    m_stockpile->SetValue(stockpile_plusminus_next_turn, 1);
    m_stockpile->ClearBrowseInfoWnd();
    m_stockpile->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_stockpile->SetBrowseInfoWnd(GG::Wnd::Create<ResourceBrowseWnd>(
        UserString("MAP_STOCKPILE_TITLE"), UserString("PRODUCTION_INFO_PP"),
        -1.0f, -1.0f, -1.0f,
        true, stockpile_used, stockpile, expected_stockpile,
        true, stockpile_use_capacity));

    // red "waste" icon if the non-project transfer to IS is more than either 3x per-turn use or 80% total output
    // else yellow icon if the non-project transfer to IS is more than 20% total output, or if there is any transfer
    // to IS and the IS is expected to be above 10x per-turn use.
    if (total_PP_wasted > 0.05 || (total_PP_excess > std::min(3.0 * stockpile_use_capacity, 0.8 * total_PP_output))) {
        DebugLogger()  << "MapWnd::RefreshIndustryResourceIndicator: Showing Industry Wasted Icon with Industry spent: "
                       << total_PP_spent << " and Industry Production: " << total_PP_output << ", wasting " << total_PP_wasted;
        boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";
        m_industry_wasted->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(button_texture_dir /
                                                                "wasted_resource.png", false)));
        m_industry_wasted->SetPressedGraphic(GG::SubTexture(ClientUI::GetTexture(button_texture_dir /
                                                                "wasted_resource_clicked.png", false)));
        m_industry_wasted->SetRolloverGraphic(GG::SubTexture(ClientUI::GetTexture(button_texture_dir /
                                                                "wasted_resource_mouseover.png", false)));
        m_industry_wasted->Show();
        m_industry_wasted->ClearBrowseInfoWnd();
        m_industry_wasted->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
        m_industry_wasted->SetBrowseInfoWnd(GG::Wnd::Create<WastedStockpiledResourceBrowseWnd>(
            UserString("MAP_PRODUCTION_WASTED_TITLE"), UserString("PRODUCTION_INFO_PP"),
            total_PP_output, total_PP_excess,
            true, stockpile_use_capacity, total_PP_to_stockpile, total_PP_wasted,
            UserString("MAP_PROD_CLICK_TO_OPEN")));
    } else if (total_PP_to_stockpile > 0.05 && (expected_stockpile > (10 * stockpile_use_capacity) ||
                                                total_PP_excess > 0.2 * total_PP_output)) {
        boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";
        m_industry_wasted->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(button_texture_dir /
                                                                "warned_resource.png", false)));
        m_industry_wasted->SetPressedGraphic(GG::SubTexture(ClientUI::GetTexture(button_texture_dir /
                                                                "warned_resource_clicked.png", false)));
        m_industry_wasted->SetRolloverGraphic(GG::SubTexture(ClientUI::GetTexture(button_texture_dir /
                                                                "warned_resource_mouseover.png", false)));
        m_industry_wasted->Show();
        m_industry_wasted->ClearBrowseInfoWnd();
        m_industry_wasted->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
        m_industry_wasted->SetBrowseInfoWnd(GG::Wnd::Create<WastedStockpiledResourceBrowseWnd>(
            UserString("MAP_PRODUCTION_WASTED_TITLE"), UserString("PRODUCTION_INFO_PP"),
            total_PP_output, total_PP_excess,
            true, stockpile_use_capacity, total_PP_to_stockpile, total_PP_wasted,
            UserString("MAP_PROD_CLICK_TO_OPEN")));
    } else {
        m_industry_wasted->Hide();
    }
}

void MapWnd::RefreshPopulationIndicator() {
    Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
    if (!empire) {
        m_population->SetValue(0.0);
        return;
    }
    m_population->SetValue(empire->GetPopulationPool().Population());
    m_population->ClearBrowseInfoWnd();

    const auto pop_center_ids = empire->GetPopulationPool().PopCenterIDs();
    std::map<std::string, float> population_counts;
    std::map<std::string, float> tag_counts;
    const ObjectMap& objects = Objects();

    //tally up all species population counts
    for (int pop_center_id : pop_center_ids) {
        auto obj = objects.Object(pop_center_id);
        auto pc = std::dynamic_pointer_cast<const PopCenter>(obj);
        if (!pc)
            continue;

        const std::string& species_name = pc->SpeciesName();
        if (species_name.empty())
            continue;
        float this_pop = pc->InitialMeterValue(METER_POPULATION);
        population_counts[species_name] += this_pop;
        if (const Species* species = GetSpecies(species_name) ) {
            for (const std::string& tag : species->Tags()) {
                tag_counts[tag] += this_pop;
            }
        }
    }

    m_population->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_population->SetBrowseInfoWnd(GG::Wnd::Create<CensusBrowseWnd>(
        UserString("MAP_POPULATION_DISTRIBUTION"), population_counts, tag_counts, GetSpeciesManager().census_order()));
}

void MapWnd::UpdateSidePanelSystemObjectMetersAndResourcePools() {
    GetUniverse().UpdateMeterEstimates(SidePanel::SystemID(), true);
    UpdateEmpireResourcePools();
}

void MapWnd::UpdateEmpireResourcePools() {
    //std::cout << "MapWnd::UpdateEmpireResourcePools" << std::endl;
    Empire *empire = GetEmpire( HumanClientApp::GetApp()->EmpireID() );
    /* Recalculate stockpile, available, production, predicted change of
     * resources.  When resource pools update, they emit ChangeSignal, which is
     * connected to MapWnd::Refresh???ResourceIndicator, which updates the
     * empire resource pool indicators of the MapWnd. */
    empire->UpdateResourcePools();

    // Update indicators on sidepanel, which are not directly connected to from the ResourcePool ChangedSignal
    SidePanel::Update();
}

bool MapWnd::ZoomToHomeSystem() {
    const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return false;
    int home_id = empire->CapitalID();

    if (home_id != INVALID_OBJECT_ID) {
        auto object = GetUniverseObject(home_id);
        if (!object)
            return false;
        CenterOnObject(object->SystemID());
        SelectSystem(object->SystemID());
    }

    return true;
}

namespace {
    struct CustomRowCmp {
        bool operator()(const std::pair<std::string, int>& lhs, const std::pair<std::string, int>& rhs) {
            return GetLocale("en_US.UTF-8").operator()(lhs.first, rhs.first);    // todo: use .second values to break ties
        }
    };

    std::set<std::pair<std::string, int>, CustomRowCmp> GetSystemNamesIDs() {
        // get systems, store alphabetized
        std::set<std::pair<std::string, int>, CustomRowCmp> system_names_ids;
        for (auto& system : Objects().FindObjects<System>()) {
            system_names_ids.insert({system->Name(), system->ID()});
        }
        return system_names_ids;
    }

    std::set<std::pair<std::string, int>, CustomRowCmp> GetOwnedSystemNamesIDs(int empire_id) {
        auto owned_planets = Objects().FindObjects(OwnedVisitor<Planet>(empire_id));

        // get IDs of systems that contain any owned planets
        std::unordered_set<int> system_ids;
        for (auto& obj : owned_planets)
        { system_ids.insert(obj->SystemID()); }

        // store systems, sorted alphabetically
        std::set<std::pair<std::string, int>, CustomRowCmp> system_names_ids;
        for (int system_id : system_ids) {
            if (auto sys = GetSystem(system_id))
                system_names_ids.insert({sys->Name(), sys->ID()});
        }

        return system_names_ids;
    }
}

bool MapWnd::ZoomToPrevOwnedSystem() {
    // get planets owned by client's player, sorted alphabetically
    auto system_names_ids = GetOwnedSystemNamesIDs(HumanClientApp::GetApp()->EmpireID());
    if (system_names_ids.empty())
        return false;

    // find currently selected system in list
    auto it = system_names_ids.rend();
    auto sel_sys = GetSystem(SidePanel::SystemID());
    if (sel_sys) {
        it = std::find(system_names_ids.rbegin(), system_names_ids.rend(),  std::make_pair(sel_sys->Name(), sel_sys->ID()));
        if (it != system_names_ids.rend())
            ++it;
    }
    if (it == system_names_ids.rend())
        it = system_names_ids.rbegin();

    if (it != system_names_ids.rend()) {
        CenterOnObject(it->second);
        SelectSystem(it->second);
    }

    return true;
}

bool MapWnd::ZoomToNextOwnedSystem() {
    // get planets owned by client's player, sorted alphabetically
    auto system_names_ids = GetOwnedSystemNamesIDs(HumanClientApp::GetApp()->EmpireID());
    if (system_names_ids.empty())
        return false;

    auto it = system_names_ids.end();

    // find currently selected system in list
    auto sel_sys = GetSystem(SidePanel::SystemID());
    if (sel_sys) {
        it = std::find(system_names_ids.begin(), system_names_ids.end(), std::make_pair(sel_sys->Name(), sel_sys->ID()));
        if (it != system_names_ids.end())
            ++it;
    }
    if (it == system_names_ids.end())
        it = system_names_ids.begin();

    if (it != system_names_ids.end()) {
        CenterOnObject(it->second);
        SelectSystem(it->second);
    }

    return true;
}

bool MapWnd::ZoomToPrevSystem() {
    auto system_names_ids = GetSystemNamesIDs();
    if (system_names_ids.empty())
        return false;

    // find currently selected system in list
    auto it = system_names_ids.rend();
    auto sel_sys = GetSystem(SidePanel::SystemID());
    if (sel_sys) {
        it = std::find(system_names_ids.rbegin(), system_names_ids.rend(),  std::make_pair(sel_sys->Name(), sel_sys->ID()));
        if (it != system_names_ids.rend())
            ++it;
    }
    if (it == system_names_ids.rend())
        it = system_names_ids.rbegin();

    if (it != system_names_ids.rend()) {
        CenterOnObject(it->second);
        SelectSystem(it->second);
    }

    return true;
}

bool MapWnd::ZoomToNextSystem() {
    auto system_names_ids = GetSystemNamesIDs();
    if (system_names_ids.empty())
        return false;

    auto it = system_names_ids.end();

    // find currently selected system in list
    auto sel_sys = GetSystem(SidePanel::SystemID());
    if (sel_sys) {
        it = std::find(system_names_ids.begin(), system_names_ids.end(), std::make_pair(sel_sys->Name(), sel_sys->ID()));
        if (it != system_names_ids.end())
            ++it;
    }
    if (it == system_names_ids.end())
        it = system_names_ids.begin();

    if (it != system_names_ids.end()) {
        CenterOnObject(it->second);
        SelectSystem(it->second);
    }

    return true;
}

bool MapWnd::ZoomToPrevIdleFleet() {
    auto vec = GetUniverse().Objects().FindObjectIDs(StationaryFleetVisitor(HumanClientApp::GetApp()->EmpireID()));
    auto it = std::find(vec.begin(), vec.end(), m_current_fleet_id);
    const auto& destroyed_object_ids = GetUniverse().DestroyedObjectIds();
    if (it != vec.begin())
        --it;
    else
        it = vec.end();
    while (it != vec.begin() && (it == vec.end() || destroyed_object_ids.count(*it)))
        --it;
    m_current_fleet_id = it != vec.end() ? *it : vec.empty() ? INVALID_OBJECT_ID : vec.back();

    if (m_current_fleet_id != INVALID_OBJECT_ID) {
        CenterOnObject(m_current_fleet_id);
        SelectFleet(m_current_fleet_id);
    }

    return true;
}

bool MapWnd::ZoomToNextIdleFleet() {
    auto vec = GetUniverse().Objects().FindObjectIDs(StationaryFleetVisitor(HumanClientApp::GetApp()->EmpireID()));
    auto it = std::find(vec.begin(), vec.end(), m_current_fleet_id);
    const auto& destroyed_object_ids = GetUniverse().DestroyedObjectIds();
    if (it != vec.end())
        ++it;
    while (it != vec.end() && destroyed_object_ids.count(*it))
        ++it;
    m_current_fleet_id = it != vec.end() ? *it : vec.empty() ? INVALID_OBJECT_ID : vec.front();

    if (m_current_fleet_id != INVALID_OBJECT_ID) {
        CenterOnObject(m_current_fleet_id);
        SelectFleet(m_current_fleet_id);
    }

    return true;
}

bool MapWnd::ZoomToPrevFleet() {
    auto vec = GetUniverse().Objects().FindObjectIDs(OwnedVisitor<Fleet>(HumanClientApp::GetApp()->EmpireID()));
    auto it = std::find(vec.begin(), vec.end(), m_current_fleet_id);
    const auto& destroyed_object_ids = GetUniverse().DestroyedObjectIds();
    if (it != vec.begin())
        --it;
    else
        it = vec.end();
    while (it != vec.begin() && (it == vec.end() || destroyed_object_ids.count(*it)))
        --it;
    m_current_fleet_id = it != vec.end() ? *it : vec.empty() ? INVALID_OBJECT_ID : vec.back();

    if (m_current_fleet_id != INVALID_OBJECT_ID) {
        CenterOnObject(m_current_fleet_id);
        SelectFleet(m_current_fleet_id);
    }

    return true;
}

bool MapWnd::ZoomToNextFleet() {
    auto vec = GetUniverse().Objects().FindObjectIDs(OwnedVisitor<Fleet>(HumanClientApp::GetApp()->EmpireID()));
    auto it = std::find(vec.begin(), vec.end(), m_current_fleet_id);
    auto& destroyed_object_ids = GetUniverse().DestroyedObjectIds();
    if (it != vec.end())
        ++it;
    while (it != vec.end() && destroyed_object_ids.count(*it))
        ++it;
    m_current_fleet_id = it != vec.end() ? *it : vec.empty() ? INVALID_OBJECT_ID : vec.front();

    if (m_current_fleet_id != INVALID_OBJECT_ID) {
        CenterOnObject(m_current_fleet_id);
        SelectFleet(m_current_fleet_id);
    }

    return true;
}

bool MapWnd::ZoomToSystemWithWastedPP() {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = GetEmpire(empire_id);
    if (!empire)
        return false;

    const ProductionQueue& queue = empire->GetProductionQueue();
    const auto pool = empire->GetResourcePool(RE_INDUSTRY);
    if (!pool)
        return false;
    auto wasted_PP_objects(queue.ObjectsWithWastedPP(pool));
    if (wasted_PP_objects.empty())
        return false;

    // pick first object in first group
    auto& obj_group = *wasted_PP_objects.begin();
    if (obj_group.empty())
        return false; // shouldn't happen?
    for (const auto& obj_ids : wasted_PP_objects) {
        for (int obj_id : obj_ids) {
            auto obj = GetUniverseObject(obj_id);
            if (obj && obj->SystemID() != INVALID_OBJECT_ID) {
                // found object with wasted PP that is in a system.  zoom there.
                CenterOnObject(obj->SystemID());
                SelectSystem(obj->SystemID());
                ShowProduction();
                return true;
            }
        }
    }
    return false;
}

namespace {
    /// On when the MapWnd window is visible and not covered
    //  by one of the full screen covering windows
    class NotCoveredMapWndCondition {
    protected:
        const MapWnd& target;

    public:
        NotCoveredMapWndCondition(const MapWnd& tg) : target(tg) {}
        bool operator()() const {
            return target.Visible() && !target.InResearchViewMode() && !target.InDesignViewMode();
        };
    };
}

void MapWnd::ConnectKeyboardAcceleratorSignals() {
    HotkeyManager* hkm = HotkeyManager::GetManager();

    hkm->Connect(boost::bind(&MapWnd::ReturnToMap, this), "ui.map.open",
                 AndCondition({VisibleWindowCondition(this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::EndTurn, this), "ui.turn.end",
                 AndCondition({VisibleWindowCondition(this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ToggleSitRep, this), "ui.map.sitrep",
                 AndCondition({VisibleWindowCondition(this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ToggleResearch, this), "ui.research",
                 AndCondition({VisibleWindowCondition(this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ToggleProduction, this), "ui.production",
                 AndCondition({VisibleWindowCondition(this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ToggleDesign, this), "ui.design",
                 AndCondition({VisibleWindowCondition(this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ToggleObjects, this), "ui.map.objects",
                 AndCondition({VisibleWindowCondition(this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ToggleMessages, this), "ui.map.messages",
                 AndCondition({VisibleWindowCondition(this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ToggleEmpires, this), "ui.map.empires",
                 AndCondition({VisibleWindowCondition(this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::TogglePedia, this), "ui.pedia",
                 AndCondition({VisibleWindowCondition(this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ShowGraphs, this), "ui.map.graphs",
                 AndCondition({VisibleWindowCondition(this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ShowMenu, this), "ui.gamemenu",
                 AndCondition({VisibleWindowCondition(this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::KeyboardZoomIn, this), "ui.zoom.in",
                 AndCondition({NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::KeyboardZoomIn, this), "ui.zoom.in.alt",
                 AndCondition({NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::KeyboardZoomOut, this), "ui.zoom.out",
                 AndCondition({NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::KeyboardZoomOut, this), "ui.zoom.out.alt",
                 AndCondition({NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ZoomToHomeSystem, this), "ui.map.system.zoom.home",
                 AndCondition({NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ZoomToPrevSystem, this), "ui.map.system.zoom.prev",
                 AndCondition({NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ZoomToNextSystem, this), "ui.map.system.zoom.next",
                 AndCondition({NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ZoomToPrevOwnedSystem, this), "ui.map.system.owned.zoom.prev",
                 AndCondition({NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ZoomToNextOwnedSystem, this), "ui.map.system.owned.zoom.next",
                 AndCondition({NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition}));

    // the list of windows for which the fleet shortcuts are blacklisted.
    std::initializer_list<const GG::Wnd*> bl = {m_research_wnd.get(), m_production_wnd.get(), m_design_wnd.get()};

    hkm->Connect(boost::bind(&MapWnd::ZoomToPrevFleet, this), "ui.map.fleet.zoom.prev",
                 AndCondition({OrCondition({InvisibleWindowCondition(bl), VisibleWindowCondition(this)}), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ZoomToNextFleet, this), "ui.map.fleet.zoom.next",
                 AndCondition({OrCondition({InvisibleWindowCondition(bl), VisibleWindowCondition(this)}), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ZoomToPrevIdleFleet, this), "ui.map.fleet.idle.zoom.prev",
                 AndCondition({OrCondition({InvisibleWindowCondition(bl), VisibleWindowCondition(this)}), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::ZoomToNextIdleFleet, this), "ui.map.fleet.idle.zoom.next",
                 AndCondition({OrCondition({InvisibleWindowCondition(bl), VisibleWindowCondition(this)}), NoModalWndsOpenCondition}));

    hkm->Connect(boost::bind(&MapWnd::PanX, this, GG::X(50)),   "ui.pan.right",
                 AndCondition({OrCondition({InvisibleWindowCondition(bl), VisibleWindowCondition(this)}), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::PanX, this, GG::X(-50)),  "ui.pan.left",
                 AndCondition({OrCondition({InvisibleWindowCondition(bl), VisibleWindowCondition(this)}), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::PanY, this, GG::Y(50)),   "ui.pan.down",
                 AndCondition({OrCondition({InvisibleWindowCondition(bl), VisibleWindowCondition(this)}), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&MapWnd::PanY, this, GG::Y(-50)),  "ui.pan.up",
                 AndCondition({OrCondition({InvisibleWindowCondition(bl), VisibleWindowCondition(this)}), NoModalWndsOpenCondition}));

    hkm->Connect(boost::bind(&ToggleBoolOption, "ui.map.scale.legend.shown"), "ui.map.scale.legend",
                 AndCondition({OrCondition({InvisibleWindowCondition(bl), VisibleWindowCondition(this)}), NoModalWndsOpenCondition}));
    hkm->Connect(boost::bind(&ToggleBoolOption, "ui.map.scale.circle.shown"), "ui.map.scale.circle",
                 AndCondition({OrCondition({InvisibleWindowCondition(bl), VisibleWindowCondition(this)}), NoModalWndsOpenCondition}));


    // these are general-use hotkeys, only connected here as a convenient location to do so once.
    hkm->Connect(boost::bind(&GG::GUI::CutFocusWndText, GG::GUI::GetGUI()), "ui.cut");
    hkm->Connect(boost::bind(&GG::GUI::CopyFocusWndText, GG::GUI::GetGUI()), "ui.copy");
    hkm->Connect(boost::bind(&GG::GUI::PasteFocusWndClipboardText, GG::GUI::GetGUI()), "ui.paste");

    hkm->Connect(boost::bind(&GG::GUI::FocusWndSelectAll, GG::GUI::GetGUI()), "ui.select.all");
    hkm->Connect(boost::bind(&GG::GUI::FocusWndDeselect, GG::GUI::GetGUI()), "ui.select.none");

    hkm->Connect(boost::bind(&GG::GUI::SetPrevFocusWndInCycle, GG::GUI::GetGUI()), "ui.focus.prev",
                 NoModalWndsOpenCondition);
    hkm->Connect(boost::bind(&GG::GUI::SetNextFocusWndInCycle, GG::GUI::GetGUI()), "ui.focus.next",
                 NoModalWndsOpenCondition);

    hkm->RebuildShortcuts();
}

void MapWnd::CloseAllPopups() {
    GG::ProcessThenRemoveExpiredPtrs(m_popups,
                                     [](std::shared_ptr<MapWndPopup>& wnd)
                                     { wnd->Close(); });
}

void MapWnd::HideAllPopups() {
    GG::ProcessThenRemoveExpiredPtrs(m_popups,
                                     [](std::shared_ptr<MapWndPopup>& wnd)
                                     { wnd->Hide(); });
}

void MapWnd::SetFleetExploring(const int fleet_id) {
    if (!std::count(m_fleets_exploring.begin(), m_fleets_exploring.end(), fleet_id)) {
        m_fleets_exploring.insert(fleet_id);
        DispatchFleetsExploring();
    }
}

void MapWnd::StopFleetExploring(const int fleet_id) {
    auto it = m_fleets_exploring.find(fleet_id);
    if (it == m_fleets_exploring.end())
        return;

    m_fleets_exploring.erase(it);

    DispatchFleetsExploring();
    // force UI update. Removing a fleet from the UI's list of exploring fleets
    // doesn't actually change the Fleet object's state in any way, so the UI
    // would otherwise still show the fleet as "exploring"
    if (auto fleet = GetFleet(fleet_id))
        fleet->StateChangedSignal();
}

bool MapWnd::IsFleetExploring(const int fleet_id){
    return std::count(m_fleets_exploring.begin(), m_fleets_exploring.end(), fleet_id);
}

namespace {
    typedef std::unordered_set<int> SystemIDListType;
    typedef std::unordered_set<int> FleetIDListType;
    typedef std::vector<int> RouteListType;
    typedef std::pair<double, RouteListType> OrderedRouteType;
    typedef std::pair<int, RouteListType> FleetRouteType;
    typedef std::pair<double, FleetRouteType> OrderedFleetRouteType;
    typedef std::unordered_map<int, int> SystemFleetMap;

    /** Number of jumps in a given route */
    int JumpsForRoute(const RouteListType& route) {
        int count = static_cast<int>(route.size());
        if (count > 0) // dont count source system
            -- count;
        return count;
    }

    /** If @p fleet can determine an eta for @p route */
    bool FleetRouteInRange(const std::shared_ptr<Fleet>& fleet, const RouteListType& route) {
        std::list<int> route_list;
        std::copy(route.begin(), route.end(), std::back_inserter(route_list));

        auto eta = fleet->ETA(fleet->MovePath(route_list));
        if (eta.first == Fleet::ETA_NEVER || eta.first == Fleet::ETA_UNKNOWN || eta.first == Fleet::ETA_OUT_OF_RANGE)
            return false;

        return true;
    }

    //helper function for DispatchFleetsExploring
    //return the set of all systems ID with a starlane connecting them to a system in set
    SystemIDListType AddNeighboorsToSet(const Empire *empire, const SystemIDListType& system_ids){
        SystemIDListType retval;
        auto starlanes = empire->KnownStarlanes();
        for (auto system_id : system_ids) {
            auto new_neighboors_it = starlanes.find(system_id);
            if (new_neighboors_it != starlanes.end()){
                for (auto neighbor_id : new_neighboors_it->second) {
                    retval.insert(neighbor_id);
                }
            }
        }

        return retval;
    }

    /** Get the shortest suitable route from @p start_id to @p destination_id as known to @p empire_id */
    OrderedRouteType GetShortestRoute(int empire_id, int start_id, int destination_id) {
        auto start_system = GetSystem(start_id);
        auto dest_system = GetSystem(destination_id);
        if (!start_system || !dest_system) {
            WarnLogger() << "Invalid start or destination system";
            return OrderedRouteType();
        }

        auto ignore_hostile = GetOptionsDB().Get<bool>("ui.fleet.explore.hostile.ignored");
        auto fleet_pred = std::make_shared<HostileVisitor<Fleet>>(empire_id);
        std::pair<std::list<int>, double> route_distance;

        if (ignore_hostile)
            route_distance = GetPathfinder()->ShortestPath(start_id, destination_id, empire_id);
        else
            route_distance = GetPathfinder()->ShortestPath(start_id, destination_id, empire_id, fleet_pred);

        if (!route_distance.first.empty() && route_distance.second > 0.0) {
            RouteListType route(route_distance.first.begin(), route_distance.first.end());
            return std::make_pair(route_distance.second, route);
        }

        return OrderedRouteType();
    }

    /** Route from @p fleet current location to @p destination */
    OrderedFleetRouteType GetOrderedFleetRoute(const std::shared_ptr<Fleet>& fleet,
                                               const std::shared_ptr<System>& destination)
    {
        if (!fleet || !destination) {
            WarnLogger() << "Invalid fleet or system";
            return OrderedFleetRouteType();
        }
        if ((fleet->Fuel() < 1.0f) || !fleet->MovePath().empty()) {
            WarnLogger() << "Fleet has no fuel or non-empty move path";
            return OrderedFleetRouteType();
        }

        auto order_route = GetShortestRoute(fleet->Owner(), fleet->SystemID(), destination->ID());

        if (order_route.first <= 0.0) {
            TraceLogger() << "No suitable route from system " << fleet->SystemID() << " to " << destination->ID()
                          << " (" << order_route.second.size() << ">" << order_route.first << ")";
            return OrderedFleetRouteType();
        }

        if (!FleetRouteInRange(fleet, order_route.second)) {
            TraceLogger() << "Fleet " << std::to_string(fleet->ID())
                          << " has no eta for route to " << std::to_string(*order_route.second.rbegin());
            return OrderedFleetRouteType();
        }

        // decrease priority of system if previously viewed but not yet explored
        if (!destination->Name().empty()) {
            order_route.first *= GetOptionsDB().Get<float>("ui.fleet.explore.system.known.multiplier");
            TraceLogger() << "Deferred priority for system " << destination->Name() << " (" << destination->ID() << ")";
        }

        auto fleet_route = std::make_pair(fleet->ID(), order_route.second);
        return std::make_pair(order_route.first, fleet_route);
    }

    /** Shortest route not exceeding @p max_jumps from @p dest_id to a system with supply as known to @p empire */
    OrderedRouteType GetNearestSupplyRoute(const Empire* empire, int dest_id, int max_jumps = -1) {
        OrderedRouteType retval;

        if (!empire) {
            WarnLogger() << "Invalid empire";
            return retval;
        }

        auto supplyable_systems = GetSupplyManager().FleetSupplyableSystemIDs(empire->EmpireID(), true);
        if (!supplyable_systems.empty()) {
            TraceLogger() << [supplyable_systems]() {
                    std::string msg = "Supplyable systems:";
                    for (auto sys : supplyable_systems)
                        msg.append(" " + std::to_string(sys));
                    return msg;
                }();
        }

            OrderedRouteType shortest_route;

        for (auto supply_system_id : supplyable_systems) {
            shortest_route = GetShortestRoute(empire->EmpireID(), dest_id, supply_system_id);
            TraceLogger() << [shortest_route, dest_id]() {
                    std::string msg = "Checking supply route from " + std::to_string(dest_id) +
                                      " dist:" + std::to_string(shortest_route.first) + " systems:";
                    for (auto node : shortest_route.second)
                        msg.append(" " + std::to_string(node));
                    return msg;
                }();

            auto route_jumps = JumpsForRoute(shortest_route.second);
            if (max_jumps > -1 && route_jumps > max_jumps) {
                TraceLogger() << "Rejecting route to " << std::to_string(*shortest_route.second.rbegin())
                              << " jumps " << std::to_string(route_jumps) << " exceed max " << std::to_string(max_jumps);
                continue;
            }

            if (shortest_route.first <= 0.0 || shortest_route.second.empty()) {
                TraceLogger() << "Invalid route";
                continue;
            }

            if (retval.first <= 0.0 || shortest_route.first < retval.first) {
                TraceLogger() << "Setting " << std::to_string(*shortest_route.second.rbegin()) << " as shortest route";
                retval = shortest_route;
            }
        }

        return retval;
    }

    /** If @p fleet would be able to reach a system with supply after completing @p route */
    bool CanResupplyAfterDestination(const std::shared_ptr<Fleet>& fleet, const RouteListType& route) {
        if (!fleet || route.empty()) {
            WarnLogger() << "Invalid fleet or empty route";
            return false;
        }
        auto empire = GetEmpire(fleet->Owner());
        if (!empire) {
            WarnLogger() << "Invalid empire";
            return false;
        }

        int max_jumps = std::trunc(fleet->Fuel());
        if (max_jumps < 1) {
            TraceLogger() << "Not enough fuel " << std::to_string(max_jumps)
                          << " to move fleet " << std::to_string(fleet->ID());
            return false;
        }

        auto dest_nearest_supply = GetNearestSupplyRoute(empire, *route.rbegin(), max_jumps);
        auto dest_nearest_supply_jumps = JumpsForRoute(dest_nearest_supply.second);
        auto dest_jumps = JumpsForRoute(route);
        int total_jumps = dest_jumps + dest_nearest_supply_jumps;

        if (total_jumps > max_jumps) {
            TraceLogger() << "Not enough fuel " << std::to_string(max_jumps)
                          << " for fleet " << std::to_string(fleet->ID())
                          << " to resupply after destination " << std::to_string(total_jumps);
            return false;
        }

        return true;
    }

    /** Route from current system of @p fleet to nearest system with supply as determined by owning empire of @p fleet  */
    OrderedRouteType ExploringFleetResupplyRoute(const std::shared_ptr<Fleet>& fleet) {
        auto empire = GetEmpire(fleet->Owner());
        if (!empire) {
            WarnLogger() << "Invalid empire for id " << fleet->Owner();
            return OrderedRouteType();
        }

        auto nearest_supply = GetNearestSupplyRoute(empire, fleet->SystemID(), std::trunc(fleet->Fuel()));
        if (nearest_supply.first > 0.0 && FleetRouteInRange(fleet, nearest_supply.second)) {
            return nearest_supply;
        }

        return OrderedRouteType();
    }

    /** Issue an order for @p fleet to move to nearest system with supply */
    bool IssueFleetResupplyOrder(const std::shared_ptr<Fleet>& fleet) {
        if (!fleet) {
            WarnLogger() << "Invalid fleet";
            return false;
        }

        auto route = ExploringFleetResupplyRoute(fleet);
        // Attempt move order if route is not empty and fleet has enough fuel to reach it
        if (route.second.empty()) {
            TraceLogger() << "Empty route for resupply of exploring fleet " << fleet->ID();
            return false;
        }

        auto num_jumps_resupply = JumpsForRoute(route.second);
        int max_fleet_jumps = std::trunc(fleet->Fuel());
        if (num_jumps_resupply <= max_fleet_jumps) {
            HumanClientApp::GetApp()->Orders().IssueOrder(
                    std::make_shared<FleetMoveOrder>(fleet->Owner(), fleet->ID(), *route.second.rbegin()));
        } else {
            TraceLogger() << "Not enough fuel for fleet " << fleet->ID()
                          << " to resupply at system " << *route.second.rbegin();
            return false;
        }

        if (fleet->FinalDestinationID() == *route.second.rbegin()) {
            TraceLogger() << "Sending fleet " << fleet->ID()
                          << " to refuel at system " << *route.second.rbegin();
            return true;
        } else {
            TraceLogger() << "Fleet move order failed fleet:" << fleet->ID() << " route:"
                          << [route]() {
                                 std::string retval = "";
                                 for (auto node : route.second)
                                     retval.append(" " + std::to_string(node));
                                 return retval;
                             }();
        }

        return false;
    }

    /** Issue order for @p fleet to move using @p route */
    bool IssueFleetExploreOrder(const std::shared_ptr<Fleet>& fleet, const RouteListType& route) {
        if (!fleet || route.empty()) {
            WarnLogger() << "Invalid fleet or empty route";
            return false;
        }
        if (!FleetRouteInRange(fleet, route)) {
            TraceLogger() << "Fleet " << std::to_string(fleet->ID())
                          << " has no eta for route to " << std::to_string(*route.rbegin());
            return false;
        }

        HumanClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<FleetMoveOrder>(fleet->Owner(), fleet->ID(), *route.rbegin()));
        if (fleet->FinalDestinationID() == *route.rbegin()) {
            TraceLogger() << "Sending fleet " << fleet->ID() << " to explore system " << *route.rbegin();
            return true;
        }

        TraceLogger() << "Fleet move order failed fleet:" << fleet->ID() << " dest:" << *route.rbegin();
        return false;
    }

    /** Determine and issue move order for fleet and route @p fleet_route */
    void IssueExploringFleetOrders(FleetIDListType& idle_fleets,
                                   SystemFleetMap& systems_being_explored,
                                   const FleetRouteType& fleet_route)
    {
        auto route = fleet_route.second;
        if (route.empty()) { // no route
            WarnLogger() << "Attempted to issue move order with empty route";
            return;
        }

        if (idle_fleets.empty()) { // no more fleets to issue orders to
            TraceLogger() << "No idle fleets";
            return;
        }

        if (systems_being_explored.count(*route.rbegin())) {
            TraceLogger() << "System " << std::to_string(*route.rbegin()) << " already being explored";
            return;
        }

        auto fleet_id = fleet_route.first;
        auto idle_fleet_it = idle_fleets.find(fleet_id);
        if (idle_fleet_it == idle_fleets.end()) { // fleet no longer idle
            TraceLogger() << "Fleet " << std::to_string(fleet_id) << " not idle";
            return;
        }
        auto fleet = GetFleet(fleet_id);
        if (!fleet) {
            ErrorLogger() << "No valid fleet with id " << fleet_id;
            idle_fleets.erase(idle_fleet_it);
            return;
        }

        if (std::trunc(fleet->Fuel()) < 1) {  // wait for fuel
            TraceLogger() << "Not enough fuel to move fleet " << std::to_string(fleet->ID());
            return;
        }

        // Determine if fleet should refuel
        if (fleet->Fuel() < fleet->MaxFuel() &&
            !CanResupplyAfterDestination(fleet, route))
        {
            if (IssueFleetResupplyOrder(fleet)) {
                idle_fleets.erase(idle_fleet_it);
                return;
            }
            TraceLogger() << "Fleet " << std::to_string(fleet->ID()) << " can not reach resupply";
        }

        if (IssueFleetExploreOrder(fleet, route)) {
            idle_fleets.erase(idle_fleet_it);
            systems_being_explored.emplace(*route.rbegin(), fleet->ID());
        }
    }

};

void MapWnd::DispatchFleetsExploring() {
    DebugLogger() << "MapWnd::DispatchFleetsExploring called";
    SectionedScopedTimer timer("MapWnd::DispatchFleetsExploring", true);

    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire *empire = GetEmpire(empire_id);
    if (!empire) {
        WarnLogger() << "Invalid empire";
        return;
    }
    int max_routes_per_system = GetOptionsDB().Get<int>("ui.fleet.explore.system.route.limit");
    auto destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(empire_id);

    FleetIDListType idle_fleets;
    /** all systems ID for which an exploring fleet is in route and the fleet assigned */
    SystemFleetMap systems_being_explored;

    // clean the fleet list by removing non-existing fleet, and extract the
    // fleets waiting for orders
    timer.EnterSection("idle fleets/systems being explored");
    for (auto it = m_fleets_exploring.begin(); it != m_fleets_exploring.end();) {
        auto fleet = GetFleet(*it);
        if (!fleet || destroyed_objects.count(fleet->ID())) {
            it = m_fleets_exploring.erase(it); //this fleet can't explore anymore
        } else {
             if (fleet->MovePath().empty())
                idle_fleets.insert(fleet->ID());
            else
                systems_being_explored.emplace(fleet->FinalDestinationID(), fleet->ID());
            ++it;
        }
    }
    timer.EnterSection("");

    if (idle_fleets.empty())
        return;

    TraceLogger() << [idle_fleets]() {
            std::string retval = "MapWnd::DispatchFleetsExploring Idle Exploring Fleet IDs:";
            for (auto fleet : idle_fleets)
                retval.append(" " + std::to_string(fleet));
            return retval;
        }();

    //list all unexplored systems by taking the neighboors of explored systems because ObjectMap does not list them all.
    timer.EnterSection("candidate unknown systems");
    SystemIDListType candidates_unknown_systems;
    const auto& empire_explored_systems = empire->ExploredSystems();
    SystemIDListType explored_systems(empire_explored_systems.begin(), empire_explored_systems.end());
    candidates_unknown_systems = AddNeighboorsToSet(empire, explored_systems);
    auto neighboors = AddNeighboorsToSet(empire, candidates_unknown_systems);
    candidates_unknown_systems.insert(neighboors.begin(), neighboors.end());

    // Populate list of unexplored systems
    timer.EnterSection("unexplored systems");
    SystemIDListType unexplored_systems;
    for (int system_id : candidates_unknown_systems) {
        auto system = GetSystem(system_id);
        if (!system)
            continue;
        if (!empire->HasExploredSystem(system->ID()) &&
            !systems_being_explored.count(system_id))
        { unexplored_systems.insert(system->ID()); }
    }
    timer.EnterSection("");

    if (unexplored_systems.empty()) {
        TraceLogger() << "No unknown systems to explore";
        return;
    }

    TraceLogger() << [unexplored_systems]() {
            std::string retval = "MapWnd::DispatchFleetsExploring Unknown System IDs:";
            for (auto system : unexplored_systems)
                retval.append(" " + std::to_string(system));
            return retval;
        }();

    std::multimap<double, FleetRouteType> fleet_routes;  // priority, (fleet, route)

    // Determine fleet routes for each unexplored system
    timer.EnterSection("fleet_routes");
    std::unordered_map<int, int> fleet_route_count;
    for (const auto& unexplored_system_id : unexplored_systems) {
        auto unexplored_system = GetSystem(unexplored_system_id);
        if (!unexplored_system) {
            WarnLogger() << "Invalid system " << unexplored_system_id;
            continue;
        }

        for (const auto& fleet_id : idle_fleets) {
            if (max_routes_per_system > 0 &&
                fleet_route_count[unexplored_system_id] > max_routes_per_system)
            { break; }

            auto fleet = GetFleet(fleet_id);
            if (!fleet) {
                WarnLogger() << "Invalid fleet " << fleet_id;
                continue;
            }
            if (fleet->Fuel() < 1.0f)
                continue;

            auto route = GetOrderedFleetRoute(fleet, unexplored_system);
            if (route.first > 0.0) {
                ++fleet_route_count[unexplored_system_id];
                fleet_routes.emplace(route);
            }
        }
    }
    timer.EnterSection("");

    if (!fleet_routes.empty()) {
        TraceLogger() << [fleet_routes]() {
                std::string retval = "MapWnd::DispatchFleetsExploring Explorable Systems:\n\t Priority\tFleet\tDestination";
                for (auto route : fleet_routes) {
                    retval.append("\n\t" + std::to_string(route.first) + "\t" + std::to_string(route.second.first) +
                                  "\t " + std::to_string(route.second.second.empty() ? -1 : *route.second.second.rbegin()));
                }
                return retval;
            }();
    }

    // Issue fleet orders
    timer.EnterSection("issue orders");
    for (auto fleet_route : fleet_routes) {
        IssueExploringFleetOrders(idle_fleets, systems_being_explored, fleet_route.second);
    }
    timer.EnterSection("");

    // verify fleets have expected destination
    for (SystemFleetMap::iterator system_fleet_it = systems_being_explored.begin();
         system_fleet_it != systems_being_explored.end(); ++system_fleet_it)
    {
        auto fleet = GetFleet(system_fleet_it->second);
        if (!fleet)
            continue;

        auto dest_id = fleet->FinalDestinationID();
        if (dest_id == system_fleet_it->first)
            continue;

        WarnLogger() << "Non idle exploring fleet "<< system_fleet_it->second << " has differing destination:"
                     << fleet->FinalDestinationID() << " expected:" << system_fleet_it->first;

        idle_fleets.insert(system_fleet_it->second);
        // systems_being_explored.erase(system_fleet_it);
    }

    if (!idle_fleets.empty()) {
        DebugLogger() << [idle_fleets]() {
                std::string retval = "MapWnd::DispatchFleetsExploring Idle exploring fleets after orders:";
                for (auto fleet_id : idle_fleets)
                    retval.append(" " + std::to_string(fleet_id));
                return retval;
            }();
    }
}

void MapWnd::ShowAllPopups() {
    GG::ProcessThenRemoveExpiredPtrs(m_popups,
                                     [](std::shared_ptr<MapWndPopup>& wnd)
                                     { wnd->Show(); });
}
