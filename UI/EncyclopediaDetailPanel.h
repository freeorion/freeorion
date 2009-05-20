// -*- C++ -*-
#ifndef _ENCYCLOPEDIA_DETAIL_PANEL_H_
#define _ENCYCLOPEDIA_DETAIL_PANEL_H_

#include "CUIWnd.h"

class Tech;
class PartType;
class HullType;
class BuildingType;
class ShipDesign;
class Special;
class CUIMultiEdit;
namespace GG {
    class TextControl;
    class StaticGraphic;
    class MultiEdit;
}

/** UI class that displays in-game encyclopedic information about game content.  Tech, PartType, HullType,
  * Buildingtype
  */
class EncyclopediaDetailPanel : public CUIWnd {
public:
    //! \name Structors //!@{
    EncyclopediaDetailPanel(GG::X x, GG::Y y);
    //!@}

    /** \name Mutators */ //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void    Render();
    virtual void    LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);

    /* need to redefine this so that icons and name can be put at the top of the Wnd, rather
       than being restricted to the client area of a CUIWnd */
    virtual GG::Pt  ClientUpperLeft() const;

    void            SetItem(const Tech* tech);
    void            SetItem(const PartType* part);
    void            SetItem(const HullType* hull);
    void            SetItem(const BuildingType* building);
    void            SetItem(const ShipDesign* design);
    void            SetItem(const Special* special);
    void            UnsetAll();

    void            Refresh();
    //@}

private:
    void DoLayout();
    bool NothingSet();

    const Tech*         m_tech;
    const PartType*     m_part;
    const HullType*     m_hull;
    const BuildingType* m_building;
    const ShipDesign*   m_design;
    const Special*      m_special;

    GG::TextControl*    m_name_text;        // name
    GG::TextControl*    m_cost_text;        // cost and time to build or research
    GG::TextControl*    m_summary_text;     // general purpose item
    GG::MultiEdit*      m_description_box;  // detailed and lengthy description
    GG::StaticGraphic*  m_icon;
    GG::StaticGraphic*  m_other_icon;
};

#endif
