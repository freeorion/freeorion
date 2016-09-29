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
#include <GG/DrawUtil.h>
#include <GG/Layout.h>

#include <boost/format.hpp>

namespace {
    // These margins determine how we avoid drawing children on top of the
    // bevel in the lower right corner.
    GG::X BEVEL_MARGIN_X(6);
    GG::Y BEVEL_MARGIN_Y(12);

    // The height of the space where the display option buttons are put
    GG::Y OPTION_BAR_HEIGHT(25);
    // The height of option buttons
    GG::Y OPTION_BUTTON_HEIGHT(20);
    GG::X OPTION_BUTTON_PADDING(8);

    // How much space to leave for the y axis
    GG::X AXIS_WIDTH(10);
    // How much space to leave for the axis
    // at the bottom of a SideBar
    GG::Y AXIS_HEIGHT(10);

    // Space arounf the labels of the axes
    GG::X Y_AXIS_LABEL_MARGIN(3);
    GG::Y X_AXIS_LABEL_MARGIN(8);

    // Margin between participant bars and whatever is above the side bars top
    GG::Y PARTICIPANT_BAR_UP_MARGIN(5);

    // Space between side boxes
    GG::Y SIDE_BOX_MARGIN(8);

    int MIN_SIDE_BAR_WIDTH = 100;
    int MIN_SIDE_BAR_HEIGHT = 40;

    // The participant bar height when its parent side bar height is
    // MIN_SIDE_BAR_HEIGHT.
    int MIN_PARTICIPANT_BAR_HEIGHT = MIN_SIDE_BAR_HEIGHT -
                                     Value(AXIS_HEIGHT) -
                                     Value(X_AXIS_LABEL_MARGIN) -
                                     Value(PARTICIPANT_BAR_UP_MARGIN);

    const float EPSILON = 0.00001f;

    const std::string OPTIONS_ROOT = "UI.combat.summary.graph.";

    const std::string TOGGLE_BAR_HEIGHT_PROPORTIONAL = "bar_height_proportional";
    const std::string TOGGLE_BAR_WIDTH_PROPORTIONAL = "bar_width_proportional";
    const std::string TOGGLE_BAR_HEALTH_SMOOTH = "bar_health_smooth";
    const std::string TOGGLE_GRAPH_HEIGHT_PROPORTIONAL = "graph_height_proportional";

    // command-line options
    void AddOptions(OptionsDB& db) {
        db.Add<bool>(OPTIONS_ROOT + TOGGLE_BAR_HEIGHT_PROPORTIONAL,
                     UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_BAR_HEIGHT_PROPORTIONAL"),
                     false);
        db.Add<bool>(OPTIONS_ROOT + TOGGLE_BAR_WIDTH_PROPORTIONAL,
                     UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_BAR_WIDTH_PROPORTIONAL"),
                     true);
        db.Add<bool>(OPTIONS_ROOT + TOGGLE_BAR_HEALTH_SMOOTH,
                     UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_BAR_HEALTH_SMOOTH"),
                     true);
        db.Add<bool>(OPTIONS_ROOT + TOGGLE_GRAPH_HEIGHT_PROPORTIONAL,
                     UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_GRAPH_HEIGHT_PROPORTIONAL"),
                     false);

        db.Add<StreamableColor>("UI.combat.summary.dead-color", UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_DEAD_COLOR"), StreamableColor(GG::Clr(128, 0, 0, 255)));
        db.Add<StreamableColor>("UI.combat.summary.wound-color", UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_WOUND_COLOR"), StreamableColor(GG::CLR_RED));
        db.Add<StreamableColor>("UI.combat.summary.health-color", UserStringNop("OPTIONS_DB_UI_COMBAT_SUMMARY_HEALTH_COLOR"), StreamableColor(GG::CLR_GREEN));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    // Removes the space taken up by axes of a sidebox from size.
    GG::Pt AdjustForAxes(const GG::Pt& size) {
        GG::Pt adjusted(size);
        adjusted.x = std::max( GG::X0, size.x - AXIS_WIDTH - Y_AXIS_LABEL_MARGIN);
        adjusted.y = std::max( GG::Y0, size.y - AXIS_HEIGHT - X_AXIS_LABEL_MARGIN - PARTICIPANT_BAR_UP_MARGIN);
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

    BarSizer( const CombatSummaryMap& combat_summaries , const GG::Pt& available_size):
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
        for (std::map<int, CombatSummary>::const_iterator it = combat_summaries.begin(); it != combat_summaries.end(); ++it ) {
            m_max_total_max_health = std::max( it->second.total_max_health, m_max_total_max_health );
            m_max_units_on_a_side = std::max( static_cast<int>(it->second.unit_summaries.size()), m_max_units_on_a_side );
            m_sum_of_max_max_healths += it->second.max_max_health;

            if (0.f > m_min_of_max_max_healths) {
                m_min_of_max_max_healths = it->second.total_max_health;
            } else {
                m_min_of_max_max_healths = std::min(it->second.max_max_health, m_min_of_max_max_healths);
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

        CombatSummaryMap::const_iterator side_summary_it = m_summaries.find(participant.empire_id);

        if ( side_summary_it == m_summaries.end() ) {
            ErrorLogger() << "The empire of the object " << participant.object_id
                          << " is not known to be in this battle. (empire_id = " << participant.empire_id << ")";
            return GG::Pt(GG::X(10), GG::Y(10));
        }

        const CombatSummary& side_summary = side_summary_it->second;

        GG::X width;
        if ( participant.current_health > 0.0 ) {
            width = CalculateAliveWidth( participant, total_space );
        } else {
            width = CalculateDeadWidth( participant, total_space );
        }
        GG::Y height;
        if ( Get( TOGGLE_BAR_HEIGHT_PROPORTIONAL) ) {
            height = ( (participant.max_health / side_summary.max_max_health) * total_space.y );
        } else {
            height = total_space.y;
        }

        return GG::Pt(std::max(width, GG::X(3)), height);
    }

    GG::X CalculateAliveWidth( const ParticipantSummary& participant, const GG::Pt& total_space ) const {
        if ( Get(TOGGLE_BAR_WIDTH_PROPORTIONAL) ) {
            return GG::X( (participant.current_health / m_max_total_max_health) * total_space.x );
        } else {
            return GG::X( total_space.x / m_max_units_on_a_side );
        }
    }

    GG::X CalculateDeadWidth( const ParticipantSummary& participant, const GG::Pt& total_space) const {
        if ( Get(TOGGLE_BAR_WIDTH_PROPORTIONAL) ) {
            return GG::X( (participant.max_health / m_max_total_max_health) * total_space.x );
        } else {
            return GG::X( total_space.x / m_max_units_on_a_side );
        }
    }

    GG::Pt GetSideBarSize(const Empire* empire ) const
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
                          (AXIS_HEIGHT + X_AXIS_LABEL_MARGIN + PARTICIPANT_BAR_UP_MARGIN) *
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

        GG::Y reqd_available_total_height(reqd_available_client_height + (SIDE_BOX_MARGIN + AXIS_HEIGHT + X_AXIS_LABEL_MARGIN + PARTICIPANT_BAR_UP_MARGIN) * static_cast<int>(m_summaries.size()) + SIDE_BOX_MARGIN);

        return GG::Pt(GG::X(MIN_SIDE_BAR_WIDTH), reqd_available_total_height);
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

    GG::Pt GetSideBarSize( int empire_id ) const {
        // The client (participant bar) height of this side bar.
        GG::Y calculated_height(GG::Y0);

        CombatSummaryMap::const_iterator summary_it = m_summaries.find(empire_id);

        if ( Get(TOGGLE_GRAPH_HEIGHT_PROPORTIONAL) && summary_it != m_summaries.end() ) {
            calculated_height = m_available_participant_bar_height * summary_it->second.max_max_health / m_sum_of_max_max_healths;
        } else {
            calculated_height = m_available_participant_bar_height / static_cast<int>(m_summaries.size());
        }

        // Reconstruct the total size for this side bar from the available
        // total width and the calculated client height (which is converted to
        // the total height here).
        return GG::Pt( std::max(GG::X(MIN_SIDE_BAR_WIDTH), m_available_side_bar_space.x),
                       std::max(GG::Y(MIN_SIDE_BAR_HEIGHT), calculated_height + (AXIS_HEIGHT + X_AXIS_LABEL_MARGIN + PARTICIPANT_BAR_UP_MARGIN)) );
    }
};

/// A Bar that shows the health of a single battle participant
class ParticipantBar : public GG::Wnd {
public:
    ParticipantBar(const ParticipantSummary& participant, const BarSizer& sizer):
        GG::Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE),
        m_participant(participant),
        m_sizer(sizer),
        m_hovered(false)
    {
        TemporaryPtr<UniverseObject> object =  Objects().Object(participant.object_id);
        if (object) {
            SetBrowseText(object->PublicName(ClientApp::GetApp()->EmpireID()) + " " +
                          DoubleToString(participant.current_health, 3, false) + "/" +
                          DoubleToString(participant.max_health, 3, false)
            );
        }
        SetBrowseModeTime(300);

        OptionsDB& options = GetOptionsDB();
        m_dead_color = options.Get<StreamableColor>("UI.combat.summary.dead-color").ToClr();
        m_wound_color = options.Get<StreamableColor>("UI.combat.summary.wound-color").ToClr();
        m_health_color = options.Get<StreamableColor>("UI.combat.summary.health-color").ToClr();
    }

    virtual void Render() {
        GG::Clr base_color = Alive() ? m_wound_color : m_dead_color;

        // Always draw the red background, health will cover it
        GG::FlatRectangle(ClientUpperLeft(), ClientLowerRight(), base_color, m_hovered ? GG::CLR_WHITE : GG::CLR_BLACK, 1);

        if ( m_sizer.Get( TOGGLE_BAR_HEALTH_SMOOTH ) ) {
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
                GG::Y health_height( (m_participant.current_health / m_participant.max_health) * Value(ClientHeight()) );
                GG::FlatRectangle(GG::Pt(ClientUpperLeft().x, ClientLowerRight().y - health_height), ClientLowerRight(), m_health_color, GG::CLR_ZERO, 1);
            }
        }
    }

    virtual void MouseEnter(const GG::Pt& pt, GG::Flags< GG::ModKey > mod_keys)
    { m_hovered = true; }

    virtual void MouseLeave()
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
    bool Alive() const
    { return m_participant.current_health > 0.0; }

private:
    const ParticipantSummary& m_participant;
    const BarSizer& m_sizer;
    bool m_hovered;
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
        m_x_axis_label(0),
        m_sizer(sizer)
    {

        GG::Clr axis_label_color = combat_summary.SideColor();

        m_x_axis_label = new CUILabel( (boost::format( UserString("COMBAT_FLEET_HEALTH_AXIS_LABEL") )
                                      % combat_summary.SideName()
                                      % static_cast<int>(combat_summary.total_current_health)
                                      % static_cast<int>(combat_summary.total_max_health)).str(), GG::FORMAT_LEFT);
        m_x_axis_label->SetColor(axis_label_color);
        AttachChild(m_x_axis_label);

        m_y_axis_label = new CUILabel(UserString("COMBAT_UNIT_HEALTH_AXIS_LABEL"));
        m_y_axis_label->SetColor(axis_label_color);
        AttachChild(m_y_axis_label);

        m_dead_label = new CUILabel( (boost::format(UserString("COMBAT_DESTROYED_LABEL")) % m_side_summary.DestroyedUnits() ).str(), GG::FORMAT_RIGHT);
        m_dead_label->SetColor(axis_label_color);
        AttachChild(m_dead_label);

        MakeBars();
    }

    void MakeBars() {
        for (CombatSummary::UnitSummaries::const_iterator it = m_side_summary.unit_summaries.begin();
                it != m_side_summary.unit_summaries.end();
                ++it) {
            if ((*it)->max_health > 0) {
                m_participant_bars.push_back(new ParticipantBar(**it, m_sizer));
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

        GG::Pt alive_ll = GG::Pt(GG::X0, GG::Y0);
        alive_ll.y += ClientHeight();
        GG::Pt dead_lr = ClientSize();

        for (std::vector<ParticipantBar*>::const_iterator it = m_participant_bars.begin();
                it != m_participant_bars.end();
                ++it)
        {
            ParticipantBar* bar = *it;
            bar->DoLayout(m_y_axis_label->MinUsableSize().x);
            if (bar->Alive()) {
                bar->MoveBottomTo(alive_ll);
                alive_ll.x += bar->Width();
            } else {
                dead_lr.x -= bar->Width();
                bar->MoveBottomTo(dead_lr);
            }
        }

        m_x_axis_label->MoveTo(GG::Pt(GG::X0, Height() - m_x_axis_label->Height()));
        m_y_axis_label->MoveTo(GG::Pt(-m_y_axis_label->MinUsableSize().x / 2 - AXIS_WIDTH, Height()/2 - m_y_axis_label->Height()/2));
        m_dead_label->MoveTo(GG::Pt(ClientWidth() -  m_dead_label->Width(), Height() - m_dead_label->Height()));
    }

    void DrawArrow(GG::Pt begin, GG::Pt end) {
        double head_width = 5.0;
        // A vector (math) of the arrow we wish to draw
        GG::Pt direction = end - begin;
        double length = sqrt(1.0*(Value(direction.x)*Value(direction.x) +
                                  Value(direction.y)*Value(direction.y)));
        if (length == 0) {
            return;
        }

        // The point in the main line of the arrow,
        // paraller to which the head ends
        //          \.
        //           \.
        // --------h-->
        //           /.
        //          /.
        // h is at the handle
        GG::Pt handle;
        // How much to move off the handle to get to
        // the end point of one of the head lines
        GG::X delta_x;
        GG::Y delta_y;

        if (direction.x != 0 && direction.y != 0) {
            // In a skewed arrow we need
            // a bit of geometry to figure out the head
            double x = Value(direction.x);
            double y = Value(direction.y);
            double normalizer = head_width / sqrt(1 + x*x / (y*y));
            delta_x = GG::X(normalizer);
            delta_y = GG::Y(- x / y * normalizer);

            handle = end - GG::Pt((head_width / length) * direction.x, (head_width / length) * direction.y);
        } else if (direction.x == 0) {
            // Vertical arrow
            handle = end;
            handle.y -= boost::math::sign(Value(direction.y))*GG::Y(head_width);
            delta_x = GG::X(head_width);
            delta_y = GG::Y0;
        } else {
            //horizontal arrow
            handle = end;
            handle.x -= boost::math::sign(Value(direction.x)) * GG::X(head_width);
            delta_x = GG::X0;
            delta_y = GG::Y(head_width);
        }

        GG::Pt left_head = handle;
        GG::Pt right_head = handle;

        left_head.x += delta_x;
        left_head.y += delta_y;
        // The other line is on the opposite side of the handle
        right_head.x -=  delta_x;
        right_head.y -= delta_y;

        GG::glColor(GG::CLR_WHITE);
        glLineWidth(2);
        glDisable(GL_TEXTURE_2D);

        GG::GL2DVertexBuffer verts;
        verts.reserve(6);
        verts.store(Value(begin.x),     Value(begin.y));
        verts.store(Value(end.x),       Value(end.y));
        verts.store(Value(end.x),       Value(end.y));
        verts.store(Value(left_head.x), Value(left_head.y));
        verts.store(Value(end.x),       Value(end.y));
        verts.store(Value(right_head.x),Value(right_head.y));
        verts.activate();

        glDrawArrays(GL_LINES, 0, verts.size());

        glEnable(GL_TEXTURE_2D);
    }

    virtual void Render() {
        // Draw the axes outside th3e client area
        GG::Pt begin(ClientUpperLeft().x - AXIS_WIDTH/2, ClientLowerRight().y + AXIS_HEIGHT/2);
        GG::Pt x_end(ClientLowerRight().x, begin.y);
        GG::Pt y_end(begin.x, ClientUpperLeft().y);
        DrawArrow(begin, x_end);
        DrawArrow(begin, y_end);
    }

    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        GG::Wnd::SizeMove(ul, lr);
        DoLayout();
    }

    GG::Pt RelativeClientUpperLeft() const {
        GG::Pt ul(AXIS_WIDTH + m_y_axis_label->MinUsableSize().x + Y_AXIS_LABEL_MARGIN, GG::Y0);
        return ul;
    }

    virtual GG::Pt ClientUpperLeft() const
    { return  GG::Wnd::UpperLeft() + RelativeClientUpperLeft(); }

    virtual GG::Pt ClientLowerRight() const {
        // The axes are considered to be outside the client area.
        GG::Pt lr = GG::Wnd::ClientLowerRight();
        lr.y -= AXIS_HEIGHT;
        lr.y -= m_x_axis_label->Height() + X_AXIS_LABEL_MARGIN;
        return lr;
    }

private:
    const CombatSummary&            m_side_summary;
    std::vector<ParticipantBar*>    m_participant_bars;
    GG::Label*                      m_x_axis_label;
    GG::Label*                      m_y_axis_label;
    GG::Label*                      m_dead_label;
    const BarSizer&                 m_sizer;

    float MaxMaxHealth() {
        float max_health = -1.0f;
        for (CombatSummary::UnitSummaries::const_iterator it = m_side_summary.unit_summaries.begin();
             it != m_side_summary.unit_summaries.end(); ++it)
        { max_health = std::max(max_health, (*it)->max_health); }
        return max_health;
    }
};

/// A Bar that contains the display options
class OptionsBar : public GG::Wnd {
    struct ToggleData;
public:
    boost::signals2::signal<void ()> ChangedSignal;

    OptionsBar(boost::scoped_ptr<BarSizer>& sizer) :
        GG::Wnd(),
        m_sizer(sizer)
    {
        m_toggles.push_back(new ToggleData(UserString("COMBAT_SUMMARY_PARTICIPANT_RELATIVE"),
                                           UserString("COMBAT_SUMMARY_PARTICIPANT_EQUAL"),
                                           UserString("COMBAT_SUMMARY_PARTICIPANT_RELATIVE_TIP"),
                                           UserString("COMBAT_SUMMARY_PARTICIPANT_EQUAL_TIP"),
                                           TOGGLE_BAR_WIDTH_PROPORTIONAL, &m_sizer, this));
        m_toggles.push_back(new ToggleData(UserString("COMBAT_SUMMARY_HEALTH_SMOOTH"),
                                           UserString("COMBAT_SUMMARY_HEALTH_BAR"),
                                           UserString("COMBAT_SUMMARY_HEALTH_SMOOTH_TIP"),
                                           UserString("COMBAT_SUMMARY_HEALTH_BAR_TIP"),
                                           TOGGLE_BAR_HEALTH_SMOOTH, &m_sizer, this));
        m_toggles.push_back(new ToggleData(UserString("COMBAT_SUMMARY_BAR_HEIGHT_PROPORTIONAL"),
                                           UserString("COMBAT_SUMMARY_BAR_HEIGHT_EQUAL"),
                                           UserString("COMBAT_SUMMARY_BAR_HEIGHT_PROPORTIONAL_TIP"),
                                           UserString("COMBAT_SUMMARY_BAR_HEIGHT_EQUAL_TIP"),
                                           TOGGLE_BAR_HEIGHT_PROPORTIONAL, &m_sizer, this));
        m_toggles.push_back(new ToggleData(UserString("COMBAT_SUMMARY_GRAPH_HEIGHT_PROPORTIONAL"),
                                           UserString("COMBAT_SUMMARY_GRAPH_HEIGHT_EQUAL"),
                                           UserString("COMBAT_SUMMARY_GRAPH_HEIGHT_PROPORTIONAL_TIP"),
                                           UserString("COMBAT_SUMMARY_GRAPH_HEIGHT_EQUAL_TIP"),
                                           TOGGLE_GRAPH_HEIGHT_PROPORTIONAL, &m_sizer, this));
        DoLayout();
    }

    virtual ~OptionsBar() {
        for (std::vector<ToggleData*>::iterator it = m_toggles.begin(); it != m_toggles.end(); ++it)
        { delete *it; }
        m_toggles.clear();
    }

    virtual GG::Pt MinUsableSize() const {
        GG::Pt min_size(GG::X0, GG::Y0);

        for (std::vector<ToggleData*>::const_iterator it = m_toggles.begin();
             it != m_toggles.end(); ++it)
        { min_size.x += (*it)->button->Width() + OPTION_BUTTON_PADDING; }

        min_size.y = OPTION_BAR_HEIGHT;

        return min_size;
    }

    void DoLayout() {
        boost::shared_ptr<GG::Font> cui_font = ClientUI::GetFont();

        GG::Pt pos(GG::X(0), GG::Y(0));
        for (std::vector<ToggleData*>::iterator it = m_toggles.begin(); it != m_toggles.end(); ++it) {
            ToggleData& toggle = **it;
            toggle.button->Resize(GG::Pt(cui_font->TextExtent(toggle.button->Text(), GG::FORMAT_LEFT).x + OPTION_BUTTON_PADDING,
                                         OPTION_BUTTON_HEIGHT));
            toggle.button->MoveTo(pos);
            pos.x += toggle.button->Width() + OPTION_BUTTON_PADDING;
        }
    }

private:
    boost::scoped_ptr<BarSizer>& m_sizer;
    std::vector<ToggleData*> m_toggles;

    struct ToggleData : public boost::signals2::trackable {
        typedef bool (BarSizer::*ToggleGetter)() const;
        typedef void (BarSizer::*ToggleSetter)(bool value);

        std::string label_true;
        std::string label_false;
        std::string tip_true;
        std::string tip_false;
        std::string option_key;
        boost::scoped_ptr<BarSizer>* sizer;
        OptionsBar* parent;
        GG::Button* button;

        void Toggle()
        { SetValue(!GetValue()); }

        void SetValue(bool value) {
            (**sizer).Set(option_key, value);
            button->SetText(value?label_true:label_false);
            button->SetBrowseText(value?tip_true:tip_false);
            parent->DoLayout();
            parent->ChangedSignal();
        }

        bool GetValue() const
        { return (**sizer).Get(option_key); }

        ToggleData(const std::string& label_true, const std::string& label_false,
                   const std::string& tip_true, const std::string& tip_false,
                   std::string option_key,
                   boost::scoped_ptr<BarSizer>* sizer, OptionsBar* parent) :
            label_true(label_true),
            label_false(label_false),
            tip_true(tip_true),
            tip_false(tip_false),
            option_key(option_key),
            sizer(sizer),
            parent(parent),
            button(0)
        {
            button = new CUIButton("-");
            parent->AttachChild(button);
            GG::Connect(button->LeftClickedSignal, &ToggleData::Toggle, this);
            SetValue(GetValue());
        }
    };
};


GraphicalSummaryWnd::GraphicalSummaryWnd() :
    GG::Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::NO_WND_FLAGS),
    m_sizer(0),
    m_options_bar(0)
{}

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

    for (std::vector<SideBar*>::iterator it = m_side_boxes.begin(); it != m_side_boxes.end(); ++it ) {
        SideBar* box = *it;
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
    if (!CombatLogAvailable(log_id)) {
        ErrorLogger() << "CombatReportWnd::CombatReportPrivate::MakeSummaries: Could not find log: " << log_id;
    } else {
        const CombatLog& log = GetCombatLog(log_id);
        for (std::set<int>::const_iterator it = log.object_ids.begin(); it != log.object_ids.end(); ++it) {
            int object_id = *it;
            if (object_id < 0)
                continue;   // fighters and invalid objects
            TemporaryPtr<UniverseObject> object = GetUniverseObject(object_id);
            if (!object) {
                ErrorLogger() << "GraphicalSummaryWnd::MakeSummaries couldn't find object with id: " << object_id;
                continue;
            }

            int owner_id = object->Owner();
            if (m_summaries.find(owner_id) == m_summaries.end())
                m_summaries[owner_id] = CombatSummary(owner_id);

            std::map<int, CombatParticipantState>::const_iterator map_it = log.participant_states.find(object_id);
            if (map_it != log.participant_states.end()) {
                m_summaries[owner_id].AddUnit(object_id, map_it->second);
            } else {
                ErrorLogger() << "Participant state missing from log. Object id: " << object_id << " log id: " << log_id;
            }
        }

        for (std::map<int, CombatSummary>::iterator it = m_summaries.begin(); it != m_summaries.end(); ++it) {
            DebugLogger() << "MakeSummaries: empire " << it->first
                          << " total health: " << it->second.total_current_health
                          << " max health: " << it->second.total_max_health
                          << " units: " << it->second.unit_summaries.size();
        }
    }

    GenerateGraph();
}

void GraphicalSummaryWnd::DeleteSideBars() {
    for (std::vector<SideBar*>::iterator it = m_side_boxes.begin();
         it != m_side_boxes.end(); ++it)
    { DeleteChild(*it); }
    m_side_boxes.clear();
}

void GraphicalSummaryWnd::GenerateGraph() {
    DeleteSideBars();

    m_sizer.reset(new BarSizer(m_summaries, ClientSize()));

    for (std::map<int, CombatSummary>::iterator it = m_summaries.begin(); it != m_summaries.end(); ++it) {
        if (it->second.total_max_health > EPSILON) {
            it->second.Sort();
            SideBar* box = new SideBar(it->second, *m_sizer);
            m_side_boxes.push_back(box);
            AttachChild(box);
        }
    }

    if (m_options_bar) {
        DebugLogger() << "GraphicalSummaryWnd::GenerateGraph(): m_options_bar "
                         "already exists, calling DeleteChild(m_options_bar) "
                         "before creating a new one.";
        DeleteChild(m_options_bar);
    }
    m_options_bar = new OptionsBar(m_sizer);
    AttachChild(m_options_bar);
    GG::Connect(m_options_bar->ChangedSignal,
                &GraphicalSummaryWnd::HandleButtonChanged,
                this);

    MinSizeChangedSignal();
    DoLayout();
}
