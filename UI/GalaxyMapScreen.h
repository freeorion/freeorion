// -*- C++ -*-
//GalaxyMapScreen.h
#ifndef _GalaxyMapScreen_h_
#define _GalaxyMapScreen_h_

#include <vector>

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#ifndef _GGButton_h_
#include "GGButton.h"
#endif

#ifndef _GGTexture_h_
#include "GGTexture.h"
#endif

#ifndef _GGTextControl_h_
#include "GGTextControl.h"
#endif

#ifndef _OrderSet_h_
#include "../util/OrderSet.h"
#endif

#ifndef _ClientEmpireManager_h_
#include "../Empire/ClientEmpireManager.h"
#endif

#ifndef _ClientUniverse_h_
#include "../universe/ClientUniverse.h"
#endif

#ifndef _MapWnd_h_
#include "MapWnd.h"
#endif

class SystemIcon;  //extends staticgraphic
class Icon;         //a staticgraphic that responds to events, extends staticgraphic
class MapWnd;
    

//! Contains the main game screen
/** This screen consists of the galaxy map, interface buttons
    menus, etc.
    
 */ 
class GalaxyMapScreen : public GG::Wnd
{

public:
    //! \name Structors //!@{
    GalaxyMapScreen();     //!< default ctor
    ~GalaxyMapScreen();    //!< default dtor
    
    //!@}
    
    //! \name Accessors //!@{
    //!@}
    
    //! name Mutators //!@{
    void InitTurn();        //!< sets up a turn
    virtual int Render();   //!< rendering code
    //!@} 

private:
    //! \name Controls //!@{
    GG::TextControl* m_turn_indicator; //!< displays current turn number
    GG::Button*     m_turn_button;    //!< pressed to end the turn
    
    //! \name Interface Buttons
    GG::Button*     m_sitrep_button; //!< displays the sitrep when pressed
    //more to come...
    //!@}

    //!@}
    
    //! Misc screens //!@{
    MapWnd* m_map_wnd;               //!< the actual window that holds the systems
    //!@}
    
    //! \name Private Members //!@{
    int m_selected_index;    //!< the index of the star that is currently selected, -1 if none
    OrderSet* m_orders;       //!< the orderset of the current turn
    static GG::Rect s_scissor_rect;  //!< portion of the screen that this window gets drawn to
    static double s_scale_factor;    //!< the scale level of the map
    
    //!@}
    
    friend class MapWnd;    //!< friend because it needs access to the screen rectangle
    friend class SystemIcon;//!< friend because it needs access to the screen rectangle and zoom level
};//GalaxyMapScreen

#endif
