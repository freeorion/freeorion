#include "MapWnd.h"

#include <deque>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <valarray>
#include <vector>
#include <boost/graph/graph_concepts.hpp>
#include <boost/optional/optional.hpp>
#include <boost/unordered_set.hpp>
#include <GG/Layout.h>
#include <GG/MultiEdit.h>
#include <GG/PtRect.h>
#include <GG/WndEvent.h>
#include "CensusBrowseWnd.h"
#include "ChatWnd.h"
#include "ClientUI.h"
#include "CombatReport/CombatReportWnd.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"
#include "DesignWnd.h"
#include "EncyclopediaDetailPanel.h"
#include "FieldIcon.h"
#include "FleetButton.h"
#include "FleetWnd.h"
#include "GovernmentWnd.h"
#include "Hotkeys.h"
#include "InGameMenu.h"
#include "ModeratorActionsWnd.h"
#include "ObjectListWnd.h"
#include "PlayerListWnd.h"
#include "ProductionWnd.h"
#include "ResearchWnd.h"
#include "ResourceBrowseWnd.h"
#include "ShaderProgram.h"
#include "SidePanel.h"
#include "SitRepPanel.h"
#include "Sound.h"
#include "SystemIcon.h"
#include "TextBrowseWnd.h"
#include "../client/ClientNetworking.h"
#include "../client/human/GGHumanClientApp.h"
#include "../Empire/Empire.h"
#include "../network/Message.h"
#include "../universe/Field.h"
#include "../universe/Fleet.h"
#include "../universe/Pathfinder.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/Ship.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/UniverseObject.h"
#include "../universe/Universe.h"
#include "../util/GameRules.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/ModeratorAction.h"
#include "../util/OptionsDB.h"
#include "../util/Order.h"
#include "../util/Random.h"
#include "../util/ranges.h"
#include "../util/ScopedTimer.h"


namespace {
    consteval double Pow(double base, int exp) {
        double retval = 1.0;
        bool invert = exp < 0;
        std::size_t abs_exp = exp >= 0 ? exp : -exp;
        while (abs_exp--)
            retval *= base;
        return invert ? (1.0 / retval) : retval;
    }

    // "Babylonian Method" of finding square roots...
    consteval double SqrtIterative2(double a, double c) {
        double g = 0.5 * (c + a/c);
        return (g == c) ? g : SqrtIterative2(a, g);
    }
    consteval double Sqrt(double a)
    { return SqrtIterative2(a, a); }

    constexpr double ZOOM_STEP_SIZE = Sqrt(Sqrt(2.0));
    constexpr double ZOOM_IN_MAX_STEPS = 12.0;
    constexpr double ZOOM_IN_MIN_STEPS = -10.0; // negative zoom steps indicates zooming out
    constexpr double ZOOM_MAX = Pow(ZOOM_STEP_SIZE, ZOOM_IN_MAX_STEPS);
    constexpr double ZOOM_MIN = Pow(ZOOM_STEP_SIZE, ZOOM_IN_MIN_STEPS);

    constexpr GG::X  SITREP_PANEL_WIDTH{400};
    constexpr GG::Y  SITREP_PANEL_HEIGHT{200};

    const std::string SITREP_WND_NAME = "map.sitrep";
    const std::string MAP_PEDIA_WND_NAME = "map.pedia";
    const std::string OBJECT_WND_NAME = "map.object-list";
    const std::string MODERATOR_WND_NAME = "map.moderator";
    const std::string COMBAT_REPORT_WND_NAME = "combat.summary";
    const std::string MAP_SIDEPANEL_WND_NAME = "map.sidepanel";
    const std::string GOVERNMENT_WND_NAME = "map.government";

    constexpr GG::Y ZOOM_SLIDER_HEIGHT{200};
    constexpr GG::Y SCALE_LINE_HEIGHT{20};
    constexpr GG::X SCALE_LINE_MAX_WIDTH{240};
    constexpr int   MIN_SYSTEM_NAME_SIZE = 10;
    constexpr int   LAYOUT_MARGIN = 5;
    constexpr GG::Y TOOLBAR_HEIGHT{32};

    constexpr double TWO_PI = 2.0*3.1415926536;

    constexpr GG::X ICON_SINGLE_WIDTH{40};
    constexpr GG::X ICON_DUAL_WIDTH{64};
    constexpr GG::X ICON_WIDTH{24};
    constexpr GG::Pt ICON_SIZE{GG::X{24}, GG::Y{24}};
    constexpr GG::Pt MENU_ICON_SIZE{GG::X{32}, GG::Y{32}};


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
        db.Add("ui.map.background.starfields.scale",        UserStringNop("OPTIONS_DB_GALAXY_MAP_STARFIELDS_SCALE"),            1.0,                            RangedStepValidator<double>(0.2, 0.2, 4.0));

        db.Add("ui.map.scale.legend.shown",                 UserStringNop("OPTIONS_DB_GALAXY_MAP_SCALE_LINE"),                  true,                           Validator<bool>());
        db.Add("ui.map.scale.circle.shown",                 UserStringNop("OPTIONS_DB_GALAXY_MAP_SCALE_CIRCLE"),                false,                          Validator<bool>());

        db.Add("ui.map.zoom.slider.shown",                  UserStringNop("OPTIONS_DB_GALAXY_MAP_ZOOM_SLIDER"),                 false,                          Validator<bool>());

        db.Add("ui.map.starlane.thickness",                 UserStringNop("OPTIONS_DB_STARLANE_THICKNESS"),                     2.0,                            RangedStepValidator<double>(0.2, 0.2, 10.0));
        db.Add("ui.map.starlane.thickness.factor",          UserStringNop("OPTIONS_DB_STARLANE_CORE"),                          2.0,                            RangedStepValidator<double>(1.0, 1.0, 10.0));
        db.Add("ui.map.starlane.empire.color.shown",        UserStringNop("OPTIONS_DB_RESOURCE_STARLANE_COLOURING"),            true,                           Validator<bool>());

        db.Add("ui.map.fleet.supply.shown",                 UserStringNop("OPTIONS_DB_FLEET_SUPPLY_LINES"),                     true,                           Validator<bool>());
        db.Add("ui.map.fleet.supply.width",                 UserStringNop("OPTIONS_DB_FLEET_SUPPLY_LINE_WIDTH"),                3.0,                            RangedStepValidator<double>(0.2, 0.2, 10.0));
        db.Add("ui.map.fleet.supply.dot.spacing",           UserStringNop("OPTIONS_DB_FLEET_SUPPLY_LINE_DOT_SPACING"),          20,                             RangedStepValidator<int>(1, 3, 40));
        db.Add("ui.map.fleet.supply.dot.rate",              UserStringNop("OPTIONS_DB_FLEET_SUPPLY_LINE_DOT_RATE"),             0.02,                           RangedStepValidator<double>(0.01, 0.01, 0.1));

        //db.Add("ui.fleet.explore.hostile.ignored",          UserStringNop("OPTIONS_DB_FLEET_EXPLORE_IGNORE_HOSTILE"),           false,                          Validator<bool>());
        db.Add("ui.fleet.explore.system.route.limit",       UserStringNop("OPTIONS_DB_FLEET_EXPLORE_SYSTEM_ROUTE_LIMIT"),       25,                             StepValidator<int>(1, -1));
        db.Add("ui.fleet.explore.system.known.multiplier",  UserStringNop("OPTIONS_DB_FLEET_EXPLORE_SYSTEM_KNOWN_MULTIPLIER"),  10.0f,                          Validator<float>());

        db.Add("ui.map.starlane.color",                     UserStringNop("OPTIONS_DB_UNOWNED_STARLANE_COLOUR"),                GG::Clr(72,  72,  72,  255),    Validator<GG::Clr>());

        db.Add("ui.map.detection.range.shown",              UserStringNop("OPTIONS_DB_GALAXY_MAP_DETECTION_RANGE"),             true,                           Validator<bool>());
        db.Add("ui.map.detection.range.future.shown",       UserStringNop("OPTIONS_DB_GALAXY_MAP_DETECTION_RANGE_FUTURE"),      true,                           Validator<bool>());

        db.Add("ui.map.scanlines.shown",                    UserStringNop("OPTIONS_DB_UI_SYSTEM_FOG"),                          true,                           Validator<bool>());
        db.Add("ui.map.system.scanlines.spacing",           UserStringNop("OPTIONS_DB_UI_SYSTEM_FOG_SPACING"),                  4.0,                            RangedStepValidator<double>(0.2, 1.4, 8.0));
        db.Add("ui.map.system.scanlines.color",             UserStringNop("OPTIONS_DB_UI_SYSTEM_FOG_CLR"),                      GG::Clr(36, 36, 36, 192),       Validator<GG::Clr>());
        db.Add("ui.map.field.scanlines.color",              UserStringNop("OPTIONS_DB_UI_FIELD_FOG_CLR"),                       GG::Clr(0, 0, 0, 64),           Validator<GG::Clr>());

        db.Add("ui.map.system.icon.size",                   UserStringNop("OPTIONS_DB_UI_SYSTEM_ICON_SIZE"),                    14,                             RangedValidator<int>(8, 50));

        db.Add("ui.map.system.circle.shown",                UserStringNop("OPTIONS_DB_UI_SYSTEM_CIRCLES"),                      true,                           Validator<bool>());
        db.Add("ui.map.system.circle.size",                 UserStringNop("OPTIONS_DB_UI_SYSTEM_CIRCLE_SIZE"),                  1.5,                            RangedStepValidator<double>(0.1, 1.0, 2.5));
        db.Add("ui.map.system.circle.inner.width",          UserStringNop("OPTIONS_DB_UI_SYSTEM_INNER_CIRCLE_WIDTH"),           2.0,                            RangedStepValidator<double>(0.5, 1.0, 8.0));
        db.Add("ui.map.system.circle.outer.width",          UserStringNop("OPTIONS_DB_UI_SYSTEM_OUTER_CIRCLE_WIDTH"),           2.0,                            RangedStepValidator<double>(0.5, 1.0, 8.0));
        db.Add("ui.map.system.circle.inner.max.width",      UserStringNop("OPTIONS_DB_UI_SYSTEM_INNER_CIRCLE_MAX_WIDTH"),       5.0,                            RangedStepValidator<double>(0.5, 1.0, 12.0));
        db.Add("ui.map.system.circle.distance",             UserStringNop("OPTIONS_DB_UI_SYSTEM_CIRCLE_DISTANCE"),              2.0,                            RangedStepValidator<double>(0.5, 1.0, 8.0));

        db.Add("ui.map.system.unexplored.rollover.enabled", UserStringNop("OPTIONS_DB_UI_SYSTEM_UNEXPLORED_OVERLAY"),           true,                           Validator<bool>());

        db.Add("ui.map.system.icon.tiny.threshold",         UserStringNop("OPTIONS_DB_UI_SYSTEM_TINY_ICON_SIZE_THRESHOLD"),     10,                             RangedValidator<int>(1, 16));

        db.Add("ui.map.system.select.indicator.size",       UserStringNop("OPTIONS_DB_UI_SYSTEM_SELECTION_INDICATOR_SIZE"),     1.6,                            RangedStepValidator<double>(0.1, 0.5, 5));
        db.Add("ui.map.system.select.indicator.rpm",        UserStringNop("OPTIONS_DB_UI_SYSTEM_SELECTION_INDICATOR_FPS"),      12,                             RangedValidator<int>(1, 60));

        db.Add("ui.map.system.unowned.name.color",          UserStringNop("OPTIONS_DB_UI_SYSTEM_NAME_UNOWNED_COLOR"),           GG::Clr(160, 160, 160, 255),    Validator<GG::Clr>());

        db.Add("ui.map.fleet.button.tiny.zoom.threshold",   UserStringNop("OPTIONS_DB_UI_TINY_FLEET_BUTTON_MIN_ZOOM"),          0.8,                            RangedStepValidator<double>(0.1, 0.1, 4.0));
        db.Add("ui.map.fleet.button.small.zoom.threshold",  UserStringNop("OPTIONS_DB_UI_SMALL_FLEET_BUTTON_MIN_ZOOM"),         1.50,                           RangedStepValidator<double>(0.1, 0.1, 4.0));
        db.Add("ui.map.fleet.button.medium.zoom.threshold", UserStringNop("OPTIONS_DB_UI_MEDIUM_FLEET_BUTTON_MIN_ZOOM"),        3.00,                           RangedStepValidator<double>(0.1, 0.1, 8.0));
        db.Add("ui.map.fleet.button.big.zoom.threshold",    UserStringNop("OPTIONS_DB_UI_BIG_FLEET_BUTTON_MIN_ZOOM"),           6.00,                           RangedStepValidator<double>(0.1, 0.1, 8.0));

        db.Add("ui.map.detection.range.opacity",            UserStringNop("OPTIONS_DB_GALAXY_MAP_DETECTION_RANGE_OPACITY"),     3,                              RangedValidator<int>(0, 8));

        db.Add("ui.map.lock",                               UserStringNop("OPTIONS_DB_UI_GALAXY_MAP_LOCK"),                     false,                          Validator<bool>());

        db.Add("ui.map.menu.enabled",                       UserStringNop("OPTIONS_DB_UI_GALAXY_MAP_POPUP"),                    false,                          Validator<bool>());

        db.Add("ui.production.mappanels.removed",           UserStringNop("OPTIONS_DB_UI_HIDE_MAP_PANELS"),                     false,                          Validator<bool>());

        db.Add("ui.map.sidepanel.width",                    UserStringNop("OPTIONS_DB_UI_SIDEPANEL_WIDTH"),                     512,                            Validator<int>());

        db.Add("ui.map.sidepanel.meter-refresh",            UserStringNop("OPTIONS_DB_UI_SIDEPANEL_OPEN_METER_UPDATE"),         true,                           Validator<bool>());
        db.Add("ui.map.object-changed.meter-refresh",       UserStringNop("OPTIONS_DB_UI_OBJECT_CHANGED_METER_UPDATE"),         true,                           Validator<bool>());

        // Register hotkey names/default values for the context "map".
        Hotkey::AddHotkey("ui.map.open",                    UserStringNop("HOTKEY_MAP_RETURN_TO_MAP"),                          GG::Key::GGK_ESCAPE);
        Hotkey::AddHotkey("ui.turn.end",                    UserStringNop("HOTKEY_MAP_END_TURN"),                               GG::Key::GGK_RETURN,            GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.sitrep",                  UserStringNop("HOTKEY_MAP_SIT_REP"),                                GG::Key::GGK_n,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.research",                    UserStringNop("HOTKEY_MAP_RESEARCH"),                               GG::Key::GGK_r,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.production",                  UserStringNop("HOTKEY_MAP_PRODUCTION"),                             GG::Key::GGK_p,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.government",                  UserStringNop("HOTKEY_MAP_GOVERNMENT"),                             GG::Key::GGK_i,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.design",                      UserStringNop("HOTKEY_MAP_DESIGN"),                                 GG::Key::GGK_d,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.objects",                 UserStringNop("HOTKEY_MAP_OBJECTS"),                                GG::Key::GGK_o,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.messages",                UserStringNop("HOTKEY_MAP_MESSAGES"),                               GG::Key::GGK_t,                 GG::MOD_KEY_ALT);
        Hotkey::AddHotkey("ui.map.empires",                 UserStringNop("HOTKEY_MAP_EMPIRES"),                                GG::Key::GGK_e,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.pedia",                       UserStringNop("HOTKEY_MAP_PEDIA"),                                  GG::Key::GGK_F1);
        Hotkey::AddHotkey("ui.map.graphs",                  UserStringNop("HOTKEY_MAP_GRAPHS"),                                 GG::Key::GGK_NONE);
        Hotkey::AddHotkey("ui.gamemenu",                    UserStringNop("HOTKEY_MAP_MENU"),                                   GG::Key::GGK_F10);
        Hotkey::AddHotkey("ui.zoom.in",                     UserStringNop("HOTKEY_MAP_ZOOM_IN"),                                GG::Key::GGK_z,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.zoom.in.alt",                 UserStringNop("HOTKEY_MAP_ZOOM_IN_ALT"),                            GG::Key::GGK_KP_PLUS,           GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.zoom.out",                    UserStringNop("HOTKEY_MAP_ZOOM_OUT"),                               GG::Key::GGK_x,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.zoom.out.alt",                UserStringNop("HOTKEY_MAP_ZOOM_OUT_ALT"),                           GG::Key::GGK_KP_MINUS,          GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.system.zoom.home",        UserStringNop("HOTKEY_MAP_ZOOM_HOME_SYSTEM"),                       GG::Key::GGK_h,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.system.zoom.prev",        UserStringNop("HOTKEY_MAP_ZOOM_PREV_SYSTEM"),                       GG::Key::GGK_COMMA,             GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.system.zoom.next",        UserStringNop("HOTKEY_MAP_ZOOM_NEXT_SYSTEM"),                       GG::Key::GGK_PERIOD,            GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.system.owned.zoom.prev",  UserStringNop("HOTKEY_MAP_ZOOM_PREV_OWNED_SYSTEM"),                 GG::Key::GGK_COMMA,             GG::MOD_KEY_CTRL | GG::MOD_KEY_SHIFT);
        Hotkey::AddHotkey("ui.map.system.owned.zoom.next",  UserStringNop("HOTKEY_MAP_ZOOM_NEXT_OWNED_SYSTEM"),                 GG::Key::GGK_PERIOD,            GG::MOD_KEY_CTRL | GG::MOD_KEY_SHIFT);
        Hotkey::AddHotkey("ui.map.fleet.zoom.prev",         UserStringNop("HOTKEY_MAP_ZOOM_PREV_FLEET"),                        GG::Key::GGK_f,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.fleet.zoom.next",         UserStringNop("HOTKEY_MAP_ZOOM_NEXT_FLEET"),                        GG::Key::GGK_g,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.map.fleet.idle.zoom.prev",    UserStringNop("HOTKEY_MAP_ZOOM_PREV_IDLE_FLEET"),                   GG::Key::GGK_f,                 GG::MOD_KEY_ALT);
        Hotkey::AddHotkey("ui.map.fleet.idle.zoom.next",    UserStringNop("HOTKEY_MAP_ZOOM_NEXT_IDLE_FLEET"),                   GG::Key::GGK_g,                 GG::MOD_KEY_ALT);

        Hotkey::AddHotkey("ui.pan.right",                   UserStringNop("HOTKEY_MAP_PAN_RIGHT"),                              GG::Key::GGK_RIGHT,             GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.pan.left",                    UserStringNop("HOTKEY_MAP_PAN_LEFT"),                               GG::Key::GGK_LEFT,              GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.pan.up",                      UserStringNop("HOTKEY_MAP_PAN_UP"),                                 GG::Key::GGK_UP,                GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.pan.down",                    UserStringNop("HOTKEY_MAP_PAN_DOWN"),                               GG::Key::GGK_DOWN,              GG::MOD_KEY_CTRL);

        Hotkey::AddHotkey("ui.map.scale.legend",            UserStringNop("HOTKEY_MAP_TOGGLE_SCALE_LINE"),                      GG::Key::GGK_l,                 GG::MOD_KEY_ALT);
        Hotkey::AddHotkey("ui.map.scale.circle",            UserStringNop("HOTKEY_MAP_TOGGLE_SCALE_CIRCLE"),                    GG::Key::GGK_c,                 GG::MOD_KEY_ALT);

        Hotkey::AddHotkey("ui.cut",                         UserStringNop("HOTKEY_CUT"),                                        GG::Key::GGK_x,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.copy",                        UserStringNop("HOTKEY_COPY"),                                       GG::Key::GGK_c,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.paste",                       UserStringNop("HOTKEY_PASTE"),                                      GG::Key::GGK_v,                 GG::MOD_KEY_CTRL);

        Hotkey::AddHotkey("ui.select.all",                  UserStringNop("HOTKEY_SELECT_ALL"),                                 GG::Key::GGK_a,                 GG::MOD_KEY_CTRL);
        Hotkey::AddHotkey("ui.select.none",                 UserStringNop("HOTKEY_DESELECT"),                                   GG::Key::GGK_d,                 GG::MOD_KEY_CTRL);

        // stepping through UI controls doesn't really work as of this writing, so I'm commenting out these hotkey commands
        //Hotkey::AddHotkey("ui.focus.prev",                  UserStringNop("HOTKEY_FOCUS_PREV_WND"),                             GG::Key::GGK_NONE);
        //Hotkey::AddHotkey("ui.focus.next",                  UserStringNop("HOTKEY_FOCUS_NEXT_WND"),                             GG::Key::GGK_NONE);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    /* Returns fractional distance along line segment between two points that a
     * third point between them is.assumes the "mid" point is between the
     * "start" and "end" points, in which case the returned fraction is between
     * 0.0 and 1.0 */
    double FractionalDistanceBetweenPoints(double startX, double startY, double midX,
                                           double midY, double endX, double endY)
    {
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
    constexpr auto PositionFractionalAtDistanceBetweenPoints(
        double X1, double Y1, double X2, double Y2, double dist)
    {
        double newX = X1 + (X2 - X1) * dist;
        double newY = Y1 + (Y2 - Y1) * dist;
        return std::pair<double, double>{newX, newY};
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
    boost::optional<std::pair<double, double>> ScreenPosOnStarlane(
        double X, double Y, int lane_start_sys_id, int lane_end_sys_id,
        LaneEndpoints screen_lane_endpoints, const ScriptingContext& context)
    {
        // get endpoints of lane in universe.  may be different because on-
        // screen lanes are drawn between system circles, not system centres
        auto prev = context.ContextObjects().get(lane_start_sys_id);
        auto next = context.ContextObjects().get(lane_end_sys_id);
        if (!next || !prev) {
            ErrorLogger() << "ScreenPosOnStarlane couldn't find next system " << lane_start_sys_id
                          << " or prev system " << lane_end_sys_id;
            return boost::none;
        }

        // get fractional distance along lane that fleet's universe position is
        double dist_along_lane = FractionalDistanceBetweenPoints(
            prev->X(), prev->Y(), X, Y, next->X(), next->Y());

        return PositionFractionalAtDistanceBetweenPoints(screen_lane_endpoints.X1, screen_lane_endpoints.Y1,
                                                         screen_lane_endpoints.X2, screen_lane_endpoints.Y2,
                                                         dist_along_lane);
    }

    bool InRect(GG::X left, GG::Y top, GG::X right, GG::Y bottom, const GG::Pt pt)
    { return pt.x >= left && pt.y >= top && pt.x < right && pt.y < bottom; } //pt >= ul && pt < lr;

    auto InWndRect(const GG::Wnd* top_wnd) {
        return [top_wnd](const GG::Wnd* wnd, const GG::Pt pt) -> bool {
            return wnd && InRect(wnd->Left(), top_wnd ? top_wnd->Top() : wnd->Top(),
                                 wnd->Right(), wnd->Bottom(),
                                 pt);
        };
    }

    GG::X AppWidth() noexcept {
        if (const auto* app = GGHumanClientApp::GetApp())
            return app->AppWidth();
        return GG::X0;
    }

    GG::Y AppHeight() noexcept {
        if (GGHumanClientApp* app = GGHumanClientApp::GetApp())
            return app->AppHeight();
        return GG::Y0;
    }

    bool ClientPlayerIsModerator()
    { return GGHumanClientApp::GetApp()->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR; }

    void PlayTurnButtonClickSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.button.turn.press.sound.path"), true); }

    bool ToggleBoolOption(const std::string& option_name) {
        const bool initially_enabled = GetOptionsDB().Get<bool>(option_name);
        GetOptionsDB().Set(option_name, !initially_enabled);
        return !initially_enabled;
    }

    void ShowTurnButtonPopup(std::shared_ptr<GG::Button> turn_btn,
                             std::function<void()> end_turn_action,
                             std::function<void()> revoke_orders_action)
    {
        const auto* app = ClientApp::GetApp();
        if (!app)
            return;
        const auto order_count = app->Orders().size();
        const auto turn = app->CurrentTurn();


        // create popup menu with a commands in it
        const GG::Pt pt = turn_btn ? turn_btn->LowerRight() : GG::Pt{GG::X1, GG::Y1};
        auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

        //popup->AddMenuItem(GG::MenuItem(true)); // separator

        const bool disable_end_turn = !turn_btn || turn_btn->Disabled();
        auto start_label = boost::io::str(FlexibleFormat(UserString("MAP_BTN_TURN_SEND_AND_START")) % turn % order_count);
        popup->AddMenuItem(GG::MenuItem(std::move(start_label), disable_end_turn, false, end_turn_action));

        const bool disable_revoke = disable_end_turn || order_count < 1;
        auto revoke_label = boost::io::str(FlexibleFormat(UserString("MAP_BTN_TURN_REVOKE")) % turn % order_count);
        popup->AddMenuItem(GG::MenuItem(std::move(revoke_label), disable_revoke, false, revoke_orders_action));

        popup->Run();
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
            m_margin(LAYOUT_MARGIN)
        {
            GG::X value_col_width{(m_margin * 3) + (ClientUI::Pts() * 3)};
            m_col_widths = {width - value_col_width, value_col_width};

            RequirePreRender();
        }

        void PreRender() override {
            SetChildClippingMode(ChildClippingMode::ClipToClient);

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
            static constexpr GG::Y offset{32};
            const GG::Clr BG_CLR = ClientUI::WndColor();
            const GG::Clr BORDER_CLR = ClientUI::WndOuterBorderColor();
            const GG::Pt UL = GG::Pt(UpperLeft().x, UpperLeft().y + offset);
            const GG::Pt LR = LowerRight();

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
            static constexpr GG::Y offset{32};
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
                      m_ship_design_labels.empty() ? GG::Pt0 : space_row);

            for (auto it = m_ship_design_labels.begin(); it != m_ship_design_labels.end(); ++it) {
                LayoutRow(*it, descr_ul, descr_lr, value_ul, value_lr,
                          std::next(it) == m_ship_design_labels.end()? GG::Pt0 : next_row);
            }

            Resize(GG::Pt(value_lr.x + (m_margin * 3), value_lr.y + (m_margin * 3)));
        }

        /** Constructs and attaches new description and value labels
         *  for the given description row @p descr. */
        void NewLabelValue(const std::string& descr, bool title = false) {
            if (m_labels.contains(descr))
                return;

            GG::Y height{ClientUI::Pts()};
            // center format for title label
            m_labels.emplace(descr, std::pair(
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

            const ScriptingContext& context = IApp::GetApp()->GetContext();
            const Universe& universe = context.ContextUniverse();
            const ObjectMap& objects = context.ContextObjects();
            const SpeciesManager& sm = context.species;

            const auto& destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(m_empire_id);
            for (auto* ship : objects.allRaw<Ship>()) {
                if (!ship->OwnedBy(m_empire_id) || destroyed_objects.contains(ship->ID()))
                    continue;
                m_values[FLEET_DETAIL_SHIP_COUNT]++;

                if (ship->IsArmed(context))
                    m_values[FLEET_DETAIL_ARMED_COUNT]++;
                else
                    m_values[FLEET_DETAIL_UNARMED_COUNT]++;

                if (ship->CanColonize(universe, sm))
                    m_values[FLEET_DETAIL_COLONY_COUNT]++;

                if (ship->HasTroops(universe))
                    m_values[FLEET_DETAIL_TROOP_COUNT]++;

                if (ship->HasFighters(universe))
                    m_values[FLEET_DETAIL_CARRIER_COUNT]++;

                const ShipDesign* design = universe.GetShipDesign(ship->DesignID());
                if (!design)
                    continue;
                m_ship_design_counts[design->ID()]++;
                for (const std::string& part : design->Parts()) {
                    m_values[FLEET_DETAIL_SLOT_COUNT]++;
                    if (!part.empty())
                        m_values[FLEET_DETAIL_PART_COUNT]++;
                }
            }

        }

        /** Resize/Move controls for row @p descr
         *  and then advance sizes by @p row_advance */
        void LayoutRow(const std::string& descr,
                       GG::Pt& descr_ul, GG::Pt& descr_lr,
                       GG::Pt& value_ul, GG::Pt& value_lr,
                       const GG::Pt row_advance)
        {
            if (!m_labels.contains(descr)) {
                ErrorLogger() << "Unable to find expected label key " << descr;
                return;
            }

            LayoutRow(m_labels.at(descr), descr_ul, descr_lr, value_ul, value_lr, row_advance);
        }

        void LayoutRow(LabelValueType& row, GG::Pt& descr_ul, GG::Pt& descr_lr,
                       GG::Pt& value_ul, GG::Pt& value_lr, const GG::Pt row_advance)
        {
            row.first->SizeMove(descr_ul, descr_lr);
            if (row.second)
                row.second->SizeMove(value_ul, value_lr);
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
                    GG::Wnd::Create<CUILabel>(
                        GetUniverse().GetShipDesign(entry.first)->Name(),
                        GG::FORMAT_RIGHT,
                        GG::NO_WND_FLAGS, GG::X0, GG::Y0,
                        m_col_widths.at(0) - (m_margin * 2), height),
                    GG::Wnd::Create<CUILabel>(
                        std::to_string(entry.second),
                        GG::FORMAT_RIGHT,
                        GG::NO_WND_FLAGS, GG::X0, GG::Y0,
                        m_col_widths.at(1) - (m_margin * 2), height)
                );
            }
            std::sort(m_ship_design_labels.begin(), m_ship_design_labels.end(),
                [](LabelValueType a, LabelValueType b)
                { return a.first->Text() < b.first->Text(); }
            );
            for (auto& labels : m_ship_design_labels) {
                AttachChild(labels.first);
                AttachChild(labels.second);
            }
        }

    private:
        void UpdateImpl(std::size_t mode, const Wnd* target) override {
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
        GG::Control(x, y, w, h, GG::ONTOP)
    {
        m_label = GG::Wnd::Create<GG::TextControl>(GG::X0, GG::Y0, GG::X1, GG::Y1, "",
                                                   ClientUI::GetFont(), ClientUI::TextColor());
    }

    void CompleteConstruction() override {
        GG::Control::CompleteConstruction();
        AttachChild(m_label);
        std::set<int> dummy = std::set<int>();
        Update(1.0, dummy, INVALID_OBJECT_ID, GGHumanClientApp::GetApp()->GetContext().ContextObjects());
        m_legend_show_connection = GetOptionsDB().OptionChangedSignal("ui.map.scale.legend.shown").connect(
            [this]() { UpdateEnabled(); });
        UpdateEnabled();
    }

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

    GG::X GetLength() const noexcept { return m_line_length; }

    double GetScaleFactor() const noexcept { return m_scale_factor; }

    void Update(double zoom_factor, std::set<int>& fleet_ids, int sel_system_id, const ObjectMap& objects) {
        // The uu length of the map scale line is generally adjusted in this routine up or down by factors of two or five as
        // the zoom_factor changes, so that it's pixel length on the screen is kept to a reasonable distance.  We also add
        // additional stopping points for the map scale to augment the usefulness of the linked map scale circle (whose
        // radius is the same as the map scale length).  These additional stopping points include the speeds and detection
        // ranges of any selected fleets, and the detection ranges of all planets in the currently selected system,
        // provided such values are at least 20 uu.

        // get selected fleet speeds and detection ranges
        std::set<double> fixed_distances;
        for (const auto* fleet : objects.findRaw<Fleet>(fleet_ids)) {
            if (!fleet)
                continue;
            if (fleet->Speed(objects) > 20)
                fixed_distances.insert(fleet->Speed(objects));
            for (const auto* ship :objects.findRaw<Ship>(fleet->ShipIDs())) {
                if (!ship)
                    continue;
                const float ship_range = ship->GetMeter(MeterType::METER_DETECTION)->Initial();
                if (ship_range > 20)
                    fixed_distances.insert(ship_range);
                const float ship_speed = ship->Speed();
                if (ship_speed > 20)
                    fixed_distances.insert(ship_speed);
            }
        }
        // get detection ranges for planets in the selected system (if any)
        if (const auto* system = objects.getRaw<System>(sel_system_id)) {
            for (const auto* planet : objects.findRaw<Planet>(system->PlanetIDs())) {
                if (!planet)
                    continue;
                const float planet_range = planet->GetMeter(MeterType::METER_DETECTION)->Initial();
                if (planet_range > 20)
                    fixed_distances.insert(planet_range);
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
            auto distance_ub = fixed_distances.upper_bound(max_shown_length/ZOOM_STEP_SIZE);
            if (distance_ub != fixed_distances.end() && *distance_ub <= max_shown_length) {
                TraceLogger()  << " MapScaleLine::Update distance_ub: " << *distance_ub;
                shown_length = *distance_ub;
            } else {
                distance_ub = fixed_distances.upper_bound(shown_length);
                if (distance_ub != fixed_distances.end() && *distance_ub <= max_shown_length) {
                    TraceLogger()  << " MapScaleLine::Update distance_ub: " << *distance_ub;
                    shown_length = *distance_ub;
                }
            }
        }

        // determine end of drawn scale line
        m_line_length = GG::X(static_cast<int>(shown_length * m_scale_factor));

        // update text
        auto label_text = boost::io::str(FlexibleFormat(UserString("MAP_SCALE_INDICATOR")) %
                                         std::to_string(static_cast<int>(shown_length)));
        m_label->SetText(std::move(label_text));
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

    double                              m_scale_factor = 1.0;
    GG::X                               m_line_length = GG::X1;
    std::shared_ptr<GG::TextControl>    m_label;
    boost::signals2::scoped_connection  m_legend_show_connection;
    bool                                m_enabled = false;
};

////////////////////////////////////////////////////////////
// MapWndPopup
////////////////////////////////////////////////////////////
MapWndPopup::MapWndPopup(std::string t, GG::X default_x, GG::Y default_y, GG::X default_w, GG::Y default_h,
                         GG::Flags<GG::WndFlag> flags, std::string_view config_name) :
    CUIWnd(std::move(t), default_x, default_y, default_w, default_h, flags, config_name)
{}

MapWndPopup::MapWndPopup(std::string t, GG::Flags<GG::WndFlag> flags, std::string_view config_name) :
    CUIWnd(std::move(t), flags, config_name)
{}

void MapWndPopup::CompleteConstruction() {
    CUIWnd::CompleteConstruction();

    // MapWndPopupWnd is registered as a top level window, the same as ClientUI and MapWnd.
    // Consequently, when the GUI shutsdown either could be destroyed before this Wnd
    if (auto client = ClientUI::GetClientUI())
        if (auto mapwnd = client->GetMapWnd(false))
            mapwnd->RegisterPopup(std::static_pointer_cast<MapWndPopup>(shared_from_this()));
}

MapWndPopup::~MapWndPopup() {
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
        if (auto mapwnd = client->GetMapWnd(false))
            mapwnd->RemovePopup(this);
}

void MapWndPopup::CloseClicked()
{ CUIWnd::CloseClicked(); }

void MapWndPopup::Close()
{ CloseClicked(); }


////////////////////////////////////////////////
// MapWnd::MovementLineData
////////////////////////////////////////////////
MapWnd::MovementLineData::MovementLineData(const std::vector<MovePathNode>& path_,
                                           const std::map<std::pair<int, int>, LaneEndpoints>& lane_end_points_map,
                                           GG::Clr colour_, int empireID) :
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
    const   MovePathNode& first_node =  path.front();
    double  prev_node_x =               first_node.x;
    double  prev_node_y =               first_node.y;
    int     prev_sys_id =               first_node.object_id;
    int     next_sys_id =               INVALID_OBJECT_ID;
    auto    prev_eta =                  first_node.eta;

    const ScriptingContext& context = IApp::GetApp()->GetContext();
    const auto empire = context.GetEmpire(empireID);
    std::set<int> unobstructed;
    bool s_flag = false;
    bool calc_s_flag = false;
    if (empire) {
        unobstructed = empire->SupplyUnobstructedSystems();
        calc_s_flag = true;
        //s_flag = ((first_node.object_id != INVALID_OBJECT_ID) && !unobstructed.contains(first_node.object_id));
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
        auto start_xy = ScreenPosOnStarlane(prev_node_x, prev_node_y, prev_sys_id, next_sys_id, lane_endpoints, context);
        auto end_xy =   ScreenPosOnStarlane(node.x,      node.y,      prev_sys_id, next_sys_id, lane_endpoints, context);

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
            ((node.object_id != INVALID_OBJECT_ID) && !unobstructed.contains(node.object_id)));
        vertices.emplace_back(start_xy->first, start_xy->second, prev_eta, false,         b_flag, s_flag);
        vertices.emplace_back(end_xy->first,   end_xy->second,   node.eta, node.turn_end, b_flag, s_flag);


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
            GG::INTERACTIVE | GG::DRAGABLE)
{}

void MapWnd::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    SetName("MapWnd");

    using boost::placeholders::_1;
    using boost::placeholders::_2;

    const ScriptingContext& context = IApp::GetApp()->GetContext();

    m_obj_delete_connection =  context.ContextUniverse().UniverseObjectDeleteSignal.connect(
        [this](std::shared_ptr<const UniverseObject> obj) { UniverseObjectDeleted(obj); });

    // toolbar
    m_toolbar = GG::Wnd::Create<CUIToolBar>();
    m_toolbar->SetName("MapWnd Toolbar");
    GG::GUI::GetGUI()->Register(m_toolbar);
    m_toolbar->Hide();


    //////////////////////////////
    // Toolbar buttons and icons
    //////////////////////////////

    // turn button
    // determine size from the text that will go into the button, using a test year string
    std::string unready_longest_reasonable = boost::io::str(FlexibleFormat(UserString("MAP_BTN_TURN_UNREADY")) % "99999");
    m_btn_turn = Wnd::Create<CUIButton>(unready_longest_reasonable);
    m_btn_turn->Resize(m_btn_turn->MinUsableSize());
    m_btn_turn->LeftClickedSignal.connect([this]() { EndTurn(); });
    m_btn_turn->LeftClickedSignal.connect(&PlayTurnButtonClickSound);
    m_btn_turn->RightClickedSignal.connect(
        [this]() { ShowTurnButtonPopup(m_btn_turn, [this]() { EndTurn(); }, [this]() { RevertOrders(); }); });

    RefreshTurnButtonTooltip();

    boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

    // auto turn button
    m_btn_auto_turn = Wnd::Create<CUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "manual_turn.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "auto_turn.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "manual_turn_mouseover.png")));

    m_btn_auto_turn->LeftClickedSignal.connect([this]() { ToggleAutoEndTurn(); });
    m_btn_auto_turn->Resize(ICON_SIZE);
    m_btn_auto_turn->SetMinSize(ICON_SIZE);
    ToggleAutoEndTurn();    // toggle twice to set textures without changing default setting state
    ToggleAutoEndTurn();

    // timeout remain label
    // determine size from the text that will go into label, using a test time string
    std::string timeout_longest_reasonable =
        boost::io::str(FlexibleFormat(UserString("MAP_TIMEOUT_HRS_MINS")) % 999 % 59); // minutes part never exceeds 59, turn interval hopefully doesn't exceed month
    m_timeout_remain = Wnd::Create<CUILabel>(std::move(timeout_longest_reasonable));
    m_timeout_remain->Resize(m_timeout_remain->MinUsableSize());

    // FPS indicator
    m_FPS = GG::Wnd::Create<FPSIndicator>();
    m_FPS->Hide();


    // Menu button
    m_btn_menu = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "menu.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "menu_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "menu_mouseover.png")),
        InWndRect(m_toolbar.get()));
    m_btn_menu->SetMinSize(MENU_ICON_SIZE);
    m_btn_menu->LeftClickedSignal.connect([this]() { ShowMenu(); });
    m_btn_menu->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_menu->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_MENU"), UserString("MAP_BTN_MENU_DESC")));

    // Encyclo"pedia" button
    m_btn_pedia = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "pedia.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "pedia_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "pedia_mouseover.png")),
        InWndRect(m_toolbar.get()));
    m_btn_pedia->SetMinSize(MENU_ICON_SIZE);
    m_btn_pedia->LeftClickedSignal.connect([this]() { TogglePedia(); });
    m_btn_pedia->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_pedia->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_PEDIA"), UserString("MAP_BTN_PEDIA_DESC")));

    // Graphs button
    m_btn_graphs = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "charts.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "charts_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "charts_mouseover.png")),
        InWndRect(m_toolbar.get()));
    m_btn_graphs->SetMinSize(MENU_ICON_SIZE);
    m_btn_graphs->LeftClickedSignal.connect([this]() { ShowGraphs(); });
    m_btn_graphs->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_graphs->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_GRAPH"), UserString("MAP_BTN_GRAPH_DESC")));

    // Design button
    m_btn_design = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "design.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "design_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "design_mouseover.png")),
        InWndRect(m_toolbar.get()));
    m_btn_design->SetMinSize(MENU_ICON_SIZE);
    m_btn_design->LeftClickedSignal.connect([this]() { ToggleDesign(); });
    m_btn_design->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_design->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_DESIGN"), UserString("MAP_BTN_DESIGN_DESC")));

    // Government button
    m_btn_government = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "government.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "government_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "government_mouseover.png")),
        InWndRect(m_toolbar.get()));
    m_btn_government->SetMinSize(MENU_ICON_SIZE);
    m_btn_government->LeftClickedSignal.connect([this]() { ToggleGovernment(); });
    m_btn_government->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_government->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_GOVERNMENT"), UserString("MAP_BTN_GOVERNMENT_DESC")));

    // Production button
    m_btn_production = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "production.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "production_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "production_mouseover.png")),
        InWndRect(m_toolbar.get()));
    m_btn_production->SetMinSize(MENU_ICON_SIZE);
    m_btn_production->LeftClickedSignal.connect([this]() { ToggleProduction(); });
    m_btn_production->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_production->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_PRODUCTION"), UserString("MAP_BTN_PRODUCTION_DESC")));

    // Research button
    m_btn_research = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "research.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "research_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "research_mouseover.png")),
        InWndRect(m_toolbar.get()));
    m_btn_research->SetMinSize(MENU_ICON_SIZE);
    m_btn_research->LeftClickedSignal.connect(
        [this]() { ToggleResearch(IApp::GetApp()->GetContext()); });
    m_btn_research->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_research->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_RESEARCH"), UserString("MAP_BTN_RESEARCH_DESC")));

    // Objects button
    m_btn_objects = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "objects.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "objects_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "objects_mouseover.png")),
        InWndRect(m_toolbar.get()));
    m_btn_objects->SetMinSize(MENU_ICON_SIZE);
    m_btn_objects->LeftClickedSignal.connect([this]() { ToggleObjects(); });
    m_btn_objects->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_objects->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_OBJECTS"), UserString("MAP_BTN_OBJECTS_DESC")));

    // Empires button
    m_btn_empires = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "empires.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "empires_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "empires_mouseover.png")),
        InWndRect(m_toolbar.get()));
    m_btn_empires->SetMinSize(MENU_ICON_SIZE);
    m_btn_empires->LeftClickedSignal.connect([this]() { ToggleEmpires(); });
    m_btn_empires->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_empires->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_EMPIRES"), UserString("MAP_BTN_EMPIRES_DESC")));

    // SitRep button
    m_btn_siterep = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "sitrep.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "sitrep_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "sitrep_mouseover.png")),
        InWndRect(m_toolbar.get()));
    m_btn_siterep->SetMinSize(MENU_ICON_SIZE);
    m_btn_siterep->LeftClickedSignal.connect([this]() { ToggleSitRep(); });
    m_btn_siterep->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_siterep->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_SITREP"), UserString("MAP_BTN_SITREP_DESC")));

    // Messages button
    m_btn_messages = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "messages.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "messages_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "messages_mouseover.png")),
        InWndRect(m_toolbar.get()));
    m_btn_messages->SetMinSize(MENU_ICON_SIZE);
    m_btn_messages->LeftClickedSignal.connect([this]() { ToggleMessages(); });
    m_btn_messages->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_messages->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_MESSAGES"), UserString("MAP_BTN_MESSAGES_DESC")));

    // Moderator button
    m_btn_moderator = Wnd::Create<SettableInWindowCUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "moderator.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "moderator_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "moderator_mouseover.png")),
        InWndRect(m_toolbar.get()));
    m_btn_moderator->SetMinSize(MENU_ICON_SIZE);
    m_btn_moderator->LeftClickedSignal.connect([this]() { ToggleModeratorActions(); });
    m_btn_moderator->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_moderator->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_MODERATOR"), UserString("MAP_BTN_MODERATOR_DESC")));


    // resources
    m_population = GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(MeterType::METER_POPULATION),
                                                  0, 3, false, ICON_SINGLE_WIDTH, m_btn_turn->Height());
    m_population->SetName("Population StatisticIcon");

    m_industry = GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(MeterType::METER_INDUSTRY),
                                                0, 3, false, ICON_SINGLE_WIDTH, m_btn_turn->Height());
    m_industry->SetName("Industry StatisticIcon");
    m_industry->LeftClickedSignal.connect([this](auto) { ToggleProduction(); });

    m_stockpile = GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(MeterType::METER_STOCKPILE),
                                                 0, 3, false, ICON_DUAL_WIDTH, m_btn_turn->Height());
    m_stockpile->SetName("Stockpile StatisticIcon");

    m_research = GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(MeterType::METER_RESEARCH),
                                                0, 3, false, ICON_SINGLE_WIDTH, m_btn_turn->Height());
    m_research->SetName("Research StatisticIcon");
    m_research->LeftClickedSignal.connect([this](auto) { ToggleResearch(IApp::GetApp()->GetContext()); });

    m_influence = GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(MeterType::METER_INFLUENCE),
                                                 0, 3, false, ICON_DUAL_WIDTH, m_btn_turn->Height());
    m_influence->SetName("Influence StatisticIcon");
    m_influence->LeftClickedSignal.connect([this](auto) { ToggleGovernment(); });

    m_fleet = GG::Wnd::Create<StatisticIcon>(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "sitrep" / "fleet_arrived.png"),
                                             0, 3, false, ICON_SINGLE_WIDTH, m_btn_turn->Height());
    m_fleet->SetName("Fleet StatisticIcon");

    m_detection = GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(MeterType::METER_DETECTION),
                                                 0, 3, false, ICON_SINGLE_WIDTH, m_btn_turn->Height());
    m_detection->SetName("Detection StatisticIcon");

    GG::SubTexture wasted_ressource_subtexture = GG::SubTexture(
        ClientUI::GetTexture(button_texture_dir / "wasted_resource.png", false));
    GG::SubTexture wasted_ressource_mouseover_subtexture = GG::SubTexture(
        ClientUI::GetTexture(button_texture_dir / "wasted_resource_mouseover.png", false));
    GG::SubTexture wasted_ressource_clicked_subtexture = GG::SubTexture(
        ClientUI::GetTexture(button_texture_dir / "wasted_resource_clicked.png", false));

    m_industry_wasted = Wnd::Create<CUIButton>(
        wasted_ressource_subtexture,
        wasted_ressource_clicked_subtexture,
        wasted_ressource_mouseover_subtexture);

    m_research_wasted = Wnd::Create<CUIButton>(
        wasted_ressource_subtexture,
        wasted_ressource_clicked_subtexture,
        wasted_ressource_mouseover_subtexture);

    m_industry_wasted->Resize(ICON_SIZE);
    m_industry_wasted->SetMinSize(ICON_SIZE);
    m_research_wasted->Resize(ICON_SIZE);
    m_research_wasted->SetMinSize(ICON_SIZE);

    m_industry_wasted->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_research_wasted->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    m_industry_wasted->LeftClickedSignal.connect([this]() { ZoomToSystemWithWastedPP(); });
    m_research_wasted->LeftClickedSignal.connect([this]() { ToggleResearch(IApp::GetApp()->GetContext()); });

    //Set the correct tooltips
    RefreshIndustryResourceIndicator();
    RefreshResearchResourceIndicator();



    /////////////////////////////////////
    // place buttons / icons on toolbar
    /////////////////////////////////////
    std::vector<GG::X> widths{
        m_btn_turn->Width(),        ICON_WIDTH,                 ICON_WIDTH,
        ICON_WIDTH,                 ICON_WIDTH,                 ICON_SINGLE_WIDTH,
        ICON_DUAL_WIDTH,            ICON_WIDTH,                 ICON_SINGLE_WIDTH,
        ICON_DUAL_WIDTH,            ICON_SINGLE_WIDTH,          ICON_SINGLE_WIDTH,
        ICON_SINGLE_WIDTH,          MENU_ICON_SIZE.x,           MENU_ICON_SIZE.x,
        MENU_ICON_SIZE.x,           MENU_ICON_SIZE.x,           MENU_ICON_SIZE.x,
        MENU_ICON_SIZE.x,           MENU_ICON_SIZE.x,           MENU_ICON_SIZE.x,
        MENU_ICON_SIZE.x,           MENU_ICON_SIZE.x,           MENU_ICON_SIZE.x,
        MENU_ICON_SIZE.x};

    std::vector<float> stretches{
        0.0f,                       0.0f,                       0.0f,
        0.0f,                       0.0f,                       1.0f,
        2.0f,                       0.0f,                       1.0f,
        2.0f,                       1.0f,                       1.0f,
        1.0f,                       0.0f,                       0.0f,
        0.0f,                       0.0f,                       0.0f,
        0.0f,                       0.0f,                       0.0f,
        0.0f,                       0.0f,                       0.0f,
        0.0f};

    auto layout = GG::Wnd::Create<GG::Layout>(m_toolbar->ClientUpperLeft().x, m_toolbar->ClientUpperLeft().y,
                                              m_toolbar->ClientWidth(),       m_toolbar->ClientHeight(),
                                              1, widths.size());
    layout->SetName("Toolbar Layout");
    layout->SetCellMargin(5);
    layout->SetBorderMargin(5);
    layout->SetMinimumRowHeight(0, ICON_SIZE.y);
    //layout->RenderOutline(true);
    layout->SetColumnStretches(stretches);
    layout->SetMinimumColumnWidths(widths);

    int layout_column{0};

    layout->Add(m_btn_turn,         0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);  // 0
    layout->Add(m_btn_auto_turn,    0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    layout->Add(m_timeout_remain,   0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    layout->Add(m_FPS,              0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    layout->Add(m_industry_wasted,  0, layout_column++, GG::ALIGN_RIGHT  | GG::ALIGN_VCENTER);
    layout->Add(m_industry,         0, layout_column++, GG::ALIGN_LEFT   | GG::ALIGN_VCENTER);  // 5
    layout->Add(m_stockpile,        0, layout_column++, GG::ALIGN_LEFT   | GG::ALIGN_VCENTER);
    layout->Add(m_research_wasted,  0, layout_column++, GG::ALIGN_RIGHT  | GG::ALIGN_VCENTER);
    layout->Add(m_research,         0, layout_column++, GG::ALIGN_LEFT   | GG::ALIGN_VCENTER);
    layout->Add(m_influence,        0, layout_column++, GG::ALIGN_LEFT   | GG::ALIGN_VCENTER);
    layout->Add(m_fleet,            0, layout_column++, GG::ALIGN_LEFT   | GG::ALIGN_VCENTER);  // 10
    layout->Add(m_population,       0, layout_column++, GG::ALIGN_LEFT   | GG::ALIGN_VCENTER);
    layout->Add(m_detection,        0, layout_column++, GG::ALIGN_LEFT   | GG::ALIGN_VCENTER);
    layout->Add(m_btn_moderator,    0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    layout->Add(m_btn_messages,     0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    layout->Add(m_btn_siterep,      0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);  // 15
    layout->Add(m_btn_empires,      0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    layout->Add(m_btn_objects,      0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    layout->Add(m_btn_research,     0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    layout->Add(m_btn_production,   0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    layout->Add(m_btn_design,       0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);  // 20
    layout->Add(m_btn_government,   0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    layout->Add(m_btn_graphs,       0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    layout->Add(m_btn_pedia,        0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
    layout->Add(m_btn_menu,         0, layout_column++, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);

    m_toolbar->SetLayout(layout);

    ///////////////////
    // Misc widgets on map screen
    ///////////////////
    // scale line
    m_scale_line = GG::Wnd::Create<MapScaleLine>(GG::X(LAYOUT_MARGIN),   GG::Y(LAYOUT_MARGIN) + m_toolbar->Height(),
                                                 SCALE_LINE_MAX_WIDTH,   SCALE_LINE_HEIGHT);
    GG::GUI::GetGUI()->Register(m_scale_line);
    int sel_system_id = SidePanel::SystemID();
    m_scale_line->Update(ZoomFactor(), m_selected_fleet_ids, sel_system_id, context.ContextObjects());
    m_scale_line->Hide();

    // Zoom slider
    static constexpr int ZOOM_SLIDER_MIN = static_cast<int>(ZOOM_IN_MIN_STEPS),
                         ZOOM_SLIDER_MAX = static_cast<int>(ZOOM_IN_MAX_STEPS);
    m_zoom_slider = GG::Wnd::Create<CUISlider<double>>(ZOOM_SLIDER_MIN, ZOOM_SLIDER_MAX,
                                                       GG::Orientation::VERTICAL,
                                                       GG::INTERACTIVE | GG::ONTOP);
    m_zoom_slider->MoveTo(GG::Pt(m_btn_turn->Left(), m_scale_line->Bottom() + GG::Y(LAYOUT_MARGIN)));
    m_zoom_slider->Resize(GG::Pt(GG::X(ClientUI::ScrollWidth()), ZOOM_SLIDER_HEIGHT));
    m_zoom_slider->SlideTo(m_zoom_steps_in);
    GG::GUI::GetGUI()->Register(m_zoom_slider);
    m_zoom_slider->Hide();
    m_zoom_slider->SlidSignal.connect([this](double pos, double, double) { SetZoom(pos, false); });
    m_slider_show_connection = GetOptionsDB().OptionChangedSignal("ui.map.zoom.slider.shown").connect(
        [this]() { RefreshSliders(); });

    ///////////////////
    // Map sub-windows
    ///////////////////

    // system-view side panel
    m_side_panel = GG::Wnd::Create<SidePanel>(MAP_SIDEPANEL_WND_NAME);
    GG::GUI::GetGUI()->Register(m_side_panel);

    SidePanel::SystemSelectedSignal.connect(        boost::bind(&MapWnd::SelectSystem, this, _1));
    SidePanel::PlanetSelectedSignal.connect(        boost::bind(&MapWnd::SelectPlanet, this, _1));
    SidePanel::PlanetDoubleClickedSignal.connect(   boost::bind(&MapWnd::PlanetDoubleClicked, this, _1));
    SidePanel::PlanetRightClickedSignal.connect(    boost::bind(&MapWnd::PlanetRightClicked, this, _1));
    SidePanel::BuildingRightClickedSignal.connect(  boost::bind(&MapWnd::BuildingRightClicked, this, _1));

    // not strictly necessary, as in principle whenever any Planet
    // changes, all meter estimates and resource pools should / could be
    // updated.  however, this is a convenience to limit the updates to
    // what is actually being shown in the sidepanel right now, which is
    // useful since most Planet changes will be due to focus
    // changes on the sidepanel, and most differences in meter estimates
    // and resource pools due to this will be in the same system
    SidePanel::ResourceCenterChangedSignal.connect([this](){
        if (GetOptionsDB().Get<bool>("ui.map.object-changed.meter-refresh")) {
            auto* app = GGHumanClientApp::GetApp();
            auto& context = app->GetContext();
            context.ContextUniverse().UpdateMeterEstimates(context);
            UpdateEmpireResourcePools(context, app->EmpireID());
        }
    });

    // situation report window
    m_sitrep_panel = GG::Wnd::Create<SitRepPanel>(SITREP_WND_NAME);
    // Wnd is manually closed by user
    m_sitrep_panel->ClosingSignal.connect(boost::bind(&MapWnd::HideSitRep, this));
    if (m_sitrep_panel->Visible()) {
        PushWndStack(m_sitrep_panel);
        m_btn_siterep->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "sitrep_mouseover.png")));
        m_btn_siterep->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "sitrep.png")));
    }

    // encyclopedia panel
    m_pedia_panel = GG::Wnd::Create<EncyclopediaDetailPanel>(GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE | PINABLE, MAP_PEDIA_WND_NAME);
    // Wnd is manually closed by user
    m_pedia_panel->ClosingSignal.connect(boost::bind(&MapWnd::HidePedia, this));
    if (m_pedia_panel->Visible()) {
        PushWndStack(m_pedia_panel);
        m_btn_pedia->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "pedia_mouseover.png")));
        m_btn_pedia->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "pedia.png")));
    }

    // objects list
    m_object_list_wnd = GG::Wnd::Create<ObjectListWnd>(OBJECT_WND_NAME);
    // Wnd is manually closed by user
    m_object_list_wnd->ClosingSignal.connect(boost::bind(&MapWnd::HideObjects, this));
    m_object_list_wnd->ObjectDumpSignal.connect(boost::bind(&ClientUI::DumpObject, ClientUI::GetClientUI(), _1));
    if (m_object_list_wnd->Visible()) {
        PushWndStack(m_object_list_wnd);
        m_btn_objects->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "objects_mouseover.png")));
        m_btn_objects->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "objects.png")));
    }

    // moderator actions
    m_moderator_wnd = GG::Wnd::Create<ModeratorActionsWnd>(MODERATOR_WND_NAME);
    m_moderator_wnd->ClosingSignal.connect(boost::bind(&MapWnd::HideModeratorActions, this));
    if (m_moderator_wnd->Visible()) {
        PushWndStack(m_moderator_wnd);
        m_btn_moderator->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "moderator_mouseover.png")));
        m_btn_moderator->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "moderator.png")));
    }

    // Combat report
    m_combat_report_wnd = GG::Wnd::Create<CombatReportWnd>(COMBAT_REPORT_WND_NAME);

    // government window
    m_government_wnd = GG::Wnd::Create<GovernmentWnd>(GOVERNMENT_WND_NAME);
    // Wnd is manually closed by user
    m_government_wnd->ClosingSignal.connect(
        boost::bind(&MapWnd::HideGovernment, this));
    if (m_government_wnd->Visible()) {
        PushWndStack(m_government_wnd);
        m_btn_government->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "government_mouseover.png")));
        m_btn_government->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "government.png")));
    }


    // position CUIWnds owned by the MapWnd
    InitializeWindows();

    // messages and empires windows
    if (ClientUI* cui = ClientUI::GetClientUI()) {
        if (const auto& msg_wnd = cui->GetMessageWnd()) {
            // Wnd is manually closed by user
            msg_wnd->ClosingSignal.connect(boost::bind(&MapWnd::HideMessages, this));
            if (msg_wnd->Visible()) {
                PushWndStack(msg_wnd);
                m_btn_messages->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "messages_mouseover.png")));
                m_btn_messages->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "messages.png")));
            }
        }
        if (const auto& plr_wnd = cui->GetPlayerListWnd()) {
            // Wnd is manually closed by user
            plr_wnd->ClosingSignal.connect(boost::bind(&MapWnd::HideEmpires, this));
            if (plr_wnd->Visible()) {
                PushWndStack(plr_wnd);
                m_btn_empires->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "empires_mouseover.png")));
                m_btn_empires->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "empires.png")));
            }
        }
    }

    GGHumanClientApp::GetApp()->RepositionWindowsSignal.connect(
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
    m_production_wnd->SystemSelectedSignal.connect(boost::bind(&MapWnd::SelectSystem, this, _1));
    m_production_wnd->PlanetSelectedSignal.connect(boost::bind(&MapWnd::SelectPlanet, this, _1));

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

    m_timeout_clock.Stop();
    m_timeout_clock.Connect(this);
}

void MapWnd::DoLayout() {
    m_toolbar->Resize(GG::Pt(AppWidth(), TOOLBAR_HEIGHT));
    m_research_wnd->Resize(GG::Pt(AppWidth(), AppHeight() - m_toolbar->Height()));
    m_production_wnd->Resize(GG::Pt(AppWidth(), AppHeight() - m_toolbar->Height()));
    m_design_wnd->Resize(GG::Pt(AppWidth(), AppHeight() - m_toolbar->Height()));
    m_government_wnd->ValidatePosition();
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
    const GG::X SIDEPANEL_WIDTH(GetOptionsDB().Get<GG::X>("ui.map.sidepanel.width"));

    // system-view side panel
    const GG::Pt sidepanel_ul(AppWidth() - SIDEPANEL_WIDTH, m_toolbar->Bottom());
    const GG::Pt sidepanel_wh(SIDEPANEL_WIDTH, AppHeight() - m_toolbar->Height());

    // situation report window
    const GG::Pt sitrep_ul(SCALE_LINE_MAX_WIDTH + LAYOUT_MARGIN, m_toolbar->Bottom());
    static constexpr GG::Pt sitrep_wh(SITREP_PANEL_WIDTH, SITREP_PANEL_HEIGHT);

    // encyclopedia panel
    const GG::Pt pedia_ul(SCALE_LINE_MAX_WIDTH + LAYOUT_MARGIN, m_toolbar->Bottom() + SITREP_PANEL_HEIGHT);
    static constexpr GG::Pt pedia_wh(SITREP_PANEL_WIDTH*3/2, SITREP_PANEL_HEIGHT*3);

    // objects list
    const GG::Pt object_list_ul(SCALE_LINE_MAX_WIDTH/2, m_scale_line->Bottom() + GG::Y(LAYOUT_MARGIN));
    static constexpr GG::Pt object_list_wh(SITREP_PANEL_WIDTH*2, SITREP_PANEL_HEIGHT*3);

    // moderator actions
    const GG::Pt moderator_ul(GG::X0, m_scale_line->Bottom() + GG::Y(LAYOUT_MARGIN));
    static constexpr GG::Pt moderator_wh(SITREP_PANEL_WIDTH, SITREP_PANEL_HEIGHT);

    // Combat report
    static constexpr GG::Pt combat_log_ul(GG::X(150), GG::Y(50));
    static constexpr GG::Pt combat_log_wh(GG::X(400), GG::Y(300));

    // government window
    const GG::Pt gov_ul(GG::X0, m_scale_line->Bottom() + m_scale_line->Height() + GG::Y(LAYOUT_MARGIN*2));
    static constexpr GG::Pt gov_wh(SITREP_PANEL_WIDTH*2, SITREP_PANEL_HEIGHT*3);

    m_side_panel->       InitSizeMove(sidepanel_ul,     sidepanel_ul + sidepanel_wh);
    m_sitrep_panel->     InitSizeMove(sitrep_ul,        sitrep_ul + sitrep_wh);
    m_pedia_panel->      InitSizeMove(pedia_ul,         pedia_ul + pedia_wh);
    m_object_list_wnd->  InitSizeMove(object_list_ul,   object_list_ul + object_list_wh);
    m_moderator_wnd->    InitSizeMove(moderator_ul,     moderator_ul + moderator_wh);
    m_combat_report_wnd->InitSizeMove(combat_log_ul,    combat_log_ul + combat_log_wh);
    m_government_wnd->   InitSizeMove(gov_ul,           gov_ul + gov_wh);
}

GG::Pt MapWnd::ClientUpperLeft() const noexcept
{ return UpperLeft() + GG::Pt(AppWidth(), AppHeight()); }

double MapWnd::ZoomFactor() const
{ return ZoomScaleFactor(m_zoom_steps_in); }

GG::Pt MapWnd::ScreenCoordsFromUniversePosition(double universe_x, double universe_y) const {
    GG::Pt cl_ul = ClientUpperLeft();
    GG::X x(GG::ToX(universe_x * ZoomFactor()) + cl_ul.x);
    GG::Y y(GG::ToY(universe_y * ZoomFactor()) + cl_ul.y);
    return GG::Pt(x, y);
}

std::pair<double, double> MapWnd::UniversePositionFromScreenCoords(GG::Pt screen_coords) const {
    GG::Pt cl_ul = ClientUpperLeft();
    double x = (screen_coords - cl_ul).x / ZoomFactor();
    double y = (screen_coords - cl_ul).y / ZoomFactor();
    return {x, y};
}

int MapWnd::SelectedSystemID() const
{ return SidePanel::SystemID(); }

int MapWnd::SelectedPlanetID() const // TODO: noexcept ?
{ return m_production_wnd->SelectedPlanetID(); }

int MapWnd::SelectedFleetID() const {
    if (!m_selected_fleet_ids.empty())
        return *m_selected_fleet_ids.begin();
    return INVALID_OBJECT_ID;
}

int MapWnd::SelectedShipID() const {
    if (!m_selected_ship_ids.empty())
        return *m_selected_ship_ids.begin();
    return INVALID_OBJECT_ID;
}

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

            const float brightness = 1.0f - std::pow(RandZeroToOne(), 2);
            m_starfield_colours.store(GG::BlendClr(GG::CLR_WHITE, GG::CLR_ZERO, brightness));
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


    float point_star_scale = GetOptionsDB().Get<double>("ui.map.background.starfields.scale");
    glPointSize(point_star_scale * std::max(1.0, std::sqrt(ZoomFactor())));
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
    for (auto& [field_texture, buffer] : m_field_vertices_not_visible) {
        glBindTexture(GL_TEXTURE_2D, field_texture->OpenGLId());
        buffer.activate();
        m_field_texture_coords.activate();
        glDrawArrays(GL_QUADS, 0, buffer.size());
    }

    // if any, render scanlines over not-visible fields
    if (!m_field_scanline_circles.empty()
        && GGHumanClientApp::GetApp()->EmpireID() != ALL_EMPIRES
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
    for (auto& [field_texture, buffer] : m_field_vertices_visible) {
        glBindTexture(GL_TEXTURE_2D, field_texture->OpenGLId());
        buffer.activate();
        m_field_texture_coords.activate();
        glDrawArrays(GL_QUADS, 0, buffer.size());
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
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    const float HALO_SCALE_FACTOR = static_cast<float>(SystemHaloScaleFactor());
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

    const ScriptingContext& context = IApp::GetApp()->GetContext();
    const Universe& universe = context.ContextUniverse();
    const ObjectMap& objects = context.ContextObjects();
    const EmpireManager& empires = context.Empires();
    const SupplyManager& supply = context.supply;

    int empire_id = GGHumanClientApp::GetApp()->EmpireID();
    if (empire_id != ALL_EMPIRES && GetOptionsDB().Get<bool>("ui.map.scanlines.shown"))
        fog_scanlines = true;

    RenderScaleCircle();

    if (!fog_scanlines && !circles) {
        glPopClientAttrib();
        return;
    }

    auto& known_destroyed_object_ids = universe.EmpireKnownDestroyedObjectIDs(empire_id);


    // distance between inner and outer system circle
    const double circle_distance = GetOptionsDB().Get<double>("ui.map.system.circle.distance");
    // width of outer...
    //const double outer_circle_width = GetOptionsDB().Get<double>("ui.map.system.circle.outer.width");
    // ... and inner circle line at close zoom
    const double inner_circle_width = GetOptionsDB().Get<double>("ui.map.system.circle.inner.width");
    // width of inner circle line when map is zoomed out
    const double max_inner_circle_width = GetOptionsDB().Get<double>("ui.map.system.circle.inner.max.width");
    // thick
    const float line_thick = std::max(std::min(2 / ZoomFactor(), max_inner_circle_width), inner_circle_width);


    std::vector<std::pair<int, int>> colony_count_by_empire_id;
    colony_count_by_empire_id.reserve(static_cast<std::size_t>(empires.NumEmpires()) + 1u);
    auto increment_empire_colony_count = [&colony_count_by_empire_id](int col_empire_id) {
        auto it = std::find_if(colony_count_by_empire_id.begin(), colony_count_by_empire_id.end(),
                               [col_empire_id](const auto& e) { return e.first == col_empire_id; });
        if (it != colony_count_by_empire_id.end())
            it->second++;
        else
            colony_count_by_empire_id.emplace_back(col_empire_id, 1);
    };


    std::vector<std::pair<int, GG::Clr>> empire_colours;
    empire_colours.reserve(colony_count_by_empire_id.size());
    std::transform(empires.begin(), empires.end(), std::back_inserter(empire_colours),
                   [](const auto& e) { return std::pair{e.first, e.second->Color()}; });
    auto get_empire_colour = [&empire_colours,
                              neutral_colour{GetOptionsDB().Get<GG::Clr>("ui.map.starlane.color")}]
                              (int empire_id)
    {
        auto it = std::find_if(empire_colours.begin(), empire_colours.end(),
                               [empire_id](const auto& id_clr) { return empire_id == id_clr.first; });
        if (it != empire_colours.end())
            return it->second;
        return neutral_colour;
    };

    m_scanline_circle_vertices.clear();
    m_system_circle_vertices.clear();
    m_system_circle_colours.clear();
    m_scanline_circle_vertices.reserve(120*m_system_icons.size());
    m_system_circle_vertices.reserve(120*m_system_icons.size());
    m_system_circle_colours.reserve(120*m_system_icons.size());


    for (const auto& [system_id, icon] : m_system_icons) {
        GG::Pt icon_size = icon->LowerRight() - icon->UpperLeft();
        GG::Pt icon_middle = icon->UpperLeft() + (icon_size / 2);
        GG::Pt circle_size = GG::Pt(static_cast<GG::X>(icon->EnclosingCircleDiameter()),
                                    static_cast<GG::Y>(icon->EnclosingCircleDiameter()));
        GG::Pt circle_ul = icon_middle - (circle_size / 2);
        GG::Pt circle_lr = circle_ul + circle_size;


        // prep scanlines
        if (fog_scanlines &&
            empire_id != ALL_EMPIRES &&
            universe.GetObjectVisibilityByEmpire(system_id, empire_id) <= Visibility::VIS_BASIC_VISIBILITY)
        {
            BufferStoreCircleArcVertices(m_scanline_circle_vertices, circle_ul, circle_lr,
                                         0, TWO_PI, true, 24, false);
        }


        if (!circles)
            continue;
        auto* system = objects.getRaw<const System>(system_id);
        if (!system || system->NumStarlanes() < 1)
            continue;


        // prep circles around systems that have at least one starlane, if they are enabled
        const GG::Pt circle_distance_pt = GG::Pt(GG::X1, GG::Y1) * circle_distance;
        const GG::Pt inner_circle_ul = circle_ul + (circle_distance_pt * ZoomFactor());
        const GG::Pt inner_circle_lr = circle_lr - (circle_distance_pt * ZoomFactor());

        bool has_empire_planet = false;
        bool has_neutrals = false;
        colony_count_by_empire_id.clear();

        for (const auto* planet : objects.findRaw<const Planet>(system->PlanetIDs())) {
            if (known_destroyed_object_ids.contains(planet->ID()))
                continue;

            // remember if this system has a player-owned planet, count # of colonies for each empire
            if (!planet->Unowned()) {
                has_empire_planet = true;
                increment_empire_colony_count(planet->Owner());
            }

            // remember if this system has neutrals
            if (planet->Unowned() && !planet->SpeciesName().empty() &&
                planet->GetMeter(MeterType::METER_POPULATION)->Initial() > 0.0f)
            {
                has_neutrals = true;
                increment_empire_colony_count(ALL_EMPIRES);
            }
        }


        // outer circle in color of supplying empire
        const int supply_empire_id = supply.EmpireThatCanSupplyAt(system_id);
        const auto pre_sz1 = m_system_circle_vertices.size();
        BufferStoreCircleArcVertices(m_system_circle_vertices, circle_ul, circle_lr,
                                     0.0, TWO_PI, false, 0, false);
        {
            const std::size_t count1 = m_system_circle_vertices.size() - pre_sz1;
            const auto clr_e = get_empire_colour(supply_empire_id);
            for (std::size_t n = 0; n < count1; ++n)
                m_system_circle_colours.store(clr_e);
        }


        // systems with neutrals and no empire have a segmented inner circle
        if (has_neutrals && !has_empire_planet) {
            static constexpr std::size_t segments = 24;
            static constexpr double segment_arc = TWO_PI / segments;

            const auto pre_sz2 = m_system_circle_vertices.size();
            for (std::size_t n = 0; n < segments; n = n + 2) {
                const auto theta1 = n * segment_arc;
                const auto theta2 = (n+1) * segment_arc;
                BufferStoreCircleArcVertices(m_system_circle_vertices, inner_circle_ul, inner_circle_lr,
                                             theta1, theta2, false, 48, false);
            }
            const std::size_t count2 = m_system_circle_vertices.size() - pre_sz2;
            const auto clr_txt = ClientUI::TextColor();
            for (std::size_t n = 0; n < count2; ++n)
                m_system_circle_colours.store(clr_txt);
        }


        // systems with empire planets have an unbroken inner circle,
        // with different-color segments for each empire present
        if (!has_empire_planet)
            continue;

        const int colonized_planets = std::transform_reduce(
            colony_count_by_empire_id.begin(), colony_count_by_empire_id.end(),
            0, std::plus<>(), [](const auto& e) { return e.second; });
        const std::size_t segments = std::max(colonized_planets, 1);
        const double segment_arc = TWO_PI / segments;


        std::size_t n = 0;
        for (const auto& [colony_empire_id, colony_count] : colony_count_by_empire_id) {
            const auto pre_sz3 = m_system_circle_vertices.size();
            const auto theta1 = n*segment_arc;
            const auto theta2 = (n + colony_count)*segment_arc;
            BufferStoreCircleArcVertices(m_system_circle_vertices, inner_circle_ul, inner_circle_lr,
                                         theta1, theta2, false, 30, false);
            const std::size_t count3 = m_system_circle_vertices.size() - pre_sz3;
            n += colony_count;
            const auto clr_e2 = (colony_empire_id == ALL_EMPIRES) ?
                ClientUI::TextColor() : get_empire_colour(colony_empire_id);
            for (std::size_t n2 = 0; n2 < count3; ++n2)
                m_system_circle_colours.store(clr_e2);
        }
    }


    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    //glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS); // already pushed above
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);


    // render scanlines
    m_scanline_shader.SetColor(GetOptionsDB().Get<GG::Clr>("ui.map.system.scanlines.color"));
    m_scanline_shader.StartUsing();
    m_scanline_circle_vertices.activate();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_scanline_circle_vertices.size()));
    m_scanline_shader.StopUsing();


    // render system circles
    glEnableClientState(GL_COLOR_ARRAY);
    glLineWidth(line_thick);
    m_system_circle_vertices.activate();
    m_system_circle_colours.activate();
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_system_circle_vertices.size()));


    glPopClientAttrib();
    glDisable(GL_LINE_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    glLineWidth(1.0f);
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
                             double thickness, bool coloured, bool do_base_render)
{
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

        glPopClientAttrib();
        glPopAttrib();

        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LINE_SMOOTH);
    }

    glLineWidth(1.0);
}

namespace {
    auto MoveLineDotTexture()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "move_line_dot.png"); }
}

void MapWnd::BufferAddMoveLineVertices(GG::GL2DVertexBuffer& dot_verts_buf,
                                       GG::GLRGBAColorBuffer& dot_colours_buf,
                                       GG::GLTexCoordBuffer& dot_star_texture_coords_buf,
                                       float offset, float dot_size, int dot_spacing,
                                       const MapWnd::MovementLineData& move_line,
                                       GG::Clr colour_override) const
{
    const float dot_half_sz = dot_size / 2.0f;

    const auto colour = colour_override == GG::CLR_ZERO ? move_line.colour : colour_override;

    std::vector<std::pair<int, int>> vert_screen_coords;
    vert_screen_coords.reserve(move_line.vertices.size());
    std::transform(move_line.vertices.begin(), move_line.vertices.end(),
                   std::back_inserter(vert_screen_coords),
                   [left{Value(ClientUpperLeft().x)},
                    top{Value(ClientUpperLeft().y)},
                    zoom{ZoomFactor()}] (const auto& vert)
    {
        int x = (vert.x * zoom) + left;
        int y = (vert.y * zoom) + top;
        return std::pair{x, y};
    });

    const auto vert_coord_end = vert_screen_coords.end();
    for (auto vert_coord_it = vert_screen_coords.begin(); vert_coord_it != vert_coord_end;) {
        // get next two vertices screen positions
        const auto& [x1, y1] = *vert_coord_it;
        ++vert_coord_it;
        if (vert_coord_it == vert_coord_end)
            break;
        const auto& [x2, y2] = *vert_coord_it;

        // get unit vector along line connecting vertices
        const float deltaX = x2 - x1;
        const float deltaY = y2 - y1;
        const float length = std::sqrt(deltaX*deltaX + deltaY*deltaY);
        if (!std::isnormal(length)) // safety check
            continue;
        const float uVecX = deltaX / length;
        const float uVecY = deltaY / length;

        // increment along line, adding dots to buffers, until end of line segment is passed
        while (offset < length) {
            // don't know why the dot needs to be shifted half a dot size down/right and
            // rendered 2 x dot size on each axis, but apparently it does...

            // find position of dot from initial vertex position, offset length and unit vectors
            const auto left = x1 + offset * uVecX + dot_half_sz;
            const auto top =  y1 + offset * uVecY + dot_half_sz;

            dot_verts_buf.store(left - dot_size, top - dot_size);
            dot_verts_buf.store(left - dot_size, top + dot_size);
            dot_verts_buf.store(left + dot_size, top + dot_size);
            dot_verts_buf.store(left + dot_size, top - dot_size);

            // move offset to that for next dot
            offset += dot_spacing;

            dot_colours_buf.store(colour);
            dot_colours_buf.store(colour);
            dot_colours_buf.store(colour);
            dot_colours_buf.store(colour);

            dot_star_texture_coords_buf.store(0.0f, 0.0f);
            dot_star_texture_coords_buf.store(0.0f, 1.0f);
            dot_star_texture_coords_buf.store(1.0f, 1.0f);
            dot_star_texture_coords_buf.store(1.0f, 0.0f);
        }

        offset -= length;   // so next segment's dots meld smoothly into this segment's
    }
}

void MapWnd::RenderFleetMovementLines() {
    if (ZoomFactor() < ClientUI::TinyFleetButtonZoomThreshold())
        return;

    // determine animation shift for move lines
    const int dot_spacing = GetOptionsDB().Get<int>("ui.map.fleet.supply.dot.spacing");
    const float rate = static_cast<float>(GetOptionsDB().Get<double>("ui.map.fleet.supply.dot.rate"));
    const int ticks = GG::GUI::GetGUI()->Ticks();
    /* Updated each frame to shift rendered posistion of dots that are drawn to
     * show fleet move lines. */
    const float move_line_animation_shift = static_cast<int>(ticks * rate) % dot_spacing;

    // texture for dots
    const auto move_line_dot_texture = MoveLineDotTexture();
    const float dot_size = Value(move_line_dot_texture->DefaultWidth());

    // dots rendered same size for all zoom levels, so do positioning in screen
    // space instead of universe space
    glPushMatrix();
    glLoadIdentity();

    // render movement lines for all fleets
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindTexture(GL_TEXTURE_2D, move_line_dot_texture->OpenGLId());

    const auto sz = (m_fleet_lines.size() + m_projected_fleet_lines.size()) * 4;
    m_fleet_move_dot_vertices.clear();
    m_fleet_move_dot_vertices.reserve(sz);
    m_fleet_move_dot_colours.clear();
    m_fleet_move_dot_colours.reserve(sz);
    m_fleet_move_dot_star_texture_coords.clear();
    m_fleet_move_dot_star_texture_coords.reserve(sz);

    for (const auto& move_line : m_fleet_lines | range_values) {
        if (move_line.vertices.empty() || move_line.vertices.size() % 2 == 1)
            continue;
        BufferAddMoveLineVertices(m_fleet_move_dot_vertices, m_fleet_move_dot_colours,
                                  m_fleet_move_dot_star_texture_coords, move_line_animation_shift,
                                  dot_size, dot_spacing, move_line);
    }
    m_projected_move_dots_start_index = m_fleet_move_dot_vertices.size();

    for (const auto& proj_line : m_projected_fleet_lines | range_values) {
        if (proj_line.vertices.empty() || proj_line.vertices.size() % 2 == 1)
            continue;
        BufferAddMoveLineVertices(m_fleet_move_dot_vertices, m_fleet_move_dot_colours,
                                  m_fleet_move_dot_star_texture_coords, move_line_animation_shift,
                                  dot_size, dot_spacing, proj_line, GG::CLR_WHITE);
    }


    // re-render selected fleets' movement lines in white
    for (int fleet_id : m_selected_fleet_ids) {
        auto line_it = m_fleet_lines.find(fleet_id);
        if (line_it != m_fleet_lines.end()) {
            const auto& move_line = line_it->second;
            if (!move_line.vertices.empty() && move_line.vertices.size() % 2 == 0) {
                BufferAddMoveLineVertices(m_fleet_move_dot_vertices, m_fleet_move_dot_colours,
                                          m_fleet_move_dot_star_texture_coords,
                                          move_line_animation_shift,
                                          dot_size, dot_spacing, move_line,
                                          GG::CLR_WHITE);
            }
        }
    }

    // after adding all dots to buffer, render general fleet move dots in one call
    m_fleet_move_dot_vertices.activate();
    m_fleet_move_dot_colours.activate();
    m_fleet_move_dot_star_texture_coords.activate();
    glDrawArrays(GL_QUADS, 0, m_projected_move_dots_start_index);

    glDisableClientState(GL_COLOR_ARRAY);

    // render move line ETA indicators for selected fleets
    for (int fleet_id : m_selected_fleet_ids) {
        auto line_it = m_fleet_lines.find(fleet_id);
        if (line_it != m_fleet_lines.end())
            RenderMovementLineETAIndicators(line_it->second);
    }

    // render projected move lines, starting from offset index
    glBindTexture(GL_TEXTURE_2D, move_line_dot_texture->OpenGLId());
    m_fleet_move_dot_vertices.activate();
    m_fleet_move_dot_colours.activate();
    m_fleet_move_dot_star_texture_coords.activate();
    glDrawArrays(GL_QUADS, m_projected_move_dots_start_index,
                 m_fleet_move_dot_vertices.size() - m_projected_move_dots_start_index);


    // render projected move line ETA indicators
    for (const auto& eta_indicator : m_projected_fleet_lines)
        RenderMovementLineETAIndicators(eta_indicator.second, GG::CLR_WHITE);

    glPopClientAttrib();
    glPopMatrix();
}

void MapWnd::RenderMovementLineETAIndicators(const MapWnd::MovementLineData& move_line, GG::Clr clr) {
    const auto& vertices = move_line.vertices;
    if (vertices.empty())
        return; // nothing to draw.


    static constexpr GG::Pt MARKER_HALF_SIZE_PT{GG::X{9}, GG::Y{9}};
    const int MARKER_PTS = ClientUI::Pts();
    auto font = ClientUI::GetBoldFont(MARKER_PTS);
    auto flags = GG::FORMAT_CENTER | GG::FORMAT_VCENTER;

    glPushMatrix();
    glLoadIdentity();
    static constexpr int flag_border = 5;

    for (const auto& vert : vertices) {
        if (!vert.show_eta)
            continue;

        // draw background disc in empire colour, or passed-in colour
        GG::Pt marker_centre = ScreenCoordsFromUniversePosition(vert.x, vert.y);
        GG::Pt ul = marker_centre - MARKER_HALF_SIZE_PT;
        GG::Pt lr = marker_centre + MARKER_HALF_SIZE_PT;

        glDisable(GL_TEXTURE_2D);

        // segmented circle of wedges to indicate blockades
        if (vert.flag_blockade) {
            static constexpr float wedge = static_cast<float>(TWO_PI)/12.0f;
            for (int n = 0; n < 12; n = n + 2) {
                glColor(GG::CLR_BLACK);
                CircleArc(ul + GG::Pt(-flag_border*GG::X1,      -flag_border*GG::Y1),   lr + GG::Pt(flag_border*GG::X1,     flag_border*GG::Y1),    n*wedge,        (n+1)*wedge, true);
                glColor(GG::CLR_RED);
                CircleArc(ul + GG::Pt(-(flag_border)*GG::X1,    -(flag_border)*GG::Y1), lr + GG::Pt((flag_border)*GG::X1,   (flag_border)*GG::Y1),  (n+1)*wedge,    (n+2)*wedge, true);
            }
        } else if (vert.flag_supply_block) {
            static constexpr float wedge = static_cast<float>(TWO_PI)/12.0f;
            for (int n = 0; n < 12; n = n + 2) {
                glColor(GG::CLR_BLACK);
                CircleArc(ul + GG::Pt(-flag_border*GG::X1,      -flag_border*GG::Y1),   lr + GG::Pt(flag_border*GG::X1,     flag_border*GG::Y1),    n*wedge,        (n+1)*wedge, true);
                glColor(GG::CLR_YELLOW);
                CircleArc(ul + GG::Pt(-(flag_border)*GG::X1,    -(flag_border)*GG::Y1), lr + GG::Pt((flag_border)*GG::X1,   (flag_border)*GG::Y1),  (n+1)*wedge,    (n+2)*wedge, true);
            }
        }


        // empire-coloured central fill within wedged outer ring
        glColor(clr == GG::CLR_ZERO ? move_line.colour : clr);
        CircleArc(ul, lr, 0.0, TWO_PI, true);
        glEnable(GL_TEXTURE_2D);


        // render ETA number in white with black shadows
        const std::string text = "<s>" + std::to_string(vert.eta) + "</s>"; // TODO: use to_chars and reused string?
        GG::Font::RenderState rs{GG::CLR_WHITE};
        // TODO cache the text_elements
        const auto text_elements = font->ExpensiveParseFromTextToTextElements(text, flags);
        const auto lines = font->DetermineLines(text, flags, lr.x - ul.x, text_elements);
        font->RenderText(ul, lr, text, flags, lines, rs);
    }
    glPopMatrix();
}

namespace {
    constexpr GG::Pt BORDER_INSET{GG::X1, GG::Y1};

    // Reimplementation of the boost::hash_range function, embedding
    // boost::hash_combine and using std::hash instead of boost::hash
    struct hash_clr {
        std::size_t operator()(const GG::Clr clr) const noexcept {
            static constexpr std::hash<uint32_t> hasher;
            return hasher(uint32_t(clr));
        }
    };

    auto GetFleetFutureTurnDetectionRangeCircles(const ScriptingContext& context,
                                                 const std::set<int>& fleet_ids)
    {
        std::unordered_map<GG::Clr, std::vector<std::pair<GG::Pt, GG::Pt>>, hash_clr> retval;

        for (const auto* fleet : context.ContextObjects().findRaw<Fleet>(fleet_ids)) {
            float fleet_detection_range = 0.0f;
            for (const auto* ship : context.ContextObjects().findRaw<Ship>(fleet->ShipIDs())) {
                if (const Meter* detection_meter = ship->GetMeter(MeterType::METER_DETECTION))
                    fleet_detection_range = std::max(fleet_detection_range, detection_meter->Current());
            }
            // skip fleets with no detection range
            if (fleet_detection_range <= 0.0f)
                continue;
            const int radius = static_cast<int>(fleet_detection_range);
            const GG::Pt rad_pt{GG::X{radius}, GG::Y{radius}};

            // get colour... empire, monster, or neutral
            auto empire = context.GetEmpire(fleet->Owner());
            const GG::Clr empire_colour = empire ? empire->Color() :
                fleet->HasMonsters(context.ContextUniverse()) ? GG::CLR_RED : GG::CLR_WHITE;

            // get all current and future positions of fleet
            for (const auto& node : fleet->MovePath(false, context)) {
                // only show detection circles at turn-end positions
                if (!node.turn_end)
                    continue;

                // if out of system detection not allowed, skip fleets not expected to be in systems
                if (!GetGameRules().Get<bool>("RULE_EXTRASOLAR_SHIP_DETECTION") &&
                    node.object_id == INVALID_OBJECT_ID)
                { continue; }

                GG::Pt circle_centre = GG::Pt{GG::X(node.x), GG::Y(node.y)};
                retval[empire_colour].emplace_back(circle_centre - rad_pt, circle_centre + rad_pt);
            }
        }

        return retval;
    }
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
    for (const auto& [radii_start_run, border_start_run] : m_radii_radii_vertices_indices_runs) {
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

    if (GetOptionsDB().Get<bool>("ui.map.detection.range.future.shown")) {
        glDisable(GL_STENCIL_TEST);

        // future position ranges for selected fleets
        const ScriptingContext& context = IApp::GetApp()->GetContext();
        auto future_turn_circles = GetFleetFutureTurnDetectionRangeCircles(context, m_selected_fleet_ids);
        GG::GL2DVertexBuffer verts;
        verts.reserve(120);
        GG::GLRGBAColorBuffer vert_colours;
        vert_colours.reserve(120);

        for (const auto& [circle_colour, ul_lrs] : future_turn_circles) {
            // get empire colour and calculate brighter radii outline colour
            for (const auto& [ul, lr] : ul_lrs) {
                // store line segments for border lines of radii
                verts.clear();
                vert_colours.clear();
                BufferStoreCircleArcVertices(verts, ul + BORDER_INSET, lr - BORDER_INSET,
                                             0.0, TWO_PI, false, 72, false);

                // store colours for line segments
                vert_colours.store(verts.size(), circle_colour);

                verts.activate();
                vert_colours.activate();
                glDrawArrays(GL_LINES, 0, verts.size());
            }
        }
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
        InitScaleCircleRenderingBuffer(GGHumanClientApp::GetApp()->GetContext().ContextObjects());

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnable(GL_LINE_SMOOTH);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(2.0f);

    const GG::Clr circle_colour = []() { auto retval = GG::CLR_WHITE; retval.a = 128; return retval; }();
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
    if (GGHumanClientApp* app = GGHumanClientApp::GetApp()) {
        app->Register(m_sitrep_panel);
        app->Register(m_object_list_wnd);
        app->Register(m_pedia_panel);
        app->Register(m_side_panel);
        app->Register(m_combat_report_wnd);
        app->Register(m_moderator_wnd);
        app->Register(m_government_wnd);
        // message and player list wnds are managed by the HumanClientFSM
    }
}

void MapWnd::RemoveWindows() {
    // Hide windows by unregistering them which works regardless of their
    // m_visible attribute.
    if (GGHumanClientApp* app = GGHumanClientApp::GetApp()) {
        app->Remove(m_sitrep_panel);
        app->Remove(m_object_list_wnd);
        app->Remove(m_pedia_panel);
        app->Remove(m_side_panel);
        app->Remove(m_combat_report_wnd);
        app->Remove(m_moderator_wnd);
        app->Remove(m_government_wnd);
        // message and player list wnds are managed by the HumanClientFSM
    }
}

void MapWnd::Pan(const GG::Pt delta) {
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

void MapWnd::LButtonDown(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ m_drag_offset = pt - ClientUpperLeft(); }

void MapWnd::LDrag(GG::Pt pt, GG::Pt move, GG::Flags<GG::ModKey> mod_keys) {
    if (GetOptionsDB().Get<bool>("ui.map.lock"))
        return;

    GG::Pt move_to_pt = pt - m_drag_offset;
    CorrectMapPosition(move_to_pt);

    MoveTo(move_to_pt - GG::Pt(AppWidth(), AppHeight()));
    m_dragged = true;
}

void MapWnd::LButtonUp(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    m_drag_offset = GG::Pt(-GG::X1, -GG::Y1);
    m_dragged = false;
}

void MapWnd::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
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

void MapWnd::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    // if in moderator mode, treat as moderator action click
    if (ClientPlayerIsModerator()) {
        // only supported action on empty map location at present is creating a system
        if (m_moderator_wnd->SelectedAction() == ModeratorActionSetting::MAS_CreateSystem) {
            ClientNetworking& net = GGHumanClientApp::GetApp()->Networking();
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

void MapWnd::MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) {
    if (move)
        Zoom(move, pt);
}

void MapWnd::KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if (key == GG::Key::GGK_LSHIFT || key == GG::Key::GGK_RSHIFT)
        ReplotProjectedFleetMovement(mod_keys & GG::MOD_KEY_SHIFT);
}

void MapWnd::KeyRelease(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if (key == GG::Key::GGK_LSHIFT || key == GG::Key::GGK_RSHIFT)
        ReplotProjectedFleetMovement(mod_keys & GG::MOD_KEY_SHIFT);
}

void MapWnd::EnableOrderIssuing(bool enable) {
    // disallow order enabling if this client does not have an empire
    // and is not a moderator
    const auto* app = GGHumanClientApp::GetApp();
    bool moderator = false;
    bool observer = false;
    m_btn_turn->Disable(GGHumanClientApp::GetApp()->SinglePlayerGame() && !enable);
    if (!app) {
        enable = false;
        m_btn_turn->Disable(true);
    } else {
        bool have_empire = (app->EmpireID() != ALL_EMPIRES);
        moderator = (app->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR);
        if (!have_empire && !moderator) {
            enable = false;
            m_btn_turn->Disable(true);
        }
        observer = (app->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER);
    }

    m_moderator_wnd->EnableActions(enable && moderator);
    m_ready_turn = !enable;

    const auto& button_label = (!app) ? UserString("ERROR") :
        (!moderator && !observer && m_ready_turn && !app->SinglePlayerGame()) ?
            UserString("MAP_BTN_TURN_UNREADY") : UserString("MAP_BTN_TURN_UPDATE");

    m_btn_turn->SetText(boost::io::str(FlexibleFormat(button_label) %
                                       std::to_string(app ? app->CurrentTurn() : 0)));
    RefreshTurnButtonTooltip();
    m_side_panel->EnableOrderIssuing(enable);
    m_production_wnd->EnableOrderIssuing(enable);
    m_research_wnd->EnableOrderIssuing(enable);
    m_design_wnd->EnableOrderIssuing(enable);
    m_government_wnd->EnableOrderIssuing(enable);
    FleetUIManager::GetFleetUIManager().EnableOrderIssuing(enable);
}

void MapWnd::InitTurn(ScriptingContext& context) {
    DebugLogger() << "Initializing turn " << context.current_turn;
    SectionedScopedTimer timer("MapWnd::InitTurn");
    timer.EnterSection("init");

    //DebugLogger() << GetSupplyManager().Dump();

    GGHumanClientApp* app = GGHumanClientApp::GetApp();
    const auto client_empire_id = app->EmpireID();
    Universe& universe = context.ContextUniverse();
    ObjectMap& objects = context.ContextObjects();


    TraceLogger(effects) << "MapWnd::InitTurn initial:";
    for (auto obj : objects.allRaw())
        TraceLogger(effects) << obj->Dump();

    timer.EnterSection("meter estimates");
    // update effect accounting and meter estimates
    universe.InitMeterEstimatesAndDiscrepancies(context);

    timer.EnterSection("orders");
    // if we've just loaded the game there may be some unexecuted orders, we
    // should reapply them now, so they are reflected in the UI, but do not
    // influence current meters or their discrepancies for this turn
    app->Orders().ApplyOrders(context);

    timer.EnterSection("meter estimates");
    universe.UpdateMeterEstimates(context);
    universe.ApplyAppearanceEffects(context);

    timer.EnterSection("init rendering");
    // set up system icons, starlanes, galaxy gas rendering
    InitTurnRendering();

    timer.EnterSection("fleet signals");
    // connect system fleet add and remove signals
    for (auto system : objects.allRaw<System>()) {
        m_system_fleet_insert_remove_signals[system->ID()].emplace_back(system->FleetsInsertedSignal.connect(
            [this](std::vector<int>&& fleet_ids, const ObjectMap& objects) {
                RefreshFleetButtons(true);
                for (const auto* fleet : objects.findRaw<Fleet>(std::move(fleet_ids))) {
                    m_fleet_state_change_signals.try_emplace(
                        fleet->ID(),
                        fleet->StateChangedSignal.connect(boost::bind(&MapWnd::RefreshFleetButtons, this, true)));
                }
            }));
        m_system_fleet_insert_remove_signals[system->ID()].emplace_back(system->FleetsRemovedSignal.connect(
            [this](std::vector<int>&& fleets) {
                RefreshFleetButtons(true);
                for (const auto fleet_id : fleets)
                    m_fleet_state_change_signals.erase(fleet_id);
            }));
    }

    m_fleet_state_change_signals.clear(); // should disconnect scoped signals

    // connect fleet change signals to update fleet movement lines, so that ordering
    // fleets to move updates their displayed path and rearranges fleet buttons (if necessary)
    for (const auto fleet : objects.allRaw<Fleet>()) {
        m_fleet_state_change_signals[fleet->ID()] = fleet->StateChangedSignal.connect(
            boost::bind(&MapWnd::RefreshFleetButtons, this, true));
    }

    // set turn button to current turn
    m_btn_turn->SetText(boost::io::str(FlexibleFormat(UserString("MAP_BTN_TURN_UPDATE")) %
                                       std::to_string(context.current_turn)));
    RefreshTurnButtonTooltip();

    m_ready_turn = false;
    MoveChildUp(m_btn_turn);


    timer.EnterSection("sitreps");
    // are there any sitreps to show?
    bool show_intro_sitreps = context.current_turn == 1 &&
        GetOptionsDB().Get<Aggression>("setup.ai.aggression") <= Aggression::TYPICAL;
    DebugLogger() << "showing intro sitreps : " << show_intro_sitreps;
    if (show_intro_sitreps || m_sitrep_panel->NumVisibleSitrepsThisTurn() > 0) {
        m_sitrep_panel->ShowSitRepsForTurn(context.current_turn);
        if (!m_design_wnd->Visible() && !m_research_wnd->Visible()
            && !m_production_wnd->Visible())
        { ShowSitRep(); }
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


    auto this_client_empire = context.GetEmpire(client_empire_id);

    // empire is recreated each turn based on turn update from server, so
    // connections of signals emitted from the empire must be remade each turn
    // (unlike connections to signals from the sidepanel)
    if (this_client_empire) {
        this_client_empire->GetInfluencePool().ChangedSignal.connect(
            boost::bind(&MapWnd::RefreshInfluenceResourceIndicator, this));
        this_client_empire->GetResearchPool().ChangedSignal.connect(
            boost::bind(&MapWnd::RefreshResearchResourceIndicator, this));
        this_client_empire->GetIndustryPool().ChangedSignal.connect(
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
    for (auto& empire : context.Empires() | range_values) {
        empire->UpdateResourcePools(context,
                                    empire->TechCostsTimes(context),
                                    empire->PlanetAnnexationCosts(context),
                                    empire->PolicyAdoptionCosts(context),
                                    empire->ProductionCostsTimes(context));
    }

    timer.EnterSection("refresh government");
    m_government_wnd->Refresh();


    timer.EnterSection("refresh research");
    m_research_wnd->Refresh(context);


    timer.EnterSection("refresh sidepanel");
    SidePanel::Refresh();       // recreate contents of all SidePanels.  ensures previous turn's objects and signals are disposed of


    timer.EnterSection("refresh production wnd");
    m_production_wnd->Refresh(context);


    if (context.current_turn == 1 && this_client_empire) {
        // start first turn with player's system selected
        if (const auto obj = objects.getRaw(this_client_empire->CapitalID())) {
            SelectSystem(obj->SystemID());
            CenterOnMapCoord(obj->X(), obj->Y());
        }

        // default the tech tree to be centred on something interesting
        m_research_wnd->Reset(context);
    } else if (context.current_turn == 1 && !this_client_empire) {
        CenterOnMapCoord(0.0, 0.0);
    }

    timer.EnterSection("refresh indicators");
    RefreshIndustryResourceIndicator();
    RefreshResearchResourceIndicator();
    RefreshInfluenceResourceIndicator();
    RefreshFleetResourceIndicator(context, client_empire_id);
    RefreshPopulationIndicator();
    RefreshDetectionIndicator();


    timer.EnterSection("dispatch exploring");
    FleetUIManager::GetFleetUIManager().RefreshAll(client_empire_id, context);
    DispatchFleetsExploring();

    timer.EnterSection("enable observers");
    if (app->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR) {
        // this client is a moderator
        m_btn_moderator->Disable(false);
        m_btn_moderator->Show();
    } else {
        HideModeratorActions();
        m_btn_moderator->Disable();
        m_btn_moderator->Hide();
    }
    if (app->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER) {
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
    ScopedTimer timer("MapWnd::MidTurnUpdate");

    auto* app = GGHumanClientApp::GetApp();
    ScriptingContext& context = app->GetContext();

    context.ContextUniverse().InitializeSystemGraph(context.Empires(), context.ContextObjects());
    context.ContextUniverse().UpdateCommonFilteredSystemGraphsWithMainObjectMap(context.Empires());

    // set up system icons, starlanes, galaxy gas rendering
    InitTurnRendering();

    FleetUIManager::GetFleetUIManager().RefreshAll(app->EmpireID(), context);
    SidePanel::Refresh();

    // show or hide system names, depending on zoom.  replicates code in MapWnd::Zoom
    if (ZoomFactor() * ClientUI::Pts() < MIN_SYSTEM_NAME_SIZE)
        HideSystemNames();
    else
        ShowSystemNames();
}

void MapWnd::InitTurnRendering() {
    DebugLogger() << "MapWnd::InitTurnRendering";
    ScopedTimer timer("MapWnd::InitTurnRendering");

    using boost::placeholders::_1;
    using boost::placeholders::_2;

    int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
    const Universe& universe = GetUniverse();
    const auto& this_client_known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(client_empire_id);
    const auto& this_client_stale_object_info = universe.EmpireStaleKnowledgeObjectIDs(client_empire_id);
    const ObjectMap& objects = universe.Objects();

    {
        // adjust size of map window for universe and application size
        const auto zoomed_offset_width = universe.UniverseWidth() * ZOOM_MAX;
        Resize(GG::Pt(static_cast<GG::X>(zoomed_offset_width + AppWidth() * 1.5),
                      static_cast<GG::Y>(zoomed_offset_width + AppHeight() * 1.5)));
    }


    // remove any existing fleet movement lines or projected movement lines.  this gets cleared
    // here instead of with the movement line stuff because that would clear some movement lines
    // that come from the SystemIcons
    m_fleet_lines.clear();
    ClearProjectedFleetMovementLines();


    // remove old system icons
    for (const auto& system_icon : m_system_icons | range_values)
        DetachChild(system_icon);
    m_system_icons.clear();

    // create system icons
    for (auto* sys : objects.allRaw<System>()) {
        const int sys_id = sys->ID();

        // skip known destroyed objects
        if (this_client_known_destroyed_objects.contains(sys_id))
            continue;

        // create new system icon
        auto icon = GG::Wnd::Create<SystemIcon>(GG::X0, GG::Y0, GG::X(10), sys_id);
        m_system_icons.insert_or_assign(sys_id, icon);
        icon->InstallEventFilter(shared_from_this());
        if (SidePanel::SystemID() == sys_id)
            icon->SetSelected(true);
        AttachChild(icon);

        // connect UI response signals.  TODO: Make these configurable in GUI?
        icon->LeftClickedSignal.connect(boost::bind(&MapWnd::SystemLeftClicked, this, _1));
        icon->RightClickedSignal.connect(boost::bind(
            static_cast<void (MapWnd::*)(int, GG::Flags<GG::ModKey>)>(&MapWnd::SystemRightClicked), this, _1, _2));
        icon->LeftDoubleClickedSignal.connect(boost::bind(&MapWnd::SystemDoubleClicked, this, _1));
        icon->MouseEnteringSignal.connect(boost::bind(&MapWnd::MouseEnteringSystem, this, _1, _2));
        icon->MouseLeavingSignal.connect(boost::bind(&MapWnd::MouseLeavingSystem, this, _1));
    }

    // temp: reset starfield each turn
    m_starfield_verts.clear();
    m_starfield_colours.clear();
    // end temp

    // create buffers for system icon and galaxy gas rendering, and starlane rendering
    InitSystemRenderingBuffers();
    InitStarlaneRenderingBuffers();

    // position system icons
    DoSystemIconsLayout(objects);


    // remove old field icons
    for (const auto& field_icon : m_field_icons)
        DetachChild(field_icon);
    m_field_icons.clear();

    // create field icons
    std::vector<std::pair<int, float>> field_ids_by_size;
    for (auto* field : objects.allRaw<Field>())
        field_ids_by_size.emplace_back(field->ID(), field->GetMeter(MeterType::METER_SIZE)->Initial());
    std::sort(field_ids_by_size.begin(), field_ids_by_size.end(), [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });

    for (const auto& [fld_id, field_size] : field_ids_by_size) {
        // skip known destroyed and stale fields
        if (this_client_known_destroyed_objects.contains(fld_id))
            continue;
        if (this_client_stale_object_info.contains(fld_id))
            continue;
        // don't skip not visible but not stale fields; still expect these to be where last seen, or near there
        //if (field->GetVisibility(client_empire_id, universe) <= Visibility::VIS_NO_VISIBILITY)
        //    continue;

        // create new system icon
        auto icon = GG::Wnd::Create<FieldIcon>(fld_id);
        m_field_icons.push_back(icon);
        icon->InstallEventFilter(shared_from_this());

        AttachChild(icon);

        icon->RightClickedSignal.connect(boost::bind(
            static_cast<void (MapWnd::*)(int)>(&MapWnd::FieldRightClicked), this, _1));
    }

    // position field icons
    DoFieldIconsLayout(objects);
    InitFieldRenderingBuffers();

    InitVisibilityRadiiRenderingBuffers();

    // create fleet buttons and move lines.  needs to be after InitStarlaneRenderingBuffers so that m_starlane_endpoints is populated
    RefreshFleetButtons(true);

    // move field icons to bottom of child stack so that other icons can be moused over with a field
    for (const auto& field_icon : m_field_icons)
        MoveChildDown(field_icon);
}

void MapWnd::InitSystemRenderingBuffers() {
    DebugLogger() << "MapWnd::InitSystemRenderingBuffers";
    ScopedTimer timer("MapWnd::InitSystemRenderingBuffers");

    // clear out all the old buffers
    ClearSystemRenderingBuffers();

    // Generate texture coordinates to be used for subsequent vertex buffer creation.
    // Note these coordinates assume the texture is twice as large as it should
    // be.  This allows us to use one set of texture coords for everything, even
    // though the star-halo textures must be rendered at sizes as much as twice
    // as large as the star-disc textures.
    static constexpr std::array<GLfloat, 4> tex_coords{-0.5, -0.5, 1.5, 1.5};

    for (std::size_t i = 0; i < m_system_icons.size(); ++i)
        GG::Texture::InitBuffer(m_star_texture_coords, tex_coords);

    const auto& objects = GGHumanClientApp::GetApp()->GetContext().ContextObjects();


    for (const auto& [system_id, icon] : m_system_icons) {
        auto* system = objects.getRaw<const System>(system_id);
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
            GG::Texture::InitBuffer(core_vertices, icon_ul_x, icon_ul_y, icon_lr_x, icon_lr_y);
        }

        if (icon->HaloTexture()) {
            auto& halo_vertices = m_star_halo_quad_vertices[icon->HaloTexture()];
            GG::Texture::InitBuffer(halo_vertices, icon_ul_x, icon_ul_y, icon_lr_x, icon_lr_y);
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

            const auto [tex_coord_min_x, tex_coord_min_y, tex_coord_max_x, tex_coord_max_y] =
                gas_texture->DefaultTexCoords();

            // gas texture is expected to be a 4 wide by 3 high grid
            // also add a bit of padding to hopefully avoid artifacts of texture edges
            const GLfloat tx_low_x = tex_coord_min_x  + (subtexture_x_index + 0)*(tex_coord_max_x - tex_coord_min_x)/4;
            const GLfloat tx_high_x = tex_coord_min_x + (subtexture_x_index + 1)*(tex_coord_max_x - tex_coord_min_x)/4;
            const GLfloat tx_low_y = tex_coord_min_y  + (subtexture_y_index + 0)*(tex_coord_max_y - tex_coord_min_y)/3;
            const GLfloat tx_high_y = tex_coord_min_y + (subtexture_y_index + 1)*(tex_coord_max_y - tex_coord_min_y)/3;

            const std::array<GLfloat, 4> rot_tex_coords{tx_low_x, tx_low_y, tx_high_x, tx_high_y};
            GG::Texture::InitBuffer(m_galaxy_gas_texture_coords, rot_tex_coords);
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
    typedef std::unordered_multimap<int,int> SupplyLaneMMap;

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

    /**
    GetPathsThroughSupplyLanes starts with:

    \p terminal_points are system ids of systems that contain either
    a resource source or a resource sink.

    \p supply_lanes are pairs of system ids at the end of supply
    lanes.

    GetPathsThroughSupplyLanes returns a good path.

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
    boost::container::flat_set<int> GetPathsThroughSupplyLanes(
        const std::unordered_set<int> & terminal_points, const SupplyLaneMMap& supply_lanes)
    {
        boost::container::flat_set<int> good_path;

        // No terminal points, so all paths lead nowhere.
        if (terminal_points.empty())
            return good_path;

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
            try_next.emplace_back(terminal_point, terminal_point, terminal_point);
            visited.emplace(terminal_point, PathInfo(terminal_point));
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
                    visited.emplace(next, PathInfo(curr.curr, curr.origin));
                    try_next.emplace_back(curr.curr, next, curr.origin);

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
            return good_path;

        // Return all systems on any path back to a terminal point.
        // Start from every mid point and back track along all paths
        // from that mid point adding each system to the good path.
        // Stop back tracking when you hit a system already on the good
        // path.

        // All visited systems on the path(s) from this midpoint not yet processed.
        boost::unordered_set<int> unprocessed;

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
                if ((previous_ii_sys = visited.find(ii_sys)) != visited.end() && (!good_path.contains(ii_sys))) {
                    good_path.insert(ii_sys);
                    unprocessed.insert(previous_ii_sys->second.one_hop_back.begin(),
                                       previous_ii_sys->second.one_hop_back.end());
                }
            }
        }

        return good_path;
    }
}

namespace {
    // Reimplementation of the boost::hash_range function, embedding
    // boost::hash_combine and using std::hash instead of boost::hash
    struct hash_set {
        static constexpr std::hash<int> hasher{};

        static constexpr bool is_noexcept =
            noexcept((std::size_t{} ^ hasher(42)) + 0x9e3779b9 + (13<<6) + (13>>2));
        std::size_t operator()(const std::set<int>& set) const noexcept(is_noexcept) {
            std::size_t seed(2283351); // arbitrary number
            for (auto element : set)
                seed ^= hasher(element) + 0x9e3779b9 + (seed<<6) + (seed>>2);
            return seed;
        }
    };


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

    using flat_int_set = boost::container::flat_set<int>;
    using flat_map_int_sets = boost::container::flat_map<flat_int_set, flat_int_set>;
    using flat_map_int_int_set = boost::container::flat_map<int, flat_int_set>;

    auto GetResPoolLaneInfo(const ObjectMap& objects, const Empire& empire, const SupplyManager& supply) {
        flat_map_int_sets res_pool_systems;
        flat_map_int_sets res_group_cores;
        flat_int_set res_group_core_members;
        flat_map_int_int_set member_to_core;
        flat_int_set under_alloc_res_grp_core_members;

        const ProductionQueue& queue = empire.GetProductionQueue();
        auto& allocated_pp(queue.AllocatedPP());
        auto& available_pp(empire.GetIndustryPool().Output());

        // For each industry set,
        // add all planet's systems to res_pool_systems[industry set]
        for (const auto& available_pp_group : available_pp) {
            float group_pp = available_pp_group.second;
            if (group_pp < 1e-4f)
                continue;

            // std::string this_pool = "( ";
            for (int object_id : available_pp_group.first) {
                // this_pool += std::to_string(object_id) +", ";

                auto* planet = objects.getRaw<Planet>(object_id);
                if (!planet)
                    continue;

                //DebugLogger() << "Empire " << empire_id << "; Planet (" << object_id << ") is named " << planet->Name();

                int system_id = planet->SystemID();
                auto* system = objects.getRaw<System>(system_id);
                if (!system)
                    continue;

                res_pool_systems[available_pp_group.first].insert(system_id);
            }
            // this_pool += ")";
            //DebugLogger() << "Empire " << empire_id << "; ResourcePool[RE_INDUSTRY] resourceGroup (" << this_pool << ") has (" << available_pp_group.second << " PP available";
            //DebugLogger() << "Empire " << empire_id << "; ResourcePool[RE_INDUSTRY] resourceGroup (" << this_pool << ") has (" << allocated_pp[available_pp_group.first] << " PP allocated";
        }


        // Convert supply starlanes to non-directional.  This saves half of the lookups.
        GetPathsThroughSupplyLanes::SupplyLaneMMap resource_supply_lanes_undirected;
        const auto& resource_supply_lanes_directed = supply.SupplyStarlaneTraversals(empire.EmpireID());

        for (const auto& supply_lane : resource_supply_lanes_directed) {
            resource_supply_lanes_undirected.emplace(supply_lane.first, supply_lane.second);
            resource_supply_lanes_undirected.emplace(supply_lane.second, supply_lane.first);
        }

        // For each pool of resources find all paths available through the supply network.

        for (auto& [group_core, group_systems] : res_pool_systems) {
            // All individual resource system are included in the network on their own.
            group_core.insert(group_systems.begin(), group_systems.end());
            res_group_core_members.insert(group_systems.begin(), group_systems.end());

            // Convert res_pool_system.second from set<int> to
            // unordered_set<int> to improve lookup speed.
            std::unordered_set<int> terminal_points{group_systems.begin(), group_systems.end()};

            const auto paths = GetPathsThroughSupplyLanes::GetPathsThroughSupplyLanes(
                terminal_points, resource_supply_lanes_undirected);

            // All systems on the paths are valid end points so they are
            // added to the core group of systems that will be rendered
            // with thick lines.
            for (int waypoint : paths) {
                group_core.insert(waypoint);
                res_group_core_members.insert(waypoint);
                member_to_core[waypoint] = group_core;
            }
        }

        // Take note of all systems of under-allocated resource groups.
        for (const auto& available_pp_group : available_pp) {
            float group_pp = available_pp_group.second;
            if (group_pp < 0.01f)
                continue;

            auto allocated_it = allocated_pp.find(available_pp_group.first);
            if (allocated_it == allocated_pp.end() || (group_pp > allocated_it->second + 0.05)) {
                auto group_core_it = res_group_cores.find(available_pp_group.first);
                if (group_core_it != res_group_cores.end()) {
                    under_alloc_res_grp_core_members.insert(group_core_it->second.begin(),
                                                            group_core_it->second.end());
                }
            }
        }

        return std::tuple{res_pool_systems, res_group_cores, res_group_core_members,
                          member_to_core, under_alloc_res_grp_core_members};
    }


    void PrepFullLanesToRender(const std::unordered_map<int, std::shared_ptr<SystemIcon>>& sys_icons,
                               GG::GL2DVertexBuffer& starlane_vertices,
                               GG::GLRGBAColorBuffer& starlane_colors,
                               int empire_id, const ScriptingContext& context)
    {
        const auto& this_client_known_destroyed_objects =
            context.ContextUniverse().EmpireKnownDestroyedObjectIDs(empire_id);
        const auto& empires = context.Empires();
        const auto& sm = context.supply;
        const auto& o = context.ContextObjects();
        const GG::Clr UNOWNED_LANE_COLOUR = GetOptionsDB().Get<GG::Clr>("ui.map.starlane.color");

        std::set<std::pair<int, int>> already_rendered_full_lanes;

        for (const auto& id_icon : sys_icons) {
            int system_id = id_icon.first;

            // skip systems that don't actually exist
            if (this_client_known_destroyed_objects.contains(system_id))
                continue;

            auto start_system = o.get<System>(system_id);
            if (!start_system) {
                ErrorLogger() << "GetFullLanesToRender couldn't get system with id " << system_id;
                continue;
            }

            // add system's starlanes
            for (const auto lane_end_sys_id : start_system->Starlanes()) {
                // skip lanes to systems that don't actually exist
                if (this_client_known_destroyed_objects.contains(lane_end_sys_id))
                    continue;

                auto* dest_system = o.getRaw<const System>(lane_end_sys_id);
                if (!dest_system)
                    continue;


                // check that this lane isn't already in map / being rendered.
                if (already_rendered_full_lanes.contains({start_system->ID(), dest_system->ID()}))
                    continue;
                already_rendered_full_lanes.emplace(start_system->ID(), dest_system->ID());
                already_rendered_full_lanes.emplace(dest_system->ID(), start_system->ID());


                // add vertices for this full-length starlane
                LaneEndpoints lane_endpoints = StarlaneEndPointsFromSystemPositions(start_system->X(), start_system->Y(), dest_system->X(), dest_system->Y());
                starlane_vertices.store(lane_endpoints.X1, lane_endpoints.Y1);
                starlane_vertices.store(lane_endpoints.X2, lane_endpoints.Y2);


                // determine colour(s) for lane based on which empire(s) can transfer resources along the lane.
                // todo: multiple rendered lanes (one for each empire) when multiple empires use the same lane.
                GG::Clr lane_colour = UNOWNED_LANE_COLOUR;    // default colour if no empires transfer resources along starlane
                for (const auto& [empire_id, empire] : empires) {
                    const auto& resource_supply_lanes = sm.SupplyStarlaneTraversals(empire_id);

                    std::pair<int, int> lane_forward{start_system->ID(), dest_system->ID()};
                    std::pair<int, int> lane_backward{dest_system->ID(), start_system->ID()};

                    // see if this lane exists in this empire's supply propagation lanes set.  either direction accepted.
                    if (resource_supply_lanes.contains(lane_forward) ||
                        resource_supply_lanes.contains(lane_backward))
                    {
                        lane_colour = empire->Color();
                        break;
                    }
                }

                // vertex colours for starlane
                starlane_colors.store(lane_colour);
                starlane_colors.store(lane_colour);
            }
        }
    }

    void PrepResourceConnectionLanesToRender(const std::unordered_map<int, std::shared_ptr<SystemIcon>>& sys_icons,
                                             int empire_id,
                                             std::set<std::pair<int, int>>& rendered_half_starlanes,
                                             GG::GL2DVertexBuffer& rc_starlane_vertices,
                                             GG::GLRGBAColorBuffer& rc_starlane_colors,
                                             const ScriptingContext& context)
    {
        rendered_half_starlanes.clear();

        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return;
        const auto lane_colour = empire->Color();

        auto [res_pool_systems, // map keyed by ResourcePool (set of objects) to the corresponding set of system ids
              res_group_cores,  // map keyed by ResourcePool to the set of systems considered the core of the corresponding ResGroup
              res_group_core_members,
              member_to_core,
              under_alloc_res_grp_core_members] =
            GetResPoolLaneInfo(context.ContextObjects(), *empire, context.supply);

        const auto& this_client_known_destroyed_objects = context.ContextUniverse().DestroyedObjectIds();
        //unused variable const GG::Clr UNOWNED_LANE_COLOUR = GetOptionsDB().Get<GG::Clr>("ui.map.starlane.color");


        for (const auto system_id : sys_icons | range_keys) {
            // skip systems that don't actually exist
            if (this_client_known_destroyed_objects.contains(system_id))
                continue;

            const auto* const start_system = context.ContextObjects().getRaw<System>(system_id);
            if (!start_system) {
                ErrorLogger() << "GetFullLanesToRender couldn't get system with id " << system_id;
                continue;
            }

            // add system's starlanes
            for (const auto lane_end_sys_id : start_system->Starlanes()) {
                // skip lanes to systems that don't actually exist
                if (this_client_known_destroyed_objects.contains(lane_end_sys_id))
                    continue;

                const auto* const dest_system = context.ContextObjects().getRaw<const System>(lane_end_sys_id);
                if (!dest_system)
                    continue;
                //std::cout << "colouring lanes between " << start_system->Name() << " and " << dest_system->Name() << std::endl;


                // check that this lane isn't already going to be rendered.  skip it if it is.
                if (rendered_half_starlanes.contains({start_system->ID(), dest_system->ID()}))
                    continue;


                // add resource connection highlight lanes
                //std::pair<int, int> lane_forward{start_system->ID(), dest_system->ID()};
                //std::pair<int, int> lane_backward{dest_system->ID(), start_system->ID()};
                LaneEndpoints lane_endpoints = StarlaneEndPointsFromSystemPositions(start_system->X(), start_system->Y(), dest_system->X(), dest_system->Y());

                if (!res_group_core_members.contains(start_system->ID()))
                    continue;

                //start system is a res Grp core member for empire -- highlight
                float indicator_extent = 0.5f;
                GG::Clr lane_colour_to_use{lane_colour};
                if (under_alloc_res_grp_core_members.contains(start_system->ID()))
                    lane_colour_to_use = GG::DarkenClr(GG::InvertClr(lane_colour));

                const auto start_core = member_to_core.find(start_system->ID());
                const auto dest_core = member_to_core.find(dest_system->ID());
                if (start_core != member_to_core.end() && dest_core != member_to_core.end()
                    && (start_core->second != dest_core->second))
                { indicator_extent = 0.2f; }

                rc_starlane_vertices.store(lane_endpoints.X1, lane_endpoints.Y1);
                rc_starlane_vertices.store((lane_endpoints.X2 - lane_endpoints.X1) * indicator_extent + lane_endpoints.X1,  // part way along starlane
                                           (lane_endpoints.Y2 - lane_endpoints.Y1) * indicator_extent + lane_endpoints.Y1);

                rc_starlane_colors.store(lane_colour_to_use);
                rc_starlane_colors.store(lane_colour_to_use);
            }
        }
    }

    void PrepObstructedLaneTraversalsToRender(const auto& sys_icons,
                                              std::set<std::pair<int, int>>& rendered_half_starlanes, // TODO: pass as better container...
                                              GG::GL2DVertexBuffer& starlane_vertices,
                                              GG::GLRGBAColorBuffer& starlane_colors,
                                              const ScriptingContext& context)
    {
        static_assert(std::is_same_v<int, std::decay_t<decltype(sys_icons.begin()->first)>>);
        static_assert(std::is_same_v<SystemIcon, std::decay_t<decltype(*sys_icons.begin()->second)>>);

        const auto& this_client_known_destroyed_objects = context.ContextUniverse().DestroyedObjectIds();


        for (const auto system_id : sys_icons | range_keys) {
            // skip systems that don't actually exist
            if (this_client_known_destroyed_objects.contains(system_id))
                continue;

            auto start_system = context.ContextObjects().get<System>(system_id);
            if (!start_system) {
                ErrorLogger() << "MapWnd::InitStarlaneRenderingBuffers couldn't get system with id " << system_id;
                continue;
            }

            // add system's starlanes
            for (const auto lane_end_sys_id : start_system->Starlanes()) {
                // skip lanes to systems that don't actually exist
                if (this_client_known_destroyed_objects.contains(lane_end_sys_id))
                    continue;

                auto dest_system = context.ContextObjects().get<System>(lane_end_sys_id);
                if (!dest_system)
                    continue;
                //std::cout << "colouring lanes between " << start_system->Name() << " and " << dest_system->Name() << std::endl;


                // check that this lane isn't already going to be rendered.  skip it if it is.
                if (rendered_half_starlanes.contains({start_system->ID(), dest_system->ID()}))
                    continue;


                // add obstructed lane traversals as half lanes
                for (const auto& [loop_empire_id, loop_empire] : context.Empires()) {
                    const auto& resource_obstructed_supply_lanes =
                        context.supply.SupplyObstructedStarlaneTraversals(loop_empire_id);

                    // see if this lane exists in this empire's obstructed supply propagation lanes set.  either direction accepted.
                    if (!resource_obstructed_supply_lanes.contains({start_system->ID(), dest_system->ID()}))
                        continue;

                    // found an empire that has a half lane here, so add it.
                    rendered_half_starlanes.emplace(start_system->ID(), dest_system->ID());  // inserted as ordered pair, so both directions can have different half-lanes

                    LaneEndpoints lane_endpoints = StarlaneEndPointsFromSystemPositions(
                        start_system->X(), start_system->Y(), dest_system->X(), dest_system->Y());
                    starlane_vertices.store(lane_endpoints.X1, lane_endpoints.Y1);
                    starlane_vertices.store((lane_endpoints.X1 + lane_endpoints.X2) * 0.5f,   // half way along starlane
                                            (lane_endpoints.Y1 + lane_endpoints.Y2) * 0.5f);

                    GG::Clr lane_colour = loop_empire->Color();
                    starlane_colors.store(lane_colour);
                    starlane_colors.store(lane_colour);

                    //std::cout << "Adding half lane between " << start_system->Name() << " to " << dest_system->Name() << " with colour of empire " << empire->Name() << std::endl;

                    break;
                }
            }
        }
    }

    auto CalculateStarlaneEndpoints(const std::unordered_map<int, std::shared_ptr<SystemIcon>>& sys_icons,
                                    const ScriptingContext& context, int empire_id)
    {
        std::map<std::pair<int, int>, LaneEndpoints> retval;

        const auto& this_client_known_destroyed_objects =
            context.ContextUniverse().EmpireKnownDestroyedObjectIDs(empire_id);

        const auto& objects = context.ContextObjects();

        for (auto const system_id : sys_icons | range_keys) {
            // skip systems that don't actually exist
            if (this_client_known_destroyed_objects.contains(system_id))
                continue;

            auto start_system = objects.get<System>(system_id);
            if (!start_system) {
                ErrorLogger() << "GetFullLanesToRender couldn't get system with id " << system_id;
                continue;
            }

            // add system's starlanes
            for (const auto lane_end_sys_id : start_system->Starlanes()) {
                // skip lanes to systems that don't actually exist
                if (this_client_known_destroyed_objects.contains(lane_end_sys_id))
                    continue;

                auto dest_system = objects.get<System>(lane_end_sys_id);
                if (!dest_system)
                    continue;

                retval.insert_or_assign(
                    {system_id, lane_end_sys_id},
                    StarlaneEndPointsFromSystemPositions(start_system->X(), start_system->Y(),
                                                         dest_system->X(), dest_system->Y()));
                retval.insert_or_assign(
                    {lane_end_sys_id, system_id},
                    StarlaneEndPointsFromSystemPositions(dest_system->X(), dest_system->Y(),
                                                         start_system->X(), start_system->Y()));
            }
        }

        return retval;
    }
}

void MapWnd::InitStarlaneRenderingBuffers() {
    DebugLogger() << "MapWnd::InitStarlaneRenderingBuffers";
    ScopedTimer timer("MapWnd::InitStarlaneRenderingBuffers", true);

    ClearStarlaneRenderingBuffers();

    const auto* app = GGHumanClientApp::GetApp();
    const int empire_id = app->EmpireID();
    const auto& context = app->GetContext();

    // todo: move this somewhere better... fill in starlane endpoint cache
    m_starlane_endpoints = CalculateStarlaneEndpoints(m_system_icons, context, empire_id);


    // temp storage
    std::set<std::pair<int, int>> rendered_half_starlanes;  // stored as unaltered pairs, so that a each direction of traversal can be shown separately


    // add vertices and colours to lane rendering buffers
    PrepFullLanesToRender(m_system_icons, m_starlane_vertices, m_starlane_colors, empire_id, context);
    PrepResourceConnectionLanesToRender(m_system_icons, empire_id, rendered_half_starlanes,
                                        m_RC_starlane_vertices, m_RC_starlane_colors, context);
    PrepObstructedLaneTraversalsToRender(m_system_icons, rendered_half_starlanes,
                                         m_starlane_vertices, m_starlane_colors, context);


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
    const auto* app = GGHumanClientApp::GetApp();
    const auto empire_id = app->EmpireID();
    const auto current_turn = app->CurrentTurn();

    // reverse size processing so large fields are painted first and smaller ones on top of larger ones
    for (auto& field_icon : m_field_icons | range_reverse) {
        bool current_field_visible =
            (empire_id == ALL_EMPIRES) ||
            universe.GetObjectVisibilityByEmpire(field_icon->FieldID(), empire_id) > Visibility::VIS_BASIC_VISIBILITY;
        auto field = universe.Objects().get<Field>(field_icon->FieldID());
        if (!field)
            continue;
        const float FIELD_SIZE = field->GetMeter(MeterType::METER_SIZE)->Initial();  // field size is its radius
        if (FIELD_SIZE <= 0)
            continue;
        const auto& field_texture = field_icon->FieldTexture();
        if (!field_texture)
            continue;

        // group by texture as much as possible for fewer GL calls, but generally paint fields one by one according to size
        // so smaller ones get painted over larger ones, including across different textures
        // -> if field_vertices is empty (initial conditions), or the last considered texture is not the same as the current field_texture, we create new buffer;
        //    otherwise, the field type/texture did not change, so we keep adding vertices to the old buffer, and end up with fewer gl calls during rendering
        auto& field_vertices = current_field_visible ? m_field_vertices_visible : m_field_vertices_not_visible;
        const bool should_create_new_buffer = (field_vertices.empty() || field_vertices.back().first != field_texture);
        GG::GL2DVertexBuffer& current_field_vertex_buffer =
            should_create_new_buffer ?
                field_vertices.emplace_back(field_texture, GG::GL2DVertexBuffer()).second :
                field_vertices.back().second;

        // determine field rotation angle...
        float rotation_angle = field->ID() * 27.0f; // arbitrary rotation in radians ("27.0" is just a number that produces pleasing results)
        // per-turn rotation of texture. TODO: make depend on something scriptable
        float rotation_speed = 0.03f;               // arbitrary rotation rate in radians
        if (rotation_speed != 0.0f)
            rotation_angle += current_turn * rotation_speed;

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

    std::size_t max_buffer_size = 0;

    static constexpr auto field_texture_exists = [](const auto& field_vertices_pair) { return field_vertices_pair.first.get(); };
    for (auto& field_vertices : { std::ref(m_field_vertices_not_visible), std::ref(m_field_vertices_visible) }) {
        for (auto& [field_texture, buffer] : field_vertices.get() | range_filter(field_texture_exists)) {
            // TODO: why the binding here?
            glBindTexture(GL_TEXTURE_2D, field_texture->OpenGLId());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            buffer.createServerBuffer();
            max_buffer_size = std::max(max_buffer_size, buffer.size());
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // make texture coords buffer's size proportional to largest number of fields rendered
    // in one GL call; proxy for that is max_buffer_size computed earlier, and since those
    // vertex buffers store 4 vertices per field, we divide by 4 to get field count
    const std::size_t max_fields_per_texture = max_buffer_size / 4;
    for (std::size_t i = 0; i < max_fields_per_texture; ++i) {
        m_field_texture_coords.store(1.0f, 0.0f);
        m_field_texture_coords.store(0.0f, 0.0f);
        m_field_texture_coords.store(0.0f, 1.0f);
        m_field_texture_coords.store(1.0f, 1.0f);
    }
    m_field_texture_coords.createServerBuffer();
}

void MapWnd::ClearFieldRenderingBuffers() {
    m_field_vertices_not_visible.clear();
    m_field_vertices_visible.clear();
    m_field_texture_coords.clear();
    m_field_scanline_circles.clear();
}

void MapWnd::InitVisibilityRadiiRenderingBuffers() {
    DebugLogger() << "MapWnd::InitVisibilityRadiiRenderingBuffers";
    //std::cout << "InitVisibilityRadiiRenderingBuffers" << std::endl;
    ScopedTimer timer("MapWnd::InitVisibilityRadiiRenderingBuffers", true);

    ClearVisibilityRadiiRenderingBuffers();

    const ScriptingContext& context = IApp::GetApp()->GetContext();
    const Universe& universe = context.ContextUniverse();
    const ObjectMap& objects = context.ContextObjects();

    int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
    const auto& stale_object_ids = universe.EmpireStaleKnowledgeObjectIDs(client_empire_id);
    auto empire_position_max_detection_ranges = universe.GetEmpiresAndNeutralPositionDetectionRanges(objects, stale_object_ids);
    //auto empire_position_max_detection_ranges = universe.GetEmpiresPositionNextTurnFleetDetectionRanges(context);


    std::unordered_map<GG::Clr, std::vector<std::pair<GG::Pt, GG::Pt>>, hash_clr> circles;


    for (const auto& [empire_id, detection_circles] : empire_position_max_detection_ranges) {
        if (empire_id == ALL_EMPIRES)
            continue;
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "InitVisibilityRadiiRenderingBuffers couldn't find empire with id: " << empire_id;
            continue;
        }

        for (const auto& [centre, radius] : detection_circles) {
            if (radius < 5.0f || radius > 2048.0f)  // hide uselessly small and ridiculously large circles. the latter so super-testers don't have an empire-coloured haze over the whole map.
                continue;
            const auto& [X, Y] = centre;

            GG::Clr circle_colour = empire->Color();
            circle_colour.a = 8*GetOptionsDB().Get<int>("ui.map.detection.range.opacity");

            const GG::Pt circle_centre = GG::Pt{GG::X(X), GG::Y(Y)};
            const GG::Pt rad_pt{GG::X(radius), GG::Y(radius)};
            const GG::Pt ul = circle_centre - rad_pt;
            const GG::Pt lr = circle_centre + rad_pt;

            circles[circle_colour].emplace_back(ul, lr);
        }
        //std::cout << "adding radii circle at: " << circle_centre << " for empire: " << it->first.first << std::endl;
    }


    // loop over colours / empires, adding a batch of triangles to buffers for
    // each's visibilty circles and outlines
    for (const auto& [circle_colour, ul_lrs] : circles) {
        // get empire colour and calculate brighter radii outline colour
        GG::Clr border_colour = AdjustBrightness(circle_colour, 2.0, true);
        border_colour.a = std::min(255, border_colour.a + 80);

        const std::size_t radii_start_index = m_visibility_radii_vertices.size();
        const std::size_t border_start_index = m_visibility_radii_border_vertices.size();

        for (const auto& [ul, lr] : ul_lrs) {
            static constexpr std::size_t verts_per_circle = 36;
            static constexpr std::size_t vert_per_cricle_edge = 72;

            unsigned int initial_size = m_visibility_radii_vertices.size();
            // store triangles for filled / transparent part of radii
            BufferStoreCircleArcVertices(m_visibility_radii_vertices, ul, lr,
                                         0.0, TWO_PI, true, verts_per_circle, false);

            // store colours for triangles
            unsigned int size_increment = m_visibility_radii_vertices.size() - initial_size;
            for (unsigned int count = 0; count < size_increment; ++count)
                 m_visibility_radii_colors.store(circle_colour);

            // store line segments for border lines of radii
            initial_size = m_visibility_radii_border_vertices.size();
            BufferStoreCircleArcVertices(m_visibility_radii_border_vertices, ul + BORDER_INSET, lr - BORDER_INSET,
                                         0.0, TWO_PI, false, vert_per_cricle_edge, false);

            // store colours for line segments
            size_increment = m_visibility_radii_border_vertices.size() - initial_size;
            for (unsigned int count = 0; count < size_increment; ++count)
                 m_visibility_radii_border_colors.store(border_colour);
        }

        // store how many vertices to render for this colour
        std::size_t radii_end_index = m_visibility_radii_vertices.size();
        std::size_t border_end_index = m_visibility_radii_border_vertices.size();

        m_radii_radii_vertices_indices_runs.emplace_back(
            std::pair{radii_start_index, radii_end_index - radii_start_index},
            std::pair{border_start_index, border_end_index - border_start_index});
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

void MapWnd::InitScaleCircleRenderingBuffer(const ObjectMap& objects) {
    ClearScaleCircleRenderingBuffer();

    if (!m_scale_line)
        return;

    int radius = static_cast<int>(Value(m_scale_line->GetLength()) / std::max(0.001, m_scale_line->GetScaleFactor()));
    if (radius < 5)
        return;

    auto selected_system = objects.get<System>(SidePanel::SystemID());
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
    for (auto& system_icon : m_system_icons)
        system_icon.second->ShowName();
}

void MapWnd::HideSystemNames() {
    for (auto& system_icon : m_system_icons)
        system_icon.second->HideName();
}

void MapWnd::CenterOnMapCoord(double x, double y) {
    if (GetOptionsDB().Get<bool>("ui.map.lock"))
        return;

    const GG::Pt ul = ClientUpperLeft();
    const auto zf = ZoomFactor();
    const double current_x = (AppWidth()/2 - ul.x) / zf;
    const double current_y = (AppHeight()/2 - ul.y) / zf;
    GG::Pt map_move = GG::Pt(static_cast<GG::X>((current_x - x) * zf),
                             static_cast<GG::Y>((current_y - y) * zf));
    OffsetMove(map_move);

    // this correction ensures that the centering doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ClientUpperLeft();
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
    m_combat_report_wnd->SetLog(log_id);
    m_combat_report_wnd->Show();
    GG::GUI::GetGUI()->MoveUp(m_combat_report_wnd);
    PushWndStack(m_combat_report_wnd);
}

void MapWnd::ShowTech(std::string tech_name) {
    if (m_research_wnd->Visible())
        m_research_wnd->ShowTech(tech_name);
    if (m_in_production_view_mode) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowTechInEncyclopedia(std::move(tech_name));
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetTech(std::move(tech_name));
    }
}

void MapWnd::ShowPolicy(std::string policy_name) {
    if (m_production_wnd->Visible()) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowPolicyInEncyclopedia(std::move(policy_name));
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetPolicy(std::move(policy_name));
    }
}

void MapWnd::ShowBuildingType(std::string building_type_name) {
    if (m_production_wnd->Visible()) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowBuildingTypeInEncyclopedia(std::move(building_type_name));
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetBuildingType(std::move(building_type_name));
    }
}

void MapWnd::ShowShipPart(std::string ship_part_name) {
    if (m_design_wnd->Visible())
        m_design_wnd->ShowShipPartInEncyclopedia(ship_part_name);
    if (m_in_production_view_mode) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowShipPartInEncyclopedia(std::move(ship_part_name));
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetShipPart(std::move(ship_part_name));
    }
}

void MapWnd::ShowShipHull(std::string ship_hull_name) {
    if (m_design_wnd->Visible()) {
        m_design_wnd->ShowShipHullInEncyclopedia(std::move(ship_hull_name));
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetShipHull(std::move(ship_hull_name));
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

void MapWnd::ShowSpecial(std::string special_name) {
    if (m_production_wnd->Visible()) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowSpecialInEncyclopedia(std::move(special_name));
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetSpecial(std::move(special_name));
    }
}

void MapWnd::ShowSpecies(std::string species_name) {
    if (m_production_wnd->Visible()) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowSpeciesInEncyclopedia(std::move(species_name));
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetSpecies(std::move(species_name));
    }
}

void MapWnd::ShowFieldType(std::string field_type_name) {
    if (m_production_wnd->Visible()) {
        m_production_wnd->ShowPedia();
        m_production_wnd->ShowFieldTypeInEncyclopedia(std::move(field_type_name));
    } else {
        if (!m_pedia_panel->Visible())
            TogglePedia();
        m_pedia_panel->SetFieldType(std::move(field_type_name));
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

void MapWnd::ShowMeterTypeArticle(std::string meter_string) {
    ShowPedia();
    m_pedia_panel->SetMeterType(std::move(meter_string));
}

void MapWnd::ShowMeterTypeArticle(MeterType meter_type) {
    ShowPedia();
    m_pedia_panel->SetMeterType(meter_type);
}

void MapWnd::ShowEncyclopediaEntry(std::string str) {
    if (!m_pedia_panel->Visible())
        TogglePedia();
    m_pedia_panel->SetEncyclopediaArticle(std::move(str));
}

void MapWnd::CenterOnObject(int id) {
    if (auto obj = GGHumanClientApp::GetApp()->GetContext().ContextObjects().get(id))
        CenterOnMapCoord(obj->X(), obj->Y());
}

void MapWnd::ReselectLastSystem() {
    if (SidePanel::SystemID() != INVALID_OBJECT_ID)
        SelectSystem(SidePanel::SystemID());
}

void MapWnd::SelectSystem(int system_id) {
    ScriptingContext& context = GGHumanClientApp::GetApp()->GetContext();

    auto system = context.ContextObjects().get<System>(system_id);
    if (!system && system_id != INVALID_OBJECT_ID) {
        ErrorLogger() << "MapWnd::SelectSystem couldn't find system with id " << system_id << " so is selected no system instead";
        system_id = INVALID_OBJECT_ID;
    }


    if (system && GetOptionsDB().Get<bool>("ui.map.sidepanel.meter-refresh")) {
        // ensure meter estimates are up to date, particularly for which ship is selected
        context.ContextUniverse().UpdateMeterEstimates(context, true);
    }


    if (SidePanel::SystemID() != system_id) {
        // remove map selection indicator from previously selected system
        if (SidePanel::SystemID() != INVALID_OBJECT_ID) {
            const auto it = m_system_icons.find(SidePanel::SystemID());
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
            const auto it = m_system_icons.find(SidePanel::SystemID());
            if (it != m_system_icons.end())
                it->second->SetSelected(true);
        }
    }

    InitScaleCircleRenderingBuffer(context.ContextObjects());

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
    for (const auto* fleet : objects.findRaw<Fleet>(m_selected_fleet_ids)) {
        if (fleet)
            missing_fleets.insert(fleet->ID());
    }
    for (int fleet_id : missing_fleets)
        m_selected_fleet_ids.erase(fleet_id);


    // select a not-missing fleet, if any
    for (int fleet_id : m_selected_fleet_ids) {
        SelectFleet(fleet_id);
        break;              // abort after first fleet selected... don't need to do more
    }
}

void MapWnd::SelectPlanet(int planetID, const ScriptingContext& context)
{ m_production_wnd->SelectPlanet(planetID, context); }

void MapWnd::SelectPlanet(int planetID)
{ m_production_wnd->SelectPlanet(planetID, GGHumanClientApp::GetApp()->GetContext()); }

void MapWnd::SelectFleet(int fleet_id)
{ SelectFleet(GGHumanClientApp::GetApp()->GetContext().ContextObjects().get<Fleet>(fleet_id)); }

void MapWnd::SelectFleet(const std::shared_ptr<Fleet>& fleet) {
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
        const auto* app = GGHumanClientApp::GetApp();

        // Add any overlapping fleet buttons for moving or offroad fleets.
        auto wnd_fleet_ids = FleetIDsOfFleetButtonsOverlapping(fleet->ID(), app->GetContext(), app->EmpireID());
        if (wnd_fleet_ids.empty())
            wnd_fleet_ids.push_back(fleet->ID());

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
    if (m_selected_fleet_ids.size() == 1 && m_selected_fleet_ids.contains(fleet->ID()))
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
    RefreshFleetButtons(true);
}

void MapWnd::SetFleetMovementLine(int fleet_id) {
    if (fleet_id == INVALID_OBJECT_ID)
        return;

    const ScriptingContext& context = GGHumanClientApp::GetApp()->GetContext();

    auto fleet = context.ContextObjects().get<Fleet>(fleet_id);
    if (!fleet) {
        ErrorLogger() << "MapWnd::SetFleetMovementLine was passed invalid fleet id " << fleet_id;
        return;
    }
    //std::cout << "creating fleet movement line for fleet at (" << fleet->X() << ", " << fleet->Y() << ")" << std::endl;


    // get colour: empire colour, or white if no single empire applicable
    GG::Clr line_colour = GG::CLR_WHITE;
    const auto empire = context.GetEmpire(fleet->Owner());
    if (empire)
        line_colour = empire->Color();
    else if (fleet->Unowned() && fleet->HasMonsters(GetUniverse()))
        line_colour = GG::CLR_RED;

    // create and store line
    auto route(fleet->TravelRoute());
    auto path = fleet->MovePath(route, true, context);
    auto route_it = route.begin();
    if (!route.empty() && (++route_it) != route.end()) {
        //DebugLogger() << "MapWnd::SetFleetMovementLine fleet id " << fleet_id<<" checking for blockade at system "<< route.front() <<
        //    " with m_arrival_lane "<< fleet->ArrivalStarlane()<<" and next destination "<<*route_it;
        if (fleet->SystemID() == route.front() &&
            fleet->BlockadedAtSystem(route.front(), *route_it, context))
        {
            //adjust ETAs if necessary
            //if (!route.empty() && fleet->SystemID()==route.front() && (++(path.begin()))->post_blockade) {
            //DebugLogger() << "MapWnd::SetFleetMovementLine fleet id " << fleet_id<<" blockaded at system "<< route.front() <<
            //    " with m_arrival_lane "<< fleet->ArrivalStarlane()<<" and next destination "<<*route_it;
            if (route_it != route.end() && !( (*route_it == fleet->ArrivalStarlane())  ||
                (empire && empire->PreservedLaneTravel(fleet->SystemID(), *route_it)) ) )
            {
                for (MovePathNode& node : path) {
                    //DebugLogger() <<   "MapWnd::SetFleetMovementLine fleet id " << fleet_id<<" node obj " << node.object_id <<
                    //                            ", node lane end " << node.lane_end_id << ", is post-blockade (" << node.post_blockade << ")";
                    if (node.eta >= 250)
                        node.eta = Fleet::ETA_NEVER;
                    else
                        node.eta++;
                }
            } else {
                //DebugLogger() << "MapWnd::SetFleetMovementLine fleet id " << fleet_id<<" slips through second block check";
            }
        }
    }
    m_fleet_lines.insert_or_assign(
        fleet_id, MovementLineData(path, m_starlane_endpoints, line_colour, fleet->Owner()));
}

void MapWnd::SetProjectedFleetMovementLine(int fleet_id, const std::vector<int>& travel_route) {
    if (fleet_id == INVALID_OBJECT_ID)
        return;

    const ScriptingContext& context = IApp::GetApp()->GetContext();

    // ensure passed fleet exists
    auto fleet = context.ContextObjects().get<Fleet>(fleet_id);
    if (!fleet) {
        ErrorLogger() << "MapWnd::SetProjectedFleetMovementLine was passed invalid fleet id " << fleet_id;
        return;
    }

    //std::cout << "creating projected fleet movement line for fleet at (" << fleet->X() << ", " << fleet->Y() << ")" << std::endl;
    auto empire = context.GetEmpire(fleet->Owner());

    // get move path to show.  if there isn't one, show nothing
    auto path = fleet->MovePath(travel_route, true, context);



    // We need the route to contain the current system
    // even when it is empty to switch between non appending
    // and appending projections on shift changes
    if (path.empty())
        path.emplace_back(fleet->X(), fleet->Y(), true, 0, fleet->SystemID(),
                          INVALID_OBJECT_ID, INVALID_OBJECT_ID, false, false);

    auto route_it = travel_route.begin();
    if (!travel_route.empty() && (++route_it) != travel_route.end()) {
        if (fleet->SystemID() == travel_route.front() &&
            fleet->BlockadedAtSystem(travel_route.front(), *route_it, context))
        {
            //adjust ETAs if necessary
            //if (!route.empty() && fleet->SystemID()==route.front() && (++(path.begin()))->post_blockade) {
            //DebugLogger() << "MapWnd::SetFleetMovementLine fleet id " << fleet_id<<" blockaded at system "<< route.front() <<
            //" with m_arrival_lane "<< fleet->ArrivalStarlane()<<" and next destination "<<*route_it;
            if (route_it != travel_route.end() && !((*route_it == fleet->ArrivalStarlane()) ||
                (empire && empire->PreservedLaneTravel(fleet->SystemID(), *route_it))))
            {
                for (MovePathNode& node : path) {
                    //DebugLogger() <<   "MapWnd::SetFleetMovementLine fleet id " << fleet_id << " node obj " << node.object_id <<
                    //                            ", node lane end " << node.lane_end_id << ", is post-blockade (" << node.post_blockade << ")";
                    if (node.eta >= 250)
                        node.eta = Fleet::ETA_NEVER;
                    else
                        node.eta++;
                }
            }
        }
    }

    // get colour: empire colour, or white if no single empire applicable
    const auto line_colour = empire ? empire->Color() : GG::CLR_WHITE;

    // create and store line
    m_projected_fleet_lines[fleet_id] = MovementLineData(path, m_starlane_endpoints,
                                                         line_colour, fleet->Owner());
}

void MapWnd::SetProjectedFleetMovementLines(const std::vector<int>& fleet_ids,
                                            const std::vector<int>& travel_route)
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
    auto* app = GGHumanClientApp::GetApp();
    ScriptingContext& context = app->GetContext();
    ObjectMap& objects{context.ContextObjects()};
    Universe& universe{context.ContextUniverse()};
    auto obj = objects.get(id);
    if (!obj)
        return;

    // If there is only 1 ship in a fleet, forget the fleet
    const Ship* ship = nullptr;
    if (obj->ObjectType() == UniverseObjectType::OBJ_SHIP) {
        ship = static_cast<const Ship*>(obj.get());
        if (auto ship_s_fleet = objects.get<const Fleet>(ship->FleetID())) {
            bool only_ship_in_fleet = ship_s_fleet->NumShips() == 1;
            if (only_ship_in_fleet)
                return ForgetObject(ship->FleetID());
        }
    }

    const int client_empire_id = app->EmpireID();

    app->Orders().IssueOrder<ForgetOrder>(context, client_empire_id, obj->ID());

    // Client changes for immediate effect
    // Force the client to change immediately.
    universe.ForgetKnownObject(ALL_EMPIRES, obj->ID());

    // Force a refresh
    RequirePreRender();

    // Update fleet wnd if needed
    if (obj->ObjectType() == UniverseObjectType::OBJ_FLEET) {
        auto fleet = static_cast<Fleet*>(obj.get());
        RemoveFleet(fleet->ID());
        fleet->StateChangedSignal();
    }

    if (ship)
        ship->StateChangedSignal();
}

void MapWnd::DoSystemIconsLayout(const ObjectMap& objects) {
    // position and resize system icons and gaseous substance
    const int SYSTEM_ICON_SIZE = SystemIconSize();
    for (const auto& [sys_id, system_icon] : m_system_icons) {
        auto system = objects.get<System>(sys_id);
        if (!system) {
            ErrorLogger() << "MapWnd::DoSystemIconsLayout couldn't get system with id " << sys_id;
            continue;
        }

        GG::Pt icon_ul(GG::X(static_cast<int>(system->X()*ZoomFactor() - SYSTEM_ICON_SIZE / 2.0)),
                       GG::Y(static_cast<int>(system->Y()*ZoomFactor() - SYSTEM_ICON_SIZE / 2.0)));
        system_icon->SizeMove(icon_ul, icon_ul + GG::Pt(GG::X(SYSTEM_ICON_SIZE), GG::Y(SYSTEM_ICON_SIZE)));
    }
}

void MapWnd::DoFieldIconsLayout(const ObjectMap& objects) {
    // position and resize field icons
    const double zoom_factor = ZoomFactor();

    std::for_each(m_field_icons.cbegin(), m_field_icons.cend(),
                  [zoom_factor, &objects](const auto& field_icon)
    {
        auto field = objects.get<Field>(field_icon->FieldID());
        if (!field) {
            ErrorLogger() << "MapWnd::DoFieldIconsLayout couldn't get field with id " << field_icon->FieldID();
        } else {
            double RADIUS = zoom_factor * field->GetMeter(MeterType::METER_SIZE)->Initial();    // Field's MeterType::METER_SIZE gives the radius of the field

            GG::Pt icon_ul(GG::X(static_cast<int>(field->X() * zoom_factor - RADIUS)),
                           GG::Y(static_cast<int>(field->Y() * zoom_factor - RADIUS)));
            field_icon->SizeMove(icon_ul, icon_ul + GG::Pt(GG::X(2 * RADIUS), GG::Y(2 * RADIUS)));
        }
    });
}

void MapWnd::DoFleetButtonsLayout(const ObjectMap& objects) {
    auto place_system_fleet_btn = [this, &objects](int sys_id, const auto& buttons, bool is_departing) {
        // calculate system icon position
        const auto system = objects.get<System>(sys_id);
        if (!system) {
            ErrorLogger() << "MapWnd::DoFleetButtonsLayout couldn't find system with id " << sys_id;
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
        for (auto& button : buttons) {
            GG::Pt ul = system_icon->NthFleetButtonUpperLeft(n, is_departing);
            ++n;
            button->MoveTo(ul + icon_ul);
        }
    };

    // position departing fleet buttons
    for (const auto& [sys_id, buttons] : m_departing_fleet_buttons)
        place_system_fleet_btn(sys_id, buttons, true);

    // position stationary fleet buttons
    for (const auto& [sys_id, buttons] : m_stationary_fleet_buttons)
        place_system_fleet_btn(sys_id, buttons, false);

    // position moving fleet buttons
    for (auto& lane_and_fbs : m_moving_fleet_buttons) {
        for (auto& fb : lane_and_fbs.second) {
            const GG::Pt FLEET_BUTTON_SIZE = fb->Size();
            std::shared_ptr<const Fleet> fleet;

            // skip button if it has no fleets (somehow...?) or if the first fleet in the button is 0
            if (fb->Fleets().empty() || !(fleet = objects.get<Fleet>(fb->Fleets().front()))) {
                ErrorLogger() << "DoFleetButtonsLayout couldn't get first fleet for button";
                continue;
            }

            const auto button_pos = MovingFleetMapPositionOnLane(fleet);
            if (!button_pos)
                continue;

            // position button
            GG::Pt button_ul(GG::ToX(button_pos->first  * ZoomFactor() - FLEET_BUTTON_SIZE.x / 2.0),
                             GG::ToY(button_pos->second * ZoomFactor() - FLEET_BUTTON_SIZE.y / 2.0));

            fb->MoveTo(button_ul);
        }
    }

    // position offroad fleet buttons
    for (auto& [button_pos, buttons] : m_offroad_fleet_buttons) {
        for (auto& fb : buttons) {
            const GG::Pt FLEET_BUTTON_SIZE = fb->Size();
            const auto& fb_fleets = fb->Fleets();

            // skip button if it has no fleets (somehow...?) or if the first fleet in the button is 0
            if (fb_fleets.empty() || !objects.getRaw<Fleet>(fb_fleets.front())) {
                ErrorLogger() << "DoFleetButtonsLayout couldn't get first fleet for button";
                continue;
            }

            // position button
            GG::Pt button_ul(GG::ToX(button_pos.first  * ZoomFactor() - FLEET_BUTTON_SIZE.x / 2.0),
                             GG::ToY(button_pos.second * ZoomFactor() - FLEET_BUTTON_SIZE.y / 2.0));

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
    const int sys1_id = fleet->PreviousSystemID();
    const int sys2_id = fleet->NextSystemID();

    // get apparent positions of endpoints for this lane that have been pre-calculated
    auto endpoints_it = m_starlane_endpoints.find({sys1_id, sys2_id});
    if (endpoints_it == m_starlane_endpoints.end()) {
        // couldn't find an entry for the lane this fleet is one, so just
        // return actual position of fleet on starlane - ignore the distance
        // away from the star centre at which starlane endpoints should appear
        return std::pair(fleet->X(), fleet->Y());
    }

    const ScriptingContext& context = IApp::GetApp()->GetContext();

    // return apparent position of fleet on starlane
    const LaneEndpoints& screen_lane_endpoints = endpoints_it->second;
    return ScreenPosOnStarlane(fleet->X(), fleet->Y(), sys1_id, sys2_id,
                               screen_lane_endpoints, context);
}

namespace {
    template <typename Key>
    using KeyToFleetsMap = std::unordered_map<Key, std::vector<int>, boost::hash<Key>>;
    using SystemXEmpireToFleetsMap = KeyToFleetsMap<std::pair<int, int>>;
    using LocationXEmpireToFleetsMap = KeyToFleetsMap<std::pair<std::pair<double, double>, int>>;
    using StarlaneToFleetsMap = KeyToFleetsMap<std::pair<int, int>>;

    /** Return fleet if \p obj is not destroyed, not stale, a fleet and not empty.*/
    std::shared_ptr<const Fleet> IsQualifiedFleet(auto&& fleet, int empire_id,
                                                  const auto& known_destroyed_objects,
                                                  const auto& stale_object_info)
    {
        static_assert(std::is_same_v<std::decay_t<decltype(*fleet)>, Fleet>);
        if (!fleet || fleet->Empty())
            return nullptr;

        const int fleet_id = fleet->ID();
        if (known_destroyed_objects.contains(fleet_id) || stale_object_info.contains(fleet_id))
            return nullptr;

        return fleet;
    }

    /** If the \p fleet has orders and is departing from a valid system, return the system*/
    std::shared_ptr<const System> IsDepartingFromSystem(const auto& fleet, const ObjectMap& objects) {
        if (fleet->FinalDestinationID() != INVALID_OBJECT_ID
            && !fleet->TravelRoute().empty()
            && fleet->SystemID() != INVALID_OBJECT_ID)
        {
            if (auto system = objects.get<System>(fleet->SystemID()))
                return system;
            ErrorLogger() << "Couldn't get system with id " << fleet->SystemID()
                          << " of a departing fleet named " << fleet->Name();
        }
        return nullptr;
    }

    /** If the \p fleet is stationary in a valid system, return the system*/
    std::shared_ptr<const System> IsStationaryInSystem(const auto& fleet, const ObjectMap& objects) {
        if ((fleet->FinalDestinationID() == INVALID_OBJECT_ID
             || fleet->TravelRoute().empty())
            && fleet->SystemID() != INVALID_OBJECT_ID)
        {
            auto system = objects.get<System>(fleet->SystemID());
            if (system)
                return system;
            ErrorLogger() << "Couldn't get system with id " << fleet->SystemID()
                          << " of a stationary fleet named " << fleet->Name();
        }
        return nullptr;
    }

    /** If the \p fleet has a valid destination and it not at a system, return true*/
    bool IsMoving(const auto& fleet) {
        return (fleet->FinalDestinationID() != INVALID_OBJECT_ID
                && fleet->SystemID() == INVALID_OBJECT_ID);
    }

    /** Return the starlane's endpoints if the \p fleet is on a starlane. */
    boost::optional<std::pair<int, int>> IsOnStarlane(const std::shared_ptr<const Fleet>& fleet,
                                                      const ObjectMap& objects)
    {
        // get endpoints of lane on screen
        const int sys1_id = fleet->PreviousSystemID();
        const int sys2_id = fleet->NextSystemID();

        const auto sys1 = objects.get<System>(sys1_id);
        if (sys1 && sys1->HasStarlaneTo(sys2_id))
            return std::pair(std::min(sys1_id, sys2_id), std::max(sys1_id, sys2_id));

        return boost::none;
    }

    /** If the \p fleet has a valid destination, is not at a system and is
        on a starlane, return the starlane's endpoint system ids */
    boost::optional<std::pair<int, int>> IsMovingOnStarlane(const auto& fleet, const ObjectMap& objects) {
        if (!IsMoving(fleet))
            return boost::none;

        return IsOnStarlane(fleet, objects);
    }

    /** If the \p fleet has a valid destination and it not on a starlane, return true*/
    bool IsOffRoad(const auto& fleet, const ObjectMap& objects)
    { return (fleet->SystemID() == INVALID_OBJECT_ID && !IsOnStarlane(fleet, objects)); }
}

void MapWnd::RefreshFleetButtons(bool recreate) {
    RequirePreRender();
    m_deferred_refresh_fleet_buttons |= !recreate;
    m_deferred_recreate_fleet_buttons |= recreate;
}

void MapWnd::DeferredRefreshFleetButtons() {
    const auto* app = GGHumanClientApp::GetApp();
    const auto& context = app->GetContext();
    const auto& objects = context.ContextObjects();

    if (!m_deferred_recreate_fleet_buttons && !m_deferred_refresh_fleet_buttons) {
        DoFleetButtonsLayout(objects);
        return;

    } else if (!m_deferred_recreate_fleet_buttons) {
        // just do refresh
        m_deferred_refresh_fleet_buttons = false;
        ScopedTimer timer("RefreshFleetButtons( just refresh )", true);

        const auto sz = FleetButtonSizeType();
        for (auto& fleet_button : m_fleet_buttons | range_values)
            fleet_button->Refresh(sz);

        DoFleetButtonsLayout(objects);
        return;

    } // else do recreate

    m_deferred_recreate_fleet_buttons = false;
    m_deferred_refresh_fleet_buttons = false;

    ScopedTimer timer("RefreshFleetButtons( full recreate )", true);

    // determine fleets that need buttons so that fleets at the same location can
    // be grouped by empire owner and buttons created


    const auto client_empire_id = app->EmpireID();
    const auto& universe = context.ContextUniverse();

    const auto& this_client_known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(client_empire_id);
    const auto& this_client_stale_object_info = universe.EmpireStaleKnowledgeObjectIDs(client_empire_id);

    SystemXEmpireToFleetsMap   departing_fleets;
    SystemXEmpireToFleetsMap   stationary_fleets;
    m_moving_fleets.clear();
    LocationXEmpireToFleetsMap moving_fleets;
    LocationXEmpireToFleetsMap offroad_fleets;

    for (auto& [fleet_id, cfleet] : objects.allExisting<Fleet>()) {
        auto fleet = IsQualifiedFleet(cfleet, client_empire_id,
                                      this_client_known_destroyed_objects,
                                      this_client_stale_object_info);
        if (!fleet)
            continue;

        // Collect fleets with a travel route just departing.
        if (auto departure_system = IsDepartingFromSystem(fleet, objects)) {
            departing_fleets[{departure_system->ID(), fleet->Owner()}].push_back(fleet->ID());

            // Collect stationary fleets by system.
        } else if (auto stationary_system = IsStationaryInSystem(fleet, objects)) {
            // DebugLogger() << fleet->Name() << " is Stationary." ;
            stationary_fleets[{stationary_system->ID(), fleet->Owner()}].push_back(fleet->ID());

            // Collect traveling fleets between systems by which starlane they
            // are on (ignoring location on lane and direction of travel)
        } else if (auto starlane_end_systems = IsMovingOnStarlane(fleet, objects)) {
            moving_fleets[{*starlane_end_systems, fleet->Owner()}].push_back(fleet->ID());
            m_moving_fleets[*starlane_end_systems].push_back(fleet->ID());

        } else if (IsOffRoad(fleet, objects)) {
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
    CreateFleetButtonsOfType(m_departing_fleet_buttons,  departing_fleets,  FLEETBUTTON_SIZE, objects);
    CreateFleetButtonsOfType(m_stationary_fleet_buttons, stationary_fleets, FLEETBUTTON_SIZE, objects);
    CreateFleetButtonsOfType(m_moving_fleet_buttons,     moving_fleets,     FLEETBUTTON_SIZE, objects);
    CreateFleetButtonsOfType(m_offroad_fleet_buttons,    offroad_fleets,    FLEETBUTTON_SIZE, objects);

    // position fleetbuttons
    DoFleetButtonsLayout(objects);

    // add selection indicators to fleetbuttons
    RefreshFleetButtonSelectionIndicators();

    // create movement lines (after positioning buttons, so lines will originate from button location)
    for (const auto& fleet_button_id : m_fleet_buttons | range_keys)
        SetFleetMovementLine(fleet_button_id);
}

template <typename FleetButtonMap, typename FleetsMap>
void MapWnd::CreateFleetButtonsOfType(FleetButtonMap& type_fleet_buttons,
                                      const FleetsMap& fleets_map,
                                      const FleetButton::SizeType& fleet_button_size,
                                      const ObjectMap& objects)
{
    for (const auto& [system_empire, fleet_IDs] : fleets_map) {
        if (fleet_IDs.empty())
            continue;
        const auto key = static_cast<typename FleetButtonMap::key_type>(system_empire.first);

        // sort fleets by position
        std::map<std::pair<double, double>, std::vector<int>> fleet_positions_ids;
        for (const auto* fleet : objects.findRaw<Fleet>(fleet_IDs)) {
            if (fleet)
                fleet_positions_ids[{fleet->X(), fleet->Y()}].emplace_back(fleet->ID());
        }

        // create separate FleetButton for each cluster of fleets
        for (auto& ids_in_cluster : fleet_positions_ids | range_values) {
            // create new fleetbutton for this cluster of fleets
            auto fb = GG::Wnd::Create<FleetButton>(std::move(ids_in_cluster), fleet_button_size);

            // store per type of fleet button.
            type_fleet_buttons[key].insert(fb);

            // store FleetButton for fleets in current cluster
            for (int fleet_id : fb->Fleets())
                m_fleet_buttons[fleet_id] = fb;

            fb->LeftClickedSignal.connect(boost::bind(&MapWnd::FleetButtonLeftClicked, this, fb.get()));
            fb->RightClickedSignal.connect(boost::bind(&MapWnd::FleetButtonRightClicked, this, fb.get()));
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
    static constexpr int    SYSTEM_NAME_MINIMUM_PTS = 6;    // limit to absolute minimum point size
    static constexpr double MAX_NAME_ZOOM_FACTOR = 1.5;     // limit to relative max above standard UI font size
    const double            NAME_ZOOM_FACTOR = std::min(MAX_NAME_ZOOM_FACTOR, ZoomFactor());
    const int               ZOOMED_PTS = static_cast<int>(ClientUI::Pts() * NAME_ZOOM_FACTOR);
    return std::max(ZOOMED_PTS, SYSTEM_NAME_MINIMUM_PTS);
}

double MapWnd::SystemHaloScaleFactor() const
{ return 1.0 + log10(ZoomFactor()); }

FleetButton::SizeType MapWnd::FleetButtonSizeType() const {
    // no SizeType::LARGE as these icons are too big for the map.  (they can be used in the FleetWnd, however)
    if      (ZoomFactor() > ClientUI::BigFleetButtonZoomThreshold())
        return FleetButton::SizeType::LARGE;

    else if (ZoomFactor() > ClientUI::MediumFleetButtonZoomThreshold())
        return FleetButton::SizeType::MEDIUM;

    else if (ZoomFactor() > ClientUI::SmallFleetButtonZoomThreshold())
        return FleetButton::SizeType::SMALL;

    else if (ZoomFactor() > ClientUI::TinyFleetButtonZoomThreshold())
        return FleetButton::SizeType::TINY;

    else
        return FleetButton::SizeType::NONE;
}

void MapWnd::Zoom(int delta) {
    GG::Pt center = GG::Pt(AppWidth()/2, AppHeight()/2);
    Zoom(delta, center);
}

void MapWnd::Zoom(int delta, const GG::Pt position) {
    if (delta == 0)
        return;

    // increment zoom steps in by delta steps
    double new_zoom_steps_in = m_zoom_steps_in + static_cast<double>(delta);
    SetZoom(new_zoom_steps_in, true, position);
}

void MapWnd::SetZoom(double steps_in, bool update_slide) {
    GG::Pt center = GG::Pt(AppWidth()/2, AppHeight()/2);
    SetZoom(steps_in, update_slide, center);
}

void MapWnd::SetZoom(double steps_in, bool update_slide, const GG::Pt position) {
    if (GetOptionsDB().Get<bool>("ui.map.lock"))
        return;

    ScopedTimer timer("MapWnd::SetZoom(steps_in=" + std::to_string(steps_in) +
                      ", update_slide=" + std::to_string(update_slide) +
                      ", position=" + std::string{position}, true);
    // impose range limits on zoom steps
    double new_steps_in = std::max(std::min(steps_in, ZOOM_IN_MAX_STEPS), ZOOM_IN_MIN_STEPS);

    // abort if no change
    if (new_steps_in == m_zoom_steps_in)
        return;


    // save position offsets and old zoom factors
    GG::Pt                      ul =                    ClientUpperLeft();
    GG::X                       center_x =              GG::ToX(AppWidth() / 2.0);
    GG::Y                       center_y =              GG::ToY(AppHeight() / 2.0);
    GG::X                       ul_offset_x =           ul.x - center_x;
    GG::Y                       ul_offset_y =           ul.y - center_y;
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
    const auto NEW_ZOOM = ZoomFactor();
    ul_offset_x *= (NEW_ZOOM / OLD_ZOOM);
    ul_offset_y *= (NEW_ZOOM / OLD_ZOOM);

    // now add the zoom position offset at the new zoom level
    ul_offset_x += position_center_delta.x;
    ul_offset_y += position_center_delta.y;

    // show or hide system names, depending on zoom.  replicates code in MapWnd::Zoom
    if (ZoomFactor() * ClientUI::Pts() < MIN_SYSTEM_NAME_SIZE)
        HideSystemNames();
    else
        ShowSystemNames();

    const auto& objects = GGHumanClientApp::GetApp()->GetContext().ContextObjects();


    DoSystemIconsLayout(objects);
    DoFieldIconsLayout(objects);


    // if fleet buttons need to change size, need to fully refresh them (clear
    // and recreate).  If they are the same size as before the zoom, then can
    // just reposition them without recreating
    const FleetButton::SizeType NEW_FLEETBUTTON_SIZE = FleetButtonSizeType();
    if (OLD_FLEETBUTTON_SIZE != NEW_FLEETBUTTON_SIZE)
        RefreshFleetButtons(false);
    else
        DoFleetButtonsLayout(objects);


    // move field icons to bottom of child stack so that other icons can be moused over with a field
    for (const auto& field_icon : m_field_icons)
        MoveChildDown(field_icon);


    // translate map and UI widgets to account for the change in upper left due to zooming
    GG::Pt map_move(center_x + ul_offset_x - ul.x, center_y + ul_offset_y - ul.y);
    OffsetMove(map_move);

    // this correction ensures that zooming in doesn't leave too large a margin to the side
    GG::Pt move_to_pt = ul = ClientUpperLeft();
    CorrectMapPosition(move_to_pt);

    MoveTo(move_to_pt - GG::Pt(AppWidth(), AppHeight()));

    if (m_scale_line)
        m_scale_line->Update(NEW_ZOOM, m_selected_fleet_ids, SidePanel::SystemID(), objects);
    if (update_slide && m_zoom_slider)
        m_zoom_slider->SlideTo(m_zoom_steps_in);

    InitScaleCircleRenderingBuffer(objects);

    ZoomedSignal(NEW_ZOOM);
}

void MapWnd::CorrectMapPosition(GG::Pt& move_to_pt) {
    GG::X contents_width(GG::ToX(ZoomFactor() * GetUniverse().UniverseWidth()));
    GG::X app_width =  AppWidth();
    GG::Y app_height = AppHeight();
    GG::X map_margin_width(app_width);

    //std::cout << "MapWnd::CorrectMapPosition appwidth: " << Value(app_width) << " appheight: " << Value(app_height)
    //          << " to_x: " << Value(move_to_pt.x) << " to_y: " << Value(move_to_pt.y) << std::endl;

    // restrict map positions to prevent map from being dragged too far off screen.
    // add extra padding to restrictions when universe to be shown is larger than
    // the screen area in which to show it.
    if (app_width - map_margin_width < contents_width || GG::X{Value(app_height)} - map_margin_width < contents_width) {
        if (map_margin_width < move_to_pt.x)
            move_to_pt.x = map_margin_width;
        if (move_to_pt.x + contents_width < app_width - map_margin_width)
            move_to_pt.x = app_width - map_margin_width - contents_width;
        if (Value(map_margin_width) < Value(move_to_pt.y))
            move_to_pt.y = GG::Y{Value(map_margin_width)};
        if (Value(move_to_pt.y) + contents_width < GG::X{Value(app_height)} - map_margin_width)
            move_to_pt.y = app_height - Value(map_margin_width) - Value(contents_width);
    } else {
        if (move_to_pt.x < GG::X0)
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
        ClientNetworking& net = GGHumanClientApp::GetApp()->Networking();

        if (mas == ModeratorActionSetting::MAS_Destroy)
            net.SendMessage(ModeratorActionMessage(Moderator::DestroyUniverseObject(field_id)));
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
    const auto& objects = GGHumanClientApp::GetApp()->GetContext().ContextObjects();

    if (ClientPlayerIsModerator()) {
        ModeratorActionSetting mas = m_moderator_wnd->SelectedAction();
        ClientNetworking& net = GGHumanClientApp::GetApp()->Networking();

        if (mas == ModeratorActionSetting::MAS_Destroy) {
            net.SendMessage(ModeratorActionMessage(
                Moderator::DestroyUniverseObject(system_id)));

        } else if (mas == ModeratorActionSetting::MAS_CreatePlanet) {
            net.SendMessage(ModeratorActionMessage(
                Moderator::CreatePlanet(system_id, m_moderator_wnd->SelectedPlanetType(),
                                        m_moderator_wnd->SelectedPlanetSize())));

        } else if (mas == ModeratorActionSetting::MAS_AddStarlane) {
            int selected_system_id = SidePanel::SystemID();
            if (objects.getRaw<System>(selected_system_id)) {
                net.SendMessage(ModeratorActionMessage(
                    Moderator::AddStarlane(system_id, selected_system_id)));
            }

        } else if (mas == ModeratorActionSetting::MAS_RemoveStarlane) {
            int selected_system_id = SidePanel::SystemID();
            if (objects.getRaw<System>(selected_system_id)) {
                net.SendMessage(ModeratorActionMessage(
                    Moderator::RemoveStarlane(system_id, selected_system_id)));
            }
        } else if (mas == ModeratorActionSetting::MAS_SetOwner) {
            auto system = objects.get<System>(system_id);
            if (!system)
                return;

            int empire_id = m_moderator_wnd->SelectedEmpire();
            for (auto* obj : objects.findRaw<const UniverseObject>(system->ContainedObjectIDs())) {
                UniverseObjectType obj_type = obj->ObjectType();
                if (obj_type >= UniverseObjectType::OBJ_BUILDING &&
                    obj_type < UniverseObjectType::OBJ_SYSTEM)
                { net.SendMessage(ModeratorActionMessage(Moderator::SetOwner(obj->ID(), empire_id))); }
            }
        }
    }

    if (!m_in_production_view_mode && FleetUIManager::GetFleetUIManager().ActiveFleetWnd()) {
        if (system_id == INVALID_OBJECT_ID)
            ClearProjectedFleetMovementLines();
        else
            PlotFleetMovement(system_id, !m_ready_turn, mod_keys & GG::MOD_KEY_SHIFT); // TODO: pass in context
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

    const ScriptingContext& context = IApp::GetApp()->GetContext();

    // retrieve system_id from planet_id
    auto planet = context.ContextObjects().get<Planet>(planet_id);
    if (!planet)
        return;

    // open production screen
    if (!m_in_production_view_mode) {
        if (!m_production_wnd->Visible())
            ToggleProduction();
        CenterOnObject(planet->SystemID());
        m_production_wnd->SelectSystem(planet->SystemID());
        m_production_wnd->SelectPlanet(planet_id, context);
    }
}

void MapWnd::PlanetRightClicked(int planet_id) {
    if (planet_id == INVALID_OBJECT_ID)
        return;
    if (!ClientPlayerIsModerator())
        return;

    ModeratorActionSetting mas = m_moderator_wnd->SelectedAction();
    ClientNetworking& net = GGHumanClientApp::GetApp()->Networking();

    if (mas == ModeratorActionSetting::MAS_Destroy) {
        net.SendMessage(ModeratorActionMessage(
            Moderator::DestroyUniverseObject(planet_id)));
    } else if (mas == ModeratorActionSetting::MAS_SetOwner) {
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
    ClientNetworking& net = GGHumanClientApp::GetApp()->Networking();

    if (mas == ModeratorActionSetting::MAS_Destroy) {
        net.SendMessage(ModeratorActionMessage(
            Moderator::DestroyUniverseObject(building_id)));
    } else if (mas == ModeratorActionSetting::MAS_SetOwner) {
        int empire_id = m_moderator_wnd->SelectedEmpire();
        net.SendMessage(ModeratorActionMessage(
            Moderator::SetOwner(building_id, empire_id)));
    }
}

void MapWnd::ReplotProjectedFleetMovement(bool append) {
    TraceLogger() << "MapWnd::ReplotProjectedFleetMovement" << (append?" append":"");
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

    if (execute_move || append)
        DebugLogger() << "PlotFleetMovement " << (execute_move?" execute":"") << (append?" append":"");
    else
        TraceLogger() << "PlotfleetMovement";

    auto* app = GGHumanClientApp::GetApp();
    int empire_id = app->EmpireID();
    auto fleet_ids = FleetUIManager::GetFleetUIManager().ActiveFleetWnd()->SelectedFleetIDs();
    ScriptingContext& context = app->GetContext();
    ObjectMap& objects{context.ContextObjects()};
    const Universe& universe{context.ContextUniverse()};

    // apply to all selected this-player-owned fleets in currently-active FleetWnd
    for (const auto* fleet : objects.findRaw<Fleet>(fleet_ids)) {
        if (!fleet)
            continue;

        // only give orders / plot prospective move paths of fleets owned by player
        if (!(fleet->OwnedBy(empire_id)) || !(fleet->NumShips()))
            continue;

        // plot empty move pathes if destination is not a known system
        if (system_id == INVALID_OBJECT_ID) {
            m_projected_fleet_lines.erase(fleet->ID());
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
        auto route = universe.GetPathfinder().ShortestPath(start_system, system_id, objects).first;
        // Prepend a non-empty old_route to the beginning of route.
        if (append && !fleet->TravelRoute().empty()) {
            auto old_route(fleet->TravelRoute());
            old_route.erase(--old_route.end()); //end of old is begin of new
            route.insert(route.begin(), old_route.begin(), old_route.end()); // route.splice(route.begin(), old_route);
        }

        // disallow "offroad" (direct non-starlane non-wormhole) travel
        if (route.size() == 2 && route.front() != route.back()) {
            int begin_id = route.front();
            auto begin_sys = objects.getRaw<System>(begin_id);
            int end_id = route.back();
            auto end_sys = objects.getRaw<System>(end_id);

            if (!begin_sys || !end_sys || (!begin_sys->HasStarlaneTo(end_id) && !end_sys->HasStarlaneTo(begin_id)))
                continue;
        }

        // if actually ordering fleet movement, not just prospectively previewing, ... do so
        if (execute_move && !route.empty()){
            app->Orders().IssueOrder<FleetMoveOrder>(context, empire_id, fleet->ID(), system_id, append);
            StopFleetExploring(fleet->ID(), objects);
        }

        // show route on map
        SetProjectedFleetMovementLine(fleet->ID(), route);
    }
}

std::vector<int> MapWnd::FleetIDsOfFleetButtonsOverlapping(int fleet_id,
                                                           const ScriptingContext& context,
                                                           int empire_id) const
{
    std::vector<int> fleet_ids;

    const auto& objects = context.ContextObjects();
    const auto& universe = context.ContextUniverse();

    const auto fleet = objects.get<Fleet>(fleet_id);
    if (!fleet) {
        ErrorLogger() << "MapWnd::FleetIDsOfFleetButtonsOverlapping: Fleet id "
                      << fleet_id << " does not exist.";
        return fleet_ids;
    }

    const auto it = m_fleet_buttons.find(fleet_id);
    if (it == m_fleet_buttons.end()) {
        // Log that a FleetButton could not be found for the requested fleet, and include when the fleet was last seen
        const auto& vis_turn_map = universe.GetObjectVisibilityTurnMapByEmpire(fleet_id, empire_id);
        const auto vis_it = vis_turn_map.find(Visibility::VIS_BASIC_VISIBILITY);
        const int vis_turn = (vis_it != vis_turn_map.end()) ? vis_it->second : -1;
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

        return retval;
    };

    // There are 4 types of fleet buttons: moving on a starlane, offroad,
    // and stationary or departing from a system.

    // Moving fleet buttons only overlap with fleet buttons on the same starlane
    if (const auto starlane_end_systems = IsMovingOnStarlane(fleet, objects)) {
        const auto& lane_btns_it = m_moving_fleet_buttons.find(*starlane_end_systems);
        if (lane_btns_it == m_moving_fleet_buttons.end())
            return fleet_ids;

        // Add all fleets for each overlapping button on the starlane
        for (const auto& test_fb : lane_btns_it->second)
            if (overlaps_fleet_btn(*test_fb))
                std::copy(test_fb->Fleets().begin(), test_fb->Fleets().end(),
                          std::back_inserter(fleet_ids));

        return fleet_ids;
    }

    // Offroad fleet buttons only overlap other offroad fleet buttons.
    if (IsOffRoad(fleet, objects)) {
        // This scales poorly (linearly) with increasing universe size if
        // offroading is common.
        for (const auto& fbs: m_offroad_fleet_buttons | range_values) {
            if (fbs.empty())
                continue;

            // Since all buttons are at the same position, only check if the first
            // button overlaps fleet_btn
            if (!overlaps_fleet_btn(**fbs.begin()))
                continue;

            // Add all fleets for all fleet buttons to btn_fleet
            for (const auto& overlapped_fb : fbs)
                fleet_ids.insert(fleet_ids.end(), overlapped_fb->Fleets().begin(),
                                 overlapped_fb->Fleets().end());
        }

        return fleet_ids;
    }

    // Stationary and departing fleet buttons should not overlap with each
    // other because of their offset placement for each empire.
    return fleet_btn->Fleets();
}

std::vector<int> MapWnd::FleetIDsOfFleetButtonsOverlapping(const FleetButton& fleet_btn,
                                                           const ScriptingContext& context,
                                                           int empire_id) const
{
    // get possible fleets to select from, and a pointer to one of those fleets
    if (fleet_btn.Fleets().empty()) {
        ErrorLogger() << "Clicked FleetButton contained no fleets!";
        return std::vector<int>();
    }

    // Add any overlapping fleet buttons for moving or offroad fleets.
    const auto overlapped_fleets = FleetIDsOfFleetButtonsOverlapping(fleet_btn.Fleets()[0], context, empire_id);

    if (overlapped_fleets.empty())
        ErrorLogger() << "Clicked FleetButton and overlapping buttons contained no fleets!";
    return overlapped_fleets;
}

void MapWnd::FleetButtonLeftClicked(const FleetButton* fleet_btn) {
    if (!fleet_btn)
        return;

    // allow switching to fleetView even when in production mode
    if (m_in_production_view_mode) {
        HideProduction();
        RestoreSidePanel();
    }

    const auto* app = GGHumanClientApp::GetApp();
    const auto empire_id = app->EmpireID();
    const auto& context = app->GetContext();

    // Add any overlapping fleet buttons for moving or offroad fleets.
    const auto fleet_ids_to_include_in_fleet_wnd =
        FleetIDsOfFleetButtonsOverlapping(*fleet_btn, context, empire_id);
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
        SelectFleet(fleet_to_select_id); // TODO: pass context or objects
}

void MapWnd::FleetButtonRightClicked(const FleetButton* fleet_btn) {
    if (!fleet_btn)
        return;

    const auto* app = GGHumanClientApp::GetApp();
    const auto empire_id = app->EmpireID();
    const auto& context = app->GetContext();
    const auto& objects = context.ContextObjects();
    const auto& universe = context.ContextUniverse();

    // Add any overlapping fleet buttons for moving or offroad fleets.
    const auto fleet_ids = FleetIDsOfFleetButtonsOverlapping(*fleet_btn, context, empire_id);
    if (fleet_ids.empty())
        return;

    // if fleetbutton holds currently not visible fleets, offer to dismiss them
    std::vector<int> sensor_ghosts;

    // find sensor ghosts
    for (const auto* fleet : objects.findRaw<Fleet>(fleet_ids)) {
        if (empire_id == ALL_EMPIRES || !fleet || fleet->OwnedBy(empire_id))
            continue;
        if (universe.GetObjectVisibilityByEmpire(fleet->ID(), empire_id) >= Visibility::VIS_BASIC_VISIBILITY)
            continue;
        sensor_ghosts.push_back(fleet->ID());
    }

    // should there be sensor ghosts, offer to dismiss them
    if (sensor_ghosts.size() > 0) {
        auto popup = GG::Wnd::Create<CUIPopupMenu>(fleet_btn->LowerRight().x, fleet_btn->LowerRight().y);

        auto forget_fleet_actions = [this, sensor_ghosts]() {
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
    std::vector<int> fleet_ids{fleet_id};
    FleetsRightClicked(fleet_ids);
}

void MapWnd::FleetsRightClicked(const std::vector<int>& fleet_ids) {
    if (fleet_ids.empty())
        return;
    if (!ClientPlayerIsModerator())
        return;

    ModeratorActionSetting mas = m_moderator_wnd->SelectedAction();
    ClientNetworking& net = GGHumanClientApp::GetApp()->Networking();

    if (mas == ModeratorActionSetting::MAS_Destroy) {
        for (int fleet_id : fleet_ids) {
            net.SendMessage(ModeratorActionMessage(
                Moderator::DestroyUniverseObject(fleet_id)));
        }
    } else if (mas == ModeratorActionSetting::MAS_SetOwner) {
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
    ClientNetworking& net = GGHumanClientApp::GetApp()->Networking();

    if (mas == ModeratorActionSetting::MAS_Destroy) {
        for (int ship_id : ship_ids) {
            net.SendMessage(ModeratorActionMessage(
                Moderator::DestroyUniverseObject(ship_id)));
        }
    } else if (mas == ModeratorActionSetting::MAS_SetOwner) {
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

    for (auto& offroad_fleet_button : m_offroad_fleet_buttons) {
        for (auto& button : offroad_fleet_button.second)
            button->SetSelected(false);
    }

    // add new selection indicators
    for (int fleet_id : m_selected_fleet_ids) {
        const auto& button_it = m_fleet_buttons.find(fleet_id);
        if (button_it != m_fleet_buttons.end())
            button_it->second->SetSelected(true);
    }
}

void MapWnd::UniverseObjectDeleted(const std::shared_ptr<const UniverseObject>& obj) {
    if (obj)
        DebugLogger() << "MapWnd::UniverseObjectDeleted: " << obj->ID();
    else
        DebugLogger() << "MapWnd::UniverseObjectDeleted: NO OBJECT";
    if (obj && obj->ObjectType() == UniverseObjectType::OBJ_FLEET)
        RemoveFleet(obj->ID());
}

void MapWnd::RegisterPopup(std::shared_ptr<MapWndPopup>&& popup) {
    if (popup)
        m_popups.emplace_back(std::move(popup));
}

void MapWnd::RemovePopup(MapWndPopup* popup) {
    if (!popup)
        return;

    auto it = std::find_if(m_popups.begin(), m_popups.end(),
                           [popup](const auto& xx){ return xx.lock().get() == popup;});
    if (it != m_popups.end())
        m_popups.erase(it);
}

void MapWnd::ResetEmpireShown() {
    const auto* app = GGHumanClientApp::GetApp();
    const auto empire_id = app->EmpireID();
    const ScriptingContext& context = app->GetContext();
    m_production_wnd->SetEmpireShown(empire_id, context);
    m_research_wnd->SetEmpireShown(empire_id, context);
    // TODO: Design?
}

void MapWnd::Sanitize() {
    ShowAllPopups(); // make sure popups don't save visible = 0 to the config
    CloseAllPopups();
    HideResearch();
    HideProduction();
    HideDesign();
    HideGovernment();
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

    const ScriptingContext& context = IApp::GetApp()->GetContext();

    m_research_wnd->Sanitize();
    m_production_wnd->Sanitize(context.ContextObjects());
    m_design_wnd->Sanitize();
    m_government_wnd->Sanitize();

    m_selected_fleet_ids.clear();
    m_selected_ship_ids.clear();

    m_starlane_endpoints.clear();

    DeleteFleetButtons();

    m_fleet_state_change_signals.clear(); // should disconnect scoped signals
    m_system_fleet_insert_remove_signals.clear(); // should disconnect scoped signals
    m_fleet_lines.clear();
    m_projected_fleet_lines.clear();
    m_system_icons.clear();
    m_fleets_exploring.clear();

    DetachChildren();
}

void MapWnd::ResetTimeoutClock(int timeout) {
    m_timeout_time = timeout <= 0 ?
                     std::chrono::time_point<std::chrono::high_resolution_clock>() :
                     std::chrono::high_resolution_clock::now() + std::chrono::high_resolution_clock::duration(std::chrono::seconds(timeout));

    TimerFiring(0, &m_timeout_clock);
}

void MapWnd::TimerFiring(unsigned int ticks, GG::Timer* timer) {
    const auto remaining = m_timeout_time - std::chrono::high_resolution_clock::now();
    const auto remaining_sec = std::chrono::duration_cast<std::chrono::seconds>(remaining);
    if (remaining_sec.count() <= 0) {
        m_timeout_clock.Stop();
        m_timeout_remain->SetText("");
        return;
    }

    int sec_part = remaining_sec.count() % 60;
    int min_part = remaining_sec.count() / 60 % 60;
    int hour_part = remaining_sec.count() / 3600;

    if (hour_part == 0) {
        if (min_part == 0) {
            m_timeout_remain->SetText(boost::io::str(FlexibleFormat(UserString("MAP_TIMEOUT_SECONDS")) % sec_part));
        } else {
            m_timeout_remain->SetText(boost::io::str(FlexibleFormat(UserString("MAP_TIMEOUT_MINS_SECS")) % min_part % sec_part));
        }
    } else {
         m_timeout_remain->SetText(boost::io::str(FlexibleFormat(UserString("MAP_TIMEOUT_HRS_MINS")) % hour_part % min_part));
    }

    if (!m_timeout_clock.Running()) {
        m_timeout_clock.Reset(GG::GUI::GetGUI()->Ticks());
        m_timeout_clock.Start();
    }
}

void MapWnd::PushWndStack(std::shared_ptr<GG::Wnd> wnd) {
    if (!wnd)
        return;
    // First remove it from its current location in the stack (if any), to prevent it from being
    // present in two locations at once.
    RemoveFromWndStack(wnd);
    m_wnd_stack.push_back(std::move(wnd));
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
        ToggleResearch(IApp::GetApp()->GetContext());
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
    } else if (wnd == m_government_wnd) {
        HideGovernment();
    } else {
        ErrorLogger() << "Unknown GG::Wnd " << wnd->Name() << " found in MapWnd::m_wnd_stack";
    }

    return true;
}

bool MapWnd::RevertOrders() {
    ClientUI* cui = ClientUI::GetClientUI();
    if (!cui) {
        ErrorLogger() << "MapWnd::RevertOrders: No client UI available";
        return false;
    }
    GGHumanClientApp::GetApp()->RevertOrders();
    return true;
}

bool MapWnd::EndTurn() {
    if (m_ready_turn) {
        GGHumanClientApp::GetApp()->UnreadyTurn();
    } else {
        ClientUI* cui = ClientUI::GetClientUI();
        if (!cui) {
            ErrorLogger() << "MapWnd::EndTurn: No client UI available";
            return false;
        }
        SaveGameUIData ui_data;
        cui->GetSaveGameUIData(ui_data);
        GGHumanClientApp::GetApp()->StartTurn(ui_data);
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
    HideGovernment();
    RestoreSidePanel();

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
    if (!m_moderator_wnd->Visible() || m_production_wnd->Visible() ||
        m_research_wnd->Visible() || m_design_wnd->Visible())
    {
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
    RestoreSidePanel();

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
    if (!m_object_list_wnd->Visible() || m_production_wnd->Visible() ||
        m_research_wnd->Visible() || m_design_wnd->Visible())
    {
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
    RestoreSidePanel();

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
    if (!m_sitrep_panel->Visible() || m_production_wnd->Visible() ||
        m_research_wnd->Visible() || m_design_wnd->Visible())
    {
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
    RestoreSidePanel();

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
    if (!msg_wnd->Visible() || m_production_wnd->Visible() ||
        m_research_wnd->Visible() || m_design_wnd->Visible())
    {
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
    RestoreSidePanel();

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
    if (!plr_wnd->Visible() || m_production_wnd->Visible() ||
        m_research_wnd->Visible() || m_design_wnd->Visible())
    {
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
    // same for research
    if (m_research_wnd->Visible()) {
        m_research_wnd->TogglePedia();
        return;
    }
    // design screen already has a pedia in it...
    if (m_design_wnd->Visible())
        return;

    ClearProjectedFleetMovementLines();

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
    if (!m_pedia_panel->Visible() || m_production_wnd->Visible() ||
        m_research_wnd->Visible() || m_design_wnd->Visible())
    {
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

void MapWnd::HideSidePanelAndRememberIfItWasVisible() {
    // a kludge, so the sidepanel will reappear after opening and closing a full screen wnd
    m_sidepanel_open_before_showing_other = m_side_panel->Visible();
    m_side_panel->Hide();
}

void MapWnd::RestoreSidePanel() {
    if (m_sidepanel_open_before_showing_other)
        ReselectLastSystem();
}

void MapWnd::ShowResearch(const ScriptingContext& context) {
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideProduction();
    HideDesign();
    HideSidePanelAndRememberIfItWasVisible();

    // show the research window
    m_research_wnd->Show();
    GG::GUI::GetGUI()->MoveUp(m_research_wnd);
    PushWndStack(m_research_wnd);

    m_research_wnd->Update(context);

    // hide pedia again if it is supposed to be hidden persistently
    if (GetOptionsDB().Get<bool>("ui.research.pedia.hidden.enabled"))
        m_research_wnd->HidePedia();

    // indicate selection on button
    m_btn_research->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "research_mouseover.png")));
    m_btn_research->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "research.png")));
}

void MapWnd::HideResearch() {
    GGHumanClientApp::GetApp()->SendPartialOrders();

    m_research_wnd->Hide();
    RemoveFromWndStack(m_research_wnd);
    m_btn_research->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "research.png")));
    m_btn_research->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "research_mouseover.png")));
}

bool MapWnd::ToggleResearch(const ScriptingContext& context) {
    if (m_research_wnd->Visible()) {
        HideResearch();
        RestoreSidePanel();
    } else {
        ShowResearch(context);
    }
    return true;
}

void MapWnd::ShowProduction() {
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideResearch();
    HideDesign();
    HideSidePanelAndRememberIfItWasVisible();
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

    const ScriptingContext& context = IApp::GetApp()->GetContext();

    // if no system is currently shown in sidepanel, default to this empire's
    // home system (ie. where the capital is)
    if (SidePanel::SystemID() == INVALID_OBJECT_ID) {
        if (auto empire = context.GetEmpire(GGHumanClientApp::GetApp()->EmpireID()))
            if (auto obj = context.ContextObjects().get(empire->CapitalID()))
                SelectSystem(obj->SystemID());
    } else {
        // if a system is already shown, make sure a planet gets selected by
        // default when the production screen opens up
        m_production_wnd->SelectDefaultPlanet(context.ContextObjects());
    }
    m_production_wnd->Update(context);
    m_production_wnd->Show();

    // hide pedia again if it is supposed to be hidden persistently
    if (GetOptionsDB().Get<bool>("ui.production.pedia.hidden.enabled"))
        m_production_wnd->TogglePedia();
}

void MapWnd::HideProduction() {
    GGHumanClientApp::GetApp()->SendPartialOrders();

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
}

bool MapWnd::ToggleProduction() {
    if (m_in_production_view_mode) {
        HideProduction();
        RestoreSidePanel();
    } else {
        ShowProduction();
    }

    // make info panels in production/map window's side panel update their expand-collapse state
    m_side_panel->Update();

    return true;
}

void MapWnd::ShowDesign() {
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideResearch();
    HideProduction();
    HideSidePanelAndRememberIfItWasVisible();

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
    GGHumanClientApp::GetApp()->SendPartialOrders();

    m_design_wnd->Hide();
    RemoveFromWndStack(m_design_wnd);
    m_btn_design->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "design.png")));
    m_btn_design->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "design_mouseover.png")));
}

bool MapWnd::ToggleDesign() {
    if (m_design_wnd->Visible()) {
        HideDesign();
        RestoreSidePanel();
    } else {
        ShowDesign();
    }
    return true;
}

void MapWnd::ShowGovernment() {
    ClearProjectedFleetMovementLines();

    // hide other "competing" windows
    HideResearch();
    HideProduction();
    HideDesign();
    RestoreSidePanel();

    // show the government window
    m_government_wnd->Show();
    GG::GUI::GetGUI()->MoveUp(m_government_wnd);
    PushWndStack(m_government_wnd);
    m_government_wnd->Reset();

    // indicate selection on button
    m_btn_government->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "government_mouseover.png")));
    m_btn_government->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "government.png")));
}

void MapWnd::HideGovernment() {
    GGHumanClientApp::GetApp()->SendPartialOrders();

    m_government_wnd->Hide();
    RemoveFromWndStack(m_government_wnd);
    m_btn_government->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "government.png")));
    m_btn_government->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "government_mouseover.png")));
}

bool MapWnd::ToggleGovernment() {
    if (!m_government_wnd->Visible() || m_production_wnd->Visible() ||
        m_research_wnd->Visible() || m_design_wnd->Visible())
    {
        ShowGovernment();
    } else {
        HideGovernment();
    }
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
    GGHumanClientApp::GetApp()->SendPartialOrders();

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

void MapWnd::RefreshTurnButtonTooltip() {
    auto app = GGHumanClientApp::GetApp();
    std::string btn_turn_tooltip;

    if (!m_ready_turn) {
        if (app->SinglePlayerGame())
            btn_turn_tooltip = UserString("MAP_BTN_TURN_TOOLTIP_DESC_SP");
        else
            btn_turn_tooltip = UserString("MAP_BTN_TURN_TOOLTIP_DESC_MP");
        if (app->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR)
            btn_turn_tooltip = UserString("MAP_BTN_TURN_TOOLTIP_DESC_MOD");
    }
    if (m_ready_turn && !app->SinglePlayerGame())
        btn_turn_tooltip = UserString("MAP_BTN_TURN_TOOLTIP_DESC_WAIT");
    if (app->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER)
        btn_turn_tooltip = UserString("MAP_BTN_TURN_TOOLTIP_DESC_OBS");

    m_btn_turn->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_btn_turn->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_BTN_TURN_TOOLTIP"), btn_turn_tooltip));
}

void MapWnd::RefreshInfluenceResourceIndicator() {
    const Empire* empire = GetEmpire(GGHumanClientApp::GetApp()->EmpireID());
    if (!empire) {
        m_influence->SetValue(0.0);
        return;
    }
    const double total_IP_spent = empire->GetInfluenceQueue().TotalIPsSpent();
    const double total_IP_output = empire->GetInfluencePool().TotalOutput();
    const double total_IP_target_output = empire->GetInfluencePool().TargetOutput();
    const float  stockpile = empire->GetInfluencePool().Stockpile();
    const float  stockpile_used = empire->GetInfluenceQueue().AllocatedStockpileIP();
    const float  expected_stockpile = empire->GetInfluenceQueue().ExpectedNewStockpileAmount();

    const float  stockpile_plusminus_next_turn = expected_stockpile - stockpile;

    m_influence->SetValue(stockpile);
    m_influence->SetValue(stockpile_plusminus_next_turn, 1);

    DebugLogger() << "MapWnd::RefreshInfluenceResourceIndicator stockpile: " << stockpile
                  << " plusminus: " << stockpile_plusminus_next_turn;

    m_influence->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_influence->SetBrowseInfoWnd(GG::Wnd::Create<ResourceBrowseWnd>(
        UserString("MAP_INFLUENCE_TITLE"), UserString("GOVERNMENT_INFO_IP"),
        total_IP_spent, total_IP_output, total_IP_target_output,
        true, stockpile_used, stockpile, expected_stockpile));
}

void MapWnd::RefreshFleetResourceIndicator(const ScriptingContext& context, int empire_id) {
    if (!GetEmpire(empire_id)) {
        m_fleet->SetValue(0.0);
        return;
    }

    const auto& this_client_known_destroyed_objects =
        context.ContextUniverse().EmpireKnownDestroyedObjectIDs(empire_id);

    int total_fleet_count = 0;
    for (auto* ship : context.ContextObjects().allRaw<Ship>()) {
        if (ship->OwnedBy(empire_id) && !this_client_known_destroyed_objects.contains(ship->ID()))
            total_fleet_count++;
    }

    m_fleet->SetValue(total_fleet_count);
    m_fleet->ClearBrowseInfoWnd();
    m_fleet->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_fleet->SetBrowseInfoWnd(GG::Wnd::Create<FleetDetailBrowseWnd>(
        empire_id, GG::X(FontBasedUpscale(250))));
}

void MapWnd::RefreshResearchResourceIndicator() {
    const Empire* empire = GetEmpire(GGHumanClientApp::GetApp()->EmpireID());
    if (!empire) {
        m_research->SetValue(0.0);
        m_research_wasted->Hide();
        return;
    }
    m_research->SetValue(empire->ResourceOutput(ResourceType::RE_RESEARCH));
    m_research->ClearBrowseInfoWnd();
    m_research->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    float total_RP_spent = empire->GetResearchQueue().TotalRPsSpent();
    float total_RP_output = empire->GetResearchPool().TotalOutput();
    float total_RP_wasted = total_RP_output - total_RP_spent;
    float total_RP_target_output = empire->GetResearchPool().TargetOutput();

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
    const Empire* empire = GetEmpire(GGHumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return;
    m_detection->SetValue(empire->GetMeter("METER_DETECTION_STRENGTH")->Current());
    m_detection->ClearBrowseInfoWnd();
    m_detection->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_detection->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
        UserString("MAP_DETECTION_TITLE"), UserString("MAP_DETECTION_TEXT")));
}

void MapWnd::RefreshIndustryResourceIndicator() {
    const ScriptingContext& context = IApp::GetApp()->GetContext();
    auto empire = context.GetEmpire(GGHumanClientApp::GetApp()->EmpireID());
    if (!empire) {
        m_industry->SetValue(0.0);
        m_industry_wasted->Hide();
        m_stockpile->SetValue(0.0);
        return;
    }
    m_industry->SetValue(empire->ResourceOutput(ResourceType::RE_INDUSTRY));
    m_industry->ClearBrowseInfoWnd();
    m_industry->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    double total_PP_spent = empire->GetProductionQueue().TotalPPsSpent();
    double total_PP_output = empire->GetIndustryPool().TotalOutput();
    double total_PP_target_output = empire->GetIndustryPool().TargetOutput();
    float  stockpile = empire->GetIndustryPool().Stockpile();
    const auto stockpile_values = empire->GetProductionQueue().AllocatedStockpilePP() | range_values;
    float  stockpile_used = std::accumulate(stockpile_values.begin(), stockpile_values.end(), 0.0f);
    float  stockpile_use_capacity = empire->GetProductionQueue().StockpileCapacity(context.ContextObjects());
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
    const auto* app = GGHumanClientApp::GetApp();
    const auto empire_id = app->EmpireID();
    const auto& context = app->GetContext();

    float target_population = 0.0f;
    const auto empire = context.GetEmpire(empire_id);
    if (!empire) {
        m_population->SetValue(0.0);
        return;
    } else {
        target_population = empire->GetPopulationPool().Population();
    }
    m_population->SetValue(target_population);
    m_population->ClearBrowseInfoWnd();

    const auto& pop_center_ids = empire->GetPopulationPool().PopCenterIDs();
    std::map<std::string, float> population_counts;
    std::map<std::string, int>   population_worlds;
    std::map<std::string, float> tag_counts;
    std::map<std::string, int>   tag_worlds;

    //tally up all species population counts
    for (const auto* pc : context.ContextObjects().findRaw<Planet>(pop_center_ids)) {
        if (!pc)
            continue;

        const auto& species_name = pc->SpeciesName();
        if (species_name.empty())
            continue;
        const float this_pop = pc->UniverseObject::GetMeter(MeterType::METER_POPULATION)->Initial();
        population_counts[species_name] += this_pop;
        population_worlds[species_name] += 1;
        if (const Species* species = context.species.GetSpecies(species_name) ) {
            for (auto tag : species->Tags()) {
                tag_counts[std::string{tag}] += this_pop;
                tag_worlds[std::string{tag}] += 1;
            }
        }
    }

    m_population->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_population->SetBrowseInfoWnd(GG::Wnd::Create<CensusBrowseWnd>(
        UserString("MAP_POPULATION_DISTRIBUTION"),
        target_population,
        std::move(population_counts),std::move(population_worlds),
        std::move(tag_counts), std::move(tag_worlds), context.species.census_order()
    ));
}

void MapWnd::UpdateEmpireResourcePools(ScriptingContext& context, int empire_id) {
    auto empire = context.GetEmpire(empire_id);
    if (!empire)
        return;

    /* Recalculate stockpile, available, production, predicted change of
     * resources.  When resource pools update, they emit ChangeSignal, which is
     * connected to MapWnd::Refresh???ResourceIndicator, which updates the
     * empire resource pool indicators of the MapWnd. */
    empire->UpdateResourcePools(context, empire->TechCostsTimes(context),
                                empire->PlanetAnnexationCosts(context),
                                empire->PolicyAdoptionCosts(context),
                                empire->ProductionCostsTimes(context));

    // Update indicators on sidepanel, which are not directly connected to from the ResourcePool ChangedSignal
    SidePanel::Update();
}

bool MapWnd::ZoomToHomeSystem() {
    const auto* app = GGHumanClientApp::GetApp();
    const auto& context = app->GetContext();

    const auto empire = context.GetEmpire(app->EmpireID());
    if (!empire)
        return false;
    int home_id = empire->CapitalID();

    if (home_id != INVALID_OBJECT_ID) {
        const auto object = context.ContextObjects().get(home_id);
        if (!object)
            return false;
        CenterOnObject(object->SystemID());
        SelectSystem(object->SystemID());
    }

    return true;
}

namespace {
    struct AllTrue { constexpr bool operator()(const auto*) { return true; } };

    template <typename T, typename P = AllTrue>
        requires std::is_base_of_v<UniverseObject, T> &&
                 requires(P p, const T* t) { {p(t)} -> std::same_as<bool>; }
    auto IDsSortedByName(const ObjectMap& objects, P&& pred = P{}) {
        constexpr auto to_id_name = [](const auto* obj) -> std::pair<int, std::string_view> { return {obj->ID(), obj->Name()}; };

        const auto objs = [pred, &objects]() -> decltype(auto) {
            if constexpr (std::is_same_v<std::decay_t<decltype(pred)>, AllTrue>)
                return objects.allExistingRaw<const T>();
            else
                return objects.findExistingRaw<const T>(pred);
        }();
        // collect ids and corresponding names
        std::vector<std::pair<int, std::string_view>> ids_names;
        ids_names.reserve(objects.size<T>());
        range_copy(objs | range_transform(to_id_name), std::back_inserter(ids_names));
        // alphabetize, with empty names at end
        auto not_empty_it = std::partition(ids_names.begin(), ids_names.end(),
                                           [](const auto& id_name) { return !id_name.second.empty(); });
        std::sort(ids_names.begin(), not_empty_it,
                  [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
        // extract ordered ids
        std::vector<int> retval;
        retval.reserve(ids_names.size());
        range_copy(ids_names | range_keys, std::back_inserter(retval));
        return retval;
    }

    template <typename It>
    constexpr auto LoopNext(const It begin_it, const It from_it, const It end_it) {
        if (from_it != end_it) {
            auto it = std::next(from_it);
            if (it != end_it)
                return it;
        }
        return begin_it;
    };

    constexpr std::array<int, 10> test_nums = {1,2,3,4,5,6,7,8,9,10};
    constexpr auto tnsb = test_nums.begin(), tnsrt = std::next(test_nums.begin(), 5), tnsnd = test_nums.end();
    static_assert(*tnsrt == 6 && *LoopNext(tnsb, tnsrt, tnsnd) == 7);
    static_assert(LoopNext(tnsb, tnsnd, tnsnd) == tnsb);

    enum class SearchDir : bool { FORWARD, REVERSE };

    // gets id of loop-next id in \a ids
    int GetNext(const auto& ids, const int start_from_id, SearchDir dir = SearchDir::FORWARD) {
        if (ids.empty()) [[unlikely]]
            return INVALID_OBJECT_ID;

        if (ids.size() == 1u) [[unlikely]]
            return *ids.begin(); // forward and backwards are the same in this case...

        if (dir == SearchDir::FORWARD)
            return *LoopNext(ids.begin(), std::find(ids.begin(), ids.end(), start_from_id), ids.end());
        else
            return *LoopNext(ids.rbegin(), std::find(ids.rbegin(), ids.rend(), start_from_id), ids.rend());
    }

    bool ZoomToPrevOrNextOwnedSystem(SearchDir dir, MapWnd& mw) {
        const auto* app = GGHumanClientApp::GetApp();
        const auto empire_id = app->EmpireID();
        const auto& objs = app->GetContext().ContextObjects();

        const auto contains_owned_by_empire = [empire_id, &objs](const System* sys) {
            auto is_owned_and_contained = [empire_id, sys_id{sys->ID()}](const Planet* obj)
            { return obj && obj->ContainedBy(sys_id) && obj->OwnedBy(empire_id); };

            return objs.check_if_any<Planet, decltype(is_owned_and_contained), false>(is_owned_and_contained);
        };

        const auto next_sys_id = GetNext(IDsSortedByName<System>(objs, contains_owned_by_empire),
                                         SidePanel::SystemID(), dir);
        if (next_sys_id == INVALID_OBJECT_ID)
            return false;

        mw.CenterOnObject(next_sys_id);
        mw.SelectSystem(next_sys_id);

        return true;
    }

    bool ZoomToPrevOrNextSystem(SearchDir dir, MapWnd& mw) {
        const auto& objs = GGHumanClientApp::GetApp()->GetContext().ContextObjects();
        const auto next_sys_id = GetNext(IDsSortedByName<System>(objs), SidePanel::SystemID(), dir);
        if (next_sys_id == INVALID_OBJECT_ID)
            return false;

        mw.CenterOnObject(next_sys_id);
        mw.SelectSystem(next_sys_id);

        return true;
    }

    bool ZoomToPrevOrNextOwnedFleet(SearchDir dir, MapWnd& mw) {
        const auto* app = GGHumanClientApp::GetApp();
        const auto& objs = app->GetContext().ContextObjects();

        const auto is_owned_fleet = [client_empire_id{app->EmpireID()}](const Fleet* fleet)
        { return client_empire_id == ALL_EMPIRES || fleet->OwnedBy(client_empire_id); };

        const auto next_fleet_id = GetNext(IDsSortedByName<Fleet>(objs, is_owned_fleet),
                                           mw.SelectedFleetID(), dir);
        if (next_fleet_id == INVALID_OBJECT_ID)
            return false;

        mw.CenterOnObject(next_fleet_id);
        mw.SelectFleet(next_fleet_id);

        return true;
    }

    bool ZoomToPrevOrNextIdleFleet(SearchDir dir, MapWnd& mw) {
        const auto* app = GGHumanClientApp::GetApp();
        const auto& objs = app->GetContext().ContextObjects();

        auto is_stationary_owned_fleet = [client_empire_id{app->EmpireID()}](const Fleet* fleet) {
            return (fleet->FinalDestinationID() == INVALID_OBJECT_ID || fleet->TravelRoute().empty()) &&
                    (client_empire_id == ALL_EMPIRES ||
                    (!fleet->Unowned() && fleet->Owner() == client_empire_id));
        };

        const auto next_fleet_id = GetNext(IDsSortedByName<Fleet>(objs, is_stationary_owned_fleet),
                                           mw.SelectedFleetID(), dir);
        if (next_fleet_id == INVALID_OBJECT_ID)
            return false;

        mw.CenterOnObject(next_fleet_id);
        mw.SelectFleet(next_fleet_id);
        return true;
    }
}

bool MapWnd::ZoomToPrevOwnedSystem()
{ return ZoomToPrevOrNextOwnedSystem(SearchDir::REVERSE, *this); }

bool MapWnd::ZoomToNextOwnedSystem()
{ return ZoomToPrevOrNextOwnedSystem(SearchDir::FORWARD, *this); }

bool MapWnd::ZoomToPrevSystem()
{ return ZoomToPrevOrNextSystem(SearchDir::REVERSE, *this); }

bool MapWnd::ZoomToNextSystem()
{ return ZoomToPrevOrNextSystem(SearchDir::FORWARD, *this); }

bool MapWnd::ZoomToPrevIdleFleet()
{ return ZoomToPrevOrNextIdleFleet(SearchDir::REVERSE, *this); }

bool MapWnd::ZoomToNextIdleFleet()
{ return ZoomToPrevOrNextIdleFleet(SearchDir::FORWARD, *this); }

bool MapWnd::ZoomToPrevFleet()
{ return ZoomToPrevOrNextOwnedFleet(SearchDir::REVERSE, *this); }

bool MapWnd::ZoomToNextFleet()
{ return ZoomToPrevOrNextOwnedFleet(SearchDir::FORWARD, *this); }

bool MapWnd::ZoomToSystemWithWastedPP() {
    const ScriptingContext& context = IApp::GetApp()->GetContext();
    const auto empire = context.GetEmpire(GGHumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return false;
    const ProductionQueue& queue = empire->GetProductionQueue();
    const auto& pool = empire->GetIndustryPool();
    auto wasted_PP_objects(queue.ObjectsWithWastedPP(pool));
    if (wasted_PP_objects.empty())
        return false;

    for (const auto& obj_ids : wasted_PP_objects) {
        for (auto id : obj_ids) {
            const auto* obj = context.ContextObjects().getRaw(id);
            if (!obj)
                continue;
            const auto sys_id = obj->SystemID();
            if (sys_id == INVALID_OBJECT_ID)
                continue;
            // found object with wasted PP that is in a system.  zoom there.
            CenterOnObject(sys_id);
            SelectSystem(sys_id);
            ShowProduction();
            return true;
        }
    }
    return false;
}

namespace {
    /// On when the MapWnd window is visible and not covered by one of the full screen covering windows
    struct NotCoveredMapWndCondition {
        NotCoveredMapWndCondition(const MapWnd& tg) : target(tg) {}
        bool operator()() const
        { return target.Visible() && !target.InResearchViewMode() && !target.InDesignViewMode(); };
        const MapWnd& target;
    };
}

void MapWnd::ConnectKeyboardAcceleratorSignals() {
    HotkeyManager& hkm = HotkeyManager::GetManager();

    hkm.Connect(boost::bind(&MapWnd::ReturnToMap, this), "ui.map.open",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::EndTurn, this), "ui.turn.end",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ToggleSitRep, this), "ui.map.sitrep",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect([this]() { return ToggleResearch(IApp::GetApp()->GetContext()); }, "ui.research",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ToggleProduction, this), "ui.production",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ToggleGovernment, this), "ui.government",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ToggleDesign, this), "ui.design",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ToggleObjects, this), "ui.map.objects",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ToggleMessages, this), "ui.map.messages",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ToggleEmpires, this), "ui.map.empires",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::TogglePedia, this), "ui.pedia",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ShowGraphs, this), "ui.map.graphs",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ShowMenu, this), "ui.gamemenu",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::KeyboardZoomIn, this), "ui.zoom.in",
                AndCondition(NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::KeyboardZoomIn, this), "ui.zoom.in.alt",
                AndCondition(NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::KeyboardZoomOut, this), "ui.zoom.out",
                AndCondition(NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::KeyboardZoomOut, this), "ui.zoom.out.alt",
                AndCondition(NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ZoomToHomeSystem, this), "ui.map.system.zoom.home",
                AndCondition(NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ZoomToPrevSystem, this), "ui.map.system.zoom.prev",
                AndCondition(NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ZoomToNextSystem, this), "ui.map.system.zoom.next",
                AndCondition(NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ZoomToPrevOwnedSystem, this), "ui.map.system.owned.zoom.prev",
                AndCondition(NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ZoomToNextOwnedSystem, this), "ui.map.system.owned.zoom.next",
                AndCondition(NotCoveredMapWndCondition(*this), NoModalWndsOpenCondition));

    // the list of windows for which the fleet shortcuts are blacklisted.
    std::array<const GG::Wnd*, 3> bl = {m_research_wnd.get(), m_production_wnd.get(), m_design_wnd.get()};

    hkm.Connect(boost::bind(&MapWnd::ZoomToPrevFleet, this), "ui.map.fleet.zoom.prev",
                AndCondition(OrCondition(InvisibleWindowCondition(bl), VisibleWindowCondition(this)), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ZoomToNextFleet, this), "ui.map.fleet.zoom.next",
                AndCondition(OrCondition(InvisibleWindowCondition(bl), VisibleWindowCondition(this)), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ZoomToPrevIdleFleet, this), "ui.map.fleet.idle.zoom.prev",
                AndCondition(OrCondition(InvisibleWindowCondition(bl), VisibleWindowCondition(this)), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::ZoomToNextIdleFleet, this), "ui.map.fleet.idle.zoom.next",
                AndCondition(OrCondition(InvisibleWindowCondition(bl), VisibleWindowCondition(this)), NoModalWndsOpenCondition));

    hkm.Connect(boost::bind(&MapWnd::PanX, this, GG::X(50)),   "ui.pan.right",
                AndCondition(OrCondition(InvisibleWindowCondition(bl), VisibleWindowCondition(this)), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::PanX, this, GG::X(-50)),  "ui.pan.left",
                AndCondition(OrCondition(InvisibleWindowCondition(bl), VisibleWindowCondition(this)), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::PanY, this, GG::Y(50)),   "ui.pan.down",
                AndCondition(OrCondition(InvisibleWindowCondition(bl), VisibleWindowCondition(this)), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&MapWnd::PanY, this, GG::Y(-50)),  "ui.pan.up",
                AndCondition(OrCondition(InvisibleWindowCondition(bl), VisibleWindowCondition(this)), NoModalWndsOpenCondition));

    hkm.Connect(boost::bind(&ToggleBoolOption, "ui.map.scale.legend.shown"), "ui.map.scale.legend",
                AndCondition(OrCondition(InvisibleWindowCondition(bl), VisibleWindowCondition(this)), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&ToggleBoolOption, "ui.map.scale.circle.shown"), "ui.map.scale.circle",
                AndCondition(OrCondition(InvisibleWindowCondition(bl), VisibleWindowCondition(this)), NoModalWndsOpenCondition));


    // these are general-use hotkeys, only connected here as a convenient location to do so once.
    hkm.Connect(boost::bind(&GG::GUI::CutFocusWndText, GG::GUI::GetGUI()), "ui.cut");
    hkm.Connect(boost::bind(&GG::GUI::CopyFocusWndText, GG::GUI::GetGUI()), "ui.copy");
    hkm.Connect(boost::bind(&GG::GUI::PasteFocusWndClipboardText, GG::GUI::GetGUI()), "ui.paste");

    hkm.Connect(boost::bind(&GG::GUI::FocusWndSelectAll, GG::GUI::GetGUI()), "ui.select.all");
    hkm.Connect(boost::bind(&GG::GUI::FocusWndDeselect, GG::GUI::GetGUI()), "ui.select.none");

    //hkm.Connect(boost::bind(&GG::GUI::SetPrevFocusWndInCycle, GG::GUI::GetGUI()), "ui.focus.prev",
    //             NoModalWndsOpenCondition);
    //hkm.Connect(boost::bind(&GG::GUI::SetNextFocusWndInCycle, GG::GUI::GetGUI()), "ui.focus.next",
    //             NoModalWndsOpenCondition);

    hkm.RebuildShortcuts();
}

void MapWnd::CloseAllPopups()
{ GG::ProcessThenRemoveExpiredPtrs(m_popups, [](auto& wnd) { wnd->Close(); }); }

void MapWnd::HideAllPopups()
{ GG::ProcessThenRemoveExpiredPtrs(m_popups, [](auto& wnd) { wnd->Hide(); }); }

void MapWnd::SetFleetExploring(const int fleet_id) {
    if (!std::count(m_fleets_exploring.begin(), m_fleets_exploring.end(), fleet_id)) {
        m_fleets_exploring.insert(fleet_id);
        DispatchFleetsExploring();
    }
}

void MapWnd::StopFleetExploring(const int fleet_id, ObjectMap& objects) {
    auto it = m_fleets_exploring.find(fleet_id);
    if (it == m_fleets_exploring.end())
        return;

    m_fleets_exploring.erase(it);

    DispatchFleetsExploring();
    // force UI update. Removing a fleet from the UI's list of exploring fleets
    // doesn't actually change the Fleet object's state in any way, so the UI
    // would otherwise still show the fleet as "exploring"
    if (auto fleet = objects.get<Fleet>(fleet_id))
        fleet->StateChangedSignal();
}

bool MapWnd::IsFleetExploring(const int fleet_id) const
{ return m_fleets_exploring.contains(fleet_id); }

namespace {
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
    bool FleetRouteInRange(const Fleet* fleet, const RouteListType& route,
                           const ScriptingContext& context)
    {
        const auto eta_final_turns = fleet->ETA(fleet->MovePath(route, false, context)).first;
        return (eta_final_turns != Fleet::ETA_NEVER &&
                eta_final_turns != Fleet::ETA_UNKNOWN &&
                eta_final_turns != Fleet::ETA_OUT_OF_RANGE);
    }

    // helper function for DispatchFleetsExploring
    // return systems ID with a starlane connecting them to a system in \a system_ids
    boost::container::flat_set<int> NeighbourSystemsOf(const Empire* empire, const Universe& universe,
                                                       const auto& system_ids)
    {
        const auto starlanes{empire->KnownStarlanes(universe)};
        using starlanes_t = std::decay_t<decltype(starlanes)>;
        static_assert(std::is_same_v<starlanes_t, boost::container::flat_set<Empire::LaneEndpoints>>,
                      "make sure starlanes is sorted for efficient insertion into flat_set below");
        const auto lane_starts_in_system_in_ids = [&system_ids](const auto lane) {
            return std::any_of(system_ids.begin(), system_ids.end(),
                               [lane](const auto sys_id) { return lane.start == sys_id; });
        };
        auto rng = starlanes | range_filter(lane_starts_in_system_in_ids)
            | range_transform([](const auto lane) { return lane.end; });
        boost::container::flat_set<int> retval;
        retval.reserve(starlanes.size());
        retval.insert(rng.begin(), rng.end());
        return retval;
    }

    /** Get the shortest suitable route from @p start_id to @p destination_id as known to @p empire_id */
    OrderedRouteType GetShortestRoute(int empire_id, int start_id, int destination_id) {
        const Universe& universe = GetUniverse();
        const ObjectMap& objects = universe.Objects();
        const auto start_system = objects.getRaw<System>(start_id);
        const auto dest_system = objects.getRaw<System>(destination_id);
        if (!start_system || !dest_system) {
            WarnLogger() << "GetShortestRoute: couldn't find start or destination systems";
            return {};
        }

        auto [system_list, path_length] =
            universe.GetPathfinder().ShortestPath(start_id, destination_id, empire_id, objects);

        if (!system_list.empty() && path_length > 0.0)
            return {path_length, system_list};

        return {};
    }

    /** Route from @p fleet current location to @p destination */
    OrderedFleetRouteType GetOrderedFleetRoute(const Fleet* fleet,
                                               const System* destination,
                                               const ScriptingContext& context)
    {
        const ObjectMap& objects{context.ContextObjects()};

        if (!fleet || !destination) {
            WarnLogger() << "GetOrderedFleetRoute: null fleet or system";
            return {};
        }
        if ((fleet->Fuel(objects) < 1.0f) || !fleet->MovePath(false, context).empty()) {
            WarnLogger() << "GetOrderedFleetRoute: no fuel or non-empty move path";
            return {};
        }

        auto [route_length, route_ids] = GetShortestRoute(fleet->Owner(), fleet->SystemID(), destination->ID());

        if (route_length <= 0.0) {
            TraceLogger() << "No suitable route from system " << fleet->SystemID() << " to " << destination->ID()
                          << " (" << route_ids.size() << ">" << route_length << ")";
            return {};
        }

        if (!FleetRouteInRange(fleet, route_ids, context)) {
            TraceLogger() << "Fleet " << std::to_string(fleet->ID())
                          << " has no ETA for route to " << std::to_string(route_ids.back());
            return {};
        }

        // decrease priority of system if previously viewed but not yet explored
        if (!destination->Name().empty()) {
            route_length *= GetOptionsDB().Get<float>("ui.fleet.explore.system.known.multiplier");
            TraceLogger() << "Deferred priority for system " << destination->Name() << " (" << destination->ID() << ")";
        }

        return std::pair{route_length, std::pair{fleet->ID(), std::move(route_ids)}};
    }

    /** Shortest route not exceeding @p max_jumps from @p dest_id to a system with supply as known to @p empire */
    OrderedRouteType GetNearestSupplyRoute(const std::shared_ptr<const Empire>& empire,
                                           int dest_id, int max_jumps,
                                           const ScriptingContext& context)
    {
        OrderedRouteType retval;

        if (!empire) {
            WarnLogger() << "Invalid empire";
            return retval;
        }

        auto supplyable_systems = context.supply.FleetSupplyableSystemIDs(
            empire->EmpireID(), true, context);
        if (!supplyable_systems.empty()) {
            TraceLogger() << [&supplyable_systems]() {
                    std::string msg = "Supplyable systems:";
                    for (auto sys : supplyable_systems)
                        msg.append(" " + std::to_string(sys));
                    return msg;
                }();
        }

            OrderedRouteType shortest_route;

        for (auto supply_system_id : supplyable_systems) {
            shortest_route = GetShortestRoute(empire->EmpireID(), dest_id, supply_system_id);
            TraceLogger() << [&shortest_route, dest_id]() {
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
                retval = std::move(shortest_route);
            }
        }

        return retval;
    }

    /** If @p fleet would be able to reach a system with supply after completing @p route */
    bool CanResupplyAfterDestination(const Fleet* fleet, const RouteListType& route,
                                     const ScriptingContext& context)
    {
        const ObjectMap& objects{context.ContextObjects()};

        if (!fleet || route.empty()) {
            WarnLogger() << "Invalid fleet or empty route";
            return false;
        }
        auto empire = context.GetEmpire(fleet->Owner());
        if (!empire) {
            WarnLogger() << "Invalid empire";
            return false;
        }

        int max_jumps = std::trunc(fleet->Fuel(objects));
        if (max_jumps < 1) {
            TraceLogger() << "Not enough fuel " << std::to_string(max_jumps)
                          << " to move fleet " << std::to_string(fleet->ID());
            return false;
        }

        auto dest_nearest_supply = GetNearestSupplyRoute(
            empire, route.back(), max_jumps, context);
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
    OrderedRouteType ExploringFleetResupplyRoute(const Fleet* fleet,
                                                 const ScriptingContext& context)
    {
        auto empire = context.GetEmpire(fleet->Owner());
        if (!empire) {
            WarnLogger() << "Invalid empire for id " << fleet->Owner();
            return {};
        }

        auto nearest_supply = GetNearestSupplyRoute(empire, fleet->SystemID(),
                                                    std::trunc(fleet->Fuel(context.ContextObjects())),
                                                    context);
        if (nearest_supply.first > 0.0 && FleetRouteInRange(fleet, nearest_supply.second, context))
            return nearest_supply;

        return {};
    }

    /** Issue an order for @p fleet to move to nearest system with supply */
    bool IssueFleetResupplyOrder(const Fleet* fleet, ScriptingContext& context) {
        if (!fleet) {
            WarnLogger() << "Invalid fleet";
            return false;
        }

        auto route = ExploringFleetResupplyRoute(fleet, context);
        // Attempt move order if route is not empty and fleet has enough fuel to reach it
        if (route.second.empty()) {
            TraceLogger() << "Empty route for resupply of exploring fleet " << fleet->ID();
            return false;
        }

        auto num_jumps_resupply = JumpsForRoute(route.second);
        int max_fleet_jumps = std::trunc(fleet->Fuel(context.ContextObjects()));
        if (num_jumps_resupply <= max_fleet_jumps) {
            auto* app = GGHumanClientApp::GetApp();
            app->Orders().IssueOrder<FleetMoveOrder>(
                context, fleet->Owner(), fleet->ID(), *route.second.crbegin(), false);
        } else {
            TraceLogger() << "Not enough fuel for fleet " << fleet->ID()
                          << " to resupply at system " << *route.second.crbegin();
            return false;
        }

        if (fleet->FinalDestinationID() == *route.second.crbegin()) {
            TraceLogger() << "Sending fleet " << fleet->ID()
                          << " to refuel at system " << *route.second.crbegin();
            return true;
        } else {
            TraceLogger() << "Fleet move order failed fleet:" << fleet->ID() << " route:"
                          << [&route]() {
                                 std::string retval;
                                 for (auto node : route.second)
                                     retval += " " + std::to_string(node);
                                 return retval;
                             }();
        }

        return false;
    }

    /** Issue order for @p fleet to move using @p route */
    bool IssueFleetExploreOrder(const Fleet* fleet, const RouteListType& route,
                                ScriptingContext& context)
    {
        if (!fleet || route.empty()) {
            WarnLogger() << "Invalid fleet or empty route";
            return false;
        }
        if (!FleetRouteInRange(fleet, route, context)) {
            TraceLogger() << "Fleet " << std::to_string(fleet->ID())
                          << " has no eta for route to " << std::to_string(route.back());
            return false;
        }

        GGHumanClientApp::GetApp()->Orders().IssueOrder<FleetMoveOrder>(
            context, fleet->Owner(), fleet->ID(), route.back(), false);
        if (fleet->FinalDestinationID() == route.back()) {
            TraceLogger() << "Sending fleet " << fleet->ID() << " to explore system " << route.back();
            return true;
        }

        TraceLogger() << "Fleet move order failed fleet:" << fleet->ID() << " dest:" << route.back();
        return false;
    }

    /** Determine and issue move order for fleet and route @p fleet_route */
    void IssueExploringFleetOrders(boost::unordered_set<int>& idle_fleets,
                                   SystemFleetMap& systems_being_explored,
                                   const FleetRouteType& fleet_route,
                                   ScriptingContext& context)
    {
        ObjectMap& objects{context.ContextObjects()};

        const auto& route = fleet_route.second;
        if (route.empty()) { // no route
            WarnLogger() << "Attempted to issue move order with empty route";
            return;
        }

        if (idle_fleets.empty()) { // no more fleets to issue orders to
            TraceLogger() << "No idle fleets";
            return;
        }

        if (systems_being_explored.contains(route.back())) {
            TraceLogger() << "System " << std::to_string(route.back()) << " already being explored";
            return;
        }

        auto fleet_id = fleet_route.first;
        auto idle_fleet_it = idle_fleets.find(fleet_id);
        if (idle_fleet_it == idle_fleets.end()) { // fleet no longer idle
            TraceLogger() << "Fleet " << std::to_string(fleet_id) << " not idle";
            return;
        }
        auto fleet = objects.getRaw<Fleet>(fleet_id);
        if (!fleet) {
            ErrorLogger() << "No valid fleet with id " << fleet_id;
            idle_fleets.erase(idle_fleet_it);
            return;
        }

        if (std::trunc(fleet->Fuel(objects)) < 1) {  // wait for fuel
            TraceLogger() << "Not enough fuel to move fleet " << std::to_string(fleet->ID());
            return;
        }

        // Determine if fleet should refuel
        if (fleet->Fuel(objects) < fleet->MaxFuel(objects) &&
            !CanResupplyAfterDestination(fleet, route, context))
        {
            if (IssueFleetResupplyOrder(fleet, context)) {
                idle_fleets.erase(idle_fleet_it);
                return;
            }
            TraceLogger() << "Fleet " << std::to_string(fleet->ID()) << " can not reach resupply";
        }

        if (IssueFleetExploreOrder(fleet, route, context)) {
            idle_fleets.erase(idle_fleet_it);
            systems_being_explored.emplace(*route.rbegin(), fleet->ID());
        }
    }

};

void MapWnd::DispatchFleetsExploring() {
    ScriptingContext& context = IApp::GetApp()->GetContext();
    const auto& universe{context.ContextUniverse()};
    const auto& objects{context.ContextObjects()};

    int empire_id = GGHumanClientApp::GetApp()->EmpireID();
    auto empire = context.GetEmpire(empire_id);
    if (!empire) {
        WarnLogger() << "Invalid empire";
        return;
    }
    int max_routes_per_system = GetOptionsDB().Get<int>("ui.fleet.explore.system.route.limit");
    const auto& destroyed_objects{universe.EmpireKnownDestroyedObjectIDs(empire_id)};

    boost::unordered_set<int> idle_fleets;
    /** all systems ID for which an exploring fleet is in route and the fleet assigned */
    SystemFleetMap systems_being_explored;

    // clean the fleet list by removing non-existing fleet, and extract the
    // fleets waiting for orders
    for (const auto* fleet : objects.findRaw<Fleet>(m_fleets_exploring)) {
        if (!fleet)
            continue;
        if (destroyed_objects.contains(fleet->ID())) {
            m_fleets_exploring.erase(fleet->ID()); //this fleet can't explore anymore
        } else {
            if (fleet->MovePath(false, context).empty())
                idle_fleets.insert(fleet->ID());
            else
                systems_being_explored.emplace(fleet->FinalDestinationID(), fleet->ID());
        }
    }

    if (idle_fleets.empty())
        return;

    TraceLogger() << [&idle_fleets]() {
            std::string retval = "MapWnd::DispatchFleetsExploring Idle Exploring Fleet IDs:";
            for (auto fleet_id : idle_fleets)
                retval += " " + std::to_string(fleet_id);
            return retval;
        }();

    //list all unexplored systems by taking the neighboors of explored systems because ObjectMap does not list them all.
    auto explored_systems{empire->ExploredSystems()};
    auto candidates_unknown_systems{NeighbourSystemsOf(empire.get(), universe, explored_systems)};
    candidates_unknown_systems.merge(NeighbourSystemsOf(empire.get(), universe, candidates_unknown_systems));

    // Populate list of unexplored systems
    boost::unordered_set<int> unexplored_systems;
    for (const auto* system : objects.findRaw<System>(candidates_unknown_systems)) {
        if (!system)
            continue;
        if (!empire->HasExploredSystem(system->ID()) &&
            !systems_being_explored.contains(system->ID()))
        { unexplored_systems.emplace(system->ID()); }
    }

    if (unexplored_systems.empty()) {
        TraceLogger() << "No unknown systems to explore";
        return;
    }

    TraceLogger() << [&unexplored_systems]() {
            std::string retval = "MapWnd::DispatchFleetsExploring Unknown System IDs:";
            for (auto system_id : unexplored_systems)
                retval += " " + std::to_string(system_id);
            return retval;
        }();

    std::multimap<double, FleetRouteType> fleet_routes;  // priority, (fleet, route)

    // Determine fleet routes for each unexplored system
    std::unordered_map<int, int> fleet_route_count;
    for (const auto* unexplored_system : objects.findRaw<System>(unexplored_systems)) {
        if (!unexplored_system)
            continue;

        for (const auto& fleet_id : idle_fleets) {
            if (max_routes_per_system > 0 &&
                fleet_route_count[unexplored_system->ID()] > max_routes_per_system)
            { break; }

            auto fleet = objects.getRaw<Fleet>(fleet_id);
            if (!fleet) {
                WarnLogger() << "Invalid fleet " << fleet_id;
                continue;
            }
            if (fleet->Fuel(objects) < 1.0f)
                continue;

            auto route = GetOrderedFleetRoute(fleet, unexplored_system, context);
            if (route.first > 0.0) {
                ++fleet_route_count[unexplored_system->ID()];
                fleet_routes.insert(std::move(route));
            }
        }
    }

    if (!fleet_routes.empty()) {
        TraceLogger() << [&fleet_routes]() {
            std::string retval = "MapWnd::DispatchFleetsExploring Explorable Systems:\n\t Priority\tFleet\tDestination";
            for (const auto& route : fleet_routes) {
                retval.append("\n\t" + std::to_string(route.first) + "\t" +
                              std::to_string(route.second.first) + "\t " +
                              std::to_string(route.second.second.empty() ? -1 : *route.second.second.rbegin()));
            }
            return retval;
        }();
    }

    // Issue fleet orders
    for (const auto& fleet_route : fleet_routes)
        IssueExploringFleetOrders(idle_fleets, systems_being_explored, fleet_route.second, context);

    // verify fleets have expected destination
    for (const auto& [dest_sys_id, fleet_id] : systems_being_explored) {
        auto fleet = objects.get<Fleet>(fleet_id);
        if (!fleet)
            continue;

        auto dest_id = fleet->FinalDestinationID();
        if (dest_id == dest_sys_id)
            continue;

        WarnLogger() << "Non idle exploring fleet "<< fleet_id << " has differing destination:"
                     << fleet->FinalDestinationID() << " expected:" << dest_sys_id;

        idle_fleets.emplace(fleet_id);
        // systems_being_explored.erase(system_fleet_it);
    }

    if (!idle_fleets.empty()) {
        DebugLogger() << [&idle_fleets]() {
            std::string retval = "MapWnd::DispatchFleetsExploring Idle exploring fleets after orders:";
            retval.reserve(retval.size() + 10*idle_fleets.size()); // rough guesstimate
            for (auto fleet_id : idle_fleets)
                retval.append(" ").append(std::to_string(fleet_id));
            return retval;
        }();
    }
}

void MapWnd::ShowAllPopups()
{ GG::ProcessThenRemoveExpiredPtrs(m_popups, [](auto& wnd) { wnd->Show(); }); }
