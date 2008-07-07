// -*- C++ -*-
//MapWnd.h
#ifndef _MapWnd_h_
#define _MapWnd_h_

#include <vector>

#include "CUIWnd.h"
#include "../universe/Enums.h"
#include "../universe/Fleet.h"

class CUIButton;
class CUIEdit;
class CUITurnButton;
class FleetButton;
class FleetWnd;
class MapWndPopup;
class DesignWnd;
class ProductionWnd;
class ResearchWnd;
struct SaveGameUIData;
class SidePanel;
class SitRepPanel;
class SystemIcon;
class StatisticIcon;
class CUIToolBar;
class UniverseObject;
class FPSIndicator;
struct MovePathNode;
namespace GG {
    class Texture;
    class MultiEdit;
    class WndEvent;
    class StaticGraphic;
}


/** This class is a window that graphically displays everything in the universe */
class MapWnd : public GG::Wnd
{
public:
    //! \name Signal Types //!@{
    typedef boost::signal<void (int)> SystemLeftClickedSignalType;  //!< emitted when the user left-clicks a star system
    typedef boost::signal<void (int)> SystemRightClickedSignalType; //!< emitted when the user right-clicks a star system
    typedef boost::signal<void (int)> SystemBrowsedSignalType;      //!< emitted when the user moves the mouse over a star system
    //!@}

    //! \name Slot Types //!@{
    typedef SystemLeftClickedSignalType::slot_type  SystemLeftClickedSlotType;  //!< type of functor invoked when the user left-clicks a star system
    typedef SystemRightClickedSignalType::slot_type SystemRightClickedSlotType; //!< type of functor invoked when the user right-clicks a star system
    typedef SystemBrowsedSignalType::slot_type      SystemBrowsedSlotType;      //!< type of functor invoked when the user moves the mouse over a star system
    //!@}

    //! \name Structors //!@{
    MapWnd();    //!< default ctor
    ~MapWnd();   //!< default dtor
    //!@}

    //! \name Accessors //!@{
    virtual GG::Pt ClientUpperLeft() const;

    double         ZoomFactor() const;
    int            SystemIconSize() const;
    int            FleetButtonSize() const;
    SidePanel*     GetSidePanel() const;
    void           GetSaveGameUIData(SaveGameUIData& data) const;   //!< populates the relevant UI state that should be restored after a save-and-load cycle
    bool           InProductionViewMode() const;                    //!< returns true iff this MapWnd is visible and usable for interaction, but the allowed interactions are restricted to those appropriate to the production screen

    /* returns the position on the screen that corresponds to the specified universe X and Y coordinates. */
    GG::Pt                      ScreenCoordsFromUniversePosition(double universe_x, double universe_y) const;
    /* returns the universe position (X and Y in pair) that corresponds to the specified screen coordinates. */
    std::pair<double, double>   UniversePositionFromScreenCoords(GG::Pt screen_coords) const;
    //!@}

    //! \name Mutators //!@{
    virtual void   Render();
    virtual void   KeyPress (GG::Key key, GG::Flags<GG::ModKey> mod_keys);
    virtual void   LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void   LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);
    virtual void   LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void   LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void   RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void   MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);

    void           InitTurn(int turn_number);                       //!< called at the start of each turn
    void           RestoreFromSaveData(const SaveGameUIData& data); //!< restores the UI state that was saved in an earlier call to GetSaveGameUIData().
    void           ShowSystemNames();                               //!< enables the system name text
    void           HideSystemNames();                               //!< disables the system name text
    void           HandlePlayerChatMessage(const std::string& msg); //!< displays incoming player chat text

    mutable SystemLeftClickedSignalType  SystemLeftClickedSignal;
    mutable SystemRightClickedSignalType SystemRightClickedSignal;
    mutable SystemBrowsedSignalType      SystemBrowsedSignal;

    void            CenterOnMapCoord(double x, double y);           //!< centers the map on map position (x, y)
    void            CenterOnObject(int id);                         //!< centers the map on object with id \a id
    void            CenterOnObject(const UniverseObject* obj);      //!< centers the map on object \a id
    void            ShowTech(const std::string& tech_name);                    //!< brings up the research screen and centers the tech tree on \a tech_name
    void            ShowBuildingType(const std::string& building_type_name);   //!< brings up the production screen and displays info about the buildtype \a type_name
    void            SelectSystem(int systemID);                     //!< catches emitted signals from the system icons, and allows programmatic selection of planets
    void            ReselectLastSystem();                           //!< re-selects the most recently selected system, if a valid one exists
    void            SelectFleet(int fleetID);                       //!< allows programmatic selection of fleets
    void            SelectFleet(Fleet* fleet);                      //!< allows programmatic selection of fleets

    void            SetFleetMovementLine(const FleetButton* fleet_button);          //!< creates fleet movement lines for all fleets in the given FleetButton to indicate where (and whether) they are moving.  Move lines originate from the FleetButton.
    void            SetFleetMovementLine(const Fleet* fleet);                       //!< creates fleet movement line for a single fleet.  Move lines originate from the fleet's button location.
    void            SetFleetETAIndicators(const std::vector<const Fleet*>& fleets); //!< removes existing fleet move line ETA indicators, and adds indicators for the specified \a fleets move lines

    /* creates specially-coloured projected fleet movement line for specified fleet following the specified
       route.  Move line originates from the fleet's button location. */
    void            SetProjectedFleetMovementLine(const Fleet* fleet, const std::list<System*>& travel_route);
    /* creates specially-coloured projected fleet movement lines for specified fleets following the specified
       route.  Move lines originates from the fleets' button locations. */
    void            SetProjectedFleetMovementLines(const std::vector<const Fleet*>& fleets, const std::list<System*>& travel_route);
    void            RemoveProjectedFleetMovementLine(const Fleet* fleet);   //!< removes projected fleet movement line for specified fleet.
    void            ClearProjectedFleetMovementLines();                     //!< removes all projected fleet movement lines
    void            SetProjectedFleetETAIndicators();                       //!< creates fleet move line ETA indicators for current projected fleet move lines

    void            RegisterPopup(MapWndPopup* popup);              //!< registers a MapWndPopup, which can be cleaned up with a call to DeleteAllPopups( )
    void            RemovePopup(MapWndPopup* popup);                //!< removes a MapWndPopup from the list cleaned up on a call to DeleteAllPopups( )
    void            Cleanup();                                      //!< cleans up the MapWnd at the end of a turn (ie, closes all windows and disables all keyboard accelerators)
    void            Sanitize();                                     //!< sanitizes the MapWnd after a game
    //!@}

    static const int SIDE_PANEL_WIDTH;

protected:
    virtual bool    EventFilter(GG::Wnd* w, const GG::WndEvent& event);

private:
    void            RefreshFoodResourceIndicator();                 ///< gets stockpile and expected change and updates resource indicator
    void            RefreshMineralsResourceIndicator();
    void            RefreshTradeResourceIndicator();
    void            RefreshResearchResourceIndicator();
    void            RefreshIndustryResourceIndicator();
    void            RefreshPopulationIndicator();

    void            UpdateMetersAndResourcePools();                                                     ///< update meter estimates and resource pool amounts for this client's empire
    void            UpdateMetersAndResourcePools(const std::vector<int>& objects_vec);                  ///< update meter estimates for indicated objects, and resource pool amounts for this client's empire
    void            UpdateMetersAndResourcePools(int object_id, bool update_contained_objects = false); ///< update meter esimtates for indiacted objects, and resource pool amounts for this client's empire
    void            UpdateSidePanelSystemObjectMetersAndResourcePools();                                ///< update meter estimates for objects contained within the current system shown in the sidepanel, or all objects if there is no system shown
    void            UpdateMeterEstimates();                                                             ///< re-estimates meter values of all known objects based on orders given
    void            UpdateMeterEstimates(int object_id, bool update_contained_objects = false);         ///< re-estimates meter values of specified objects
    void            UpdateMeterEstimates(const std::vector<int>& objects_vec);                          ///< re-estimates meter values of specified objects
    void            UpdateEmpireResourcePools();                                                        ///< recalculates production and predicted changes of player's empire's resource and population pools

    /** contains all the information necessary to render a single fleet movement line on the main map */
    struct MovementLineData
    {
        MovementLineData();
        MovementLineData(double x, double y, const std::list<MovePathNode>& path, GG::Clr colour = GG::CLR_WHITE);
        MovementLineData(const FleetButton* button, const std::list<MovePathNode>& path, GG::Clr colour = GG::CLR_WHITE);

        GG::Clr Colour() const;                      ///< colour in which to draw line
        const std::list<MovePathNode>& Path() const; ///< path to draw
        std::pair<double, double> Start() const;     ///< universe x and y at which to originate line (start point isn't in the path)

    private:
        GG::Clr m_colour;
        std::list<MovePathNode> m_path;
        const FleetButton* m_button;    ///< fleet button at which this movement path originates (if zero, m_x and m_y should be used instead)
        double m_x, m_y;                ///< universe x and y at which to originate line (start point isn't in the path) (if m_button is nonzero, its galaxy position should be used instead)
    };

    class FleetETAMapIndicator;

    void Zoom(int delta);                           //!< changes the zoomlevel of the main map
    void DoMovingFleetButtonsLayout();              //!< does layout of fleet buttons for moving fleets
    void DoSystemIconsLayout();                     //!< does layout of system icons

    void RenderStarfields();                        //!< renders the background starfiends
    void RenderNebulae();                           //!< renders nebulae
    void RenderGalaxyGas();                         //!< renders gassy substance to make shape of galaxy
    void RenderSystems();                           //!< renders stars and halos
    void RenderStarlanes();                         //!< renders the starlanes between the systems
    void RenderFleetMovementLines();                //!< renders the dashed lines indicating where each fleet is going
    void RenderMovementLine(const MapWnd::MovementLineData& move_line); //!< renders a single fleet movement line

    void CorrectMapPosition(GG::Pt &move_to_pt);    //!< ensures that the map data are positioned sensibly
    void SystemDoubleClicked(int system_id);
    void SystemLeftClicked(int system_id);
    void SystemRightClicked(int system_id);
    void MouseEnteringSystem(int system_id);
    void MouseLeavingSystem(int system_id);
    void PlotFleetMovement(int system_id, bool execute_move);   //!< issues fleet move orders to appropriate fleets in active FleetWnd
    void FleetButtonLeftClicked(FleetButton& fleet_btn, bool fleet_departing);
    void UniverseObjectDeleted(const UniverseObject *obj);
    bool ReturnToMap();
    bool OpenChatWindow();
    bool EndTurn();
    bool ToggleSitRep();
    bool ToggleResearch();
    bool ToggleProduction();
    bool ToggleDesign();
    bool ShowMenu();
    bool CloseSystemView();                      //!< closes off the current system view
    bool KeyboardZoomIn();
    bool KeyboardZoomOut();
    bool ZoomToHomeSystem();
    bool ZoomToPrevOwnedSystem();
    bool ZoomToNextOwnedSystem();
    bool ZoomToPrevIdleFleet();
    bool ZoomToNextIdleFleet();
    bool ZoomToPrevFleet();
    bool ZoomToNextFleet();

    void SetAccelerators();

    void RemoveAccelerators();
    /** Disables keyboard accelerators that use an alphanumeric key without modifiers. This is useful if a
     * keyboard input is required, so that the keys aren't interpreted as an accelerator.
     * @note Repeated calls of DisableAlphaNumAccels have to be followed by the same number of calls to 
     * EnableAlphaNumAccels to re-enable the accelerators. */
    void DisableAlphaNumAccels();
    void EnableAlphaNumAccels();                //!< Re-enable accelerators disabled by DisableAlphaNumAccels

    void CloseAllPopups();
    void HideAllPopups();
    void ShowAllPopups();

    void FleetWndClosing(FleetWnd* fleet_wnd);
    void HandleEmpireElimination(int empire_id);    //!< cleans up internal storage of now-invalidated empire ID

    std::set<GG::Key> m_disabled_accels_list;       //!< the list of Accelerators disabled by \a DisableAlphaNumAccels

    std::vector<boost::shared_ptr<GG::Texture> >    m_backgrounds;      //!< starfield backgrounds
    std::vector<boost::shared_ptr<GG::Texture> >    m_nebulae;          //!< decorative nebula textures
    std::vector<GG::Pt>                             m_nebula_centers;   //!< the centerpoints of each of the nebula textures
    std::vector<double>                             m_bg_scroll_rate;   //!< array, the rates at which each background scrolls

    int                         m_previously_selected_system;

    double                      m_zoom_factor;      //!< current zoom level; clamped to [MIN_SCALE_FACTOR, MAX_SCALE_FACTOR]
    SidePanel*                  m_side_panel;       //!< planet view panel on the side of the main map
    std::map<int, SystemIcon*>  m_system_icons;     //!< system icons in the main map, indexed by system id
    SitRepPanel*                m_sitrep_panel;     //!< sitrep panel
    ResearchWnd*                m_research_wnd;     //!< research screen
    ProductionWnd*              m_production_wnd;   //!< production screen
    DesignWnd*                  m_design_wnd;       //!< design screen
    GG::MultiEdit*              m_chat_display;     //!< (read-only) MP-chat output multi-line edit box
    CUIEdit*                    m_chat_edit;        //!< MP-chat input edit box

    std::vector<FleetButton*>                       m_moving_fleet_buttons;                 //!< moving fleets in the main map (SystemIcons contain stationary fleet buttons)

    std::map<const Fleet*, MovementLineData>        m_fleet_lines;                          //!< lines used for moving fleets in the main map
    std::map<const Fleet*,
             std::vector<FleetETAMapIndicator*> >   m_fleet_eta_map_indicators;             //!< indicators that appear adjacent to fleet move lines that indicate the eta for points on a move path

    std::map<const Fleet*, MovementLineData>        m_projected_fleet_lines;                //!< lines that show the projected path of the active fleet in the FleetWnd
    std::map<const Fleet*,
             std::vector<FleetETAMapIndicator*> >   m_projected_fleet_eta_map_indicators;   //!< indicators that appear adjacent to projected fleet move lines that indicate the eta for points on a move path

    // OpenGL buffers objects containing vertices, texture coordinates, etc.
    struct GLBuffer
    {
        GLBuffer();
        GLuint      m_name;
        std::size_t m_size;
    };
    std::map<boost::shared_ptr<GG::Texture>, GLBuffer> m_star_core_quad_vertices;
    std::map<boost::shared_ptr<GG::Texture>, GLBuffer> m_star_halo_quad_vertices;
    std::map<boost::shared_ptr<GG::Texture>, GLBuffer> m_galaxy_gas_quad_vertices;
    GLBuffer m_star_texture_coords;
    GLBuffer m_starlane_vertices;
    GLBuffer m_starlane_supply_vertices;
    GLBuffer m_starlane_supply_colors;

    GG::Pt                      m_drag_offset;      //!< distance the cursor is from the upper-left corner of the window during a drag ((-1, -1) if no drag is occurring)
    bool                        m_dragged;          //!< tracks whether or not a drag occurs during a left button down sequence of events
    CUITurnButton*              m_turn_update;      //!< button that updates player's turn
    std::list<MapWndPopup*>     m_popups;           //!< list of currently active popup windows
    bool                        m_menu_showing;     //!< set during ShowMenu() to prevent reentrency
    int                         m_current_owned_system;
    int                         m_current_fleet;
    bool                        m_in_production_view_mode;

    CUIToolBar                  *m_toolbar;
    StatisticIcon               *m_food, *m_mineral, *m_trade, *m_population, *m_research, *m_industry;

    CUIButton                   *m_btn_siterep, *m_btn_research, *m_btn_production, *m_btn_design, *m_btn_menu;

    FPSIndicator*               m_FPS;

    static double s_min_scale_factor;
    static double s_max_scale_factor;

    struct FleetButtonClickedFunctor
    {
        FleetButtonClickedFunctor(FleetButton& fleet_btn, MapWnd& map_wnd);
        void operator()();
        FleetButton& m_fleet_btn;
        MapWnd& m_map_wnd;
    };
};


/** Derive any window from this class to have it managed by MapWnd. For example, MapWnd will delete all open popups
   when the end turn button is hit. */
class MapWndPopup : public CUIWnd
{
public:
    MapWndPopup(const std::string& t, int x, int y, int h, int w, GG::Flags<GG::WndFlag> flags);
    virtual ~MapWndPopup();
    void CloseClicked();
    void Close();
};

#endif // _MapWnd_h_
