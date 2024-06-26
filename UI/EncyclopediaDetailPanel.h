#ifndef _ENCYCLOPEDIA_DETAIL_PANEL_H_
#define _ENCYCLOPEDIA_DETAIL_PANEL_H_

#include <memory>
#include <GG/GGFwd.h>
#include "CUIWnd.h"
#include "../universe/EnumsFwd.h"


class Planet;
class Tech;
class Policy;
class ShipPart;
class ShipHull;
class BuildingType;
class Special;
class Species;
class FieldType;
class UniverseObject;
class Empire;
class ShipDesign;
class GraphControl;


//! UI class that displays in-game encyclopedic information about game content.
//!
//! Tech, ShipPart, ShipHull, BuildingType, ShipDesign, etc.
class EncyclopediaDetailPanel final : public CUIWnd {
public:
    EncyclopediaDetailPanel(GG::Flags<GG::WndFlag> flags = GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE |
                                                           GG::RESIZABLE | CLOSABLE | PINABLE,
                            std::string_view config_name = "");
    void CompleteConstruction() override;
    ~EncyclopediaDetailPanel() = default;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;
    void KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;
    void PreRender() override;
    void Render() override;

    /** Need to redefine this so that icons and name can be put at the top of
      * the Wnd, rather than being restricted to the client area of a CUIWnd */
    GG::Pt ClientUpperLeft() const noexcept override { return GG::Wnd::UpperLeft(); }

    void AddItem(std::string_view type, std::string name); // adds a new item to m_items
    void PopItem();
    void ClearItems();
    int GetItemsSize() { return m_items.size(); }

    void SetPlanet(int planet_id);
    void SetItem(const std::shared_ptr<const Planet>& planet);
    void SetTech(std::string tech_name);
    void SetItem(const Tech* tech);
    void SetPolicy(std::string policy_name);
    void SetItem(const Policy* policy);
    void SetShipPart(std::string part_name);
    void SetItem(const ShipPart* part);
    void SetShipHull(std::string hull_name);
    void SetItem(const ShipHull* ship_hull);
    void SetBuildingType(std::string building_name);
    void SetItem(const BuildingType* building_type);
    void SetSpecial(std::string special_name);
    void SetItem(const Special* special);
    void SetSpecies(std::string species_name);
    void SetItem(const Species* species);
    void SetFieldType(std::string field_type_name);
    void SetItem(const FieldType* field_type);
    void SetObject(int object_id);
    void SetObject(std::string object_id);
    void SetItem(const std::shared_ptr<const UniverseObject>& obj);
    void SetEmpire(int empire_id);
    void SetEmpire(std::string empire_id);
    void SetItem(const Empire* empire);
    void SetDesign(int design_id);
    void SetDesign(std::string design_id);
    void SetItem(const ShipDesign* design);
    void SetIncompleteDesign(std::weak_ptr<const ShipDesign> incomplete_design);
    void SetMeterType(std::string meter_string);
    void SetMeterType(MeterType meter_type);
    void SetItem(MeterType meter_type);
    void SetGraph(std::string graph_id);
    void SetIndex();
    void SetEncyclopediaArticle(std::string name);

    void Refresh();
    void OnIndex();
    void OnBack();
    void OnNext();

    mutable boost::signals2::signal<void ()> ClosingSignal;

protected:
    void SetText(const std::string& text, bool lookup_in_stringtable = true);
    void InitBuffers() override;

private:
    void CloseClicked() override;

    void DoLayout();
    void RefreshImpl();

    void HandleLinkClick(const std::string& link_type, const std::string& data);
    void HandleLinkDoubleClick(const std::string& link_type, const std::string& data);
    void HandleSearchTextEntered();

    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event) override;

    static std::list<std::pair<std::string, std::string>>           m_items;    // stores all items which have been observed in the past
                                                                                // .first == item type; .second == item.name
    static std::list<std::pair<std::string, std::string>>::iterator m_items_it; // stores actual position within m_items

    std::weak_ptr<const ShipDesign> m_incomplete_design;

    std::shared_ptr<GG::Label>          m_name_text;            // name
    std::shared_ptr<GG::Label>          m_cost_text;            // cost and time to build or research
    std::shared_ptr<GG::Label>          m_summary_text;         // general purpose item
    std::shared_ptr<GG::RichText>       m_description_rich_text;// detailed and lengthy description
    std::shared_ptr<GG::ScrollPanel>    m_scroll_panel;         // scroller for m_description_rich_text
    std::shared_ptr<GG::StaticGraphic>  m_icon;
    std::shared_ptr<GG::Button>         m_index_button;
    std::shared_ptr<GG::Button>         m_back_button;
    std::shared_ptr<GG::Button>         m_next_button;
    std::shared_ptr<GG::Edit>           m_search_edit;          // box to type to search
    std::shared_ptr<GraphControl>       m_graph;
    bool                                m_needs_refresh = false;// Indicates that data is stale.
};


#endif
