#include "ProductionWnd.h"

#include "../util/AppInterface.h"
#include "BuildDesignatorWnd.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "QueueListBox.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Building.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

#include <boost/cast.hpp>
#include <boost/format.hpp>

#include <cmath>


namespace {
    const GG::X PRODUCTION_INFO_AND_QUEUE_WIDTH(250);
    const double PI = 3.141594;
    const double OUTER_LINE_THICKNESS = 2.0;

    //////////////////////////////////////////////////
    // QueueRow
    //////////////////////////////////////////////////
    struct QueueRow : GG::ListBox::Row
    {
        QueueRow(GG::X w, const ProductionQueue::Element& build, int queue_index_);
        const int queue_index;
    };

    //////////////////////////////////////////////////
    // QueueBuildPanel
    //////////////////////////////////////////////////
    class QueueBuildPanel : public GG::Control
    {
    public:
        QueueBuildPanel(GG::X w, const ProductionQueue::Element& build, double turn_cost, int turns, int number, int turns_completed, double partially_complete_turn);
        virtual void Render();

    private:
        void Draw(GG::Clr clr, bool fill);

        const ProductionQueue::Element  m_build;
        GG::TextControl*                m_name_text;
        GG::TextControl*                m_location_text;
        GG::TextControl*                m_PPs_and_turns_text;
        GG::TextControl*                m_turns_remaining_until_next_complete_text;
        GG::StaticGraphic*              m_icon;
        MultiTurnProgressBar*           m_progress_bar;
        bool                            m_in_progress;
        int                             m_total_turns;
        int                             m_turns_completed;
        double                          m_partially_complete_turn;
    };

    //////////////////////////////////////////////////
    // QueueRow implementation
    //////////////////////////////////////////////////
    QueueRow::QueueRow(GG::X w, const ProductionQueue::Element& build, int queue_index_) :
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

        GG::Control* panel = new QueueBuildPanel(w, build, build.allocated_pp, turns, build.remaining, static_cast<int>(progress / turn_cost), std::fmod(progress, turn_cost) / turn_cost);
        Resize(panel->Size());
        push_back(panel);

        SetDragDropDataType("PRODUCTION_QUEUE_ROW");
    }

    //////////////////////////////////////////////////
    // QueueBuildPanel implementation
    //////////////////////////////////////////////////
    QueueBuildPanel::QueueBuildPanel(GG::X w, const ProductionQueue::Element& build, double turn_spending, int turns, int number, int turns_completed, double partially_complete_turn) :
        GG::Control(GG::X0, GG::Y0, w, GG::Y(10), GG::Flags<GG::WndFlag>()),
        m_build(build),
        m_name_text(0),
        m_location_text(0),
        m_PPs_and_turns_text(0),
        m_turns_remaining_until_next_complete_text(0),
        m_icon(0),
        m_progress_bar(0),
        m_in_progress(build.allocated_pp),
        m_total_turns(turns),
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

        // get graphic and player-visible name text for item
        boost::shared_ptr<GG::Texture> graphic;
        std::string name_text;
        if (build.item.build_type == BT_BUILDING) {
            graphic = ClientUI::BuildingTexture(build.item.name);
            name_text = UserString(build.item.name);
        } else if (build.item.build_type == BT_SHIP) {
            graphic = ClientUI::ShipIcon(build.item.design_id);
            name_text = GetShipDesign(build.item.design_id)->Name();
        } else {
            graphic = ClientUI::GetTexture(""); // get "missing texture" texture by supply intentionally bad path
            name_text = UserString("FW_UNKNOWN_DESIGN_NAME");
        }

        using boost::io::str;
        using boost::format;

        // things other than buildings can be built in multiple copies with one order
        if (build.item.build_type != BT_BUILDING)
            name_text = str(format(UserString("PRODUCTION_QUEUE_MULTIPLES")) % number) + name_text;


        // get location indicator text
        std::string location_text;
        if (const UniverseObject* location = GetUniverse().Object(build.location))
            location_text = str(format(UserString("PRODUCTION_QUEUE_ITEM_LOCATION")) % location->Name());


        // create and arrange widgets to display info
        GG::Y top(MARGIN);
        GG::X left(MARGIN);

        if (graphic)
            m_icon = new GG::StaticGraphic(left, top, GG::X(GRAPHIC_SIZE), GG::Y(GRAPHIC_SIZE), graphic, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        else
            m_icon = 0;

        left += GRAPHIC_SIZE + MARGIN;

        m_name_text = new GG::TextControl(left, top, NAME_WIDTH, GG::Y(FONT_PTS + 2*MARGIN), name_text, font, clr, GG::FORMAT_TOP | GG::FORMAT_LEFT);
        m_name_text->ClipText(true);

        m_location_text = new GG::TextControl(left, top, NAME_WIDTH, GG::Y(FONT_PTS + 2*MARGIN), location_text, font, clr, GG::FORMAT_TOP | GG::FORMAT_RIGHT);

        top += m_name_text->Height();    // not sure why I need two margins here... otherwise the progress bar appears over the bottom of the text

        m_progress_bar = new MultiTurnProgressBar(METER_WIDTH, METER_HEIGHT, turns, turns_completed, 
                                                  partially_complete_turn, ClientUI::TechWndProgressBar(),
                                                  ClientUI::TechWndProgressBarBackground(), clr);
        m_progress_bar->MoveTo(GG::Pt(left, top));

        top += m_progress_bar->Height() + MARGIN;

        std::string turn_spending_text = str(FlexibleFormat(UserString("PRODUCTION_TURN_COST_STR")) % DoubleToString(turn_spending, 3, false));
        m_PPs_and_turns_text = new GG::TextControl(left, top, TURNS_AND_COST_WIDTH, GG::Y(FONT_PTS + MARGIN),
                                                   turn_spending_text, font, clr, GG::FORMAT_LEFT);

        left += TURNS_AND_COST_WIDTH;


        int turns_left = build.turns_left_to_next_item;
        std::string turns_left_text = turns_left < 0 ? UserString("PRODUCTION_TURNS_LEFT_NEVER") : str(format(UserString("PRODUCTION_TURNS_LEFT_STR")) % turns_left);
        m_turns_remaining_until_next_complete_text = new GG::TextControl(left, top, TURNS_AND_COST_WIDTH, GG::Y(FONT_PTS + MARGIN),
                                                                         turns_left_text, font, clr, GG::FORMAT_RIGHT);
        m_turns_remaining_until_next_complete_text->ClipText(true);

        if (m_icon) AttachChild(m_icon);
        AttachChild(m_name_text);
        AttachChild(m_location_text);
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
ProductionWnd::ProductionWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE | GG::ONTOP),
    m_production_info_panel(0),
    m_queue_lb(0),
    m_build_designator_wnd(0)
{
    m_production_info_panel = new ProductionInfoPanel(PRODUCTION_INFO_AND_QUEUE_WIDTH, GG::Y(200), UserString("PRODUCTION_INFO_PANEL_TITLE"), UserString("PRODUCTION_INFO_PP"),
                                                      OUTER_LINE_THICKNESS, ClientUI::KnownTechFillColor(), ClientUI::KnownTechTextAndBorderColor());

    m_queue_lb = new QueueListBox(GG::X(2), m_production_info_panel->LowerRight().y, m_production_info_panel->Width() - 4, ClientSize().y - 4 - m_production_info_panel->Height(), "PRODUCTION_QUEUE_ROW");
    m_queue_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL | GG::LIST_USERDELETE);

    GG::Pt buid_designator_wnd_size = ClientSize() - GG::Pt(m_production_info_panel->Width(), GG::Y(6));
    m_build_designator_wnd = new BuildDesignatorWnd(buid_designator_wnd_size.x, buid_designator_wnd_size.y);
    m_build_designator_wnd->MoveTo(GG::Pt(m_production_info_panel->Width(), GG::Y0));

    EnableChildClipping(true);

    GG::Connect(m_build_designator_wnd->AddNamedBuildToQueueSignal,     &ProductionWnd::AddBuildToQueueSlot, this);
    GG::Connect(m_build_designator_wnd->AddIDedBuildToQueueSignal,      &ProductionWnd::AddBuildToQueueSlot, this);
    GG::Connect(m_build_designator_wnd->BuildQuantityChangedSignal,     &ProductionWnd::ChangeBuildQuantitySlot, this);
    GG::Connect(m_build_designator_wnd->SystemSelectedSignal,           SystemSelectedSignal);
    GG::Connect(m_queue_lb->QueueItemMoved,                             &ProductionWnd::QueueItemMoved, this);
    GG::Connect(m_queue_lb->ErasedSignal,                               &ProductionWnd::QueueItemDeletedSlot, this);
    GG::Connect(m_queue_lb->LeftClickedSignal,                          &ProductionWnd::QueueItemClickedSlot, this);
    GG::Connect(m_queue_lb->DoubleClickedSignal,                        &ProductionWnd::QueueItemDoubleClickedSlot, this);

    AttachChild(m_production_info_panel);
    AttachChild(m_queue_lb);
    AttachChild(m_build_designator_wnd);
}

ProductionWnd::~ProductionWnd()
{
    m_empire_connection.disconnect();
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
    glVertex(ul.x, ul.y);
    glVertex(lr.x, ul.y);
    glVertex(lr.x, clip_rect.ul.y);
    glVertex(ul.x, clip_rect.ul.y);

    glVertex(ul.x, clip_rect.ul.y);
    glVertex(clip_rect.ul.x, clip_rect.ul.y);
    glVertex(clip_rect.ul.x, clip_rect.lr.y);
    glVertex(ul.x, clip_rect.lr.y);

    glVertex(clip_rect.lr.x, clip_rect.ul.y);
    glVertex(lr.x, clip_rect.ul.y);
    glVertex(lr.x, clip_rect.lr.y);
    glVertex(clip_rect.lr.x, clip_rect.lr.y);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex(ul.x, clip_rect.lr.y);
    glVertex(lr.x, clip_rect.lr.y);
    glVertex(lr.x, lr.y);
    glVertex(ul.x, lr.y);
    glVertex(ul.x, clip_rect.lr.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);
}

void ProductionWnd::Refresh()
{
    // useful at start of turn or when loading empire from save.
    // since empire object is recreated based on turn update from server, 
    // connections of signals emitted from the empire must be remade
    m_empire_connection.disconnect();
    EmpireManager& manager = HumanClientApp::GetApp()->Empires();
    if (Empire* empire = manager.Lookup(HumanClientApp::GetApp()->EmpireID()))
        m_empire_connection = GG::Connect(empire->GetProductionQueue().ProductionQueueChangedSignal,
                                          &ProductionWnd::ProductionQueueChangedSlot, this);
    Update();
}

void ProductionWnd::Reset()
{
    //std::cout << "ProductionWnd::Reset()" << std::endl;
    UpdateInfoPanel();
    UpdateQueue();
    m_queue_lb->BringRowIntoView(m_queue_lb->begin());
    m_build_designator_wnd->Reset();
}

void ProductionWnd::Update()
{
    //std::cout << "ProductionWnd::Update()" << this << std::endl;
    UpdateInfoPanel();
    UpdateQueue();

    m_build_designator_wnd->Update();
}

void ProductionWnd::CenterOnBuild(int queue_idx)
{
    m_build_designator_wnd->CenterOnBuild(queue_idx);
}

void ProductionWnd::SelectPlanet(int planet_id)
{
    //std::cout << "ProductionWnd::SelectPlanet(" << planet_id << ")" << std::endl;
    m_build_designator_wnd->SelectPlanet(planet_id);
}

void ProductionWnd::SelectDefaultPlanet()
{
    m_build_designator_wnd->SelectDefaultPlanet();
}

void ProductionWnd::SelectSystem(int system_id)
{
    m_build_designator_wnd->SelectSystem(system_id);
}

void ProductionWnd::QueueItemMoved(GG::ListBox::Row* row, std::size_t position)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(
        OrderPtr(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(),
                                          boost::polymorphic_downcast<QueueRow*>(row)->queue_index,
                                          position)));
}

void ProductionWnd::Sanitize()
{
    m_build_designator_wnd->Clear();
}

void ProductionWnd::ProductionQueueChangedSlot()
{
    UpdateInfoPanel();
    UpdateQueue();
}

void ProductionWnd::UpdateQueue()
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ProductionQueue& queue = empire->GetProductionQueue();
    std::size_t first_visible_queue_row = std::distance(m_queue_lb->begin(), m_queue_lb->FirstRowShown());
    m_queue_lb->Clear();
    const GG::X QUEUE_WIDTH = m_queue_lb->Width() - 8 - 14;

    int i = 0;
    for (ProductionQueue::const_iterator it = queue.begin(); it != queue.end(); ++it, ++i)
        m_queue_lb->Insert(new QueueRow(QUEUE_WIDTH, *it, i));

    if (!m_queue_lb->Empty())
        m_queue_lb->BringRowIntoView(--m_queue_lb->end());
    if (first_visible_queue_row < m_queue_lb->NumRows())
        m_queue_lb->BringRowIntoView(boost::next(m_queue_lb->begin(), first_visible_queue_row));
}

void ProductionWnd::UpdateInfoPanel()
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    const ProductionQueue& queue = empire->GetProductionQueue();
    double PPs = empire->ProductionPoints();
    double total_queue_cost = queue.TotalPPsSpent();
    ProductionQueue::const_iterator underfunded_it = queue.UnderfundedProject(empire);
    double PPs_to_underfunded_projects = underfunded_it == queue.end() ? 0.0 : underfunded_it->allocated_pp;
    m_production_info_panel->Reset(PPs, total_queue_cost, queue.ProjectsInProgress(), PPs_to_underfunded_projects, queue.size());
    /* Altering production queue may have freed up or required more PP, which may require extra
       or free up excess minerals.  Signalling that the MineralResPool has changed causes the
       MapWnd to be signalled that that pool has changed, which causes the resource indicator
       to be updated (which polls the ProductionQueue to determine how many PPs are being spent) */
    empire->GetResourcePool(RE_MINERALS)->ChangedSignal();
}

void ProductionWnd::AddBuildToQueueSlot(BuildType build_type, const std::string& name, int number, int location)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), build_type, name, number, location)));
    m_build_designator_wnd->CenterOnBuild(m_queue_lb->NumRows() - 1);
}

void ProductionWnd::AddBuildToQueueSlot(BuildType build_type, int design_id, int number, int location)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), build_type, design_id, number, location)));
    m_build_designator_wnd->CenterOnBuild(m_queue_lb->NumRows() - 1);
}

void ProductionWnd::ChangeBuildQuantitySlot(int queue_idx, int quantity)
{
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), queue_idx, quantity, true)));
}

void ProductionWnd::QueueItemDeletedSlot(GG::ListBox::iterator it)
{
    //std::cout << "ProductionWnd::QueueItemDeletedSlot" << std::endl;
    HumanClientApp::GetApp()->Orders().IssueOrder(
        OrderPtr(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(),
                                          std::distance(m_queue_lb->begin(), it))));
}

void ProductionWnd::QueueItemClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt)
{
    //std::cout << "ProductionWnd::QueueItemClickedSlot" << std::endl;
    m_build_designator_wnd->CenterOnBuild(std::distance(m_queue_lb->begin(), it));
}

void ProductionWnd::QueueItemDoubleClickedSlot(GG::ListBox::iterator it)
{
    //std::cout << "ProductionWnd::QueueItemDoubleClickedSlot" << std::endl;
    m_queue_lb->ErasedSignal(it);
}
