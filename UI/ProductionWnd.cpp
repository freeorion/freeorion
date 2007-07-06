#include "ProductionWnd.h"

#include "../util/AppInterface.h"
#include "BuildDesignatorWnd.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Building.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

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
    };

    //////////////////////////////////////////////////
    // QueueListBox
    //////////////////////////////////////////////////
    class QueueListBox : public CUIListBox
    {
    public:
        QueueListBox(int x, int y, int w, int h, ProductionWnd* production_wnd) :
            CUIListBox(x, y, w, h),
            m_production_wnd(production_wnd),
            m_drop_point(-1)
        {}
        // HACK!  This is sort of a dirty trick, but we return false here in all cases, even when we accept the dropped
        // item.  This keeps things simpler than if we handled ListBox::DroppedRow signals, since we are explicitly
        // updating everything on drops anyway.
        virtual void AcceptDrops(std::list<Wnd*>& wnds, const GG::Pt& pt)
        {
            assert(wnds.size() == 1);
            if ((*wnds.begin())->DragDropDataType() == "PRODUCTION_QUEUE_ROW") {
                GG::ListBox::Row* row = static_cast<GG::ListBox::Row*>(*wnds.begin());
                int original_row_idx = -1;
                for (int i = 0; i < NumRows(); ++i) {
                    if (&GetRow(i) == row) {
                        original_row_idx = i;
                        break;
                    }
                }
                assert(original_row_idx != -1);
                int row_idx = RowUnderPt(pt);
                if (row_idx < 0 || row_idx > NumRows())
                    row_idx = NumRows();
                m_production_wnd->QueueItemMoved(row_idx, row);
            }
            wnds.clear();
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
        virtual void DragDropEnter(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, Uint32 keys)
        {
            DragDropHere(pt, drag_drop_wnds, keys);
        }
        virtual void DragDropHere(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, Uint32 keys)
        {
            if (drag_drop_wnds.size() == 1 && drag_drop_wnds.begin()->first->DragDropDataType() == "PRODUCTION_QUEUE_ROW") {
                m_drop_point = RowUnderPt(pt);
                if (m_drop_point < 0)
                    m_drop_point = 0;
                if (NumRows() < m_drop_point)
                    m_drop_point = NumRows();
            } else {
                m_drop_point = -1;
            }
        }
        virtual void DragDropLeave()
        {
            m_drop_point = -1;
        }
    private:
        ProductionWnd* m_production_wnd;
        int            m_drop_point;
    };

    //////////////////////////////////////////////////
    // QueueBuildPanel
    //////////////////////////////////////////////////
    class QueueBuildPanel : public GG::Control
    {
    public:
        QueueBuildPanel(int w, const ProductionQueue::Element& build, double turn_cost, int turns, int number, int turns_completed, double partially_complete_turn);
        virtual void Render();

    private:
        void Draw(GG::Clr clr, bool fill);

        const ProductionQueue::Element m_build;
        GG::TextControl*               m_name_text;
        GG::TextControl*               m_PPs_and_turns_text;
        GG::TextControl*               m_turns_remaining_until_next_complete_text;
        GG::StaticGraphic*             m_icon;
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
        GG::ListBox::Row(),
        queue_index(queue_index_)
    {
        const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
        double turn_cost;
        int turns;
        boost::tie(turn_cost, turns) = empire->ProductionCostAndTime(build.item);
        double progress = empire->ProductionStatus(queue_index);
        if (progress == -1.0)
            progress = 0.0;

        GG::Control* panel = new QueueBuildPanel(w, build, turn_cost, turns, build.remaining, static_cast<int>(progress / turn_cost), std::fmod(progress, turn_cost) / turn_cost);
        Resize(panel->Size());
        push_back(panel);

        SetDragDropDataType("PRODUCTION_QUEUE_ROW");
    }

    //////////////////////////////////////////////////
    // QueueBuildPanel implementation
    //////////////////////////////////////////////////
    QueueBuildPanel::QueueBuildPanel(int w, const ProductionQueue::Element& build, double turn_cost, int turns, int number, int turns_completed, double partially_complete_turn) :
        GG::Control(0, 0, w, 10, 0),
        m_build(build),
        m_in_progress(build.spending),
        m_total_turns(turns),
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

        // get graphic and player-visible name text for item
        boost::shared_ptr<GG::Texture> graphic;
        std::string name_text;
        if (build.item.build_type == BT_BUILDING) {
            graphic = ClientUI::BuildingTexture(GetBuildingType(build.item.name));
            name_text = UserString(build.item.name);
        } else if (build.item.build_type == BT_SHIP) {
            graphic = ClientUI::GetTexture(ClientUI::ArtDir() / GetShipDesign(build.item.design_id)->graphic);
            name_text = GetShipDesign(build.item.design_id)->name;
        } else if (build.item.build_type == BT_ORBITAL) {
            graphic = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "base1.png"); // this is a kludge for v0.3 only
            name_text = UserString(build.item.name);
        } else {
            graphic = ClientUI::GetTexture(""); // get "missing texture" texture by supply intentionally bad path
            name_text = UserString("FW_UNKNOWN_DESIGN_NAME");
        }

        using boost::io::str;
        using boost::format;

        // things other than buildings can be built in multiple copies with one order
        if (build.item.build_type != BT_BUILDING)
            name_text = str(format(UserString("PRODUCTION_QUEUE_MULTIPLES")) % number) + name_text;


        // create and arrange widgets to display info
        int top = MARGIN;
        int left = MARGIN;

        if (graphic)
            m_icon = new GG::StaticGraphic(left, top, GRAPHIC_SIZE, GRAPHIC_SIZE, graphic, GG::GR_FITGRAPHIC);
        else
            m_icon = 0;

        left += GRAPHIC_SIZE + MARGIN;

        m_name_text = new GG::TextControl(left, top, NAME_WIDTH, FONT_PTS + 2*MARGIN, name_text, font, clr, GG::TF_TOP | GG::TF_LEFT);
        m_name_text->ClipText(true);

        top += m_name_text->Height();    // not sure why I need two margins here... otherwise the progress bar appears over the bottom of the text
        
        m_progress_bar = new MultiTurnProgressBar(METER_WIDTH, METER_HEIGHT, turns, turns_completed, 
                                                  partially_complete_turn, ClientUI::TechWndProgressBar(),
                                                  ClientUI::TechWndProgressBarBackground(), clr);
        m_progress_bar->MoveTo(GG::Pt(left, top));

        top += m_progress_bar->Height() + MARGIN;

        std::string turns_cost_text = str(format(UserString("PRODUCTION_TURN_COST_STR")) % turn_cost % turns);
        m_PPs_and_turns_text = new GG::TextControl(left, top, TURNS_AND_COST_WIDTH, FONT_PTS + MARGIN,
                                                   turns_cost_text, font, clr, GG::TF_LEFT);

        left += TURNS_AND_COST_WIDTH;
        

        int turns_left = build.turns_left_to_next_item;
        std::string turns_left_text = turns_left < 0 ? UserString("PRODUCTION_TURNS_LEFT_NEVER") : str(format(UserString("PRODUCTION_TURNS_LEFT_STR")) % turns_left);
        m_turns_remaining_until_next_complete_text = new GG::TextControl(left, top, TURNS_AND_COST_WIDTH, FONT_PTS + MARGIN,
                                                                         turns_left_text, font, clr, GG::TF_RIGHT);
        m_turns_remaining_until_next_complete_text->ClipText(true);

        if (m_icon) AttachChild(m_icon);
        AttachChild(m_name_text);
        AttachChild(m_PPs_and_turns_text);
        AttachChild(m_turns_remaining_until_next_complete_text);
        AttachChild(m_progress_bar);
    }

    void QueueBuildPanel::Render()
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

    void QueueBuildPanel::Draw(GG::Clr clr, bool fill)
    {
        const int CORNER_RADIUS = 7;
        GG::Pt ul = UpperLeft(), lr = LowerRight();
        glColor(clr);
        PartlyRoundedRect(UpperLeft(), LowerRight(), CORNER_RADIUS, true, false, true, false, fill);
    }
}


//////////////////////////////////////////////////
// ProductionWnd                                //
//////////////////////////////////////////////////
ProductionWnd::ProductionWnd(int w, int h) :
    GG::Wnd(0, 0, w, h, GG::ONTOP),
    m_production_info_panel(0),
    m_queue_lb(0),
    m_build_designator_wnd(0)
{
    m_production_info_panel = new ProductionInfoPanel(PRODUCTION_INFO_AND_QUEUE_WIDTH, 200, UserString("PRODUCTION_INFO_PANEL_TITLE"), UserString("PRODUCTION_INFO_PP"),
                                                      OUTER_LINE_THICKNESS, ClientUI::KnownTechFillColor(), ClientUI::KnownTechTextAndBorderColor());
    m_queue_lb = new QueueListBox(2, m_production_info_panel->LowerRight().y, m_production_info_panel->Width() - 4, ClientSize().y - 4 - m_production_info_panel->Height(), this);
    m_queue_lb->SetStyle(GG::LB_NOSORT | GG::LB_NOSEL | GG::LB_USERDELETE);
    GG::Pt buid_designator_wnd_size = ClientSize() - GG::Pt(m_production_info_panel->Width(), 6);
    m_build_designator_wnd = new BuildDesignatorWnd(buid_designator_wnd_size.x, buid_designator_wnd_size.y);
    m_build_designator_wnd->MoveTo(GG::Pt(m_production_info_panel->Width(), 0));

    EnableChildClipping(true);

    GG::Connect(m_build_designator_wnd->AddNamedBuildToQueueSignal, &ProductionWnd::AddBuildToQueueSlot, this);
    GG::Connect(m_build_designator_wnd->AddIDedBuildToQueueSignal, &ProductionWnd::AddBuildToQueueSlot, this);
    GG::Connect(m_build_designator_wnd->BuildQuantityChangedSignal, &ProductionWnd::ChangeBuildQuantitySlot, this);
    GG::Connect(m_build_designator_wnd->SystemSelectedSignal, SystemSelectedSignal);
    GG::Connect(m_queue_lb->ErasedSignal, &ProductionWnd::QueueItemDeletedSlot, this);
    GG::Connect(m_queue_lb->LeftClickedSignal, &ProductionWnd::QueueItemClickedSlot, this);
    GG::Connect(m_queue_lb->DoubleClickedSignal, &ProductionWnd::QueueItemDoubleClickedSlot, this);

    AttachChild(m_production_info_panel);
    AttachChild(m_queue_lb);
    AttachChild(m_build_designator_wnd);
}

ProductionWnd::~ProductionWnd()
{
    // disconnect all signals
    while (!m_misc_connections.empty()) {
        m_misc_connections.begin()->disconnect();
        m_misc_connections.erase(m_misc_connections.begin());
    }
}

bool ProductionWnd::InWindow(const GG::Pt& pt) const
{
    GG::Rect clip_rect = m_build_designator_wnd->MapViewHole() + m_build_designator_wnd->UpperLeft();
    return clip_rect.Contains(pt) ? m_build_designator_wnd->InWindow(pt) : GG::Wnd::InWindow(pt);
}

bool ProductionWnd::InClient(const GG::Pt& pt) const
{
    GG::Rect clip_rect = m_build_designator_wnd->MapViewHole() + m_build_designator_wnd->UpperLeft();
    return clip_rect.Contains(pt) ? m_build_designator_wnd->InClient(pt) : GG::Wnd::InClient(pt);
}

void ProductionWnd::Render()
{
    GG::Rect clip_rect = m_build_designator_wnd->MapViewHole() + m_build_designator_wnd->UpperLeft();
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glColor(ClientUI::WndColor());
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
    glVertex2i(lr.x, lr.y);
    glVertex2i(ul.x, lr.y);
    glVertex2i(ul.x, clip_rect.lr.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);
}

void ProductionWnd::Reset()
{
    ResetInfoPanel();
    UpdateQueue();
    m_queue_lb->BringRowIntoView(0);
    m_build_designator_wnd->Reset();
}

void ProductionWnd::CenterOnBuild(int queue_idx)
{
    m_build_designator_wnd->CenterOnBuild(queue_idx);
}

void ProductionWnd::SelectPlanet(int planet)
{
    m_build_designator_wnd->SelectPlanet(planet);
}

void ProductionWnd::SelectSystem(int system)
{
    m_build_designator_wnd->SelectSystem(system);
}

void ProductionWnd::QueueItemMoved(int row_idx, GG::ListBox::Row* row)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), static_cast<QueueRow*>(row)->queue_index, row_idx));
    UpdateQueue();
    ResetInfoPanel();
}

void ProductionWnd::Sanitize()
{
    m_build_designator_wnd->Clear();
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
    
    for (ProductionQueue::const_iterator it = queue.begin(); it != queue.end(); ++it, ++i)
        m_queue_lb->Insert(new QueueRow(QUEUE_WIDTH, *it, i));

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
    /* Altering production queue may have freed up or required more PP, which may require extra
       or free up excess minerals.  Signalling that the MineralResPool has changed causes the
       MapWnd to be signalled that that pool has changed, which causes the resource indicator
       to be updated (which polls the ProductionQueue to determine how many PPs are being spent) */
    empire->GetMineralResPool().ChangedSignal();
}

void ProductionWnd::AddBuildToQueueSlot(BuildType build_type, const std::string& name, int number, int location)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), build_type, name, number, location));
    UpdateQueue();
    ResetInfoPanel();
    m_build_designator_wnd->CenterOnBuild(m_queue_lb->NumRows() - 1);
}

void ProductionWnd::AddBuildToQueueSlot(BuildType build_type, int design_id, int number, int location)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), build_type, design_id, number, location));
    UpdateQueue();
    ResetInfoPanel();
    m_build_designator_wnd->CenterOnBuild(m_queue_lb->NumRows() - 1);
}

void ProductionWnd::ChangeBuildQuantitySlot(int queue_idx, int quantity)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), queue_idx, quantity, true));
    UpdateQueue();
    ResetInfoPanel();
}

void ProductionWnd::QueueItemDeletedSlot(int row_idx, GG::ListBox::Row* row)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), row_idx));
    UpdateQueue();      ///< rebuild on-screen queue
    ResetInfoPanel();
}

void ProductionWnd::QueueItemClickedSlot(int row_idx, GG::ListBox::Row* row, const GG::Pt& pt)
{
    m_build_designator_wnd->CenterOnBuild(row_idx);
}

void ProductionWnd::QueueItemDoubleClickedSlot(int row_idx, GG::ListBox::Row* row)
{
    // TODO: if there is progress on this item, ask for confirmation before actually deleting it
    delete m_queue_lb->Erase(row_idx);
}
