// -*- C++ -*-
//SystemIcon.h
#ifndef _SystemIcon_h_
#define _SystemIcon_h_


#ifndef _GGTexture_h_
#include "GGTexture.h"
#endif

#ifndef _GGTextControl_h_
#include "GGTextControl.h"
#endif

#ifndef _GGControl_h_
#include "GGControl.h"
#endif

#ifndef _GGStaticGraphic_h_
#include "GGStaticGraphic.h"
#endif

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

//! This class is a GUI control that allows interaction with a star system
/**
    This class allows user interaction with star systems
    on the galaxy map.  It contains the graphic to display the 
    system, along with the object ID of the UniverseObject associated
    with it.
*/

class SystemIcon : public GG::Control
{
public:
    //! \name Static Constants //!@{
    static const int ICON_WIDTH;
    static const int ICON_HEIGHT;
    //!@}
    
public:
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
    SystemIcon(int id); //!< standard construction from a universe ID
    ~SystemIcon();      //!< dtor
    
    //!@}
    //! \name Accessors //!@{
    
    //!@}
    //! \name Mutators //!@{
    virtual int Render() {return 1;}
    virtual int LClick(const GG::Pt& pt, Uint32 keys) {if(!Disabled()) m_left_click_signal(m_systemID); return 1;}
    virtual int RClick(const GG::Pt& pt, Uint32 keys) {if(!Disabled()) m_right_click_signal(m_systemID); return 1;}
    virtual int LDoubleClick(const GG::Pt& pt, Uint32 keys) {if(!Disabled()) m_left_double_click_signal(m_systemID); return 1;}
    
    LeftClickedSignalType&       LeftClickedSignal() {return m_left_click_signal;}
    RightClickedSignalType&      RightClickedSignal() {return m_right_click_signal;}
    LeftDoubleClickedSignalType& LeftDoubleClickedSignal() {return m_left_double_click_signal;}
    //!@}

protected:
    int                 m_systemID;    //!< the ID of the System object associated with this SystemIcon
    GG::StaticGraphic*  m_static_graphic; //!< the control used to render the displayed texture
    GG::TextControl*    m_name;        //!< the control that holds the name of the system
    
private:
    //! \name Signals //!@{
    LeftClickedSignalType       m_left_click_signal;
    RightClickedSignalType      m_right_click_signal;
    LeftDoubleClickedSignalType m_left_double_click_signal;
    //!@}
};


#endif
