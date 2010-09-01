// -*- C++ -*-
#ifndef _DesignWnd_h_
#define _DesignWnd_h_

#include <GG/Wnd.h>

class EncyclopediaDetailPanel;

/** Lets the player design ships */
class DesignWnd : public GG::Wnd {
public:
    /** \name Structors */ //@{
    DesignWnd(GG::X w, GG::Y h);
    //@}

    /** \name Mutators */ //@{
    void    Reset();
    void    Sanitize();
    void    Render();

    /** Shows \a part_type in design encyclopedia window */
    void    ShowPartTypeInEncyclopedia(const std::string& part_type);

    /** Shows \a hull_type in design encyclopedia window */
    void    ShowHullTypeInEncyclopedia(const std::string& hull_type);

    /** Shows ship design with id \a design_id in design encyclopedia window */
    void    ShowShipDesignInEncyclopedia(int design_id);

    /** Enables, or disables if \a enable is false, issuing orders via this DesignWnd. */
    void    EnableOrderIssuing(bool enable = true);
    //@}

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
