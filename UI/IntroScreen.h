#ifndef _IntroScreen_h_
#define _IntroScreen_h_

#include <GG/GGFwd.h>
#include <GG/GUI.h>
#include <GG/Wnd.h>

class CUIWnd;
class CreditsWnd;


/** This is the first screen the user sees in FreeOrion.  It will always be the
  * size of the Application main window.  It will display a splash screen with
  * a menu window on one side. */
class IntroScreen : public GG::Wnd {
public:
    /** \name Structors*/ //!@{
    IntroScreen();

    ~IntroScreen();
    //!@}

    void CompleteConstruction() override;

    /** \name Mutators*/ //!@{
    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;

    void Render() override;

    void            OnSinglePlayer();  //!< called when single player is clicked
    void            OnQuickStart();    //!< called when quick start is clicked
    void            OnMultiPlayer();   //!< ...
    void            OnLoadGame();
    void            OnOptions();
    void            OnPedia();
    void            OnAbout();
    void            OnWebsite();
    void            OnCredits();
    void            OnExitGame();


    void            DoLayout();
    virtual void    Close();
    //!@}

private:
    GG::Button*         m_single_player;//!< opens up the single player game dialog
    GG::Button*         m_quick_start;  //!< starts a single-player game with the default options (no dialog)
    GG::Button*         m_multi_player; //!< opens up the multi player game dialog
    GG::Button*         m_load_game;    //!< loads a saved single player game
    GG::Button*         m_options;      //!< opens the options dialog
    GG::Button*         m_pedia;        //!< shows the pedia window
    GG::Button*         m_about;        //!< opens a dialog to choose to see credits or license
    GG::Button*         m_website;      //!< opens web browser (or however system handles urls) to http://freeorion.org
    GG::Button*         m_credits;      //!< displays credits
    GG::Button*         m_exit_game;    //!< button that exits the program

    CUIWnd*             m_menu;
    GG::StaticGraphic*  m_splash;
    GG::StaticGraphic*  m_logo;
    GG::Label*          m_version;
};

#endif // _IntroScreen_h_
