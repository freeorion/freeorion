// -*- C++ -*-
//MapWnd.h
#ifndef _MapWnd_h_
#define _MapWnd_h_

#include <vector>

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#define GMAP_NUM_BACKGROUNDS 3

class SidePanel;
class SystemIcon;
class Icon;
namespace GG {
class Texture;
}

/**
    This class is a huge window that stores EVERYTHING in the whole universe graphically
*/
class MapWnd : public GG::Wnd
{
public:
    static const int MAX_SCALE_FACTOR;
    
    typedef std::vector<SystemIcon*> StarControlVec;
    typedef std::vector<Icon*> IconVec;
    
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
    
public:

    //! \name Structors //!@{
    MapWnd();    //!< default ctor
    ~MapWnd();   //!< default dtor
    
    //!@}
    
    //! \name Accessors //!@{

    //!@}
    
    //! \name Mutators //!@{
    void           InitTurn();        //!< called at the start of each turn by

    virtual int    Render();
    virtual int    LButtonDown(const GG::Pt& pt, Uint32 keys);
    virtual int    LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys);
    virtual int    LButtonUp(const GG::Pt& pt, Uint32 keys);
    virtual int    LClick(const GG::Pt& pt, Uint32 keys);
    
    SelectedSystemSignalType& SelectedSystemSignal() {return m_selected_system_signal;}

    void SelectSystem(int systemID); //!< catches emitted signals from the system icons
    //!@}

private:
    //! \name Private methods //!@{
    void RenderBackgrounds();    //!< this renders the backgrounds onto the screen
    void MoveBackgrounds(const GG::Pt& move);        //!< this scrolls the backgrounds at their respective rates
    
    //!@}
    
private:
    //! \name Background Data Storage //!@{
    // TODO: Convert this to a "Background" class
    boost::shared_ptr<GG::Texture>    m_background[GMAP_NUM_BACKGROUNDS];  //!< array, 3 layers of backgrounds
    float           m_bg_scroll_rate[GMAP_NUM_BACKGROUNDS]; //!< array, the rates at which each background scrolls
    float           m_bg_position_X[GMAP_NUM_BACKGROUNDS]; //!< array, the X position of the first full background image
    float           m_bg_position_Y[GMAP_NUM_BACKGROUNDS]; //!< array, the Y positions of the backgrounds
    
    //!@}

    SidePanel*      m_side_panel;     //! the planet view panel on the side of the main map
    StarControlVec  m_stars;          //! a vector of StarControls
    IconVec         m_icons;          //! a list of icons

    GG::Pt          m_drag_offset;    //! the distance the cursor is from the upper-left corner of the window during a drag ((-1, -1) if no drag is occurring)
    bool            m_dragged;        //! tracks whether or not a drag occurs during a left button down sequence of events
    
    //! \name Signals //!@{
    SelectedSystemSignalType m_selected_system_signal;
    
    //!@}

    friend class GalaxyMapScreen;    //this is basically a part of that screen anyway
};

#endif
