// -*- C++ -*-
#ifndef _SitRepPanel_h_
#define _SitRepPanel_h_

#ifndef _CUIWnd_h_
#include "CUIWnd.h"
#endif


class CUIListBox;
class CUIButton;
namespace GG {class TextControl;}


class SitRepPanel : public CUIWnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> ClosingSignalType;   ///< emitted when the window is manually closed by user by clicking on the sitrep panel itself
    //@}

    /** \name Structors */ //@{
    SitRepPanel(GG::X x, GG::Y y, GG::X w, GG::Y h); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void KeyPress (GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);
    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    void Update(); ///< loads all the relevant SitReps into the window

    virtual void OnClose();

    mutable ClosingSignalType ClosingSignal;
    //@}

protected:
    virtual void CloseClicked();

private:
    CUIListBox*           m_sitreps_lb;
};

#endif // _SitRepPanel_h_
