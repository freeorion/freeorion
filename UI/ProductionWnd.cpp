#include "ProductionWnd.h"

#include "../util/AppInterface.h"
#include "BuildDesignatorWnd.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"

#include <boost/format.hpp>
#include <cmath>

namespace {
    const int PRODUCTION_INFO_AND_QUEUE_WIDTH = 250;
    const double PI = 3.141594;
    const double OUTER_LINE_THICKNESS = 2.0;

    //////////////////////////////////////////////////
    // QueueRow
    //////////////////////////////////////////////////
    struct QueueRow : GG::ListBox::Row
    {
        QueueRow(int w, const ProductionQueue::Element& build, int queue_index_);
        const int queue_index;
        static const int HEIGHT = 60;
    };

    //////////////////////////////////////////////////
    // QueueBuildPanel
    //////////////////////////////////////////////////
    class QueueBuildPanel : public GG::Control
    {
    public:
        QueueBuildPanel(int w, const ProductionQueue::Element& build, double turn_cost, int turns, int turns_completed, double partially_complete_turn);
        virtual bool Render();

    private:
        void Draw(GG::Clr clr, bool fill);

        const ProductionQueue::Element m_build;
        GG::TextControl*               m_name_text;
        GG::TextControl*               m_PPs_and_turns_text;
        GG::TextControl*               m_turns_remaining_until_next_complete_text;
        GG::TextControl*               m_turns_remaining_until_all_complete_text;
        MultiTurnProgressBar*          m_progress_bar;
        bool                           m_in_progress;
        int                            m_total_turns;
        int                            m_turns_completed;
        double                         m_partially_complete_turn;
    };

    //////////////////////////////////////////////////
    // QueueRow implementation
    //////////////////////////////////////////////////
    QueueRow::QueueRow(int w, const ProductionQueue::Element& build, int queue_index_) :
        queue_index(queue_index_)
    {
        const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
        double turn_cost;
        int turns;
        boost::tie(turn_cost, turns) = empire->ProductionCostAndTime(build.item.build_type, build.item.name);
        double progress = empire->ProductionStatus(queue_index);
        if (progress == -1.0)
            progress = 0.0;
        push_back(new QueueBuildPanel(w, build, turn_cost, turns, static_cast<int>(progress / turn_cost), std::fmod(progress, turn_cost) / turn_cost));
    }

    //////////////////////////////////////////////////
    // QueueBuildPanel implementation
    //////////////////////////////////////////////////
    QueueBuildPanel::QueueBuildPanel(int w, const ProductionQueue::Element& build, double turn_cost, int turns, int turns_completed, double partially_complete_turn) :
        GG::Control(0, 0, w, QueueRow::HEIGHT, 0),
        m_build(build),
        m_in_progress(build.spending),
        m_total_turns(turns),
        m_turns_completed(turns_completed),
        m_partially_complete_turn(partially_complete_turn)
    {
        GG::Clr text_and_border = m_in_progress ? GG::LightColor(ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR) : ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;
        std::string name_text = UserString(build.item.name);
        if (build.item.build_type == BT_SHIP)
            name_text = build.item.name;
        m_name_text = new GG::TextControl(4, 2, w - 4, QueueRow::HEIGHT - 2, name_text, ClientUI::FONT, ClientUI::PTS + 2, text_and_border, GG::TF_TOP | GG::TF_LEFT);
        m_name_text->ClipText(true);
        using boost::io::str;
        using boost::format;
        const int LOWER_TEXT_Y = QueueRow::HEIGHT - (ClientUI::PTS + 4) - 2;
        m_PPs_and_turns_text = new GG::TextControl(4, LOWER_TEXT_Y, w - 8, ClientUI::PTS + 4,
                                                   str(format(UserString("PRODUCTION_TURN_COST_STR")) % turn_cost % turns),
                                                   ClientUI::FONT, ClientUI::PTS, text_and_border, GG::TF_LEFT);
        int turns_left = build.turns_left_to_next_item;
        std::string turns_left_text = turns_left < 0 ? UserString("PRODUCTION_TURNS_LEFT_NEVER") : str(format(UserString("PRODUCTION_TURNS_LEFT_STR")) % turns_left);
        m_turns_remaining_until_next_complete_text = new GG::TextControl(4, LOWER_TEXT_Y, w - 8, ClientUI::PTS + 4, turns_left_text, ClientUI::FONT, ClientUI::PTS, text_and_border, GG::TF_RIGHT);
        const int PROGRESS_METER_MARGIN = 6;
        const int PROGRESS_METER_WIDTH = Width() - 2 * PROGRESS_METER_MARGIN;
        const int PROGRESS_METER_HEIGHT = 18;
        m_progress_bar = new MultiTurnProgressBar(PROGRESS_METER_WIDTH, PROGRESS_METER_HEIGHT, turns,
                                                  turns_completed, partially_complete_turn, ClientUI::TECH_WND_PROGRESS_BAR,
                                                  ClientUI::TECH_WND_PROGRESS_BAR_BACKGROUND, text_and_border);
        m_progress_bar->MoveTo(PROGRESS_METER_MARGIN, m_PPs_and_turns_text->UpperLeft().y - 3 - PROGRESS_METER_HEIGHT);

        AttachChild(m_name_text);
        AttachChild(m_PPs_and_turns_text);
        AttachChild(m_turns_remaining_until_next_complete_text);
        AttachChild(m_progress_bar);
    }

    bool QueueBuildPanel::Render()
    {
        GG::Clr fill = m_in_progress ? GG::LightColor(ClientUI::RESEARCHABLE_TECH_FILL_COLOR) : ClientUI::RESEARCHABLE_TECH_FILL_COLOR;
        GG::Clr text_and_border = m_in_progress ? GG::LightColor(ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR) : ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;

        glDisable(GL_TEXTURE_2D);
        Draw(fill, true);
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(OUTER_LINE_THICKNESS);
        Draw(GG::Clr(text_and_border.r, text_and_border.g, text_and_border.b, 127), false);
        glLineWidth(1.0);
        glDisable(GL_LINE_SMOOTH);
        Draw(GG::Clr(text_and_border.r, text_and_border.g, text_and_border.b, 255), false);
        glEnable(GL_TEXTURE_2D);

        return true;
    }

    void QueueBuildPanel::Draw(GG::Clr clr, bool fill)
    {
        const int CORNER_RADIUS = 7;
        GG::Pt ul = UpperLeft(), lr = LowerRight();
        glColor4ubv(clr.v);
        PartlyRoundedRect(UpperLeft(), LowerRight(), CORNER_RADIUS, true, false, true, false, fill);
    }

    bool temp_header_bool = RecordHeaderFile(ProductionWndRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


//////////////////////////////////////////////////
// ProductionWnd                                //
//////////////////////////////////////////////////
ProductionWnd::ProductionWnd(int w, int h) :
    CUI_Wnd(UserString("PRODUCTION_WND_TITLE"), 0, 0, w, h, GG::Wnd::CLICKABLE | GG::Wnd::ONTOP),
    m_production_info_panel(0),
    m_queue_lb(0),
    m_buid_designator_wnd(0)
{
    m_production_info_panel = new ProductionInfoPanel(PRODUCTION_INFO_AND_QUEUE_WIDTH, 200, UserString("PRODUCTION_INFO_PANEL_TITLE"), UserString("PRODUCTION_INFO_PP"),
                                                      OUTER_LINE_THICKNESS, ClientUI::KNOWN_TECH_FILL_COLOR, ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR);
    m_queue_lb = new CUIListBox(2, m_production_info_panel->LowerRight().y, m_production_info_panel->Width() - 4, ClientSize().y - 4 - m_production_info_panel->Height());
    m_queue_lb->SetStyle(GG::LB_NOSORT | GG::LB_NOSEL | GG::LB_DRAGDROP | GG::LB_USERDELETE);
    m_queue_lb->SetRowHeight(QueueRow::HEIGHT + 4);
    GG::Pt buid_designator_wnd_size = ClientSize() - GG::Pt(m_production_info_panel->Width() + 6, 6);
    m_buid_designator_wnd = new BuildDesignatorWnd(buid_designator_wnd_size.x, buid_designator_wnd_size.y);
    m_buid_designator_wnd->MoveTo(m_production_info_panel->Width() + 3, 3);

    GG::Connect(m_buid_designator_wnd->AddBuildToQueueSignal, &ProductionWnd::AddBuildToQueueSlot, this);
    GG::Connect(m_queue_lb->DroppedSignal, &ProductionWnd::QueueItemMovedSlot, this);
    GG::Connect(m_queue_lb->DeletedSignal, &ProductionWnd::QueueItemDeletedSlot, this);
    GG::Connect(m_queue_lb->LeftClickedSignal, &ProductionWnd::QueueItemClickedSlot, this);
    GG::Connect(m_queue_lb->DoubleClickedSignal, &ProductionWnd::QueueItemDoubleClickedSlot, this);

    AttachChild(m_production_info_panel);
    AttachChild(m_queue_lb);
    AttachChild(m_buid_designator_wnd);
}

GG::Pt ProductionWnd::ClientUpperLeft() const
{
    return UpperLeft() + GG::Pt(LeftBorder(), TopBorder());
}

GG::Pt ProductionWnd::ClientLowerRight() const
{
    return LowerRight() - GG::Pt(RightBorder(), BottomBorder());
}

bool ProductionWnd::InWindow(const GG::Pt& pt) const
{
    GG::Rect clip_rect = m_buid_designator_wnd->MapViewHole() + m_buid_designator_wnd->UpperLeft();
    return clip_rect.Contains(pt) ? m_buid_designator_wnd->InWindow(pt) : CUI_Wnd::InWindow(pt);
}

bool ProductionWnd::InClient(const GG::Pt& pt) const
{
    GG::Rect clip_rect = m_buid_designator_wnd->MapViewHole() + m_buid_designator_wnd->UpperLeft();
    return clip_rect.Contains(pt) ? m_buid_designator_wnd->InClient(pt) : CUI_Wnd::InClient(pt);
}

bool ProductionWnd::Render()
{
    GG::Rect clip_rect = m_buid_designator_wnd->MapViewHole() + m_buid_designator_wnd->UpperLeft();
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt cl_ul = ul + GG::Pt(BORDER_LEFT, BORDER_TOP);
    GG::Pt cl_lr = lr - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM);

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glColor4ubv(ClientUI::WND_COLOR.v);
    glBegin(GL_QUADS);
    glVertex2i(ul.x, ul.y);
    glVertex2i(lr.x, ul.y);
    glVertex2i(lr.x, clip_rect.ul.y);
    glVertex2i(ul.x, clip_rect.ul.y);

    glVertex2i(ul.x, clip_rect.ul.y);
    glVertex2i(clip_rect.ul.x, clip_rect.ul.y);
    glVertex2i(clip_rect.ul.x, clip_rect.lr.y);
    glVertex2i(ul.x, clip_rect.lr.y);

    glVertex2i(clip_rect.lr.x, clip_rect.ul.y);
    glVertex2i(lr.x, clip_rect.ul.y);
    glVertex2i(lr.x, clip_rect.lr.y);
    glVertex2i(clip_rect.lr.x, clip_rect.lr.y);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2i(ul.x, clip_rect.lr.y);
    glVertex2i(lr.x, clip_rect.lr.y);
    glVertex2i(lr.x, lr.y - OUTER_EDGE_ANGLE_OFFSET);
    glVertex2i(lr.x - OUTER_EDGE_ANGLE_OFFSET, lr.y);
    glVertex2i(ul.x, lr.y);
    glVertex2i(ul.x, clip_rect.lr.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
    glColor4ubv(ClientUI::WND_OUTER_BORDER_COLOR.v);
    glVertex2i(ul.x, ul.y);
    glVertex2i(lr.x, ul.y);
    glVertex2i(lr.x, lr.y - OUTER_EDGE_ANGLE_OFFSET);
    glVertex2i(lr.x - OUTER_EDGE_ANGLE_OFFSET, lr.y);
    glVertex2i(ul.x, lr.y);
    glVertex2i(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    // draw inner border, including extra resize-tab lines
    glBegin(GL_LINE_STRIP);
    glColor4ubv(ClientUI::WND_INNER_BORDER_COLOR.v);
    glVertex2i(cl_ul.x, cl_ul.y);
    glVertex2i(cl_lr.x, cl_ul.y);
    glVertex2i(cl_lr.x, cl_lr.y - INNER_BORDER_ANGLE_OFFSET);
    glVertex2i(cl_lr.x - INNER_BORDER_ANGLE_OFFSET, cl_lr.y);
    glVertex2i(cl_ul.x, cl_lr.y);
    glVertex2i(cl_ul.x, cl_ul.y);
    glEnd();
    glBegin(GL_LINES);
    // draw the extra lines of the resize tab
    if (m_resizable) {
        glColor4ubv(ClientUI::WND_INNER_BORDER_COLOR.v);
    } else {
        glColor4ubv(GG::DisabledColor(ClientUI::WND_INNER_BORDER_COLOR).v);
    }
    glVertex2i(cl_lr.x, cl_lr.y - RESIZE_HASHMARK1_OFFSET);
    glVertex2i(cl_lr.x - RESIZE_HASHMARK1_OFFSET, cl_lr.y);
            
    glVertex2i(cl_lr.x, cl_lr.y - RESIZE_HASHMARK2_OFFSET);
    glVertex2i(cl_lr.x - RESIZE_HASHMARK2_OFFSET, cl_lr.y);
    glEnd();
    glEnable(GL_TEXTURE_2D);

    glColor4ubv(ClientUI::TEXT_COLOR.v);
    boost::shared_ptr<GG::Font> font = GG::App::GetApp()->GetFont(ClientUI::TITLE_FONT, ClientUI::TITLE_PTS);
    font->RenderText(ul.x + BORDER_LEFT, ul.y, WindowText());

    return true;
}

void ProductionWnd::Reset()
{
    ResetInfoPanel();
    UpdateQueue();
    m_queue_lb->BringRowIntoView(0);
    m_buid_designator_wnd->Reset();
}

void ProductionWnd::CenterOnBuild(/* TODO */)
{
    m_buid_designator_wnd->CenterOnBuild(/* TODO */);
}

void ProductionWnd::SelectSystem(int system)
{
    m_buid_designator_wnd->SelectSystem(system);
}

void ProductionWnd::Sanitize()
{
    m_buid_designator_wnd->Clear();
}

void ProductionWnd::UpdateQueue()
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ProductionQueue& queue = empire->GetProductionQueue();
    int first_visible_queue_row = m_queue_lb->FirstRowShown();
    int original_queue_length = m_queue_lb->NumRows();
    m_queue_lb->Clear();
    const int QUEUE_WIDTH = m_queue_lb->Width() - 8 - 14;
    int i = 0;
    for (ProductionQueue::const_iterator it = queue.begin(); it != queue.end(); ++it, ++i) {
        m_queue_lb->Insert(new QueueRow(QUEUE_WIDTH, *it, i));
    }
    m_queue_lb->BringRowIntoView(m_queue_lb->NumRows() - 1);
    if (m_queue_lb->NumRows() <= original_queue_length)
        m_queue_lb->BringRowIntoView(first_visible_queue_row);
}

void ProductionWnd::ResetInfoPanel()
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ProductionQueue& queue = empire->GetProductionQueue();
    double PPs = empire->ProductionPoints();
    double total_queue_cost = queue.TotalPPsSpent();
    ProductionQueue::const_iterator underfunded_it = queue.UnderfundedProject(empire);
    double PPs_to_underfunded_projects = underfunded_it == queue.end() ? 0.0 : underfunded_it->spending;
    m_production_info_panel->Reset(PPs, total_queue_cost, queue.ProjectsInProgress(), PPs_to_underfunded_projects, queue.size());
}

void ProductionWnd::AddBuildToQueueSlot(BuildType build_type, const std::string& name, int number, int location)
{
    HumanClientApp::Orders().IssueOrder(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), build_type, name, number, location));
    UpdateQueue();
    ResetInfoPanel();
}

void ProductionWnd::QueueItemDeletedSlot(int row_idx, const boost::shared_ptr<GG::ListBox::Row>& row)
{
    HumanClientApp::Orders().IssueOrder(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), row_idx));
    UpdateQueue();
    ResetInfoPanel();
}

void ProductionWnd::QueueItemMovedSlot(int row_idx, const boost::shared_ptr<GG::ListBox::Row>& row)
{
    HumanClientApp::Orders().IssueOrder(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), boost::static_pointer_cast<QueueRow>(row)->queue_index, row_idx));
    UpdateQueue();
    ResetInfoPanel();
}

void ProductionWnd::QueueItemClickedSlot(int row_idx, const boost::shared_ptr<GG::ListBox::Row>& row, const GG::Pt& pt)
{
    m_buid_designator_wnd->CenterOnBuild(/*TODO*/);
}

void ProductionWnd::QueueItemDoubleClickedSlot(int row_idx, const boost::shared_ptr<GG::ListBox::Row>& row)
{
    m_queue_lb->Delete(row_idx);
}
