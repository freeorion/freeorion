// -*- C++ -*-
//SystemIcon.h
#ifndef _SystemIcon_h_
#define _SystemIcon_h_

#ifndef _GGControl_h_
#include "GGControl.h"
#endif

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
    int SystemID() const {return m_systemID;}
    //!@}

    //! \name Mutators //!@{
    virtual void   SizeMove(int x1, int y1, int x2, int y2);
    virtual int    Render() {return 1;}
    virtual int    LClick(const GG::Pt& pt, Uint32 keys) {if(!Disabled()) m_left_click_signal(m_systemID); return 1;}
    virtual int    RClick(const GG::Pt& pt, Uint32 keys) {if(!Disabled()) m_right_click_signal(m_systemID); return 1;}
    virtual int    LDoubleClick(const GG::Pt& pt, Uint32 keys) {if(!Disabled()) m_left_double_click_signal(m_systemID); return 1;}

    void           ShowName(); //!< enables the system name text
    void           HideName(); //!< disables the system name text

    LeftClickedSignalType&       LeftClickedSignal() {return m_left_click_signal;}
    RightClickedSignalType&      RightClickedSignal() {return m_right_click_signal;}
    LeftDoubleClickedSignalType& LeftDoubleClickedSignal() {return m_left_double_click_signal;}
    //!@}

protected:
    int                 m_systemID;    //!< the ID of the System object associated with this SystemIcon
    GG::StaticGraphic*  m_static_graphic; //!< the control used to render the displayed texture
    GG::TextControl*    m_name;        //!< the control that holds the name of the system

private:
    void PositionSystemName();

    //! \name Signals //!@{
    LeftClickedSignalType       m_left_click_signal;
    RightClickedSignalType      m_right_click_signal;
    LeftDoubleClickedSignalType m_left_double_click_signal;
    //!@}
};


#endif
