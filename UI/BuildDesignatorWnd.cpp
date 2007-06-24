#include "BuildDesignatorWnd.h"

#include "CUIControls.h"
#include "ClientUI.h"
#include "CUISpin.h"
#include "CUIWnd.h"
#include "SidePanel.h"
#include "TechTreeWnd.h"
#include "MapWnd.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/UniverseObject.h"
#include "../Empire/Empire.h"
#include "../universe/Building.h"
#include "../universe/ShipDesign.h"
#include "../universe/Effect.h"

#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>

#include <boost/format.hpp>

namespace {
    const int DEFENSE_BASE_BUILD_TURNS = 10; // this is a kludge for v0.3 only
    const int DEFENSE_BASE_BUILD_COST = 20; // this is a kludge for v0.3 only

    class BuildableItemsListBox : public CUIListBox
    {
    public:
        BuildableItemsListBox(int x, int y, int w, int h) :
            CUIListBox(x, y, w, h)
        {}
        virtual void GainingFocus() {DeselectAll();}
    };

    struct ToggleBuildTypeFunctor
    {
        ToggleBuildTypeFunctor(BuildDesignatorWnd* designator_wnd, BuildType type) : m_designator_wnd(designator_wnd), m_build_type(type) {}
        void operator()() {m_designator_wnd->ToggleType(m_build_type);}
        BuildDesignatorWnd* const m_designator_wnd;
        const BuildType m_build_type;
    };

    struct ToggleAllBuildTypesFunctor
    {
        ToggleAllBuildTypesFunctor(BuildDesignatorWnd* designator_wnd) : m_designator_wnd(designator_wnd) {}
        void operator()() {m_designator_wnd->ToggleAllTypes();}
        BuildDesignatorWnd* const m_designator_wnd;
    };

    struct ToggleAvailabilityFunctor
    {
        ToggleAvailabilityFunctor(BuildDesignatorWnd* designator_wnd, bool available) : m_designator_wnd(designator_wnd), m_available(available) {}
        void operator()() {m_designator_wnd->ToggleAvailabilitly(m_available);}
        BuildDesignatorWnd* const m_designator_wnd;
        const bool m_available; // true: toggle whether to show available techs; false: toggle whether to show unavailable techs
    };
}

//////////////////////////////////////////////////
// BuildDesignatorWnd::BuildDetailPanel
//////////////////////////////////////////////////
class BuildDesignatorWnd::BuildDetailPanel : public GG::Wnd
{
public:
    BuildDetailPanel(int w, int h);
    ~BuildDetailPanel();
    int QueueIndexShown() const;
    virtual void Render();
    void SelectedBuildLocation(int location);
    void SetBuildItem(BuildType build_type, const std::string& item = "");
    void SetBuildItem(BuildType build_type, int design_id);
    void SetBuild(int queue_idx);
    void Reset();
    void Clear();
    mutable boost::signal<void (int)> CenterOnBuildSignal;
    mutable boost::signal<void (BuildType, const std::string&, int)> RequestNamedBuildItemSignal;
    mutable boost::signal<void (BuildType, int, int)> RequestIDedBuildItemSignal;
    mutable boost::signal<void (int, int)> BuildQuantityChangedSignal;

private:
    GG::Pt ItemGraphicUpperLeft() const;
    bool DisplayingQueueItem() const;
    void CenterClickedSlot();
    void AddToQueueClickedSlot();
    void ItemsToBuildChangedSlot(int value);
    void CheckBuildability();                   ///< Enables / Disables UI widgets according to whether the currently selected build item can be built, and whether multiple copies of it can be built
    void ConfigureForQueueItemView();
    void ConfigureForNewBuildView();

    BuildType           m_build_type;
    std::string         m_item_name;
    int                 m_item_design_id;
    int                 m_queue_idx;
    int                 m_build_location;
    GG::TextControl*    m_build_location_name_text;
    GG::TextControl*    m_item_name_text;
    GG::TextControl*    m_cost_text;
    CUIButton*          m_recenter_button;
    CUIButton*          m_add_to_queue_button;
    GG::TextControl*    m_num_items_to_build_label;
    CUISpin<int>*       m_num_items_to_build;
    CUIMultiEdit*       m_description_box;
    GG::StaticGraphic*  m_item_graphic;

    std::set<boost::signals::connection> m_misc_connections;
};

BuildDesignatorWnd::BuildDetailPanel::BuildDetailPanel(int w, int h) :
    Wnd(0, 0, w, h, GG::CLICKABLE),
    m_build_type(INVALID_BUILD_TYPE),
    m_item_name(""),
    m_queue_idx(-1),
    m_build_location(UniverseObject::INVALID_OBJECT_ID)
{
    const int NAME_PTS = ClientUI::Pts() + 8;
    const int COST_PTS = ClientUI::Pts();
    const int BUTTON_WIDTH = 150;
    boost::shared_ptr<GG::Font> default_font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());
    m_item_name_text = new GG::TextControl(1, 0, w - 1 - BUTTON_WIDTH, NAME_PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::FontBold(), NAME_PTS), ClientUI::TextColor());
    m_cost_text = new GG::TextControl(1, m_item_name_text->LowerRight().y, w - 1 - BUTTON_WIDTH, COST_PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), COST_PTS), ClientUI::TextColor());
    m_add_to_queue_button = new CUIButton(w - 1 - BUTTON_WIDTH, 1, BUTTON_WIDTH, UserString("PRODUCTION_DETAIL_ADD_TO_QUEUE"));
    m_recenter_button = new CUIButton(w - 1 - BUTTON_WIDTH, 1, BUTTON_WIDTH, UserString("PRODUCTION_DETAIL_CENTER_ON_BUILD"));
    m_recenter_button->Disable();
    m_add_to_queue_button->Disable();
    m_num_items_to_build = new CUISpin<int>(0, 0, 75, 1, 1, 1, 1000, true);
    m_num_items_to_build->MoveTo(GG::Pt(1, h - m_num_items_to_build->Height() - 1));
    m_num_items_to_build_label = new GG::TextControl(m_num_items_to_build->LowerRight().x + 3, m_num_items_to_build->UpperLeft().y + (m_num_items_to_build->Height() - (NAME_PTS + 4)) / 2, w - 1 - BUTTON_WIDTH - 3 - (m_num_items_to_build->LowerRight().x + 3), NAME_PTS + 4,
                                                     UserString("PRODUCTION_DETAIL_NUMBER_TO_BUILD"), default_font, ClientUI::TextColor(), GG::TF_LEFT);
    m_build_location_name_text = new GG::TextControl(w - 1 - BUTTON_WIDTH, m_num_items_to_build->UpperLeft().y + (m_num_items_to_build->Height() - (NAME_PTS + 4)) / 2, BUTTON_WIDTH, ClientUI::Pts() + 4,
                                                     "", default_font, ClientUI::TextColor());
    m_description_box = new CUIMultiEdit(1, m_cost_text->LowerRight().y, w - 2 - BUTTON_WIDTH, m_num_items_to_build_label->UpperLeft().y - 2 - m_cost_text->LowerRight().y, "", GG::TF_WORDBREAK | GG::MultiEdit::READ_ONLY);
    m_description_box->SetColor(GG::CLR_ZERO);
    m_description_box->SetInteriorColor(GG::CLR_ZERO);

    m_item_graphic = 0;

    m_misc_connections.insert(GG::Connect(m_recenter_button->ClickedSignal, &BuildDesignatorWnd::BuildDetailPanel::CenterClickedSlot, this));
    m_misc_connections.insert(GG::Connect(m_add_to_queue_button->ClickedSignal, &BuildDesignatorWnd::BuildDetailPanel::AddToQueueClickedSlot, this));
    m_misc_connections.insert(GG::Connect(m_num_items_to_build->ValueChangedSignal, &BuildDesignatorWnd::BuildDetailPanel::ItemsToBuildChangedSlot, this));

    AttachChild(m_item_name_text);
    AttachChild(m_cost_text);
    AttachChild(m_recenter_button);
    AttachChild(m_add_to_queue_button);
    AttachChild(m_num_items_to_build);
    AttachChild(m_num_items_to_build_label);
    AttachChild(m_build_location_name_text);
    AttachChild(m_description_box);
}

BuildDesignatorWnd::BuildDetailPanel::~BuildDetailPanel()
{
    // disconnect all signals
    while (!m_misc_connections.empty()) {
        m_misc_connections.begin()->disconnect();
        m_misc_connections.erase(m_misc_connections.begin());
    }
}

int BuildDesignatorWnd::BuildDetailPanel::QueueIndexShown() const
{
    return m_queue_idx;
}

void BuildDesignatorWnd::BuildDetailPanel::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::WndColor(), GG::CLR_ZERO, 0);
}

void BuildDesignatorWnd::BuildDetailPanel::SelectedBuildLocation(int location)
{
    m_build_location = location;
    CheckBuildability();
}

void BuildDesignatorWnd::BuildDetailPanel::SetBuildItem(BuildType build_type, const std::string& item)
{
    if (build_type != BT_BUILDING && build_type != BT_ORBITAL && build_type != INVALID_BUILD_TYPE)
        throw std::invalid_argument("Attempted to SetBuildItem with a name that wasn't a BT_BUILDING or BT_ORBITAL");
    m_build_type = build_type;
    m_item_name = item;
    m_item_design_id = UniverseObject::INVALID_OBJECT_ID;
    m_queue_idx = -1;
    Reset();
}

void BuildDesignatorWnd::BuildDetailPanel::SetBuildItem(BuildType build_type, int design_id)
{
    if (build_type != BT_SHIP)
        throw std::invalid_argument("Attempted to SetBuildItem with a design id that wasn't a BT_SHIP");
    m_build_type = build_type;
    m_item_name = "";
    m_item_design_id = design_id;
    m_queue_idx = -1;
    Reset();
}

void BuildDesignatorWnd::BuildDetailPanel::SetBuild(int queue_idx)
{
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ProductionQueue& queue = empire->GetProductionQueue();
    if (0 <= queue_idx && queue_idx < static_cast<int>(queue.size())) {
        m_build_type = queue[queue_idx].item.build_type;
        m_item_name = queue[queue_idx].item.name;
        m_item_design_id = queue[queue_idx].item.design_id;
        m_queue_idx = queue_idx;
    } else {
        m_build_type = INVALID_BUILD_TYPE;
        m_item_name = "";
        m_item_design_id = UniverseObject::INVALID_OBJECT_ID;
        m_queue_idx = -1;
    }
    Reset();
}

void BuildDesignatorWnd::BuildDetailPanel::Reset()
{
    m_item_name_text->SetText("");
    m_cost_text->SetText("");
    m_build_location_name_text->SetText("");
    m_description_box->SetText("");

    if (m_item_graphic) {
        DeleteChild(m_item_graphic);
        m_item_graphic = 0;
    }

    if (m_build_type == INVALID_BUILD_TYPE) {
        DetachChild(m_recenter_button);
        DetachChild(m_add_to_queue_button);
        DetachChild(m_num_items_to_build);
        DetachChild(m_build_location_name_text);
        DetachChild(m_num_items_to_build_label);
        return;
    }

    m_recenter_button->Show();
    m_add_to_queue_button->Show();
    m_num_items_to_build->Show();
    m_num_items_to_build_label->Show();
    m_build_location_name_text->Show();
    m_recenter_button->Disable(!DisplayingQueueItem());
    m_num_items_to_build->SetValue(1);

    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return;

    const ProductionQueue& queue = empire->GetProductionQueue();
    if (static_cast<int>(queue.size()) <= m_queue_idx) {
        if (!queue.empty()) {
            m_queue_idx = queue.size() - 1;
            m_build_type = queue[m_queue_idx].item.build_type;
            m_item_name = queue[m_queue_idx].item.name;
            m_item_design_id = queue[m_queue_idx].item.design_id;
        } else {
            m_build_type = INVALID_BUILD_TYPE;
            m_item_name = "";
            m_item_design_id = UniverseObject::INVALID_OBJECT_ID;
            m_queue_idx = -1;
        }
    }

    if (DisplayingQueueItem()) {
        ConfigureForQueueItemView();
        m_num_items_to_build->SetValue(queue[m_queue_idx].remaining);
        m_build_location_name_text->SetText(GetUniverse().Object(queue[m_queue_idx].location)->Name());
    } else {
        ConfigureForNewBuildView();
    }

    CheckBuildability();

    using boost::io::str;
    using boost::format;
    double cost_per_turn = 0;
    int turns = 0;
    std::string item_name_str = UserString(m_item_name);
    std::string description_str;
    boost::shared_ptr<GG::Texture> graphic;
    if (m_build_type == BT_BUILDING) {
        assert(empire);
        const BuildingType* building_type = GetBuildingType(m_item_name);
        assert(building_type);
        turns = building_type->BuildTime();
        boost::tie(cost_per_turn, turns) = empire->ProductionCostAndTime(BT_BUILDING, m_item_name);
        if (building_type->Effects().empty()) {
            description_str = str(format(UserString("PRODUCTION_DETAIL_BUILDING_DESCRIPTION_STR"))
                                  % UserString(building_type->Description()));
        } else {
            description_str = str(format(UserString("PRODUCTION_DETAIL_BUILDING_DESCRIPTION_STR_WITH_EFFECTS"))
                                  % UserString(building_type->Description())
                                  % EffectsDescription(building_type->Effects()));
        }
        if (!building_type->Graphic().empty())
            graphic = ClientUI::GetTexture(ClientUI::ArtDir() / building_type->Graphic());
    } else if (m_build_type == BT_SHIP) {
        assert(empire);
        const ShipDesign* design = GetShipDesign(m_item_design_id);
        assert(design);
        turns = 5; // this is a kludge for v0.3 only
        boost::tie(cost_per_turn, turns) = empire->ProductionCostAndTime(BT_SHIP, m_item_design_id);
        item_name_str = design->name;
        description_str = str(format(UserString("PRODUCTION_DETAIL_SHIP_DESCRIPTION_STR"))
                              % design->description
                              % design->attack
                              % design->defense
                              % design->speed);
        graphic = ClientUI::GetTexture(ClientUI::ArtDir() / design->graphic);
    } else if (m_build_type == BT_ORBITAL) {
        turns = DEFENSE_BASE_BUILD_TURNS;
        cost_per_turn = DEFENSE_BASE_BUILD_COST;
        item_name_str = UserString("DEFENSE_BASE");
        description_str = UserString("DEFENSE_BASE_DESCRIPTION");
        graphic = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "base1.png"); // this is a kludge for v0.3 only
    }

    if (graphic) {
        GG::Pt ul = ItemGraphicUpperLeft();
        m_item_graphic = new GG::StaticGraphic(ul.x, ul.y, 128, 128, graphic, GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
        m_item_graphic->Show();
        AttachChild(m_item_graphic);
    }

    m_item_name_text->SetText(item_name_str);
    m_cost_text->SetText(str(format(UserString("PRODUCTION_TOTAL_COST_STR"))
                             % static_cast<int>(cost_per_turn + 0.5)
                             % turns));
    m_description_box->SetText(description_str);
}

void BuildDesignatorWnd::BuildDetailPanel::Clear()
{
    SetBuildItem(INVALID_BUILD_TYPE, "");
}

GG::Pt BuildDesignatorWnd::BuildDetailPanel::ItemGraphicUpperLeft() const
{
    return GG::Pt(Width() - 2 - 150 + (150 - 128) / 2, 1 + m_recenter_button->Height() + 5);
}

bool BuildDesignatorWnd::BuildDetailPanel::DisplayingQueueItem() const
{
    return m_queue_idx != -1;
}

void BuildDesignatorWnd::BuildDetailPanel::CenterClickedSlot()
{
    if (m_build_type != INVALID_BUILD_TYPE && DisplayingQueueItem())
        CenterOnBuildSignal(m_queue_idx);
}

void BuildDesignatorWnd::BuildDetailPanel::AddToQueueClickedSlot()
{
    if (m_build_type == BT_BUILDING || m_build_type == BT_ORBITAL)
        RequestNamedBuildItemSignal(m_build_type, m_item_name, m_num_items_to_build->Value());
    else if (m_build_type == BT_SHIP)
        RequestIDedBuildItemSignal(m_build_type, m_item_design_id, m_num_items_to_build->Value());
}

void BuildDesignatorWnd::BuildDetailPanel::ItemsToBuildChangedSlot(int value)
{
    if (DisplayingQueueItem())
        BuildQuantityChangedSignal(m_queue_idx, value);
}

void BuildDesignatorWnd::BuildDetailPanel::CheckBuildability()
{
    // disable widgets by default...
    m_add_to_queue_button->Disable();
    m_num_items_to_build->Disable();
    m_num_items_to_build_label->Disable();


    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (!empire) return;
    
    if (m_build_type == BT_SHIP) {
        if (!empire->BuildableItem(m_build_type, m_item_design_id, m_build_location)) return;
        m_add_to_queue_button->Disable(false);
        m_num_items_to_build->Disable(false);
        m_num_items_to_build_label->Disable(false);

    } else if (m_build_type == BT_ORBITAL) {
        if (!empire->BuildableItem(m_build_type, m_item_name, m_build_location)) return;
        m_add_to_queue_button->Disable(false);
        m_num_items_to_build->Disable(false);
        m_num_items_to_build_label->Disable(false);

    } else if (m_build_type == BT_BUILDING) {
        if (!empire->BuildableItem(m_build_type, m_item_name, m_build_location)) return;
        m_add_to_queue_button->Disable(false);
    }
}

void BuildDesignatorWnd::BuildDetailPanel::ConfigureForQueueItemView()
{
    DetachChild(m_add_to_queue_button);
    AttachChild(m_recenter_button);
    AttachChild(m_num_items_to_build);
    AttachChild(m_num_items_to_build_label);
    AttachChild(m_build_location_name_text);
}

void BuildDesignatorWnd::BuildDetailPanel::ConfigureForNewBuildView()
{
    AttachChild(m_add_to_queue_button);
    DetachChild(m_recenter_button);
    AttachChild(m_num_items_to_build);
    AttachChild(m_num_items_to_build_label);
    DetachChild(m_build_location_name_text);
}


//////////////////////////////////////////////////
// BuildDesignatorWnd::BuildSelector
//////////////////////////////////////////////////
class BuildDesignatorWnd::BuildSelector : public CUIWnd
{
public:
    BuildSelector(int w, int h);
    ~BuildSelector();

    const std::set<BuildType>&      GetBuildTypesShown() const;
    const std::pair<bool, bool>&    GetAvailabilitiesShown() const; // .first -> available items; .second -> unavailable items

    virtual void                    MinimizeClicked();

    void SetBuildLocation(int location_id);

    void Reset(bool keep_selection);

    void ShowType(BuildType type, bool refresh_list = true);
    void ShowAllTypes(bool refresh_list = true);
    void HideType(BuildType type, bool refresh_list = true);
    void HideAllTypes(bool refresh_list = true);
    
    void ShowAvailability(bool available, bool refresh_list = true);
    void HideAvailability(bool available, bool refresh_list = true);

    mutable boost::signal<void (BuildType, const std::string&)>         DisplayNamedBuildItemSignal;
    mutable boost::signal<void (BuildType, const std::string&, int)>    RequestNamedBuildItemSignal;
    mutable boost::signal<void (BuildType, int)>                        DisplayIDedBuildItemSignal;
    mutable boost::signal<void (BuildType, int, int)>                   RequestIDedBuildItemSignal;

private:
    void DoLayout();

    bool BuildableItemVisible(BuildType build_type, const std::string& name);
    bool BuildableItemVisible(BuildType build_type, int design_id);

    void PopulateList(bool keep_selection);
    
    void BuildItemSelected(const std::set<int>& selections);
    void BuildItemDoubleClicked(int row_index, GG::ListBox::Row* row);

    std::map<BuildType, CUIButton*>         m_build_type_buttons;
    std::vector<CUIButton*>                 m_availability_buttons;

    std::set<BuildType>                     m_build_types_shown;
    std::pair<bool, bool>                   m_availabilities_shown; // .first -> available items; .second -> unavailable items
    
    BuildableItemsListBox*                  m_buildable_items;
    std::map<GG::ListBox::Row*, BuildType>  m_build_types;
    GG::Pt                                  m_original_ul;

    std::set<boost::signals::connection>    m_misc_connections;

    int m_build_location;

    int row_height;
    int icon_col_width;
    int name_col_width;
    int cost_col_width;
    int time_col_width;
    int desc_col_width;

    friend class BuildDesignatorWnd;        // so BuildDesignatorWnd can access buttons
};

BuildDesignatorWnd::BuildSelector::BuildSelector(int w, int h) :
    CUIWnd(UserString("PRODUCTION_WND_BUILD_ITEMS_TITLE"), 0, 0, w, h, GG::CLICKABLE | CUIWnd::MINIMIZABLE),
    m_build_location(UniverseObject::INVALID_OBJECT_ID)
{
    // create build type toggle buttons (ship, building, orbital, all)
    m_build_type_buttons[BT_BUILDING] = new CUIButton(0, 0, 1, UserString("PRODUCTION_WND_CATEGORY_BT_BUILDING"));
    AttachChild(m_build_type_buttons[BT_BUILDING]);
    m_build_type_buttons[BT_SHIP] = new CUIButton(0, 0, 1, UserString("PRODUCTION_WND_CATEGORY_BT_SHIP"));
    AttachChild(m_build_type_buttons[BT_SHIP]);
    m_build_type_buttons[BT_ORBITAL] = new CUIButton(0, 0, 1, UserString("PRODUCTION_WND_CATEGORY_BT_ORBITAL"));
    AttachChild(m_build_type_buttons[BT_ORBITAL]);
    m_build_type_buttons[NUM_BUILD_TYPES] = new CUIButton(0, 0, 1, UserString("ALL"));
    AttachChild(m_build_type_buttons[NUM_BUILD_TYPES]);

    // create availability toggle buttons (available, not available)
    m_availability_buttons.push_back(new CUIButton(0, 0, 1, UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE")));
    AttachChild(m_availability_buttons.back());
    m_availability_buttons.push_back(new CUIButton(0, 0, 1, UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE")));
    AttachChild(m_availability_buttons.back());

    // selectable list of buildable items
    m_buildable_items = new BuildableItemsListBox(0, 0, 1, 1);
    AttachChild(m_buildable_items);
    m_misc_connections.insert(GG::Connect(m_buildable_items->SelChangedSignal, &BuildDesignatorWnd::BuildSelector::BuildItemSelected, this));
    m_misc_connections.insert(GG::Connect(m_buildable_items->DoubleClickedSignal, &BuildDesignatorWnd::BuildSelector::BuildItemDoubleClicked, this));
    m_buildable_items->SetStyle(GG::LB_NOSORT | GG::LB_SINGLESEL);

    DoLayout();
}

BuildDesignatorWnd::BuildSelector::~BuildSelector()
{
    // disconnect all signals
    while (!m_misc_connections.empty()) {
        m_misc_connections.begin()->disconnect();
        m_misc_connections.erase(m_misc_connections.begin());
    }
}

const std::set<BuildType>& BuildDesignatorWnd::BuildSelector::GetBuildTypesShown() const
{
   return m_build_types_shown;
}

const std::pair<bool, bool>& BuildDesignatorWnd::BuildSelector::GetAvailabilitiesShown() const
{
    return m_availabilities_shown;
}

void BuildDesignatorWnd::BuildSelector::DoLayout()
{
    GG::Pt client_size = ClientSize();

    int x = 0, button_width = 60, button_height = 20;
    for (unsigned int i = 0; i < m_build_type_buttons.size() - 1; ++i) {
        m_build_type_buttons[BuildType(BT_BUILDING + i)]->SizeMove(GG::Pt(x, 0), GG::Pt(x + button_width, button_height));
        x += button_width;
    }
    m_build_type_buttons[NUM_BUILD_TYPES]->SizeMove(GG::Pt(x, 0), GG::Pt(x + button_width, button_height)); x += button_width;

    m_availability_buttons[0]->SizeMove(GG::Pt(x, 0), GG::Pt(x + button_width, button_height)); x += button_width;
    m_availability_buttons[1]->SizeMove(GG::Pt(x, 0), GG::Pt(x + button_width, button_height));

    m_buildable_items->SizeMove(GG::Pt(0, button_height), client_size);

    row_height = ClientUI::Pts()*3/2;
    icon_col_width = row_height;
    name_col_width = ClientUI::Pts()*18;
    cost_col_width = ClientUI::Pts()*3;
    time_col_width = ClientUI::Pts()*2;
    desc_col_width = m_buildable_items->ClientWidth() 
                     - (icon_col_width + name_col_width + cost_col_width + time_col_width)
                     - ClientUI::ScrollWidth();

    m_buildable_items->SetNumCols(5);
    m_buildable_items->LockColWidths();
    m_buildable_items->SetColWidth(0, icon_col_width);
    m_buildable_items->SetColWidth(1, name_col_width);
    m_buildable_items->SetColWidth(2, cost_col_width);
    m_buildable_items->SetColWidth(3, time_col_width);
    m_buildable_items->SetColWidth(4, desc_col_width);
    m_buildable_items->SetColAlignment(0, GG::ALIGN_LEFT);
    m_buildable_items->SetColAlignment(1, GG::ALIGN_LEFT);
    m_buildable_items->SetColAlignment(2, GG::ALIGN_LEFT);
    m_buildable_items->SetColAlignment(3, GG::ALIGN_LEFT);
    m_buildable_items->SetColAlignment(4, GG::ALIGN_LEFT);
}

void BuildDesignatorWnd::BuildSelector::MinimizeClicked()
{
    if (!m_minimized) {
        m_minimized = true;
        m_original_size = Size();
        m_original_ul = RelativeUpperLeft();
        GG::Pt original_lr = m_original_ul + m_original_size;
        GG::Pt new_size(Width(), BORDER_TOP);
        SetMinSize(GG::Pt(new_size.x, new_size.y));
        SizeMove(original_lr - new_size, original_lr);
        GG::Pt button_ul = GG::Pt(Width() - BUTTON_RIGHT_OFFSET, BUTTON_TOP_OFFSET);
        if (m_close_button)
            m_close_button->MoveTo(GG::Pt(button_ul.x, button_ul.y));
        if (m_minimize_button)
            m_minimize_button->MoveTo(GG::Pt(button_ul.x - (m_close_button ? BUTTON_RIGHT_OFFSET : 0), button_ul.y));
        Hide();
        Show(false);
        if (m_close_button)
            m_close_button->Show();
        if (m_minimize_button)
            m_minimize_button->Show();
    } else {
        m_minimized = false;
        SetMinSize(GG::Pt(Width(), BORDER_TOP + INNER_BORDER_ANGLE_OFFSET + BORDER_BOTTOM));
        SizeMove(m_original_ul, m_original_ul + m_original_size);
        GG::Pt button_ul = GG::Pt(Width() - BUTTON_RIGHT_OFFSET, BUTTON_TOP_OFFSET) + UpperLeft() - ClientUpperLeft();
        if (m_close_button)
            m_close_button->MoveTo(GG::Pt(button_ul.x, button_ul.y));
        if (m_minimize_button)
            m_minimize_button->MoveTo(GG::Pt(button_ul.x - (m_close_button ? BUTTON_RIGHT_OFFSET : 0), button_ul.y));
        Show();
    }
}

void BuildDesignatorWnd::BuildSelector::SetBuildLocation(int location_id)
{
    m_build_location = location_id;
    PopulateList(true);
}

void BuildDesignatorWnd::BuildSelector::Reset(bool keep_selection)
{
    PopulateList(keep_selection);
    DoLayout();
}

void BuildDesignatorWnd::BuildSelector::ShowType(BuildType type, bool refresh_list)
{
    if (m_build_types_shown.find(type) == m_build_types_shown.end()) {
        m_build_types_shown.insert(type);
        if (refresh_list) PopulateList(true);
    }
}

void BuildDesignatorWnd::BuildSelector::HideType(BuildType type, bool refresh_list)
{
    std::set<BuildType>::iterator it = m_build_types_shown.find(type);
    if (it != m_build_types_shown.end()) {
        m_build_types_shown.erase(it);
        if (refresh_list) PopulateList(true);
    }
}

void BuildDesignatorWnd::BuildSelector::ShowAllTypes(bool refresh_list)
{
    m_build_types_shown.insert(BT_BUILDING);
    m_build_types_shown.insert(BT_SHIP);
    m_build_types_shown.insert(BT_ORBITAL);
    if (refresh_list) PopulateList(true);
}

void BuildDesignatorWnd::BuildSelector::HideAllTypes(bool refresh_list)
{
    m_build_types_shown.clear();
    if (refresh_list) PopulateList(false);
}

void BuildDesignatorWnd::BuildSelector::ShowAvailability(bool available, bool refresh_list)
{
    if (available) {
        if (!m_availabilities_shown.first) {
            m_availabilities_shown.first = true;
            if (refresh_list) PopulateList(true);
        }
    } else {
        if (!m_availabilities_shown.second) {
            m_availabilities_shown.second = true;
            if (refresh_list) PopulateList(true);
        }
    }
}

void BuildDesignatorWnd::BuildSelector::HideAvailability(bool available, bool refresh_list)
{
    if (available) {
        if (m_availabilities_shown.first) {
            m_availabilities_shown.first = false;
            if (refresh_list) PopulateList(true);
        }
    } else {
        if (m_availabilities_shown.second) {
            m_availabilities_shown.second = false;
            if (refresh_list) PopulateList(true);
        }
    }
}

bool BuildDesignatorWnd::BuildSelector::BuildableItemVisible(BuildType build_type, const std::string& name)
{
    if (build_type != BT_BUILDING && build_type != BT_ORBITAL)
        throw std::invalid_argument("BuildableItemVisible was passed an invalid build type with a name");

    if (m_build_types_shown.find(build_type) == m_build_types_shown.end())
        return false;

    bool available = false;

    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (build_type == BT_BUILDING)
        available = empire->BuildingTypeAvailable(name);
    else if (build_type == BT_ORBITAL)
        available = true;

    if (available)
        return m_availabilities_shown.first;
    else
        return m_availabilities_shown.second;
}

bool BuildDesignatorWnd::BuildSelector::BuildableItemVisible(BuildType build_type, int design_id)
{
    if (build_type != BT_SHIP)
        throw std::invalid_argument("BuildableItemVisible was passed an invalid build type with an id");

    if (m_build_types_shown.find(build_type) == m_build_types_shown.end())
        return false;

    bool available = false;

    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (build_type == BT_SHIP)
        available = empire->ShipDesignAvailable(design_id);

    if (available)
        return m_availabilities_shown.first;
    else
        return m_availabilities_shown.second;
}

void BuildDesignatorWnd::BuildSelector::PopulateList(bool keep_selection)
{
    if (!Visible()) return;

    Logger().debugStream() << "PopulateList start";
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (!empire) return;
    
    // keep track of initially selected row, so that new rows added may be compared to it to see if they should be selected after repopulating
    std::string selected_row;
    if (m_buildable_items->Selections().size() == 1) {
        selected_row = m_buildable_items->GetRow(*m_buildable_items->Selections().begin()).DragDropDataType();
    }


    m_buildable_items->Clear(); // the list of items to be populated
    m_build_types.clear();      // map from row to BuildType


    boost::shared_ptr<GG::Font> default_font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());

    int row_to_select = -1; // may be set while populating - used to reselect previously selected row after populating
    int i = 0;              // counter that keeps track of how many rows have been added so far

    // populate list with building types
    Logger().debugStream() << "Adding Buildings";
    if (m_build_types_shown.find(BT_BUILDING) != m_build_types_shown.end()) {
        BuildingTypeManager& manager = GetBuildingTypeManager();

        for (BuildingTypeManager::iterator it = manager.begin(); it != manager.end(); ++it, ++i) {
            const BuildingType* type = it->second;
            const std::string name = it->first;

            if (!BuildableItemVisible(BT_BUILDING, name)) continue;

            GG::ListBox::Row* row = new GG::ListBox::Row();
            row->SetDragDropDataType(name);

            // icon
            GG::Control* icon = new GG::StaticGraphic(0, 0, icon_col_width, row_height, 
                ClientUI::GetTexture(ClientUI::ArtDir() / type->Graphic()),
                GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
            row->push_back(icon);

            // building name
            row->push_back(UserString(name), default_font, ClientUI::TextColor());

            // cost / turn, and minimum production turns
            const std::pair<double, int> cost_time = empire->ProductionCostAndTime(BT_BUILDING, name);
            std::string cost_text = boost::lexical_cast<std::string>(cost_time.first);
            row->push_back(cost_text, default_font, ClientUI::TextColor());
            std::string time_text = boost::lexical_cast<std::string>(cost_time.second);
            row->push_back(time_text, default_font, ClientUI::TextColor());

            // brief description
            std::string desc_text = UserString("BT_BUILDING");
            GG::Control* desc_control = new GG::TextControl(0, 0, desc_col_width, row_height, desc_text, default_font, ClientUI::TextColor(), GG::TF_LEFT); ///< ctor taking a font directly
            row->push_back(desc_control);

            // is item buildable?  If not, disable row
            if (!empire->BuildableItem(BT_BUILDING, name, m_build_location)) {
                row->Disable(true);
            } else {
                row->Disable(false);
            }

            m_buildable_items->Insert(row);
            m_build_types[row] = BT_BUILDING;
            if (row->DragDropDataType() == selected_row)
                row_to_select = i;
        }
    }
    // populate with ship designs
    Logger().debugStream() << "Adding ship designs";
    if (m_build_types_shown.find(BT_SHIP) != m_build_types_shown.end()) {
        Empire::ShipDesignItr end_it = empire->ShipDesignEnd();
        for (Empire::ShipDesignItr it = empire->ShipDesignBegin(); it != end_it; ++it, ++i) {
            const int ship_design_id = *it;

            if (!BuildableItemVisible(BT_SHIP, ship_design_id)) continue;

            const ShipDesign* ship_design = GetShipDesign(ship_design_id);
            if (!ship_design) continue;

            GG::ListBox::Row* row = new GG::ListBox::Row();
            row->SetDragDropDataType(boost::lexical_cast<std::string>(ship_design_id));

            // icon
            GG::StaticGraphic* icon = new GG::StaticGraphic(0, 0, icon_col_width, row_height, 
                ClientUI::GetTexture(ClientUI::ArtDir() / ship_design->graphic),
                GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
            row->push_back(dynamic_cast<GG::Control*>(icon));

            // ship design name
            row->push_back(ship_design->name, default_font, ClientUI::TextColor());

            // cost / turn, and minimum production turns
            const std::pair<double, int> cost_time = empire->ProductionCostAndTime(BT_SHIP, ship_design_id);
            std::string cost_text = boost::lexical_cast<std::string>(cost_time.first);
            row->push_back(cost_text, default_font, ClientUI::TextColor());
            std::string time_text = boost::lexical_cast<std::string>(cost_time.second);
            row->push_back(time_text, default_font, ClientUI::TextColor());

            // brief description            
            std::string desc_text = UserString("BT_SHIP");
            GG::Control* desc_control = new GG::TextControl(0, 0, desc_col_width, row_height, desc_text, default_font, ClientUI::TextColor(), GG::TF_LEFT); ///< ctor taking a font directly
            row->push_back(desc_control);

            // is item buildable?  If not, disable row
            if (!empire->BuildableItem(BT_SHIP, ship_design_id, m_build_location)) {
                row->Disable(true);
            } else {
                row->Disable(false);
            }

            m_buildable_items->Insert(row);
            m_build_types[row] = BT_SHIP;
            if (row->DragDropDataType() == selected_row)
                row_to_select = i;
        }
    }
    // populate with orbitals
    Logger().debugStream() << "Adding Orbitals";
    if (m_build_types_shown.find(BT_ORBITAL) != m_build_types_shown.end()) {
        if (BuildableItemVisible(BT_ORBITAL, "")) {
            GG::ListBox::Row* row = new GG::ListBox::Row();
            row->SetDragDropDataType("DEFENSE_BASE");

            // icon
            GG::StaticGraphic* icon = new GG::StaticGraphic(0, 0, icon_col_width, row_height, 
                ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "defensebase.png"),
                GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
            row->push_back(dynamic_cast<GG::Control*>(icon));

            // Defense Base "name"
            row->push_back(UserString(row->DragDropDataType()), default_font, ClientUI::TextColor());

            // cost / turn, and minimum production turns
            const std::pair<double, int> cost_time = empire->ProductionCostAndTime(BT_ORBITAL, UserString(row->DragDropDataType()));
            std::string cost_text = boost::lexical_cast<std::string>(cost_time.first);
            row->push_back(cost_text, default_font, ClientUI::TextColor());
            std::string time_text = boost::lexical_cast<std::string>(cost_time.second);
            row->push_back(time_text, default_font, ClientUI::TextColor());

            // brief description            
            std::string desc_text = UserString("BT_ORBITAL");
            GG::Control* desc_control = new GG::TextControl(0, 0, desc_col_width, row_height, desc_text, default_font, ClientUI::TextColor(), GG::TF_LEFT); ///< ctor taking a font directly
            row->push_back(desc_control);

            // is item buildable?  If not, disable row
            if (!empire->BuildableItem(BT_ORBITAL, "", m_build_location)) {
                row->Disable(true);
            } else {
                row->Disable(false);
            }

            m_buildable_items->Insert(row);
            m_build_types[row] = BT_ORBITAL;
            if (row->DragDropDataType() == selected_row)
                row_to_select = i;
        }
    }

    Logger().debugStream() << "Selecting Row";
    if (row_to_select != -1)
        m_buildable_items->SelectRow(row_to_select);
    Logger().errorStream() << "Done";
}

void BuildDesignatorWnd::BuildSelector::BuildItemSelected(const std::set<int>& selections)
{
    if (selections.size() == 1) {
        GG::ListBox::Row* row = &m_buildable_items->GetRow(*selections.begin());
        BuildType build_type = m_build_types[row];
        if (build_type == BT_BUILDING || build_type == BT_ORBITAL)
            DisplayNamedBuildItemSignal(m_build_types[row], row->DragDropDataType());
        else if (build_type == BT_SHIP)
            DisplayIDedBuildItemSignal(m_build_types[row], boost::lexical_cast<int>(row->DragDropDataType()));
    }
}

void BuildDesignatorWnd::BuildSelector::BuildItemDoubleClicked(int row_index, GG::ListBox::Row* row)
{
    if (row->Disabled()) return;
    BuildType build_type = m_build_types[row];
    if (build_type == BT_BUILDING || build_type == BT_ORBITAL)
        RequestNamedBuildItemSignal(build_type, row->DragDropDataType(), 1);
    else if (build_type == BT_SHIP)
        RequestIDedBuildItemSignal(build_type, boost::lexical_cast<int>(row->DragDropDataType()), 1);
}


//////////////////////////////////////////////////
// BuildDesignatorWnd
//////////////////////////////////////////////////
BuildDesignatorWnd::BuildDesignatorWnd(int w, int h) :
    Wnd(0, 0, w, h, GG::CLICKABLE),
    m_build_location(UniverseObject::INVALID_OBJECT_ID)
{
    int CHILD_WIDTHS = w - MapWnd::SIDE_PANEL_WIDTH;
    const int DETAIL_PANEL_HEIGHT = TechTreeWnd::NAVIGATOR_AND_DETAIL_HEIGHT;
    const int BUILD_SELECTOR_HEIGHT = DETAIL_PANEL_HEIGHT;

    m_build_detail_panel = new BuildDetailPanel(CHILD_WIDTHS, DETAIL_PANEL_HEIGHT);

    m_side_panel = new SidePanel(Width() - MapWnd::SIDE_PANEL_WIDTH, 0, MapWnd::SIDE_PANEL_WIDTH, GG::GUI::GetGUI()->AppHeight());
    m_side_panel->Hide();

    m_map_view_hole = GG::Rect(0, 0, CHILD_WIDTHS + SidePanel::MAX_PLANET_DIAMETER, h);

    m_build_selector = new BuildSelector(CHILD_WIDTHS, BUILD_SELECTOR_HEIGHT);
    m_build_selector->MoveTo(GG::Pt(0, h - BUILD_SELECTOR_HEIGHT));


    m_misc_connections.insert(GG::Connect(m_build_detail_panel->RequestNamedBuildItemSignal, &BuildDesignatorWnd::BuildItemRequested, this));
    m_misc_connections.insert(GG::Connect(m_build_detail_panel->RequestIDedBuildItemSignal, &BuildDesignatorWnd::BuildItemRequested, this));
    m_misc_connections.insert(GG::Connect(m_build_detail_panel->BuildQuantityChangedSignal, BuildQuantityChangedSignal));

    m_misc_connections.insert(GG::Connect(m_build_selector->DisplayNamedBuildItemSignal, &BuildDesignatorWnd::BuildDetailPanel::SetBuildItem, m_build_detail_panel));
    m_misc_connections.insert(GG::Connect(m_build_selector->DisplayIDedBuildItemSignal, &BuildDesignatorWnd::BuildDetailPanel::SetBuildItem, m_build_detail_panel));
    m_misc_connections.insert(GG::Connect(m_build_selector->RequestNamedBuildItemSignal, &BuildDesignatorWnd::BuildItemRequested, this));
    m_misc_connections.insert(GG::Connect(m_build_selector->RequestIDedBuildItemSignal, &BuildDesignatorWnd::BuildItemRequested, this));

    m_misc_connections.insert(GG::Connect(m_side_panel->PlanetSelectedSignal, &BuildDesignatorWnd::SelectPlanet, this));
    m_misc_connections.insert(GG::Connect(m_side_panel->SystemSelectedSignal, SystemSelectedSignal));


    // connect build type button clicks to update display
    GG::Connect(m_build_selector->m_build_type_buttons[BT_BUILDING]->ClickedSignal, ToggleBuildTypeFunctor(this, BT_BUILDING));
    GG::Connect(m_build_selector->m_build_type_buttons[BT_SHIP]->ClickedSignal, ToggleBuildTypeFunctor(this, BT_SHIP));
    GG::Connect(m_build_selector->m_build_type_buttons[BT_ORBITAL]->ClickedSignal, ToggleBuildTypeFunctor(this, BT_ORBITAL));
    GG::Connect(m_build_selector->m_build_type_buttons[NUM_BUILD_TYPES]->ClickedSignal, ToggleAllBuildTypesFunctor(this));    // last button should be "All" button

    // connect availability button clicks to update display
    GG::Connect(m_build_selector->m_availability_buttons.at(0)->ClickedSignal, ToggleAvailabilityFunctor(this, true));    // available items
    GG::Connect(m_build_selector->m_availability_buttons.at(1)->ClickedSignal, ToggleAvailabilityFunctor(this, false));   // UNavailable items

    AttachChild(m_build_detail_panel);
    AttachChild(m_build_selector);
    AttachChild(m_side_panel);

    ShowAllTypes(false);            // without populating the list
    ShowAvailability(false, false); // ...
    ShowAvailability(true, false);  // ...
}

BuildDesignatorWnd::~BuildDesignatorWnd()
{
    // disconnect all signals
    while (!m_misc_connections.empty()) {
        m_misc_connections.begin()->disconnect();
        m_misc_connections.erase(m_misc_connections.begin());
    }
}

const std::set<BuildType>& BuildDesignatorWnd::GetBuildTypesShown() const
{
    return m_build_selector->GetBuildTypesShown();
}

const std::pair<bool, bool>& BuildDesignatorWnd::GetAvailabilitiesShown() const
{
    return m_build_selector->GetAvailabilitiesShown();
}

bool BuildDesignatorWnd::InWindow(const GG::Pt& pt) const
{
    GG::Rect clip_rect = m_map_view_hole + UpperLeft();
    return clip_rect.Contains(pt) ?
        (m_build_detail_panel->InWindow(pt) || m_build_selector->InWindow(pt) || m_side_panel->InWindow(pt)) :
        Wnd::InClient(pt);
}

bool BuildDesignatorWnd::InClient(const GG::Pt& pt) const
{
    GG::Rect clip_rect = m_map_view_hole + UpperLeft();
    return clip_rect.Contains(pt) ?
        (m_build_detail_panel->InClient(pt) || m_build_selector->InClient(pt) || m_side_panel->InClient(pt)) :
        Wnd::InClient(pt);
}

GG::Rect BuildDesignatorWnd::MapViewHole() const
{
    return m_map_view_hole;
}

int BuildDesignatorWnd::QueueIndexShown() const
{
    return m_build_detail_panel->QueueIndexShown();
}

void BuildDesignatorWnd::CenterOnBuild(int queue_idx)
{
    m_build_detail_panel->SetBuild(queue_idx);
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ProductionQueue& queue = empire->GetProductionQueue();
    if (0 <= queue_idx && queue_idx < static_cast<int>(queue.size())) {
        UniverseObject* build_location = GetUniverse().Object(queue[queue_idx].location);
        assert(build_location);
        // this code assumes that the build site is a planet
        int system = build_location->SystemID();
        MapWnd* map = ClientUI::GetClientUI()->GetMapWnd();
        map->CenterOnSystem(system);
        if (m_side_panel->SystemID() != system)
            SystemSelectedSignal(system);
        m_side_panel->SelectPlanet(queue[queue_idx].location);
    }
}

void BuildDesignatorWnd::SelectSystem(int system)
{
    if (system != UniverseObject::INVALID_OBJECT_ID) {
        if (system != m_side_panel->SystemID()) {
            m_build_location = UniverseObject::INVALID_OBJECT_ID;
        }
        SelectDefaultPlanet(system);
    }
}

void BuildDesignatorWnd::SelectPlanet(int planet)
{
    m_build_location = planet;
    m_build_detail_panel->SelectedBuildLocation(planet);
    m_build_selector->SetBuildLocation(planet);
    if (planet != UniverseObject::INVALID_OBJECT_ID)
        m_system_default_planets[m_side_panel->SystemID()] = planet;
}

void BuildDesignatorWnd::Reset()
{
    // default to the home system when nothing is selected in the main map's SidePanel
    if (m_side_panel->SystemID() == UniverseObject::INVALID_OBJECT_ID) {
        int home_system_id = GetUniverse().Object<Planet>(Empires().Lookup(HumanClientApp::GetApp()->EmpireID())->HomeworldID())->SystemID();
        SystemSelectedSignal(home_system_id);
    }
    SelectDefaultPlanet(m_side_panel->SystemID());
    m_build_selector->Reset(true);
    m_build_detail_panel->Reset();
    m_side_panel->Refresh();
}

void BuildDesignatorWnd::Clear()
{
    m_build_detail_panel->Clear();
    m_build_selector->Reset(false);
    SystemSelectedSignal(UniverseObject::INVALID_OBJECT_ID);
    m_side_panel->Hide();
    m_build_location = UniverseObject::INVALID_OBJECT_ID;
    m_system_default_planets.clear();
}

void BuildDesignatorWnd::ShowType(BuildType type, bool refresh_list)
{
    Logger().errorStream() << "BuildDesignatorWnd::ShowType(" << boost::lexical_cast<std::string>(type) << ")";
    if (type == BT_BUILDING || type == BT_SHIP || type == BT_ORBITAL) {
        m_build_selector->ShowType(type, refresh_list);
        m_build_selector->m_build_type_buttons[type]->MarkSelectedGray();
    } else {
        throw std::invalid_argument("BuildDesignatorWnd::ShowType was passed an invalid BuildType");
    }
}

void BuildDesignatorWnd::ShowAllTypes(bool refresh_list)
{
    m_build_selector->ShowAllTypes(refresh_list);
    m_build_selector->m_build_type_buttons[BT_BUILDING]->MarkSelectedGray();
    m_build_selector->m_build_type_buttons[BT_SHIP]->MarkSelectedGray();
    m_build_selector->m_build_type_buttons[BT_ORBITAL]->MarkSelectedGray();
}

void BuildDesignatorWnd::HideType(BuildType type, bool refresh_list)
{
    Logger().errorStream() << "BuildDesignatorWnd::HideType(" << boost::lexical_cast<std::string>(type) << ")";
    if (type == BT_BUILDING || type == BT_SHIP || type == BT_ORBITAL) {
        m_build_selector->HideType(type, refresh_list);
        m_build_selector->m_build_type_buttons[type]->MarkNotSelected();
    } else {
        throw std::invalid_argument("BuildDesignatorWnd::HideType was passed an invalid BuildType");
    }
}

void BuildDesignatorWnd::HideAllTypes(bool refresh_list)
{
    m_build_selector->HideAllTypes(refresh_list);
    m_build_selector->m_build_type_buttons[BT_BUILDING]->MarkNotSelected();
    m_build_selector->m_build_type_buttons[BT_SHIP]->MarkNotSelected();
    m_build_selector->m_build_type_buttons[BT_ORBITAL]->MarkNotSelected();
}

void BuildDesignatorWnd::ToggleType(BuildType type, bool refresh_list)
{
    if (type == BT_BUILDING || type == BT_SHIP || type == BT_ORBITAL) {
        const std::set<BuildType>& types_shown = m_build_selector->GetBuildTypesShown();
        if (types_shown.find(type) == types_shown.end())
            ShowType(type, refresh_list);
        else
            HideType(type, refresh_list);
    } else {
        throw std::invalid_argument("BuildDesignatorWnd::ShowType was passed an invalid BuildType");
    } 
}

void BuildDesignatorWnd::ToggleAllTypes(bool refresh_list)
{
    const std::set<BuildType>& types_shown = m_build_selector->GetBuildTypesShown();
    if (types_shown.size() == 3)    // will need to update this if more build types are added
        HideAllTypes(refresh_list);
    else
        ShowAllTypes(refresh_list);
}

void BuildDesignatorWnd::ShowAvailability(bool available, bool refresh_list)
{
    m_build_selector->ShowAvailability(available, refresh_list);
    if (available)
        m_build_selector->m_availability_buttons.at(0)->MarkSelectedGray();
    else
        m_build_selector->m_availability_buttons.at(1)->MarkSelectedGray();
}

void BuildDesignatorWnd::HideAvailability(bool available, bool refresh_list)
{
    m_build_selector->HideAvailability(available, refresh_list);
    if (available)
        m_build_selector->m_availability_buttons.at(0)->MarkNotSelected();
    else
        m_build_selector->m_availability_buttons.at(1)->MarkNotSelected();
}

void BuildDesignatorWnd::ToggleAvailabilitly(bool available, bool refresh_list)
{
    const std::pair<bool, bool>& avail_shown = m_build_selector->GetAvailabilitiesShown();
    if (available) {
        if (avail_shown.first)
            HideAvailability(true, refresh_list);
        else
            ShowAvailability(true, refresh_list);
    } else {
        if (avail_shown.second)
            HideAvailability(false, refresh_list);
        else
            ShowAvailability(false, refresh_list);
    }
}

void BuildDesignatorWnd::BuildItemRequested(BuildType build_type, const std::string& item, int num_to_build)
{
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire && empire->BuildableItem(build_type, item, m_build_location))
        AddNamedBuildToQueueSignal(build_type, item, num_to_build, m_build_location);
}

void BuildDesignatorWnd::BuildItemRequested(BuildType build_type, int design_id, int num_to_build)
{
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire && empire->BuildableItem(build_type, design_id, m_build_location))
        AddIDedBuildToQueueSignal(build_type, design_id, num_to_build, m_build_location);
}

void BuildDesignatorWnd::BuildQuantityChanged(int queue_idx, int quantity)
{
    BuildQuantityChangedSignal(queue_idx, quantity);
}

void BuildDesignatorWnd::SelectDefaultPlanet(int system)
{
    m_side_panel->SetValidSelectionPredicate(boost::shared_ptr<UniverseObjectVisitor>(new OwnedVisitor<Planet>(HumanClientApp::GetApp()->EmpireID())));
    std::map<int, int>::iterator it = m_system_default_planets.find(system);
    if (it != m_system_default_planets.end()) {
        m_side_panel->SelectPlanet(it->second);
    } else {
        System::ObjectVec owned_planets =
            GetUniverse().Object<System>(system)->FindObjects(OwnedVisitor<Planet>(HumanClientApp::GetApp()->EmpireID()));
        if (!owned_planets.empty()) {
            int planet_id = owned_planets[0]->ID();
            double max_pop = owned_planets[0]->GetMeter(METER_POPULATION)->Current();
            for (unsigned int i = 1; i < owned_planets.size(); ++i) {
                double pop = owned_planets[0]->GetMeter(METER_POPULATION)->Current();
                if (max_pop < pop) {
                    max_pop = pop;
                    planet_id = owned_planets[0]->ID();
                }
            }
            m_side_panel->SelectPlanet(planet_id);
        }
    }
}
