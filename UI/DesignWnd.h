// -*- C++ -*-
#ifndef _DesignWnd_h_
#define _DesignWnd_h_

#include <GG/Wnd.h>

class EncyclopediaDetailPanel;

/** Lets the player design ships */
class DesignWnd : public GG::Wnd {
public:
    /** \name Structors */ //@{
    DesignWnd(int w, int h);
    //@}

    /** \name Mutators */ //@{
    void    Reset();
    void    Sanitize();
    void    Render();
    //@}

    mutable boost::signal<void ()>  EmpireDesignsChangedSignal; //!< emitted when a new design is created or an old design is forgotten

private:
    class BaseSelector;     // has tabs to pick empty hull, previously-saved design or an autodesign template to start new design
    class PartPalette;      // shows parts that can be clicked for detailed or dragged on slots in design
    class MainPanel;        // shows image of hull, slots and parts, design name and description entry boxes, confirm button

    void    AddDesign();    ///< adds current design to those stored by this empire, allowing ships of this design to be produced

    EncyclopediaDetailPanel*    m_detail_panel;
    BaseSelector*               m_base_selector;
    PartPalette*                m_part_palette;
    MainPanel*                  m_main_panel;
};

#endif // _DesignWnd_h_
