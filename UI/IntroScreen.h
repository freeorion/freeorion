//IntroScreen.h

#ifndef _IntroScreen_h_
#define _IntroScreen_h_

#ifndef _GGApp_h_
#include "GGApp.h"
#endif

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#ifndef _GGTexture_h_
#include "GG::Texture.h"
#endif

#ifndef _GGButton_h_
#include "GGButton.h"
#endif

//! This is the first screen the user sees in FreeOrion.  It will always be the
//! size of the Application main window.  It will display a splash screen with 
//! a menu window on one side.

class IntroScreen : public GG::Wnd
{
public:
//! \name Structors
//!@{

    IntroScreen();                              //!< default ctor
    IntroScreen(const GG::XMLElement &elem);    //!< construction from an XML element
    ~IntroScreen();                             //!< default dtor

//!@}

//! \name Accessors
//!@{

//!@}

//! \name Mutators
//!@{

    virtual int Render();    //!< rendering code
    
    void OnStartGame();    //!< called when m_start_game is clicked
    void OnQuickStart();    //!< called when m_quick_start is clicked
    void OnExitGame();    //!< called when m_exit_game is clicked   

//!@}

//! \name Overrides
//!@{
    
    virtual GG::XMLElement XMLEncode() const;    //!< load from XML element

//!@}

private:
//! \name Data Members
//!@{

    GG::Texture* m_background;    //!< the background texture

//!@}

//! \name GG Controls
//!@{

    GG::Button* m_start_game;    //!< button that starts a new game
    GG::Button* m_quick_start;    //!< button that starts a quick game (no universe setup, etc.)
    GG::Button* m_exit_game;    //!< button that exits the program

//!@}

};//IntroScreen

#endif
