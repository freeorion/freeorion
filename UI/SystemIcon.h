// -*- C++ -*-
//SystemIcon.h
#ifndef _SystemIcon_h_
#define _SystemIcon_h_

#ifndef _GGButton_h_
#include "GGButton.h"
#endif

#ifndef _CUIDrawUtil_h_
#include "CUIDrawUtil.h"
#endif

class System;
namespace GG {
class StaticGraphic;
class TextControl;
}

/** a GUI control that allows interaction with a star system.
    This class allows user interaction with star systems
    on the galaxy map.  It contains the graphic to display the 
    system, along with the object ID of the UniverseObject associated
    with it.
*/

class SystemIcon : public GG::Control
{
public:
    using Wnd::SizeMove;

    //! \name Static Constants //!@{
    static const int ICON_WIDTH;
    static const int ICON_HEIGHT;
    //!@}

    //! \name Signal Types //!@{
    typedef boost::signal<void (int)> LeftClickedSignalType; //!< emitted when the user left clicks the icon, returns the objectID
    typedef boost::signal<void (int)> RightClickedSignalType; //!< emitted when the user right clicks the icon, returns the objectID
    typedef boost::signal<void (int)> LeftDoubleClickedSignalType; //!< emitted when the user left double-clicks the icon, returns the object id
    //!@}

    //! \name Slot Types //!@{
    typedef LeftClickedSignalType::slot_type LeftClickedSlotType; //!< type of functor invoked when the user left clicks
    typedef RightClickedSignalType::slot_type RightClickedSlotType; //!< type of functor invoked when the user right clicks
    typedef LeftDoubleClickedSignalType::slot_type LeftDoubleClickedSlotType; //!< type of functor invoked when the user left double-clicks
    //!@}

    //! \name Structors //!@{
    SystemIcon(int id, double zoom); //!< construct from a universe ID, to be placed in a MapWnd at zoom level \a zoom
    ~SystemIcon();      //!< dtor
    //!@}

    //! \name Accessors //!@{
    int SystemID() const {return m_system_ID;}
    //!@}

    //! \name Mutators //!@{
    virtual void   SizeMove(int x1, int y1, int x2, int y2);
    virtual int    Render() {return 1;}
    virtual int    LClick(const GG::Pt& pt, Uint32 keys) {if(!Disabled()) m_left_click_signal(m_system_ID); return 1;}
    virtual int    RClick(const GG::Pt& pt, Uint32 keys) {if(!Disabled()) m_right_click_signal(m_system_ID); return 1;}
    virtual int    LDoubleClick(const GG::Pt& pt, Uint32 keys) {if(!Disabled()) m_left_double_click_signal(m_system_ID); return 1;}

    void           ShowName(); //!< enables the system name text
    void           HideName(); //!< disables the system name text

    LeftClickedSignalType&       LeftClickedSignal() {return m_left_click_signal;}
    RightClickedSignalType&      RightClickedSignal() {return m_right_click_signal;}
    LeftDoubleClickedSignalType& LeftDoubleClickedSignal() {return m_left_double_click_signal;}
    //!@}

private:
    class FleetButton : public GG::Button
    {
    public:
        /** \name Structors */ //@{
        FleetButton(int x, int y, int w, int h, GG::Clr color, ShapeOrientation orientation); ///< basic ctor
        FleetButton(const GG::XMLElement& elem); ///< ctor that constructs a FleetButton object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a FleetButton object
        //@}

        /** \name Accessors */ //@{
        virtual bool           InWindow(const GG::Pt& pt) const;
        virtual GG::XMLElement XMLEncode() const;

        /** returns the orientation of the fleet marker (will be one of SHAPE_LEFT ans SHAPE_RIGHT) */
        ShapeOrientation Orientation() const {return m_orientation;}
        //@}

        /** \name Mutators */ //@{
        /** sets the orientation of the fleet marker (must be one of SHAPE_LEFT ans SHAPE_RIGHT; otherwise,
            SHAPE_LEFT will be used) */
        void SetOrientation(ShapeOrientation orientation) {m_orientation = orientation;}
        //@}

    protected:
        /** \name Mutators */ //@{
        virtual void   RenderUnpressed();
        virtual void   RenderPressed();
        virtual void   RenderRollover();
        //@}

    private:
        ShapeOrientation m_orientation;
    };

    void CreateFleetButtons(const System* sys);
    void Refresh();
    void PositionSystemName();

    int                 m_system_ID;      //!< the ID of the System object associated with this SystemIcon
    GG::StaticGraphic*  m_static_graphic; //!< the control used to render the displayed texture
    GG::TextControl*    m_name;           //!< the control that holds the name of the system

    std::map<int, FleetButton*> m_stationary_fleet_markers; //!< the fleet buttons for the fleets that are stationary in the system, indexed by Empire ID of the owner
    std::map<int, FleetButton*> m_moving_fleet_markers;     //!< the fleet buttons for the fleets that are under orders to move out of the system, indexed by Empire ID of the owner

    LeftClickedSignalType       m_left_click_signal;
    RightClickedSignalType      m_right_click_signal;
    LeftDoubleClickedSignalType m_left_double_click_signal;
};


#endif
