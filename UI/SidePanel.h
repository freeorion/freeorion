// -*- C++ -*-
#ifndef _SidePanel_h_
#define _SidePanel_h_

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#ifndef _GGSignalsAndSlots_h_
#include "GGSignalsAndSlots.h"
#endif

#ifndef _GGTexture_h_
#include "GGTexture.h"
#endif

#ifndef _ProdCenter_h_
#include "../universe/ProdCenter.h"
#endif

class CUIDropDownList;
class CUIScroll;
class Planet;
class System;
namespace GG {class TextControl;}


class SidePanel : public GG::Wnd
{
public:
    /** \name Structors */ //@{
    SidePanel(int x, int y, int w, int h);
    //@}

    /** \name Accessors */ //@{
    virtual bool InWindow(const GG::Pt& pt) const;
    //@}

    /** \name Mutators */ //@{
    virtual int  Render();

    void         SetSystem(int system_id); ///< sets the system currently being viewed in the side panel
    //@}

private:
    /** a single planet's info and controls; several of these may appear at any one time in a SidePanel */
    class PlanetPanel : public GG::Wnd
    {
    public:
        /** \name Signal Types */ //@{
        typedef boost::signal<void ()> DoubleClickedSignalType; ///< emitted when the planet graphic is double clicked by the user
        //@}
   
        /** \name Slot Types */ //@{
        typedef DoubleClickedSignalType::slot_type DoubleClickedSlotType; ///< type of functor(s) invoked on a DoubleClickedSignalType
        //@}

        /** \name Structors */ //@{
        PlanetPanel(const Planet& planet, int y, int parent_width, int h); ///< basic ctor
        //@}

        /** \name Accessors */ //@{
        virtual bool InWindow(const GG::Pt& pt) const;
        //@}

        /** \name Mutators */ //@{
        virtual int Render();

        DoubleClickedSignalType& DoubleClickedSignal() {return m_double_clicked_sig;} ///< returns the double clicked signal object for this Planet panel
        //@}

    private:
        int  PlanetDiameter() const;
        bool InPlanet(const GG::Pt& pt) const;
        void BuildSelected(int idx) const;

        int m_planet_id;

        int m_parent_visible_width; ///< the width of the SidePanel, not counting the amount that the planets hang over

        GG::SubTexture     m_planet_graphic;
        CUIDropDownList*   m_construction;
        std::vector< ProdCenter::BuildType > m_construction_prod_idx;

        DoubleClickedSignalType       m_double_clicked_sig;

        static GG::SubTexture m_pop_icon;
        static GG::SubTexture m_industry_icon;
        static GG::SubTexture m_research_icon;
        static GG::SubTexture m_mining_icon;
        static GG::SubTexture m_farming_icon;
    };

    const System*     m_system;

    GG::TextControl*  m_name_text;

    std::vector<GG::SubTexture> m_fleet_icons;
    std::vector<PlanetPanel*>   m_planet_panels;

    int               m_first_row_shown;
    int               m_first_col_shown;

    CUIScroll*        m_vscroll; ///< the vertical scroll (for viewing all the planet panes)
    CUIScroll*        m_hscroll; ///< the horizontal scroll (for viewing all the fleet icons)
};

#endif // _SidePanel_h_
