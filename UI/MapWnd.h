// -*- C++ -*-
//MapWnd.h
#ifndef _MapWnd_h_
#define _MapWnd_h_

#include <vector>

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#ifndef _InGameOptions_h_
#include "InGameOptions.h"
#endif

class CUIButton;
class Fleet;
class FleetButton;
class SidePanel;
class SitRepPanel;
class System;
class SystemIcon;
namespace GG {
class Texture;
}

/**
    This class is a huge window that stores EVERYTHING in the whole universe graphically
*/
class MapWnd : public GG::Wnd
{
public:
    static const double MIN_SCALE_FACTOR;
    static const double MAX_SCALE_FACTOR;

    enum     //!< enumeration for indices to the icon vector
    {
        FLEET_ICON          = 0,
        NUTRIENTS_ICON      = 1,
        SCIENCE_ICON        = 2,
        MINERALS_ICON       = 3,
        INDUSTRY_ICON       = 4,
        POPULATION_ICON     = 5
    };

    //! \name Signal Types //!@{
    typedef boost::signal<void (int)> SelectedSystemSignalType; //!< emitted when the user selects a star system
    //!@}

    //! \name Slot Types //!@{
    typedef SelectedSystemSignalType::slot_type SelectedSystemSlotType; //!< type of functor invoked when the user selects a star system
    //!@}

    //! \name Structors //!@{
    MapWnd();    //!< default ctor
    ~MapWnd();   //!< default dtor
    //!@}

    //! \name Accessors //!@{
    virtual GG::Pt ClientUpperLeft() const;

    double ZoomFactor() const {return m_zoom_factor;}
    //!@}

    //! \name Mutators //!@{
    virtual int    Render();
    virtual int    Keypress (GG::Key key, Uint32 key_mods);
    virtual int    LButtonDown(const GG::Pt& pt, Uint32 keys);
    virtual int    LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys);
    virtual int    LButtonUp(const GG::Pt& pt, Uint32 keys);
    virtual int    LClick(const GG::Pt& pt, Uint32 keys);
    virtual int    MouseWheel(const GG::Pt& pt, int move, Uint32 keys);

    void           InitTurn( int turn_number );        //!< called at the start of each turn by
    void           ShowSystemNames(); //!< enables the system name text
    void           HideSystemNames(); //!< disables the system name text

    SelectedSystemSignalType& SelectedSystemSignal() {return m_selected_system_signal;}

    void SelectSystem(int systemID); //!< catches emitted signals from the system icons

    void SetFleetMovement(FleetButton* fleet_button); //!< allows code that creates FleetButtons to indicate where (and whether) they are moving

    void OnTurnUpdate();   //!< called when m_turn_update is clicked
    //!@}
        
private:
    struct MovementLineData
    {
        MovementLineData() {}
        MovementLineData(System* dest) : destination(dest) {}
        System* destination;
        // TODO : color, other properties(?), based on moving empire and destination, etc.
    };

    void RenderBackgrounds();    //!< renders the backgrounds onto the screen
    void RenderFleetMovementLines(); //!< renders the dashed lines indicating where each fleet is going
    void MoveBackgrounds(const GG::Pt& move); //!< scrolls the backgrounds at their respective rates
    void CorrectMapPosition(GG::Pt &move_to_pt); //!< ensures that the map data are positioned sensibly

    std::vector<boost::shared_ptr<GG::Texture> > m_backgrounds; //!< starfield backgrounds
    std::vector<boost::shared_ptr<GG::Texture> > m_nebulae;     //!< decorative nebula textures
    std::vector<GG::Pt>   m_nebula_centers; //!< the centerpoints of each of the nebula textures
    std::vector<double>   m_bg_scroll_rate; //!< array, the rates at which each background scrolls
    std::vector<double>   m_bg_position_X;  //!< array, the X position of the first full background image
    std::vector<double>   m_bg_position_Y;  //!< array, the Y positions of the backgrounds

    double                          m_zoom_factor;   //! the current zoom level; clamped to [MIN_SCALE_FACTOR, MAX_SCALE_FACTOR]
    SidePanel*                      m_side_panel;    //! the planet view panel on the side of the main map
    std::vector<SystemIcon*>        m_system_icons;  //! the system icons in the main map
    SitRepPanel*      	            m_sitrep_panel;  //! the sitrep panel
    std::vector<FleetButton*>       m_fleet_buttons; //! the moving fleets in the main map
    std::map<FleetButton*, 
             MovementLineData>      m_fleet_lines;   //! the lines used for moving fleets in the main map

    GG::Pt     m_drag_offset;  //! the distance the cursor is from the upper-left corner of the window during a drag ((-1, -1) if no drag is occurring)
    bool       m_dragged;      //! tracks whether or not a drag occurs during a left button down sequence of events

    CUIButton* m_turn_update;    //!< button that updates player's turn

    SelectedSystemSignalType m_selected_system_signal;

    static const int NUM_BACKGROUNDS;

    friend class GalaxyMapScreen;    //this is basically a part of that screen anyway
};

#endif
