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

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

//! This is the first screen the user sees in FreeOrion.  It will always be the
//! size of the Application main window.  It will display a splash screen with 
//! a menu window on one side.

class IntroScreen : public CUI_Wnd
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

//    virtual int Render();    //!< rendering code

    void OnSinglePlayer();  //!< called when m_single_player is clicked
    void OnMultiPlayer();   //!< called when m_multi_player is clicked
    void OnOptions();    //!< called when m_options is clicked
    void OnAbout();    //!< called when m_about is clicked
    void OnExitGame();    //!< called when m_exit_game is clicked
    
    inline virtual void Close() {OnExitGame();}    //!< override to exit the game 

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

    GG::Button* m_single_player;    //!< opens up the single player game dialog
    GG::Button* m_multi_player;    //!< opens up the multi player game dialog
    GG::Button* m_options;	//!< opens the options dialog
    GG::Button* m_about;	//!< opens a dialog to choose to see credits or license
    GG::Button* m_exit_game;    //!< button that exits the program

//!@}

};//IntroScreen

#endif
