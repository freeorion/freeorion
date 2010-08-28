#include "ResearchWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "QueueListBox.h"
#include "../Empire/Empire.h"
#include "../universe/Tech.h"
#include "../util/MultiplayerCommon.h"
#include "../util/AppInterface.h"
#include "../UI/TechTreeWnd.h"
#include "../client/human/HumanClientApp.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

#include <boost/cast.hpp>

#include <cmath>

namespace {
    const GG::X RESEARCH_INFO_AND_QUEUE_WIDTH(250);
    const double PI = 3.141594;
    const float OUTER_LINE_THICKNESS = 2.0f;

    //////////////////////////////////////////////////
    // QueueRow
    //////////////////////////////////////////////////
    struct QueueRow : GG::ListBox::Row {
        QueueRow(GG::X w, const Tech* tech_, double allocated_rp, int turns_left);
        const Tech* const tech;
    };

    //////////////////////////////////////////////////
    // QueueTechPanel
    //////////////////////////////////////////////////
    class QueueTechPanel : public GG::Control {
    public:
        QueueTechPanel(GG::X w, const Tech* tech, double allocated_rp, int turns_left, int turns_completed, double partially_complete_turn);
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
    QueueRow::QueueRow(GG::X w, const Tech* tech_, double allocated_rp, int turns_left) :
        GG::ListBox::Row(),
        tech(tech_)
    {
        const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
        double turn_cost = tech->ResearchCost();
        double progress = empire->ResearchStatus(tech->Name());
        if (progress == -1.0)
            progress = 0.0;

        GG::Control* panel = new QueueTechPanel(w, tech_, allocated_rp, turns_left, static_cast<int>(progress / turn_cost), std::fmod(progress, turn_cost) / turn_cost);
        Resize(panel->Size());
        push_back(panel);

        SetDragDropDataType("RESEARCH_QUEUE_ROW");
    }

    //////////////////////////////////////////////////
    // QueueTechPanel implementation
    //////////////////////////////////////////////////
    QueueTechPanel::QueueTechPanel(GG::X w, const Tech* tech, double turn_spending, int turns_left, int turns_completed, double partially_complete_turn) :
        GG::Control(GG::X0, GG::Y0, w, GG::Y(10), GG::Flags<GG::WndFlag>()),
        m_tech(tech),
        m_in_progress(turn_spending),
        m_total_turns(tech->ResearchTurns()),
        m_turns_completed(turns_completed),
        m_partially_complete_turn(partially_complete_turn)
    {
        const int MARGIN = 2;

        const int FONT_PTS = ClientUI::Pts();
        const GG::Y METER_HEIGHT(FONT_PTS);

        const GG::Y HEIGHT = MARGIN + FONT_PTS + MARGIN + METER_HEIGHT + MARGIN + FONT_PTS + MARGIN + 6;

        const int GRAPHIC_SIZE = Value(HEIGHT - 9);    // 9 pixels accounts for border thickness so the sharp-cornered icon doesn't with the rounded panel corner

        const GG::X NAME_WIDTH = w - GRAPHIC_SIZE - 2*MARGIN - 3;
        const GG::X METER_WIDTH = w - GRAPHIC_SIZE - 3*MARGIN - 3;
        const GG::X TURNS_AND_COST_WIDTH = NAME_WIDTH/2;

        Resize(GG::Pt(w, HEIGHT));


        GG::Clr clr = m_in_progress ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor()) : ClientUI::ResearchableTechTextAndBorderColor();
        boost::shared_ptr<GG::Font> font = ClientUI::GetFont();

        GG::Y top(MARGIN);
        GG::X left(MARGIN);


        m_icon = new GG::StaticGraphic(left, top, GG::X(GRAPHIC_SIZE), GG::Y(GRAPHIC_SIZE), ClientUI::TechTexture(m_tech->Name()), GG::GRAPHIC_FITGRAPHIC);
        m_icon->SetColor(ClientUI::CategoryColor(m_tech->Category()));

        left += m_icon->Width() + MARGIN;

        m_name_text = new GG::TextControl(left, top, NAME_WIDTH, GG::Y(FONT_PTS + 2*MARGIN), UserString(tech->Name()), font, clr, GG::FORMAT_TOP | GG::FORMAT_LEFT);
        m_name_text->ClipText(true);

        top += m_name_text->Height();    // not sure why I need two margins here... otherwise the progress bar appears over the bottom of the text

        m_progress_bar = new MultiTurnProgressBar(METER_WIDTH, METER_HEIGHT, tech->ResearchTurns(),
                                                  turns_completed, partially_complete_turn, ClientUI::TechWndProgressBarColor(),
                                                  ClientUI::TechWndProgressBarBackgroundColor(), clr);
        m_progress_bar->MoveTo(GG::Pt(left, top));

        top += m_progress_bar->Height() + MARGIN;

        using boost::io::str;

        std::string turns_cost_text = str(FlexibleFormat(UserString("TECH_TURN_COST_STR")) % DoubleToString(turn_spending, 3, false));
        m_RPs_and_turns_text = new GG::TextControl(left, top, TURNS_AND_COST_WIDTH, GG::Y(FONT_PTS + MARGIN),
                                                   turns_cost_text, font, clr, GG::FORMAT_LEFT);

        left += TURNS_AND_COST_WIDTH;

        std::string turns_left_text = turns_left < 0 ? UserString("TECH_TURNS_LEFT_NEVER") : str(FlexibleFormat(UserString("TECH_TURNS_LEFT_STR")) % turns_left);
        m_turns_remaining_text = new GG::TextControl(left, top, TURNS_AND_COST_WIDTH, GG::Y(FONT_PTS + MARGIN),
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
        glLineWidth(static_cast<GLfloat>(OUTER_LINE_THICKNESS));
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
ResearchWnd::ResearchWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE | GG::ONTOP),
    m_research_info_panel(0),
    m_queue_lb(0),
    m_tech_tree_wnd(0)
{
    m_research_info_panel = new ProductionInfoPanel(RESEARCH_INFO_AND_QUEUE_WIDTH, GG::Y(200), UserString("RESEARCH_INFO_PANEL_TITLE"), UserString("RESEARCH_INFO_RP"),
                                                    OUTER_LINE_THICKNESS, ClientUI::KnownTechFillColor(), ClientUI::KnownTechTextAndBorderColor());

    m_queue_lb = new QueueListBox(GG::X(2), m_research_info_panel->LowerRight().y,
                                  m_research_info_panel->Width() - 4, ClientSize().y - 4 - m_research_info_panel->Height(),
                                  "RESEARCH_QUEUE_ROW");
    GG::Connect(m_queue_lb->QueueItemMoved, &ResearchWnd::QueueItemMoved, this);
    m_queue_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL | GG::LIST_USERDELETE);

    GG::Pt tech_tree_wnd_size = ClientSize() - GG::Pt(m_research_info_panel->Width(), GG::Y0);
    m_tech_tree_wnd = new TechTreeWnd(tech_tree_wnd_size.x, tech_tree_wnd_size.y);
    m_tech_tree_wnd->MoveTo(GG::Pt(m_research_info_panel->Width(), GG::Y0));

    GG::Connect(m_tech_tree_wnd->AddTechToQueueSignal,          &ResearchWnd::AddTechToQueueSlot, this);
    GG::Connect(m_tech_tree_wnd->AddMultipleTechsToQueueSignal, &ResearchWnd::AddMultipleTechsToQueueSlot, this);
    GG::Connect(m_queue_lb->ErasedSignal,                       &ResearchWnd::QueueItemDeletedSlot, this);
    GG::Connect(m_queue_lb->LeftClickedSignal,                  &ResearchWnd::QueueItemClickedSlot, this);
    GG::Connect(m_queue_lb->DoubleClickedSignal,                &ResearchWnd::QueueItemDoubleClickedSlot, this);

    AttachChild(m_research_info_panel);
    AttachChild(m_queue_lb);
    AttachChild(m_tech_tree_wnd);

    SetChildClippingMode(ClipToClient);
}

ResearchWnd::~ResearchWnd()
{
    m_empire_connection.disconnect();
}

void ResearchWnd::Refresh()
{
    // useful at start of turn or when loading empire from save.
    // since empire object is recreated based on turn update from server, 
    // connections of signals emitted from the empire must be remade
    m_empire_connection.disconnect();
    EmpireManager& manager = HumanClientApp::GetApp()->Empires();
    if (Empire* empire = manager.Lookup(HumanClientApp::GetApp()->EmpireID()))
        m_empire_connection = GG::Connect(empire->GetResearchQueue().ResearchQueueChangedSignal,
                                          &ResearchWnd::ResearchQueueChangedSlot, this);
    Update();
}

void ResearchWnd::Reset()
{
    m_tech_tree_wnd->Reset();
    UpdateQueue();
    UpdateInfoPanel();
    m_queue_lb->BringRowIntoView(m_queue_lb->begin());
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

void ResearchWnd::ShowTech(const std::string& tech_name)
{
    m_tech_tree_wnd->CenterOnTech(GetTech(tech_name));
    m_tech_tree_wnd->SetEncyclopediaTech(GetTech(tech_name));
    m_tech_tree_wnd->SelectTech(GetTech(tech_name));
}

void ResearchWnd::QueueItemMoved(GG::ListBox::Row* row, std::size_t position)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(
        OrderPtr(new ResearchQueueOrder(HumanClientApp::GetApp()->EmpireID(),
                                        boost::polymorphic_downcast<QueueRow*>(row)->tech->Name(),
                                        position)));
}

void ResearchWnd::Sanitize()
{
    m_tech_tree_wnd->Clear();
}

void ResearchWnd::Render()
{}

void ResearchWnd::ResearchQueueChangedSlot()
{
    UpdateQueue();
    UpdateInfoPanel();
}

void ResearchWnd::UpdateQueue()
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ResearchQueue& queue = empire->GetResearchQueue();
    std::size_t first_visible_queue_row = std::distance(m_queue_lb->begin(), m_queue_lb->FirstRowShown());
    m_queue_lb->Clear();
    const GG::X QUEUE_WIDTH = m_queue_lb->Width() - 8 - 14;

    for (ResearchQueue::const_iterator it = queue.begin(); it != queue.end(); ++it) {
        m_queue_lb->Insert(new QueueRow(QUEUE_WIDTH, it->tech, it->allocated_rp, it->turns_left));
    }

    if (!m_queue_lb->Empty())
        m_queue_lb->BringRowIntoView(--m_queue_lb->end());
    if (first_visible_queue_row < m_queue_lb->NumRows())
        m_queue_lb->BringRowIntoView(boost::next(m_queue_lb->begin(), first_visible_queue_row));
}

void ResearchWnd::UpdateInfoPanel()
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ResearchQueue& queue = empire->GetResearchQueue();
    double RPs = empire->ResourceProduction(RE_RESEARCH);
    double total_queue_cost = queue.TotalRPsSpent();
    ResearchQueue::const_iterator underfunded_it = queue.UnderfundedProject();
    double RPs_to_underfunded_projects = underfunded_it == queue.end() ? 0.0 : underfunded_it->allocated_rp;
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
        m_tech_tree_wnd->Update();
    }
}

void ResearchWnd::AddMultipleTechsToQueueSlot(const std::vector<const Tech*>& tech_vec)
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

    m_tech_tree_wnd->Update();
}

void ResearchWnd::QueueItemDeletedSlot(GG::ListBox::iterator it)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(
        OrderPtr(new ResearchQueueOrder(HumanClientApp::GetApp()->EmpireID(),
                                        boost::polymorphic_downcast<QueueRow*>(*it)->tech->Name())));
    m_tech_tree_wnd->Update();
    ResearchQueueChangedSlot();
}

void ResearchWnd::QueueItemClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt)
{
    const Tech* tech = boost::polymorphic_downcast<QueueRow*>(*it)->tech;
    if (!tech) {
        Logger().errorStream() << "ResearchWnd::QueueItemClickedSlot couldn't get tech from clicked row";
        return;
    }
    m_tech_tree_wnd->CenterOnTech(tech);
    m_tech_tree_wnd->SetEncyclopediaTech(tech);
}

void ResearchWnd::QueueItemDoubleClickedSlot(GG::ListBox::iterator it)
{
    m_queue_lb->ErasedSignal(it);
}
