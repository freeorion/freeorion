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
    //! \name Structors //!@{
    SystemIcon(int id); //!< standard construction from a universe ID
    ~SystemIcon();      //!< dtor
    
    //!@}
    //! \name Accessors //!@{
    
    //!@}
    //! \name Mutators //!@{
    virtual int Render();    //!< rendering code
    //!@}
    
protected:
    int                 m_systemID;    //!< the ID of the System object associated with this SystemIcon
    GG::Texture*        m_graphic;     //!< the graphic displayed by thie object
    GG::StaticGraphic*  m_static_graphic; //!< the control used to render the displayed texture
    GG::TextControl*    m_name;        //!< the control that holds the name of the system
};


#endif
