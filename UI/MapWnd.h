#ifndef _MapWnd_h_
#define _MapWnd_h_

#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/functional/hash.hpp>
#include <GG/GGFwd.h>
#include <GG/GLClientAndServerBuffer.h>
#include "CUISlider.h"
#include "CUIWnd.h"
#include "FleetButton.h"
#include "../universe/EnumsFwd.h"
#include "../universe/Fleet.h"


class FleetWnd;
class MapWndPopup;
class DesignWnd;
class ProductionWnd;
class ResearchWnd;
class GovernmentWnd;
class EncyclopediaDetailPanel;
class CombatReportWnd;
class ObjectListWnd;
class ModeratorActionsWnd;
struct SaveGameUIData;
class SidePanel;
class SitRepPanel;
class SystemIcon;
class FieldIcon;
class StatisticIcon;
class CUIToolBar;
class UniverseObject;
struct MovePathNode;
class ShaderProgram;

/* Start and end points in universe coordinates as seen in MapWnd.  Lanes are drawn to
 * and from a circle surrounding system icons, note the centre of the system icon. The
 * location of these start ane endpoints is used for rendering the starlane and for
 * positioning fleet buttons that are moving along the starlane. */
struct LaneEndpoints {
    constexpr LaneEndpoints() noexcept = default;
    constexpr LaneEndpoints(float x1, float y1, float x2, float y2) noexcept :
        X1(x1),
        Y1(y1),
        X2(x2),
        Y2(y2)
    {}
    float X1 = UniverseObject::INVALID_POSITION;
    float Y1 = UniverseObject::INVALID_POSITION;
    float X2 = UniverseObject::INVALID_POSITION;
    float Y2 = UniverseObject::INVALID_POSITION;
};


/** This class is a window that graphically displays everything in the universe */
class MapWnd : public GG::Wnd {
public:
    MapWnd();
    void CompleteConstruction() override;

    GG::Pt ClientUpperLeft() const noexcept override;

    double ZoomFactor() const;
    int    SystemIconSize() const;
    int    SystemNamePts() const;
    double SystemHaloScaleFactor() const;

    /** returns what size type (tiny, small, large) fleet buttons on this map
      * are shown at */
    FleetButton::SizeType FleetButtonSizeType() const;

    /** populates the relevant UI state that should be restored after a
      * save-and-load cycle */
    void GetSaveGameUIData(SaveGameUIData& data) const;

    /** returns true if MapWnd is visible and usable behind a production window.
     * MapWnd interactions are restricted to those appropriate to the production window */
    bool InProductionViewMode() const;

    /** returns true if MapWnd is visible and usable behind a research window.
     * MapWnd interactions are restricted to those appropriate to the research window.
     * Currently, there are no interactions with the MapWnd while the research window
     * is visible because although the MapWnd is visible the research window is opaque
     * and on top.*/
    bool InResearchViewMode() const;

    /** returns true if MapWnd is visible and usable behind a design window.
     * MapWnd interactions are restricted to those appropriate to the design window
     * Currently, there are no interactions with the MapWnd while the design window
     * is visible because although the MapWnd is visible the design window is opaque
     * and on top.*/
    bool InDesignViewMode() const;

    /** returns the currently set moderator action in this MapWnd's
      * ModeratorActionsWnd. */
    ModeratorActionSetting      GetModeratorActionSetting() const;

    bool AutoEndTurnEnabled() const;

    /** returns the position on the screen that corresponds to the specified
      * universe X and Y coordinates. */
    GG::Pt ScreenCoordsFromUniversePosition(double universe_x, double universe_y) const;
    /** returns the universe position (X and Y in pair) that corresponds to
      * the specified screen coordinates. */
    std::pair<double, double>   UniversePositionFromScreenCoords(GG::Pt screen_coords) const;

    /** Returns the id of the currently-selected object or INVALID_OBJECT_ID if no planet is selected */
    int SelectedSystemID() const;
    int SelectedPlanetID() const;
    int SelectedFleetID() const;
    int SelectedShipID() const;

    void PreRender() override;
    void Render() override;
    void LButtonDown(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDrag(GG::Pt pt, GG::Pt move, GG::Flags<GG::ModKey> mod_keys) override;
    void LButtonUp(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) override;
    void KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;
    void KeyRelease(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;
    void TimerFiring(unsigned int ticks, GG::Timer* timer) override;

    void DoLayout();

    void RegisterWindows();                                      //!< registers owned wnds with the GUI (also registers message & player list wnds)
    void RemoveWindows();                                        //!< removes owned wnds from the GUI (also removes message & player list wnds)

    void EnableOrderIssuing(bool enable = true);                 //!< enables or disables order issuing and pressing the turn button.

    void InitTurn(ScriptingContext& context);                    //!< called at the start of each turn
    void MidTurnUpdate();                                        //!< called after receiving updated Universe during turn processing, but not when the full turn update is received

    void RestoreFromSaveData(const SaveGameUIData& data);        //!< restores the UI state that was saved in an earlier call to GetSaveGameUIData().
    void ShowSystemNames();                                      //!< enables the system name text
    void HideSystemNames();                                      //!< disables the system name text

    mutable boost::signals2::signal<void (int)>    SystemLeftClickedSignal;
    mutable boost::signals2::signal<void (int)>    SystemRightClickedSignal;
    mutable boost::signals2::signal<void (int)>    SystemBrowsedSignal;
    mutable boost::signals2::signal<void (double)> ZoomedSignal;

    void CenterOnMapCoord(double x, double y);                   //!< centers the map on map position (x, y)
    void CenterOnObject(int id);                                 //!< centers the map on object with id \a id
    void CenterOnObject(const auto& obj)
    { if (obj) CenterOnMapCoord(obj->X(), obj->Y()); }

    void ShowPlanet(int planet_id);                       //!< brings up encyclopedia panel and displays info about the planet
    void ShowCombatLog(int log_id);                       //!< brings up encyclopedia panel and displays info about the combat
    void ShowTech(std::string tech_name);                 //!< brings up the research screen and centers the tech tree on \a tech_name
    void ShowPolicy(std::string policy_name);             //!< brings up ??? and displays info about the policy with name \a policy_name
    void ShowBuildingType(std::string building_type_name);//!< brings up the production screen and displays info about the buildtype \a type_name

    //! Brings up the production screen and displays info about
    //! the ShipPart @a ship_part_name.
    void ShowShipPart(std::string ship_part_name);

    //! Brings up the production screen and displays info about the ShipHull
    //! @p ship_hull_name
    void ShowShipHull(std::string ship_hull_name);

    void ShowShipDesign(int design_id);                  //!< brings up the production screen and displays info about the buildtype \a type_name
    void ShowSpecial(std::string special_name);          //!< brings up encyclopedia panel and displays info about the special with name \a special_name
    void ShowSpecies(std::string species_name);          //!< brings up encyclopedia panel and displays info about the species with name \a species_name
    void ShowFieldType(std::string field_type_name);     //!< brings up encyclopedia panel and displays info about the field type with name \a field_type_name
    void ShowEmpire(int empire_id);                      //!< brings up encyclopedia panel and displays info about the empire with id \a empire_id
    void ShowMeterTypeArticle(std::string meter_string); //!< brings up encyclopedia panel and displays info about the MeterType @a meter_type
    void ShowMeterTypeArticle(MeterType meter_type);     //!< brings up encyclopedia panel and displays info about the MeterType @a meter_type
    void ShowEncyclopediaEntry(std::string str);         //!< brings up encyclopedia panel and displays info about the specified string \a str

    void SelectSystem(int systemID); //!< programatically selects systems on map, sidepanel, and production screen.  catches signals from these when the user changes the selected system
    void ReselectLastSystem();       //!< re-selects the most recently selected system, if a valid one exists
    void SelectPlanet(int planetID, const ScriptingContext& context); //!< programatically selects planets on sidepanels.  catches signals from production wnd or sidepanel for when the user changes the selected planet
    void SelectPlanet(int planetID); //!< programatically selects planets on sidepanels.  catches signals from production wnd or sidepanel for when the user changes the selected planet
    void SelectFleet(int fleetID);   //!< programatically selects fleets by ID

    /** Programatically selects fleets. */
    void SelectFleet(const std::shared_ptr<Fleet>& fleet);
    void ReselectLastFleet();                   //!< re-selects the most recent selected fleet, if a valid one exists

    void RemoveFleet(int fleet_id);             //!< removes specified fleet.
    void SetFleetMovementLine(int fleet_id);    //!< creates fleet movement line for a single fleet.  Move lines originate from the fleet's button location.

    /* creates specially-coloured projected fleet movement line for specified
     * fleet following the specified route.  Move line originates from the
     * fleet's button location. */
    void SetProjectedFleetMovementLine(int fleet_id, const std::vector<int>& travel_route);
    /* creates specially-coloured projected fleet movement lines for specified
     * fleets following the specified route.  Move lines originates from the
     * fleets' button locations. */
    void SetProjectedFleetMovementLines(const std::vector<int>& fleet_ids, const std::vector<int>& travel_route);

    void ClearProjectedFleetMovementLines();     //!< removes all projected fleet movement lines

    /** Forget object with \p id.  Used for sensor ghosts. */
    void ForgetObject(int id);

    void ResetEmpireShown();                     //!< auto-resets the shown empire in any contained Wnds, to the current client's empire (if any)

    void RegisterPopup(std::shared_ptr<MapWndPopup>&& popup);   //!< registers a MapWndPopup, which can be cleaned up with a call to DeleteAllPopups( )
    void RemovePopup(MapWndPopup* popup);        //!< removes a MapWndPopup from the list cleaned up on a call to DeleteAllPopups( )
    void Sanitize();                             //!< sanitizes the MapWnd after a game
    void ResetTimeoutClock(int timeout);         //!< start count down \a timeout seconds

    void SetFleetExploring(const int fleet_id);
    void StopFleetExploring(const int fleet_id, ObjectMap& objects);
    bool IsFleetExploring(const int fleet_id) const;
    void DispatchFleetsExploring();              //!< called at each turn begin and when a fleet start/stop exploring to redispatch everyone.


private:
    void RefreshTurnButtonTooltip();

    void RefreshInfluenceResourceIndicator();
    void RefreshFleetResourceIndicator(const ScriptingContext& context, int empire_id);
    void RefreshResearchResourceIndicator();
    void RefreshIndustryResourceIndicator();
    void RefreshPopulationIndicator();
    void RefreshDetectionIndicator();

    /** recalculates production and predicted changes the specied empire's resource and population pools */
    void UpdateEmpireResourcePools(ScriptingContext& context, int empire_id);

    /** contains information necessary to render a single fleet movement line
      * on the main map. also contains cached infromation */
    struct MovementLineData {
        // apparent universe positions of move line points, derived from actual universe positions contained in MovePathNodes
        struct Vertex {
            constexpr Vertex(double x_, double y_, uint8_t eta_, bool show_eta_,
                             bool flag_blockade_ = false, bool flag_supply_block_ = false) noexcept :
                x(x_), y(y_), eta(eta_), show_eta(show_eta_),
                flag_blockade(flag_blockade_), flag_supply_block(flag_supply_block_)
            {}
            double  x, y;       // apparent in-universe position of a point on move line.  not actual universe positions, but rather where the move line vertices are drawn
            uint8_t eta;        // turns taken to reach point by object travelling along move line
            bool    show_eta;   // should an ETA indicator / number be shown over this vertex?
            bool    flag_blockade;
            bool    flag_supply_block;
        };
        MovementLineData() = default;
        MovementLineData(const std::vector<MovePathNode>& path_,
                         const std::map<std::pair<int, int>, LaneEndpoints>& lane_end_points_map,
                         GG::Clr colour_ = GG::CLR_WHITE, int empireID = ALL_EMPIRES);

        std::vector<MovePathNode> path;                  // raw path data from which line rendering is determined
        std::vector<Vertex>       vertices;              // cached apparent universe positions of starts and ends of line segments drawn to represent move path
        GG::Clr                   colour = GG::CLR_ZERO; // colour of line
    };

    void BufferAddMoveLineVertices(GG::GL2DVertexBuffer& dot_verts_buf,
                                   GG::GLRGBAColorBuffer& dot_colours_buf,
                                   GG::GLTexCoordBuffer& dot_star_texture_coords_buf,
                                   float offset, float dot_size, int dot_spacing,
                                   const MapWnd::MovementLineData& move_line,
                                   GG::Clr colour_override = GG::CLR_ZERO) const;

    class MapScaleLine;

    void InitializeWindows();

    void Zoom(int delta);                            //!< changes the zoom level of the main map by zoom step size to the power of \a delta (adds delta to the current zoom exponent)
    void Zoom(int delta, const GG::Pt position);     //!< changes the zoom level of the main map by zoom step size to the power of \a delta (adds delta to the current zoom exponent) Keeps the screen position \a position in the same place after zooming
    void SetZoom(double steps_in, bool update_slide);//!< sets zoom level of the main map to zoom step size to the power of \a steps_in and updates zoom slider position if \a update_slide is true
    void SetZoom(double steps_in, bool update_slide, const GG::Pt position);//!< sets zoom level of the main map to zoom step size to the power of \a steps_in and updates zoom slider position if \a update_slide is true. Keeps the screen position \a position in the same place after zooming

    void Pan(const GG::Pt delta);
    bool PanX(GG::X x);
    bool PanY(GG::Y y);

    /** Mark all fleet buttons for a refresh. */
    void RefreshFleetButtons(bool recreate = true);
    /** Removes old / existing and create new fleet buttons. Only called once
      * per render interval.*/
    void DeferredRefreshFleetButtons();

    /** Use the vectors of fleet ids from \p fleets_map to create fleet buttons
      * in \p type_fleet_buttons and record the fleet buttons in
      * \p m_fleet_buttons.*/
    template <typename FleetButtonMap, typename FleetsMap>
    void CreateFleetButtonsOfType(
        FleetButtonMap& type_fleet_buttons,
        const FleetsMap& fleets_map,
        const FleetButton::SizeType& fleet_button_size,
        const ObjectMap& objects);

    /** Delete all fleet buttons.*/
    void DeleteFleetButtons();

    void RefreshFleetButtonSelectionIndicators();        //!< marks (only) selected fleets' buttons as selected

    void DoFleetButtonsLayout(const ObjectMap& objects); //!< does layout of fleet buttons

    /** Return fleets ids of all fleet buttons containing or overlapping the
        fleet button for \p fleet_id. */
    std::vector<int> FleetIDsOfFleetButtonsOverlapping(int fleet_id,
                                                       const ScriptingContext& context,
                                                       int empire_id) const;
    /** Return fleets ids of all fleet buttons containing or overlapping \p fleet_btn. */
    std::vector<int> FleetIDsOfFleetButtonsOverlapping(const FleetButton& fleet_btn,
                                                       const ScriptingContext& context,
                                                       int empire_id) const;

    /** Returns position on map where a moving fleet should be displayed.  This
        is different from the fleet's actual universe position due to the
        squishing of fleets moving along a lane into the space between the
        system circles at the ends of the lane.  Return boost::none if the
        fleet has no valid screen position. */
    boost::optional<std::pair<double, double>> MovingFleetMapPositionOnLane(std::shared_ptr<const Fleet> fleet) const;

    void DoSystemIconsLayout(const ObjectMap& objects); //!< does layout of system icons
    void DoFieldIconsLayout(const ObjectMap& objects);  //!< does layout of field icons

    void RefreshSliders();               //!< shows or hides sliders on map

    void InitTurnRendering();            //!< sets up rendering of system icons, galaxy gas, starlanes at start of turn
    void InitSystemRenderingBuffers();   //!< initializes or refreshes buffers for rendering of system icons and galaxy gas
    void ClearSystemRenderingBuffers();
    void InitStarlaneRenderingBuffers(); //!< initializes or refreshes buffers for rendering of starlanes
    void ClearStarlaneRenderingBuffers();
    void InitFieldRenderingBuffers();
    void ClearFieldRenderingBuffers();
    void InitVisibilityRadiiRenderingBuffers();
    void ClearVisibilityRadiiRenderingBuffers();
    void InitScaleCircleRenderingBuffer(const ObjectMap& objects);
    void ClearScaleCircleRenderingBuffer();
    void ClearStarfieldRenderingBuffers();

    void RenderStarfields();     //!< renders the background starfiends
    void RenderGalaxyGas();      //!< renders gassy substance to make shape of galaxy
    void RenderSystemOverlays(); //!< renders textures "overlayed" on systems by effects
    void RenderSystems();        //!< renders stars and halos
    void RenderStarlanes();      //!< renders the starlanes between the systems
    void RenderStarlanes(GG::GL2DVertexBuffer& vertices,
                         GG::GLRGBAColorBuffer& colours,
                         double thickness, bool coloured, bool do_base_render);
    void RenderFields();         //!< renders field objects

    /* renders the dashed lines indicating where each fleet is going */
    void RenderFleetMovementLines();

    /* renders ETA indicators at end-of-turn positions for a single fleet movement
     * line.  if \a clr is GG::CLR_ZERO, the indicators are filled with the .colour
     * attribute of \a move_line */
    void RenderMovementLineETAIndicators(const MapWnd::MovementLineData& move_line, GG::Clr clr = GG::CLR_ZERO);

    /* renders circles around objects' locations indicating distance they have
     * visibility */
    void RenderVisibilityRadii();

    /* renders scale circle around selected system. */
    void RenderScaleCircle();

    void CorrectMapPosition(GG::Pt& move_to_pt);     //!< constrains \a move_to_pt so that if the map is repositioned to that location, it will not be problematically positioned, so that galaxy contents remain visible

    void FieldRightClicked(int field_id);

    void SystemDoubleClicked(int system_id);
    void SystemLeftClicked(int system_id);
    void SystemRightClicked(int system_id, GG::Flags< GG::ModKey > mod_keys);
    void MouseEnteringSystem(int system_id, GG::Flags< GG::ModKey > mod_keys);
    void MouseLeavingSystem(int system_id);

    void PlanetDoubleClicked(int planet_id);
    void PlanetRightClicked(int planet_id);
    void BuildingRightClicked(int building_id);

    void ReplotProjectedFleetMovement(bool append); //!< Find any projected movement plots and replots them with the current append state
    void PlotFleetMovement(int system_id, bool execute_move, bool append);   //!< issues fleet move orders to appropriate fleets in active FleetWnd

    void FleetButtonLeftClicked(const FleetButton* fleet_btn);
    void FleetButtonRightClicked(const FleetButton* fleet_btn);
    void FleetRightClicked(int fleet_id);
    void FleetsRightClicked(const std::vector<int>& fleet_ids);

    void ShipRightClicked(int fleet_id);
    void ShipsRightClicked(const std::vector<int>& fleet_ids);

    void UniverseObjectDeleted(const std::shared_ptr<const UniverseObject>& obj);

    void PushWndStack(std::shared_ptr<GG::Wnd> wnd);
    void RemoveFromWndStack(std::shared_ptr<GG::Wnd> wnd);
    bool ReturnToMap();

    bool RevertOrders();

    bool EndTurn();
    void ToggleAutoEndTurn();

    bool ToggleModeratorActions();
    void ShowModeratorActions();
    void HideModeratorActions();

    bool ToggleMessages();
    void ShowMessages();
    void HideMessages();

    bool ToggleSitRep();
    void ShowSitRep();
    void HideSitRep();

    bool ToggleEmpires();
    void ShowEmpires();
    void HideEmpires();

    bool ToggleObjects();
    void ShowObjects();
    void HideObjects();

    bool TogglePedia();
    void ShowPedia();
    void HidePedia();

    bool ShowGraphs();

    void HideSidePanelAndRememberIfItWasVisible();
    void RestoreSidePanel();

    bool ToggleResearch(const ScriptingContext& context);
    void ShowResearch(const ScriptingContext& context);
    void HideResearch();

    bool ToggleProduction();
    void ShowProduction();
    void HideProduction();

    bool ToggleDesign();
    void ShowDesign();
    void HideDesign();

    bool ToggleGovernment();
    void ShowGovernment();
    void HideGovernment();

    bool ShowMenu();

    bool CloseSystemView(); //!< closes off the current system view

    bool KeyboardZoomIn();
    bool KeyboardZoomOut();

    bool ZoomToHomeSystem();
    bool ZoomToPrevOwnedSystem();
    bool ZoomToNextOwnedSystem();
    bool ZoomToPrevSystem();
    bool ZoomToNextSystem();
    bool ZoomToPrevIdleFleet();
    bool ZoomToNextIdleFleet();
    bool ZoomToPrevFleet();
    bool ZoomToNextFleet();
    bool ZoomToSystemWithWastedPP();

    void ConnectKeyboardAcceleratorSignals();   //!< connects signals from keyboard accelerators to various GUI responses

    void CloseAllPopups();
    void HideAllPopups();
    void ShowAllPopups();

    void SelectedFleetsChanged();
    void SelectedShipsChanged();

    std::set<int>                               m_selected_fleet_ids;
    std::set<int>                               m_selected_ship_ids;

    double                                      m_zoom_steps_in = 1.0;      //!< number of zoom steps in.  each 1.0 step increases display scaling by the same zoom step factor
    std::shared_ptr<SidePanel>                  m_side_panel;               //!< planet view panel on the side of the main map
    std::unordered_map<int, std::shared_ptr<SystemIcon>> m_system_icons;    //!< system icons in the main map, indexed by system id
    std::vector<std::shared_ptr<FieldIcon>>     m_field_icons;              //!< field icons in the main map, sorted by field size
    std::shared_ptr<SitRepPanel>                m_sitrep_panel;             //!< sitrep panel
    std::shared_ptr<ResearchWnd>                m_research_wnd;             //!< research screen
    std::shared_ptr<ProductionWnd>              m_production_wnd;           //!< production screen
    std::shared_ptr<DesignWnd>                  m_design_wnd;               //!< design screen
    std::shared_ptr<GovernmentWnd>              m_government_wnd;           //!< government screen
    std::shared_ptr<EncyclopediaDetailPanel>    m_pedia_panel;              //!< encyclpedia panel
    std::shared_ptr<ObjectListWnd>              m_object_list_wnd;          //!< filterable list of objects in universe
    std::shared_ptr<ModeratorActionsWnd>        m_moderator_wnd;            //!< buttons to select moderator actions
    std::shared_ptr<CombatReportWnd>            m_combat_report_wnd;        //!< shows graphical reports of combats

    std::vector<std::weak_ptr<GG::Wnd>>         m_wnd_stack;                //!< stack of open windows, to allow closing them with escape in a LIFO order

    std::map<std::pair<int, int>, LaneEndpoints>m_starlane_endpoints;       //!< map from starlane start and end system IDs (stored in pair in increasing order) to the universe coordiates at which to draw the starlane ends

    /** Icons representing fleets at a system that are not departing, indexed
        by system. */
    std::unordered_map<int, std::unordered_set<std::shared_ptr<FleetButton>>>
        m_stationary_fleet_buttons;

    /** Icons representing fleets at a system that are departing, indexed by
        system. */
    std::unordered_map<int, std::unordered_set<std::shared_ptr<FleetButton>>>
        m_departing_fleet_buttons;

    /** Sets of fleet ids of fleets moving on a starlane, keyed by starlane end
        system ids. */
    std::unordered_map<std::pair<int, int>,
                       std::vector<int>,
                       boost::hash<std::pair<int, int>>>
        m_moving_fleets;

    /** Icons representing fleets moving on a starlane, keyed by starlane end
        system ids. */
    std::unordered_map<std::pair<int, int>,
                       std::unordered_set<std::shared_ptr<FleetButton>>,
                       boost::hash<std::pair<int, int>>>
        m_moving_fleet_buttons;

    /** Icons representing fleets moving and not on a starlane, indexed by
        (x,y) location. */
    std::unordered_map<std::pair<double, double>,
                       std::unordered_set<std::shared_ptr<FleetButton>>,
                       boost::hash<std::pair<double, double>>>
        m_offroad_fleet_buttons;

    std::unordered_map<int, std::shared_ptr<FleetButton>>
        m_fleet_buttons;                        //!< fleet icons, index by fleet

    std::unordered_map<int, boost::signals2::scoped_connection>
        m_fleet_state_change_signals;
    std::unordered_map<int, std::vector<boost::signals2::scoped_connection>>
        m_system_fleet_insert_remove_signals;

    std::map<int, MovementLineData> m_fleet_lines;                  //!< lines used for moving fleets in the main map
    std::map<int, MovementLineData> m_projected_fleet_lines;        //!< lines that show the projected path of the active fleet in the FleetWnd

    std::map<std::shared_ptr<GG::Texture>, GG::GL2DVertexBuffer> m_star_core_quad_vertices;
    std::map<std::shared_ptr<GG::Texture>, GG::GL2DVertexBuffer> m_star_halo_quad_vertices;
    GG::GL2DVertexBuffer    m_galaxy_gas_quad_vertices;
    GG::GLTexCoordBuffer    m_galaxy_gas_texture_coords;
    GG::GLTexCoordBuffer    m_star_texture_coords;
    GG::GL2DVertexBuffer    m_star_circle_vertices;

    GG::GL2DVertexBuffer    m_starlane_vertices;
    GG::GLRGBAColorBuffer   m_starlane_colors;
    GG::GL2DVertexBuffer    m_RC_starlane_vertices;
    GG::GLRGBAColorBuffer   m_RC_starlane_colors;

    GG::GL2DVertexBuffer    m_fleet_move_dot_vertices;
    GG::GLRGBAColorBuffer   m_fleet_move_dot_colours;
    GG::GLTexCoordBuffer    m_fleet_move_dot_star_texture_coords;
    std::size_t             m_projected_move_dots_start_index = 0;

    GG::GL2DVertexBuffer    m_scanline_circle_vertices;
    GG::GL2DVertexBuffer    m_system_circle_vertices;
    GG::GLRGBAColorBuffer   m_system_circle_colours;

    std::vector<std::pair<std::shared_ptr<GG::Texture>, GG::GL2DVertexBuffer>> m_field_vertices_visible;
    std::vector<std::pair<std::shared_ptr<GG::Texture>, GG::GL2DVertexBuffer>> m_field_vertices_not_visible;

    GG::GL2DVertexBuffer            m_field_scanline_circles;
    GG::GLTexCoordBuffer            m_field_texture_coords;

    GG::GL2DVertexBuffer            m_visibility_radii_vertices;
    GG::GLRGBAColorBuffer           m_visibility_radii_colors;
    GG::GL2DVertexBuffer            m_visibility_radii_border_vertices;
    GG::GLRGBAColorBuffer           m_visibility_radii_border_colors;
    std::vector<std::pair<std::pair<std::size_t, std::size_t>,
                          std::pair<std::size_t, std::size_t>>>
                                    m_radii_radii_vertices_indices_runs;

    GG::GL2DVertexBuffer            m_scale_circle_vertices;

    GG::GL3DVertexBuffer            m_starfield_verts;
    GG::GLRGBAColorBuffer           m_starfield_colours;

    ScanlineRenderer                m_scanline_shader;

    GG::Pt                          m_drag_offset = {-GG::X1, -GG::Y1}; //!< distance the cursor is from the upper-left corner of the window during a drag ((-1, -1) if no drag is occurring)
    bool                            m_dragged = false;          //!< tracks whether or not a drag occurs during a left button down sequence of events

    std::shared_ptr<GG::Button>     m_btn_turn;                 //!< button that updates player's turn;
    std::shared_ptr<GG::Button>     m_btn_auto_turn;            //!< button that toggles whether to automatically end turns;
    bool                            m_auto_end_turn = false;    //!< should turns be ended automatically by this client?
    bool                            m_ready_turn = false;       //!< is turn orders are ready and sent to server?
    std::shared_ptr<GG::Label>      m_timeout_remain;           //!< label to show remaining time
    GG::Timer                       m_timeout_clock{1000};      //!< clock to update remaining time
    std::vector<std::weak_ptr<MapWndPopup>> m_popups;           //!< currently active popup windows
    bool                            m_menu_showing = false;     //!< set during ShowMenu() to prevent reentrency
    int                             m_current_owned_system = INVALID_OBJECT_ID;
    int                             m_current_fleet_id = INVALID_OBJECT_ID;
    bool                            m_in_production_view_mode = false;

    bool                            m_sidepanel_open_before_showing_other = false;  //!< was the sidepanel open before switching to production, research or design screens?  If so, it should be restored when leaving them.

    std::shared_ptr<CUIToolBar>     m_toolbar;
    std::shared_ptr<StatisticIcon>  m_influence, m_population, m_research,
                                    m_industry, m_stockpile, m_detection,
                                    m_fleet;
    std::shared_ptr<GG::Button>     m_industry_wasted, m_research_wasted,
                                    m_btn_moderator, m_btn_messages, m_btn_empires,
                                    m_btn_siterep, m_btn_research, m_btn_production,
                                    m_btn_design, m_btn_government, m_btn_pedia,
                                    m_btn_graphs, m_btn_objects, m_btn_menu;
    std::shared_ptr<GG::Label>      m_FPS;

    std::shared_ptr<MapScaleLine>       m_scale_line;   //!< indicates the on-screen distance that reprensents an in-universe distance
    std::shared_ptr<GG::Slider<double>> m_zoom_slider;  //!< allows user to set zoom level;

    boost::signals2::scoped_connection  m_slider_show_connection;
    boost::signals2::scoped_connection  m_obj_delete_connection;

    std::set<int>                   m_fleets_exploring;

    /// indicates that refresh fleet button work should be done before rendering.
    bool                            m_deferred_recreate_fleet_buttons = false;
    bool                            m_deferred_refresh_fleet_buttons = false;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_timeout_time;

    friend struct IntroMenu;
    friend struct WaitingForGameStart;
    friend struct PlayingTurn;
    friend struct PlayingGame;
};


/** Derive any window from this class to have it managed by MapWnd. */
class MapWndPopup : public CUIWnd {
public:
    MapWndPopup(std::string t, GG::X default_x, GG::Y default_y, GG::X default_w, GG::Y default_h,
                GG::Flags<GG::WndFlag> flags, std::string_view config_name = "");

    MapWndPopup(std::string t, GG::Flags<GG::WndFlag> flags, std::string_view config_name = "");

    void CompleteConstruction() override;

    virtual ~MapWndPopup();

    void CloseClicked() override;

    void    Close();
};


#endif
