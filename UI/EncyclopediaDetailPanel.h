#ifndef _ENCYCLOPEDIA_DETAIL_PANEL_H_
#define _ENCYCLOPEDIA_DETAIL_PANEL_H_


#include <GG/GGFwd.h>

#include "CUIWnd.h"
#include "../universe/EnumsFwd.h"

#include <memory>


class Planet;
class Tech;
class PartType;
class HullType;
class BuildingType;
class Special;
class Species;
class FieldType;
class UniverseObject;
class Empire;
class ShipDesign;
class GraphControl;


/** UI class that displays in-game encyclopedic information about game
  * content.  Tech, PartType, HullType, BuildingType, ShipDesign, etc. */
class EncyclopediaDetailPanel : public CUIWnd {
public:
    //! \name Structors //!@{
    EncyclopediaDetailPanel(GG::Flags<GG::WndFlag> flags = GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE |
                                                           GG::RESIZABLE | CLOSABLE | PINABLE,
                            const std::string& config_name = "");
    void CompleteConstruction() override;
    virtual ~EncyclopediaDetailPanel();
    //!@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;
    void PreRender() override;
    void Render() override;

    /** Need to redefine this so that icons and name can be put at the top of
      * the Wnd, rather than being restricted to the client area of a CUIWnd */
    GG::Pt ClientUpperLeft() const override;

    void AddItem(const std::string& type, const std::string& name); // adds a new item to m_items
    void PopItem();
    void ClearItems();
    int GetItemsSize() { return m_items.size(); }

    void SetPlanet(int planet_id);
    void SetItem(std::shared_ptr<const Planet> planet);
    void SetTech(const std::string& tech_name);
    void SetItem(const Tech* tech);
    void SetPartType(const std::string& part_name);
    void SetItem(const PartType* part);
    void SetHullType(const std::string& hull_name);
    void SetItem(const HullType* hull_type);
    void SetBuildingType(const std::string& building_name);
    void SetItem(const BuildingType* building_type);
    void SetSpecial(const std::string& special_name);
    void SetItem(const Special* special);
    void SetSpecies(const std::string& species_name);
    void SetItem(const Species* species);
    void SetFieldType(const std::string& field_type_name);
    void SetItem(const FieldType* field_type);
    void SetObject(int object_id);
    void SetObject(const std::string& object_id);
    void SetItem(std::shared_ptr<const UniverseObject> obj);
    void SetEmpire(int empire_id);
    void SetEmpire(const std::string& empire_id);
    void SetItem(const Empire* empire);
    void SetDesign(int design_id);
    void SetDesign(const std::string& design_id);
    void SetItem(const ShipDesign* design);
    void SetIncompleteDesign(std::weak_ptr<const ShipDesign> incomplete_design);
    void SetMeterType(const std::string& meter_string);
    void SetItem(const MeterType& meter_type);
    void SetGraph(const std::string& graph_id);
    void SetIndex();
    void SetEncyclopediaArticle(const std::string& name);

    void Refresh();
    void OnIndex();
    void OnBack();
    void OnNext();
    //@}

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

    static std::list<std::pair <std::string, std::string>>              m_items;    // stores all items which have been observed in the past
                                                                                    // .first == item type; .second == item.name
    static std::list<std::pair <std::string, std::string>>::iterator    m_items_it; // stores actual position within m_items

    std::weak_ptr<const ShipDesign> m_incomplete_design;

    std::shared_ptr<GG::Label>          m_name_text = nullptr;              // name
    std::shared_ptr<GG::Label>          m_cost_text = nullptr;              // cost and time to build or research
    std::shared_ptr<GG::Label>          m_summary_text = nullptr;           // general purpose item
    std::shared_ptr<GG::RichText>       m_description_rich_text = nullptr;  // detailed and lengthy description
    std::shared_ptr<GG::ScrollPanel>    m_scroll_panel = nullptr;           // scroller for m_description_rich_text
    std::shared_ptr<GG::StaticGraphic>  m_icon = nullptr;
    std::shared_ptr<GG::Button>         m_index_button = nullptr;
    std::shared_ptr<GG::Button>         m_back_button = nullptr;
    std::shared_ptr<GG::Button>         m_next_button = nullptr;
    std::shared_ptr<GG::Edit>           m_search_edit = nullptr;            // box to type to search
    std::shared_ptr<GraphControl>       m_graph = nullptr;
    bool                                m_needs_refresh = false;            // Indicates that data is stale.
};


#endif
