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
#include <GG/StaticGraphic.h>

#include <boost/cast.hpp>
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
    };

    //////////////////////////////////////////////////
    // QueueListBox
    //////////////////////////////////////////////////
    class QueueListBox : public CUIListBox
    {
    public:
        QueueListBox(int x, int y, int w, int h, ResearchWnd* research_wnd) :
            CUIListBox(x, y, w, h),
            m_research_wnd(research_wnd),
            m_drop_point(-1)
        {
            AllowDropType("RESEARCH_QUEUE_ROW");
        }

        virtual void DropsAcceptable(DropsAcceptableIter first,
                                     DropsAcceptableIter last,
                                     const GG::Pt& pt) const
        {
            assert(std::distance(first, last) == 1);
            for (DropsAcceptableIter it = first; it != last; ++it) {
                it->second = AllowedDropTypes().find(it->first->DragDropDataType()) != AllowedDropTypes().end();
            }
        }

        virtual void AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt)
        {
            assert(wnds.size() == 1);
            GG::ListBox::Row* row =
                boost::polymorphic_downcast<GG::ListBox::Row*>(*wnds.begin());
            assert(std::find(Begin(), End(), row) != End());
            int row_idx = RowUnderPt(pt);
            if (row_idx < 0 || row_idx > NumRows())
                row_idx = NumRows();
            m_research_wnd->QueueItemMoved(row_idx, row);
        }

        virtual void Render()
        {
            ListBox::Render();
            if (m_drop_point != -1) {
                GG::ListBox::Row& row = GetRow(m_drop_point == NumRows() ? NumRows() - 1 : m_drop_point);
                GG::Control* panel = row[0];
                GG::Pt ul = row.UpperLeft(), lr = row.LowerRight();
                if (m_drop_point == NumRows())
                    ul.y = lr.y;
                ul.x = panel->UpperLeft().x;
                lr.x = panel->LowerRight().x;
                GG::FlatRectangle(ul.x, ul.y - 1, lr.x, ul.y, GG::CLR_ZERO, GG::CLR_WHITE, 1);
            }
        }
        virtual void DragDropHere(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys)
        {
            if (drag_drop_wnds.size() == 1 && AllowedDropTypes().find(drag_drop_wnds.begin()->first->DragDropDataType()) != AllowedDropTypes().end()) {
                m_drop_point = RowUnderPt(pt);
                if (m_drop_point < 0)
                    m_drop_point = 0;
                if (NumRows() < m_drop_point)
                    m_drop_point = NumRows();
            } else {
                m_drop_point = -1;
            }
            ListBox::DragDropHere(pt, drag_drop_wnds, mod_keys);
        }
        virtual void DragDropLeave()
        {
            m_drop_point = -1;
            ListBox::DragDropLeave();
        }
    private:
        ResearchWnd* m_research_wnd;
        int          m_drop_point;
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

        const Tech* const       m_tech;
        GG::TextControl*        m_name_text;
        GG::TextControl*        m_RPs_and_turns_text;
        GG::TextControl*        m_turns_remaining_text;
        GG::StaticGraphic*      m_icon;
        MultiTurnProgressBar*   m_progress_bar;
        bool                    m_in_progress;
        int                     m_total_turns;
        int                     m_turns_completed;
        double                  m_partially_complete_turn;
    };

    //////////////////////////////////////////////////
    // QueueRow implementation
    //////////////////////////////////////////////////
    QueueRow::QueueRow(int w, const Tech* tech_, bool in_progress, int turns_left) :
        GG::ListBox::Row(),
        tech(tech_)
    {
        const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
        double turn_cost = tech->ResearchCost();
        double progress = empire->ResearchStatus(tech->Name());
        if (progress == -1.0)
            progress = 0.0;

        GG::Control* panel = new QueueTechPanel(w, tech_, in_progress, turns_left, static_cast<int>(progress / turn_cost), std::fmod(progress, turn_cost) / turn_cost);
        Resize(panel->Size());
        push_back(panel);
        
        SetDragDropDataType("RESEARCH_QUEUE_ROW");
    }

    //////////////////////////////////////////////////
    // QueueTechPanel implementation
    //////////////////////////////////////////////////
    QueueTechPanel::QueueTechPanel(int w, const Tech* tech, bool in_progress, int turns_left, int turns_completed, double partially_complete_turn) :
        GG::Control(0, 0, w, 10, GG::Flags<GG::WndFlag>()),
        m_tech(tech),
        m_in_progress(in_progress),
        m_total_turns(tech->ResearchTurns()),
        m_turns_completed(turns_completed),
        m_partially_complete_turn(partially_complete_turn)
    {
        const int MARGIN = 2;

        const int FONT_PTS = ClientUI::Pts();
        const int METER_HEIGHT = FONT_PTS;

        const int HEIGHT = MARGIN + FONT_PTS + MARGIN + METER_HEIGHT + MARGIN + FONT_PTS + MARGIN + 6;

        const int GRAPHIC_SIZE = HEIGHT - 9;    // 9 pixels accounts for border thickness so the sharp-cornered icon doesn't with the rounded panel corner

        const int NAME_WIDTH = w - GRAPHIC_SIZE - 2*MARGIN - 3;
        const int METER_WIDTH = w - GRAPHIC_SIZE - 3*MARGIN - 3;
        const int TURNS_AND_COST_WIDTH = NAME_WIDTH/2;

        Resize(GG::Pt(w, HEIGHT));


        GG::Clr clr = m_in_progress ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor()) : ClientUI::ResearchableTechTextAndBorderColor();
        boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());

        int top = MARGIN;
        int left = MARGIN;


        m_icon = new GG::StaticGraphic(left, top, GRAPHIC_SIZE, GRAPHIC_SIZE, ClientUI::TechTexture(m_tech->Name()), GG::GRAPHIC_FITGRAPHIC);
        m_icon->SetColor(ClientUI::CategoryColor(m_tech->Category()));

        left += m_icon->Width() + MARGIN;

        m_name_text = new GG::TextControl(left, top, NAME_WIDTH, FONT_PTS + 2*MARGIN, UserString(tech->Name()), font, clr, GG::FORMAT_TOP | GG::FORMAT_LEFT);
        m_name_text->ClipText(true);

        top += m_name_text->Height();    // not sure why I need two margins here... otherwise the progress bar appears over the bottom of the text
        
        m_progress_bar = new MultiTurnProgressBar(METER_WIDTH, METER_HEIGHT, tech->ResearchTurns(),
                                                  turns_completed, partially_complete_turn, ClientUI::TechWndProgressBar(),
                                                  ClientUI::TechWndProgressBarBackground(), clr);
        m_progress_bar->MoveTo(GG::Pt(left, top));

        top += m_progress_bar->Height() + MARGIN;
        
        using boost::io::str;
        using boost::format;

        std::string turns_cost_text = str(format(UserString("TECH_TURN_COST_STR")) % tech->ResearchCost() % tech->ResearchTurns());
        m_RPs_and_turns_text = new GG::TextControl(left, top, TURNS_AND_COST_WIDTH, FONT_PTS + MARGIN,
                                                   turns_cost_text, font, clr, GG::FORMAT_LEFT);

        left += TURNS_AND_COST_WIDTH;
        
        std::string turns_left_text = turns_left < 0 ? UserString("TECH_TURNS_LEFT_NEVER") : str(format(UserString("TECH_TURNS_LEFT_STR")) % turns_left);
        m_turns_remaining_text = new GG::TextControl(left, top, TURNS_AND_COST_WIDTH, FONT_PTS + MARGIN,
                                                     turns_left_text, font, clr, GG::FORMAT_RIGHT);
        m_turns_remaining_text->ClipText(true);


        AttachChild(m_name_text);
        AttachChild(m_RPs_and_turns_text);
        AttachChild(m_turns_remaining_text);
        AttachChild(m_icon);
        AttachChild(m_progress_bar);
    }

    void QueueTechPanel::Render()
    {
        GG::Clr fill = m_in_progress ? GG::LightColor(ClientUI::ResearchableTechFillColor()) : ClientUI::ResearchableTechFillColor();
        GG::Clr text_and_border = m_in_progress ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor()) : ClientUI::ResearchableTechTextAndBorderColor();

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
        glColor(clr);
        PartlyRoundedRect(UpperLeft(), LowerRight(), CORNER_RADIUS, true, false, true, false, fill);
    }

}


//////////////////////////////////////////////////
// ResearchWnd                                  //
//////////////////////////////////////////////////
ResearchWnd::ResearchWnd(int w, int h) :
    GG::Wnd(0, 0, w, h, GG::ONTOP),
    m_research_info_panel(0),
    m_queue_lb(0),
    m_tech_tree_wnd(0)
{
    m_research_info_panel = new ProductionInfoPanel(RESEARCH_INFO_AND_QUEUE_WIDTH, 200, UserString("RESEARCH_INFO_PANEL_TITLE"), UserString("RESEARCH_INFO_RP"),
                                                    OUTER_LINE_THICKNESS, ClientUI::KnownTechFillColor(), ClientUI::KnownTechTextAndBorderColor());
    m_queue_lb = new QueueListBox(2, m_research_info_panel->LowerRight().y, m_research_info_panel->Width() - 4, ClientSize().y - 4 - m_research_info_panel->Height(), this);
    m_queue_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL | GG::LIST_USERDELETE);
    GG::Pt tech_tree_wnd_size = ClientSize() - GG::Pt(m_research_info_panel->Width() + 6, 6);
    m_tech_tree_wnd = new TechTreeWnd(tech_tree_wnd_size.x, tech_tree_wnd_size.y);
    m_tech_tree_wnd->MoveTo(GG::Pt(m_research_info_panel->Width() + 3, 3));

    GG::Connect(m_tech_tree_wnd->AddTechToQueueSignal, &ResearchWnd::AddTechToQueueSlot, this);
    GG::Connect(m_tech_tree_wnd->AddMultipleTechsToQueueSignal, &ResearchWnd::AddMultipleTechsToQueueSlot, this);
    GG::Connect(m_queue_lb->ErasedSignal, &ResearchWnd::QueueItemDeletedSlot, this);
    GG::Connect(m_queue_lb->LeftClickedSignal, &ResearchWnd::QueueItemClickedSlot, this);
    GG::Connect(m_queue_lb->DoubleClickedSignal, &ResearchWnd::QueueItemDoubleClickedSlot, this);

    AttachChild(m_research_info_panel);
    AttachChild(m_queue_lb);
    AttachChild(m_tech_tree_wnd);

    EnableChildClipping(true);
}

void ResearchWnd::Reset()
{
    m_tech_tree_wnd->Reset();
    UpdateQueue();
    UpdateInfoPanel();
    m_queue_lb->BringRowIntoView(0);
}

void ResearchWnd::Update()
{
    m_tech_tree_wnd->Update();
    UpdateQueue();
    UpdateInfoPanel();
}

void ResearchWnd::CenterOnTech(const std::string& tech_name)
{
    m_tech_tree_wnd->CenterOnTech(GetTech(tech_name));
}

void ResearchWnd::QueueItemMoved(int row_idx, GG::ListBox::Row* row)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ResearchQueueOrder(HumanClientApp::GetApp()->EmpireID(), dynamic_cast<QueueRow*>(row)->tech->Name(), row_idx)));
    UpdateQueue();
    UpdateInfoPanel();
}

void ResearchWnd::Sanitize()
{
    m_tech_tree_wnd->Clear();
}

void ResearchWnd::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);
    glEnable(GL_TEXTURE_2D);
}

void ResearchWnd::UpdateQueue()
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ResearchQueue& queue = empire->GetResearchQueue();
    int first_visible_queue_row = m_queue_lb->FirstRowShown();
    int original_queue_length = m_queue_lb->NumRows();
    m_queue_lb->Clear();
    const int QUEUE_WIDTH = m_queue_lb->Width() - 8 - 14;
    for (ResearchQueue::const_iterator it = queue.begin(); it != queue.end(); ++it) {
        m_queue_lb->Insert(new QueueRow(QUEUE_WIDTH, it->tech, it->spending, it->turns_left));
    }
    m_queue_lb->BringRowIntoView(m_queue_lb->NumRows() - 1);
    if (m_queue_lb->NumRows() <= original_queue_length)
        m_queue_lb->BringRowIntoView(first_visible_queue_row);
}

void ResearchWnd::UpdateInfoPanel()
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ResearchQueue& queue = empire->GetResearchQueue();
    double RPs = empire->ResourceProduction(RE_RESEARCH);
    double total_queue_cost = queue.TotalRPsSpent();
    ResearchQueue::const_iterator underfunded_it = queue.UnderfundedProject();
    double RPs_to_underfunded_projects = underfunded_it == queue.end() ? 0.0 : underfunded_it->spending;
    m_research_info_panel->Reset(RPs, total_queue_cost, queue.ProjectsInProgress(), RPs_to_underfunded_projects, queue.size());
    /* Altering research queue may have freed up or required more RP.  Signalling that the
       ResearchResPool has changed causes the MapWnd to be signalled that that pool has changed,
       which causes the resource indicator to be updated (which polls the ResearchQueue to
       determine how many RPs are being spent).  If/when RP are stockpilable, this might matter,
       so then the following line should be uncommented.*/
    //empire->GetResearchResPool().ChangedSignal(); 
}

void ResearchWnd::AddTechToQueueSlot(const Tech* tech)
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ResearchQueue& queue = empire->GetResearchQueue();
    if (!queue.InQueue(tech)) {
        HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ResearchQueueOrder(HumanClientApp::GetApp()->EmpireID(), tech->Name(), -1)));
        UpdateQueue();
        UpdateInfoPanel();
        m_tech_tree_wnd->Update();
    }
}

void ResearchWnd::AddMultipleTechsToQueueSlot(std::vector<const Tech*> tech_vec)
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ResearchQueue& queue = empire->GetResearchQueue();
    const int id = HumanClientApp::GetApp()->EmpireID();
    OrderSet& orders = HumanClientApp::GetApp()->Orders();
    for (std::vector<const Tech*>::const_iterator it = tech_vec.begin(); it != tech_vec.end(); ++it) {
        const Tech* tech = *it;
        if (!queue.InQueue(tech))
            orders.IssueOrder(OrderPtr(new ResearchQueueOrder(id, tech->Name(), -1)));
    }

    UpdateQueue();
    UpdateInfoPanel();
    m_tech_tree_wnd->Update();
}

void ResearchWnd::QueueItemDeletedSlot(int row_idx, GG::ListBox::Row* row)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ResearchQueueOrder(HumanClientApp::GetApp()->EmpireID(), dynamic_cast<QueueRow*>(row)->tech->Name())));
    UpdateQueue();
    UpdateInfoPanel();
    m_tech_tree_wnd->Update();
}

void ResearchWnd::QueueItemClickedSlot(int row_idx, GG::ListBox::Row* row, const GG::Pt& pt)
{
    m_tech_tree_wnd->CenterOnTech(dynamic_cast<QueueRow*>(row)->tech);
}

void ResearchWnd::QueueItemDoubleClickedSlot(int row_idx, GG::ListBox::Row* row)
{
    delete m_queue_lb->Erase(row_idx);
}
