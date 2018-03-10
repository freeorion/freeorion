#include "GovernmentWnd.h"

#include "ClientUI.h"
#include "CUIWnd.h"
#include "CUIControls.h"
#include "IconTextBrowseWnd.h"
#include "TextBrowseWnd.h"
#include "../parse/Parse.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Order.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include "../util/Directories.h"
#include "../Empire/Empire.h"
#include "../Empire/Government.h"
#include "../client/human/HumanClientApp.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/TabWnd.h>

namespace {
    const std::string   POLIVY_CONTROL_DROP_TYPE_STRING = "Policy Control";
    const std::string   EMPTY_STRING = "";
    const std::string   DES_MAIN_WND_NAME = "government.edit";
    const std::string   DES_PART_PALETTE_WND_NAME = "government.policies";
    const GG::X         POLICY_CONTROL_WIDTH(54);
    const GG::Y         POLICY_CONTROL_HEIGHT(54);
    const GG::X         SLOT_CONTROL_WIDTH(60);
    const GG::Y         SLOT_CONTROL_HEIGHT(60);
    const int           PAD(3);

    /** Returns texture with which to render a SlotControl, depending on \a slot_type. */
    std::shared_ptr<GG::Texture> SlotBackgroundTexture(const std::string& category) {
        return ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "missing.png", true);
    }

    /** Returns background texture with which to render a PolicyControl, depending on the
      * types of slot that the indicated \a policy can be put into. */
    std::shared_ptr<GG::Texture> PolicyBackgroundTexture(const Policy* policy) {
        if (policy) {
            bool social = policy->Category() == "ECONOMIC_CATEGORY";
            bool economic = policy->Category() == "SOCIAL_CATEGORY";

            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ship_policies" / "core_policy.png", true);
        }
        return ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "missing.png", true);
    }
}

//////////////////////////////////////////////////
// PolicyControl                                //
//////////////////////////////////////////////////
/** UI representation of a government policy.  Displayed in the PolicyPalette,
  * and can be dragged onto SlotControls to add policies to the government. */
class PolicyControl : public GG::Control {
public:
    /** \name Structors */ //@{
    PolicyControl(const Policy* policy);
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    const std::string&  PolicyName() const  { return m_policy ? m_policy->Name() : EMPTY_STRING; }
    const Policy*       Policy() const      { return m_policy; }
    //@}

    /** \name Mutators */ //@{
    void Render() override;
    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;type);
    //@}

    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> ClickedSignal;
    mutable boost::signals2::signal<void (const Policy*, const GG::Pt& pt)> RightClickedSignal;
    mutable boost::signals2::signal<void (const Policy*)> DoubleClickedSignal;

private:
    std::shared_ptr<GG::StaticGraphic>  m_icon;
    std::shared_ptr<GG::StaticGraphic>  m_background;
    const Policy*                       m_policy;
};

PolicyControl::PolicyControl(const Policy* policy) :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE),
    m_icon(nullptr),
    m_background(nullptr),
    m_policy(policy)
{}

void PolicyControl::CompleteConstruction() {
    GG::Control::CompleteConstruction();
    if (!m_policy)
        return;

    m_background = GG::Wnd::Create<GG::StaticGraphic>(PolicyBackgroundTexture(m_policy), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_background->Resize(GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT));
    m_background->Show();
    AttachChild(m_background);


    // position of policy image centred within policy control.  control is size of a slot, but the
    // policy image is smaller
    GG::X policy_left = (Width() - PART_CONTROL_WIDTH) / 2;
    GG::Y policy_top = (Height() - PART_CONTROL_HEIGHT) / 2;

    //DebugLogger() << "PolicyControl::PolicyControl this: " << this << " policy: " << policy << " named: " << (policy ? policy->Name() : "no policy");
    m_icon = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::PolicyIcon(m_policy->Name()), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_icon->MoveTo(GG::Pt(policy_left, policy_top));
    m_icon->Resize(GG::Pt(PART_CONTROL_WIDTH, PART_CONTROL_HEIGHT));
    m_icon->Show();
    AttachChild(m_icon);

    SetDragDropDataType(POLICY_CONTROL_DROP_TYPE_STRING);

    //DebugLogger() << "PolicyControl::PolicyControl policy name: " << m_policy->Name();
    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
        ClientUI::PolicyIcon(m_policy->Name()),
        UserString(m_policy->Name()),
        UserString(m_policy->Description()) + "\n" + m_policy->CapacityDescription()
    ));
}

void PolicyControl::Render() {}

void PolicyControl::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ ClickedSignal(m_policy, mod_keys); }

void PolicyControl::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ DoubleClickedSignal(m_policy); }

void PolicyControl::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ RightClickedSignal(m_policy, pt); }


//////////////////////////////////////////////////
// PoliciesListBox                               //
//////////////////////////////////////////////////
/** Arrangement of PolicyControls that can be dragged onto SlotControls */
class PoliciesListBox : public CUIListBox {
public:
    class PoliciesListBoxRow : public CUIListBox::Row {
    public:
        PoliciesListBoxRow(GG::X w, GG::Y h);
        void ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds,
                                 const GG::Wnd* destination) override;
    };

    /** \name Structors */ //@{
    PoliciesListBox();
    //@}

    /** \name Accessors */ //@{
    const std::set<std::string>& GetCategoriesShown() const;
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    void AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                     GG::Flags<GG::ModKey> mod_keys) override;

    void Populate();

    void ShowCategory(const std::string& category, bool refresh_list = true);
    void ShowAllCategories(bool refresh_list = true);
    void HideCategory(const std::string& category, bool refresh_list = true);
    void HideAllCategories(bool refresh_list = true);
    //@}

    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> PolicyTypeClickedSignal;
    mutable boost::signals2::signal<void (const Policy*)> PolicyTypeDoubleClickedSignal;
    mutable boost::signals2::signal<void (const Policy*, const GG::Pt& pt)> PolicyTypeRightClickedSignal;

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    std::set<std::string>   m_policy_categories_shown;  // which policy categories should be shown
    int                     m_previous_num_columns;
};

PoliciesListBox::PoliciesListBoxRow::PoliciesListBoxRow(GG::X w, GG::Y h) :
    CUIListBox::Row(w, h, "")   // drag_drop_data_type = "" implies not draggable row
{}

void PoliciesListBox::PoliciesListBoxRow::ChildrenDraggedAway(
    const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination)
{
    if (wnds.empty())
        return;
    // should only be one wnd in list because PolicyControls doesn't allow selection, so dragging is
    // only one-at-a-time
    auto control = dynamic_cast<GG::Control*>(wnds.front());
    if (!control || empty())
        return;

    // find control in row
    unsigned int ii = 0;
    for (; ii < size(); ++ii) {
        if (at(ii) != control)
            continue;
    }

    if (ii == size())
        return;

    RemoveCell(ii);  // Wnd that accepts drop takes ownership of dragged-away control

    auto policy_control = dynamic_cast<PolicyControl*>(control);
    if (!policy_control)
        return;

    const auto policy_type = policy_control->Policy();
    if (!policy_type)
        return;

    auto new_policy_control = GG::Wnd::Create<PolicyControl>(policy_type);
    const auto parent = dynamic_cast<const PoliciesListBox*>(Parent().get());
    if (parent) {
        new_policy_control->ClickedSignal.connect(parent->PolicyTypeClickedSignal);
        new_policy_control->DoubleClickedSignal.connect(parent->PolicyTypeDoubleClickedSignal);
        new_policy_control->RightClickedSignal.connect(parent->PolicyTypeRightClickedSignal);
    }

    // set availability shown
    auto shown = m_availabilities_state.DisplayedPolicyAvailability(policy_type->Name());
    if (shown)
        new_policy_control->SetAvailability(*shown);

    SetCell(ii, new_policy_control);
}

PoliciesListBox::PoliciesListBox() :
    CUIListBox(),
    m_policy_categories_shown(),
    m_previous_num_columns(-1)
{
    ManuallyManageColProps();
    NormalizeRowsOnInsert(false);
    SetStyle(GG::LIST_NOSEL);
}

const std::set<const std::string&>& PoliciesListBox::GetCategoriesShown() const
{ return m_policy_categories_shown; }

void PoliciesListBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    // maybe later do something interesting with docking
    CUIListBox::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size()) {
        // determine how many columns can fit in the box now...
        const GG::X TOTAL_WIDTH = Size().x - ClientUI::ScrollWidth();
        const int NUM_COLUMNS = std::max(1,
            Value(TOTAL_WIDTH / (SLOT_CONTROL_WIDTH + GG::X(PAD))));

        if (NUM_COLUMNS != m_previous_num_columns)
            Populate();
    }
}

void PoliciesListBox::AcceptDrops(const GG::Pt& pt,
                                 std::vector<std::shared_ptr<GG::Wnd>> wnds,
                                 GG::Flags<GG::ModKey> mod_keys)
{
    // If ctrl is pressed then signal all policies of the same type to be cleared.
    if (!(GG::GUI::GetGUI()->ModKeys() & GG::MOD_KEY_CTRL))
        return;

    if (wnds.empty())
        return;

    auto* control = boost::polymorphic_downcast<const PolicyControl*>(wnds.begin()->get());
    const Policy* policy_type = control ? control->Policy() : nullptr;
    if (!policy_type)
        return;

    ClearPolicySignal(policy_type->Name());
}

std::map<std::string, std::vector<const Policy*>>
PoliciesListBox::GroupAvailableDisplayablePolicies(const Empire* empire) {
    std::map<std::string, std::vector<const Policy*>> policies_categorized;

    // loop through all possible policies
    for (const auto& entry : GetPolicyManager()) {
        const auto& policy = entry.second;
        const auto& category = policy->Category();

        // check whether this policy should be shown in list
        if (m_policy_categories_shown.find(category) == m_policy_categories_shown.end())
            continue;   // policy of this class is not requested to be shown
        if (empire && !empire->AvailablePolicies().count(policy))
            continue;

        policies_categorized[category].push_back(policy);
    }
    return policies_categorized;
}

void PoliciesListBox::Populate() {
    ScopedTimer scoped_timer("PoliciesListBox::Populate");


    const GG::X TOTAL_WIDTH = ClientWidth() - ClientUI::ScrollWidth();
    const int NUM_COLUMNS = std::max(1, Value(TOTAL_WIDTH / (SLOT_CONTROL_WIDTH + GG::X(PAD))));

    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = GetEmpire(empire_id);  // may be nullptr

    int cur_col = NUM_COLUMNS;
    std::shared_ptr<PoliciesListBoxRow> cur_row;
    int num_policies = 0;

    // remove policies currently in rows of listbox
    Clear();

    /// filter policies by availability and current designation of categories
    // for display
    auto cats_policies = GroupAvailableDisplayablePolicies(empire);

    // get empire id and location to use for cost and time comparisons
    int loc_id = INVALID_OBJECT_ID;
    if (empire) {
        auto location = GetUniverseObject(empire->CapitalID());
        loc_id = location ? location->ID() : INVALID_OBJECT_ID;
    }

    for (auto& cat : cats_policies) {
        // take the sorted policies and make UI element rows for the PoliciesListBox
        for (const auto* policy: cat.second) {
            // check if current row is full, and make a new row if necessary
            if (cur_col >= NUM_COLUMNS) {
                if (cur_row)
                    Insert(cur_row);
                cur_col = 0;
                cur_row = GG::Wnd::Create<PoliciesListBoxRow>(
                    TOTAL_WIDTH, SLOT_CONTROL_HEIGHT + GG::Y(PAD));
            }
            ++cur_col;
            ++num_policies;

            // make new policy control and add to row
            auto control = GG::Wnd::Create<PolicyControl>(policy);
            control->ClickedSignal.connect(PoliciesListBox::PolicyTypeClickedSignal);
            control->DoubleClickedSignal.connect(PoliciesListBox::PolicyTypeDoubleClickedSignal);
            control->RightClickedSignal.connect(PoliciesListBox::PolicyTypeRightClickedSignal);

            cur_row->push_back(control);
        }
    }
    // add any incomplete rows
    if (cur_row)
        Insert(cur_row);

    // keep track of how many columns are present now
    m_previous_num_columns = NUM_COLUMNS;

    // If there are no policies add a prompt to suggest a solution.
    if (num_policies == 0)
        Insert(GG::Wnd::Create<PromptRow>(TOTAL_WIDTH,
                                          UserString("ALL_AVAILABILITY_FILTERS_BLOCKING_PROMPT")),
               begin(), false);
}

void PoliciesListBox::ShowCategory(const std::string& category, bool refresh_list = true) {
    if (m_policy_categories_shown.find(category) == m_policy_categories_shown.end()) {
        m_policy_categories_shown.insert(category);
        if (refresh_list)
            Populate();
    }
}

void PoliciesListBox::ShowAllCategoryes(bool refresh_list) {
    for (const auto& category : GetPolicyManager().PolicyCategories())
        m_policy_categories_shown.insert(category);
    if (refresh_list)
        Populate();
}

void PoliciesListBox::HideCategory(const std::string& category, bool refresh_list) {
    auto it = m_policy_categories_shown.find(category);
    if (it != m_policy_categories_shown.end()) {
        m_policy_categories_shown.erase(it);
        if (refresh_list)
            Populate();
    }
}

void PoliciesListBox::HideAllCategoryes(bool refresh_list) {
    m_policy_categories_shown.clear();
    if (refresh_list)
        Populate();
}


//////////////////////////////////////////////////
// GovernmentWnd::PolicyPalette                 //
//////////////////////////////////////////////////
/** Contains graphical list of PolicyControl which can be dragged and dropped
  * onto slots to assign policies to those slots */
class GovernmentWnd::PolicyPalette : public CUIWnd {
public:
    /** \name Structors */ //@{
    PolicyPalette(const std::string& config_name);
    void CompleteConstruction() override;
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void ShowCategory(const std::string& category, bool refresh_list = true);
    void ShowAllCategoryes(bool refresh_list = true);
    void HideCategory(const std::string& category, bool refresh_list = true);
    void HideAllCategoryes(bool refresh_list = true);
    void ToggleCategory(const std::string& category, bool refresh_list = true);
    void ToggleAllCategoryes(bool refresh_list = true);

    void Populate();
    //@}

    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> PolicyTypeClickedSignal;
    mutable boost::signals2::signal<void (const Policy*)> PolicyTypeDoubleClickedSignal;
    mutable boost::signals2::signal<void (const Policy*, const GG::Pt& pt)> PolicyTypeRightClickedSignal;
    mutable boost::signals2::signal<void (const std::string&)> ClearPolicySignal;

private:
    void DoLayout();

    /** A policy type click with ctrl obsoletes policy. */
    void HandlePolicyTypeClicked(const Policy*, GG::Flags<GG::ModKey>);
    void HandlePolicyTypeRightClicked(const Policy*, const GG::Pt& pt);

    std::shared_ptr<PoliciesListBox>                                m_policies_list;
    std::map<const std::string&, std::shared_ptr<CUIStateButton>>   m_class_buttons;
};

GovernmentWnd::PolicyPalette::PolicyPalette(const std::string& config_name) :
    CUIWnd(UserString("GOVERNMENT_WND_POLICY_PALETTE_TITLE"),
           GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE,
           config_name),
    m_policies_list(nullptr)
{}

void GovernmentWnd::PolicyPalette::CompleteConstruction() {
    SetChildClippingMode(ClipToClient);

    m_policies_list = GG::Wnd::Create<PoliciesListBox>(m_availabilities_state);
    AttachChild(m_policies_list);
    m_policies_list->PolicyTypeClickedSignal.connect(
        boost::bind(&GovernmentWnd::PolicyPalette::HandlePolicyTypeClicked, this, _1, _2));
    m_policies_list->PolicyTypeDoubleClickedSignal.connect(
        PolicyTypeDoubleClickedSignal);
    m_policies_list->PolicyTypeRightClickedSignal.connect(
        boost::bind(&GovernmentWnd::PolicyPalette::HandlePolicyTypeRightClicked, this, _1, _2));
    m_policies_list->ClearPolicySignal.connect(ClearPolicySignal);

    const PolicyTypeManager& policy_manager = GetPolicyTypeManager();

    // class buttons
    for (auto& category = GetPolicyManager().PolicyCategories()) {
        // are there any policies of this class?
        bool policy_of_this_class_exists = false;
        for (const auto& entry : policy_manager) {
            if (const auto& policy = entry.second) {
                if (policy->Category() == category) {
                    policy_of_this_class_exists = true;
                    break;
                }
            }
        }
        if (!policy_of_this_class_exists)
            continue;

        m_class_buttons[category] = GG::Wnd::Create<CUIStateButton>(UserString(boost::lexical_cast<std::string>(category)), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
        AttachChild(m_class_buttons[category]);
        m_class_buttons[category]->CheckedSignal.connect(
            boost::bind(&GovernmentWnd::PolicyPalette::ToggleCategory, this, category, true));
    }

    // default to showing nothing
    ShowAllCategories(false);
    Populate();

    CUIWnd::CompleteConstruction();

    DoLayout();
    SaveDefaultedOptions();
}

void GovernmentWnd::PolicyPalette::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    CUIWnd::SizeMove(ul, lr);
    DoLayout();
}

void GovernmentWnd::PolicyPalette::DoLayout() {
    const int PTS = ClientUI::Pts();
    const GG::X PTS_WIDE(PTS/2);         // guess at how wide per character the font needs
    const GG::Y  BUTTON_HEIGHT(PTS*3/2);
    const int BUTTON_SEPARATION = 3;    // vertical or horizontal sepration between adjacent buttons
    const int BUTTON_EDGE_PAD = 2;      // distance from edges of control to buttons
    const GG::X RIGHT_EDGE_PAD(8);       // to account for border of CUIWnd

    const GG::X USABLE_WIDTH = std::max(ClientWidth() - RIGHT_EDGE_PAD, GG::X1);   // space in which to fit buttons
    const int GUESSTIMATE_NUM_CHARS_IN_BUTTON_LABEL = 14;                   // rough guesstimate... avoid overly long policy class names
    const GG::X MIN_BUTTON_WIDTH = PTS_WIDE*GUESSTIMATE_NUM_CHARS_IN_BUTTON_LABEL;
    const int MAX_BUTTONS_PER_ROW = std::max(Value(USABLE_WIDTH / (MIN_BUTTON_WIDTH + BUTTON_SEPARATION)), 1);

    const int NUM_CLASS_BUTTONS = std::max(1, static_cast<int>(m_class_buttons.size()));
    const int NUM_SUPERFLUOUS_CULL_BUTTONS = 1;
    const int NUM_AVAILABILITY_BUTTONS = 3;
    const int NUM_NON_CLASS_BUTTONS = NUM_SUPERFLUOUS_CULL_BUTTONS + NUM_AVAILABILITY_BUTTONS;

    // determine whether to put non-class buttons (availability and redundancy)
    // in one column or two.
    // -> if class buttons fill up fewer rows than (the non-class buttons in one
    // column), split the non-class buttons into two columns
    int num_non_class_buttons_per_row = 1;
    if (NUM_CLASS_BUTTONS < NUM_NON_CLASS_BUTTONS*(MAX_BUTTONS_PER_ROW - num_non_class_buttons_per_row))
        num_non_class_buttons_per_row = 2;

    const int MAX_CLASS_BUTTONS_PER_ROW = std::max(1, MAX_BUTTONS_PER_ROW - num_non_class_buttons_per_row);

    const int NUM_CLASS_BUTTON_ROWS = static_cast<int>(std::ceil(static_cast<float>(NUM_CLASS_BUTTONS) / MAX_CLASS_BUTTONS_PER_ROW));
    const int NUM_CLASS_BUTTONS_PER_ROW = static_cast<int>(std::ceil(static_cast<float>(NUM_CLASS_BUTTONS) / NUM_CLASS_BUTTON_ROWS));

    const int TOTAL_BUTTONS_PER_ROW = NUM_CLASS_BUTTONS_PER_ROW + num_non_class_buttons_per_row;

    const GG::X BUTTON_WIDTH = (USABLE_WIDTH - (TOTAL_BUTTONS_PER_ROW - 1)*BUTTON_SEPARATION) / TOTAL_BUTTONS_PER_ROW;

    const GG::X COL_OFFSET = BUTTON_WIDTH + BUTTON_SEPARATION;    // horizontal distance between each column of buttons
    const GG::Y ROW_OFFSET = BUTTON_HEIGHT + BUTTON_SEPARATION;   // vertical distance between each row of buttons

    // place class buttons
    int col = NUM_CLASS_BUTTONS_PER_ROW;
    int row = -1;
    for (auto& entry : m_class_buttons) {
        if (col >= NUM_CLASS_BUTTONS_PER_ROW) {
            col = 0;
            ++row;
        }
        GG::Pt ul(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        entry.second->SizeMove(ul, lr);
        ++col;
    }

    // place policies list.  note: assuming at least as many rows of class buttons as availability buttons, as should
    //                          be the case given how num_non_class_buttons_per_row is determined
    m_policies_list->SizeMove(GG::Pt(GG::X0, BUTTON_EDGE_PAD + ROW_OFFSET*(row + 1)),
                           ClientSize() - GG::Pt(GG::X(BUTTON_SEPARATION), GG::Y(BUTTON_SEPARATION)));

    // place slot type buttons
    col = NUM_CLASS_BUTTONS_PER_ROW;
    row = 0;
    auto ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    auto lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_superfluous_policies_button->SizeMove(ul, lr);

    // a function to place availability buttons either in a single column below the
    // superfluous button or to complete a 2X2 grid left of the class buttons.
    auto place_avail_button_adjacent =
        [&col, &row, &num_non_class_buttons_per_row, NUM_CLASS_BUTTONS_PER_ROW,
         BUTTON_EDGE_PAD, COL_OFFSET, ROW_OFFSET, BUTTON_WIDTH, BUTTON_HEIGHT]
        (GG::Wnd* avail_btn)
        {
            if (num_non_class_buttons_per_row == 1)
                ++row;
            else {
                if (col >= NUM_CLASS_BUTTONS_PER_ROW + num_non_class_buttons_per_row - 1) {
                    col = NUM_CLASS_BUTTONS_PER_ROW - 1;
                    ++row;
                }
                ++col;
            }

            auto ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
            auto lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
            avail_btn->SizeMove(ul, lr);
        };

    //place availability buttons
    // TODO: C++17, Replace with structured binding auto [a, b, c] = m_availabilities;
    auto& m_obsolete_button = std::get<Availability::Obsolete>(m_availabilities_buttons);
    auto& m_available_button = std::get<Availability::Available>(m_availabilities_buttons);
    auto& m_unavailable_button = std::get<Availability::Future>(m_availabilities_buttons);

    place_avail_button_adjacent(m_obsolete_button.get());
    place_avail_button_adjacent(m_available_button.get());
    place_avail_button_adjacent(m_unavailable_button.get());
}

void GovernmentWnd::PolicyPalette::HandlePolicyTypeClicked(const Policy* policy_type, GG::Flags<GG::ModKey> modkeys) {
    // Toggle obsolete for a control click.
    if (modkeys & GG::MOD_KEY_CTRL) {
        auto& manager = GetCurrentDesignsManager();
        const auto obsolete = manager.IsPolicyObsolete(policy_type->Name());
        manager.SetPolicyObsolete(policy_type->Name(), !obsolete);

        PolicyObsolescenceChangedSignal();
        Populate();
    }
    else
        PolicyTypeClickedSignal(policy_type, modkeys);
}

void GovernmentWnd::PolicyPalette::HandlePolicyTypeRightClicked(const Policy* policy_type, const GG::Pt& pt) {
    // Context menu actions
    auto& manager = GetCurrentDesignsManager();
    const auto& policy_name = policy_type->Name();
    auto is_obsolete = manager.IsPolicyObsolete(policy_name);
    auto toggle_obsolete_design_action = [&manager, &policy_name, is_obsolete, this]() {
        manager.SetPolicyObsolete(policy_name, !is_obsolete);
        PolicyObsolescenceChangedSignal();
        Populate();
    };

    // create popup menu with a commands in it
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    const auto empire_id = HumanClientApp::GetApp()->EmpireID();
    if (empire_id != ALL_EMPIRES)
        popup->AddMenuItem(GG::MenuItem(
                               (is_obsolete
                                ? UserString("DESIGN_WND_UNOBSOLETE_PART")
                                : UserString("DESIGN_WND_OBSOLETE_PART")),
                               false, false, toggle_obsolete_design_action));

    popup->Run();

    PolicyTypeRightClickedSignal(policy_type, pt);
}

void GovernmentWnd::PolicyPalette::ShowCategory(const std::string& category, bool refresh_list) {
    if (category >= const std::string&(0) && category < NUM_SHIP_PART_CLASSES) {
        m_policies_list->ShowCategory(category, refresh_list);
        m_class_buttons[category]->SetCheck();
    } else {
        throw std::invalid_argument("PolicyPalette::ShowCategory was passed an invalid const std::string&");
    }
}

void GovernmentWnd::PolicyPalette::ShowAllCategoryes(bool refresh_list) {
    m_policies_list->ShowAllCategoryes(refresh_list);
    for (auto& entry : m_class_buttons)
        entry.second->SetCheck();
}

void GovernmentWnd::PolicyPalette::HideCategory(const std::string& category, bool refresh_list) {
    if (category >= const std::string&(0) && category < NUM_SHIP_PART_CLASSES) {
        m_policies_list->HideCategory(category, refresh_list);
        m_class_buttons[category]->SetCheck(false);
    } else {
        throw std::invalid_argument("PolicyPalette::HideCategory was passed an invalid const std::string&");
    }
}

void GovernmentWnd::PolicyPalette::HideAllCategoryes(bool refresh_list) {
    m_policies_list->HideAllCategoryes(refresh_list);
    for (auto& entry : m_class_buttons)
        entry.second->SetCheck(false);
}

void GovernmentWnd::PolicyPalette::ToggleCategory(const std::string& category, bool refresh_list) {
    if (category >= const std::string&(0) && category < NUM_SHIP_PART_CLASSES) {
        const auto& classes_shown = m_policies_list->GetCategoryesShown();
        if (classes_shown.find(category) == classes_shown.end())
            ShowCategory(category, refresh_list);
        else
            HideCategory(category, refresh_list);
    } else {
        throw std::invalid_argument("PolicyPalette::ToggleCategory was passed an invalid const std::string&");
    }
}

void GovernmentWnd::PolicyPalette::ToggleAllCategoryes(bool refresh_list)
{
    const auto& classes_shown = m_policies_list->GetCategoryesShown();
    if (classes_shown.size() == NUM_SHIP_PART_CLASSES)
        HideAllCategoryes(refresh_list);
    else
        ShowAllCategoryes(refresh_list);
}

void GovernmentWnd::PolicyPalette::Populate()
{ m_policies_list->Populate(); }


//////////////////////////////////////////////////
// SlotControl                                  //
//////////////////////////////////////////////////
/** UI representation and drop-target for policies of a government.
  * PolicyControl may be dropped into slots to add the corresponding policies to
  * the government, or the policy may be set programmatically with SetPolicy(). */
class SlotControl : public GG::Control {
public:
    /** \name Structors */ //@{
    SlotControl();
    explicit SlotControl(const std::string& slot_category);
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    const std::string&  SlotCategory() const;
    const Policy*       GetPolicy() const;
    //@}

    /** \name Mutators */ //@{
    void StartingChildDragDrop(const GG::Wnd* wnd, const GG::Pt& offset) override;
    void CancellingChildDragDrop(const std::vector<const GG::Wnd*>& wnds) override;
    void AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                     GG::Flags<GG::ModKey> mod_keys) override;
    void ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds,
                             const GG::Wnd* destination) override;
    void DragDropEnter(const GG::Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                       GG::Flags<GG::ModKey> mod_keys) override;
    void DragDropLeave() override;
    void Render() override;

    void Highlight(bool actually = true);

    void SetPolicy(const std::string& policy_name); //!< used to programmatically set the PolicyControl in this slot.  Does not emit signal
    void SetPolicy(const Policy* policy = nullptr); //!< used to programmatically set the PolicyControl in this slot.  Does not emit signal
    //@}

    /** emitted when the contents of a slot are altered by the dragging
      * a PolicyControl in or out of the slot.  signal should be caught and the
      * slot contents set using SetPolicy accordingly */
    mutable boost::signals2::signal<void (const Policy*, bool)> SlotContentsAlteredSignal;
    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> PolicyTypeClickedSignal;

protected:
    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event) override;
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    bool                                m_highlighted = false;
    std::string                         m_slot_category = "";
    std::shared_ptr<PolicyControl>      m_policy_control = nullptr;
    std::shared_ptr<GG::StaticGraphic>  m_background = nullptr;
};

SlotControl::SlotControl() :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE)
{}

SlotControl::SlotControl(const std::string& slot_category) :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE),
    m_slot_category(slot_category)
{}

void SlotControl::CompleteConstruction() {
    GG::Control::CompleteConstruction();

    m_background = GG::Wnd::Create<GG::StaticGraphic>(SlotBackgroundTexture(m_slot_category), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_background->Resize(GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT));
    m_background->Show();
    AttachChild(m_background);

    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
        SlotBackgroundTexture(m_slot_category),
        title_text,
        UserString("SL_TOOLTIP_DESC")
    ));
}

bool SlotControl::EventFilter(GG::Wnd* w, const GG::WndEvent& event) {
    if (w == this)
        return false;

    switch (event.Type()) {
    case GG::WndEvent::DragDropEnter:
    case GG::WndEvent::DragDropHere:
    case GG::WndEvent::CheckDrops:
    case GG::WndEvent::DragDropLeave:
    case GG::WndEvent::DragDroppedOn:
        HandleEvent(event);
        return true;
        break;
    default:
        return false;
    }
}

void SlotControl::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                  const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    // if more than one control dropped somehow, reject all
    if (std::distance(first, last) != 1)
        return;

    for (DropsAcceptableIter it = first; it != last; ++it) {
        if (it->first->DragDropDataType() != POLICY_CONTROL_DROP_TYPE_STRING)
            continue;
        const auto policy_control = boost::polymorphic_downcast<const PolicyControl* const>(it->first);
        const Policy* policy_type = policy_control->Policy();
        if (policy_type &&
            policy_type->CanMountInSlotCategory(m_slot_category) &&
            policy_control != m_policy_control.get())
        {
            it->second = true;
            return;
        }
    }
}

const std::string& SlotControl::SlotCategory() const
{ return m_slot_category; }

const Policy* SlotControl::GetPolicy() const {
    if (m_policy_control)
        return m_policy_control->Policy();
    else
        return nullptr;
}

void SlotControl::StartingChildDragDrop(const GG::Wnd* wnd, const GG::Pt& offset) {
    if (!m_policy_control)
        return;

    const auto control = dynamic_cast<const PolicyControl*>(wnd);
    if (!control)
        return;

    if (control == m_policy_control.get())
        m_policy_control->Hide();
}

void SlotControl::CancellingChildDragDrop(const std::vector<const GG::Wnd*>& wnds) {
    if (!m_policy_control)
        return;

    for (const auto& wnd : wnds) {
        const auto control = dynamic_cast<const PolicyControl*>(wnd);
        if (!control)
            continue;

        if (control == m_policy_control.get())
            m_policy_control->Show();
    }
}

void SlotControl::AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                              GG::Flags<GG::ModKey> mod_keys)
{
    if (wnds.size() != 1)
        ErrorLogger() << "SlotControl::AcceptDrops given multiple wnds unexpectedly...";

    const auto wnd = *(wnds.begin());
    auto* control = boost::polymorphic_downcast<const PolicyControl*>(wnd.get());
    const Policy* policy_type = control ? control->Policy() : nullptr;

    if (policy_type)
        SlotContentsAlteredSignal(policy_type, (mod_keys & GG::MOD_KEY_CTRL));
}

void SlotControl::ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds,
                                      const GG::Wnd* destination)
{
    if (wnds.empty())
        return;
    const GG::Wnd* wnd = wnds.front();
    const auto policy_control = dynamic_cast<const PolicyControl*>(wnd);
    if (policy_control != m_policy_control.get())
        return;
    DetachChildAndReset(m_policy_control);
    SlotContentsAlteredSignal(nullptr, false);
}

void SlotControl::DragDropEnter(const GG::Pt& pt,
                                std::map<const Wnd*, bool>& drop_wnds_acceptable,
                                GG::Flags<GG::ModKey> mod_keys)
{

    if (drop_wnds_acceptable.empty())
        return;

    DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);

    // Note:  If this SlotControl is being dragged over this indicates
    //        the dragged policy would replace this policy.
    if (drop_wnds_acceptable.begin()->second && m_policy_control)
        m_policy_control->Hide();
}

void SlotControl::DragDropLeave() {
    // Note:  If m_policy_control is being dragged, this does nothing, because it is detached.
    //        If this SlotControl is being dragged over this indicates the dragged policy would
    //        replace this policy.
    if (m_policy_control && !GG::GUI::GetGUI()->DragDropWnd(m_policy_control.get()))
        m_policy_control->Show();
}

void SlotControl::Render()
{}

void SlotControl::Highlight(bool actually)
{ m_highlighted = actually; }

void SlotControl::SetPolicy(const std::string& policy_name)
{ SetPolicy(GetPolicyType(policy_name)); }

void SlotControl::SetPolicy(const Policy* policy) {
    // remove existing policy control, if any
    DetachChildAndReset(m_policy_control);

    if (!policy)
        return;

    // create new policy control for passed in policy_type
    m_policy_control = GG::Wnd::Create<PolicyControl>(policy_type);
    AttachChild(m_policy_control);
    m_policy_control->InstallEventFilter(shared_from_this());

    // single click shows encyclopedia data
    m_policy_control->ClickedSignal.connect(PolicyTypeClickedSignal);

    // double click clears slot
    m_policy_control->DoubleClickedSignal.connect(
        [this](const Policy*){ this->SlotContentsAlteredSignal(nullptr, false); });
    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    // set policy occupying slot's tool tip to say slot type
    std::string title_text = UserString(policy->Name());

    m_policy_control->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
        ClientUI::PolicyIcon(policy->Name()),
        UserString(policy->Name()) + " (" + title_text + ")",
        UserString(policy->Description())
    ));
}

/** PoliciesListBox accepts policies that are being removed from a SlotControl.*/
void PoliciesListBox::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                   const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    // if more than one control dropped somehow, reject all
    if (std::distance(first, last) != 1)
        return;

    const auto&& parent = first->first->Parent();
    if (first->first->DragDropDataType() == POLICY_CONTROL_DROP_TYPE_STRING
        && parent
        && dynamic_cast<const SlotControl*>(parent.get()))
    {
        first->second = true;
    }
}

//////////////////////////////////////////////////
// GovernmentWnd::MainPanel                     //
//////////////////////////////////////////////////
class GovernmentWnd::MainPanel : public CUIWnd {
public:
    /** \name Structors */ //@{
    MainPanel(const std::string& config_name);
    void CompleteConstruction() override;
    //@}

    /** \name Accessors */ //@{
    std::vector<std::string> Policies() const; //!< returns vector of names of policies in slots of current shown design.  empty slots are represented with empty string
    //@}

    /** \name Mutators */ //@{
    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) override;
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    void Sanitize();

    void SetPolicy(const std::string& policy_name, unsigned int slot);  //!< puts specified policy in specified slot.  does nothing if slot is out of range of available slots for current hull

    /** Sets the policy in \p slot to \p policy and emits and signal if
      * requested. */
    void SetPolicy(const Policy* policy, unsigned int slot, bool emit_signal = false);
    void SetPolicies(const std::vector<std::string>& policies);         //!< puts specified policies in slots.  attempts to put each policy into the slot corresponding to its place in the passed vector.  if a policy cannot be placed, it is ignored.  more policies than there are slots available are ignored, and slots for which there are insufficient policies in the passed vector are unmodified

    /** If a suitable slot is available, adds the specified policy to the
      * government. */
    void AddPolicy(const Policy* policy);
    bool CanPolicyBeAdded(const Policy* policy) const;
    void ClearPolicies(); //!< removes all policies from design.  hull is not altered
    void ClearPolicy(const std::string& policy_name);
    //@}

    /** emitted when the design is changed (by adding or removing policies, not
      * name or description changes) */
    mutable boost::signals2::signal<void ()> PoliciesChangedSignal;
    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> PolicyTypeClickedSignal;

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    void Populate();        //!< creates and places SlotControls for current hull
    void DoLayout();        //!< positions buttons, text entry boxes and SlotControls
    void PoliciesChanged();

    bool AddPolicyEmptySlot(const Policy* policy, int slot_number);
    int FindEmptySlotForPolicy(const Policy* policy);

    std::vector<std::shared_ptr<SlotControl>>   m_slots;
    std::shared_ptr<GG::StaticGraphic>          m_background_image;
    std::shared_ptr<GG::Button>                 m_confirm_button;
    std::shared_ptr<GG::Button>                 m_clear_button;
};


GovernmentWnd::MainPanel::MainPanel(const std::string& config_name) :
    CUIWnd(UserString("DESIGN_WND_MAIN_PANEL_TITLE"),
           GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE,
           config_name),
    m_hull(nullptr),
    m_slots(),
    m_replaced_design_id(boost::none),
    m_replaced_design_uuid(boost::none),
    m_incomplete_design(),
    m_background_image(nullptr),
    m_design_name_label(nullptr),
    m_design_name(nullptr),
    m_design_description_label(nullptr),
    m_design_description(nullptr),
    m_replace_button(nullptr),
    m_confirm_button(nullptr),
    m_clear_button(nullptr),
    m_disabled_by_name(false),
    m_disabled_by_policy_conflict(false)
{}

void GovernmentWnd::MainPanel::CompleteConstruction() {
    SetChildClippingMode(ClipToClient);

    m_design_name_label = GG::Wnd::Create<CUILabel>(UserString("DESIGN_WND_DESIGN_NAME"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_design_name = GG::Wnd::Create<CUIEdit>(UserString("DESIGN_NAME_DEFAULT"));
    m_design_description_label = GG::Wnd::Create<CUILabel>(UserString("DESIGN_WND_DESIGN_DESCRIPTION"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_design_description = GG::Wnd::Create<CUIEdit>(UserString("DESIGN_DESCRIPTION_DEFAULT"));
    m_replace_button = Wnd::Create<CUIButton>(UserString("DESIGN_WND_UPDATE"));
    m_confirm_button = Wnd::Create<CUIButton>(UserString("DESIGN_WND_ADD_FINISHED"));
    m_clear_button = Wnd::Create<CUIButton>(UserString("DESIGN_WND_CLEAR"));

    m_replace_button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_confirm_button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    AttachChild(m_design_name_label);
    AttachChild(m_design_name);
    AttachChild(m_design_description_label);
    AttachChild(m_design_description);
    AttachChild(m_replace_button);
    AttachChild(m_confirm_button);
    AttachChild(m_clear_button);

    m_clear_button->LeftClickedSignal.connect(
        boost::bind(&GovernmentWnd::MainPanel::ClearPolicies, this));
    m_design_name->EditedSignal.connect(
        boost::bind(&GovernmentWnd::MainPanel::DesignNameEditedSlot, this, _1));
    m_replace_button->LeftClickedSignal.connect(DesignReplacedSignal);
    m_confirm_button->LeftClickedSignal.connect(DesignConfirmedSignal);
    DesignChangedSignal.connect(boost::bind(&GovernmentWnd::MainPanel::DesignChanged, this));
    DesignReplacedSignal.connect(boost::bind(&GovernmentWnd::MainPanel::ReplaceDesign, this));
    DesignConfirmedSignal.connect(boost::bind(&GovernmentWnd::MainPanel::AddDesign, this));

    DesignChanged(); // Initialize components that rely on the current state of the design.

    CUIWnd::CompleteConstruction();

    DoLayout();
    SaveDefaultedOptions();
}

const std::vector<std::string> GovernmentWnd::MainPanel::Policies() const {
    std::vector<std::string> retval;
    for (const auto& slot : m_slots) {
        const Policy* policy_type = slot->GetPolicy();
        if (policy_type)
            retval.push_back(policy_type->Name());
        else
            retval.push_back("");
    }
    return retval;
}

void GovernmentWnd::MainPanel::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (m_hull)
        HullTypeClickedSignal(m_hull);
    CUIWnd::LClick(pt, mod_keys);
}

void GovernmentWnd::MainPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    CUIWnd::SizeMove(ul, lr);
    DoLayout();
}

void GovernmentWnd::MainPanel::Sanitize() {
    SetHull(nullptr);
    m_design_name->SetText(UserString("DESIGN_NAME_DEFAULT"));
    m_design_description->SetText(UserString("DESIGN_DESCRIPTION_DEFAULT"));
    // disconnect old empire design signal
    m_empire_designs_changed_signal.disconnect();
}

void GovernmentWnd::MainPanel::SetPolicy(const std::string& policy_name, unsigned int slot)
{ SetPolicy(GetPolicyType(policy_name), slot); }

void GovernmentWnd::MainPanel::SetPolicy(const Policy* policy, unsigned int slot, bool emit_signal /* = false */, bool change_all_similar_policies /*= false*/) {
    //DebugLogger() << "GovernmentWnd::MainPanel::SetPolicy(" << (policy ? policy->Name() : "no policy") << ", slot " << slot << ")";
    if (slot > m_slots.size()) {
        ErrorLogger() << "GovernmentWnd::MainPanel::SetPolicy specified nonexistant slot";
        return;
    }

    if (!change_all_similar_policies) {
        m_slots[slot]->SetPolicy(policy);

    } else {
        const auto original_policy = m_slots[slot]->GetPolicy();
        std::string original_policy_name = original_policy ? original_policy->Name() : "";

        if (change_all_similar_policies) {
            for (auto& slot : m_slots) {
                // skip incompatible slots
                if (!policy->CanMountInSlotCategory(slot->SlotCategory()))
                    continue;

                // skip different type policies
                const auto replaced_policy = slot->GetPolicy();
                if (replaced_policy && (replaced_policy->Name() != original_policy_name))
                    continue;

                slot->SetPolicy(policy);
            }
        }
    }

    if (emit_signal)  // to avoid unnecessary signal repetition.
        DesignChangedSignal();
}

void GovernmentWnd::MainPanel::SetPolicies(const std::vector<std::string>& policies) {
    unsigned int num_policies = std::min(policies.size(), m_slots.size());
    for (unsigned int i = 0; i < num_policies; ++i)
        m_slots[i]->SetPolicy(policies[i]);

    DesignChangedSignal();
}

void GovernmentWnd::MainPanel::AddPolicy(const Policy* policy) {
    if (AddPolicyEmptySlot(policy, FindEmptySlotForPolicy(policy)))
        return;

    if (!AddPolicyWithSwapping(policy, FindSlotForPolicyWithSwapping(policy)))
        DebugLogger() << "GovernmentWnd::MainPanel::AddPolicy(" << (policy ? policy->Name() : "no policy")
                      << ") couldn't find a slot for the policy";
}

bool GovernmentWnd::MainPanel::CanPolicyBeAdded(const Policy* policy) {
    std::pair<int, int> swap_result = FindSlotForPolicyWithSwapping(policy);
    return (FindEmptySlotForPolicy(policy) >= 0 || (swap_result.first >= 0 && swap_result.second >= 0));
}

bool GovernmentWnd::MainPanel::AddPolicyEmptySlot(const Policy* policy, int slot_number) {
    if (!policy || slot_number < 0)
        return false;
    SetPolicy(policy, slot_number);
    DesignChangedSignal();
    return true;
}

int GovernmentWnd::MainPanel::FindEmptySlotForPolicy(const Policy* policy) {
    int result = -1;
    if (!policy)
        return result;

    if (policy->Category() == PC_FIGHTER_HANGAR) {
        // give up if policy is a hangar and there is already a hangar of another type
        std::string already_seen_hangar_name;
        for (const auto& slot : m_slots) {
            const Policy* policy_type = slot->GetPolicy();
            if (!policy_type || policy_type->Category() != PC_FIGHTER_HANGAR)
                continue;
            if (policy_type->Name() != policy->Name())
                return result;
        }
    }

    for (unsigned int i = 0; i < m_slots.size(); ++i) {             // scan through slots to find one that can mount policy
        const const std::string& slot_type = m_slots[i]->SlotCategory();
        const Policy* policy_type = m_slots[i]->GetPolicy();          // check if this slot is empty

        if (!policy_type && policy->CanMountInSlotCategory(slot_type)) {    // ... and if the policy can mount here
            result = i;
            return result;
        }
    }
    return result;
}

void GovernmentWnd::MainPanel::ClearPolicies() {
    for (auto& slot : m_slots)
        slot->SetPolicy(nullptr);
    DesignChangedSignal();
}

void GovernmentWnd::MainPanel::ClearPolicy(const std::string& policy_name) {
    bool changed = false;
    for (const auto& slot : m_slots) {
        const Policy* existing_policy = slot->GetPolicy();
        if (!existing_policy)
            continue;
        if (existing_policy->Name() != policy_name)
            continue;
        slot->SetPolicy(nullptr);
        changed = true;
    }

    if (changed)
        DesignChangedSignal();
}

void GovernmentWnd::MainPanel::HighlightSlotCategory(std::vector<const std::string&>& slot_types) {
    for (auto& control : m_slots) {
        const std::string& slot_type = control->SlotCategory();
        if (std::find(slot_types.begin(), slot_types.end(), slot_type) != slot_types.end())
            control->Highlight(true);
        else
            control->Highlight(false);
    }
}

void GovernmentWnd::MainPanel::Populate(){
    for (const auto& slot: m_slots)
        DetachChild(slot);
    m_slots.clear();

    if (!m_hull)
        return;

    const std::vector<HullType::Slot>& hull_slots = m_hull->Slots();

    for (std::vector<HullType::Slot>::size_type i = 0; i != hull_slots.size(); ++i) {
        const HullType::Slot& slot = hull_slots[i];
        auto slot_control = GG::Wnd::Create<SlotControl>(slot.x, slot.y, slot.type);
        m_slots.push_back(slot_control);
        AttachChild(slot_control);
        slot_control->SlotContentsAlteredSignal.connect(
            boost::bind(static_cast<void (GovernmentWnd::MainPanel::*)(const Policy*, unsigned int, bool, bool)>(&GovernmentWnd::MainPanel::SetPolicy), this, _1, i, true, _2));
        slot_control->PolicyTypeClickedSignal.connect(
            PolicyTypeClickedSignal);
    }
}

void GovernmentWnd::MainPanel::DoLayout() {
    // position labels and text edit boxes for name and description and buttons to clear and confirm design

    const int PTS = ClientUI::Pts();
    const GG::X PTS_WIDE(PTS / 2);           // guess at how wide per character the font needs
    const GG::Y BUTTON_HEIGHT(PTS * 2);
    const GG::X LABEL_WIDTH = PTS_WIDE * 15;
    const int PAD = 6;
    const int GUESSTIMATE_NUM_CHARS_IN_BUTTON_TEXT = 25;    // rough guesstimate... avoid overly long policy class names
    const GG::X BUTTON_WIDTH = PTS_WIDE*GUESSTIMATE_NUM_CHARS_IN_BUTTON_TEXT;

    GG::X edit_right = ClientWidth();
    GG::X confirm_right = ClientWidth() - PAD;

    GG::Pt lr = GG::Pt(confirm_right, BUTTON_HEIGHT) + GG::Pt(GG::X0, GG::Y(PAD));
    GG::Pt ul = lr - GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_confirm_button->SizeMove(ul, lr);

    lr = lr - GG::Pt(BUTTON_WIDTH, GG::Y(0))- GG::Pt(GG::X(PAD),GG::Y(0));
    ul = lr - GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_replace_button->SizeMove(ul, lr);

    edit_right = ul.x - PAD;

    lr = ClientSize() + GG::Pt(-GG::X(PAD), -GG::Y(PAD));
    ul = lr - GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_clear_button->SizeMove(ul, lr);

    ul = GG::Pt(GG::X(PAD), GG::Y(PAD));
    lr = ul + GG::Pt(LABEL_WIDTH, m_design_name->MinUsableSize().y);
    m_design_name_label->SizeMove(ul, lr);

    ul.x += lr.x;
    lr.x = edit_right;
    m_design_name->SizeMove(ul, lr);

    ul.x = GG::X(PAD);
    ul.y += (m_design_name->Height() + PAD);
    lr = ul + GG::Pt(LABEL_WIDTH, m_design_name->MinUsableSize().y);
    m_design_description_label->SizeMove(ul, lr);

    ul.x = lr.x + PAD;
    lr.x = confirm_right;
    m_design_description->SizeMove(ul, lr);

    // place background image of hull
    ul.x = GG::X0;
    ul.y += m_design_name->Height();
    GG::Rect background_rect = GG::Rect(ul, ClientLowerRight());

    if (m_background_image) {
        GG::Pt bg_ul = background_rect.UpperLeft();
        GG::Pt bg_lr = ClientSize();
        m_background_image->SizeMove(bg_ul, bg_lr);
        background_rect = m_background_image->RenderedArea();
    }

    // place slot controls over image of hull
    for (auto& slot : m_slots) {
        GG::X x(background_rect.Left() - slot->Width()/2 - ClientUpperLeft().x + slot->XPositionFraction() * background_rect.Width());
        GG::Y y(background_rect.Top() - slot->Height()/2 - ClientUpperLeft().y + slot->YPositionFraction() * background_rect.Height());
        slot->MoveTo(GG::Pt(x, y));
    }
}

void GovernmentWnd::MainPanel::DesignChanged() {
    m_replace_button->ClearBrowseInfoWnd();
    m_confirm_button->ClearBrowseInfoWnd();

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    m_disabled_by_name = false;
    m_disabled_by_policy_conflict = false;

    m_replace_button->Disable(true);
    m_confirm_button->Disable(true);

    m_replace_button->SetText(UserString("DESIGN_WND_UPDATE_FINISHED"));
    m_confirm_button->SetText(UserString("DESIGN_WND_ADD_FINISHED"));

    if (!m_hull) {
        m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_UPDATE_INVALID_NO_CANDIDATE")));
        m_confirm_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_NO_HULL")));
        return;
    }

    if (client_empire_id == ALL_EMPIRES) {
        m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_MODERATOR")));
        m_confirm_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_MODERATOR")));
        return;
    }

    if (!IsDesignNameValid()) {
        m_disabled_by_name = true;

        m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_NO_NAME")));
        m_confirm_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_NO_NAME")));
        return;
    }

    if (!ShipDesign::ValidDesign(m_hull->Name(), Policies())) {
        // if a design has exclusion violations between policies and hull, highlight these and indicate it on the button

        std::pair<std::string, std::string> problematic_components;

        // check hull exclusions against all policies...
        const std::set<std::string>& hull_exclusions = m_hull->Exclusions();
        for (const std::string& policy_name : Policies()) {
            if (policy_name.empty())
                continue;
            if (hull_exclusions.find(policy_name) != hull_exclusions.end()) {
                m_disabled_by_policy_conflict = true;
                problematic_components.first = m_hull->Name();
                problematic_components.second = policy_name;
            }
        }

        // check policy exclusions against other policies and hull
        std::set<std::string> already_seen_component_names;
        already_seen_component_names.insert(m_hull->Name());
        for (const std::string& policy_name : Policies()) {
            if (m_disabled_by_policy_conflict)
                break;
            const Policy* policy_type = GetPolicyType(policy_name);
            if (!policy_type)
                continue;
            for (const std::string& excluded_policy : policy_type->Exclusions()) {
                if (already_seen_component_names.find(excluded_policy) != already_seen_component_names.end()) {
                    m_disabled_by_policy_conflict = true;
                    problematic_components.first = policy_name;
                    problematic_components.second = excluded_policy;
                    break;
                }
            }
            already_seen_component_names.insert(policy_name);
        }


        if (m_disabled_by_policy_conflict) {
            m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_COMPONENT_CONFLICT"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_COMPONENT_CONFLICT_DETAIL"))
                               % UserString(problematic_components.first)
                               % UserString(problematic_components.second))));
            m_confirm_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_COMPONENT_CONFLICT"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_COMPONENT_CONFLICT_DETAIL"))
                               % UserString(problematic_components.first)
                               % UserString(problematic_components.second))));

            // todo: mark conflicting policies somehow
        }
        return;
    }

    const auto& cur_design = GetIncompleteDesign();

    if (!cur_design)
        return;

    const auto new_design_name = ValidatedDesignName().DisplayText();

    // producible only matters for empire designs.
    // Monster designs can be edited as saved designs.
    bool producible = cur_design->Producible();

    // Current designs can not duplicate other designs, be already registered.
    const auto existing_design_name = CurrentDesignIsRegistered();

    const auto& replaced_saved_design = EditingSavedDesign();

    const auto& replaced_current_design = EditingCurrentDesign();

    // Choose text for the replace button: replace saved design, replace current design or already known.

    // A changed saved design can be replaced with an updated design
    if (replaced_saved_design) {
        if (cur_design && !(*cur_design == **replaced_saved_design)) {
            m_replace_button->SetText(UserString("DESIGN_WND_UPDATE_SAVED"));
            m_replace_button->SetBrowseInfoWnd(
                GG::Wnd::Create<TextBrowseWnd>(
                    UserString("DESIGN_WND_UPDATE_SAVED"),
                    boost::io::str(FlexibleFormat(UserString("DESIGN_WND_UPDATE_SAVED_DETAIL"))
                                   % (*replaced_saved_design)->Name()
                                   % new_design_name)));
            m_replace_button->Disable(false);
        }
    }

    if (producible && replaced_current_design) {
        if (!existing_design_name) {
            // A current design can be replaced if it doesn't duplicate an existing design
            m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_UPDATE_FINISHED"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_UPDATE_FINISHED_DETAIL"))
                               % (*replaced_current_design)->Name()
                               % new_design_name)));
            m_replace_button->Disable(false);
        } else {
            // Otherwise mark it as known.
            m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_KNOWN"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_KNOWN_DETAIL"))
                               % *existing_design_name)));
        }
    }

    // Choose text for the add new design button: add saved design, add current design or already known.

    // Add a saved design if the saved base selector was visited more recently than the current tab.
    if (m_type_to_create == GovernmentWnd::BaseSelector::BaseSelectorTab::Saved) {
        // A new saved design can always be created
        m_confirm_button->SetText(UserString("DESIGN_WND_ADD_SAVED"));
        m_confirm_button->SetBrowseInfoWnd(
            GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_ADD_SAVED"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_ADD_SAVED_DETAIL"))
                               % new_design_name)));
        m_confirm_button->Disable(false);
    } else if (producible) {
        if (!existing_design_name) {
            // A new current can be added if it does not duplicate an existing design.
            m_confirm_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_ADD_FINISHED"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_ADD_FINISHED_DETAIL"))
                               % new_design_name)));
            m_confirm_button->Disable(false);

        } else {
            // Otherwise the design is already known.
            m_confirm_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_KNOWN"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_KNOWN_DETAIL"))
                               % *existing_design_name)));
        }
    }
}

void GovernmentWnd::MainPanel::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                           const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    // if multiple things dropped simultaneously somehow, reject all
    if (std::distance(first, last) != 1)
        return;

    if (dynamic_cast<const BasesListBox::BasesListBoxRow*>(first->first))
        first->second = true;
}

void GovernmentWnd::MainPanel::AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) {
    if (wnds.size() != 1)
        ErrorLogger() << "GovernmentWnd::MainPanel::AcceptDrops given multiple wnds unexpectedly...";

    const auto& wnd = *(wnds.begin());
    if (!wnd)
        return;

    if (const auto completed_design_row = dynamic_cast<const BasesListBox::CompletedDesignListBoxRow*>(wnd.get())) {
        SetDesign(GetShipDesign(completed_design_row->DesignID()));
    }
    else if (const auto hullandpolicies_row = dynamic_cast<const BasesListBox::HullAndPoliciesListBoxRow*>(wnd.get())) {
        const std::string& hull = hullandpolicies_row->Hull();
        const std::vector<std::string>& policies = hullandpolicies_row->Policies();

        SetDesignComponents(hull, policies);
    }
    else if (const auto saved_design_row = dynamic_cast<const SavedDesignsListBox::SavedDesignListBoxRow*>(wnd.get())) {
        const auto& uuid = saved_design_row->DesignUUID();
        SetDesign(GetSavedDesignsManager().GetDesign(uuid));
    }
}


//////////////////////////////////////////////////
// GovernmentWnd                                    //
//////////////////////////////////////////////////
GovernmentWnd::GovernmentWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::ONTOP | GG::INTERACTIVE),
    m_detail_panel(nullptr),
    m_base_selector(nullptr),
    m_policy_palette(nullptr),
    m_main_panel(nullptr)
{}

void GovernmentWnd::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    Sound::TempUISoundDisabler sound_disabler;
    SetChildClippingMode(ClipToClient);

    m_detail_panel = GG::Wnd::Create<EncyclopediaDetailPanel>(GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | PINABLE, DES_PEDIA_WND_NAME);
    m_main_panel = GG::Wnd::Create<MainPanel>(DES_MAIN_WND_NAME);
    m_policy_palette = GG::Wnd::Create<PolicyPalette>(DES_PART_PALETTE_WND_NAME);
    m_base_selector = GG::Wnd::Create<BaseSelector>(DES_BASE_SELECTOR_WND_NAME);
    InitializeWindows();
    HumanClientApp::GetApp()->RepositionWindowsSignal.connect(
        boost::bind(&GovernmentWnd::InitializeWindows, this));

    AttachChild(m_detail_panel);

    AttachChild(m_main_panel);
    m_main_panel->PolicyTypeClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const Policy*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_main_panel->HullTypeClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const HullType*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_main_panel->DesignChangedSignal.connect(
        boost::bind(&GovernmentWnd::DesignChanged, this));
    m_main_panel->DesignNameChangedSignal.connect(
        boost::bind(&GovernmentWnd::DesignNameChanged, this));
    m_main_panel->CompleteDesignClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(int)>(&EncyclopediaDetailPanel::SetDesign), m_detail_panel, _1));
    m_main_panel->Sanitize();

    AttachChild(m_policy_palette);
    m_policy_palette->PolicyTypeClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const Policy*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_policy_palette->PolicyTypeDoubleClickedSignal.connect(
        boost::bind(&GovernmentWnd::MainPanel::AddPolicy, m_main_panel, _1));
    m_policy_palette->ClearPolicySignal.connect(
        boost::bind(&GovernmentWnd::MainPanel::ClearPolicy, m_main_panel, _1));

    AttachChild(m_base_selector);

    m_base_selector->DesignSelectedSignal.connect(
        boost::bind(static_cast<void (MainPanel::*)(int)>(&MainPanel::SetDesign), m_main_panel, _1));
    m_base_selector->DesignComponentsSelectedSignal.connect(
        boost::bind(&MainPanel::SetDesignComponents, m_main_panel, _1, _2));
    m_base_selector->SavedDesignSelectedSignal.connect(
        boost::bind(static_cast<void (MainPanel::*)(const boost::uuids::uuid&)>(&MainPanel::SetDesign), m_main_panel, _1));

    m_base_selector->DesignClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const ShipDesign*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_base_selector->HullClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const HullType*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_base_selector->TabChangedSignal.connect(boost::bind(&MainPanel::HandleBaseTypeChange, m_main_panel, _1));

    // Connect signals to re-populate when policy obsolescence changes
    m_policy_palette->PolicyObsolescenceChangedSignal.connect(
        boost::bind(&BaseSelector::Reset, m_base_selector));
}

void GovernmentWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size()) {
        m_detail_panel->ValidatePosition();
        m_base_selector->ValidatePosition();
        m_policy_palette->ValidatePosition();
        m_main_panel->ValidatePosition();
    }
}

void GovernmentWnd::Reset() {
    m_policy_palette->Populate();
    m_base_selector->Reset();
    m_detail_panel->Refresh();
    m_main_panel->Sanitize();
}

void GovernmentWnd::Sanitize()
{ m_main_panel->Sanitize(); }

void GovernmentWnd::Render()
{ GG::FlatRectangle(UpperLeft(), LowerRight(), ClientUI::WndColor(), GG::CLR_ZERO, 0); }

void GovernmentWnd::InitializeWindows() {
    const GG::X selector_width = GG::X(250);
    const GG::X main_width = ClientWidth() - selector_width;

    const GG::Pt pedia_ul(selector_width, GG::Y0);
    const GG::Pt pedia_wh(5*main_width/11, 2*ClientHeight()/5);

    const GG::Pt main_ul(selector_width, pedia_ul.y + pedia_wh.y);
    const GG::Pt main_wh(main_width, ClientHeight() - main_ul.y);

    const GG::Pt palette_ul(selector_width + pedia_wh.x, pedia_ul.y);
    const GG::Pt palette_wh(main_width - pedia_wh.x, pedia_wh.y);

    const GG::Pt selector_ul(GG::X0, GG::Y0);
    const GG::Pt selector_wh(selector_width, ClientHeight());

    m_detail_panel-> InitSizeMove(pedia_ul,     pedia_ul + pedia_wh);
    m_main_panel->   InitSizeMove(main_ul,      main_ul + main_wh);
    m_policy_palette-> InitSizeMove(palette_ul,   palette_ul + palette_wh);
    m_base_selector->InitSizeMove(selector_ul,  selector_ul + selector_wh);
}

void GovernmentWnd::ShowPolicyTypeInEncyclopedia(const std::string& policy_type)
{ m_detail_panel->SetPolicyType(policy_type); }

void GovernmentWnd::ShowHullTypeInEncyclopedia(const std::string& hull_type)
{ m_detail_panel->SetHullType(hull_type); }

void GovernmentWnd::ShowShipDesignInEncyclopedia(int design_id)
{ m_detail_panel->SetDesign(design_id); }

void GovernmentWnd::DesignChanged() {
    m_detail_panel->SetIncompleteDesign(m_main_panel->GetIncompleteDesign());
    m_base_selector->Reset();
}

void GovernmentWnd::DesignNameChanged() {
    m_detail_panel->SetIncompleteDesign(m_main_panel->GetIncompleteDesign());
    m_base_selector->Reset();
}

void GovernmentWnd::EnableOrderIssuing(bool enable/* = true*/)
{ m_base_selector->EnableOrderIssuing(enable); }
