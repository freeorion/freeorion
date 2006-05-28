// -*- C++ -*-
//InGameMenu.h
#ifndef _InGameMenu_h_
#define _InGameMenu_h_

#ifndef _CUIWnd_h_
#include "CUIWnd.h"
#endif

class CUIButton;

class InGameMenu : public CUIWnd
{
public:
    /** \name Structors */ //@{
    InGameMenu();  //!< default ctor
    ~InGameMenu(); //!< dtor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void KeyPress (GG::Key key, Uint32 key_mods);
    //@}

protected:
    virtual int MinimizedLength() const;//!< the width of a minimized InGameMenu wnd

private:
    void Init();        //!< Attaches children and connects signals
    void Save();        //!< when m_save_btn button is pressed
    void Load();        //!< when m_load_btn button is pressed
    void Options();     //!< when m_options_btn button is pressed
    void Exit();        //!< when m_quit_btn button is pressed
    void Done();        //!< when m_done_btn is pressed

    CUIButton*  m_save_btn;   //!< Save game button
    CUIButton*  m_load_btn;   //!< Load game button
    CUIButton*  m_options_btn;//!< Options button
    CUIButton*  m_done_btn;   //!< Done button
    CUIButton*  m_exit_btn;   //!< Quit game button
};

#endif // _InGameMenu_h_
