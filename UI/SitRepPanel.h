// -*- C++ -*-
#ifndef _SitRepPanel_h_
#define _SitRepPanel_h_

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif


class CUIListBox;
class CUIButton;
namespace GG {class TextControl;}


class SitRepPanel : public CUI_Wnd
{
public:
    /** \name Structors */ //@{
    SitRepPanel(int x, int y, int w, int h); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual int Keypress (GG::Key key, Uint32 key_mods);

    void Update(); ///< loads all the relevant SitReps into the window

    virtual void OnClose();
    //@}

private:
    CUIListBox*           m_sitreps_lb;
    CUIButton*            m_close;      //!< the button that closes the report
    GG::TextControl*      m_title;
};

#endif // _SidePanel_h_
