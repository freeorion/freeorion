#include "ResearchWnd.h"

#include "../util/AppInterface.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Tech.h"
#include "../UI/TechWnd.h"

#include <boost/format.hpp>

namespace {
    const int RESEARCH_INFO_AND_QUEUE_WIDTH = 325;
    const double PI = 3.141594;
    const double OUTER_LINE_THICKNESS = 2.0;

    //////////////////////////////////////////////////
    // QueueRow
    //////////////////////////////////////////////////
    struct QueueRow : GG::ListBox::Row
    {
        QueueRow(int w, const Tech* tech_, bool in_progress, int turns_left);
        const Tech* const tech;
        static const int HEIGHT = 50;
    };

    //////////////////////////////////////////////////
    // QueueTechPanel
    //////////////////////////////////////////////////
    class QueueTechPanel : public GG::Control
    {
    public:
        QueueTechPanel(int w, const Tech* tech, bool in_progress, int turns_left, int turns_completed, double partially_complete_turn);
        virtual bool Render();

    private:
        void Draw(GG::Clr clr, bool fill);
        void ProgressLeftEndVertices(double x1, double y1, double x2, double y2);
        void ProgressRightEndVertices(double x1, double y1, double x2, double y2);
        void TraceProgressOutline(const GG::Pt& ul, const GG::Pt& lr);

        const Tech* const m_tech;
        GG::TextControl*  m_name_text;
        GG::TextControl*  m_RPs_text;
        GG::TextControl*  m_turns_remaining_text;
        bool              m_in_progress;
        int               m_total_turns;
        int               m_turns_completed;
        double            m_partially_complete_turn;
    };

    //////////////////////////////////////////////////
    // QueueRow implementation
    //////////////////////////////////////////////////
    QueueRow::QueueRow(int w, const Tech* tech_, bool in_progress, int turns_left) :
        tech(tech_)
    {
        const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
        double turn_cost = tech->ResearchCost();
        double progress = empire->ResearchStatus(tech->Name());
        if (progress == -1.0)
            progress = 0.0;
        push_back(new QueueTechPanel(w, tech_, in_progress, turns_left, static_cast<int>(progress / turn_cost), std::fmod(progress, turn_cost) / turn_cost));
    }

    //////////////////////////////////////////////////
    // QueueTechPanel implementation
    //////////////////////////////////////////////////
    QueueTechPanel::QueueTechPanel(int w, const Tech* tech, bool in_progress, int turns_left, int turns_completed, double partially_complete_turn) :
        GG::Control(0, 0, w, QueueRow::HEIGHT, 0),
        m_tech(tech),
        m_in_progress(in_progress),
        m_total_turns(tech->ResearchTurns()),
        m_turns_completed(turns_completed),
        m_partially_complete_turn(partially_complete_turn)
    {
        GG::Clr text_and_border = m_in_progress ? GG::LightColor(ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR) : ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;
        const int TEXT_PTS = ClientUI::PTS + 2;
        m_name_text = new GG::TextControl(4, 2, w - 4, QueueRow::HEIGHT - 2, UserString(tech->Name()), ClientUI::FONT, TEXT_PTS, text_and_border, GG::TF_TOP | GG::TF_LEFT);
        using boost::io::str;
        using boost::format;
        m_RPs_text = new GG::TextControl(4, QueueRow::HEIGHT - (TEXT_PTS + 4) - 2, w - 4, TEXT_PTS + 4, str(format(UserString("TECH_TURN_COST_STR")) % boost::lexical_cast<std::string>(tech->ResearchCost())), ClientUI::FONT, TEXT_PTS, text_and_border, GG::TF_LEFT);
        std::string turns_left_text = turns_left < 0 ? UserString("TECH_TURNS_LEFT_NEVER") : str(format(UserString("TECH_TURNS_LEFT_STR")) % boost::lexical_cast<std::string>(turns_left));
        m_turns_remaining_text = new GG::TextControl(4, 2, w - 8, QueueRow::HEIGHT - 2, turns_left_text, ClientUI::FONT, TEXT_PTS, text_and_border, GG::TF_TOP | GG::TF_RIGHT);
        AttachChild(m_name_text);
        AttachChild(m_RPs_text);
        AttachChild(m_turns_remaining_text);
    }

    bool QueueTechPanel::Render()
    {
        GG::Clr fill = m_in_progress ? GG::LightColor(ClientUI::RESEARCHABLE_TECH_FILL_COLOR) : ClientUI::RESEARCHABLE_TECH_FILL_COLOR;
        GG::Clr text_and_border = m_in_progress ? GG::LightColor(ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR) : ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;

        glDisable(GL_TEXTURE_2D);
 
        // render basic shape
        Draw(fill, true);
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(OUTER_LINE_THICKNESS);
        Draw(GG::Clr(text_and_border.r, text_and_border.g, text_and_border.b, 127), false);
        glLineWidth(1.0);
        glDisable(GL_LINE_SMOOTH);
        Draw(GG::Clr(text_and_border.r, text_and_border.g, text_and_border.b, 255), false);

        // render progress meter
        const int PROGRESS_METER_WIDTH = 200;
        const int PROGRESS_METER_HEIGHT = 18;
        GG::Pt ul = UpperLeft(), lr = LowerRight();
        GG::Pt meter_ul(lr.x - PROGRESS_METER_WIDTH - 8, lr.y - 8 - PROGRESS_METER_HEIGHT);
        GG::Pt meter_lr(meter_ul.x + PROGRESS_METER_WIDTH, meter_ul.y + PROGRESS_METER_HEIGHT);
        const double TURN_SEGMENT_WIDTH = PROGRESS_METER_WIDTH / static_cast<double>(m_total_turns);
        glColor4ubv(ClientUI::TECH_WND_PROGRESS_BAR_BACKGROUND.v);
        if (m_partially_complete_turn && m_turns_completed == m_total_turns - 1) {
            GG::BeginScissorClipping(static_cast<int>(meter_lr.x - TURN_SEGMENT_WIDTH), meter_ul.y,
                                     meter_lr.x, static_cast<int>(meter_lr.y - m_partially_complete_turn * PROGRESS_METER_HEIGHT));
            glBegin(GL_POLYGON);
            ProgressRightEndVertices(meter_lr.x - TURN_SEGMENT_WIDTH, meter_ul.y, meter_lr.x, meter_lr.y);
            glEnd();
            GG::EndScissorClipping();
            GG::BeginScissorClipping(static_cast<int>(meter_lr.x - TURN_SEGMENT_WIDTH),
                                     static_cast<int>(meter_lr.y - m_partially_complete_turn * PROGRESS_METER_HEIGHT),
                                     meter_lr.x, meter_lr.y);
            glColor4ubv(ClientUI::TECH_WND_PROGRESS_BAR.v);
            glBegin(GL_POLYGON);
            ProgressRightEndVertices(meter_lr.x - TURN_SEGMENT_WIDTH, meter_ul.y, meter_lr.x, meter_lr.y);
            glEnd();
            GG::EndScissorClipping();
            glColor4ubv(ClientUI::TECH_WND_PROGRESS_BAR_BACKGROUND.v);
        } else {
            glBegin(GL_POLYGON);
            ProgressRightEndVertices(meter_lr.x - TURN_SEGMENT_WIDTH, meter_ul.y, meter_lr.x, meter_lr.y);
            glEnd();
        }
        glBegin(GL_QUADS);
        if (m_turns_completed != m_total_turns - 1) {
            glVertex2d(meter_lr.x - TURN_SEGMENT_WIDTH, meter_ul.y);
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), meter_ul.y);
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), meter_lr.y);
            glVertex2d(meter_lr.x - TURN_SEGMENT_WIDTH, meter_lr.y);
        }
        if (0 < m_turns_completed && m_turns_completed < m_total_turns - 1) {
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), meter_ul.y);
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, meter_ul.y);
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, meter_lr.y - PROGRESS_METER_HEIGHT * m_partially_complete_turn);
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), meter_lr.y - PROGRESS_METER_HEIGHT * m_partially_complete_turn);
        }
        glColor4ubv(ClientUI::TECH_WND_PROGRESS_BAR.v);
        if (0 < m_turns_completed && m_turns_completed < m_total_turns - 1) {
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), meter_lr.y - PROGRESS_METER_HEIGHT * m_partially_complete_turn);
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, meter_lr.y - PROGRESS_METER_HEIGHT * m_partially_complete_turn);
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, meter_lr.y);
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), meter_lr.y);
        }
        if (m_turns_completed) {
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, meter_ul.y);
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH, meter_ul.y);
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH, meter_lr.y);
            glVertex2d(meter_ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, meter_lr.y);
        }
        glEnd();
        if (m_partially_complete_turn && !m_turns_completed) {
            GG::BeginScissorClipping(meter_ul.x, static_cast<int>(meter_lr.y - m_partially_complete_turn * PROGRESS_METER_HEIGHT),
                                     static_cast<int>(meter_ul.x + TURN_SEGMENT_WIDTH), meter_lr.y);
            glBegin(GL_POLYGON);
            ProgressLeftEndVertices(meter_ul.x, meter_ul.y, meter_ul.x + TURN_SEGMENT_WIDTH, meter_lr.y);
            glEnd();
            GG::EndScissorClipping();
            GG::BeginScissorClipping(meter_ul.x, meter_ul.y,
                                     static_cast<int>(meter_ul.x + TURN_SEGMENT_WIDTH),
                                     static_cast<int>(meter_lr.y - m_partially_complete_turn * PROGRESS_METER_HEIGHT));
            glColor4ubv(ClientUI::TECH_WND_PROGRESS_BAR_BACKGROUND.v);
            glBegin(GL_POLYGON);
            ProgressLeftEndVertices(meter_ul.x, meter_ul.y, meter_ul.x + TURN_SEGMENT_WIDTH, meter_lr.y);
            glEnd();
            GG::EndScissorClipping();
        } else {
            if (!m_turns_completed)
                glColor4ubv(ClientUI::TECH_WND_PROGRESS_BAR_BACKGROUND.v);
            glBegin(GL_POLYGON);
            ProgressLeftEndVertices(meter_ul.x, meter_ul.y, meter_ul.x + TURN_SEGMENT_WIDTH, meter_lr.y);
            glEnd();
        }
        glColor4ubv(text_and_border.v);
        glBegin(GL_LINES);
        for (double x = meter_ul.x + TURN_SEGMENT_WIDTH; x < meter_lr.x - 1.0e-5; x += TURN_SEGMENT_WIDTH) {
            glVertex2d(x, meter_ul.y);
            glVertex2d(x, meter_lr.y);
        }
        glEnd();
        glEnable(GL_LINE_SMOOTH);
        glBegin(GL_LINE_LOOP);
        ProgressLeftEndVertices(meter_ul.x, meter_ul.y, meter_ul.x + TURN_SEGMENT_WIDTH, meter_lr.y);
        ProgressRightEndVertices(meter_lr.x - TURN_SEGMENT_WIDTH, meter_ul.y, meter_lr.x, meter_lr.y);
        glEnd();
        glDisable(GL_LINE_SMOOTH);

        glEnable(GL_TEXTURE_2D);

        return true;
    }

    void QueueTechPanel::Draw(GG::Clr clr, bool fill)
    {
        const int CORNER_RADIUS = 7;
        GG::Pt ul = UpperLeft(), lr = LowerRight();
        glColor4ubv(clr.v);
        if (!fill) {
            glBegin(GL_LINE_LOOP);
            glVertex2i(lr.x, ul.y);
        }
        CircleArc(ul.x, ul.y, ul.x + 2 * CORNER_RADIUS, ul.y + 2 * CORNER_RADIUS, PI / 2.0, PI, fill);
        if (!fill)
            glVertex2i(ul.x, lr.y);
        CircleArc(lr.x - 2 * CORNER_RADIUS, lr.y - 2 * CORNER_RADIUS, lr.x, lr.y, 3.0 * PI / 2.0, 0.0, fill);
        if (!fill) {
            glEnd();
        } else {
            glBegin(GL_QUADS);
            glVertex2i(lr.x, ul.y);
            glVertex2i(lr.x - CORNER_RADIUS, ul.y);
            glVertex2i(lr.x - CORNER_RADIUS, lr.y - CORNER_RADIUS);
            glVertex2i(lr.x, lr.y - CORNER_RADIUS);
            glVertex2i(lr.x - CORNER_RADIUS, ul.y);
            glVertex2i(ul.x + CORNER_RADIUS, ul.y);
            glVertex2i(ul.x + CORNER_RADIUS, lr.y);
            glVertex2i(lr.x - CORNER_RADIUS, lr.y);
            glVertex2i(ul.x + CORNER_RADIUS, ul.y + CORNER_RADIUS);
            glVertex2i(ul.x, ul.y + CORNER_RADIUS);
            glVertex2i(ul.x, lr.y);
            glVertex2i(ul.x + CORNER_RADIUS, lr.y);
            glEnd();
        }
    }
    void QueueTechPanel::ProgressLeftEndVertices(double x1, double y1, double x2, double y2)
    {
        glVertex2d(x2, y1);
        glVertex2d(x1 + 5, y1);
        glVertex2d(x1, y1 + 4);
        glVertex2d(x1, y2 - 4);
        glVertex2d(x1 + 5, y2);
        glVertex2d(x2, y2);
    }

    void QueueTechPanel::ProgressRightEndVertices(double x1, double y1, double x2, double y2)
    {
        glVertex2d(x1, y2);
        glVertex2d(x2 - 5, y2);
        glVertex2d(x2, y2 - 4);
        glVertex2d(x2, y1 + 4);
        glVertex2d(x2 - 5, y1);
        glVertex2d(x1, y1);
    }

    void QueueTechPanel::TraceProgressOutline(const GG::Pt& ul, const GG::Pt& lr)
    {
    }

    bool temp_header_bool = RecordHeaderFile(ResearchWndRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

//////////////////////////////////////////////////
// ResearchWnd::ResearchInfoPanel               //
//////////////////////////////////////////////////
class ResearchWnd::ResearchInfoPanel : public GG::Wnd
{
public:
    ResearchInfoPanel();
    virtual bool Render();
    void Reset();

private:
    void Draw(GG::Clr clr, bool fill);

    GG::TextControl* m_title;
    GG::TextControl* m_total_RPs_label;
    GG::TextControl* m_total_RPs;
    GG::TextControl* m_total_RPs_RP_label;
    //GG::TextControl* m_projected_RPs_label; // TODO
    //GG::TextControl* m_projected_RPs; // TODO
    //GG::TextControl* m_projected_RPs_RP_label; // TODO
    GG::TextControl* m_wasted_RPs_label;
    GG::TextControl* m_wasted_RPs;
    GG::TextControl* m_wasted_RPs_RP_label;
    GG::TextControl* m_projects_in_progress_label;
    GG::TextControl* m_projects_in_progress;
    GG::TextControl* m_RPs_to_underfunded_projects_label;
    GG::TextControl* m_RPs_to_underfunded_projects;
    GG::TextControl* m_RPs_to_underfunded_projects_RP_label;
    GG::TextControl* m_projects_in_queue_label;
    GG::TextControl* m_projects_in_queue;

    std::pair<int, int> m_center_gap;

    static const int CORNER_RADIUS = 9;
    static const int VERTICAL_SECTION_GAP = 4;
};

ResearchWnd::ResearchInfoPanel::ResearchInfoPanel() :
    // this height must agree with the value NAVIGATOR_AND_DETAIL_HEIGHT in TechTreeWnd::TechTreeWnd in TechWnd.cpp
    GG::Wnd(0, 0, RESEARCH_INFO_AND_QUEUE_WIDTH, 200, 0)
{
    const int RESEARCH_TITLE_PTS = ClientUI::PTS + 10;
    const int STAT_TEXT_PTS = ClientUI::PTS;
    const int CENTERLINE_GAP = 6;
    const int LABEL_TEXT_WIDTH = (Width() - 4 - CENTERLINE_GAP) * 3 / 4;
    const int VALUE_TEXT_WIDTH = Width() - 4 - CENTERLINE_GAP - LABEL_TEXT_WIDTH;
    const int LEFT_TEXT_X = 0;
    const int RIGHT_TEXT_X = LEFT_TEXT_X + LABEL_TEXT_WIDTH + 8 + CENTERLINE_GAP;
    const int RP_LABEL_X = RIGHT_TEXT_X + 40;
    const int RP_LABEL_WIDTH = Width() - 2 - 5 - RP_LABEL_X;
    const GG::Clr TEXT_COLOR = ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR;
    m_center_gap = std::make_pair(LABEL_TEXT_WIDTH + 2, LABEL_TEXT_WIDTH + 2 + CENTERLINE_GAP);

    m_title = new GG::TextControl(2, 4, Width() - 4, RESEARCH_TITLE_PTS + 4, UserString("RESEARCH_INFO_PANEL_TITLE"), ClientUI::FONT, RESEARCH_TITLE_PTS, TEXT_COLOR);
    m_total_RPs_label = new GG::TextControl(LEFT_TEXT_X, m_title->LowerRight().y + VERTICAL_SECTION_GAP + 4, LABEL_TEXT_WIDTH, STAT_TEXT_PTS + 4, UserString("RESEARCH_INFO_TOTAL_RPS_LABEL"), ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_RIGHT);
    m_total_RPs = new GG::TextControl(RIGHT_TEXT_X, m_title->LowerRight().y + VERTICAL_SECTION_GAP + 4, VALUE_TEXT_WIDTH, STAT_TEXT_PTS + 4, "", ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_LEFT);
    m_total_RPs_RP_label = new GG::TextControl(RP_LABEL_X, m_title->LowerRight().y + VERTICAL_SECTION_GAP + 4, RP_LABEL_WIDTH, STAT_TEXT_PTS + 4, UserString("RESEARCH_INFO_RP"), ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_LEFT);
    //m_projected_RPs_label = new GG::TextControl(); // TODO
    //m_projected_RPs = new GG::TextControl(); // TODO
    //m_total_RPs_RP_label = new GG::TextControl(); // TODO
    m_wasted_RPs_label = new GG::TextControl(LEFT_TEXT_X, m_total_RPs_label->LowerRight().y, LABEL_TEXT_WIDTH, STAT_TEXT_PTS + 4, UserString("RESEARCH_INFO_WASTED_RPS_LABEL"), ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_RIGHT);
    m_wasted_RPs = new GG::TextControl(RIGHT_TEXT_X, m_total_RPs_label->LowerRight().y, VALUE_TEXT_WIDTH, STAT_TEXT_PTS + 4, "", ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_LEFT);
    m_wasted_RPs_RP_label = new GG::TextControl(RP_LABEL_X, m_total_RPs_label->LowerRight().y, RP_LABEL_WIDTH, STAT_TEXT_PTS + 4, UserString("RESEARCH_INFO_RP"), ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_LEFT);
    m_projects_in_progress_label = new GG::TextControl(LEFT_TEXT_X, m_wasted_RPs_label->LowerRight().y + VERTICAL_SECTION_GAP + 4, LABEL_TEXT_WIDTH, STAT_TEXT_PTS + 4, UserString("RESEARCH_INFO_PROJECTS_IN_PROGRESS_LABEL"), ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_RIGHT);
    m_projects_in_progress = new GG::TextControl(RIGHT_TEXT_X, m_wasted_RPs_label->LowerRight().y + VERTICAL_SECTION_GAP + 4, VALUE_TEXT_WIDTH, STAT_TEXT_PTS + 4, "", ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_LEFT);
    m_RPs_to_underfunded_projects_label = new GG::TextControl(LEFT_TEXT_X, m_projects_in_progress_label->LowerRight().y, LABEL_TEXT_WIDTH, STAT_TEXT_PTS + 4, UserString("RESEARCH_INFO_RPS_TO_UNDERFUNDED_PROJECTS_LABEL"), ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_RIGHT);
    m_RPs_to_underfunded_projects = new GG::TextControl(RIGHT_TEXT_X, m_projects_in_progress_label->LowerRight().y, VALUE_TEXT_WIDTH, STAT_TEXT_PTS + 4, "", ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_LEFT);
    m_RPs_to_underfunded_projects_RP_label = new GG::TextControl(RP_LABEL_X, m_projects_in_progress_label->LowerRight().y, RP_LABEL_WIDTH, STAT_TEXT_PTS + 4, UserString("RESEARCH_INFO_RP"), ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_LEFT);
    m_projects_in_queue_label = new GG::TextControl(LEFT_TEXT_X, m_RPs_to_underfunded_projects_label->LowerRight().y, LABEL_TEXT_WIDTH, STAT_TEXT_PTS + 4, UserString("RESEARCH_INFO_PROJECTS_IN_QUEUE_LABEL"), ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_RIGHT);
    m_projects_in_queue = new GG::TextControl(RIGHT_TEXT_X, m_RPs_to_underfunded_projects_label->LowerRight().y, VALUE_TEXT_WIDTH, STAT_TEXT_PTS + 4, "", ClientUI::FONT, STAT_TEXT_PTS, TEXT_COLOR, GG::TF_LEFT);

    AttachChild(m_title);
    AttachChild(m_total_RPs_label);
    AttachChild(m_total_RPs);
    AttachChild(m_total_RPs_RP_label);
    //AttachChild(m_projected_RPs_label); // TODO
    //AttachChild(m_projected_RPs); // TODO
    //AttachChild(m_total_RPs_RP_label); // TODO
    AttachChild(m_wasted_RPs_label);
    AttachChild(m_wasted_RPs);
    AttachChild(m_wasted_RPs_RP_label);
    AttachChild(m_projects_in_progress_label);
    AttachChild(m_projects_in_progress);
    AttachChild(m_RPs_to_underfunded_projects_label);
    AttachChild(m_RPs_to_underfunded_projects);
    AttachChild(m_RPs_to_underfunded_projects_RP_label);
    AttachChild(m_projects_in_queue_label);
    AttachChild(m_projects_in_queue);
}

void ResearchWnd::ResearchInfoPanel::Draw(GG::Clr clr, bool fill)
{
    GG::Pt ul = UpperLeft() + GG::Pt(3, 3), lr = LowerRight() - GG::Pt(3, 3);
    glColor4ubv(clr.v);
    if (!fill)
        glBegin(GL_LINE_LOOP);
    CircleArc(lr.x - 2 * CORNER_RADIUS, ul.y, lr.x, ul.y + 2 * CORNER_RADIUS, 0.0, PI / 2.0, fill);
    CircleArc(ul.x, ul.y, ul.x + 2 * CORNER_RADIUS, ul.y + 2 * CORNER_RADIUS, PI / 2.0, PI, fill);
    if (!fill) {
        glVertex2i(ul.x, m_title->LowerRight().y + 2);
        glVertex2i(lr.x, m_title->LowerRight().y + 2);
        glEnd();
    }
    std::pair<int, int> gap_to_use(m_center_gap.first + ul.x, m_center_gap.second + ul.x);
    glBegin(fill ? GL_QUADS : GL_LINE_LOOP);
    if (fill) {
        glVertex2i(lr.x - CORNER_RADIUS, ul.y);
        glVertex2i(ul.x + CORNER_RADIUS, ul.y);
        glVertex2i(ul.x + CORNER_RADIUS, ul.y + CORNER_RADIUS);
        glVertex2i(lr.x - CORNER_RADIUS, ul.y + CORNER_RADIUS);
        glVertex2i(lr.x, ul.y + CORNER_RADIUS);
        glVertex2i(ul.x, ul.y + CORNER_RADIUS);
        glVertex2i(ul.x, m_title->LowerRight().y + 2);
        glVertex2i(lr.x, m_title->LowerRight().y + 2);
    }
    glVertex2i(gap_to_use.first, m_total_RPs_label->UpperLeft().y - 2);
    glVertex2i(ul.x, m_total_RPs_label->UpperLeft().y - 2);
    glVertex2i(ul.x, m_wasted_RPs_label->LowerRight().y + 2);
    glVertex2i(gap_to_use.first, m_wasted_RPs_label->LowerRight().y + 2);
    if (!fill) {
        glEnd();
        glBegin(GL_LINE_LOOP);
    }
    glVertex2i(lr.x, m_total_RPs_label->UpperLeft().y - 2);
    glVertex2i(gap_to_use.second, m_total_RPs_label->UpperLeft().y - 2);
    glVertex2i(gap_to_use.second, m_wasted_RPs_label->LowerRight().y + 2);
    glVertex2i(lr.x, m_wasted_RPs_label->LowerRight().y + 2);
    if (!fill) {
        glEnd();
        glBegin(GL_LINE_LOOP);
    }
    glVertex2i(gap_to_use.first, m_projects_in_progress_label->UpperLeft().y - 2);
    glVertex2i(ul.x, m_projects_in_progress_label->UpperLeft().y - 2);
    glVertex2i(ul.x, m_projects_in_queue_label->LowerRight().y + 2);
    glVertex2i(gap_to_use.first, m_projects_in_queue_label->LowerRight().y + 2);
    if (!fill) {
        glEnd();
        glBegin(GL_LINE_LOOP);
    }
    glVertex2i(lr.x, m_projects_in_progress_label->UpperLeft().y - 2);
    glVertex2i(gap_to_use.second, m_projects_in_progress_label->UpperLeft().y - 2);
    glVertex2i(gap_to_use.second, m_projects_in_queue_label->LowerRight().y + 2);
    glVertex2i(lr.x, m_projects_in_queue_label->LowerRight().y + 2);
    glEnd();
    if (!fill)
        glBegin(GL_LINE_LOOP);
    CircleArc(ul.x, lr.y - 2 * CORNER_RADIUS, ul.x + 2 * CORNER_RADIUS, lr.y, PI, 3.0 * PI / 2.0, fill);
    CircleArc(lr.x - 2 * CORNER_RADIUS, lr.y - 2 * CORNER_RADIUS, lr.x, lr.y, 3.0 * PI / 2.0, 0.0, fill);
    if (!fill) {
        glVertex2i(lr.x, m_projects_in_queue_label->LowerRight().y + 2 + VERTICAL_SECTION_GAP);
        glVertex2i(ul.x, m_projects_in_queue_label->LowerRight().y + 2 + VERTICAL_SECTION_GAP);
        glEnd();
    } else {
        glBegin(GL_QUADS);
        glVertex2i(lr.x, m_projects_in_queue_label->LowerRight().y + 2 + VERTICAL_SECTION_GAP);
        glVertex2i(ul.x, m_projects_in_queue_label->LowerRight().y + 2 + VERTICAL_SECTION_GAP);
        glVertex2i(ul.x, lr.y - CORNER_RADIUS);
        glVertex2i(lr.x, lr.y - CORNER_RADIUS);

        glVertex2i(lr.x - CORNER_RADIUS, lr.y - CORNER_RADIUS);
        glVertex2i(ul.x + CORNER_RADIUS, lr.y - CORNER_RADIUS);
        glVertex2i(ul.x + CORNER_RADIUS, lr.y);
        glVertex2i(lr.x - CORNER_RADIUS, lr.y);
        glEnd();
    }
}

bool ResearchWnd::ResearchInfoPanel::Render()
{
    glDisable(GL_TEXTURE_2D);
    Draw(ClientUI::KNOWN_TECH_FILL_COLOR, true);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(OUTER_LINE_THICKNESS);
    Draw(GG::Clr(ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR.r, ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR.g, ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR.b, 127), false);
    glLineWidth(1.0);
    glDisable(GL_LINE_SMOOTH);
    Draw(GG::Clr(ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR.r, ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR.g, ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR.b, 255), false);
    glEnable(GL_TEXTURE_2D);
    return true;
}

void ResearchWnd::ResearchInfoPanel::Reset()
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const Empire::ResearchQueue& queue = empire->GetResearchQueue();
    double RPs = empire->ResearchResPool().Available();
    double total_queue_cost = queue.TotalRPsSpent();
    int projects_in_progress = queue.ProjectsInProgress();
    Empire::ResearchQueue::const_iterator underfunded_it = queue.UnderfundedProject();
    double RPs_to_underfunded_projects = underfunded_it == queue.end() ? 0.0 : underfunded_it->get<1>();
    double wasted_RPs = total_queue_cost < RPs ? RPs - total_queue_cost : 0.0;
    m_total_RPs->SetText(boost::lexical_cast<std::string>(static_cast<int>(RPs)));
    //m_projected_RPs->SetText(); // TODO
    m_wasted_RPs->SetText(boost::lexical_cast<std::string>(static_cast<int>(wasted_RPs)));
    m_projects_in_progress->SetText(boost::lexical_cast<std::string>(projects_in_progress));
    m_RPs_to_underfunded_projects->SetText(boost::lexical_cast<std::string>(static_cast<int>(RPs_to_underfunded_projects)));
    m_projects_in_queue->SetText(boost::lexical_cast<std::string>(queue.size()));
}

//////////////////////////////////////////////////
// ResearchWnd                                  //
//////////////////////////////////////////////////
ResearchWnd::ResearchWnd(int w, int h) :
    CUI_Wnd(UserString("RESEARCH_WND_TITLE"), 0, 0, w, h),
    m_research_info_panel(0),
    m_queue_lb(0),
    m_tech_tree_wnd(0)
{
    m_research_info_panel = new ResearchInfoPanel();
    m_queue_lb = new CUIListBox(2, m_research_info_panel->LowerRight().y, m_research_info_panel->Width() - 4, ClientSize().y - 4 - m_research_info_panel->Height());
    m_queue_lb->SetStyle(GG::LB_NOSORT | GG::LB_NOSEL | GG::LB_DRAGDROP | GG::LB_USERDELETE);
    m_queue_lb->SetRowHeight(QueueRow::HEIGHT + 4);
    GG::Pt tech_tree_wnd_size = ClientSize() - GG::Pt(m_research_info_panel->Width() + 6, 6);
    m_tech_tree_wnd = new TechTreeWnd(tech_tree_wnd_size.x, tech_tree_wnd_size.y);
    m_tech_tree_wnd->MoveTo(m_research_info_panel->Width() + 3, 3);

    GG::Connect(m_tech_tree_wnd->AddTechToQueueSignal(), &ResearchWnd::AddTechToQueueSlot, this);
    GG::Connect(m_queue_lb->DroppedSignal(), &ResearchWnd::QueueItemMovedSlot, this);
    GG::Connect(m_queue_lb->DeletedSignal(), &ResearchWnd::QueueItemDeletedSlot, this);
    GG::Connect(m_queue_lb->LeftClickedSignal(), &ResearchWnd::QueueItemClickedSlot, this);
    GG::Connect(m_queue_lb->DoubleClickedSignal(), &ResearchWnd::QueueItemDoubleClickedSlot, this);

    AttachChild(m_research_info_panel);
    AttachChild(m_queue_lb);
    AttachChild(m_tech_tree_wnd);
}

GG::Pt ResearchWnd::ClientUpperLeft() const
{
    return UpperLeft() + GG::Pt(LeftBorder(), TopBorder());
}

GG::Pt ResearchWnd::ClientLowerRight() const
{
    return LowerRight() - GG::Pt(RightBorder(), BottomBorder());
}

void ResearchWnd::Reset()
{
    m_tech_tree_wnd->Reset();
    m_research_info_panel->Reset();
    UpdateQueue();
}

void ResearchWnd::CenterOnTech(const std::string& tech_name)
{
    m_tech_tree_wnd->CenterOnTech(GetTech(tech_name));
}

void ResearchWnd::UpdateQueue()
{
    using boost::tuples::get;
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const Empire::ResearchQueue& queue = empire->GetResearchQueue();
    m_queue_lb->Clear();
    const int QUEUE_WIDTH = m_queue_lb->Width() - 8 - 14;
    for (Empire::ResearchQueue::const_iterator it = queue.begin(); it != queue.end(); ++it) {
        m_queue_lb->Insert(new QueueRow(QUEUE_WIDTH, get<0>(*it), get<1>(*it), get<2>(*it)));
    }
}

void ResearchWnd::AddTechToQueueSlot(const Tech* tech)
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const Empire::ResearchQueue& queue = empire->GetResearchQueue();
    if (!queue.InQueue(tech)) {
        HumanClientApp::Orders().IssueOrder(new ResearchQueueOrder(HumanClientApp::GetApp()->EmpireID(), tech->Name(), -1));
        UpdateQueue();
        m_research_info_panel->Reset();
        m_tech_tree_wnd->Update();
    }
}

void ResearchWnd::QueueItemDeletedSlot(int row_idx, const boost::shared_ptr<GG::ListBox::Row>& row)
{
    HumanClientApp::Orders().IssueOrder(new ResearchQueueOrder(HumanClientApp::GetApp()->EmpireID(), boost::dynamic_pointer_cast<QueueRow>(row)->tech->Name()));
    UpdateQueue();
    m_research_info_panel->Reset();
    m_tech_tree_wnd->Update();
}

void ResearchWnd::QueueItemMovedSlot(int row_idx, const boost::shared_ptr<GG::ListBox::Row>& row)
{
    HumanClientApp::Orders().IssueOrder(new ResearchQueueOrder(HumanClientApp::GetApp()->EmpireID(), boost::dynamic_pointer_cast<QueueRow>(row)->tech->Name(), row_idx));
    UpdateQueue();
    m_research_info_panel->Reset();
}

void ResearchWnd::QueueItemClickedSlot(int row_idx, const boost::shared_ptr<GG::ListBox::Row>& row, const GG::Pt& pt)
{
    m_tech_tree_wnd->CenterOnTech(boost::dynamic_pointer_cast<QueueRow>(row)->tech);
}

void ResearchWnd::QueueItemDoubleClickedSlot(int row_idx, const boost::shared_ptr<GG::ListBox::Row>& row)
{
    m_queue_lb->Delete(row_idx);
}
