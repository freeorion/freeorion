// -*- C++ -*-
//IntroScreen.h

#ifndef _IntroScreen_h_
#define _IntroScreen_h_

#ifndef _GGApp_h_
#include "GGApp.h"
#endif

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

class CUIButton;
namespace GG {class StaticGraphic;}

//! This is the first screen the user sees in FreeOrion.  It will always be the
//! size of the Application main window.  It will display a splash screen with 
//! a menu window on one side.
class IntroScreen : public CUI_Wnd
{
public:
    /** \name Structors*/ //!@{
    IntroScreen();                              //!< default ctor
    IntroScreen(const GG::XMLElement &elem);    //!< construction from an XML element
    ~IntroScreen();                             //!< default dtor
    //!@}

    /** \name Accessors*/ //!@{
    //!@}

    /** \name Mutators*/ //!@{
    void OnSinglePlayer();  //!< called when m_single_player is clicked
    void OnMultiPlayer();   //!< called when m_multi_player is clicked
    void OnOptions();    //!< called when m_options is clicked
    void OnAbout();    //!< called when m_about is clicked
    void OnExitGame();    //!< called when m_exit_game is clicked
    
    virtual void Close() {OnExitGame();}    //!< override to exit the game 
    //!@}

    virtual GG::XMLElement XMLEncode() const;    //!< load from XML element

private:
    /** \name GG Controls*/ //!@{
    GG::StaticGraphic* m_bg_graphic; //!< the background image shown in the intro screen
    CUIButton* m_single_player;    //!< opens up the single player game dialog
    CUIButton* m_multi_player;    //!< opens up the multi player game dialog
    CUIButton* m_options;	//!< opens the options dialog
    CUIButton* m_about;	//!< opens a dialog to choose to see credits or license
    CUIButton* m_exit_game;    //!< button that exits the program
    //!@}

};

#endif
