// -*- C++ -*-
#ifndef _SitRepPanel_h_
#define _SitRepPanel_h_


#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

#ifndef _GGSignalsAndSlots_h_
#include "GGSignalsAndSlots.h"
#endif

class CUIListBox;
class CUIButton;

namespace GG {class TextControl;}


class SitRepPanel : public CUI_Wnd
{
public:
    /** \name Structors */ //@{
    SitRepPanel(int x, int y, int w, int h);
    //@}

    void Update( );

    void OnClose();   

private:

    CUIListBox*           m_sitreps_lb; // Eventually this will be a subclass whic nows have to render and handle links
    CUIButton*            m_close;    //!< button that closes report
    GG::TextControl*      m_title;
};

#endif // _SidePanel_h_
