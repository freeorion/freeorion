// -*- C++ -*-
//InGameOptions.h
#ifndef _InGameOptions_h_
#define _InGameOptions_h_

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

class CUIButton;

class InGameOptions : public CUI_Wnd
{
public:
    /** \name Structors */ //@{
    InGameOptions();  //!< default ctor
    ~InGameOptions(); //!< dtor
    //@}

    /** \name Mutators */ //@{
    virtual int Render();
    virtual int Keypress (GG::Key key, Uint32 key_mods);
    //@}

private:
    void Init();          //!< Attaches children and connects signals
    void Save();        //!< when m_save_btn button is pressed
    void Load();        //!< when m_load_btn button is pressed
    void Quit();        //!< when m_quit_btn button is pressed
    void Done();        //!< when m_done_btn is pressed

    CUIButton*  m_save_btn;   //!< Save game button
    CUIButton*  m_load_btn;   //!< Load game button
    CUIButton*  m_done_btn;   //!< Done button
    CUIButton*  m_quit_btn;	  //!< Quit game button
};

#endif
