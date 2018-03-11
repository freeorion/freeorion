#include "GovernmentWnd.h"

#include "ClientUI.h"
#include "CUIWnd.h"
#include "CUIControls.h"
#include "QueueListBox.h"   // for PromptRow
#include "IconTextBrowseWnd.h"
#include "Sound.h"
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

#include <boost/cast.hpp>

namespace {
    const std::string   POLICY_CONTROL_DROP_TYPE_STRING = "Policy Control";
    const std::string   EMPTY_STRING = "";
    const std::string   GOV_MAIN_WND_NAME = "government.edit";
    const std::string   GOV_POLICY_PALETTE_WND_NAME = "government.policies";
    const GG::X         POLICY_CONTROL_WIDTH(54);
    const GG::Y         POLICY_CONTROL_HEIGHT(54);
    const GG::X         SLOT_CONTROL_WIDTH(60);
    const GG::Y         SLOT_CONTROL_HEIGHT(60);
    const int           PAD(3);

    /** Returns texture with which to render a PolicySlotControl, depending on
      * \a category */
    std::shared_ptr<GG::Texture> SlotBackgroundTexture(const std::string& category) {
        if (category == "ECONOMIC_CATEGORY")
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "policies" / "economic_slot.png", true);
        if (category == "SOCIAL_CATEGORY")
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "policies" / "social_slot.png", true);
        if (category == "MILITARY_CATEGORY")
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "policies" / "military_slot.png", true);
        return ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "missing.png", true);
    }

    /** Returns background texture with which to render a PolicyControl,
      * depending on the category of slot that the indicated \a policy can
      * be put into. */
    std::shared_ptr<GG::Texture> PolicyBackgroundTexture(const Policy* policy) {
        if (policy) {
            if (policy->Category() == "ECONOMIC_CATEGORY")
                return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "policies" / "economic_policy.png", true);
            if (policy->Category() == "SOCIAL_CATEGORY")
                return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "policies" / "social_policy.png", true);
            if (policy->Category() == "MILITARY_CATEGORY")
                return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "policies" / "military_policy.png", true);
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
    explicit PolicyControl(const Policy* policy);
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    const std::string&  PolicyName() const  { return m_policy ? m_policy->Name() : EMPTY_STRING; }
    const Policy*       GetPolicy() const   { return m_policy; }
    //@}

    /** \name Mutators */ //@{
    void Render() override;
    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
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
    GG::X policy_left = (Width() - POLICY_CONTROL_WIDTH) / 2;
    GG::Y policy_top = (Height() - POLICY_CONTROL_HEIGHT) / 2;

    //DebugLogger() << "PolicyControl::PolicyControl this: " << this << " policy: " << policy << " named: " << (policy ? policy->Name() : "no policy");
    m_icon = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::PolicyIcon(m_policy->Name()), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_icon->MoveTo(GG::Pt(policy_left, policy_top));
    m_icon->Resize(GG::Pt(POLICY_CONTROL_WIDTH, POLICY_CONTROL_HEIGHT));
    m_icon->Show();
    AttachChild(m_icon);

    SetDragDropDataType(POLICY_CONTROL_DROP_TYPE_STRING);

    //DebugLogger() << "PolicyControl::PolicyControl policy name: " << m_policy->Name();
    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
        ClientUI::PolicyIcon(m_policy->Name()),
        UserString(m_policy->Name()),
        UserString(m_policy->Description())
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

    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)>    PolicyClickedSignal;
    mutable boost::signals2::signal<void (const Policy*)>                           PolicyDoubleClickedSignal;
    mutable boost::signals2::signal<void (const Policy*, const GG::Pt& pt)>         PolicyRightClickedSignal;
    mutable boost::signals2::signal<void (const std::string&)>                      ClearPolicySignal;

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    std::map<std::string, std::vector<const Policy*>>
    GroupAvailableDisplayablePolicies(const Empire* empire) const;

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

    const auto policy_type = policy_control->GetPolicy();
    if (!policy_type)
        return;

    auto new_policy_control = GG::Wnd::Create<PolicyControl>(policy_type);
    const auto parent = dynamic_cast<const PoliciesListBox*>(Parent().get());
    if (parent) {
        new_policy_control->ClickedSignal.connect(parent->PolicyClickedSignal);
        new_policy_control->DoubleClickedSignal.connect(parent->PolicyDoubleClickedSignal);
        new_policy_control->RightClickedSignal.connect(parent->PolicyRightClickedSignal);
    }

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

const std::set<std::string>& PoliciesListBox::GetCategoriesShown() const
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
    const Policy* policy_type = control ? control->GetPolicy() : nullptr;
    if (!policy_type)
        return;

    ClearPolicySignal(policy_type->Name());
}

std::map<std::string, std::vector<const Policy*>>
PoliciesListBox::GroupAvailableDisplayablePolicies(const Empire* empire) const {
    std::map<std::string, std::vector<const Policy*>> policies_categorized;

    // loop through all possible policies
    for (const auto& entry : GetPolicyManager()) {
        const auto& policy = entry.second;
        const auto& category = policy->Category();

        // check whether this policy should be shown in list
        if (m_policy_categories_shown.find(category) == m_policy_categories_shown.end())
            continue;   // policy of this class is not requested to be shown
        if (empire && !empire->AvailablePolicies().count(policy->Name()))
            continue;

        policies_categorized[category].push_back(policy.get());
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
            control->ClickedSignal.connect(PoliciesListBox::PolicyClickedSignal);
            control->DoubleClickedSignal.connect(PoliciesListBox::PolicyDoubleClickedSignal);
            control->RightClickedSignal.connect(PoliciesListBox::PolicyRightClickedSignal);

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

void PoliciesListBox::ShowCategory(const std::string& category, bool refresh_list) {
    if (m_policy_categories_shown.find(category) == m_policy_categories_shown.end()) {
        m_policy_categories_shown.insert(category);
        if (refresh_list)
            Populate();
    }
}

void PoliciesListBox::ShowAllCategories(bool refresh_list) {
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

void PoliciesListBox::HideAllCategories(bool refresh_list) {
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
    void ShowAllCategories(bool refresh_list = true);
    void HideCategory(const std::string& category, bool refresh_list = true);
    void HideAllCategories(bool refresh_list = true);
    void ToggleCategory(const std::string& category, bool refresh_list = true);
    void ToggleAllCategories(bool refresh_list = true);

    void Populate();
    //@}

    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> PolicyClickedSignal;
    mutable boost::signals2::signal<void (const Policy*)> PolicyDoubleClickedSignal;
    mutable boost::signals2::signal<void (const Policy*, const GG::Pt& pt)> PolicyRightClickedSignal;
    mutable boost::signals2::signal<void (const std::string&)> ClearPolicySignal;

private:
    void DoLayout();

    /** A policy type click with ctrl obsoletes policy. */
    void HandlePolicyClicked(const Policy*, GG::Flags<GG::ModKey>);
    void HandlePolicyRightClicked(const Policy*, const GG::Pt& pt);

    std::shared_ptr<PoliciesListBox>                        m_policies_list;
    std::map<std::string, std::shared_ptr<CUIStateButton>>  m_category_buttons;
};

GovernmentWnd::PolicyPalette::PolicyPalette(const std::string& config_name) :
    CUIWnd(UserString("GOVERNMENT_WND_POLICY_PALETTE_TITLE"),
           GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE,
           config_name),
    m_policies_list(nullptr)
{}

void GovernmentWnd::PolicyPalette::CompleteConstruction() {
    SetChildClippingMode(ClipToClient);

    m_policies_list = GG::Wnd::Create<PoliciesListBox>();
    AttachChild(m_policies_list);
    m_policies_list->PolicyClickedSignal.connect(
        boost::bind(&GovernmentWnd::PolicyPalette::HandlePolicyClicked, this, _1, _2));
    m_policies_list->PolicyDoubleClickedSignal.connect(
        PolicyDoubleClickedSignal);
    m_policies_list->PolicyRightClickedSignal.connect(
        boost::bind(&GovernmentWnd::PolicyPalette::HandlePolicyRightClicked, this, _1, _2));
    m_policies_list->ClearPolicySignal.connect(ClearPolicySignal);

    const PolicyManager& policy_manager = GetPolicyManager();

    // class buttons
    for (auto& category : GetPolicyManager().PolicyCategories()) {
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

        m_category_buttons[category] = GG::Wnd::Create<CUIStateButton>(
            UserString(boost::lexical_cast<std::string>(category)),
            GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
        AttachChild(m_category_buttons[category]);
        m_category_buttons[category]->CheckedSignal.connect(
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

    const int NUM_CLASS_BUTTONS = std::max(1, static_cast<int>(m_category_buttons.size()));
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
    const int NUM_CATEGORY_BUTTONS_PER_ROW = static_cast<int>(std::ceil(static_cast<float>(NUM_CLASS_BUTTONS) / NUM_CLASS_BUTTON_ROWS));

    const int TOTAL_BUTTONS_PER_ROW = NUM_CATEGORY_BUTTONS_PER_ROW + num_non_class_buttons_per_row;

    const GG::X BUTTON_WIDTH = (USABLE_WIDTH - (TOTAL_BUTTONS_PER_ROW - 1)*BUTTON_SEPARATION) / TOTAL_BUTTONS_PER_ROW;

    const GG::X COL_OFFSET = BUTTON_WIDTH + BUTTON_SEPARATION;    // horizontal distance between each column of buttons
    const GG::Y ROW_OFFSET = BUTTON_HEIGHT + BUTTON_SEPARATION;   // vertical distance between each row of buttons

    // place category buttons
    int col = NUM_CATEGORY_BUTTONS_PER_ROW;
    int row = -1;
    for (auto& entry : m_category_buttons) {
        if (col >= NUM_CATEGORY_BUTTONS_PER_ROW) {
            col = 0;
            ++row;
        }
        GG::Pt ul(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        entry.second->SizeMove(ul, lr);
        ++col;
    }

    // place policies list.
    m_policies_list->SizeMove(GG::Pt(GG::X0, BUTTON_EDGE_PAD + ROW_OFFSET*(row + 1)),
                              ClientSize() - GG::Pt(GG::X(BUTTON_SEPARATION), GG::Y(BUTTON_SEPARATION)));
}

void GovernmentWnd::PolicyPalette::HandlePolicyClicked(const Policy* policy_type,
                                                       GG::Flags<GG::ModKey> modkeys)
{ PolicyClickedSignal(policy_type, modkeys); }

void GovernmentWnd::PolicyPalette::HandlePolicyRightClicked(const Policy* policy_type,
                                                            const GG::Pt& pt)
{ PolicyRightClickedSignal(policy_type, pt); }

void GovernmentWnd::PolicyPalette::ShowCategory(const std::string& category,
                                                bool refresh_list)
{
    if (!m_category_buttons.count(category)) {
        ErrorLogger() << "PolicyPalette::ShowCategory was passed an invalid category name: " << category;
        return;
    }
    m_policies_list->ShowCategory(category, refresh_list);
    m_category_buttons[category]->SetCheck();
}

void GovernmentWnd::PolicyPalette::ShowAllCategories(bool refresh_list) {
    m_policies_list->ShowAllCategories(refresh_list);
    for (auto& entry : m_category_buttons)
        entry.second->SetCheck();
}

void GovernmentWnd::PolicyPalette::HideCategory(const std::string& category,
                                                bool refresh_list)
{
    if (!m_category_buttons.count(category)) {
        ErrorLogger() << "PolicyPalette::HideCategory was passed an invalid category name: " << category;
        return;
    }
    m_policies_list->HideCategory(category, refresh_list);
    m_category_buttons[category]->SetCheck(false);
}

void GovernmentWnd::PolicyPalette::HideAllCategories(bool refresh_list) {
    m_policies_list->HideAllCategories(refresh_list);
    for (auto& entry : m_category_buttons)
        entry.second->SetCheck(false);
}

void GovernmentWnd::PolicyPalette::ToggleCategory(const std::string& category, bool refresh_list) {
    if (!m_category_buttons.count(category)) {
        ErrorLogger() << "PolicyPalette::ToggleCategory was passed an invalid category name: " << category;
        return;
    }

    const auto& categories_shown = m_policies_list->GetCategoriesShown();
    if (categories_shown.count(category))
        HideCategory(category, refresh_list);
    else
        ShowCategory(category, refresh_list);
}

void GovernmentWnd::PolicyPalette::ToggleAllCategories(bool refresh_list) {
    const auto& categories_shown = m_policies_list->GetCategoriesShown();
    if (categories_shown.size() == m_category_buttons.size())
        HideAllCategories(refresh_list);
    else
        ShowAllCategories(refresh_list);
}

void GovernmentWnd::PolicyPalette::Populate()
{ m_policies_list->Populate(); }


//////////////////////////////////////////////////
// PolicySlotControl                                  //
//////////////////////////////////////////////////
/** UI representation and drop-target for policies of a government.
  * PolicyControl may be dropped into slots to add the corresponding policies to
  * the government, or the policy may be set programmatically with SetPolicy(). */
class PolicySlotControl : public GG::Control {
public:
    /** \name Structors */ //@{
    PolicySlotControl();
    explicit PolicySlotControl(const std::string& slot_category);
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
    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> PolicyClickedSignal;

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

PolicySlotControl::PolicySlotControl() :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE)
{}

PolicySlotControl::PolicySlotControl(const std::string& slot_category) :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE),
    m_slot_category(slot_category)
{}

void PolicySlotControl::CompleteConstruction() {
    GG::Control::CompleteConstruction();

    m_background = GG::Wnd::Create<GG::StaticGraphic>(SlotBackgroundTexture(m_slot_category), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_background->Resize(GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT));
    m_background->Show();
    AttachChild(m_background);

    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
        SlotBackgroundTexture(m_slot_category),
        UserString(m_slot_category),
        UserString("SL_TOOLTIP_DESC")
    ));
}

bool PolicySlotControl::EventFilter(GG::Wnd* w, const GG::WndEvent& event) {
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

void PolicySlotControl::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
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
        const Policy* policy_type = policy_control->GetPolicy();
        if (policy_type &&
            policy_type->Category() == m_slot_category &&
            policy_control != m_policy_control.get())
        {
            it->second = true;
            return;
        }
    }
}

const std::string& PolicySlotControl::SlotCategory() const
{ return m_slot_category; }

const Policy* PolicySlotControl::GetPolicy() const {
    if (m_policy_control)
        return m_policy_control->GetPolicy();
    else
        return nullptr;
}

void PolicySlotControl::StartingChildDragDrop(const GG::Wnd* wnd, const GG::Pt& offset) {
    if (!m_policy_control)
        return;

    const auto control = dynamic_cast<const PolicyControl*>(wnd);
    if (!control)
        return;

    if (control == m_policy_control.get())
        m_policy_control->Hide();
}

void PolicySlotControl::CancellingChildDragDrop(const std::vector<const GG::Wnd*>& wnds) {
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

void PolicySlotControl::AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                                    GG::Flags<GG::ModKey> mod_keys)
{
    if (wnds.size() != 1)
        ErrorLogger() << "PolicySlotControl::AcceptDrops given multiple wnds unexpectedly...";

    const auto wnd = *(wnds.begin());
    auto* control = boost::polymorphic_downcast<const PolicyControl*>(wnd.get());
    const Policy* policy_type = control ? control->GetPolicy() : nullptr;

    if (policy_type)
        SlotContentsAlteredSignal(policy_type, (mod_keys & GG::MOD_KEY_CTRL));
}

void PolicySlotControl::ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds,
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

void PolicySlotControl::DragDropEnter(const GG::Pt& pt,
                                      std::map<const Wnd*, bool>& drop_wnds_acceptable,
                                      GG::Flags<GG::ModKey> mod_keys)
{

    if (drop_wnds_acceptable.empty())
        return;

    DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);

    // Note:  If this PolicySlotControl is being dragged over this indicates
    //        the dragged policy would replace this policy.
    if (drop_wnds_acceptable.begin()->second && m_policy_control)
        m_policy_control->Hide();
}

void PolicySlotControl::DragDropLeave() {
    // Note:  If m_policy_control is being dragged, this does nothing, because it is detached.
    //        If this PolicySlotControl is being dragged over this indicates the dragged policy would
    //        replace this policy.
    if (m_policy_control && !GG::GUI::GetGUI()->DragDropWnd(m_policy_control.get()))
        m_policy_control->Show();
}

void PolicySlotControl::Render()
{}

void PolicySlotControl::Highlight(bool actually)
{ m_highlighted = actually; }

void PolicySlotControl::SetPolicy(const std::string& policy_name)
{ SetPolicy( ::GetPolicy(policy_name)); }

void PolicySlotControl::SetPolicy(const Policy* policy) {
    // remove existing policy control, if any
    DetachChildAndReset(m_policy_control);

    if (!policy)
        return;

    // create new policy control for passed in policy_type
    m_policy_control = GG::Wnd::Create<PolicyControl>(policy);
    AttachChild(m_policy_control);
    m_policy_control->InstallEventFilter(shared_from_this());

    // single click shows encyclopedia data
    m_policy_control->ClickedSignal.connect(PolicyClickedSignal);

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

/** PoliciesListBox accepts policies that are being removed from a PolicySlotControl.*/
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
        && dynamic_cast<const PolicySlotControl*>(parent.get()))
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

    void SetEmpire(int empire_id = ALL_EMPIRES);
    void SetPolicy(const std::string& policy_name, unsigned int slot);  //!< puts specified policy in specified slot.  does nothing if slot is out of range of available slots for current hull

    /** Sets the policy in \p slot to \p policy and emits and signal if
      * requested. */
    void SetPolicy(const Policy* policy, unsigned int slot, bool emit_signal = false);
    void SetPolicies(const std::vector<std::string>& policies);         //!< puts specified policies in slots.  attempts to put each policy into the slot corresponding to its place in the passed vector.  if a policy cannot be placed, it is ignored.  more policies than there are slots available are ignored, and slots for which there are insufficient policies in the passed vector are unmodified

    /** If a suitable slot is available, adds the specified policy to the
      * government. */
    void AddPolicy(const Policy* policy);
    bool CanPolicyBeAdded(const Policy* policy) const;
    void ClearPolicies();   //!< removes all policies from government
    void ClearPolicy(const std::string& policy_name);
    //@}

    /** emitted when the design is changed (by adding or removing policies, not
      * name or description changes) */
    mutable boost::signals2::signal<void ()> PoliciesChangedSignal;
    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> PolicyClickedSignal;

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    void Populate();        //!< creates and places SlotControls for empire
    void DoLayout();        //!< positions SlotControls
    void PoliciesChanged();
    bool AddPolicyEmptySlot(const Policy* policy, int slot_number);
    int FindEmptySlotForPolicy(const Policy* policy) const;

    std::vector<std::shared_ptr<PolicySlotControl>> m_slots;
    std::shared_ptr<GG::StaticGraphic>              m_background_image;
    std::shared_ptr<GG::Button>                     m_clear_button;
};


GovernmentWnd::MainPanel::MainPanel(const std::string& config_name) :
    CUIWnd(UserString("GOVERNMENT_WND_MAIN_PANEL_TITLE"),
           GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE,
           config_name),
    m_slots(),
    m_background_image(nullptr),
    m_clear_button(nullptr)
{}

void GovernmentWnd::MainPanel::CompleteConstruction() {
    SetChildClippingMode(ClipToClient);

    m_clear_button = Wnd::Create<CUIButton>(UserString("GOVERNMENT_WND_CLEAR"));
    AttachChild(m_clear_button);

    m_clear_button->LeftClickedSignal.connect(
        boost::bind(&GovernmentWnd::MainPanel::ClearPolicies, this));
    PoliciesChangedSignal.connect(boost::bind(&GovernmentWnd::MainPanel::PoliciesChanged, this));

    PoliciesChanged(); // Initialize components that rely on the current state of the government.

    CUIWnd::CompleteConstruction();

    DoLayout();
    SaveDefaultedOptions();
}

std::vector<std::string> GovernmentWnd::MainPanel::Policies() const {
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

void GovernmentWnd::MainPanel::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ CUIWnd::LClick(pt, mod_keys); }

void GovernmentWnd::MainPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    CUIWnd::SizeMove(ul, lr);
    DoLayout();
}

void GovernmentWnd::MainPanel::Sanitize()
{ void ClearPolicies(); }

void GovernmentWnd::MainPanel::SetPolicy(const std::string& policy_name, unsigned int slot)
{ SetPolicy(GetPolicy(policy_name), slot); }

void GovernmentWnd::MainPanel::SetPolicy(const Policy* policy, unsigned int slot,
                                         bool emit_signal)
{
    //DebugLogger() << "GovernmentWnd::MainPanel::SetPolicy(" << (policy ? policy->Name() : "no policy") << ", slot " << slot << ")";
    if (slot > m_slots.size()) {
        ErrorLogger() << "GovernmentWnd::MainPanel::SetPolicy specified nonexistant slot";
        return;
    }

    if (emit_signal)  // to avoid unnecessary signal repetition.
        PoliciesChangedSignal();
}

void GovernmentWnd::MainPanel::SetPolicies(const std::vector<std::string>& policies) {
    unsigned int num_policies = std::min(policies.size(), m_slots.size());
    for (unsigned int i = 0; i < num_policies; ++i)
        m_slots[i]->SetPolicy(policies[i]);

    PoliciesChangedSignal();
}

void GovernmentWnd::MainPanel::AddPolicy(const Policy* policy)
{ AddPolicyEmptySlot(policy, FindEmptySlotForPolicy(policy)); }

bool GovernmentWnd::MainPanel::CanPolicyBeAdded(const Policy* policy) const
{ return FindEmptySlotForPolicy(policy) >= 0; }

bool GovernmentWnd::MainPanel::AddPolicyEmptySlot(const Policy* policy, int slot_number) {
    if (!policy || slot_number < 0)
        return false;
    SetPolicy(policy, slot_number);
    PoliciesChangedSignal();
    return true;
}

int GovernmentWnd::MainPanel::FindEmptySlotForPolicy(const Policy* policy) const {
    if (!policy)
        return -1;

    // scan through slots to find one that can "mount" policy
    for (unsigned int i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i]->GetPolicy())
            continue;   // slot already occupied
        const std::string& slot_category = m_slots[i]->SlotCategory();
        if (policy->Category() != slot_category)
            continue;
        return i;
    }

    return -1;
}

void GovernmentWnd::MainPanel::ClearPolicies() {
    for (auto& slot : m_slots)
        slot->SetPolicy(nullptr);
    PoliciesChangedSignal();
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
        PoliciesChangedSignal();
}

void GovernmentWnd::MainPanel::Populate(){
    for (const auto& slot: m_slots)
        DetachChild(slot);
    m_slots.clear();

    // todo: loop over policy slots the empire's government has, add slot controls
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = GetEmpire(empire_id);  // may be nullptr
    if (!empire)
        return;

    const std::map<std::string, int> policy_slots = empire->TotalPolicySlots();
    const std::map<std::string, int>& adopted_policy_turns = empire->AdoptedPolicyTurns();

    for (const auto& cat_slots : policy_slots) {
        unsigned int num_slots_in_cat = static_cast<int>(cat_slots.second);
        const std::string& cat_name = cat_slots.first;

        for (unsigned int n = 0; n < num_slots_in_cat; ++n) {
            auto slot_control = GG::Wnd::Create<PolicySlotControl>(cat_name);
            AttachChild(slot_control);
            m_slots.push_back(slot_control);

            slot_control->SlotContentsAlteredSignal.connect(
                boost::bind(static_cast<void (GovernmentWnd::MainPanel::*)(
                    const Policy*, unsigned int, bool)>(&GovernmentWnd::MainPanel::SetPolicy),
                                                        this, _1, n, _2));
            slot_control->PolicyClickedSignal.connect(PolicyClickedSignal);
        }
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

    GG::Pt lr = ClientSize() + GG::Pt(-GG::X(PAD), -GG::Y(PAD));
    GG::Pt ul = lr - GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_clear_button->SizeMove(ul, lr);

    // place background image of hull
    ul.x = GG::X0;
    ul.y = GG::Y0;
    GG::Rect background_rect = GG::Rect(ul, ClientLowerRight());

    if (m_background_image) {
        GG::Pt bg_ul = background_rect.UpperLeft();
        GG::Pt bg_lr = ClientSize();
        m_background_image->SizeMove(bg_ul, bg_lr);
        background_rect = m_background_image->RenderedArea();
    }

    // place slot controls over image of hull
    int count = 0;
    for (auto& slot : m_slots) {
        count++;
        ul.x = (count + 1)*slot->Width() * 5/4;
        ul.y = (count/5 + 1)*slot->Height() * 5/4;
        slot->MoveTo(ul);
    }
}

void GovernmentWnd::MainPanel::PoliciesChanged()
{}

void GovernmentWnd::MainPanel::DropsAcceptable(DropsAcceptableIter first,
                                               DropsAcceptableIter last,
                                               const GG::Pt& pt,
                                               GG::Flags<GG::ModKey> mod_keys) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    // if multiple things dropped simultaneously somehow, reject all
    if (std::distance(first, last) != 1)
        return;
}

void GovernmentWnd::MainPanel::AcceptDrops(const GG::Pt& pt,
                                           std::vector<std::shared_ptr<GG::Wnd>> wnds,
                                           GG::Flags<GG::ModKey> mod_keys)
{}


//////////////////////////////////////////////////
// GovernmentWnd                                    //
//////////////////////////////////////////////////
GovernmentWnd::GovernmentWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::ONTOP | GG::INTERACTIVE),
    m_policy_palette(nullptr),
    m_main_panel(nullptr)
{}

void GovernmentWnd::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    Sound::TempUISoundDisabler sound_disabler;
    SetChildClippingMode(ClipToClient);

    m_main_panel = GG::Wnd::Create<MainPanel>(GOV_MAIN_WND_NAME);
    m_policy_palette = GG::Wnd::Create<PolicyPalette>(GOV_POLICY_PALETTE_WND_NAME);
    InitializeWindows();
    HumanClientApp::GetApp()->RepositionWindowsSignal.connect(
        boost::bind(&GovernmentWnd::InitializeWindows, this));

    AttachChild(m_main_panel);
    m_main_panel->PoliciesChangedSignal.connect(
        boost::bind(&GovernmentWnd::PoliciesChanged, this));
    m_main_panel->Sanitize();

    AttachChild(m_policy_palette);
    m_policy_palette->PolicyDoubleClickedSignal.connect(
        boost::bind(&GovernmentWnd::MainPanel::AddPolicy, m_main_panel, _1));
    m_policy_palette->ClearPolicySignal.connect(
        boost::bind(&GovernmentWnd::MainPanel::ClearPolicy, m_main_panel, _1));
}

void GovernmentWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size()) {
        m_policy_palette->ValidatePosition();
        m_main_panel->ValidatePosition();
    }
}

void GovernmentWnd::Reset() {
    m_policy_palette->Populate();
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

    m_main_panel->      InitSizeMove(main_ul,      main_ul + main_wh);
    m_policy_palette->  InitSizeMove(palette_ul,   palette_ul + palette_wh);
}

void GovernmentWnd::PoliciesChanged()
{}

void GovernmentWnd::EnableOrderIssuing(bool enable/* = true*/)
{}
