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
    virtual void Keypress (GG::Key key, Uint32 key_mods);
    virtual void SizeMove(int x1, int y1, int x2, int y2);

    void Update(); ///< loads all the relevant SitReps into the window

    virtual void OnClose();
    //@}

private:
    CUIListBox*           m_sitreps_lb;
};

inline std::pair<std::string, std::string> SitRepPanelRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _SitRepPanel_h_
