#include "BuildDesignatorWnd.h"

#include "CUIControls.h"
#include "ClientUI.h"
#include "CUISpin.h"
#include "CUIWnd.h"
#include "SidePanel.h"
#include "TechTreeWnd.h"
#include "MapWnd.h"
#include "EncyclopediaDetailPanel.h"
#include "InfoPanels.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/UniverseObject.h"
#include "../Empire/Empire.h"
#include "../universe/Building.h"
#include "../universe/ShipDesign.h"
#include "../universe/Effect.h"
#include "../universe/Condition.h"

#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>
#include <GG/TextControl.h>

namespace {
    //////////////////////////////////
    // ProductionItemPanel
    //////////////////////////////////
    class ProductionItemPanel : public GG::Control {
    public:
        ProductionItemPanel(GG::X w, GG::Y h, const ProductionQueue::ProductionItem& item, int empire_id) :
            Control(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()),
            m_item(item),
            m_empire_id(empire_id),
            m_icon(0),
            m_name(0),
            m_cost(0),
            m_time(0),
            m_desc(0)
        {
            SetChildClippingMode(ClipToClient);

            const Empire* empire = Empires().Lookup(m_empire_id);

            boost::shared_ptr<GG::Texture>                  texture;
            std::string                                     name_text;
            std::string                                     desc_text;
            std::vector<const Condition::ConditionBase*>    location_conditions;
            switch (m_item.build_type) {
            case BT_BUILDING: {
                texture = ClientUI::BuildingIcon(m_item.name);
                desc_text = UserString("BT_BUILDING"); 
                name_text = UserString(m_item.name);
                break;
            }
            case BT_SHIP: {
                texture = ClientUI::ShipDesignIcon(m_item.design_id);
                desc_text = UserString("BT_SHIP");
                name_text = m_item.name;
                break;
            }
            default:
                Logger().errorStream() << "ProductionItemPanel::Init got invalid item type";
                texture = ClientUI::GetTexture("");
            }

            m_icon = new GG::StaticGraphic(GG::X0, GG::Y0, GG::X1, GG::Y1, texture,
                                           GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
            AttachChild(m_icon);

            m_name = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, name_text,
                                         ClientUI::GetFont(), ClientUI::TextColor(),
                                         GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            AttachChild(m_name);

            // cost / turn, and minimum production turns
            std::string cost_text;
            std::string time_text;
            if (empire) {
                std::pair<double, int> cost_time = empire->ProductionCostAndTime(m_item);
                cost_text = DoubleToString(cost_time.first, 3, false);
                time_text = boost::lexical_cast<std::string>(cost_time.second);
            }
            m_cost = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, cost_text,
                                         ClientUI::GetFont(), ClientUI::TextColor(),
                                         GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
            AttachChild(m_cost);

            m_time = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, time_text,
                                         ClientUI::GetFont(), ClientUI::TextColor(),
                                         GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
            AttachChild(m_time);

            m_desc = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, desc_text,
                                         ClientUI::GetFont(), ClientUI::TextColor(),
                                         GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            AttachChild(m_desc);

            DoLayout();
        }

        /** Renders panel background and border. */
        virtual void    Render() {
            GG::Clr background_clr = this->Disabled() ? ClientUI::WndColor() : ClientUI::CtrlColor();
            GG::FlatRectangle(UpperLeft(), LowerRight(), background_clr, ClientUI::WndOuterBorderColor(), 1u);
        }

        virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::Control::SizeMove(ul, lr);
            //std::cout << "ProductionItemPanel::SizeMove new size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (old_size != Size())
                DoLayout();
        }

    private:
        void            DoLayout() {
            const GG::Y ICON_HEIGHT(ClientHeight());
            const GG::X ICON_WIDTH(Value(ClientHeight()));
            const GG::X ITEM_NAME_WIDTH(ClientUI::Pts() * 16);
            const GG::X COST_WIDTH(ClientUI::Pts() * 4);
            const GG::X TIME_WIDTH(ClientUI::Pts() * 3);
            const GG::X DESC_WIDTH(ClientUI::Pts() * 18);

            GG::X left(GG::X0);
            GG::Y top(GG::Y0);
            GG::Y bottom(ClientHeight());

            m_icon->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + ICON_WIDTH, bottom));
            left += ICON_WIDTH + GG::X(3);

            m_name->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + ITEM_NAME_WIDTH, bottom));
            left += ITEM_NAME_WIDTH;

            m_cost->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + COST_WIDTH, bottom));
            left += COST_WIDTH;

            m_time->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + TIME_WIDTH, bottom));
            left += TIME_WIDTH;

            m_desc->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + DESC_WIDTH, bottom));
        }

        const ProductionQueue::ProductionItem&  m_item;
        int                 m_empire_id;

        GG::StaticGraphic*  m_icon;
        GG::TextControl*    m_name;
        GG::TextControl*    m_cost;
        GG::TextControl*    m_time;
        GG::TextControl*    m_desc;
    };

    const UniverseObject* GetSourceObjectForEmpire(int empire_id) {
        // get a source object, which is owned by the empire,
        // preferably the capital
        const UniverseObject* source = 0;
        if (empire_id == ALL_EMPIRES)
            return source;
        const Empire* empire = Empires().Lookup(empire_id);
        if (!empire)
            return source;

        if (source = GetUniverseObject(empire->CapitalID()))
            return source;

        // not a valid source?!  scan through all objects to find one owned by this empire
        const ObjectMap& objects = GetUniverse().Objects();
        for (ObjectMap::const_iterator obj_it = objects.const_begin(); obj_it != objects.const_end(); ++obj_it) {
            if (obj_it->second->OwnedBy(empire_id)) {
                source = obj_it->second;
                break;
            }
        }

        // if this empire doesn't own ANYTHING, default to a null source
        return source;
    }

    std::string LocationConditionDescription(const std::string& building_name, int candidate_object_id,
                                             int empire_id)
    {
        std::vector<const Condition::ConditionBase*> location_conditions;
        if (const BuildingType* building_type = GetBuildingType(building_name))
            location_conditions.push_back(building_type->Location());
        const UniverseObject* source = GetSourceObjectForEmpire(empire_id);
        return ConditionDescription(location_conditions, GetUniverseObject(candidate_object_id), source);
    }

    std::string LocationConditionDescription(int ship_design_id, int candidate_object_id,
                                             int empire_id)
    {
        std::vector<const Condition::ConditionBase*> location_conditions;
        if (const ShipDesign* ship_design = GetShipDesign(ship_design_id)) {
            if (const HullType* hull_type = ship_design->GetHull())
                location_conditions.push_back(hull_type->Location());
            std::vector<std::string> parts = ship_design->Parts();
            for (std::vector<std::string>::const_iterator it = parts.begin();
                 it != parts.end(); ++it)
            {
                if (const PartType* part_type = GetPartType(*it))
                    location_conditions.push_back(part_type->Location());
            }
        }
        const UniverseObject* source = GetSourceObjectForEmpire(empire_id);
        return ConditionDescription(location_conditions, GetUniverseObject(candidate_object_id), source);
    }

    boost::shared_ptr<GG::BrowseInfoWnd> ProductionItemRowBrowseWnd(const ProductionQueue::ProductionItem& item,
                                                                    int candidate_object_id,
                                                                    int empire_id)
    {
        if (item.build_type == BT_BUILDING) {
            boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd(new IconTextBrowseWnd(
                ClientUI::BuildingIcon(item.name), UserString(item.name),
                LocationConditionDescription(item.name, candidate_object_id, empire_id)));
            return browse_wnd;
        } else if (item.build_type == BT_SHIP) {
            const ShipDesign* ship_design = GetShipDesign(item.design_id);
            const std::string& name = (ship_design ? ship_design->Name() : UserString("SHIP_DESIGN"));
            boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd(new IconTextBrowseWnd(
                ClientUI::ShipDesignIcon(item.design_id), name,
                LocationConditionDescription(item.design_id, candidate_object_id, empire_id)));
            return browse_wnd;
        } else {
            return boost::shared_ptr<GG::BrowseInfoWnd>();
        }
    }

    ////////////////////////////////////////////////
    // ProductionItemRow
    ////////////////////////////////////////////////
    class ProductionItemRow : public GG::ListBox::Row {
    public:
        ProductionItemRow(GG::X w, GG::Y h, const std::string& building_name, int empire_id,
                          int production_location = INVALID_OBJECT_ID) :
            GG::ListBox::Row(w, h, "", GG::ALIGN_NONE, 0),
            m_item(BT_BUILDING, building_name),
            m_empire_id(empire_id),
            m_panel(0)
        {
            SetName("ProductionItemRow");
            SetChildClippingMode(ClipToClient);
            SetDragDropDataType(building_name);
            m_panel = new ProductionItemPanel(w, h, m_item, m_empire_id);
            push_back(m_panel);
            if (const Empire* empire = Empires().Lookup(m_empire_id)) {
                if (!empire->BuildableItem(BT_BUILDING, building_name, production_location)) {
                    this->Disable(true);
                    m_panel->Disable(true);
                }
            }
            m_panel->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
            SetBrowseInfoWnd(ProductionItemRowBrowseWnd(m_item, production_location, m_empire_id));
        }

        ProductionItemRow(GG::X w, GG::Y h, int design_id, int empire_id,
                          int production_location = INVALID_OBJECT_ID) :
            GG::ListBox::Row(w, h, "", GG::ALIGN_NONE, 0),
            m_item(BT_SHIP, design_id),
            m_empire_id(empire_id),
            m_panel(0)
        {
            SetName("ProductionItemRow");
            SetChildClippingMode(ClipToClient);
            SetDragDropDataType(boost::lexical_cast<std::string>(design_id));
            m_panel = new ProductionItemPanel(w, h, m_item, m_empire_id);
            push_back(m_panel);
            if (const Empire* empire = Empires().Lookup(m_empire_id)) {
                if (!empire->BuildableItem(BT_SHIP, design_id, production_location)) {
                    this->Disable(true);
                    m_panel->Disable(true);
                }
            }
            m_panel->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
            SetBrowseInfoWnd(ProductionItemRowBrowseWnd(m_item, production_location, m_empire_id));
        }

        const   ProductionQueue::ProductionItem& Item() const
        { return m_item; }

        void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "ProductionItemRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (!empty() && old_size != Size() && m_panel)
                m_panel->Resize(Size());
        }

    private:
        ProductionQueue::ProductionItem m_item;
        int                             m_empire_id;
        ProductionItemPanel*            m_panel;
    };

    //////////////////////////////////
    // BuildableItemsListBox
    //////////////////////////////////
    class BuildableItemsListBox : public CUIListBox {
    public:
        BuildableItemsListBox(GG::X x, GG::Y y, GG::X w, GG::Y h) :
            CUIListBox(x, y, w, h)
        {
            // preinitialize listbox/row column widths, because what
            // ListBox::Insert does on default is not suitable for this case
            SetNumCols(1);
            SetColWidth(0, GG::X0);
            LockColWidths();
        }

        virtual void    GainingFocus() {
            DeselectAll();
            SelChangedSignal(Selections());
        }

        virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            CUIListBox::SizeMove(ul, lr);
            //std::cout << "BuildableItemsListBox::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (old_size != Size()) {
                const GG::Pt row_size = ListRowSize();
                //std::cout << "BuildableItemsListBox::SizeMove list row size: (" << Value(row_size.x) << ", " << Value(row_size.y) << ")" << std::endl;
                for (GG::ListBox::iterator it = begin(); it != end(); ++it)
                    (*it)->Resize(row_size);
            }
        }

        GG::Pt          ListRowSize() const {
            return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, ListRowHeight());
        }

        static GG::Y    ListRowHeight() {
            return GG::Y(ClientUI::Pts() * 3/2);
        }
    };

    struct ToggleBuildTypeFunctor {
        ToggleBuildTypeFunctor(BuildDesignatorWnd* designator_wnd, BuildType type) : m_designator_wnd(designator_wnd), m_build_type(type) {}
        void operator()() {m_designator_wnd->ToggleType(m_build_type);}
        BuildDesignatorWnd* const m_designator_wnd;
        const BuildType m_build_type;
    };

    struct ToggleAllBuildTypesFunctor {
        ToggleAllBuildTypesFunctor(BuildDesignatorWnd* designator_wnd) : m_designator_wnd(designator_wnd) {}
        void operator()() {m_designator_wnd->ToggleAllTypes();}
        BuildDesignatorWnd* const m_designator_wnd;
    };

    struct ToggleAvailabilityFunctor {
        ToggleAvailabilityFunctor(BuildDesignatorWnd* designator_wnd, bool available) : m_designator_wnd(designator_wnd), m_available(available) {}
        void operator()() {m_designator_wnd->ToggleAvailabilitly(m_available);}
        BuildDesignatorWnd* const m_designator_wnd;
        const bool m_available; // true: toggle whether to show available techs; false: toggle whether to show unavailable techs
    };
}

//////////////////////////////////////////////////
// BuildDesignatorWnd::BuildSelector
//////////////////////////////////////////////////
class BuildDesignatorWnd::BuildSelector : public CUIWnd
{
public:
    /** \name Structors */ //@{
    BuildSelector(GG::X w, GG::Y h);
    //@}

    /** \name Accessors */ //@{
    /** returns set of BulldType shown in this selector */
    const std::set<BuildType>&   GetBuildTypesShown() const;

    /** .first -> available items; .second -> unavailable items */
    const std::pair<bool, bool>& GetAvailabilitiesShown() const;
    //@}

    /** \name Mutators */ //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void    LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MinimizeClicked();

    /** Sets build location for this selector, which may be used to filter
      * items in the list or enable / disable them at some point in the
      * future. */
    void    SetBuildLocation(int location_id, bool refresh_list = true);

    /** Sets id of empire (or ALL_EMPIRES) for which to show items in this
      * BuildSelector. */
    void    SetEmpireID(int empire_id = ALL_EMPIRES, bool refresh_list = true);

    /** Clear and refill list of buildable items, according to current
      * filter settings. */
    void    Refresh();

    /** Show or hide indicated types of buildable items */
    void    ShowType(BuildType type, bool refresh_list = true);
    void    ShowAllTypes(bool refresh_list = true);
    void    HideType(BuildType type, bool refresh_list = true);
    void    HideAllTypes(bool refresh_list = true);

    /** Show or hide indicated availabilities of buildable items.  Available
      * items are those which have been unlocked for this selector's emipre. */
    void    ShowAvailability(bool available, bool refresh_list = true);
    void    HideAvailability(bool available, bool refresh_list = true);
    //@}

    mutable boost::signal<void (const BuildingType*)>                   DisplayBuildingTypeSignal;
    mutable boost::signal<void (BuildType, const std::string&, int)>    RequestNamedBuildItemSignal;
    mutable boost::signal<void (const ShipDesign*)>                     DisplayShipDesignSignal;
    mutable boost::signal<void (BuildType, int, int)>                   RequestIDedBuildItemSignal;

private:
    typedef std::map<
        GG::ListBox::iterator,
        BuildType,
        GG::ListBox::RowPtrIteratorLess<GG::ListBox>
     > RowToBuildTypeMap;

    static const GG::X TEXT_MARGIN_X;
    static const GG::Y TEXT_MARGIN_Y;

    void                DoLayout();

    bool    BuildableItemVisible(BuildType build_type, const std::string& name);
    bool    BuildableItemVisible(BuildType build_type, int design_id);

    /** Clear and refill list of buildable items, according to current
      * filter settings. */
    void    PopulateList();

    /** respond to the user single-click to select item on the queue */
    void    BuildItemSelected(const GG::ListBox::SelectionSet& selections);

    /** respond to the user double-clicking an item on the queue */
    void    BuildItemDoubleClicked(GG::ListBox::iterator it);

    std::map<BuildType, CUIButton*>         m_build_type_buttons;
    std::vector<CUIButton*>                 m_availability_buttons;

    std::set<BuildType>                     m_build_types_shown;
    std::pair<bool, bool>                   m_availabilities_shown; //!< .first -> available items; .second -> unavailable items

    BuildableItemsListBox*                  m_buildable_items;
    RowToBuildTypeMap                       m_build_types;
    GG::Pt                                  m_original_ul;

    int                                     m_build_location;
    int                                     m_empire_id;

    mutable boost::signals::connection      m_empire_ship_designs_changed_signal;

    friend class BuildDesignatorWnd;        // so BuildDesignatorWnd can access buttons
};
const GG::X BuildDesignatorWnd::BuildSelector::TEXT_MARGIN_X(3);
const GG::Y BuildDesignatorWnd::BuildSelector::TEXT_MARGIN_Y(3);

BuildDesignatorWnd::BuildSelector::BuildSelector(GG::X w, GG::Y h) :
    CUIWnd(UserString("PRODUCTION_WND_BUILD_ITEMS_TITLE"), GG::X1, GG::Y1, w - 1, h - 1,
           GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP),
    m_buildable_items(new BuildableItemsListBox(GG::X0, GG::Y0, GG::X1, GG::Y1)),
    m_build_types(GG::ListBox::RowPtrIteratorLess<GG::ListBox>(m_buildable_items)),
    m_build_location(INVALID_OBJECT_ID),
    m_empire_id(ALL_EMPIRES)
{
    // create build type toggle buttons (ship, building, all)
    m_build_type_buttons[BT_BUILDING] = new CUIButton(GG::X0, GG::Y0, GG::X1, UserString("PRODUCTION_WND_CATEGORY_BT_BUILDING"));
    AttachChild(m_build_type_buttons[BT_BUILDING]);
    m_build_type_buttons[BT_SHIP] = new CUIButton(GG::X0, GG::Y0, GG::X1, UserString("PRODUCTION_WND_CATEGORY_BT_SHIP"));
    AttachChild(m_build_type_buttons[BT_SHIP]);
    m_build_type_buttons[NUM_BUILD_TYPES] = new CUIButton(GG::X0, GG::Y0, GG::X1, UserString("ALL"));
    AttachChild(m_build_type_buttons[NUM_BUILD_TYPES]);

    // create availability toggle buttons (available, not available)
    m_availability_buttons.push_back(new CUIButton(GG::X0, GG::Y0, GG::X1, UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE")));
    AttachChild(m_availability_buttons.back());
    m_availability_buttons.push_back(new CUIButton(GG::X0, GG::Y0, GG::X1, UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE")));
    AttachChild(m_availability_buttons.back());

    // selectable list of buildable items
    AttachChild(m_buildable_items);
    GG::Connect(m_buildable_items->SelChangedSignal,
                &BuildDesignatorWnd::BuildSelector::BuildItemSelected, this);
    GG::Connect(m_buildable_items->DoubleClickedSignal,
                &BuildDesignatorWnd::BuildSelector::BuildItemDoubleClicked, this);
    m_buildable_items->SetStyle(GG::LIST_NOSORT | GG::LIST_SINGLESEL);


    //GG::ListBox::Row* header = new GG::ListBox::Row();
    //boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
    //GG::Clr clr = ClientUI::TextColor();
    //header->push_back("item", font, clr);
    //header->push_back("PP/turn", font, clr);
    //header->push_back("turns", font, clr);
    //header->push_back("description", font, clr);
    //header->SetColWidths(col_widths);
    //m_buildable_items->SetColHeaders(header);


    //m_buildable_items->SetNumCols(static_cast<int>(col_widths.size()));
    //m_buildable_items->LockColWidths();

    //for (unsigned int i = 0; i < col_widths.size(); ++i) {
    //    m_buildable_items->SetColWidth(i, col_widths[i]);
    //    m_buildable_items->SetColAlignment(i, GG::ALIGN_LEFT);
    //}

    DoLayout();
}

const std::set<BuildType>& BuildDesignatorWnd::BuildSelector::GetBuildTypesShown() const
{return m_build_types_shown; }

const std::pair<bool, bool>& BuildDesignatorWnd::BuildSelector::GetAvailabilitiesShown() const
{ return m_availabilities_shown; }

void BuildDesignatorWnd::BuildSelector::DoLayout() {
    int num_buttons = std::max(1, static_cast<int>(m_build_type_buttons.size() + m_availability_buttons.size()));
    GG::X x(0);
    GG::X button_width = ClientWidth() / num_buttons;
    GG::Y button_height(20);
    for (unsigned int i = 0; i < m_build_type_buttons.size() - 1; ++i) {
        m_build_type_buttons[BuildType(BT_BUILDING + i)]->SizeMove(GG::Pt(x, GG::Y0), GG::Pt(x + button_width, button_height));
        x += button_width;
    }
    m_build_type_buttons[NUM_BUILD_TYPES]->SizeMove(GG::Pt(x, GG::Y0), GG::Pt(x + button_width, button_height)); x += button_width;

    m_availability_buttons[0]->SizeMove(GG::Pt(x, GG::Y0), GG::Pt(x + button_width, button_height)); x += button_width;
    m_availability_buttons[1]->SizeMove(GG::Pt(x, GG::Y0), GG::Pt(x + button_width, button_height));

    m_buildable_items->SizeMove(GG::Pt(GG::X0, button_height),
                                ClientSize() - GG::Pt(GG::X0, GG::Y(INNER_BORDER_ANGLE_OFFSET)));
}

void BuildDesignatorWnd::BuildSelector::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    // maybe later do something interesting with docking
    GG::Wnd::SizeMove(ul, lr);

    if (Visible() && old_size != GG::Wnd::Size())
        DoLayout();
}

void BuildDesignatorWnd::BuildSelector::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys) {
    if (m_drag_offset != GG::Pt(-GG::X1, -GG::Y1)) {  // resize-dragging
        GG::Pt new_lr = pt - m_drag_offset;

        // constrain to within parent
        if (GG::Wnd* parent = Parent()) {
            GG::Pt max_lr = parent->ClientLowerRight();
            new_lr.x = std::min(new_lr.x, max_lr.x);
            new_lr.y = std::min(new_lr.y, max_lr.y);
        }        

        Resize(new_lr - UpperLeft());
    } else {    // normal-dragging
        GG::Pt final_move = move;

        if (GG::Wnd* parent = Parent()) {
            GG::Pt ul = UpperLeft(), lr = LowerRight();
            GG::Pt new_ul = ul + move, new_lr = lr + move;

            GG::Pt min_ul = parent->ClientUpperLeft() + GG::Pt(GG::X1, GG::Y1);
            GG::Pt max_lr = parent->ClientLowerRight();
            GG::Pt max_ul = max_lr - this->Size();

            new_ul.x = std::max(min_ul.x, std::min(max_ul.x, new_ul.x));
            new_ul.y = std::max(min_ul.y, std::min(max_ul.y, new_ul.y));

            final_move = new_ul - ul;
        }

        GG::Wnd::LDrag(pt, final_move, mod_keys);
    }
}

void BuildDesignatorWnd::BuildSelector::MinimizeClicked() {
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
            m_minimize_button->MoveTo(GG::Pt(button_ul.x - (m_close_button ? BUTTON_RIGHT_OFFSET : GG::X0), button_ul.y));
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
            m_minimize_button->MoveTo(GG::Pt(button_ul.x - (m_close_button ? BUTTON_RIGHT_OFFSET : GG::X0), button_ul.y));
        Show();
    }
}

void BuildDesignatorWnd::BuildSelector::SetBuildLocation(int location_id, bool refresh_list) {
    //std::cout << "BuildDesignatorWnd::BuildSelector::SetBuildLocation(" << location_id << ")" << std::endl;
    if (m_build_location != location_id) {
        m_build_location = location_id;
        if (refresh_list)
            Refresh();
    }
}

void BuildDesignatorWnd::BuildSelector::SetEmpireID(int empire_id/* = ALL_EMPIRES*/, bool refresh_list/* = true*/) {
    if (empire_id == m_empire_id)
        return;

    m_empire_id = empire_id;
    if (refresh_list) {
         Refresh();
    } else {
        // ensure signal connection set up properly, without actually
        // repopulating the list, as would be dine in Refresh()
        m_empire_ship_designs_changed_signal.disconnect();
        if (const Empire* empire = Empires().Lookup(m_empire_id))
            m_empire_ship_designs_changed_signal = GG::Connect(empire->ShipDesignsChangedSignal,
                                                               &BuildDesignatorWnd::BuildSelector::Refresh,
                                                               this,
                                                               boost::signals::at_front);
    }
}

void BuildDesignatorWnd::BuildSelector::Refresh() {
    Logger().debugStream() << "BuildDesignatorWnd::BuildSelector::Refresh()";
    m_empire_ship_designs_changed_signal.disconnect();
    if (const Empire* empire = Empires().Lookup(m_empire_id))
        m_empire_ship_designs_changed_signal = GG::Connect(empire->ShipDesignsChangedSignal,
                                                           &BuildDesignatorWnd::BuildSelector::Refresh,
                                                           this,
                                                           boost::signals::at_front);
    PopulateList();
}

void BuildDesignatorWnd::BuildSelector::ShowType(BuildType type, bool refresh_list) {
    if (m_build_types_shown.find(type) == m_build_types_shown.end()) {
        m_build_types_shown.insert(type);
        if (refresh_list)
            Refresh();
    }
}

void BuildDesignatorWnd::BuildSelector::HideType(BuildType type, bool refresh_list) {
    std::set<BuildType>::iterator it = m_build_types_shown.find(type);
    if (it != m_build_types_shown.end()) {
        m_build_types_shown.erase(it);
        if (refresh_list)
            Refresh();
    }
}

void BuildDesignatorWnd::BuildSelector::ShowAllTypes(bool refresh_list) {
    m_build_types_shown.insert(BT_BUILDING);
    m_build_types_shown.insert(BT_SHIP);
    if (refresh_list)
        Refresh();
}

void BuildDesignatorWnd::BuildSelector::HideAllTypes(bool refresh_list) {
    m_build_types_shown.clear();
    if (refresh_list)
        Refresh();
}

void BuildDesignatorWnd::BuildSelector::ShowAvailability(bool available, bool refresh_list) {
    if (available) {
        if (!m_availabilities_shown.first) {
            m_availabilities_shown.first = true;
            if (refresh_list)
                Refresh();
        }
    } else {
        if (!m_availabilities_shown.second) {
            m_availabilities_shown.second = true;
            if (refresh_list)
                Refresh();
        }
    }
}

void BuildDesignatorWnd::BuildSelector::HideAvailability(bool available, bool refresh_list) {
    if (available) {
        if (m_availabilities_shown.first) {
            m_availabilities_shown.first = false;
            if (refresh_list)
                Refresh();
        }
    } else {
        if (m_availabilities_shown.second) {
            m_availabilities_shown.second = false;
            if (refresh_list)
                Refresh();
        }
    }
}

bool BuildDesignatorWnd::BuildSelector::BuildableItemVisible(BuildType build_type, const std::string& name) {
    if (build_type != BT_BUILDING)
        throw std::invalid_argument("BuildableItemVisible was passed an invalid build type with a name");

    if (m_build_types_shown.find(build_type) == m_build_types_shown.end())
        return false;

    const BuildingType* building_type = GetBuildingType(name);
    if (!building_type || !building_type->Producible())
        return false;

    const Empire* empire = Empires().Lookup(m_empire_id);
    if (!empire)
        return true;

    bool available = false;
    if (build_type == BT_BUILDING)
        available = empire->BuildingTypeAvailable(name);

    if (available)
        return m_availabilities_shown.first;
    else
        return m_availabilities_shown.second;
}

bool BuildDesignatorWnd::BuildSelector::BuildableItemVisible(BuildType build_type, int design_id) {
    if (build_type != BT_SHIP)
        throw std::invalid_argument("BuildableItemVisible was passed an invalid build type with an id");

    if (m_build_types_shown.find(build_type) == m_build_types_shown.end())
        return false;

    const ShipDesign* design = GetShipDesign(design_id);
    if (!design || !design->Producible())
        return false;

    const Empire* empire = Empires().Lookup(m_empire_id);
    if (!empire)
        return true;

    bool available = false;
    if (build_type == BT_SHIP)
        available = empire->ShipDesignAvailable(design_id);

    if (available)
        return m_availabilities_shown.first;
    else
        return m_availabilities_shown.second;
}

void BuildDesignatorWnd::BuildSelector::PopulateList() {
    //std::cout << "BuildDesignatorWnd::BuildSelector::PopulateList start" << std::endl;
    Empire* empire = Empires().Lookup(m_empire_id);
    if (!empire)
        return;

    // keep track of initially selected row, so that new rows added may be compared to it to see if they should be selected after repopulating
    std::string selected_row;
    if (m_buildable_items->Selections().size() == 1)
        selected_row = (**m_buildable_items->Selections().begin())->DragDropDataType();


    m_buildable_items->Clear(); // the list of items to be populated
    m_build_types.clear();      // map from row to BuildType


    boost::shared_ptr<GG::Font> default_font = ClientUI::GetFont();
    const GG::Pt row_size = m_buildable_items->ListRowSize();

    GG::ListBox::iterator row_to_select = m_buildable_items->end(); // may be set while populating - used to reselect previously selected row after populating
    int i = 0;              // counter that keeps track of how many rows have been added so far


    // populate list with building types
    Logger().debugStream() << "BuildDesignatorWnd::BuildSelector::PopulateList() : Adding Buildings ";
    if (m_build_types_shown.find(BT_BUILDING) != m_build_types_shown.end()) {
        BuildingTypeManager& manager = GetBuildingTypeManager();

        for (BuildingTypeManager::iterator it = manager.begin(); it != manager.end(); ++it, ++i) {
            const std::string name = it->first;

            if (!BuildableItemVisible(BT_BUILDING, name)) continue;

            ProductionItemRow* item_row = new ProductionItemRow(row_size.x, row_size.y, name,
                                                                m_empire_id, m_build_location);

            GG::ListBox::iterator row_it = m_buildable_items->Insert(item_row);
            m_build_types[row_it] = BT_BUILDING;
            if (item_row->DragDropDataType() == selected_row)
                row_to_select = row_it;

            item_row->Resize(row_size);
        }
    }
    // populate with ship designs
    Logger().debugStream() << "BuildDesignatorWnd::BuildSelector::PopulateList() : Adding ship designs";
    if (m_build_types_shown.find(BT_SHIP) != m_build_types_shown.end()) {
        // get ids of designs to show... for specific empire, or for all empires
        std::vector<int> design_ids;
        if (empire)
            for (Empire::ShipDesignItr it = empire->ShipDesignBegin(); it != empire->ShipDesignEnd(); ++it)
                design_ids.push_back(*it);
        else
            for (Universe::ship_design_iterator it = GetUniverse().beginShipDesigns(); it != GetUniverse().endShipDesigns(); ++it)
                design_ids.push_back(it->first);

        for (std::vector<int>::const_iterator it = design_ids.begin(); it != design_ids.end(); ++it, ++i) {
            int ship_design_id = *it;

            if (!BuildableItemVisible(BT_SHIP, ship_design_id)) continue;

            const ShipDesign* ship_design = GetShipDesign(ship_design_id);
            if (!ship_design) continue;

            // add build item panel

            ProductionItemRow* item_row = new ProductionItemRow(row_size.x, row_size.y, ship_design_id,
                                                                m_empire_id, m_build_location);

            GG::ListBox::iterator row_it = m_buildable_items->Insert(item_row);
            m_build_types[row_it] = BT_SHIP;
            if (item_row->DragDropDataType() == selected_row)
                row_to_select = row_it;

            item_row->Resize(row_size);
        }
    }

    Logger().debugStream() << "Selecting Row";
    if (row_to_select != m_buildable_items->end()) {
        m_buildable_items->SelectRow(row_to_select);
        BuildItemSelected(m_buildable_items->Selections());
    }
    Logger().debugStream() << "Done";
}

void BuildDesignatorWnd::BuildSelector::BuildItemSelected(const GG::ListBox::SelectionSet& selections) {
    if (selections.size() != 1)
        return;
    GG::ListBox::iterator row = *selections.begin();
    BuildType build_type = m_build_types[row];
    const std::string& dddt = (*row)->DragDropDataType();

    if (build_type == BT_BUILDING) {
        const BuildingType* building_type = GetBuildingType(dddt);
        if (!building_type) {
            Logger().errorStream() << "BuildDesignatorWnd::BuildSelector::BuildItemSelected unable to get building type: " << dddt;
            return;
        }
        DisplayBuildingTypeSignal(building_type);

    } else if (build_type == BT_SHIP) {
        int design_id = ShipDesign::INVALID_DESIGN_ID;
        try {
            design_id = boost::lexical_cast<int>(dddt);
        } catch (...) {
            Logger().errorStream() << "BuildDesignatorWnd::BuildSelector::BuildItemSelected unable to cast row drag drop data type (" << dddt << ") to design id integer";
            return;
        }
        const ShipDesign* design = GetShipDesign(design_id);
        if (!design) {
            Logger().errorStream() << "BuildDesignatorWnd::BuildSelector::BuildItemSelected unable to find design with id " << design_id;
            return;
        }
        DisplayShipDesignSignal(design);
    }
}

void BuildDesignatorWnd::BuildSelector::BuildItemDoubleClicked(GG::ListBox::iterator it) {
    //std::cout << "BuildDesignatorWnd::BuildSelector::BuildItemDoubleClicked" << std::endl;
    if ((*it)->Disabled())
        return;
    BuildType build_type = m_build_types[it];
    if (build_type == BT_BUILDING)
        RequestNamedBuildItemSignal(BT_BUILDING, (*it)->DragDropDataType(), 1);
    else if (build_type == BT_SHIP)
        RequestIDedBuildItemSignal(BT_SHIP, boost::lexical_cast<int>((*it)->DragDropDataType()), 1);
}

//////////////////////////////////////////////////
// BuildDesignatorWnd
//////////////////////////////////////////////////
BuildDesignatorWnd::BuildDesignatorWnd(GG::X w, GG::Y h) :
    Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE | GG::ONTOP)
{
    const GG::X SIDEPANEL_WIDTH =       GG::X(GetOptionsDB().Get<int>("UI.sidepanel-width"));
    const GG::X CHILD_WIDTHS =          w - SIDEPANEL_WIDTH;
    const GG::Y DETAIL_PANEL_HEIGHT =   TechTreeWnd::NAVIGATOR_AND_DETAIL_HEIGHT;
    const GG::Y BUILD_SELECTOR_HEIGHT = DETAIL_PANEL_HEIGHT;

    m_enc_detail_panel = new EncyclopediaDetailPanel(CHILD_WIDTHS, DETAIL_PANEL_HEIGHT);

    m_side_panel = new SidePanel(Width() - SIDEPANEL_WIDTH, GG::Y0, Height());
    m_side_panel->EnableSelection();
    m_side_panel->Hide();

    m_build_selector = new BuildSelector(CHILD_WIDTHS, BUILD_SELECTOR_HEIGHT);
    m_build_selector->MoveTo(GG::Pt(GG::X0, h - BUILD_SELECTOR_HEIGHT));


    GG::Connect(m_build_selector->DisplayBuildingTypeSignal,    &EncyclopediaDetailPanel::SetItem,          m_enc_detail_panel);
    GG::Connect(m_build_selector->DisplayShipDesignSignal,      &EncyclopediaDetailPanel::SetItem,          m_enc_detail_panel);
    GG::Connect(m_build_selector->RequestNamedBuildItemSignal,  &BuildDesignatorWnd::BuildItemRequested,    this);
    GG::Connect(m_build_selector->RequestIDedBuildItemSignal,   &BuildDesignatorWnd::BuildItemRequested,    this);

    GG::Connect(m_side_panel->PlanetSelectedSignal, PlanetSelectedSignal);
    GG::Connect(m_side_panel->SystemSelectedSignal, SystemSelectedSignal);


    // connect build type button clicks to update display
    GG::Connect(m_build_selector->m_build_type_buttons[BT_BUILDING]->ClickedSignal,     ToggleBuildTypeFunctor(this, BT_BUILDING));
    GG::Connect(m_build_selector->m_build_type_buttons[BT_SHIP]->ClickedSignal,         ToggleBuildTypeFunctor(this, BT_SHIP));
    GG::Connect(m_build_selector->m_build_type_buttons[NUM_BUILD_TYPES]->ClickedSignal, ToggleAllBuildTypesFunctor( this));    // last button should be "All" button

    // connect availability button clicks to update display
    GG::Connect(m_build_selector->m_availability_buttons.at(0)->ClickedSignal, ToggleAvailabilityFunctor(this, true));    // available items
    GG::Connect(m_build_selector->m_availability_buttons.at(1)->ClickedSignal, ToggleAvailabilityFunctor(this, false));   // UNavailable items

    AttachChild(m_enc_detail_panel);
    AttachChild(m_build_selector);
    AttachChild(m_side_panel);

    MoveChildUp(m_enc_detail_panel);
    MoveChildUp(m_build_selector);

    Clear();
}

const std::set<BuildType>& BuildDesignatorWnd::GetBuildTypesShown() const
{ return m_build_selector->GetBuildTypesShown(); }

const std::pair<bool, bool>& BuildDesignatorWnd::GetAvailabilitiesShown() const
{ return m_build_selector->GetAvailabilitiesShown(); }

bool BuildDesignatorWnd::InWindow(const GG::Pt& pt) const
{ return m_enc_detail_panel->InWindow(pt) || m_build_selector->InWindow(pt) || m_side_panel->InWindow(pt); }

bool BuildDesignatorWnd::InClient(const GG::Pt& pt) const
{ return m_enc_detail_panel->InClient(pt) || m_build_selector->InClient(pt) || m_side_panel->InClient(pt); }

void BuildDesignatorWnd::CenterOnBuild(int queue_idx) {
    SetBuild(queue_idx);

    const ObjectMap& objects = GetUniverse().Objects();

    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = Empires().Lookup(empire_id);
    if (!empire) {
        Logger().errorStream() << "BuildDesignatorWnd::CenterOnBuild couldn't get empire with id " << empire_id;
        return;
    }

    const ProductionQueue& queue = empire->GetProductionQueue();
    if (0 <= queue_idx && queue_idx < static_cast<int>(queue.size())) {
        int location_id = queue[queue_idx].location;
        if (const UniverseObject* build_location = objects.Object(location_id)) {
            // centre map on system of build location
            int system_id = build_location->SystemID();
            MapWnd* map = ClientUI::GetClientUI()->GetMapWnd();
            map->CenterOnObject(system_id);
        }
    }
}

void BuildDesignatorWnd::SetBuild(int queue_idx) {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = Empires().Lookup(empire_id);
    if (!empire) {
        Logger().errorStream() << "BuildDesignatorWnd::SetBuild couldn't get empire with id " << empire_id;
        return;
    }
    const ProductionQueue& queue = empire->GetProductionQueue();
    if (0 <= queue_idx && queue_idx < static_cast<int>(queue.size())) {
        BuildType buildType = queue[queue_idx].item.build_type;
        if (buildType == BT_BUILDING) {
            const BuildingType* building_type = GetBuildingType(queue[queue_idx].item.name);
            assert(building_type);
            m_build_selector->DisplayBuildingTypeSignal(building_type);
        } else if (buildType == BT_SHIP) {
            const ShipDesign* design = GetShipDesign(boost::lexical_cast<int>(queue[queue_idx].item.design_id));
            assert(design);
            m_build_selector->DisplayShipDesignSignal(design);
        }
    } else {
            m_enc_detail_panel->OnUp();
    }
    m_enc_detail_panel->Refresh();
}

void BuildDesignatorWnd::SelectSystem(int system_id) {
    //std::cout << "BuildDesignatorWnd::SelectSystem(" << system_id << ")" << std::endl;

    if (system_id == SidePanel::SystemID()) {
        // don't need to do anything.  already showing the requested system.
        return;
    }

    if (system_id != INVALID_OBJECT_ID) {
        // set sidepanel's system and autoselect a suitable planet
        SidePanel::SetSystem(system_id);
        SelectDefaultPlanet();
    }
}

void BuildDesignatorWnd::SelectPlanet(int planet_id) {
    //std::cout << "BuildDesignatorWnd::SelectPlanet(" << planet_id << ")" << std::endl;
    SidePanel::SelectPlanet(planet_id);
    if (planet_id != INVALID_OBJECT_ID)
        m_system_default_planets[SidePanel::SystemID()] = planet_id;
    m_build_selector->SetBuildLocation(this->BuildLocation());
}

void BuildDesignatorWnd::Refresh() {
    m_build_selector->SetEmpireID(HumanClientApp::GetApp()->EmpireID(), false);
    Update();
}

void BuildDesignatorWnd::Update() {
    //std::cout << "BuildDesignatorWnd::Update()" << std::endl;
    SidePanel::Update();
    m_build_selector->Refresh();
    m_enc_detail_panel->Refresh();
}

void BuildDesignatorWnd::Reset() {
    //std::cout << "BuildDesignatorWnd::Reset()" << std::endl;
    SelectSystem(INVALID_OBJECT_ID);
    ShowAllTypes(false);            // show all types without populating the list
    HideAvailability(false, false); // hide unavailable items without populating the list
    ShowAvailability(true, false);  // show available items without populating the list
    m_build_selector->Refresh();
    m_enc_detail_panel->OnUp();
}

void BuildDesignatorWnd::Clear() {
    //std::cout << "BuildDesignatorWnd::Clear()" << std::endl;
    SidePanel::SetSystem(INVALID_OBJECT_ID);
    Reset();
    m_system_default_planets.clear();
}

void BuildDesignatorWnd::ShowType(BuildType type, bool refresh_list) {
    Logger().errorStream() << "BuildDesignatorWnd::ShowType(" << boost::lexical_cast<std::string>(type) << ")";
    if (type == BT_BUILDING || type == BT_SHIP) {
        m_build_selector->ShowType(type, refresh_list);
        m_build_selector->m_build_type_buttons[type]->MarkSelectedGray();
    } else {
        throw std::invalid_argument("BuildDesignatorWnd::ShowType was passed an invalid BuildType");
    }
}

void BuildDesignatorWnd::ShowAllTypes(bool refresh_list) {
    m_build_selector->ShowAllTypes(refresh_list);
    m_build_selector->m_build_type_buttons[BT_BUILDING]->MarkSelectedGray();
    m_build_selector->m_build_type_buttons[BT_SHIP]->MarkSelectedGray();
}

void BuildDesignatorWnd::HideType(BuildType type, bool refresh_list) {
    Logger().debugStream() << "BuildDesignatorWnd::HideType(" << boost::lexical_cast<std::string>(type) << ")";
    if (type == BT_BUILDING || type == BT_SHIP) {
        m_build_selector->HideType(type, refresh_list);
        m_build_selector->m_build_type_buttons[type]->MarkNotSelected();
    } else {
        throw std::invalid_argument("BuildDesignatorWnd::HideType was passed an invalid BuildType");
    }
}

void BuildDesignatorWnd::HideAllTypes(bool refresh_list) {
    m_build_selector->HideAllTypes(refresh_list);
    m_build_selector->m_build_type_buttons[BT_BUILDING]->MarkNotSelected();
    m_build_selector->m_build_type_buttons[BT_SHIP]->MarkNotSelected();
}

void BuildDesignatorWnd::ToggleType(BuildType type, bool refresh_list)
{
    if (type == BT_BUILDING || type == BT_SHIP) {
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
    if (types_shown.size() == NUM_BUILD_TYPES - 1)  // -1 because there are no buttons for BuildType::BT_NOT_BUILDING
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

void BuildDesignatorWnd::ShowBuildingTypeInEncyclopedia(const std::string& building_type)
{
    m_enc_detail_panel->SetBuildingType(building_type);
}

void BuildDesignatorWnd::ShowShipDesignInEncyclopedia(int design_id)
{
    m_enc_detail_panel->SetDesign(design_id);
}

int BuildDesignatorWnd::BuildLocation() const
{
    return m_side_panel->SelectedPlanetID();
}

void BuildDesignatorWnd::BuildItemRequested(BuildType build_type, const std::string& item, int num_to_build)
{
    //std::cout << "BuildDesignatorWnd::BuildItemRequested item name: " << item << std::endl;
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire && empire->BuildableItem(build_type, item, BuildLocation()))
        AddNamedBuildToQueueSignal(build_type, item, num_to_build, BuildLocation());
}

void BuildDesignatorWnd::BuildItemRequested(BuildType build_type, int design_id, int num_to_build)
{
    //std::cout << "BuildDesignatorWnd::BuildItemRequested design id: " << design_id << std::endl;
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire && empire->BuildableItem(build_type, design_id, BuildLocation()))
        AddIDedBuildToQueueSignal(build_type, design_id, num_to_build, BuildLocation());
}

void BuildDesignatorWnd::BuildQuantityChanged(int queue_idx, int quantity)
{
    BuildQuantityChangedSignal(queue_idx, quantity);
}

void BuildDesignatorWnd::SelectDefaultPlanet()
{
    int system_id = SidePanel::SystemID();
    if (system_id == INVALID_OBJECT_ID) {
        this->SelectPlanet(INVALID_OBJECT_ID);
        return;
    }


    // select recorded default planet for this system, if there is one recorded
    // unless that planet can't be selected or doesn't exist in this system
    std::map<int, int>::iterator it = m_system_default_planets.find(system_id);
    if (it != m_system_default_planets.end()) {
        int planet_id = it->second;
        if (m_side_panel->PlanetSelectable(planet_id)) {
            this->SelectPlanet(it->second);
            return;
        }
    }

    // couldn't reselect stored default, so need to find a reasonable other
    // planet to select.  attempt to find one owned by this client's player

    const System* sys = GetSystem(system_id);   // only checking visible objects for this clients empire (and not the latest known objects) as an empire shouldn't be able to use a planet or system it can't currently see as a production location
    if (!sys) {
        Logger().errorStream() << "BuildDesignatorWnd::SelectDefaultPlanet couldn't get system with id " << system_id;
        return;
    }

    std::vector<int> planets = sys->FindObjectIDs<Planet>();

    if (planets.empty()) {
        this->SelectPlanet(INVALID_OBJECT_ID);
        return;
    }


    bool found_planet = false;                              // was a suitable planet found?
    int best_planet_id = INVALID_OBJECT_ID; // id of selected planet
    double best_planet_pop = -99999.9;                      // arbitrary negative number, so any planet's pop will be better

    for (std::vector<int>::iterator it = planets.begin(); it != planets.end(); ++it) {
        const Planet* planet = GetPlanet(*it);
        if (!planet) {
            Logger().errorStream() << "BuildDesignatorWnd::SetDefaultPlanet couldn't get planet with id " << *it;
            continue;
        }
        int planet_id = planet->ID();
        if (!m_side_panel->PlanetSelectable(planet_id))
            continue;

        double planet_pop = planet->CurrentMeterValue(METER_POPULATION);
        if (planet_pop > best_planet_pop) {
            // found new planet to pick
            found_planet = true;
            best_planet_pop = planet_pop;
            best_planet_id = planet_id;
        }
    }

    // select top pop planet
    if (found_planet)
        this->SelectPlanet(best_planet_id);
}
