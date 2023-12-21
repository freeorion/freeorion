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
class IntroScreen final : public GG::Wnd {
public:
    IntroScreen();
    void CompleteConstruction() override;

    void KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;
    void Render() override;

    /**Note:  Since there is poor filesystem tracking of deleted savegames, use
       RequirePreRender() to force an update of the conditional placement of
       the Continue and Load buttons when a player might have deleted the last
       savegame. */
    void PreRender() override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void OnContinue();
    void OnSinglePlayer();  //!< called when single player is clicked
    void OnQuickStart();    //!< called when quick start is clicked
    void OnMultiPlayer();   //!< ...
    void OnLoadGame();
    void OnOptions();
    void OnPedia();
    void OnAbout();
    void OnWebsite();
    void OnCredits();
    void OnExitGame();

    virtual void Close();

private:
    std::shared_ptr<GG::Button>         m_continue;     //!< continues from last autosave
    std::shared_ptr<GG::Button>         m_single_player;//!< opens up the single player game dialog
    std::shared_ptr<GG::Button>         m_quick_start;  //!< starts a single-player game with the default options (no dialog)
    std::shared_ptr<GG::Button>         m_multi_player; //!< opens up the multi player game dialog
    std::shared_ptr<GG::Button>         m_load_game;    //!< loads a saved single player game
    std::shared_ptr<GG::Button>         m_options;      //!< opens the options dialog
    std::shared_ptr<GG::Button>         m_pedia;        //!< shows the pedia window
    std::shared_ptr<GG::Button>         m_about;        //!< opens a dialog to choose to see credits or license
    std::shared_ptr<GG::Button>         m_website;      //!< opens web browser (or however system handles urls) to http://freeorion.org
    std::shared_ptr<GG::Button>         m_credits;      //!< displays credits
    std::shared_ptr<GG::Button>         m_exit_game;    //!< button that exits the program

    std::shared_ptr<CUIWnd>             m_menu;
    std::shared_ptr<GG::StaticGraphic>  m_splash;
    std::shared_ptr<GG::StaticGraphic>  m_logo;
    std::shared_ptr<GG::Label>          m_version;
};


#endif
