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
}

//////////////////////////////////////////////////
// BuildDesignatorWnd::BuildDetailPanel
//////////////////////////////////////////////////
class BuildDesignatorWnd::BuildDetailPanel : public GG::Wnd
{
public:
    BuildDetailPanel(int w, int h);
    int QueueIndexShown() const;
    virtual void Render();
    void SelectedBuildLocation(int location);
    void SetBuildItem(BuildType build_type, const std::string& item);
    void SetBuild(int queue_idx);
    void Reset();
    void Clear();
    mutable boost::signal<void (int)> CenterOnBuildSignal;
    mutable boost::signal<void (BuildType, const std::string&, int)> RequestBuildItemSignal;
    mutable boost::signal<void (int, int)> BuildQuantityChangedSignal;

private:
    GG::Pt ItemGraphicUpperLeft() const;
    bool DisplayingQueueItem() const;
    void CenterClickedSlot();
    void AddToQueueClickedSlot();
    void ItemsToBuildChangedSlot(int value);
    void CheckBuildability();
    void ConfigureForQueueItemView();
    void ConfigureForNewBuildView();

    BuildType           m_build_type;
    std::string         m_item;
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

    boost::signals::connection m_num_items_to_build_connect;
};

BuildDesignatorWnd::BuildDetailPanel::BuildDetailPanel(int w, int h) :
    Wnd(0, 0, w, h, GG::CLICKABLE),
    m_build_type(INVALID_BUILD_TYPE),
    m_item(""),
    m_queue_idx(-1),
    m_build_location(UniverseObject::INVALID_OBJECT_ID)
{
    const int NAME_PTS = ClientUI::PTS + 8;
    const int COST_PTS = ClientUI::PTS;
    const int BUTTON_WIDTH = 150;
    boost::shared_ptr<GG::Font> default_font = GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS);
    m_item_name_text = new GG::TextControl(1, 0, w - 1 - BUTTON_WIDTH, NAME_PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::FONT_BOLD, NAME_PTS), ClientUI::TEXT_COLOR);
    m_cost_text = new GG::TextControl(1, m_item_name_text->LowerRight().y, w - 1 - BUTTON_WIDTH, COST_PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::FONT, COST_PTS), ClientUI::TEXT_COLOR);
    m_add_to_queue_button = new CUIButton(w - 1 - BUTTON_WIDTH, 1, BUTTON_WIDTH, UserString("PRODUCTION_DETAIL_ADD_TO_QUEUE"));
    m_recenter_button = new CUIButton(w - 1 - BUTTON_WIDTH, 1, BUTTON_WIDTH, UserString("PRODUCTION_DETAIL_CENTER_ON_BUILD"));
    m_recenter_button->Disable();
    m_add_to_queue_button->Disable();
    m_num_items_to_build = new CUISpin<int>(0, 0, 75, 1, 1, 1, 1000, true);
    m_num_items_to_build->MoveTo(GG::Pt(1, h - m_num_items_to_build->Height() - 1));
    m_num_items_to_build_label = new GG::TextControl(m_num_items_to_build->LowerRight().x + 3, m_num_items_to_build->UpperLeft().y + (m_num_items_to_build->Height() - (NAME_PTS + 4)) / 2, w - 1 - BUTTON_WIDTH - 3 - (m_num_items_to_build->LowerRight().x + 3), NAME_PTS + 4,
                                                     UserString("PRODUCTION_DETAIL_NUMBER_TO_BUILD"), default_font, ClientUI::TEXT_COLOR, GG::TF_LEFT);
    m_build_location_name_text = new GG::TextControl(w - 1 - BUTTON_WIDTH, m_num_items_to_build->UpperLeft().y + (m_num_items_to_build->Height() - (NAME_PTS + 4)) / 2, BUTTON_WIDTH, ClientUI::PTS + 4,
                                                     "", default_font, ClientUI::TEXT_COLOR);
    m_description_box = new CUIMultiEdit(1, m_cost_text->LowerRight().y, w - 2 - BUTTON_WIDTH, m_num_items_to_build_label->UpperLeft().y - 2 - m_cost_text->LowerRight().y, "", GG::TF_WORDBREAK | GG::MultiEdit::READ_ONLY);
    m_description_box->SetColor(GG::CLR_ZERO);
    m_description_box->SetInteriorColor(GG::CLR_ZERO);

    m_item_graphic = 0;

    GG::Connect(m_recenter_button->ClickedSignal, &BuildDesignatorWnd::BuildDetailPanel::CenterClickedSlot, this);
    GG::Connect(m_add_to_queue_button->ClickedSignal, &BuildDesignatorWnd::BuildDetailPanel::AddToQueueClickedSlot, this);
    m_num_items_to_build_connect = GG::Connect(m_num_items_to_build->ValueChangedSignal, &BuildDesignatorWnd::BuildDetailPanel::ItemsToBuildChangedSlot, this);

    AttachChild(m_item_name_text);
    AttachChild(m_cost_text);
    AttachChild(m_recenter_button);
    AttachChild(m_add_to_queue_button);
    AttachChild(m_num_items_to_build);
    AttachChild(m_num_items_to_build_label);
    AttachChild(m_build_location_name_text);
    AttachChild(m_description_box);
}

int BuildDesignatorWnd::BuildDetailPanel::QueueIndexShown() const
{
    return m_queue_idx;
}

void BuildDesignatorWnd::BuildDetailPanel::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::WND_COLOR, GG::CLR_ZERO, 0);
}

void BuildDesignatorWnd::BuildDetailPanel::SelectedBuildLocation(int location)
{
    m_build_location = location;
    CheckBuildability();
}

void BuildDesignatorWnd::BuildDetailPanel::SetBuildItem(BuildType build_type, const std::string& item)
{
    m_build_type = build_type;
    m_item = item;
    m_queue_idx = -1;
    Reset();
}

void BuildDesignatorWnd::BuildDetailPanel::SetBuild(int queue_idx)
{
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ProductionQueue& queue = empire->GetProductionQueue();
    if (0 <= queue_idx && queue_idx < static_cast<int>(queue.size())) {
        m_build_type = queue[queue_idx].item.build_type;
        m_item = queue[queue_idx].item.name;
        m_queue_idx = queue_idx;
    } else {
        m_build_type = INVALID_BUILD_TYPE;
        m_item = "";
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
    m_num_items_to_build_connect.disconnect();
    m_num_items_to_build->SetValue(1);

    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (!empire) {
        m_num_items_to_build_connect = GG::Connect(m_num_items_to_build->ValueChangedSignal, &BuildDesignatorWnd::BuildDetailPanel::ItemsToBuildChangedSlot, this);
        return;
    }

    const ProductionQueue& queue = empire->GetProductionQueue();
    if (static_cast<int>(queue.size()) <= m_queue_idx) {
        if (!queue.empty()) {
            m_queue_idx = queue.size() - 1;
            m_build_type = queue[m_queue_idx].item.build_type;
            m_item = queue[m_queue_idx].item.name;
        } else {
            m_build_type = INVALID_BUILD_TYPE;
            m_item = "";
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

    m_num_items_to_build_connect = GG::Connect(m_num_items_to_build->ValueChangedSignal, &BuildDesignatorWnd::BuildDetailPanel::ItemsToBuildChangedSlot, this);

    CheckBuildability();

    using boost::io::str;
    using boost::format;
    double cost_per_turn = 0;
    int turns = 0;
    std::string item_name_str = UserString(m_item);
    std::string description_str;
    boost::shared_ptr<GG::Texture> graphic;
    if (m_build_type == BT_BUILDING) {
        assert(empire);
        const BuildingType* building_type = GetBuildingType(m_item);
        assert(building_type);
        turns = building_type->BuildTime();
        boost::tie(cost_per_turn, turns) = empire->ProductionCostAndTime(BT_BUILDING, m_item);
        if (building_type->Effects().empty()) {
            description_str = str(format(UserString("PRODUCTION_DETAIL_BUILDING_DESCRIPTION_STR"))
                                  % UserString(building_type->Description()));
        } else {
            description_str = str(format(UserString("PRODUCTION_DETAIL_BUILDING_DESCRIPTION_STR_WITH_EFFECTS"))
                                  % UserString(building_type->Description())
                                  % EffectsDescription(building_type->Effects()));
        }
        if (!building_type->Graphic().empty())
            graphic = HumanClientApp::GetApp()->GetTextureOrDefault(ClientUI::ART_DIR + building_type->Graphic());
    } else if (m_build_type == BT_SHIP) {
        assert(empire);
        const ShipDesign* design = empire->GetShipDesign(m_item);
        assert(design);
        turns = 5; // this is a kludge for v0.3 only
        boost::tie(cost_per_turn, turns) = empire->ProductionCostAndTime(BT_SHIP, m_item);
        item_name_str = m_item;
        description_str = str(format(UserString("PRODUCTION_DETAIL_SHIP_DESCRIPTION_STR"))
                              % design->description
                              % design->attack
                              % design->defense
                              % design->speed);
        graphic = HumanClientApp::GetApp()->GetTextureOrDefault(ClientUI::ART_DIR + design->graphic);
    } else if (m_build_type == BT_ORBITAL) {
        turns = DEFENSE_BASE_BUILD_TURNS;
        cost_per_turn = DEFENSE_BASE_BUILD_COST;
        item_name_str = UserString("DEFENSE_BASE");
        description_str = UserString("DEFENSE_BASE_DESCRIPTION");
        graphic = HumanClientApp::GetApp()->GetTextureOrDefault(ClientUI::ART_DIR + "misc/base1.png"); // this is a kludge for v0.3 only
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
    if (m_build_type != INVALID_BUILD_TYPE)
        RequestBuildItemSignal(m_build_type, m_item, m_num_items_to_build->Value());
}

void BuildDesignatorWnd::BuildDetailPanel::ItemsToBuildChangedSlot(int value)
{
    if (DisplayingQueueItem())
        BuildQuantityChangedSignal(m_queue_idx, value);
}

void BuildDesignatorWnd::BuildDetailPanel::CheckBuildability()
{
    m_add_to_queue_button->Disable(true);
    if (!DisplayingQueueItem() || m_build_type == BT_BUILDING) {
        m_num_items_to_build->Disable(true);
        m_num_items_to_build_label->Disable(true);
    }
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire && empire->BuildableItem(m_build_type, m_item, m_build_location)) {
        m_add_to_queue_button->Disable(false);
        if (m_build_type != BT_BUILDING) {
            m_num_items_to_build->Disable(false);
            m_num_items_to_build_label->Disable(false);
        }
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

    virtual void MinimizeClicked();

    void Reset(bool keep_selection);

    mutable boost::signal<void (BuildType, const std::string&)> DisplayBuildItemSignal;
    mutable boost::signal<void (BuildType, const std::string&, int)> RequestBuildItemSignal;

private:
    struct CategoryClickedFunctor
    {
        CategoryClickedFunctor(BuildType build_type_, BuildSelector& build_selector_);
        void operator()();
        const BuildType build_type;
        BuildSelector&  build_selector;
    };

    void PopulateList(BuildType build_type, bool keep_selection);
    void BuildItemSelected(const std::set<int>& selections);
    void BuildItemDoubleClicked(int row_index, GG::ListBox::Row* row);

    BuildType                              m_current_build_type;
    std::vector<CUIButton*>                m_build_category_buttons;
    BuildableItemsListBox*                 m_buildable_items;
    std::map<GG::ListBox::Row*, BuildType> m_build_types;
    GG::Pt                                 m_original_ul;

    friend struct PopulateListFunctor;
};

BuildDesignatorWnd::BuildSelector::CategoryClickedFunctor::CategoryClickedFunctor(BuildType build_type_, BuildSelector& build_selector_) :
    build_type(build_type_),
    build_selector(build_selector_)
{}

void BuildDesignatorWnd::BuildSelector::CategoryClickedFunctor::operator()()
{
    build_selector.PopulateList(build_type, false);
}

BuildDesignatorWnd::BuildSelector::BuildSelector(int w, int h) :
    CUIWnd(UserString("PRODUCTION_WND_BUILD_ITEMS_TITLE"), 0, 0, w, h, GG::CLICKABLE | CUIWnd::MINIMIZABLE),
    m_current_build_type(BT_BUILDING)
{
    GG::Pt client_size = ClientSize();
    GG::Layout* layout = new GG::Layout(0, 0, client_size.x, client_size.y, 1, 1, 3, 6);
    int button_height;
    for (BuildType i = BuildType(BT_NOT_BUILDING + 1); i < NUM_BUILD_TYPES; i = BuildType(i + 1)) {
        CUIButton* button = new CUIButton(0, 0, 1, UserString("PRODUCTION_WND_CATEGORY_" + boost::lexical_cast<std::string>(i)));
        button_height = button->Height();
        GG::Connect(button->ClickedSignal, CategoryClickedFunctor(i, *this));
        m_build_category_buttons.push_back(button);
        layout->Add(button, 0, i - (BT_NOT_BUILDING + 1));
    }
    CUIButton* button = new CUIButton(0, 0, 1, UserString("ALL"));
    button_height = button->Height();
    GG::Connect(button->ClickedSignal, CategoryClickedFunctor(NUM_BUILD_TYPES, *this));
    m_build_category_buttons.push_back(button);
    layout->Add(button, 0, NUM_BUILD_TYPES - (BT_NOT_BUILDING + 1));
    m_buildable_items = new BuildableItemsListBox(0, 0, 1, 1);
    GG::Connect(m_buildable_items->SelChangedSignal, &BuildDesignatorWnd::BuildSelector::BuildItemSelected, this);
    GG::Connect(m_buildable_items->DoubleClickedSignal, &BuildDesignatorWnd::BuildSelector::BuildItemDoubleClicked, this);
    m_buildable_items->SetStyle(GG::LB_NOSORT | GG::LB_SINGLESEL);
    layout->Add(m_buildable_items, 1, 0, 1, layout->Columns());
    layout->SetMinimumRowHeight(0, button_height);
    layout->SetRowStretch(0, 0);
    layout->SetRowStretch(1, 1);
    AttachChild(layout);
    PopulateList(m_current_build_type, false);
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

void BuildDesignatorWnd::BuildSelector::Reset(bool keep_selection)
{
    m_current_build_type = BT_BUILDING;
    PopulateList(m_current_build_type, keep_selection);
}

void BuildDesignatorWnd::BuildSelector::PopulateList(BuildType build_type, bool keep_selection)
{
    std::string selected_row_data_type;
    int row_to_select = -1;
    if (keep_selection && build_type == m_current_build_type && m_buildable_items->Selections().size() == 1) {
        selected_row_data_type = m_buildable_items->GetRow(*m_buildable_items->Selections().begin()).DragDropDataType();
    }

    const int ROW_HEIGHT = 24;          // should change to be font point size...
    const int ICON_COL_WIDTH = ROW_HEIGHT;
    const int NAME_COL_WIDTH = 150;
    const int COST_COL_WIDTH = 30;
    const int TIME_COL_WIDTH = 20;
    const int DESC_COL_WIDTH = m_buildable_items->Width() - (NAME_COL_WIDTH + COST_COL_WIDTH + TIME_COL_WIDTH);

    m_buildable_items->SetNumCols(5);
    m_buildable_items->LockColWidths();
    m_buildable_items->SetColWidth(0, ICON_COL_WIDTH);
    m_buildable_items->SetColWidth(1, NAME_COL_WIDTH);
    m_buildable_items->SetColWidth(2, COST_COL_WIDTH);
    m_buildable_items->SetColWidth(3, TIME_COL_WIDTH);
    m_buildable_items->SetColWidth(4, DESC_COL_WIDTH);
    m_buildable_items->SetColAlignment(0, GG::ALIGN_LEFT);
    m_buildable_items->SetColAlignment(1, GG::ALIGN_LEFT);
    m_buildable_items->SetColAlignment(2, GG::ALIGN_LEFT);
    m_buildable_items->SetColAlignment(3, GG::ALIGN_LEFT);
    m_buildable_items->SetColAlignment(4, GG::ALIGN_LEFT);

    m_current_build_type = build_type;
    m_buildable_items->Clear();
    m_build_types.clear();

    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (!empire) return;

    boost::shared_ptr<GG::Font> default_font = GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS);

    int i = 0;
    if (build_type == BT_BUILDING || build_type == NUM_BUILD_TYPES) {
        Empire::BuildingTypeItr end_it = empire->BuildingTypeEnd();
        for (Empire::BuildingTypeItr it = empire->BuildingTypeBegin(); it != end_it; ++it, ++i) {
            GG::ListBox::Row* row = new GG::ListBox::Row();
            row->SetDragDropDataType(*it);

            const BuildingType* type = GetBuildingType(*it);

            // icon
            GG::StaticGraphic* icon = new GG::StaticGraphic(0, 0, ICON_COL_WIDTH, ROW_HEIGHT, 
                HumanClientApp::GetApp()->GetTextureOrDefault(ClientUI::ART_DIR + type->Graphic()),
                GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
            row->push_back(dynamic_cast<GG::Control*>(icon));

            // building name
            row->push_back(UserString(*it), default_font, ClientUI::TEXT_COLOR);

            // cost / turn, and minimum production turns
            const std::pair<double, int> cost_time = empire->ProductionCostAndTime(BT_BUILDING, *it);
            std::string cost_text = boost::lexical_cast<std::string>(cost_time.first);
            row->push_back(cost_text, default_font, ClientUI::TEXT_COLOR);
            std::string time_text = boost::lexical_cast<std::string>(cost_time.second);
            row->push_back(time_text, default_font, ClientUI::TEXT_COLOR);

            // brief description
            std::string desc_text = "DESCRIPTIVE TEXT";
            row->push_back(desc_text, default_font, ClientUI::TEXT_COLOR);  

            m_buildable_items->Insert(row);
            m_build_types[row] = BT_BUILDING;
            if (row->DragDropDataType() == selected_row_data_type)
                row_to_select = i;
        }
    }
    if (build_type == BT_SHIP || build_type == NUM_BUILD_TYPES) {
        Empire::ShipDesignItr end_it = empire->ShipDesignEnd();
        for (Empire::ShipDesignItr it = empire->ShipDesignBegin(); it != end_it; ++it, ++i) {
            GG::ListBox::Row* row = new GG::ListBox::Row();
            row->SetDragDropDataType(it->first);

            const ShipDesign* type = GetShipDesign(empire->EmpireID(), it->first);

            // icon
            GG::StaticGraphic* icon = new GG::StaticGraphic(0, 0, ICON_COL_WIDTH, ROW_HEIGHT, 
                HumanClientApp::GetApp()->GetTextureOrDefault(ClientUI::ART_DIR + type->graphic),
                GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
            row->push_back(dynamic_cast<GG::Control*>(icon));

            // ship design name
            row->push_back(it->first, default_font, ClientUI::TEXT_COLOR);

            // cost / turn, and minimum production turns
            const std::pair<double, int> cost_time = empire->ProductionCostAndTime(BT_SHIP, it->first);
            std::string cost_text = boost::lexical_cast<std::string>(cost_time.first);
            row->push_back(cost_text, default_font, ClientUI::TEXT_COLOR);
            std::string time_text = boost::lexical_cast<std::string>(cost_time.second);
            row->push_back(time_text, default_font, ClientUI::TEXT_COLOR);

            // brief description            
            std::string desc_text = "DESCRIPTIVE TEXT";
            row->push_back(desc_text, default_font, ClientUI::TEXT_COLOR);  

            m_buildable_items->Insert(row);
            m_build_types[row] = BT_SHIP;
            if (row->DragDropDataType() == selected_row_data_type)
                row_to_select = i;
        }
    }
    if (build_type == BT_ORBITAL || build_type == NUM_BUILD_TYPES) {
        GG::ListBox::Row* row = new GG::ListBox::Row();
        row->SetDragDropDataType("DEFENSE_BASE");

        // icon
        GG::StaticGraphic* icon = new GG::StaticGraphic(0, 0, ICON_COL_WIDTH, ROW_HEIGHT, 
            HumanClientApp::GetApp()->GetTextureOrDefault(ClientUI::ART_DIR + "icons/defensebase.png"),
            GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
        row->push_back(dynamic_cast<GG::Control*>(icon));

        // Defense Base "name"
        row->push_back(UserString(row->DragDropDataType()), default_font, ClientUI::TEXT_COLOR);

        // cost / turn, and minimum production turns
        const std::pair<double, int> cost_time = empire->ProductionCostAndTime(BT_ORBITAL, UserString(row->DragDropDataType()));
        std::string cost_text = boost::lexical_cast<std::string>(cost_time.first);
        row->push_back(cost_text, default_font, ClientUI::TEXT_COLOR);
        std::string time_text = boost::lexical_cast<std::string>(cost_time.second);
        row->push_back(time_text, default_font, ClientUI::TEXT_COLOR);

        // brief description            
        std::string desc_text = "DESCRIPTIVE TEXT";
        row->push_back(desc_text, default_font, ClientUI::TEXT_COLOR);  

        m_buildable_items->Insert(row);
        m_build_types[row] = BT_ORBITAL;
        if (row->DragDropDataType() == selected_row_data_type)
            row_to_select = i;
    }

    if (row_to_select != -1)
        m_buildable_items->SelectRow(row_to_select);
}

void BuildDesignatorWnd::BuildSelector::BuildItemSelected(const std::set<int>& selections)
{
    if (selections.size() == 1) {
        GG::ListBox::Row* row = &m_buildable_items->GetRow(*selections.begin());
        DisplayBuildItemSignal(m_build_types[row], row->DragDropDataType());
    }
}

void BuildDesignatorWnd::BuildSelector::BuildItemDoubleClicked(int row_index, GG::ListBox::Row* row)
{
    RequestBuildItemSignal(m_build_types[row], row->DragDropDataType(), 1);
}


//////////////////////////////////////////////////
// BuildDesignatorWnd
//////////////////////////////////////////////////
BuildDesignatorWnd::BuildDesignatorWnd(int w, int h) :
    Wnd(0, 0, w, h, GG::CLICKABLE),
    m_build_location(UniverseObject::INVALID_OBJECT_ID)
{
    const int SIDE_PANEL_PLANET_RADIUS = SidePanel::MAX_PLANET_DIAMETER / 2;
    int CHILD_WIDTHS = w - MapWnd::SIDE_PANEL_WIDTH - SIDE_PANEL_PLANET_RADIUS;
    const int DETAIL_PANEL_HEIGHT = TechTreeWnd::NAVIGATOR_AND_DETAIL_HEIGHT;
    const int BUILD_SELECTOR_HEIGHT = DETAIL_PANEL_HEIGHT;
    m_build_detail_panel = new BuildDetailPanel(CHILD_WIDTHS, DETAIL_PANEL_HEIGHT);
    m_build_selector = new BuildSelector(CHILD_WIDTHS, BUILD_SELECTOR_HEIGHT);
    m_build_selector->MoveTo(GG::Pt(0, h - BUILD_SELECTOR_HEIGHT));
    m_side_panel = new SidePanel(CHILD_WIDTHS + SIDE_PANEL_PLANET_RADIUS, 0, MapWnd::SIDE_PANEL_WIDTH, h);
    m_side_panel->Hide();

    GG::Connect(m_build_detail_panel->RequestBuildItemSignal, &BuildDesignatorWnd::BuildItemRequested, this);
    GG::Connect(m_build_detail_panel->BuildQuantityChangedSignal, BuildQuantityChangedSignal);
    GG::Connect(m_build_selector->DisplayBuildItemSignal, &BuildDesignatorWnd::BuildDetailPanel::SetBuildItem, m_build_detail_panel);
    GG::Connect(m_build_selector->RequestBuildItemSignal, &BuildDesignatorWnd::BuildItemRequested, this);
    GG::Connect(m_side_panel->PlanetSelectedSignal, &BuildDesignatorWnd::SelectPlanet, this);

    m_map_view_hole = GG::Rect(0, 0, CHILD_WIDTHS + SIDE_PANEL_PLANET_RADIUS, h);

    AttachChild(m_build_detail_panel);
    AttachChild(m_build_selector);
    AttachChild(m_side_panel);
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
            m_side_panel->SetSystem(system);
        m_side_panel->SelectPlanet(queue[queue_idx].location);
    }
}

void BuildDesignatorWnd::SelectSystem(int system)
{
    if (system != UniverseObject::INVALID_OBJECT_ID) {
        if (system != m_side_panel->SystemID()) {
            m_side_panel->Show();
            m_side_panel->SetSystem(system);
            m_build_location = UniverseObject::INVALID_OBJECT_ID;
        }
        SelectDefaultPlanet(system);
    }
}

void BuildDesignatorWnd::SelectPlanet(int planet)
{
    m_build_location = planet;
    m_build_detail_panel->SelectedBuildLocation(planet);
    if (planet != UniverseObject::INVALID_OBJECT_ID)
        m_system_default_planets[m_side_panel->SystemID()] = planet;
}

void BuildDesignatorWnd::Reset()
{
    // default to the home system when nothing is selected in the main map's SidePanel
    if (m_side_panel->SystemID() == UniverseObject::INVALID_OBJECT_ID) {
        int home_system_id = GetUniverse().Object<Planet>(Empires().Lookup(HumanClientApp::GetApp()->EmpireID())->HomeworldID())->SystemID();
        m_side_panel->SetSystem(home_system_id);
    }
    SelectDefaultPlanet(m_side_panel->SystemID());
    m_build_selector->Reset(true);
    m_build_detail_panel->Reset();
}

void BuildDesignatorWnd::Clear()
{
    m_build_detail_panel->Clear();
    m_build_selector->Reset(false);
    m_side_panel->SetSystem(UniverseObject::INVALID_OBJECT_ID);
    m_side_panel->Hide();
    m_build_location = UniverseObject::INVALID_OBJECT_ID;
    m_system_default_planets.clear();
}

void BuildDesignatorWnd::BuildItemRequested(BuildType build_type, const std::string& item, int num_to_build)
{
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire && empire->BuildableItem(build_type, item, m_build_location))
        AddBuildToQueueSignal(build_type, item, num_to_build, m_build_location);
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
