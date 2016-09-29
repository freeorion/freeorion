#ifndef _MapWnd_h_
#define _MapWnd_h_

#include <vector>
#include <GG/GGFwd.h>
#include <GG/GLClientAndServerBuffer.h>

#include "CUIWnd.h"
#include "CUISlider.h"
#include "../universe/Enums.h"
#include "../universe/Fleet.h"
#include "FleetButton.h"

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

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
    float X1, Y1, X2, Y2;
};


/** This class is a window that graphically displays everything in the universe */
class MapWnd : public GG::Wnd {
public:
    //! \name Structors //!@{
    MapWnd();    //!< default ctor
    ~MapWnd();   //!< default dtor
    //!@}

    //! \name Accessors //!@{
    virtual GG::Pt              ClientUpperLeft() const;

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
    virtual void    Render();
    virtual void    LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    virtual void    KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);
    virtual void    KeyRelease(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);

    void            DoLayout();

    void            RegisterWindows();                                      //!< registers owned wnds with the GUI (also registers message & player list wnds)
    void            RemoveWindows();                                        //!< removes owned wnds from the GUI (also removes message & player list wnds)

    void            EnableOrderIssuing(bool enable = true);                 //!< enables or disables order issuing and pressing the turn button.

    void            InitTurn();                                             //!< called at the start of each turn
    void            MidTurnUpdate();                                        //!< called after receiving updated Universe during turn processing, but not when the full turn update is received

    void            RestoreFromSaveData(const SaveGameUIData& data);        //!< restores the UI state that was saved in an earlier call to GetSaveGameUIData().
    void            ShowSystemNames();                                      //!< enables the system name text
    void            HideSystemNames();                                      //!< disables the system name text

    mutable boost::signals2::signal<void (int)>    SystemLeftClickedSignal;
    mutable boost::signals2::signal<void (int)>    SystemRightClickedSignal;
    mutable boost::signals2::signal<void (int)>    SystemBrowsedSignal;
    mutable boost::signals2::signal<void (double)> ZoomedSignal;

    void            CenterOnMapCoord(double x, double y);                   //!< centers the map on map position (x, y)
    void            CenterOnObject(int id);                                 //!< centers the map on object with id \a id
    void            CenterOnObject(TemporaryPtr<const UniverseObject> obj);              //!< centers the map on object \a id

    void            ShowPlanet(int planet_id);                              //!< brings up encyclopedia panel and displays info about the planet
    void            ShowCombatLog(int log_id);                              //!< brings up encyclopedia panel and displays info about the combat
    void            ShowTech(const std::string& tech_name);                 //!< brings up the research screen and centers the tech tree on \a tech_name
    void            ShowBuildingType(const std::string& building_type_name);//!< brings up the production screen and displays info about the buildtype \a type_name
    void            ShowPartType(const std::string& part_type_name);        //!< brings up the production screen and displays info about the buildtype \a type_name
    void            ShowHullType(const std::string& hull_type_name);        //!< brings up the production screen and displays info about the buildtype \a type_name
    void            ShowShipDesign(int design_id);                          //!< brings up the production screen and displays info about the buildtype \a type_name
    void            ShowSpecial(const std::string& special_name);           //!< brings up encyclopedia panel and displays info about the special with name \a special_name
    void            ShowSpecies(const std::string& species_name);           //!< brings up encyclopedia panel and displays info about the species with name \a species_name
    void            ShowFieldType(const std::string& field_type_name);      //!< brings up encyclopedia panel and displays info about the field type with name \a field_type_name
    void            ShowEmpire(int empire_id);                              //!< brings up encyclopedia panel and displays info about the empire with id \a empire_id
    void            ShowEncyclopediaEntry(const std::string& str);          //!< brings up encyclopedia panel and displays info about the specified string \a str

    void            SelectSystem(int systemID);                             //!< programatically selects systems on map, sidepanel, and production screen.  catches signals from these when the user changes the selected system
    void            ReselectLastSystem();                                   //!< re-selects the most recently selected system, if a valid one exists
    void            SelectPlanet(int planetID);                             //!< programatically selects planets on sidepanels.  catches signals from production wnd or sidepanel for when the user changes the selected planet
    void            SelectFleet(int fleetID);                               //!< programatically selects fleets by ID
    void            SelectFleet(TemporaryPtr<Fleet> fleet);                              //!< programatically selects fleets
    void            ReselectLastFleet();                                    //!< re-selects the most recent selected fleet, if a valid one exists

    void            RemoveFleet(int fleet_id); //!< removes specified fleet.
    void            SetFleetMovementLine(int fleet_id);                     //!< creates fleet movement line for a single fleet.  Move lines originate from the fleet's button location.

    /* creates specially-coloured projected fleet movement line for specified
     * fleet following the specified route.  Move line originates from the
     * fleet's button location. */
    void            SetProjectedFleetMovementLine(int fleet_id, const std::list<int>& travel_route);
    /* creates specially-coloured projected fleet movement lines for specified
     * fleets following the specified route.  Move lines originates from the
     * fleets' button locations. */
    void            SetProjectedFleetMovementLines(const std::vector<int>& fleet_ids, const std::list<int>& travel_route);

    void            ClearProjectedFleetMovementLines();             //!< removes all projected fleet movement lines

    void            ResetEmpireShown();                             //!< auto-resets the shown empire in any contained Wnds, to the current client's empire (if any)

    void            RegisterPopup(MapWndPopup* popup);              //!< registers a MapWndPopup, which can be cleaned up with a call to DeleteAllPopups( )
    void            RemovePopup(MapWndPopup* popup);                //!< removes a MapWndPopup from the list cleaned up on a call to DeleteAllPopups( )
    void            Sanitize();                                     //!< sanitizes the MapWnd after a game
    //!@}

    void            SetFleetExploring(const int fleet_id);
    void            StopFleetExploring(const int fleet_id);
    bool            IsFleetExploring(const int fleet_id);
    void            DispatchFleetsExploring();                      //!< called at each turn begin and when a fleet start/stop exploring to redispatch everyone.

protected:
    virtual bool    EventFilter(GG::Wnd* w, const GG::WndEvent& event);

private:
    void            RefreshTradeResourceIndicator();
    void            RefreshFleetResourceIndicator();
    void            RefreshResearchResourceIndicator();
    void            RefreshIndustryResourceIndicator();
    void            RefreshPopulationIndicator();
    void            RefreshDetectionIndicator();

    void            UpdateSidePanelSystemObjectMetersAndResourcePools();                                ///< update meter estimates for objects contained within the current system shown in the sidepanel, or all objects if there is no system shown
    void            UpdateEmpireResourcePools();                                                        ///< recalculates production and predicted changes of player's empire's resource and population pools

    /** contains information necessary to render a single fleet movement line on the main map. also
      * contains cached infromation */
    struct MovementLineData {
        struct Vertex;                                  // apparent universe positions of move line points, derived from actual universe positions contained in MovePathNodes
        MovementLineData();
        MovementLineData(const std::list<MovePathNode>& path_,
                         const std::map<std::pair<int, int>, LaneEndpoints>& lane_end_points_map,
                         GG::Clr colour_ = GG::CLR_WHITE, int empireID = ALL_EMPIRES);

        std::list<MovePathNode>             path;       // raw path data from which line rendering is determined
        GG::Clr                             colour;     // colour of line
        std::vector<Vertex>                 vertices;   // cached apparent universe positions of starts and ends of line segments drawn to represent move path
    };

    class MapScaleLine;

    void            InitializeWindows();

    void            Zoom(int delta);                            //!< changes the zoom level of the main map by zoom step size to the power of \a delta (adds delta to the current zoom exponent)
    void            Zoom(int delta, const GG::Pt& position);    //!< changes the zoom level of the main map by zoom step size to the power of \a delta (adds delta to the current zoom exponent) Keeps the screen position \a position in the same place after zooming
    void            ZoomSlid(double pos, double low, double high);
    void            SetZoom(double steps_in, bool update_slide);//!< sets zoom level of the main map to zoom step size to the power of \a steps_in and updates zoom slider position if \a update_slide is true
    void            SetZoom(double steps_in, bool update_slide, const GG::Pt& position);//!< sets zoom level of the main map to zoom step size to the power of \a steps_in and updates zoom slider position if \a update_slide is true. Keeps the screen position \a position in the same place after zooming

    void            Pan(const GG::Pt& delta);                   //!< pans map
    bool            PanX(GG::X x = GG::X(50));
    bool            PanY(GG::Y y = GG::Y(50));

    /** Mark all fleet buttons for a refresh. */
    void            RefreshFleetButtons();
    /**  Removes old / existing and create new fleet buttons. Only called once per render interval.*/
    void            DeferredRefreshFleetButtons();

    /** Use the vectors of fleet ids from \p fleets_map to create fleet buttons in \p
        type_fleet_buttons and record the fleet buttons in \p m_fleet_buttons.*/
    template <typename K>
    void            CreateFleetButtonsOfType(
        boost::unordered_map<K, boost::unordered_set<FleetButton*> >& type_fleet_buttons,
        const boost::unordered_map<std::pair<K, int>, std::vector<int> > &fleets_map,
        const FleetButton::SizeType& fleet_button_size);

    /** Delete all fleet buttons.*/
    void            DeleteFleetButtons();

    void            RefreshFleetButtonSelectionIndicators();    //!< marks (only) selected fleets' buttons as selected

    /** Connect all \p fleets StateChangedSignal to RefreshFleetButtons. */
    void            AddFleetsStateChangedSignal(const std::vector<TemporaryPtr<Fleet> >& fleets);
    /** Disconnect all \p fleets StateChangedSignal from RefreshFleetButtons. */
    void            RemoveFleetsStateChangedSignal(const std::vector<TemporaryPtr<Fleet> >& fleets);
    /** Handle FleetsInsertedSignal by connecting signals and refreshing fleet buttons. */
    void            FleetsInsertedSignalHandler(const std::vector<TemporaryPtr<Fleet> >& fleets);
    /** Handle FleetsRemovedSignal by disconnecting signals and refreshing fleet buttons. */
    void            FleetsRemovedSignalHandler(const std::vector<TemporaryPtr<Fleet> >& fleets);


    void            DoFleetButtonsLayout();                     //!< does layout of fleet buttons
    std::pair<double, double>   MovingFleetMapPositionOnLane(TemporaryPtr<const Fleet> fleet) const; //!< returns position on map where a moving fleet should be displayed.  This is different from the fleet's actual universe position due to the squishing of fleets moving along a lane into the space between the system circles at the ends of the lane

    void            DoSystemIconsLayout();                      //!< does layout of system icons
    void            DoFieldIconsLayout();                       //!< does layout of field icons

    void            RefreshFleetSignals();                      //!< disconnects and reconnects all fleet change signals

    void            RefreshSliders();                           //!< shows or hides sliders on map

    void            InitTurnRendering();                        //!< sets up rendering of system icons, galaxy gas, starlanes at start of turn
    void            InitSystemRenderingBuffers();               //!< initializes or refreshes buffers for rendering of system icons and galaxy gas
    void            ClearSystemRenderingBuffers();
    void            InitStarlaneRenderingBuffers();             //!< initializes or refreshes buffers for rendering of starlanes
    void            ClearStarlaneRenderingBuffers();
    void            InitFieldRenderingBuffers();
    void            ClearFieldRenderingBuffers();
    void            InitVisibilityRadiiRenderingBuffers();
    void            ClearVisibilityRadiiRenderingBuffers();
    void            InitScaleCircleRenderingBuffer();
    void            ClearScaleCircleRenderingBuffer();
    void            ClearStarfieldRenderingBuffers();

    /* Takes X and Y coordinates of a pair of systems and moves these points inwards along the vector
     * between them by the radius of a system on screen (at zoom 1.0) and return result */ 
    LaneEndpoints   StarlaneEndPointsFromSystemPositions(double X1, double Y1, double X2, double Y2);

    void            RenderStarfields();                         //!< renders the background starfiends
    void            RenderGalaxyGas();                          //!< renders gassy substance to make shape of galaxy
    void            RenderSystemOverlays();                     //!< renders textures "overlayed" on systems by effects
    void            RenderSystems();                            //!< renders stars and halos
    void            RenderStarlanes();                          //!< renders the starlanes between the systems
    void            RenderStarlanes(GG::GL2DVertexBuffer& vertices, GG::GLRGBAColorBuffer& colours,
                                    double thickness, bool coloured, bool doBase);
    void            RenderFields();                             //!< renders field objects

    /* renders the dashed lines indicating where each fleet is going */
    void            RenderFleetMovementLines();

    /* renders a single fleet movement line. if \a clr is GG::CLR_ZERO, the lane
     * is rendered with the .colour attribute of \a move_line. assumes that the
     * move dot texture has already been bound. */
    void            RenderMovementLine(const MapWnd::MovementLineData& move_line, float dot_size, float dot_spacing, float dot_shift,
                                       GG::Clr clr = GG::CLR_ZERO);

    /* renders ETA indicators at end-of-turn positions for a single fleet movement
     * line.  if \a clr is GG::CLR_ZERO, the indicators are filled with the .colour
     * attribute of \a move_line */
    void            RenderMovementLineETAIndicators(const MapWnd::MovementLineData& move_line, GG::Clr clr = GG::CLR_ZERO);

    /* renders circles around objects' locations indicating distance they have
     * visibility */
    void            RenderVisibilityRadii();

    /* renders scale circle around selected system. */
    void            RenderScaleCircle();

    void            CorrectMapPosition(GG::Pt& move_to_pt);     //!< constrains \a move_to_pt so that if the map is repositioned to that location, it will not be problematically positioned, so that galaxy contents remain visible

    void            FieldRightClicked(int field_id);

    void            SystemDoubleClicked(int system_id);
    void            SystemLeftClicked(int system_id);
    void            SystemRightClicked(int system_id, GG::Flags< GG::ModKey > mod_keys);
    void            MouseEnteringSystem(int system_id, GG::Flags< GG::ModKey > mod_keys);
    void            MouseLeavingSystem(int system_id);

    void            PlanetDoubleClicked(int planet_id);
    void            PlanetRightClicked(int planet_id);
    void            BuildingRightClicked(int building_id);

    void            ReplotProjectedFleetMovement(bool append); //!< Find any projected movement plots and replots them with the current append state
    void            PlotFleetMovement(int system_id, bool execute_move, bool append);   //!< issues fleet move orders to appropriate fleets in active FleetWnd

    void            FleetButtonLeftClicked(const FleetButton* fleet_btn);
    void            FleetButtonRightClicked(const FleetButton* fleet_btn);
    void            FleetRightClicked(int fleet_id);
    void            FleetsRightClicked(const std::vector<int>& fleet_ids);
    
    void            ShipRightClicked(int fleet_id);
    void            ShipsRightClicked(const std::vector<int>& fleet_ids);

    void            UniverseObjectDeleted(TemporaryPtr<const UniverseObject> obj);

    bool            ReturnToMap();

    bool            EndTurn();
    void            ToggleAutoEndTurn();

    bool            ToggleModeratorActions();
    void            ShowModeratorActions();
    void            HideModeratorActions();

    bool            ToggleMessages();
    void            ShowMessages();
    bool            OpenMessages();
    void            HideMessages();

    bool            ToggleSitRep();
    void            ShowSitRep();
    void            HideSitRep();

    bool            ToggleEmpires();
    void            ShowEmpires();
    void            HideEmpires();

    bool            ToggleObjects();
    void            ShowObjects();
    void            HideObjects();

    bool            TogglePedia();
    void            ShowPedia();
    void            HidePedia();

    bool            ShowGraphs();

    void            HideSidePanel();
    void            RestoreSidePanel();

    bool            ToggleResearch();
    void            ShowResearch();
    void            HideResearch();

    bool            ToggleProduction();
    void            ShowProduction();
    void            HideProduction();

    bool            ToggleDesign();
    void            ShowDesign();
    void            HideDesign();

    bool            ShowMenu();

    bool            CloseSystemView();                          //!< closes off the current system view

    bool            KeyboardZoomIn();
    bool            KeyboardZoomOut();

    bool            ZoomToHomeSystem();
    bool            ZoomToPrevOwnedSystem();
    bool            ZoomToNextOwnedSystem();
    bool            ZoomToPrevIdleFleet();
    bool            ZoomToNextIdleFleet();
    bool            ZoomToPrevFleet();
    bool            ZoomToNextFleet();
    bool            ZoomToSystemWithWastedPP();

    void            ConnectKeyboardAcceleratorSignals();        //!< connects signals from keyboard accelerators to various GUI responses

    void            ChatMessageSentSlot();

    void            CloseAllPopups();
    void            HideAllPopups();
    void            ShowAllPopups();

    void            SelectedFleetsChanged();
    void            SelectedShipsChanged();

    std::set<int>               m_selected_fleet_ids;
    std::set<int>               m_selected_ship_ids;

    double                      m_zoom_steps_in;    //!< number of zoom steps in.  each 1.0 step increases display scaling by the same zoom step factor
    SidePanel*                  m_side_panel;       //!< planet view panel on the side of the main map
    boost::unordered_map<int, SystemIcon*>  m_system_icons;     //!< system icons in the main map, indexed by system id
    std::map<int, FieldIcon*>   m_field_icons;      //!< field icons in the main map, indexed by field id
    SitRepPanel*                m_sitrep_panel;     //!< sitrep panel
    ResearchWnd*                m_research_wnd;     //!< research screen
    ProductionWnd*              m_production_wnd;   //!< production screen
    DesignWnd*                  m_design_wnd;       //!< design screen
    EncyclopediaDetailPanel*    m_pedia_panel;      //!< encyclpedia panel
    ObjectListWnd*              m_object_list_wnd;  //!< filterable list of objects in universe
    ModeratorActionsWnd*        m_moderator_wnd;    //!< buttons to select moderator actions
    CombatReportWnd*            m_combat_report_wnd;//!< shows graphical reports of combats

    std::map<std::pair<int, int>, LaneEndpoints>    m_starlane_endpoints;                   //!< map from starlane start and end system IDs (stored in pair in increasing order) to the universe coordiates at which to draw the starlane ends

    boost::unordered_map<int, boost::unordered_set<FleetButton*> >          m_stationary_fleet_buttons;             //!< icons representing fleets at a system that are not departing, indexed by system
    boost::unordered_map<int, boost::unordered_set<FleetButton*> >          m_departing_fleet_buttons;              //!< icons representing fleets at a system that are departing, indexed by system
    boost::unordered_map<std::pair<double, double>,  boost::unordered_set<FleetButton*> > m_moving_fleet_buttons;   //!< icons representing fleets not at a system
    boost::unordered_map<int, FleetButton*>                     m_fleet_buttons;                        //!< fleet icons, index by fleet

    boost::unordered_map<int, boost::signals2::connection>               m_fleet_state_change_signals;
    boost::unordered_map<int, std::vector<boost::signals2::connection> > m_system_fleet_insert_remove_signals;

    std::map<int, MovementLineData>                 m_fleet_lines;                          //!< lines used for moving fleets in the main map
    std::map<int, MovementLineData>                 m_projected_fleet_lines;                //!< lines that show the projected path of the active fleet in the FleetWnd

    std::pair<int, int>                 m_line_between_systems;                             //!< set when map should render line connecting 2 systems

    std::map<boost::shared_ptr<GG::Texture>, GG::GL2DVertexBuffer>  m_star_core_quad_vertices;
    std::map<boost::shared_ptr<GG::Texture>, GG::GL2DVertexBuffer>  m_star_halo_quad_vertices;
    GG::GL2DVertexBuffer                m_galaxy_gas_quad_vertices;
    GG::GLTexCoordBuffer                m_galaxy_gas_texture_coords;
    GG::GLTexCoordBuffer                m_star_texture_coords;
    GG::GL2DVertexBuffer                m_star_circle_vertices;

    GG::GL2DVertexBuffer                m_starlane_vertices;
    GG::GLRGBAColorBuffer               m_starlane_colors;
    GG::GL2DVertexBuffer                m_RC_starlane_vertices;
    GG::GLRGBAColorBuffer               m_RC_starlane_colors;

    std::map<boost::shared_ptr<GG::Texture>, std::pair<GG::GL2DVertexBuffer, GG::GL2DVertexBuffer> >    m_field_vertices;   //!< first buffer is visible fields, second buffer is not visible (scanlined) fields for each texture
    GG::GL2DVertexBuffer                m_field_scanline_circles;
    GG::GLTexCoordBuffer                m_field_texture_coords;

    GG::GL2DVertexBuffer                m_visibility_radii_vertices;
    GG::GLRGBAColorBuffer               m_visibility_radii_colors;
    GG::GL2DVertexBuffer                m_visibility_radii_border_vertices;
    GG::GLRGBAColorBuffer               m_visibility_radii_border_colors;
    std::vector<std::pair<std::pair<std::size_t, std::size_t>, std::pair<std::size_t, std::size_t> > >
                                        m_radii_radii_vertices_indices_runs;

    GG::GL2DVertexBuffer                m_scale_circle_vertices;

    GG::GL3DVertexBuffer                m_starfield_verts;
    GG::GLRGBAColorBuffer               m_starfield_colours;

    ScanlineRenderer                    m_scanline_shader;

    GG::Pt                      m_drag_offset;      //!< distance the cursor is from the upper-left corner of the window during a drag ((-1, -1) if no drag is occurring)
    bool                        m_dragged;          //!< tracks whether or not a drag occurs during a left button down sequence of events
    GG::Button*                 m_btn_turn;         //!< button that updates player's turn
    GG::Button*                 m_btn_auto_turn;    //!< button that toggles whether to automatically end turns
    bool                        m_auto_end_turn;    //!< should turns be ended automatically by this client?
    std::list<MapWndPopup*>     m_popups;           //!< list of currently active popup windows
    bool                        m_menu_showing;     //!< set during ShowMenu() to prevent reentrency
    int                         m_current_owned_system;
    int                         m_current_fleet_id;
    bool                        m_in_production_view_mode;

    bool                        m_sidepanel_open_before_showing_other;  //!< was the sidepanel open before switching to production, research or design screens?  If so, it should be restored when leaving them.

    CUIToolBar*                 m_toolbar;
    StatisticIcon               *m_trade, *m_population, *m_research, *m_industry, *m_detection, *m_fleet;
    GG::Button                  *m_industry_wasted, *m_research_wasted;
    GG::Button                  *m_btn_moderator, *m_btn_messages, *m_btn_empires,
                                *m_btn_siterep, *m_btn_research, *m_btn_production,
                                *m_btn_design, *m_btn_pedia, *m_btn_graphs,
                                *m_btn_objects, *m_btn_menu;
    GG::Label*                  m_FPS;

    MapScaleLine*               m_scale_line;       //!< indicates the on-screen distance that reprensents an in-universe distance
    GG::Slider<double>*         m_zoom_slider;      //!< allows user to set zoom level

    std::set<int>               m_fleets_exploring;

    /// indicates that refresh fleet button work should be done before rendering.
    bool                        m_deferred_refresh_fleet_buttons;

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
    virtual ~MapWndPopup();
    void    CloseClicked();
    void    Close();
};

#endif // _MapWnd_h_
