#ifndef _InGameMenu_h_
#define _InGameMenu_h_

#include "CUIWnd.h"

class InGameMenu : public CUIWnd {
public:
    /** \name Structors */ //@{
    InGameMenu();
    void CompleteConstruction() override;

    ~InGameMenu();
    //@}

    /** \name Mutators */ //@{
    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;

    void         DoLayout();
    //@}

protected:
    GG::Rect CalculatePosition() const override;

private:
    void Save();        //!< when m_save_btn button is pressed
    void Load();        //!< when m_load_btn button is pressed
    void Options();     //!< when m_options_btn button is pressed
    void Concede();     //!< when m_concede_btn button is pressed
    void Resign();      //!< when m_resign_btn button is pressed
    void Done();        //!< when m_done_btn is pressed

    GG::X ButtonWidth() const;      //!< Helper function
    GG::Y ButtonCellHeight() const; //!< Helper function

    std::shared_ptr<GG::Button> m_save_btn;
    /** Depending on singleplayer or multiplayer mode
      * this menu shows or Load or Concede button */
    std::shared_ptr<GG::Button> m_load_or_concede_btn;
    std::shared_ptr<GG::Button> m_options_btn;
    std::shared_ptr<GG::Button> m_done_btn;
    std::shared_ptr<GG::Button> m_resign_btn;
};

#endif // _InGameMenu_h_
