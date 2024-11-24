#include "GraphicalSummary.h"

#include "../CUIControls.h"
#include "../CUIDrawUtil.h"
#include "../../client/ClientApp.h"
#include "../../Empire/Empire.h"
#include "../../universe/UniverseObject.h"
#include "../../util/i18n.h"
#include "../../util/Logger.h"
#include "../../util/OptionsDB.h"
#include "../../combat/CombatLogManager.h"

#include <GG/ClrConstants.h>
#include <GG/Layout.h>

#include <boost/format.hpp>

namespace {
    // These margins determine how we avoid drawing children on top of the
    // bevel in the lower right corner.
    constexpr GG::X BEVEL_MARGIN_X{6};

    // The height of the space where the display option buttons are put
    constexpr GG::Y OPTION_BAR_HEIGHT{25};
    // The height of option buttons
    constexpr GG::Y OPTION_BUTTON_HEIGHT{20};
    constexpr GG::X OPTION_BUTTON_PADDING{8};

    // How much space to leave for the y axis
    constexpr GG::X AXIS_WIDTH{10};
    // How much space to leave for the x axis
    // at the top of a SideBar
    constexpr GG::Y AXIS_HEIGHT{10};

    // Space around the labels of the axes
    constexpr GG::X Y_AXIS_LABEL_MARGIN{3};
    constexpr GG::Y X_AXIS_LABEL_MARGIN{8};

    // Margin between x-axis label and whatever is above the side bars top
    constexpr GG::Y SIDE_BAR_UP_MARGIN{5};

    // Space between side boxes
    constexpr GG::Y SIDE_BOX_MARGIN{8};

    constexpr GG::X MIN_SIDE_BAR_WIDTH{100};
    constexpr GG::Y MIN_SIDE_BAR_HEIGHT{40};

    // The participant bar height when its parent side bar height is
    // MIN_SIDE_BAR_HEIGHT.
    constexpr int MIN_PARTICIPANT_BAR_HEIGHT =
        Value(MIN_SIDE_BAR_HEIGHT) - Value(AXIS_HEIGHT) - Value(X_AXIS_LABEL_MARGIN) - Value(SIDE_BAR_UP_MARGIN);

    constexpr float EPSILON = 0.00001f;

    const std::string OPTIONS_ROOT = "ui.combat.summary.graph.";

    const std::string TOGGLE_BAR_HEIGHT_PROPORTIONAL = "bar.proportional.height.enabled";
    const std::string TOGGLE_BAR_WIDTH_PROPORTIONAL = "bar.proportional.width.enabled";
    const std::string TOGGLE_BAR_HEALTH_SMOOTH = "bar.health.smooth.enabled";
    const std::string TOGGLE_GRAPH_HEIGHT_PROPORTIONAL = "height.proportional.enabled";

    // command-line options
    void AddOptions(OptionsDB& db) {
        db.Add(OPTIONS_ROOT + TOGGLE_BAR_HEIGHT_PROPORTIONAL,
               UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_BAR_HEIGHT_PROPORTIONAL"),
               false);
        db.Add(OPTIONS_ROOT + TOGGLE_BAR_WIDTH_PROPORTIONAL,
               UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_BAR_WIDTH_PROPORTIONAL"),
               true);
        db.Add(OPTIONS_ROOT + TOGGLE_BAR_HEALTH_SMOOTH,
               UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_BAR_HEALTH_SMOOTH"),
               true);
        db.Add(OPTIONS_ROOT + TOGGLE_GRAPH_HEIGHT_PROPORTIONAL,
               UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_GRAPH_HEIGHT_PROPORTIONAL"),
               false);

        db.Add("ui.combat.summary.dead.color",
               UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_DEAD_COLOR"),
               GG::Clr(128, 0, 0, 255));
        db.Add("ui.combat.summary.damaged.color",
               UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_WOUND_COLOR"),
               GG::CLR_RED);
        db.Add("ui.combat.summary.undamaged.color",
               UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_HEALTH_COLOR"),
               GG::CLR_GREEN);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    // Removes the space taken up by axes of a sidebox from size.
    GG::Pt AdjustForAxes(GG::Pt size) {
        GG::Pt adjusted(size);
        adjusted.x = std::max( GG::X0, size.x - AXIS_WIDTH - Y_AXIS_LABEL_MARGIN);
        adjusted.y = std::max( GG::Y0, size.y - 2 * (SIDE_BAR_UP_MARGIN + AXIS_HEIGHT + X_AXIS_LABEL_MARGIN));
        return adjusted;
    }

    template<typename T>
    T interpolate(double modifier, const T& value1, const T& value2) {
        return static_cast<T>(modifier*value1 + (1.0 - modifier)*value2);
    }
}

class BarSizer {
public:
    typedef std::map<int, CombatSummary> CombatSummaryMap;

    BarSizer(const CombatSummaryMap& combat_summaries , GG::Pt available_size):
        m_max_total_max_health(-1.0f),
        m_max_units_on_a_side(-1),
        m_sum_of_max_max_healths(0),
        m_min_of_max_max_healths(-1.f),
        m_available_space(available_size),
        m_summaries(combat_summaries)
    {
        // We want to measure health on a single scale that shows as much as possible
        // while fitting the data of both sides.
        // Therefore we make the side with more health fill the window
        for (const auto& combat_summary : combat_summaries) {
            m_max_total_max_health = std::max(combat_summary.second.total_max_health, m_max_total_max_health);
            m_max_units_on_a_side = std::max(static_cast<int>(combat_summary.second.unit_summaries.size()), m_max_units_on_a_side);
            m_sum_of_max_max_healths += combat_summary.second.max_max_health;

            if (0.f > m_min_of_max_max_healths) {
                m_min_of_max_max_healths = combat_summary.second.total_max_health;
            } else {
                m_min_of_max_max_healths = std::min(combat_summary.second.max_max_health, m_min_of_max_max_healths);
            }
        }

        // More initialization
        SetAvailableSize(m_available_space);

        assert(m_max_total_max_health > 0);
    }

    GG::Pt GetBarSize( const ParticipantSummary& participant, const GG::X& label_margin ) const {
        GG::Pt total_space = GetSideBarSize(participant.empire_id);

        total_space = AdjustForAxes(total_space);

        // Take into account the size of the y-axis label.
        total_space.x -= label_margin;

        auto side_summary_it = m_summaries.find(participant.empire_id);

        if ( side_summary_it == m_summaries.end() ) {
            ErrorLogger() << "The empire of the object " << participant.object_id
                          << " is not known to be in this battle. (empire_id = " << participant.empire_id << ")";
            return GG::Pt(GG::X{10}, GG::Y{10});
        }

        const CombatSummary& side_summary = side_summary_it->second;

        GG::X width;
        if (participant.current_health > 0.0) {
            width = CalculateAliveWidth(participant, total_space);
        } else {
            width = CalculateDeadWidth(participant, total_space);
        }
        GG::Y height;
        if ( Get( TOGGLE_BAR_HEIGHT_PROPORTIONAL) ) {
            height = GG::ToY((participant.max_health / side_summary.max_max_health) * total_space.y);
        } else {
            height = total_space.y;
        }

        return GG::Pt(std::max(width, GG::X{3}), height);
    }

    GG::X CalculateAliveWidth(const ParticipantSummary& participant, GG::Pt total_space) const {
        if (Get(TOGGLE_BAR_WIDTH_PROPORTIONAL)) {
            return GG::X((participant.current_health / m_max_total_max_health) * total_space.x);
        } else {
            return GG::X(total_space.x / m_max_units_on_a_side);
        }
    }

    GG::X CalculateDeadWidth( const ParticipantSummary& participant, GG::Pt total_space) const {
        if (Get(TOGGLE_BAR_WIDTH_PROPORTIONAL)) {
            return GG::X((participant.max_health / m_max_total_max_health) * total_space.x);
        } else {
            return GG::X(total_space.x / m_max_units_on_a_side);
        }
    }

    GG::Pt GetSideBarSize(const Empire* empire) const
    { return GetSideBarSize( empire ? empire->EmpireID() : ALL_EMPIRES ); }

    void SetAvailableSize(GG::Pt size) {
        m_available_space = size;

        m_available_side_bar_space =
            m_available_space -
            GG::Pt( BEVEL_MARGIN_X,
                    static_cast<int>(m_summaries.size() + 1) * SIDE_BOX_MARGIN );

        m_available_participant_bar_height =
            std::max( GG::Y0,
                      m_available_side_bar_space.y -
                          (AXIS_HEIGHT + X_AXIS_LABEL_MARGIN + SIDE_BAR_UP_MARGIN) *
                          static_cast<int>(m_summaries.size()) );
    }

    bool Get(const std::string& option) const
    { return GetOptionsDB().Get<bool>(OPTIONS_ROOT + option); }

    void Set(const std::string& option, bool value) {
        GetOptionsDB().Set(OPTIONS_ROOT + option, value);
        GetOptionsDB().Commit();
    }

    GG::Pt GetMinSize() const {
        GG::Y reqd_available_client_height(GG::Y0);

        if (Get(TOGGLE_GRAPH_HEIGHT_PROPORTIONAL)) {
            // Try to ensure that the smallest bar is at least
            // MIN_SIDE_BAR_HEIGHT.
            reqd_available_client_height = GG::Y(static_cast<int>(static_cast<float>(MIN_PARTICIPANT_BAR_HEIGHT) * m_sum_of_max_max_healths / m_min_of_max_max_healths));
        } else {
            reqd_available_client_height = GG::Y(static_cast<int>(m_summaries.size()) * MIN_PARTICIPANT_BAR_HEIGHT);
        }

        GG::Y reqd_available_total_height(reqd_available_client_height + (SIDE_BOX_MARGIN + AXIS_HEIGHT + X_AXIS_LABEL_MARGIN + SIDE_BAR_UP_MARGIN) * static_cast<int>(m_summaries.size()) + SIDE_BOX_MARGIN);

        return GG::Pt(MIN_SIDE_BAR_WIDTH, reqd_available_total_height);
    }

private:
    float  m_max_total_max_health;
    int    m_max_units_on_a_side;
    float  m_sum_of_max_max_healths;
    float  m_min_of_max_max_healths;            //< Used to determine minimum size required for TOGGLE_GRAPH_HEIGHT_PROPORTIONAL mode.
    GG::Pt m_available_space;
    GG::Pt m_available_side_bar_space;          //< Caches some calculations
    GG::Y  m_available_participant_bar_height;  //< Caches some calculations

    const CombatSummaryMap& m_summaries;

    GG::Pt GetSideBarSize(int empire_id) const {
        // The client (participant bar) height of this side bar.
        GG::Y calculated_height(GG::Y0);

        auto summary_it = m_summaries.find(empire_id);

        if (Get(TOGGLE_GRAPH_HEIGHT_PROPORTIONAL) && summary_it != m_summaries.end()) {
            calculated_height = GG::ToY(m_available_participant_bar_height * summary_it->second.max_max_health / m_sum_of_max_max_healths);
        } else {
            calculated_height = m_available_participant_bar_height / static_cast<int>(m_summaries.size());
        }

        // Reconstruct the total size for this side bar from the available
        // total width and the calculated client height (which is converted to
        // the total height here).
        return GG::Pt( std::max(MIN_SIDE_BAR_WIDTH, m_available_side_bar_space.x),
                       std::max(MIN_SIDE_BAR_HEIGHT, calculated_height + (AXIS_HEIGHT + X_AXIS_LABEL_MARGIN + SIDE_BAR_UP_MARGIN)) );
    }
};

/// A Bar that shows the health of a single battle participant
class ParticipantBar : public GG::Wnd {
public:
    ParticipantBar(const ParticipantSummary& participant, const BarSizer& sizer):
        GG::Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE),
        m_participant(participant),
        m_sizer(sizer)
    {
        const ScriptingContext& context = IApp::GetApp()->GetContext();
        if (auto object = context.ContextObjects().getRaw(participant.object_id)) {
            SetBrowseText(object->PublicName(ClientApp::GetApp()->EmpireID(), context.ContextUniverse()) +
                          " " + DoubleToString(participant.current_health, 3, false) +
                          "/" + DoubleToString(participant.max_health, 3, false));
        }
        SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

        OptionsDB& options = GetOptionsDB();
        m_dead_color = options.Get<GG::Clr>("ui.combat.summary.dead.color");
        m_wound_color = options.Get<GG::Clr>("ui.combat.summary.damaged.color");
        m_health_color = options.Get<GG::Clr>("ui.combat.summary.undamaged.color");
    }

    void Render() override {
        const GG::Clr base_color = Alive() ? m_wound_color : m_dead_color;

        // Always draw the red background, health will cover it
        GG::FlatRectangle(ClientUpperLeft(), ClientLowerRight(), base_color, m_hovered ? GG::CLR_WHITE : GG::CLR_BLACK, 1);

        if (m_sizer.Get(TOGGLE_BAR_HEALTH_SMOOTH)) {
            // Use a smooth colour change based health display.
            if (Alive()) {
                double health_percentage = 1.0 * m_participant.current_health / m_participant.max_health;

                GG::Clr mixed_color(interpolate(health_percentage, m_health_color.r, m_wound_color.r),
                                    interpolate(health_percentage, m_health_color.g, m_wound_color.g),
                                    interpolate(health_percentage, m_health_color.b, m_wound_color.b),
                                    interpolate(health_percentage, m_health_color.a, m_wound_color.a));
                GG::FlatRectangle(ClientUpperLeft(), ClientLowerRight(), mixed_color, GG::CLR_ZERO, 1);
            }
        } else {
            if (Alive()) {
                GG::Y health_height(GG::ToY(m_participant.current_health / m_participant.max_health * ClientHeight()));
                GG::FlatRectangle(GG::Pt(ClientUpperLeft().x, ClientLowerRight().y - health_height),
                                         ClientLowerRight(), m_health_color, GG::CLR_ZERO, 1);
            }
        }
    }

    void MouseEnter(GG::Pt, GG::Flags<GG::ModKey>) noexcept override
    { m_hovered = true; }

    void MouseLeave() noexcept override
    { m_hovered = false; }

    /// Resizes the bar to have a width and height based on the
    /// current and maximal health of the participant.
    void DoLayout(const GG::X& label_margin)
    { Resize(m_sizer.GetBarSize(m_participant, label_margin)); }

    /// Moves the bottom left of the bar to \a pt
    void MoveBottomTo(GG::Pt pt) {
        pt.y -= Height();
        MoveTo(pt);
    }

    /// Tells whether this participant is still alive
    bool Alive() const noexcept
    { return m_participant.current_health > 0.0; }

private:
    const ParticipantSummary& m_participant;
    const BarSizer& m_sizer;
    bool m_hovered = false;
    GG::Clr m_dead_color;
    GG::Clr m_wound_color;
    GG::Clr m_health_color;
};

/// A Bar that shows the aggregate health of a side,
/// and the statuses of all its units
class SideBar : public GG::Wnd {
public:
    SideBar(const CombatSummary& combat_summary, const BarSizer& sizer):
        GG::Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE),
        m_side_summary(combat_summary),
        m_sizer(sizer)
    {}

    void CompleteConstruction() override {
        GG::Clr axis_label_color = m_side_summary.SideColor();

        m_x_axis_label = GG::Wnd::Create<CUILabel>(
            (boost::format( UserString("COMBAT_FLEET_HEALTH_AXIS_LABEL") )
             % m_side_summary.SideName()
             % static_cast<int>(m_side_summary.total_current_health)
             % static_cast<int>(m_side_summary.total_max_health)).str(),
            GG::FORMAT_LEFT);
        m_x_axis_label->SetColor(axis_label_color);
        AttachChild(m_x_axis_label);

        m_y_axis_label = GG::Wnd::Create<CUILabel>(UserString("COMBAT_UNIT_HEALTH_AXIS_LABEL"));
        m_y_axis_label->SetColor(axis_label_color);
        AttachChild(m_y_axis_label);

        m_dead_label = GG::Wnd::Create<CUILabel>(
            (boost::format(UserString("COMBAT_DESTROYED_LABEL")) % m_side_summary.DestroyedUnits()).str(),
            GG::FORMAT_RIGHT);
        m_dead_label->SetColor(axis_label_color);
        AttachChild(m_dead_label);

        MakeBars();
    }

    void MakeBars() {
        for (CombatSummary::ParticipantSummaryPtr unit : m_side_summary.unit_summaries) {
            if (unit->max_health > 0) {
                m_participant_bars.emplace_back(GG::Wnd::Create<ParticipantBar>(*unit, m_sizer));
                AttachChild(m_participant_bars.back());
            }
        }
        DoLayout();
    }

    void DoLayout() {
        GG::Pt bar_size = m_sizer.GetSideBarSize(m_side_summary.empire);
        if ( bar_size != Size() ) {
            Resize(bar_size);
        }

        m_x_axis_label->MoveTo(GG::Pt(GG::X0, SIDE_BAR_UP_MARGIN));
        m_dead_label->MoveTo(GG::Pt(ClientWidth() - m_dead_label->Width(), SIDE_BAR_UP_MARGIN));

        GG::Pt alive_ll = GG::Pt(GG::X0, ClientHeight());
        GG::Pt dead_lr = ClientSize();

        for (auto& bar : m_participant_bars) {
            bar->DoLayout(m_y_axis_label->MinUsableSize().x);
            if (bar->Alive()) {
                bar->MoveBottomTo(alive_ll);
                alive_ll.x += bar->Width();
            } else {
                dead_lr.x -= bar->Width();
                bar->MoveBottomTo(dead_lr);
            }
        }

        m_y_axis_label->MoveTo(GG::Pt(-m_y_axis_label->MinUsableSize().x / 2 - AXIS_WIDTH, Height()/2 - m_y_axis_label->Height()/2));
    }

    void SizeMove(GG::Pt ul, GG::Pt lr) override {
        GG::Wnd::SizeMove(ul, lr);
        DoLayout();
    }

    GG::Pt RelativeClientUpperLeft() const noexcept {
        GG::Pt ul{ GG::X0, GG::Y0 };
        ul.x += AXIS_WIDTH + m_y_axis_label->MinUsableSize().x + Y_AXIS_LABEL_MARGIN;
        ul.y += SIDE_BAR_UP_MARGIN + AXIS_HEIGHT + X_AXIS_LABEL_MARGIN;
        return ul;
    }

    GG::Pt ClientUpperLeft() const noexcept override
    { return GG::Wnd::UpperLeft() + RelativeClientUpperLeft(); }

private:
    const CombatSummary&                            m_side_summary;
    std::vector<std::shared_ptr<ParticipantBar>>    m_participant_bars;
    std::shared_ptr<GG::Label>                      m_x_axis_label;
    std::shared_ptr<GG::Label>                      m_y_axis_label;
    std::shared_ptr<GG::Label>                      m_dead_label;
    const BarSizer&                                 m_sizer;

    float MaxMaxHealth() {
        float max_health = -1.0f;
        for (CombatSummary::ParticipantSummaryPtr participant : m_side_summary.unit_summaries)
        { max_health = std::max(max_health, participant->max_health); }
        return max_health;
    }
};

/// A Bar that contains the display options
class OptionsBar : public GG::Wnd {
    struct ToggleData;
public:
    boost::signals2::signal<void ()> ChangedSignal;

    OptionsBar(std::unique_ptr<BarSizer>& sizer) :
        GG::Wnd(),
        m_sizer(sizer)
    {}

    void CompleteConstruction() override {
        m_toggles.emplace_back(std::make_shared<ToggleData>(
            UserString("COMBAT_SUMMARY_PARTICIPANT_RELATIVE"),
            UserString("COMBAT_SUMMARY_PARTICIPANT_EQUAL"),
            UserString("COMBAT_SUMMARY_PARTICIPANT_RELATIVE_TIP"),
            UserString("COMBAT_SUMMARY_PARTICIPANT_EQUAL_TIP"),
            TOGGLE_BAR_WIDTH_PROPORTIONAL, &m_sizer,
            std::static_pointer_cast<OptionsBar>(shared_from_this())));
        m_toggles.emplace_back(std::make_shared<ToggleData>(
            UserString("COMBAT_SUMMARY_HEALTH_SMOOTH"),
            UserString("COMBAT_SUMMARY_HEALTH_BAR"),
            UserString("COMBAT_SUMMARY_HEALTH_SMOOTH_TIP"),
            UserString("COMBAT_SUMMARY_HEALTH_BAR_TIP"),
            TOGGLE_BAR_HEALTH_SMOOTH, &m_sizer,
            std::static_pointer_cast<OptionsBar>(shared_from_this())));
        m_toggles.emplace_back(std::make_shared<ToggleData>(
            UserString("COMBAT_SUMMARY_BAR_HEIGHT_PROPORTIONAL"),
            UserString("COMBAT_SUMMARY_BAR_HEIGHT_EQUAL"),
            UserString("COMBAT_SUMMARY_BAR_HEIGHT_PROPORTIONAL_TIP"),
            UserString("COMBAT_SUMMARY_BAR_HEIGHT_EQUAL_TIP"),
            TOGGLE_BAR_HEIGHT_PROPORTIONAL, &m_sizer,
            std::static_pointer_cast<OptionsBar>(shared_from_this())));
        m_toggles.emplace_back(std::make_shared<ToggleData>(
            UserString("COMBAT_SUMMARY_GRAPH_HEIGHT_PROPORTIONAL"),
            UserString("COMBAT_SUMMARY_GRAPH_HEIGHT_EQUAL"),
            UserString("COMBAT_SUMMARY_GRAPH_HEIGHT_PROPORTIONAL_TIP"),
            UserString("COMBAT_SUMMARY_GRAPH_HEIGHT_EQUAL_TIP"),
            TOGGLE_GRAPH_HEIGHT_PROPORTIONAL, &m_sizer,
            std::static_pointer_cast<OptionsBar>(shared_from_this())));
        DoLayout();
    }

    GG::Pt MinUsableSize() const override {
        GG::Pt min_size(GG::X0, GG::Y0);

        for (auto& data : m_toggles)
        { min_size.x += data->button->Width() + OPTION_BUTTON_PADDING; }

        min_size.y = OPTION_BAR_HEIGHT;

        return min_size;
    }

    void DoLayout() {
        auto cui_font = ClientUI::GetFont();

        GG::Pt pos(GG::X0, GG::Y0);
        for (auto& data : m_toggles) {
            if (!data->button->Children().empty()) {
                //Assume first child of the button is the label
                if (const auto* label = dynamic_cast<GG::TextControl const*>(data->button->Children().front().get()))
                {
                    data->button->Resize(
                        GG::Pt(label->MinUsableSize().x + OPTION_BUTTON_PADDING,
                               OPTION_BUTTON_HEIGHT));
                }
            }
            data->button->MoveTo(pos);
            pos.x += data->button->Width() + OPTION_BUTTON_PADDING;
        }
    }

private:
    std::unique_ptr<BarSizer>&               m_sizer;
    std::vector<std::shared_ptr<ToggleData>> m_toggles;

    struct ToggleData : public boost::signals2::trackable {
        const std::string label_true;
        const std::string label_false;
        const std::string tip_true;
        const std::string tip_false;
        const std::string option_key;
        std::unique_ptr<BarSizer>* sizer;
        std::weak_ptr<OptionsBar> parent;
        std::shared_ptr<GG::Button> button;

        void Toggle()
        { SetValue(!GetValue()); }

        void SetValue(bool value) {
            if (sizer) {
                if (const auto& ptr{*sizer})
                    ptr->Set(option_key, value);
            }
            button->SetText(value ? label_true : label_false);
            button->SetBrowseText(value ? tip_true : tip_false);
            if (auto locked_parent = parent.lock()) {
                locked_parent->DoLayout();
                locked_parent->ChangedSignal();
            }
        }

        bool GetValue() const {
            if (sizer) {
                if (const auto& ptr{*sizer})
                    return ptr->Get(option_key);
            }
            return false;
        }

        ToggleData(std::string label_true_, std::string label_false_, std::string tip_true_,
                   std::string tip_false_, std::string option_key_,
                   std::unique_ptr<BarSizer>* sizer_, std::shared_ptr<OptionsBar> parent_) :
            label_true(std::move(label_true_)),
            label_false(std::move(label_false_)),
            tip_true(std::move(tip_true_)),
            tip_false(std::move(tip_false_)),
            option_key(std::move(option_key_)),
            sizer(sizer_),
            parent(std::move(parent_)),
            button(Wnd::Create<CUIButton>("-"))
        {
            button->LeftClickedSignal.connect(boost::bind(&ToggleData::Toggle, this));
            parent_->AttachChild(button);
            SetValue(GetValue());
        }
    };
};


GraphicalSummaryWnd::GraphicalSummaryWnd() :
    GG::Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::NO_WND_FLAGS)
{}

GraphicalSummaryWnd::~GraphicalSummaryWnd() = default;

GG::Pt GraphicalSummaryWnd::MinUsableSize() const {
    GG::Pt min_size(GG::X0, GG::Y0);

    // Ask the bar sizer for the required size to display the side bars because
    // it contains all of the useful sizing information in the first place,
    // even though it does not derive from GG::Wnd and have a virtual
    // MinUsableSize function.
    if (m_sizer)
        min_size += m_sizer->GetMinSize();

    if (m_options_bar) {
        GG::Pt options_bar_min_size(m_options_bar->MinUsableSize());
        min_size.x = std::max(min_size.x, options_bar_min_size.x);
        min_size.y += options_bar_min_size.y;
    }

    return min_size;
}

void GraphicalSummaryWnd::SetLog(int log_id)
{ MakeSummaries(log_id); }

void GraphicalSummaryWnd::DoLayout() {
    if (!m_sizer || !m_options_bar)
        return;

    GG::Pt ul(GG::X0, SIDE_BOX_MARGIN);

    m_options_bar->Resize( GG::Pt( ClientWidth(), OPTION_BAR_HEIGHT ) );

    GG::Pt space_for_bars = ClientSize();
    space_for_bars.y -= m_options_bar->Height();

    m_sizer->SetAvailableSize(space_for_bars);
    m_options_bar->DoLayout();

    for (auto& box : m_side_boxes) {
        box->MoveTo(ul);
        box->DoLayout();
        ul.y += box->Height() + SIDE_BOX_MARGIN;
    }

    m_options_bar->MoveTo(GG::Pt(GG::X(4), ClientSize().y - m_options_bar->Height()));
}

void GraphicalSummaryWnd::Render() {
    GG::FlatRectangle(UpperLeft() + GG::Pt(GG::X1, GG::Y0), LowerRight(), ClientUI::CtrlColor(),
                      ClientUI::CtrlBorderColor(), 1);
}

void GraphicalSummaryWnd::HandleButtonChanged() {
    MinSizeChangedSignal();
    DoLayout();
}

void GraphicalSummaryWnd::MakeSummaries(int log_id) {
    m_summaries.clear();
    boost::optional<const CombatLog&> log = GetCombatLog(log_id);
    if (!log) {
        ErrorLogger() << "CombatReportWnd::CombatReportPrivate::MakeSummaries: Could not find log: " << log_id;
    } else {
        for (int object_id : log->object_ids) {
            if (object_id < 0)
                continue;   // fighters and invalid objects
            auto object = ClientApp::GetApp()->GetContext().ContextObjects().get(object_id);
            if (!object) {
                ErrorLogger() << "GraphicalSummaryWnd::MakeSummaries couldn't find object with id: " << object_id;
                continue;
            }

            int owner_id = object->Owner();
            if (!m_summaries.contains(owner_id))
                m_summaries[owner_id] = CombatSummary(owner_id);

            auto map_it = log->participant_states.find(object_id);
            if (map_it != log->participant_states.end()) {
                m_summaries[owner_id].AddUnit(object_id, map_it->second);
            } else {
                ErrorLogger() << "Participant state missing from log. Object id: " << object_id << " log id: " << log_id;
            }
        }

        for (auto& summary : m_summaries) {
            DebugLogger() << "MakeSummaries: empire " << summary.first
                          << " total health: " << summary.second.total_current_health
                          << " max health: " << summary.second.total_max_health
                          << " units: " << summary.second.unit_summaries.size();
        }
    }

    GenerateGraph();
}

void GraphicalSummaryWnd::DeleteSideBars() {
    for (auto& box : m_side_boxes)
        DetachChild(box.get());
    m_side_boxes.clear();
}

void GraphicalSummaryWnd::GenerateGraph() {
    DeleteSideBars();

    m_sizer = std::make_unique<BarSizer>(m_summaries, ClientSize());

    for (auto& summary : m_summaries) {
        if (summary.second.total_max_health > EPSILON) {
            summary.second.Sort();
            auto box = GG::Wnd::Create<SideBar>(summary.second, *m_sizer);
            m_side_boxes.push_back(box);
            AttachChild(std::move(box));
        }
    }

    if (m_options_bar) {
        DebugLogger() << "GraphicalSummaryWnd::GenerateGraph(): m_options_bar "
                         "already exists, calling DetachChild(m_options_bar) "
                         "before creating a new one.";
        DetachChild(m_options_bar.get());
    }
    m_options_bar = GG::Wnd::Create<OptionsBar>(m_sizer);
    AttachChild(m_options_bar);
    m_options_bar->ChangedSignal.connect(boost::bind(&GraphicalSummaryWnd::HandleButtonChanged, this));

    MinSizeChangedSignal();
    DoLayout();
}
