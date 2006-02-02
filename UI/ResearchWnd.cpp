#include "ResearchWnd.h"

#include "../util/AppInterface.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Tech.h"
#include "../UI/TechTreeWnd.h"

#include <GG/DrawUtil.h>

#include <boost/format.hpp>
#include <cmath>

namespace {
    const int RESEARCH_INFO_AND_QUEUE_WIDTH = 250;
    const double PI = 3.141594;
    const double OUTER_LINE_THICKNESS = 2.0;

    //////////////////////////////////////////////////
    // QueueRow
    //////////////////////////////////////////////////
    struct QueueRow : GG::ListBox::Row
    {
        QueueRow(int w, const Tech* tech_, bool in_progress, int turns_left);
        const Tech* const tech;
        static const int HEIGHT = 64;
    };

    //////////////////////////////////////////////////
    // QueueTechPanel
    //////////////////////////////////////////////////
    class QueueTechPanel : public GG::Control
    {
    public:
        QueueTechPanel(int w, const Tech* tech, bool in_progress, int turns_left, int turns_completed, double partially_complete_turn);
        virtual void Render();

    private:
        void Draw(GG::Clr clr, bool fill);

        const Tech* const m_tech;
        GG::TextControl*  m_name_text;
        GG::TextControl*  m_RPs_and_turns_text;
        GG::TextControl*  m_turns_remaining_text;
        MultiTurnProgressBar* m_progress_bar;
        bool              m_in_progress;
        int               m_total_turns;
        int               m_turns_completed;
        double            m_partially_complete_turn;
    };

    //////////////////////////////////////////////////
    // QueueRow implementation
    //////////////////////////////////////////////////
    QueueRow::QueueRow(int w, const Tech* tech_, bool in_progress, int turns_left) :
        GG::ListBox::Row(w, HEIGHT, ""),
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
        m_name_text = new GG::TextControl(4, 2, w - 4, QueueRow::HEIGHT - 2, UserString(tech->Name()), GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS + 2), text_and_border, GG::TF_TOP | GG::TF_LEFT);
        m_name_text->ClipText(true);
        using boost::io::str;
        using boost::format;
        const int LOWER_TEXT_Y = QueueRow::HEIGHT - (ClientUI::PTS + 4) - 4;
        m_RPs_and_turns_text = new GG::TextControl(4, LOWER_TEXT_Y, w - 8, ClientUI::PTS + 4,
                                                   str(format(UserString("TECH_TURN_COST_STR")) % tech->ResearchCost() % tech->ResearchTurns()),
                                                   GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS), text_and_border, GG::TF_LEFT);
        std::string turns_left_text = turns_left < 0 ? UserString("TECH_TURNS_LEFT_NEVER") : str(format(UserString("TECH_TURNS_LEFT_STR")) % turns_left);
        m_turns_remaining_text = new GG::TextControl(4, LOWER_TEXT_Y, w - 8, ClientUI::PTS + 4, turns_left_text, GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS), text_and_border, GG::TF_RIGHT);
        const int PROGRESS_METER_MARGIN = 6;
        const int PROGRESS_METER_WIDTH = Width() - 2 * PROGRESS_METER_MARGIN;
        const int PROGRESS_METER_HEIGHT = 18;
        m_progress_bar = new MultiTurnProgressBar(PROGRESS_METER_WIDTH, PROGRESS_METER_HEIGHT, tech->ResearchTurns(),
                                                  turns_completed, partially_complete_turn, ClientUI::TECH_WND_PROGRESS_BAR,
                                                  ClientUI::TECH_WND_PROGRESS_BAR_BACKGROUND, text_and_border);
        m_progress_bar->MoveTo(GG::Pt(PROGRESS_METER_MARGIN, m_RPs_and_turns_text->UpperLeft().y - 3 - PROGRESS_METER_HEIGHT));

        AttachChild(m_name_text);
        AttachChild(m_RPs_and_turns_text);
        AttachChild(m_turns_remaining_text);
        AttachChild(m_progress_bar);
    }

    void QueueTechPanel::Render()
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
    }

    void QueueTechPanel::Draw(GG::Clr clr, bool fill)
    {
        const int CORNER_RADIUS = 7;
        GG::Pt ul = UpperLeft(), lr = LowerRight();
        glColor4ubv(clr.v);
        PartlyRoundedRect(UpperLeft(), LowerRight(), CORNER_RADIUS, true, false, true, false, fill);
    }

    bool temp_header_bool = RecordHeaderFile(ResearchWndRevision());
    bool temp_source_bool = RecordSourceFile("$Id$");
}


//////////////////////////////////////////////////
// ResearchWnd                                  //
//////////////////////////////////////////////////
ResearchWnd::ResearchWnd(int w, int h) :
    CUIWnd(UserString("RESEARCH_WND_TITLE"), 0, 0, w, h, GG::CLICKABLE | GG::ONTOP),
    m_research_info_panel(0),
    m_queue_lb(0),
    m_tech_tree_wnd(0)
{
    m_research_info_panel = new ProductionInfoPanel(RESEARCH_INFO_AND_QUEUE_WIDTH, 200, UserString("RESEARCH_INFO_PANEL_TITLE"), UserString("RESEARCH_INFO_RP"),
                                                    OUTER_LINE_THICKNESS, ClientUI::KNOWN_TECH_FILL_COLOR, ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR);
    m_queue_lb = new CUIListBox(2, m_research_info_panel->LowerRight().y, m_research_info_panel->Width() - 4, ClientSize().y - 4 - m_research_info_panel->Height());
    m_queue_lb->SetStyle(GG::LB_NOSORT | GG::LB_NOSEL | GG::LB_USERDELETE);
    GG::Pt tech_tree_wnd_size = ClientSize() - GG::Pt(m_research_info_panel->Width() + 6, 6);
    m_tech_tree_wnd = new TechTreeWnd(tech_tree_wnd_size.x, tech_tree_wnd_size.y);
    m_tech_tree_wnd->MoveTo(GG::Pt(m_research_info_panel->Width() + 3, 3));

    GG::Connect(m_tech_tree_wnd->AddTechToQueueSignal, &ResearchWnd::AddTechToQueueSlot, this);
    GG::Connect(m_queue_lb->DroppedSignal, &ResearchWnd::QueueItemMovedSlot, this);
    GG::Connect(m_queue_lb->ErasedSignal, &ResearchWnd::QueueItemDeletedSlot, this);
    GG::Connect(m_queue_lb->LeftClickedSignal, &ResearchWnd::QueueItemClickedSlot, this);
    GG::Connect(m_queue_lb->DoubleClickedSignal, &ResearchWnd::QueueItemDoubleClickedSlot, this);

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
    ResetInfoPanel();
    UpdateQueue();
    m_queue_lb->BringRowIntoView(0);
}

void ResearchWnd::CenterOnTech(const std::string& tech_name)
{
    m_tech_tree_wnd->CenterOnTech(GetTech(tech_name));
}

void ResearchWnd::Sanitize()
{
    m_tech_tree_wnd->Clear();
}

void ResearchWnd::UpdateQueue()
{
    using boost::tuples::get;
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ResearchQueue& queue = empire->GetResearchQueue();
    int first_visible_queue_row = m_queue_lb->FirstRowShown();
    int original_queue_length = m_queue_lb->NumRows();
    m_queue_lb->Clear();
    const int QUEUE_WIDTH = m_queue_lb->Width() - 8 - 14;
    for (ResearchQueue::const_iterator it = queue.begin(); it != queue.end(); ++it) {
        m_queue_lb->Insert(new QueueRow(QUEUE_WIDTH, get<0>(*it), get<1>(*it), get<2>(*it)));
    }
    m_queue_lb->BringRowIntoView(m_queue_lb->NumRows() - 1);
    if (m_queue_lb->NumRows() <= original_queue_length)
        m_queue_lb->BringRowIntoView(first_visible_queue_row);
}

void ResearchWnd::ResetInfoPanel()
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ResearchQueue& queue = empire->GetResearchQueue();
    double RPs = empire->ResearchResPool().Production();
    double total_queue_cost = queue.TotalRPsSpent();
    ResearchQueue::const_iterator underfunded_it = queue.UnderfundedProject();
    double RPs_to_underfunded_projects = underfunded_it == queue.end() ? 0.0 : underfunded_it->get<1>();
    m_research_info_panel->Reset(RPs, total_queue_cost, queue.ProjectsInProgress(), RPs_to_underfunded_projects, queue.size());
}

void ResearchWnd::AddTechToQueueSlot(const Tech* tech)
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ResearchQueue& queue = empire->GetResearchQueue();
    if (!queue.InQueue(tech)) {
        HumanClientApp::Orders().IssueOrder(new ResearchQueueOrder(HumanClientApp::GetApp()->EmpireID(), tech->Name(), -1));
        UpdateQueue();
        ResetInfoPanel();
        m_tech_tree_wnd->Update();
    }
}

void ResearchWnd::QueueItemDeletedSlot(int row_idx, GG::ListBox::Row* row)
{
    HumanClientApp::Orders().IssueOrder(new ResearchQueueOrder(HumanClientApp::GetApp()->EmpireID(), dynamic_cast<QueueRow*>(row)->tech->Name()));
    UpdateQueue();
    ResetInfoPanel();
    m_tech_tree_wnd->Update();
}

void ResearchWnd::QueueItemMovedSlot(int row_idx, GG::ListBox::Row* row)
{
    HumanClientApp::Orders().IssueOrder(new ResearchQueueOrder(HumanClientApp::GetApp()->EmpireID(), dynamic_cast<QueueRow*>(row)->tech->Name(), row_idx));
    UpdateQueue();
    ResetInfoPanel();
}

void ResearchWnd::QueueItemClickedSlot(int row_idx, GG::ListBox::Row* row, const GG::Pt& pt)
{
    m_tech_tree_wnd->CenterOnTech(dynamic_cast<QueueRow*>(row)->tech);
}

void ResearchWnd::QueueItemDoubleClickedSlot(int row_idx, GG::ListBox::Row* row)
{
    delete m_queue_lb->Erase(row_idx);
}
