#ifndef _MapWnd_h_
#define _MapWnd_h_

#include <GG/GGFwd.h>
#include <GG/GLClientAndServerBuffer.h>

#include "CUIWnd.h"
#include "CUISlider.h"
#include "../universe/EnumsFwd.h"
#include "../universe/Fleet.h"
#include "FleetButton.h"

#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

#include <unordered_map>
#include <unordered_set>
#include <vector>


class FleetWnd;
class MapWndPopup;
class DesignWnd;
class ProductionWnd;
class ResearchWnd;
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
    LaneEndpoints();
    LaneEndpoints(float x1, float y1, float x2, float y2) :
        X1(x1),
        Y1(y1),
        X2(x2),
        Y2(y2)
    {}
    float X1, Y1, X2, Y2;
};


/** This class is a window that graphically displays everything in the universe */
class MapWnd : public GG::Wnd {
public:
    //! \name Structors //!@{
    MapWnd();

    ~MapWnd();
    //!@}

    void CompleteConstruction() override;

    //! \name Accessors //!@{
    GG::Pt ClientUpperLeft() const override;

    double                      ZoomFactor() const;
    int                         SystemIconSize() const;
    int                         SystemNamePts() const;
    double                      SystemHaloScaleFactor() const;

    /** returns what size type (tiny, small, large) fleet buttons on this map
      * are shown at */
    FleetButton::SizeType       FleetButtonSizeType() const;

    /** populates the relevant UI state that should be restored after a
      * save-and-load cycle */
    void                        GetSaveGameUIData(SaveGameUIData& data) const;

    /** returns true if MapWnd is visible and usable behind a production window.
     * MapWnd interactions are restricted to those appropriate to the production window */
    bool                        InProductionViewMode() const;

    /** returns true if MapWnd is visible and usable behind a research window.
     * MapWnd interactions are restricted to those appropriate to the research window.
     * Currently, there are no interactions with the MapWnd while the research window
     * is visible because although the MapWnd is visible the research window is opaque
     * and on top.*/
    bool                        InResearchViewMode() const;

    /** returns true if MapWnd is visible and usable behind a design window.
     * MapWnd interactions are restricted to those appropriate to the design window
     * Currently, there are no interactions with the MapWnd while the design window
     * is visible because although the MapWnd is visible the design window is opaque
     * and on top.*/
    bool                        InDesignViewMode() const;

    /** returns the currently set moderator action in this MapWnd's
      * ModeratorActionsWnd. */
    ModeratorActionSetting      GetModeratorActionSetting() const;

    bool                        AutoEndTurnEnabled() const;

    /** returns the position on the screen that corresponds to the specified
      * universe X and Y coordinates. */
    GG::Pt                      ScreenCoordsFromUniversePosition(double universe_x, double universe_y) const;
    /** returns the universe position (X and Y in pair) that corresponds to
      * the specified screen coordinates. */
    std::pair<double, double>   UniversePositionFromScreenCoords(GG::Pt screen_coords) const;

    /** Returns the id of the currently-selected planet, or
      * INVALID_OBJECT_ID if no planet is selected */
    int                         SelectedPlanetID() const;
    //!@}

    //! \name Mutators //!@{
    void PreRender() override;
    void Render() override;
    void LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys) override;
    void LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) override;
    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;
    void KeyRelease(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;

    void DoLayout();

    void RegisterWindows();                                      //!< registers owned wnds with the GUI (also registers message & player list wnds)
    void RemoveWindows();                                        //!< removes owned wnds from the GUI (also removes message & player list wnds)

    void EnableOrderIssuing(bool enable = true);                 //!< enables or disables order issuing and pressing the turn button.

    void InitTurn();                                             //!< called at the start of each turn
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

    /** Centers the map on object \a id. */
    void CenterOnObject(std::shared_ptr<const UniverseObject> obj);

    void ShowPlanet(int planet_id);                              //!< brings up encyclopedia panel and displays info about the planet
    void ShowCombatLog(int log_id);                              //!< brings up encyclopedia panel and displays info about the combat
    void ShowTech(const std::string& tech_name);                 //!< brings up the research screen and centers the tech tree on \a tech_name
    void ShowBuildingType(const std::string& building_type_name);//!< brings up the production screen and displays info about the buildtype \a type_name
    void ShowPartType(const std::string& part_type_name);        //!< brings up the production screen and displays info about the buildtype \a type_name
    void ShowHullType(const std::string& hull_type_name);        //!< brings up the production screen and displays info about the buildtype \a type_name
    void ShowShipDesign(int design_id);                          //!< brings up the production screen and displays info about the buildtype \a type_name
    void ShowSpecial(const std::string& special_name);           //!< brings up encyclopedia panel and displays info about the special with name \a special_name
    void ShowSpecies(const std::string& species_name);           //!< brings up encyclopedia panel and displays info about the species with name \a species_name
    void ShowFieldType(const std::string& field_type_name);      //!< brings up encyclopedia panel and displays info about the field type with name \a field_type_name
    void ShowEmpire(int empire_id);                              //!< brings up encyclopedia panel and displays info about the empire with id \a empire_id
    void ShowMeterTypeArticle(const std::string& meter_string);  //!< brings up encyclopedia panel and displays info about the MeterType @a meter_type
    void ShowEncyclopediaEntry(const std::string& str);          //!< brings up encyclopedia panel and displays info about the specified string \a str

    void SelectSystem(int systemID); //!< programatically selects systems on map, sidepanel, and production screen.  catches signals from these when the user changes the selected system
    void ReselectLastSystem();       //!< re-selects the most recently selected system, if a valid one exists
    void SelectPlanet(int planetID); //!< programatically selects planets on sidepanels.  catches signals from production wnd or sidepanel for when the user changes the selected planet
    void SelectFleet(int fleetID);   //!< programatically selects fleets by ID

    /** Programatically selects fleets. */
    void SelectFleet(std::shared_ptr<Fleet> fleet);

    void ReselectLastFleet();                    //!< re-selects the most recent selected fleet, if a valid one exists

    void RemoveFleet(int fleet_id); //!< removes specified fleet.
    void SetFleetMovementLine(int fleet_id);     //!< creates fleet movement line for a single fleet.  Move lines originate from the fleet's button location.

    /* creates specially-coloured projected fleet movement line for specified
     * fleet following the specified route.  Move line originates from the
     * fleet's button location. */
    void SetProjectedFleetMovementLine(int fleet_id, const std::list<int>& travel_route);
    /* creates specially-coloured projected fleet movement lines for specified
     * fleets following the specified route.  Move lines originates from the
     * fleets' button locations. */
    void SetProjectedFleetMovementLines(const std::vector<int>& fleet_ids, const std::list<int>& travel_route);

    void ClearProjectedFleetMovementLines();     //!< removes all projected fleet movement lines

    /** Forget object with \p id.  Used for sensor ghosts. */
    void ForgetObject(int id);

    void ResetEmpireShown();                     //!< auto-resets the shown empire in any contained Wnds, to the current client's empire (if any)

    void RegisterPopup(const std::shared_ptr<MapWndPopup>& popup);              //!< registers a MapWndPopup, which can be cleaned up with a call to DeleteAllPopups( )
    void RemovePopup(MapWndPopup* popup);        //!< removes a MapWndPopup from the list cleaned up on a call to DeleteAllPopups( )
    void Sanitize();                             //!< sanitizes the MapWnd after a game
    //!@}

    void SetFleetExploring(const int fleet_id);
    void StopFleetExploring(const int fleet_id);
    bool IsFleetExploring(const int fleet_id);
    void DispatchFleetsExploring();              //!< called at each turn begin and when a fleet start/stop exploring to redispatch everyone.


private:
    void RefreshTradeResourceIndicator();
    void RefreshFleetResourceIndicator();
    void RefreshResearchResourceIndicator();
    void RefreshIndustryResourceIndicator();
    void RefreshPopulationIndicator();
    void RefreshDetectionIndicator();

    /** update meter estimates for objects contained within the current system
      * shown in the sidepanel, or all objects if there is no system shown */
    void UpdateSidePanelSystemObjectMetersAndResourcePools();
    /** recalculates production and predicted changes of player's empire's
      * resource and population pools */
    void UpdateEmpireResourcePools();

    /** contains information necessary to render a single fleet movement line
      * on the main map. also contains cached infromation */
    struct MovementLineData {
        struct Vertex;  // apparent universe positions of move line points, derived from actual universe positions contained in MovePathNodes
        MovementLineData();
        MovementLineData(const std::list<MovePathNode>& path_,
                         const std::map<std::pair<int, int>, LaneEndpoints>& lane_end_points_map,
                         GG::Clr colour_ = GG::CLR_WHITE, int empireID = ALL_EMPIRES);

        std::list<MovePathNode> path;       // raw path data from which line rendering is determined
        GG::Clr                 colour;     // colour of line
        std::vector<Vertex>     vertices;   // cached apparent universe positions of starts and ends of line segments drawn to represent move path
    };

    class MapScaleLine;

    void            InitializeWindows();

    void            Zoom(int delta);                            //!< changes the zoom level of the main map by zoom step size to the power of \a delta (adds delta to the current zoom exponent)
    void            Zoom(int delta, const GG::Pt& position);    //!< changes the zoom level of the main map by zoom step size to the power of \a delta (adds delta to the current zoom exponent) Keeps the screen position \a position in the same place after zooming
    void            SetZoom(double steps_in, bool update_slide);//!< sets zoom level of the main map to zoom step size to the power of \a steps_in and updates zoom slider position if \a update_slide is true
    void            SetZoom(double steps_in, bool update_slide, const GG::Pt& position);//!< sets zoom level of the main map to zoom step size to the power of \a steps_in and updates zoom slider position if \a update_slide is true. Keeps the screen position \a position in the same place after zooming

    void            Pan(const GG::Pt& delta);                   //!< pans map
    bool            PanX(GG::X x = GG::X(50));
    bool            PanY(GG::Y y = GG::Y(50));

    /** Mark all fleet buttons for a refresh. */
    void            RefreshFleetButtons();
    /** Removes old / existing and create new fleet buttons. Only called once
      * per render interval.*/
    void            DeferredRefreshFleetButtons();

    /** Use the vectors of fleet ids from \p fleets_map to create fleet buttons
      * in \p type_fleet_buttons and record the fleet buttons in
      * \p m_fleet_buttons.*/
    template <typename FleetButtonMap, typename FleetsMap>
    void CreateFleetButtonsOfType (
        FleetButtonMap& type_fleet_buttons,
        const FleetsMap &fleets_map,
        const FleetButton::SizeType & fleet_button_size);

    /** Delete all fleet buttons.*/
    void DeleteFleetButtons();

    void RefreshFleetButtonSelectionIndicators();    //!< marks (only) selected fleets' buttons as selected

    /** Connect all \p fleets StateChangedSignal to RefreshFleetButtons. */
    void AddFleetsStateChangedSignal(const std::vector<std::shared_ptr<Fleet>>& fleets);

    /** Disconnect all \p fleets StateChangedSignal from RefreshFleetButtons. */
    void RemoveFleetsStateChangedSignal(const std::vector<std::shared_ptr<Fleet>>& fleets);

    /** Handle FleetsInsertedSignal by connecting signals and refreshing fleet
        buttons. */
    void FleetsInsertedSignalHandler(const std::vector<std::shared_ptr<Fleet>>& fleets);

    /** Handle FleetsRemovedSignal by disconnecting signals and refreshing fleet
        buttons. */
    void FleetsRemovedSignalHandler(const std::vector<std::shared_ptr<Fleet>>& fleets);


    void DoFleetButtonsLayout();                     //!< does layout of fleet buttons

    /** Return fleets ids of all fleet buttons containing or overlapping the
        fleet button for \p fleet_id. */
    std::vector<int> FleetIDsOfFleetButtonsOverlapping(int fleet_id) const;
    /** Return fleets ids of all fleet buttons containing or overlapping \p fleet_btn. */
    std::vector<int> FleetIDsOfFleetButtonsOverlapping(const FleetButton& fleet_btn) const;

    /** Returns position on map where a moving fleet should be displayed.  This
        is different from the fleet's actual universe position due to the
        squishing of fleets moving along a lane into the space between the
        system circles at the ends of the lane.  Return boost::none if the
        fleet has no valid screen position. */
    boost::optional<std::pair<double, double>> MovingFleetMapPositionOnLane(std::shared_ptr<const Fleet> fleet) const;

    void DoSystemIconsLayout();          //!< does layout of system icons
    void DoFieldIconsLayout();           //!< does layout of field icons

    void RefreshFleetSignals();          //!< disconnects and reconnects all fleet change signals

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
    void InitScaleCircleRenderingBuffer();
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

    /* renders a single fleet movement line. if \a clr is GG::CLR_ZERO, the lane
     * is rendered with the .colour attribute of \a move_line. assumes that the
     * move dot texture has already been bound. */
    void RenderMovementLine(const MapWnd::MovementLineData& move_line, float dot_size, float dot_spacing, float dot_shift,
                            GG::Clr clr = GG::CLR_ZERO);

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

    void UniverseObjectDeleted(std::shared_ptr<const UniverseObject> obj);

    void PushWndStack(std::shared_ptr<GG::Wnd> wnd);
    void RemoveFromWndStack(std::shared_ptr<GG::Wnd> wnd);
    bool ReturnToMap();

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

    void HideSidePanel();
    void RestoreSidePanel(); //!< restores side panel, sends to server changes made in other windows

    bool ToggleResearch();
    void ShowResearch();
    void HideResearch();

    bool ToggleProduction();
    void ShowProduction();
    void HideProduction();

    bool ToggleDesign();
    void ShowDesign();
    void HideDesign();

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

    double                                      m_zoom_steps_in = 1.0;          //!< number of zoom steps in.  each 1.0 step increases display scaling by the same zoom step factor
    std::shared_ptr<SidePanel>                  m_side_panel = nullptr;         //!< planet view panel on the side of the main map
    boost::unordered_map<int, std::shared_ptr<SystemIcon>> m_system_icons;      //!< system icons in the main map, indexed by system id
    std::map<int, std::shared_ptr<FieldIcon>>   m_field_icons;                  //!< field icons in the main map, indexed by field id
    std::shared_ptr<SitRepPanel>                m_sitrep_panel = nullptr;       //!< sitrep panel
    std::shared_ptr<ResearchWnd>                m_research_wnd = nullptr;       //!< research screen
    std::shared_ptr<ProductionWnd>              m_production_wnd = nullptr;     //!< production screen
    std::shared_ptr<DesignWnd>                  m_design_wnd = nullptr;         //!< design screen
    std::shared_ptr<EncyclopediaDetailPanel>    m_pedia_panel = nullptr;        //!< encyclpedia panel
    std::shared_ptr<ObjectListWnd>              m_object_list_wnd = nullptr;    //!< filterable list of objects in universe
    std::shared_ptr<ModeratorActionsWnd>        m_moderator_wnd = nullptr;      //!< buttons to select moderator actions
    std::shared_ptr<CombatReportWnd>            m_combat_report_wnd = nullptr;  //!< shows graphical reports of combats

    std::vector<std::weak_ptr<GG::Wnd>>         m_wnd_stack;        //!< stack of open windows, to allow closing them with escape in a LIFO order

    std::map<std::pair<int, int>, LaneEndpoints>m_starlane_endpoints;//!< map from starlane start and end system IDs (stored in pair in increasing order) to the universe coordiates at which to draw the starlane ends

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

    boost::unordered_map<int, std::shared_ptr<FleetButton>>
        m_fleet_buttons;                        //!< fleet icons, index by fleet

    boost::unordered_map<int, boost::signals2::connection>
        m_fleet_state_change_signals;
    boost::unordered_map<int, std::vector<boost::signals2::connection>>
        m_system_fleet_insert_remove_signals;

    std::map<int, MovementLineData> m_fleet_lines;                  //!< lines used for moving fleets in the main map
    std::map<int, MovementLineData> m_projected_fleet_lines;        //!< lines that show the projected path of the active fleet in the FleetWnd

    std::pair<int, int>             m_line_between_systems;         //!< set when map should render line connecting 2 systems

    std::map<std::shared_ptr<GG::Texture>, GG::GL2DVertexBuffer> m_star_core_quad_vertices;
    std::map<std::shared_ptr<GG::Texture>, GG::GL2DVertexBuffer> m_star_halo_quad_vertices;
    GG::GL2DVertexBuffer            m_galaxy_gas_quad_vertices;
    GG::GLTexCoordBuffer            m_galaxy_gas_texture_coords;
    GG::GLTexCoordBuffer            m_star_texture_coords;
    GG::GL2DVertexBuffer            m_star_circle_vertices;

    GG::GL2DVertexBuffer            m_starlane_vertices;
    GG::GLRGBAColorBuffer           m_starlane_colors;
    GG::GL2DVertexBuffer            m_RC_starlane_vertices;
    GG::GLRGBAColorBuffer           m_RC_starlane_colors;

    /** First buffer is visible fields, second buffer is not visible (scanlined)
        fields for each texture. */
    std::map<std::shared_ptr<GG::Texture>,
             std::pair<GG::GL2DVertexBuffer, GG::GL2DVertexBuffer>>
                                    m_field_vertices;

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

    std::shared_ptr<GG::Button>     m_btn_turn = nullptr;       //!< button that updates player's turn;
    std::shared_ptr<GG::Button>     m_btn_auto_turn = nullptr;  //!< button that toggles whether to automatically end turns;
    bool                            m_auto_end_turn = false;    //!< should turns be ended automatically by this client?
    bool                            m_ready_turn = false;       //!< is turn orders are ready and sent to server?
    std::list<std::weak_ptr<MapWndPopup>> m_popups;             //!< list of currently active popup windows
    bool                            m_menu_showing = false;     //!< set during ShowMenu() to prevent reentrency
    int                             m_current_owned_system;
    int                             m_current_fleet_id;
    bool                            m_in_production_view_mode = false;

    bool                            m_sidepanel_open_before_showing_other = false;  //!< was the sidepanel open before switching to production, research or design screens?  If so, it should be restored when leaving them.

    std::shared_ptr<CUIToolBar>     m_toolbar = nullptr;
    std::shared_ptr<StatisticIcon>  m_trade = nullptr, m_population = nullptr, m_research = nullptr,
                                    m_industry = nullptr, m_stockpile = nullptr, m_detection = nullptr,
                                    m_fleet = nullptr;
    std::shared_ptr<GG::Button>     m_industry_wasted = nullptr, m_research_wasted = nullptr,
                                    m_btn_moderator = nullptr, m_btn_messages = nullptr, m_btn_empires = nullptr,
                                    m_btn_siterep = nullptr, m_btn_research = nullptr, m_btn_production = nullptr,
                                    m_btn_design = nullptr, m_btn_pedia = nullptr, m_btn_graphs = nullptr,
                                    m_btn_objects = nullptr, m_btn_menu = nullptr;
    std::shared_ptr<GG::Label>      m_FPS = nullptr;

    std::shared_ptr<MapScaleLine>       m_scale_line = nullptr;     //!< indicates the on-screen distance that reprensents an in-universe distance
    std::shared_ptr<GG::Slider<double>> m_zoom_slider = nullptr;    //!< allows user to set zoom level;

    std::set<int>                   m_fleets_exploring;

    /// indicates that refresh fleet button work should be done before rendering.
    bool                            m_deferred_refresh_fleet_buttons = false;

    friend struct IntroMenu;
    friend struct WaitingForGameStart;
    friend struct PlayingTurn;
    friend struct PlayingGame;
};


/** Derive any window from this class to have it managed by MapWnd. */
class MapWndPopup : public CUIWnd {
public:
    MapWndPopup(const std::string& t, GG::X default_x, GG::Y default_y, GG::X default_w, GG::Y default_h,
                GG::Flags<GG::WndFlag> flags, const std::string& config_name = "");

    MapWndPopup(const std::string& t, GG::Flags<GG::WndFlag> flags, const std::string& config_name = "");

    void CompleteConstruction() override;

    virtual ~MapWndPopup();

    void CloseClicked() override;

    void    Close();
};

#endif // _MapWnd_h_
