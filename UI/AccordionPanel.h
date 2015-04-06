// -*- C++ -*-
#ifndef _AccordionPanel_h_
#define _AccordionPanel_h_

#include <GG/Wnd.h>

namespace GG {
    class Button;
    class StaticGraphic;
};

class AccordionPanel : public GG::Wnd {
public:
    /** \name Structors */ //@{
    AccordionPanel(GG::X w);
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    //@}

    mutable boost::signals2::signal<void ()> ExpandCollapseSignal;

protected:
    /** \name Mutators */ //@{
    void SetCollapsed(bool collapsed);
    //@}

    GG::Button*                     m_expand_button;        ///< at top right of panel, toggles the panel open/closed to show details or minimal summary

private:
    /** \name Mutators */ //@{
    void DoLayout();
    //@}
};

#endif
