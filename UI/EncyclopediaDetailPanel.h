// -*- C++ -*-
#ifndef _ENCYCLOPEDIA_DETAIL_PANEL_H_
#define _ENCYCLOPEDIA_DETAIL_PANEL_H_

#include "CUIWnd.h"

#include <boost/weak_ptr.hpp>

class Tech;
class PartType;
class HullType;
class BuildingType;
class Special;
class Species;
class UniverseObject;
class Empire;
class ShipDesign;
class CUIMultiEdit;
namespace GG {
    class TextControl;
    class StaticGraphic;
    class MultiEdit;
}

/** UI class that displays in-game encyclopedic information about game
  * content.  Tech, PartType, HullType, BuildingType, ShipDesign, etc. */
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

    void            SetText(const std::string& text, bool lookup_in_stringtable = true);
    void            SetTech(const std::string& tech_name);
    void            SetItem(const Tech* tech);
    void            SetPartType(const std::string& part_name);
    void            SetItem(const PartType* part);
    void            SetHullType(const std::string& hull_name);
    void            SetItem(const HullType* hull_type);
    void            SetBuildingType(const std::string& building_name);
    void            SetItem(const BuildingType* building_type);
    void            SetSpecial(const std::string& special_name);
    void            SetItem(const Special* special);
    void            SetSpecies(const std::string& species_name);
    void            SetItem(const Species* species);
    void            SetObject(int object_id);
    void            SetItem(const UniverseObject* obj);
    void            SetEmpire(int empire_id);
    void            SetItem(const Empire* empire);
    void            SetDesign(int design_id);
    void            SetItem(const ShipDesign* design);
    void            SetIncompleteDesign(boost::weak_ptr<const ShipDesign> incomplete_design);
    void            UnsetAll();

    void            Refresh();
    //@}

private:
    void DoLayout();
    bool NothingSet() const;

    std::string                         m_generic_text;
    std::string                         m_tech_name;
    std::string                         m_part_name;
    std::string                         m_hull_name;
    std::string                         m_building_name;
    std::string                         m_special_name;
    std::string                         m_species_name;
    int                                 m_design_id;
    int                                 m_object_id;
    int                                 m_empire_id;
    boost::weak_ptr<const ShipDesign>   m_incomplete_design;

    GG::TextControl*    m_name_text;        // name
    GG::TextControl*    m_cost_text;        // cost and time to build or research
    GG::TextControl*    m_summary_text;     // general purpose item
    GG::MultiEdit*      m_description_box;  // detailed and lengthy description
    GG::StaticGraphic*  m_icon;
    GG::StaticGraphic*  m_other_icon;
};

#endif
