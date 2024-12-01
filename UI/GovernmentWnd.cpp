#include "GovernmentWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "QueueListBox.h"   // for PromptRow
#include "IconTextBrowseWnd.h"
#include "Sound.h"
#include "TextBrowseWnd.h"
#include "SidePanel.h"
#include "FleetWnd.h"
#include "../parse/Parse.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Order.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include "../Empire/Empire.h"
#include "../Empire/Government.h"
#include "../client/human/GGHumanClientApp.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/TabWnd.h>

#include <boost/algorithm/string.hpp>

using boost::placeholders::_1;
using boost::placeholders::_2;

enum class Availability : uint8_t {
    Adopted = 0,      // Policy is currently adopted
    Adoptable = 1,    // Policy can be adopted, but hasn't been adopted
    Unaffordable = 2, // Policy is unlocked and has no restrictions, but is too expensive to adopt
    Restricted = 3,   // Policy is unlocked, but is restricted by exclusions and thus cannot be adopted
    Locked = 4        // Policy has not been unlocked and thus cannot be adopted
};

namespace {
#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr std::string EMPTY_STRING;
#else
    const std::string EMPTY_STRING;
#endif
    constexpr std::string_view  POLICY_CONTROL_DROP_TYPE_STRING = "Policy Control";
    constexpr GG::X             SLOT_CONTROL_WIDTH{120};
    constexpr GG::Y             SLOT_CONTROL_HEIGHT{180};
    constexpr int               PAD{3};
    constexpr double            POLICY_PAD{0.125};
    constexpr double            POLICY_TEXT_POS_X{0.0625};
    constexpr double            POLICY_TEXT_POS_Y{0.75};
    constexpr double            POLICY_COST_POS_X{0.75};
    constexpr double            POLICY_COST_POS_Y{0.0625};
    constexpr GG::X             POLICY_SIZE_BUTTON_WIDTH{32};

    /** Returns texture with which to render a PolicySlotControl, depending on
      * \a category */
    std::shared_ptr<GG::Texture> SlotBackgroundTexture(const std::string& category) {
        std::string filename = boost::algorithm::to_lower_copy(category) + "_slot.png";
        return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "policies" / filename, true);
    }

    /** Returns background texture with which to render a PolicyControl,
      * depending on the category of slot that the indicated \a policy can
      * be put into. */
    std::shared_ptr<GG::Texture> PolicyBackgroundTexture(const Policy* policy) {
        if (policy) {
            std::string filename = boost::algorithm::to_lower_copy(policy->Category()) + "_policy.png";
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "policies" / filename, true);
        }
        return ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "missing.png", true);
    }

    //////////////////////////////////////////////////
    //  AvailabilityManager                         //
    //////////////////////////////////////////////////
    /** A class to allow the storage of the state of a GUI availabilty filter
        and the querying of that state WRT a policy. */
    class AvailabilityManager {
    public:
        using DisplayedAvailabilies = std::array<bool, 5>;

        AvailabilityManager(bool adopted, bool adoptable, bool unaffordable, bool restricted, bool locked);

        const DisplayedAvailabilies& GetAvailabilities() const { return m_availabilities; };
        bool GetAvailability(const Availability type) const;
        void SetAvailability(const Availability type, const bool state);
        void ToggleAvailability(const Availability type);

        /** Given the GUI's displayed availabilities as stored in this
            AvailabilityManager, return the displayed state of the \p policy.
            Return none if the \p policy should not be displayed. */
        bool PolicyDisplayed(const Policy& policy, int empire_id = GGHumanClientApp::GetApp()->EmpireID()) const;

    private:
        // A tuple of the toogle state of the 3-tuple of coupled
        // availability filters in the GUI:
        // Restricted, Available and Unavailable
        DisplayedAvailabilies m_availabilities;
    };

    AvailabilityManager::AvailabilityManager(bool adopted, bool adoptable, bool unaffordable,
                                             bool restricted, bool locked) :
        m_availabilities{adopted, adoptable, unaffordable, restricted, locked}
    {}

    bool AvailabilityManager::GetAvailability(Availability availability) const {
        auto idx = static_cast<std::underlying_type_t<Availability>>(availability);
        if (idx >= m_availabilities.size())
            return false;
        return m_availabilities[idx];
    }

    void AvailabilityManager::SetAvailability(const Availability availability, bool state) {
        auto idx = static_cast<std::underlying_type_t<Availability>>(availability);
        if (idx >= m_availabilities.size())
            ErrorLogger() << "AvailabilityManager::SetAvailability passed invalid availability: " << idx;
        else
            m_availabilities[idx] = state;
    }

    void AvailabilityManager::ToggleAvailability(const Availability type)
    { SetAvailability(type, !GetAvailability(type)); }

    bool AvailabilityManager::PolicyDisplayed(const Policy& policy, int empire_id) const {
        const auto [show_adopted, show_adoptable, show_unaffordable, show_restricted, show_locked] = m_availabilities;

        const ScriptingContext& context = GGHumanClientApp::GetApp()->GetContext();
        const auto* empire = context.GetEmpire(empire_id).get();
        if (!empire)
            return true;
        const ScriptingContext source_context{context, ScriptingContext::Source{},
                                              empire->Source(context.ContextObjects()).get()};

        const bool policy_adopted = empire->PolicyAdopted(policy.Name());
        const bool policy_affordable = empire->PolicyAffordable(policy.Name(), source_context);
        const bool policy_restricted = !empire->PolicyPrereqsAndExclusionsOK(policy.Name(), context.current_turn);
        const bool policy_locked = !empire->PolicyAvailable(policy.Name());

        if (policy_adopted && show_adopted)
            return true;

        if (show_locked && policy_locked && !policy_adopted)
            return true;

        if (show_unaffordable && !policy_affordable && !policy_adopted && !policy_restricted && !policy_locked)
            return true;

        if (show_restricted && policy_restricted && !policy_adopted && !policy_locked)
            return true;

        if (show_adoptable && !policy_adopted && policy_affordable && !policy_restricted && !policy_locked)
            return true;

        return false;
    }

    //////////////////////////
    //    PolicyBrowseWnd   //
    //////////////////////////
    std::shared_ptr<GG::BrowseInfoWnd> PolicyBrowseWnd(const std::string& policy_name) {
        const Policy* policy = GetPolicy(policy_name);
        if (!policy)
            return nullptr;

        const auto* app = GGHumanClientApp::GetApp();
        const ScriptingContext& context = app->GetContext();
        const int empire_id = app->EmpireID();

        std::string main_text;
        main_text += UserString(policy->Category()) + " - ";
        main_text += UserString(policy->ShortDescription()) + "\n\n";

        if (const auto empire = context.GetEmpire(empire_id)) {
            bool adopted = empire->PolicyAdopted(policy_name);
            bool available = empire->PolicyAvailable(policy_name);
            bool restricted = !empire->PolicyPrereqsAndExclusionsOK(policy_name, context.current_turn);

            const auto& adoption_cost_template{
                adopted ? UserString("POLICY_ADOPTED") :
                available && !restricted ? UserString("POLICY_ADOPTABLE_COST") :
                restricted ? UserString("POLICY_RESTRICTED") :
                    UserString("POLICY_LOCKED")};
            auto cost = policy->AdoptionCost(empire_id, context);
            main_text += boost::io::str(FlexibleFormat(adoption_cost_template) % cost) + "\n\n";

            bool exclusion_prereq_line_added = false;

            const auto empire_policies = empire->AdoptedPolicies();
            for (auto& ex : policy->Exclusions()) {
                auto ad_it = std::find(empire_policies.begin(), empire_policies.end(), ex);
                if (ad_it == empire_policies.end())
                    continue;
                main_text += boost::io::str(FlexibleFormat(UserString("POLICY_EXCLUDED"))
                                            % UserString(ex)) + "\n";
                exclusion_prereq_line_added = true;
            }

            const auto empire_initial_policies = empire->InitialAdoptedPolicies();
            for (auto& prereq : policy->Prerequisites()) {
                auto init_it = std::find(
                    empire_initial_policies.begin(), empire_initial_policies.end(), prereq);
                const auto& template_str = init_it == empire_initial_policies.end() ?
                    UserString("POLICY_PREREQ_MISSING") : UserString("POLICY_PREREQ_MET");
                main_text += boost::io::str(FlexibleFormat(template_str)
                                            % UserString(prereq)) + "\n";
                exclusion_prereq_line_added = true;
            }
            if (exclusion_prereq_line_added)
                main_text += "\n";

            auto current_adoption_duration = empire->CurrentTurnsPolicyHasBeenAdopted(policy_name);
            auto total_adoption_duration = empire->CumulativeTurnsPolicyHasBeenAdopted(policy_name);
            const auto& adopted_time_timeplate{adopted ?
                UserString("POLICY_ADOPTION_TIMES_CURRENT_AND_TOTAL") : UserString("POLICY_ADOPTION_TIME_TOTAL")};
            main_text += boost::io::str(FlexibleFormat(adopted_time_timeplate)
                                        % current_adoption_duration % total_adoption_duration) + "\n\n";
        }

        main_text += UserString(policy->Description());

        return GG::Wnd::Create<IconTextBrowseWnd>(
            ClientUI::PolicyIcon(policy_name), UserString(policy_name), main_text);
    }
}


//////////////////////////////////////////////////
// PolicyControl                                //
//////////////////////////////////////////////////
/** UI representation of a government policy.  Displayed in the PolicyPalette,
  * and can be dragged onto SlotControls to add policies to the government. */
class PolicyControl : public GG::Control {
public:
    explicit PolicyControl(const Policy* policy);
    void CompleteConstruction() override;

    const std::string&  PolicyName() const  { return m_policy ? m_policy->Name() : EMPTY_STRING; }
    const Policy*       GetPolicy() const   { return m_policy; }

    void Resize(GG::Pt sz, const int pts = ClientUI::Pts());
    void Render() override;

    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;

    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> ClickedSignal;
    mutable boost::signals2::signal<void (const Policy*, GG::Pt pt)> RightClickedSignal;
    mutable boost::signals2::signal<void (const Policy*)> DoubleClickedSignal;

private:
    std::shared_ptr<GG::TextControl>    m_name_label;
    std::shared_ptr<GG::TextControl>    m_cost_label;
    std::shared_ptr<GG::StaticGraphic>  m_icon;
    std::shared_ptr<GG::StaticGraphic>  m_background;
    const Policy*                       m_policy = nullptr;
};

PolicyControl::PolicyControl(const Policy* policy) :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE),
    m_policy(policy)
{}

void PolicyControl::CompleteConstruction() {
    GG::Control::CompleteConstruction();
    if (!m_policy)
        return;
    int empire_id = GGHumanClientApp::GetApp()->EmpireID();
    const ScriptingContext& context = ClientApp::GetApp()->GetContext();
    const auto& name = UserString(m_policy->Name());
    const auto cost = static_cast<int>(m_policy->AdoptionCost(empire_id, context));

    //std::cout << "PolicyControl: " << m_policy->Name() << std::endl;

    m_background = GG::Wnd::Create<GG::StaticGraphic>(PolicyBackgroundTexture(m_policy),
                                                      GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_icon = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::PolicyIcon(m_policy->Name()),
                                                      GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_name_label = GG::Wnd::Create<GG::TextControl>(GG::X0, GG::Y0, GG::X1, GG::Y1, name,
                                                      ClientUI::GetBoldFont(), ClientUI::TextColor(), GG::FORMAT_WORDBREAK);
    m_cost_label = GG::Wnd::Create<GG::TextControl>(GG::X0, GG::Y0, GG::X1, GG::Y1, std::to_string(cost),
                                                      ClientUI::GetBoldFont(), ClientUI::TextColor());

    m_background->Show();
    m_icon->Show();

    AttachChild(m_background);
    AttachChild(m_icon);
    AttachChild(m_name_label);
    AttachChild(m_cost_label);

    Resize(Size());

    //DebugLogger() << "PolicyControl::PolicyControl this: " << this << " policy: " << policy << " named: " << (policy ? policy->Name() : "no policy");
    SetDragDropDataType(POLICY_CONTROL_DROP_TYPE_STRING);

    //DebugLogger() << "PolicyControl::PolicyControl policy name: " << m_policy->Name();
    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    SetBrowseInfoWnd(PolicyBrowseWnd(m_policy->Name()));
}

void PolicyControl::Resize(GG::Pt sz, const int pts) {
    m_background->Resize(sz);
    m_icon->Resize(GG::Pt(sz.x, sz.y * 2/3));

    std::shared_ptr<GG::Font> font = ClientUI::GetBoldFont(pts);

    m_name_label->SetFont(font);
    m_cost_label->SetFont(std::move(font));

    m_name_label->SizeMove(GG::Pt(GG::ToX(sz.x * POLICY_TEXT_POS_X),        GG::ToY(sz.y * POLICY_TEXT_POS_Y)),
                           GG::Pt(GG::ToX(sz.x * (1 - POLICY_TEXT_POS_X)),  GG::ToY(sz.y * POLICY_TEXT_POS_Y)));
    m_cost_label->MoveTo(  GG::Pt(GG::ToX(sz.x * POLICY_COST_POS_X),        GG::ToY(sz.y * (1 - POLICY_COST_POS_Y))));

    GG::Control::Resize(sz);
}

void PolicyControl::Render() {}

void PolicyControl::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ ClickedSignal(m_policy, mod_keys); }

void PolicyControl::LDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ DoubleClickedSignal(m_policy); }

void PolicyControl::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ RightClickedSignal(m_policy, pt); }


//////////////////////////////////////////////////
// PoliciesListBox                              //
//////////////////////////////////////////////////
/** Arrangement of PolicyControls that can be dragged onto SlotControls */
class PoliciesListBox : public CUIListBox {
public:
    class PoliciesListBoxRow : public CUIListBox::Row {
    public:
        PoliciesListBoxRow(GG::X w, GG::Y h, const AvailabilityManager& availabilities_state);
        void ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds,
                                 const GG::Wnd* destination) override;
    private:
        const AvailabilityManager& m_availabilities_state;
    };

    explicit PoliciesListBox(const AvailabilityManager& availabilities_state);

    const auto& GetCategoriesShown() const noexcept { return m_policy_categories_shown; }
    const auto& AvailabilityState() const noexcept { return m_availabilities_state; }

    void SizeMove(GG::Pt ul, GG::Pt lr) override;
    void AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                     GG::Flags<GG::ModKey> mod_keys) override;
    void Populate();

    void ShowCategory(const std::string& category, bool refresh_list = true);
    void ShowAllCategories(bool refresh_list = true);
    void HideCategory(const std::string& category, bool refresh_list = true);
    void HideAllCategories(bool refresh_list = true);
    void ResizePolicies(GG::Pt sz, const int pts = ClientUI::Pts());

    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> PolicyClickedSignal;
    mutable boost::signals2::signal<void (const Policy*)>                        PolicyDoubleClickedSignal;
    mutable boost::signals2::signal<void (const Policy*, GG::Pt pt)>             PolicyRightClickedSignal;
    mutable boost::signals2::signal<void (const std::string&)>                   ClearPolicySignal;

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    // policies indexed by category that conform to current availability state
    std::map<std::string, std::vector<const Policy*>>
    GroupAvailableDisplayablePolicies(const Empire* empire) const;

    mutable boost::signals2::scoped_connection           m_empire_policies_changed_signal_connection;
    boost::container::flat_set<std::string, std::less<>> m_policy_categories_shown;
    int                                                  m_previous_num_columns = -1;
    const AvailabilityManager&                           m_availabilities_state;
};

PoliciesListBox::PoliciesListBoxRow::PoliciesListBoxRow(
    GG::X w, GG::Y h, const AvailabilityManager& availabilities_state) :
    CUIListBox::Row(w, h),
    m_availabilities_state(availabilities_state)
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

    const auto* policy_control = dynamic_cast<PolicyControl*>(control);
    if (!policy_control)
        return;

    const auto* policy_type = policy_control->GetPolicy();
    if (!policy_type)
        return;

    auto new_policy_control = GG::Wnd::Create<PolicyControl>(policy_type);
    if (auto parent = std::dynamic_pointer_cast<const PoliciesListBox>(Parent())) {
        new_policy_control->ClickedSignal.connect(parent->PolicyClickedSignal);
        new_policy_control->DoubleClickedSignal.connect(parent->PolicyDoubleClickedSignal);
        new_policy_control->RightClickedSignal.connect(parent->PolicyRightClickedSignal);
    }

    SetCell(ii, new_policy_control);
}

PoliciesListBox::PoliciesListBox(const AvailabilityManager& availabilities_state) :
    CUIListBox(),
    m_availabilities_state(availabilities_state)
{
    ManuallyManageColProps();
    NormalizeRowsOnInsert(false);
    SetStyle(GG::LIST_NOSEL);
}

void PoliciesListBox::SizeMove(GG::Pt ul, GG::Pt lr) {
    const GG::Pt old_size = GG::Wnd::Size();

    const auto policy_palette = Parent();
    const auto gov_wnd = std::dynamic_pointer_cast<GovernmentWnd>(policy_palette->Parent());

    const GG::Pt slot_size = gov_wnd ? gov_wnd->GetPolicySlotSize() : GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT);

    // maybe later do something interesting with docking
    CUIListBox::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size()) {
        // determine how many columns can fit in the box now...
        const GG::X TOTAL_WIDTH = Size().x - ClientUI::ScrollWidth();
        const int NUM_COLUMNS = std::max(1, TOTAL_WIDTH / (slot_size.x + PAD));

        if (NUM_COLUMNS != m_previous_num_columns)
            Populate();
    }
}

void PoliciesListBox::AcceptDrops(GG::Pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                                  GG::Flags<GG::ModKey> mod_keys)
{
    // If ctrl is pressed then signal all policies of the same type to be cleared.
    if (!(mod_keys & GG::MOD_KEY_CTRL))
        return;

    if (wnds.empty())
        return;

    auto control = std::dynamic_pointer_cast<const PolicyControl>(wnds.front());
    const Policy* policy_type = control ? control->GetPolicy() : nullptr;
    if (!policy_type)
        return;

    ClearPolicySignal(policy_type->Name());
}

std::map<std::string, std::vector<const Policy*>>
PoliciesListBox::GroupAvailableDisplayablePolicies(const Empire*) const {
    using PolicyAndCat = const std::pair<const Policy*, const std::string&>;
    static constexpr auto to_policy_and_cat = [](const Policy& p) -> PolicyAndCat { return {&p, p.Category()}; };
    const auto cat_shown_policy_displayed = [this](PolicyAndCat& p_c) {
        return m_policy_categories_shown.contains(p_c.second) &&
            m_availabilities_state.PolicyDisplayed(*p_c.first);
    };

    // loop through all possible policies, outputting those shown
    std::map<std::string, std::vector<const Policy*>> policies_categorized;
    auto policies_rng = std::as_const(GetPolicyManager()) | range_values;
    auto policy_cat_rng = policies_rng | range_transform(to_policy_and_cat)
        | range_filter(cat_shown_policy_displayed);
    for (const auto& [policy, category] : policy_cat_rng)
        policies_categorized[category].push_back(policy);
    return policies_categorized;
}

void PoliciesListBox::Populate() {
    //std::cout << "PoliciesListBox::Populate" << std::endl;
    ScopedTimer scoped_timer("PoliciesListBox::Populate");

    GG::Pt slot_size = GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT);

    auto policy_palette = Parent();
    if (auto gov_wnd = std::dynamic_pointer_cast<GovernmentWnd>(policy_palette->Parent()))
        slot_size = gov_wnd->GetPolicySlotSize();

    const GG::X TOTAL_WIDTH = ClientWidth() - ClientUI::ScrollWidth();
    const int MAX_COLUMNS = std::max(1, TOTAL_WIDTH / (slot_size.x + PAD));

    const int empire_id = GGHumanClientApp::GetApp()->EmpireID();
    const Empire* empire = GetEmpire(empire_id);  // may be nullptr

    m_empire_policies_changed_signal_connection.disconnect();
    if (empire)
        m_empire_policies_changed_signal_connection = empire->PoliciesChangedSignal.connect(
            boost::bind(&PoliciesListBox::Populate, this), boost::signals2::at_front);

    int cur_col = MAX_COLUMNS;
    std::shared_ptr<PoliciesListBoxRow> cur_row;
    int num_policies = 0;

    // remove policies currently in rows of listbox
    Clear();

    // filter policies by availability and current designation of categories
    // for display
    const auto policies = GroupAvailableDisplayablePolicies(empire);
    for (const auto& policies_vec : policies | range_values) { // TODO: if std::views::join is available, avoid douple loop by flattening
        // take the sorted policies and make UI element rows for the PoliciesListBox
        for (const auto policy : policies_vec) {
            // check if current row is full, and make a new row if necessary
            if (cur_col >= MAX_COLUMNS) {
                if (cur_row)
                    Insert(cur_row);
                cur_col = 0;
                cur_row = GG::Wnd::Create<PoliciesListBoxRow>(
                    TOTAL_WIDTH, slot_size.y + GG::Y(PAD), m_availabilities_state);
            }
            ++cur_col;
            ++num_policies;

            // make new policy control and add to row
            auto control = GG::Wnd::Create<PolicyControl>(policy);
            control->ClickedSignal.connect(PoliciesListBox::PolicyClickedSignal);
            control->DoubleClickedSignal.connect(PoliciesListBox::PolicyDoubleClickedSignal);
            control->RightClickedSignal.connect(PoliciesListBox::PolicyRightClickedSignal);

            cur_row->push_back(std::move(control));
        }
    }
    // add any incomplete rows
    if (cur_row)
        Insert(std::move(cur_row));

    // keep track of how many columns are present now
    m_previous_num_columns = MAX_COLUMNS;

    // If there are no policies add a prompt to suggest a solution.
    if (num_policies == 0)
        Insert(GG::Wnd::Create<PromptRow>(TOTAL_WIDTH, UserString("ALL_POLICY_AVAILABILITY_FILTERS_BLOCKING_PROMPT")),
               begin(), false);
}

void PoliciesListBox::ShowCategory(const std::string& category, bool refresh_list) {
    if (!m_policy_categories_shown.contains(category)) {
        m_policy_categories_shown.insert(category);
        if (refresh_list)
            Populate();
    }
}

void PoliciesListBox::ShowAllCategories(bool refresh_list) {
    const auto cats = GetPolicyManager().PolicyCategories();
    std::transform(cats.begin(), cats.end(), std::inserter(m_policy_categories_shown, m_policy_categories_shown.end()),
                   [](const auto sv) { return std::string{sv}; });
    if (refresh_list)
        Populate();
}

void PoliciesListBox::HideCategory(const std::string& category, bool refresh_list) {
    if (m_policy_categories_shown.erase(category) > 0 && refresh_list)
        Populate();
}

void PoliciesListBox::HideAllCategories(bool refresh_list) {
    m_policy_categories_shown.clear();
    if (refresh_list)
        Populate();
}

void PoliciesListBox::ResizePolicies(GG::Pt sz, const int pts) {
    auto it = begin();
    while (it != end()) {
        const auto& row = *it;
        unsigned int itt = 0;
        while (itt < row->size()) {
            auto policy_control = dynamic_cast<PolicyControl*>(row->at(itt));
            if (policy_control)
                policy_control->Resize(sz, pts);
            ++itt;
        }
        ++it;
    }
}

//////////////////////////////////////////////////
// GovernmentWnd::PolicyPalette                 //
//////////////////////////////////////////////////
/** Contains graphical list of PolicyControl which can be dragged and dropped
  * onto slots to assign policies to those slots */
class GovernmentWnd::PolicyPalette : public GG::Wnd {
public:
    PolicyPalette(GG::X w, GG::Y h);
    void CompleteConstruction() override;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void ShowCategory(const std::string& category, bool refresh_list = true);
    void ShowAllCategories(bool refresh_list = true);
    void HideCategory(const std::string& category, bool refresh_list = true);
    void HideAllCategories(bool refresh_list = true);
    void ToggleCategory(const std::string& category, bool refresh_list = true);
    void ToggleAllCategories(bool refresh_list = true);

    void ToggleAvailability(const Availability type);

    void Populate();

    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> PolicyClickedSignal;
    mutable boost::signals2::signal<void (const Policy*)> PolicyDoubleClickedSignal;
    mutable boost::signals2::signal<void (const Policy*, GG::Pt pt)> PolicyRightClickedSignal;
    mutable boost::signals2::signal<void (const std::string&)> ClearPolicySignal;

private:
    void DoLayout();

    /** A policy type click with ctrl obsoletes policy. */
    void HandlePolicyClicked(const Policy*, GG::Flags<GG::ModKey>);
    void HandlePolicyRightClicked(const Policy*, GG::Pt pt);

    std::shared_ptr<PoliciesListBox>                        m_policies_list;
    std::map<std::string, std::shared_ptr<CUIStateButton>>  m_category_buttons;
    AvailabilityManager                                     m_availabilities_state{true, true, false, false, false};
    static constexpr std::size_t NUM_AVAILABILITY_BUTTONS = 5;
    std::array<std::shared_ptr<CUIStateButton>, NUM_AVAILABILITY_BUTTONS> m_availabilities_buttons{};
};

GovernmentWnd::PolicyPalette::PolicyPalette(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::ONTOP | GG::INTERACTIVE)
{}

void GovernmentWnd::PolicyPalette::CompleteConstruction() {
    SetChildClippingMode(ChildClippingMode::ClipToClient);

    m_policies_list = GG::Wnd::Create<PoliciesListBox>(m_availabilities_state);
    AttachChild(m_policies_list);
    m_policies_list->PolicyClickedSignal.connect(
        boost::bind(&GovernmentWnd::PolicyPalette::HandlePolicyClicked, this, _1, _2));
    m_policies_list->PolicyDoubleClickedSignal.connect(
        PolicyDoubleClickedSignal);
    m_policies_list->PolicyRightClickedSignal.connect(
        boost::bind(&GovernmentWnd::PolicyPalette::HandlePolicyRightClicked, this, _1, _2));
    m_policies_list->ClearPolicySignal.connect(ClearPolicySignal);

    // class buttons
    for (auto& cat_view : GetPolicyManager().PolicyCategories()) {
        // are there any policies of this class?
        if (std::none_of(GetPolicyManager().begin(), GetPolicyManager().end(),
                         [cat_view](auto& e){ return cat_view == e.second.Category(); }))
        { continue; }

        const auto& us_cateory{UserString(cat_view)};
        auto ptr_it = m_category_buttons.emplace(std::string{cat_view}, GG::Wnd::Create<CUIStateButton>(
            us_cateory, GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>())).first;
        auto& [string_in_map, ptr_in_map] = *ptr_it;
        AttachChild(ptr_in_map);
        ptr_in_map->CheckedSignal.connect(
            boost::bind(&GovernmentWnd::PolicyPalette::ToggleCategory, this, string_in_map, true));
    }

    static constexpr std::array<std::string_view, 5> BUTTON_LABELS = {{
            UserStringNop("POLICY_LIST_ADOPTED"), UserStringNop("POLICY_LIST_ADOPTABLE"),
            UserStringNop("POLICY_LIST_UNAFFORDABLE"), UserStringNop("POLICY_LIST_RESTRICTED"),
            UserStringNop("POLICY_LIST_LOCKED")}};
    for (std::size_t n = 0; n < 5; ++n) {
        auto& button = m_availabilities_buttons[n];
        button = GG::Wnd::Create<CUIStateButton>(UserString(BUTTON_LABELS[n]),
                                                 GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
        AttachChild(button);
        button->CheckedSignal.connect(
            boost::bind(&GovernmentWnd::PolicyPalette::ToggleAvailability, this, Availability(n)));
        button->SetCheck(m_availabilities_state.GetAvailability(Availability(n)));
    }

    // default to showing everything
    ShowAllCategories(false);
    Populate();

    GG::Wnd::CompleteConstruction();
}

void GovernmentWnd::PolicyPalette::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Wnd::SizeMove(ul, lr);
    DoLayout();
}

void GovernmentWnd::PolicyPalette::DoLayout() {
    const int     PTS = ClientUI::Pts();
    const GG::X   PTS_WIDE{PTS/2};        // guess at how wide per character the font needs
    const GG::Y   BUTTON_HEIGHT{PTS*3/2};
    static constexpr int BUTTON_SEPARATION = 3;  // vertical or horizontal sepration between adjacent buttons
    static constexpr int BUTTON_EDGE_PAD = 2;    // distance from edges of control to buttons
    static constexpr GG::X RIGHT_EDGE_PAD{8};    // to account for border of CUIWnd

    const GG::X USABLE_WIDTH = std::max(ClientWidth() - RIGHT_EDGE_PAD, GG::X1);// space in which to fit buttons
    static constexpr int GUESSTIMATE_NUM_CHARS_IN_BUTTON_LABEL = 14; // rough guesstimate... avoid overly long policy class names
    const GG::X MIN_BUTTON_WIDTH = PTS_WIDE*GUESSTIMATE_NUM_CHARS_IN_BUTTON_LABEL;
    const std::size_t MAX_BUTTONS_PER_ROW = std::max(USABLE_WIDTH / (MIN_BUTTON_WIDTH + BUTTON_SEPARATION), 1);

    const std::size_t NUM_CATEGORY_BUTTONS = std::max(1, static_cast<int>(m_category_buttons.size()));
    const std::size_t TOTAL_BUTTONS = NUM_CATEGORY_BUTTONS + NUM_AVAILABILITY_BUTTONS;

    // determine how to arrange category and availability buttons into rows and columns
    // eg:
    // [CAT1] [CAT2] [CAT3] [AVB1] [AVB2] [AVB3] [AVB4] [AVB5]
    //
    // [CAT1] [CAT2] [AVB1] [AVB2] [AVB3]
    // [CAT3]        [AVB4] [AVB5]
    //
    // [CAT1] [AVB1] [AVB2]
    // [CAT2] [AVB3] [AVB4]
    // [CAT3] [AVB5]

    float NUM_ROWS = 1.0f;
    auto AVAILABILITY_BUTTONS_PER_ROW = static_cast<std::size_t>(std::ceil(NUM_AVAILABILITY_BUTTONS / NUM_ROWS));
    auto CATEGORY_BUTTONS_PER_ROW = static_cast<std::size_t>(std::ceil(NUM_CATEGORY_BUTTONS / NUM_ROWS));
    int TOTAL_BUTTONS_PER_ROW = AVAILABILITY_BUTTONS_PER_ROW + CATEGORY_BUTTONS_PER_ROW;
    while (TOTAL_BUTTONS_PER_ROW > static_cast<int>(MAX_BUTTONS_PER_ROW) && NUM_ROWS < TOTAL_BUTTONS) {
        NUM_ROWS += 1.0f;
        AVAILABILITY_BUTTONS_PER_ROW = static_cast<std::size_t>(std::ceil(NUM_AVAILABILITY_BUTTONS / NUM_ROWS));
        CATEGORY_BUTTONS_PER_ROW = static_cast<std::size_t>(std::ceil(NUM_CATEGORY_BUTTONS / NUM_ROWS));
        TOTAL_BUTTONS_PER_ROW = AVAILABILITY_BUTTONS_PER_ROW + CATEGORY_BUTTONS_PER_ROW;
    }

    //const std::size_t NUM_CATEGORY_BUTTON_ROWS = static_cast<std::size_t>(std::ceil(
    //    NUM_CATEGORY_BUTTONS * 1.0f / CATEGORY_BUTTONS_PER_ROW));
    //const std::size_t NUM_AVAILABILITY_BUTTON_ROWS = static_cast<std::size_t>(std::ceil(
    //    NUM_AVAILABILITY_BUTTONS * 1.0f / AVAILABILITY_BUTTONS_PER_ROW));

    const GG::X BUTTON_WIDTH = (USABLE_WIDTH - (TOTAL_BUTTONS_PER_ROW - 1)*BUTTON_SEPARATION) / TOTAL_BUTTONS_PER_ROW;

    const GG::X COL_OFFSET = BUTTON_WIDTH + BUTTON_SEPARATION;    // horizontal distance between each column of buttons
    const GG::Y ROW_OFFSET = BUTTON_HEIGHT + BUTTON_SEPARATION;   // vertical distance between each row of buttons


    // place category buttons
    int col = CATEGORY_BUTTONS_PER_ROW;
    int row = -1;
    for (auto& entry : m_category_buttons) {
        if (col >= static_cast<int>(CATEGORY_BUTTONS_PER_ROW)) {
            col = 0;
            ++row;
        }
        GG::Pt ul(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        entry.second->SizeMove(ul, lr);
        ++col;
    }
    const auto NUM_CATEGORY_BUTTON_ROWS = row;


    // reset row / column
    col = CATEGORY_BUTTONS_PER_ROW;
    row = 0;

    // place availability buttons
    for (auto& button : m_availabilities_buttons) {
        if (col >= TOTAL_BUTTONS_PER_ROW) {
            col = CATEGORY_BUTTONS_PER_ROW;
            row++;
        }

        auto ul = GG::Pt{BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET};
        auto lr = ul + GG::Pt{BUTTON_WIDTH, BUTTON_HEIGHT};
        button->SizeMove(ul, lr);
        col++;
    }

    const auto TOTAL_ROWS = std::max(NUM_CATEGORY_BUTTON_ROWS, row);
    // place policies list
    m_policies_list->SizeMove(GG::Pt{GG::X0, BUTTON_EDGE_PAD + ROW_OFFSET*(TOTAL_ROWS + 1)},
                              GG::Pt{ClientWidth(), ClientHeight() - GG::Y{INNER_BORDER_ANGLE_OFFSET}});

    // adjust size of policies
    GG::Pt slot_size = GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT);
    int text_pts = ClientUI::Pts();

    auto gov_wnd = std::dynamic_pointer_cast<GovernmentWnd>(Parent());
    if (gov_wnd) {
        slot_size = gov_wnd->GetPolicySlotSize();
        text_pts = gov_wnd->GetPolicyTextSize();
    }

    m_policies_list->ResizePolicies(slot_size, text_pts);
}

void GovernmentWnd::PolicyPalette::HandlePolicyClicked(const Policy* policy_type,
                                                       GG::Flags<GG::ModKey> modkeys)
{ PolicyClickedSignal(policy_type, modkeys); }

void GovernmentWnd::PolicyPalette::HandlePolicyRightClicked(const Policy* policy_type,
                                                            GG::Pt pt)
{ PolicyRightClickedSignal(policy_type, pt); }

void GovernmentWnd::PolicyPalette::ShowCategory(const std::string& category,
                                                bool refresh_list)
{
    if (!m_category_buttons.contains(category)) {
        ErrorLogger() << "PolicyPalette::ShowCategory was passed an invalid category name: " << category;
        return;
    }
    m_policies_list->ShowCategory(category, refresh_list);
    m_category_buttons[category]->SetCheck();
    DoLayout();
}

void GovernmentWnd::PolicyPalette::ShowAllCategories(bool refresh_list) {
    m_policies_list->ShowAllCategories(refresh_list);
    for (auto& entry : m_category_buttons)
        entry.second->SetCheck();
    DoLayout();
}

void GovernmentWnd::PolicyPalette::HideCategory(const std::string& category,
                                                bool refresh_list)
{
    if (!m_category_buttons.contains(category)) {
        ErrorLogger() << "PolicyPalette::HideCategory was passed an invalid category name: " << category;
        return;
    }
    m_policies_list->HideCategory(category, refresh_list);
    m_category_buttons[category]->SetCheck(false);
    DoLayout();
}

void GovernmentWnd::PolicyPalette::HideAllCategories(bool refresh_list) {
    m_policies_list->HideAllCategories(refresh_list);
    for (auto& entry : m_category_buttons)
        entry.second->SetCheck(false);
    DoLayout();
}

void GovernmentWnd::PolicyPalette::ToggleCategory(const std::string& category, bool refresh_list) {
    if (!m_category_buttons.contains(category)) {
        ErrorLogger() << "PolicyPalette::ToggleCategory was passed an invalid category name: " << category;
        return;
    }

    const auto& categories_shown = m_policies_list->GetCategoriesShown();
    if (categories_shown.contains(category))
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

void GovernmentWnd::PolicyPalette::ToggleAvailability(Availability type) {
    auto idx = static_cast<std::underlying_type_t<Availability>>(type);
    if (idx >= m_availabilities_buttons.size())
        return;
    m_availabilities_state.ToggleAvailability(type);
    bool state = m_availabilities_state.GetAvailability(type);

    auto& button = m_availabilities_buttons[idx];
    button->SetCheck(state);
    Populate();
}

void GovernmentWnd::PolicyPalette::Populate() {
    m_policies_list->Populate();
    DoLayout();
}


//////////////////////////////////////////////////
// PolicySlotControl                            //
//////////////////////////////////////////////////
/** UI representation and drop-target for policies of a government.
  * PolicyControl may be dropped into slots to add the corresponding policies to
  * the government, or the policy may be set programmatically with SetPolicy(). */
class PolicySlotControl : public GG::Control {
public:
    PolicySlotControl();
    PolicySlotControl(std::string slot_category, int category_index, std::size_t slot_index);
    void CompleteConstruction() override;

    [[nodiscard]] const auto&   SlotCategory() const noexcept  { return m_slot_category; }
    [[nodiscard]] const Policy* GetPolicy() const;
    [[nodiscard]] auto          CategoryIndex() const noexcept { return m_category_index; }
    [[nodiscard]] auto          SlotIndex() const noexcept     { return m_slot_index; }

    void StartingChildDragDrop(const GG::Wnd* wnd, GG::Pt offset) override;
    void CancellingChildDragDrop(const std::vector<const GG::Wnd*>& wnds) override;
    void AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                     GG::Flags<GG::ModKey> mod_keys) override;
    void ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds,
                             const GG::Wnd* destination) override;
    void DragDropEnter(GG::Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                       GG::Flags<GG::ModKey> mod_keys) override;
    void DragDropLeave() override;
    void Resize(GG::Pt sz, const int pts = ClientUI::Pts());
    void Render() override;

    void Highlight(bool actually = true);

    void SetPolicy(const std::string& policy_name); //!< used to programmatically set the PolicyControl in this slot.
    void SetPolicy(const Policy* policy);           //!< used to programmatically set the PolicyControl in this slot.

    /** emitted when the contents of a slot are altered by the dragging
      * a PolicyControl in or out of the slot.  signal should be caught and the
      * slot contents set using SetPolicy accordingly */
    mutable boost::signals2::signal<void (const Policy*, bool)> SlotContentsAlteredSignal;
    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> PolicyClickedSignal;

protected:
    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event) override;
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    std::string                        m_slot_category;
    std::shared_ptr<PolicyControl>     m_policy_control;
    std::shared_ptr<GG::StaticGraphic> m_background;
    int                                m_category_index = -1;
    std::size_t                        m_slot_index = 0;
    bool                               m_highlighted = false;
};

PolicySlotControl::PolicySlotControl() :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE)
{}

PolicySlotControl::PolicySlotControl(std::string slot_category, int category_index,
                                     std::size_t slot_index) :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE),
    m_slot_category(std::move(slot_category)),
    m_category_index(category_index),
    m_slot_index(slot_index)
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
    case GG::WndEvent::EventType::DragDropEnter:
    case GG::WndEvent::EventType::DragDropHere:
    case GG::WndEvent::EventType::CheckDrops:
    case GG::WndEvent::EventType::DragDropLeave:
    case GG::WndEvent::EventType::DragDroppedOn:
        HandleEvent(event);
        return true;
        break;
    default:
        return false;
    }
}

void PolicySlotControl::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                        GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    // if more than one control dropped somehow, reject all
    if (std::distance(first, last) != 1)
        return;

    const ScriptingContext& context = ClientApp::GetApp()->GetContext();

    int empire_id = GGHumanClientApp::GetApp()->EmpireID();
    auto empire = context.GetEmpire(empire_id);
    if (!empire)
        return;

    for (DropsAcceptableIter it = first; it != last; ++it) {
        if (it->first->DragDropDataType() != POLICY_CONTROL_DROP_TYPE_STRING)
            continue;
        const auto* policy_control = dynamic_cast<const PolicyControl*>(it->first);
        const Policy* policy = policy_control ? policy_control->GetPolicy() : nullptr;
        if (policy &&
            policy->Category() == m_slot_category &&
            policy_control != m_policy_control.get() &&
            empire->PolicyAvailable(policy->Name()) &&
            empire->PolicyPrereqsAndExclusionsOK(policy->Name(), context.current_turn))
        {
            it->second = true;
            return;
        }
    }
}

const Policy* PolicySlotControl::GetPolicy() const {
    if (m_policy_control)
        return m_policy_control->GetPolicy();
    else
        return nullptr;
}

void PolicySlotControl::StartingChildDragDrop(const GG::Wnd* wnd, GG::Pt offset) {
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

void PolicySlotControl::AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                                    GG::Flags<GG::ModKey> mod_keys)
{
    if (wnds.size() != 1)
        ErrorLogger() << "PolicySlotControl::AcceptDrops given multiple wnds unexpectedly...";

    const auto* wnd = wnds.front().get();
    auto* control = dynamic_cast<const PolicyControl*>(wnd);
    if (const Policy* policy_type = control ? control->GetPolicy() : nullptr)
        SlotContentsAlteredSignal(policy_type, mod_keys & GG::MOD_KEY_CTRL);
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

void PolicySlotControl::DragDropEnter(GG::Pt pt,
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

void PolicySlotControl::Resize(GG::Pt sz, const int pts) {
    if (m_policy_control)
        m_policy_control->Resize(sz, pts);

    if (m_background)
        m_background->Resize(sz);

    GG::Control::Resize(sz);
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

    m_policy_control->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
        ClientUI::PolicyIcon(policy->Name()),
        UserString(policy->Name()) + " (" + UserString(policy->Category()) + ")",
        UserString(policy->Description())
    ));
}

/** PoliciesListBox accepts policies that are being removed from a PolicySlotControl.*/
void PoliciesListBox::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                      GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const
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
class GovernmentWnd::MainPanel : public GG::Wnd {
public:
    MainPanel(GG::X w, GG::Y h);
    void CompleteConstruction() override;

    std::vector<std::string> Policies() const; //!< returns vector of names of policies in slots of current shown design.  empty slots are represented with empty string

    //void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;
    void Sanitize();
    void Refresh();

    void SetPolicy(const std::string& policy_name, unsigned int slot,
                   bool update_empire_and_refresh = true);      //!< puts specified policy in specified slot.  does nothing if slot is out of range of available slots for category
    void SetPolicy(const Policy* policy, unsigned int slot,
                   bool update_empire_and_refresh = true);      //!< Sets the policy in \p slot to \p policy
    void SetPolicies(const std::vector<std::string>& policies); //!< puts specified policies in slots.  attempts to put each policy into the slot corresponding to its place in the passed vector.  if a policy cannot be placed, it is ignored.  more policies than there are slots available are ignored, and slots for which there are insufficient policies in the passed vector are unmodified

    /** If a suitable slot is available, adds the specified policy to the government. */
    void AddPolicy(const Policy* policy);
    bool CanPolicyBeAdded(const Policy* policy) const;
    void RevertPolicies();
    void ClearPolicies();   //!< removes all policies from government
    void ClearPolicy(const std::string& policy_name);


    mutable boost::signals2::signal<void (const Policy*, GG::Flags<GG::ModKey>)> PolicyClickedSignal;

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    void PostChangeBigUpdate();

    void Populate();        //!< creates and places SlotControls for empire
    void DoLayout();        //!< positions SlotControls
    bool AddPolicyEmptySlot(const Policy* policy, int slot_number);
    int FindEmptySlotForPolicy(const Policy* policy) const;

    std::vector<std::shared_ptr<PolicySlotControl>> m_slots;
};

GovernmentWnd::MainPanel::MainPanel(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::ONTOP | GG::INTERACTIVE)
{}

void GovernmentWnd::MainPanel::CompleteConstruction() {
    SetChildClippingMode(ChildClippingMode::ClipToClient);

    GG::Wnd::CompleteConstruction();

    DoLayout();
}

std::vector<std::string> GovernmentWnd::MainPanel::Policies() const {
    std::vector<std::string> retval;
    retval.reserve(m_slots.size());
    for (const auto& slot : m_slots) {
        const Policy* policy_type = slot->GetPolicy();
        if (policy_type)
            retval.push_back(policy_type->Name());
        else
            retval.emplace_back();
    }
    return retval;
}

void GovernmentWnd::MainPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void GovernmentWnd::MainPanel::Sanitize()
{ void ClearPolicies(); }

void GovernmentWnd::MainPanel::Refresh() {
    Populate();
    DoLayout();
}

void GovernmentWnd::MainPanel::SetPolicy(const std::string& policy_name, unsigned int slot,
                                         bool update_empire_and_refresh)
{ SetPolicy(GetPolicy(policy_name), slot, update_empire_and_refresh); }

namespace {
    // returns vector of category names and indices within category
    auto ConcatenatedCategorySlots(const Empire* empire) {
        std::vector<std::pair<int, std::string_view>> retval;
        if (!empire)
            return retval;

        // for every slot in every category, add entry to retval in series. count up separately
        // for each slot in each category, to find index for each slot in that cateogry
        for (auto& [cat_name, num_slots_in_cat] : empire->TotalPolicySlots()) {
            for (unsigned int n = 0; n < static_cast<unsigned int>(std::max(0, num_slots_in_cat)); ++n) // treat negative numbers of slots as 0
                retval.emplace_back(n, cat_name);
        }

        return retval;
    }

    std::pair<int, std::string_view> OverallSlotToCategoryAndSlot(const Empire* empire, int overall_slot) {
        if (!empire || overall_slot < 0)
            return {0, ""};

        auto empire_slots = ConcatenatedCategorySlots(empire);
        if (overall_slot >= static_cast<int>(empire_slots.size()) || overall_slot < 0)
            return {0, ""};

        return empire_slots[overall_slot];
    }
}

void GovernmentWnd::MainPanel::SetPolicy(const Policy* policy, unsigned int slot,
                                         bool update_empire_and_refresh)
{
    DebugLogger() << "GovernmentWnd::MainPanel::SetPolicy(" << (policy ? policy->Name() : "no policy") << ", slot " << slot << ")";

    if (slot >= m_slots.size()) {
        ErrorLogger() << "GovernmentWnd::MainPanel::SetPolicy specified nonexistant slot";
        return;
    }

    auto* app = GGHumanClientApp::GetApp();
    ScriptingContext& context = app->GetContext();
    const int empire_id = app->EmpireID();

    auto empire = std::as_const(context).GetEmpire(empire_id);  // may be nullptr
    if (!empire) {
        ErrorLogger() << "GovernmentWnd::MainPanel::SetPolicy has no empire to set policies for";
        return;
    }

    // what category and slot is policy being adopted in
    const auto [adopt_in_category_slot, adopt_in_category] = OverallSlotToCategoryAndSlot(empire.get(), slot);
    if (adopt_in_category.empty()) {
        ErrorLogger() << "GovernmentWnd::MainPanel::SetPolicy specified invalid slot: " << slot;
        return;
    }

    // what slots are available...
    const auto total_policy_slots = empire->TotalPolicySlots();
    const auto total_policy_slots_it = std::find_if(total_policy_slots.begin(), total_policy_slots.end(),
                                                    [aic{adopt_in_category}](const auto& cat_slots)
                                                    { return cat_slots.first == aic; });
    if (total_policy_slots_it == total_policy_slots.end()) {
        ErrorLogger() << "GovernmentWnd::MainPanel::SetPolicy asked to adopt in category " << adopt_in_category << " which has no slots";
        return;
    }
    if (total_policy_slots_it->second <= adopt_in_category_slot) {
        ErrorLogger() << "GovernmentWnd::MainPanel::SetPolicy asked to adopt in category " << adopt_in_category_slot
                      << " and slot " << adopt_in_category_slot << " which is not an existing slot (max: " << total_policy_slots_it->second;
        return;
    }


    // what, if anything, is already in that slot?
    // category -> slot in category -> policy in slot
    auto initial_cats_slots_policy_adopted = empire->CategoriesSlotsPoliciesAdopted();
    auto& init_slots_adopted{initial_cats_slots_policy_adopted[adopt_in_category]};
    const std::string_view initial_policy_name{init_slots_adopted[adopt_in_category_slot]};

    // check if adopting or revoking a policy. If adopting, then pass along the name of
    // the policy to adopt. If de-adeopting, then pass the name of the policy to de-adopt.
    const bool adopt = !!policy;

    if (!adopt && initial_policy_name.empty()) {
        DebugLogger() << "GovernmentWnd::MainPanel::SetPolicy requested to de-adopt policy in slot " << slot
                      << " but that slot is already empty";
        return;
    }

    std::string order_policy_name{adopt ? policy->Name() : initial_policy_name};
    if (adopt && initial_policy_name == order_policy_name) {
        DebugLogger() << "GovernmentWnd::MainPanel::SetPolicy requested to adopt policy " << order_policy_name
                      << " in slot " << slot << " but that policy is already in that slot";
        return;
    }

    // issue order to adopt or revoke
    if (adopt)
        app->Orders().IssueOrder<PolicyOrder>(context, empire_id, std::move(order_policy_name),
                                              std::string{adopt_in_category}, adopt_in_category_slot);
    else
        app->Orders().IssueOrder<PolicyOrder>(context, empire_id, std::move(order_policy_name));

    if (update_empire_and_refresh)
        PostChangeBigUpdate();
}

void GovernmentWnd::MainPanel::PostChangeBigUpdate() {
    auto* app = GGHumanClientApp::GetApp();
    auto& context = app->GetContext();

    const int empire_id = app->EmpireID();
    auto empire = context.GetEmpire(empire_id);  // may be nullptr
    if (!empire) {
        ErrorLogger() << "GovernmentWnd::MainPanel::SetPolicy has no empire to set policies for";
        return;
    }

    empire->UpdateInfluenceSpending(context, empire->PlanetAnnexationCosts(context),
                                    empire->PolicyAdoptionCosts(context));
    Populate();
    DoLayout();
    if (auto gov_wnd = std::dynamic_pointer_cast<GovernmentWnd>(Parent()))
        gov_wnd->DoLayout();
    context.ContextUniverse().UpdateMeterEstimates(context);
    SidePanel::Refresh();
    FleetUIManager::GetFleetUIManager().RefreshAll(empire_id, context);
}

void GovernmentWnd::MainPanel::SetPolicies(const std::vector<std::string>& policies) {
    ClearPolicies();

    const auto num_policies = std::min(policies.size(), m_slots.size());
    for (decltype(policies.size()) slot = 0u; slot < num_policies; ++slot)
        this->SetPolicy(policies[slot], slot, false);

    PostChangeBigUpdate();
}

void GovernmentWnd::MainPanel::AddPolicy(const Policy* policy)
{ AddPolicyEmptySlot(policy, FindEmptySlotForPolicy(policy)); }

bool GovernmentWnd::MainPanel::CanPolicyBeAdded(const Policy* policy) const
{ return FindEmptySlotForPolicy(policy) >= 0; }

bool GovernmentWnd::MainPanel::AddPolicyEmptySlot(const Policy* policy, int slot_number) {
    if (!policy || slot_number < 0)
        return false;
    SetPolicy(policy, slot_number, true);
    return true;
}

int GovernmentWnd::MainPanel::FindEmptySlotForPolicy(const Policy* policy) const {
    if (!policy)
        return -1;

    const auto* app = GGHumanClientApp::GetApp();
    const auto empire = app->GetContext().GetEmpire(app->EmpireID());

    // reject unavailable and already-adopted policies
    if (!empire || !empire->PolicyAvailable(policy->Name())
        || empire->PolicyAdopted(policy->Name()))
    { return -1; }

    // scan through slots to find one that can "mount" policy
    for (std::size_t i = 0u; i < m_slots.size(); ++i) {
        if (m_slots[i]->GetPolicy())
            continue;   // slot already occupied
        auto& slot_category = m_slots[i]->SlotCategory();
        if (policy->Category() == slot_category)
            return i;
    }

    return -1;
}

void GovernmentWnd::MainPanel::RevertPolicies() {
    auto* app = GGHumanClientApp::GetApp();
    ScriptingContext& context = app->GetContext();

    const int empire_id = app->EmpireID();
    auto empire = context.GetEmpire(empire_id);  // may be nullptr
    if (!empire) {
        ErrorLogger() << "GovernmentWnd::MainPanel::RevertPolicies has no empire to revert policies for";
        return;
    }

    if (!empire->PoliciesModified())
        return;

    // issue order to revoke changes
    app->Orders().IssueOrder<PolicyOrder>(context, empire_id);

    PostChangeBigUpdate();
}

void GovernmentWnd::MainPanel::ClearPolicies() {
    for (unsigned int slot = 0; slot < m_slots.size(); ++slot)
        this->SetPolicy(nullptr, slot, false);

    PostChangeBigUpdate();
}

void GovernmentWnd::MainPanel::ClearPolicy(const std::string& policy_name) {
    for (auto& slot : m_slots) {
        const Policy* existing_policy = slot->GetPolicy();
        if (!existing_policy)
            continue;
        if (existing_policy->Name() != policy_name)
            continue;
        slot->SetPolicy(nullptr);
    }
}

void GovernmentWnd::MainPanel::Populate() {
    for (const auto& slot: m_slots)
        DetachChild(slot);
    m_slots.clear();

    const auto* app = GGHumanClientApp::GetApp();
    const ScriptingContext& context = app->GetContext();

    // loop over policy slots the empire's government has, add slot controls
    const auto empire = context.GetEmpire(app->EmpireID());
    if (!empire)
        return;

    const auto all_slot_cats = ConcatenatedCategorySlots(empire.get());
    auto categories_slots_policies = empire->CategoriesSlotsPoliciesAdopted();

    for (unsigned int n = 0; n < all_slot_cats.size(); ++n) {
        // create slot controls for empire's policy slots
        auto& [category_index, category_name] = all_slot_cats[n];
        auto slot_control = GG::Wnd::Create<PolicySlotControl>(std::string{category_name}, category_index, n);
        m_slots.push_back(slot_control);
        AttachChild(slot_control);

        // assign policy controls to slots that correspond to adopted policies
        if (categories_slots_policies.contains(category_name)) {
            const auto& slots = categories_slots_policies[category_name];
            if (slots.contains(category_index))
                slot_control->SetPolicy(std::string{slots.at(category_index)}); // TODO: avoid string construction if possible...
        }

        // signals to respond to UI manipulation
        slot_control->SlotContentsAlteredSignal.connect(
            [this, n](const Policy* p, bool) { SetPolicy(p, n, true); });
        slot_control->PolicyClickedSignal.connect(PolicyClickedSignal);
    }
}

void GovernmentWnd::MainPanel::DoLayout() {
    const int PTS = ClientUI::Pts();
    const GG::Y BUTTON_HEIGHT{PTS * 2};
    static constexpr int MAINPANEL_PAD = 6;
    const GG::Pt lr = ClientSize() + GG::Pt(-GG::X(MAINPANEL_PAD), -GG::Y(MAINPANEL_PAD));

    if (m_slots.empty())
        return;

    // arrange policy slots, start new row when slots overlap with right wnd border
    const auto gov_wnd = std::dynamic_pointer_cast<const GovernmentWnd>(Parent());
    const auto [slot_size, text_pts] = gov_wnd ?
        std::pair{gov_wnd->GetPolicySlotSize(), gov_wnd->GetPolicyTextSize()} :
        std::pair{GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT), PTS};

    const GG::X initial_slot_l = GG::X{PAD*2};
    GG::Pt ul = GG::Pt(initial_slot_l, BUTTON_HEIGHT / 2 + PAD*2);

    int count = 0;
    bool first_iteration = true;
    for (auto& slot : m_slots) {
        slot->Resize(slot_size, text_pts);

        // start of new row
        if (count <= 0) {
            ul.x = initial_slot_l;

            if (first_iteration)
                first_iteration = false;
            else
                ul.y += GG::ToY(slot->Height() * (1 + POLICY_PAD));
        // no new row, progress in row
        } else {
            ul.x += GG::ToX(slot->Width() * (1 + POLICY_PAD));
        }

        slot->MoveTo(ul);
        count++;

        // reset count when hitting right border
        if ((count + 1) * GG::ToX(slot->Size().x * (1 + POLICY_PAD)) > (lr.x - PAD))
            count = 0;
    }
}

void GovernmentWnd::MainPanel::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                               GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    // if multiple things dropped simultaneously somehow, reject all
    if (std::distance(first, last) != 1)
        return;
}

void GovernmentWnd::MainPanel::AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                                           GG::Flags<GG::ModKey> mod_keys)
{}


//////////////////////////////////////////////////
// GovernmentWnd                                //
//////////////////////////////////////////////////
GovernmentWnd::GovernmentWnd(std::string_view config_name) :
    CUIWnd(UserString("MAP_BTN_GOVERNMENT"),
           GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE | PINABLE,
           config_name, false)
{}

void GovernmentWnd::CompleteConstruction() {
    Sound::TempUISoundDisabler sound_disabler;
    SetChildClippingMode(ChildClippingMode::ClipToClient);

    m_main_panel = GG::Wnd::Create<MainPanel>(GG::X{100}, GG::Y{100});
    AttachChild(m_main_panel);
    m_main_panel->Sanitize();

    m_policy_palette = GG::Wnd::Create<PolicyPalette>(GG::X{100}, GG::Y{100});
    AttachChild(m_policy_palette);
    m_policy_palette->PolicyDoubleClickedSignal.connect(
        boost::bind(&GovernmentWnd::MainPanel::AddPolicy, m_main_panel, _1));
    m_policy_palette->ClearPolicySignal.connect(
        boost::bind(&GovernmentWnd::MainPanel::ClearPolicy, m_main_panel, _1));

    m_policy_size_buttons = GG::Wnd::Create<GG::RadioButtonGroup>(GG::Orientation::HORIZONTAL);
    m_policy_size_buttons->ExpandButtons(true);
    m_policy_size_buttons->ExpandButtonsProportionally(true);

    auto large_size_icon = std::make_shared<GG::SubTexture>(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "policy_large.png"));
    auto medium_size_icon = std::make_shared<GG::SubTexture>(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "policy_medium.png"));
    auto small_size_icon = std::make_shared<GG::SubTexture>(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "policy_small.png"));

    m_policy_size_buttons->AddButton(
        GG::Wnd::Create<CUIStateButton>("", GG::FORMAT_LEFT, std::make_shared<CUIIconButtonRepresenter>(large_size_icon, GG::CLR_WHITE)));
    m_policy_size_buttons->AddButton(
        GG::Wnd::Create<CUIStateButton>("", GG::FORMAT_LEFT, std::make_shared<CUIIconButtonRepresenter>(medium_size_icon, GG::CLR_WHITE)));
    m_policy_size_buttons->AddButton(
        GG::Wnd::Create<CUIStateButton>("", GG::FORMAT_LEFT, std::make_shared<CUIIconButtonRepresenter>(small_size_icon, GG::CLR_WHITE)));
    AttachChild(m_policy_size_buttons);
    m_policy_size_buttons->SetCheck(0);

    m_revert_button = GG::Wnd::Create<CUIButton>(UserString("GOVERNMENT_WND_REVERT"));
    AttachChild(m_revert_button);

    auto zoom_to_policy_action = [](const Policy* policy, GG::Flags<GG::ModKey> modkeys) { ClientUI::GetClientUI()->ZoomToPolicy(policy->Name()); };
    //m_main_panel->PolicyClickedSignal.connect(zoom_to_policy_action);
    m_policy_palette->PolicyClickedSignal.connect(zoom_to_policy_action);
    m_policy_size_buttons->ButtonChangedSignal.connect(boost::bind(&GovernmentWnd::PolicySizeButtonClicked, this, _1));
    m_revert_button->LeftClickedSignal.connect(boost::bind(&GovernmentWnd::RevertPolicies, this));

    CUIWnd::CompleteConstruction();
    DoLayout();
}

void GovernmentWnd::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    CUIWnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void GovernmentWnd::ClearPolicies()
{ m_main_panel->ClearPolicies(); }

void GovernmentWnd::RevertPolicies()
{ m_main_panel->RevertPolicies(); }

void GovernmentWnd::Reset()
{ Refresh(); }

void GovernmentWnd::Sanitize()
{ m_main_panel->Sanitize(); }

void GovernmentWnd::Refresh() {
    m_policy_palette->Populate();
    m_main_panel->Refresh();
}

double GovernmentWnd::GetPolicyZoomFactor() const {
    switch (m_policy_size_buttons->CheckedButton()) {
    case 0: return 1;
    case 1: return 0.75;
    case 2: return 0.5;
    }

    DebugLogger() << "GovernmentWnd::GetPolicyZoomFactor(): m_policy_buttons no button checked.";
    return 1;
}

GG::Pt GovernmentWnd::GetPolicySlotSize() const {
    const double zoom_factor = GetPolicyZoomFactor();
    const GG::X slot_width{GG::ToX(SLOT_CONTROL_WIDTH * zoom_factor)};
    const GG::Y slot_height{GG::ToY(SLOT_CONTROL_HEIGHT * zoom_factor)};
    return GG::Pt(slot_width, slot_height);
}

int GovernmentWnd::GetPolicyTextSize() const {
    const double zoom_factor = GetPolicyZoomFactor();
    return static_cast<int>(ClientUI::Pts() * zoom_factor);
}

void GovernmentWnd::DoLayout() {
    static constexpr int GUESSTIMATE_NUM_CHARS_IN_BUTTON_TEXT = 10; // guesstimate for clear btn
    const int PTS = ClientUI::Pts();
    const GG::X PTS_WIDE{PTS / 2};           // guess at how wide per character the font needs
    const GG::X BUTTON_WIDTH = PTS_WIDE * GUESSTIMATE_NUM_CHARS_IN_BUTTON_TEXT;
    const GG::Y BUTTON_HEIGHT{PTS * 2};

    static constexpr GG::Pt palette_ul(GG::Pt0);
    const GG::Pt palette_lr(palette_ul + GG::Pt(ClientWidth(), ClientHeight() / 2));
    const int num_size_buttons = static_cast<int>(m_policy_size_buttons->NumButtons());

    const GG::Pt main_ul(palette_ul + GG::Pt(GG::X0, ClientHeight() / 2));
    const GG::Pt main_lr(ClientWidth(), ClientHeight() - GG::Y(INNER_BORDER_ANGLE_OFFSET));

    m_main_panel->SizeMove(main_ul, main_lr);
    m_policy_palette->SizeMove(palette_ul, palette_lr);

    const GG::Pt size_buttons_ul = main_ul + GG::Pt(GG::X{PAD}, GG::Y{PAD} - BUTTON_HEIGHT / 2);
    const GG::Pt size_buttons_lr = main_ul + GG::Pt((PAD + POLICY_SIZE_BUTTON_WIDTH) * num_size_buttons,
                                                    PAD + BUTTON_HEIGHT / 2);

    m_policy_size_buttons->SizeMove(size_buttons_ul, size_buttons_lr);

    const GG::Pt revert_button_ul = GG::Pt(main_lr.x - BUTTON_WIDTH - PAD, main_ul.y + PAD - BUTTON_HEIGHT / 2);
    const GG::Pt revert_button_lr = GG::Pt(main_lr.x - PAD, main_ul.y + PAD + BUTTON_HEIGHT / 2);

    m_revert_button->SizeMove(revert_button_ul, revert_button_lr);
}

void GovernmentWnd::PolicySizeButtonClicked(std::size_t idx)
{ Refresh(); }

void GovernmentWnd::EnableOrderIssuing(bool enable)
{}

void GovernmentWnd::CloseClicked()
{ ClosingSignal(); }

