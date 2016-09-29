#include "ResearchWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "QueueListBox.h"
#include "TechTreeWnd.h"
#include "../Empire/Empire.h"
#include "../universe/Tech.h"
#include "../util/i18n.h"
#include "../util/Order.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include "../client/human/HumanClientApp.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

#include <boost/cast.hpp>

#include <cmath>

namespace {
    const float OUTER_LINE_THICKNESS = 2.0f;

    void AddOptions(OptionsDB& db) {
        // queue width used also on production screen. prevent double-adding...
        if (!db.OptionExists("UI.queue-width"))
            db.Add("UI.queue-width",    UserStringNop("OPTIONS_DB_UI_QUEUE_WIDTH"), 300,    RangedValidator<int>(200, 500));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    //////////////////////////////////////////////////
    // QueueTechPanel
    //////////////////////////////////////////////////
    class QueueTechPanel : public GG::Control {
    public:
        QueueTechPanel(GG::X w, const std::string& tech_name, double allocated_rp,
                       int turns_left, double turns_completed, int empire_id, bool paused = false);
        virtual void Render();

    private:
        void Draw(GG::Clr clr, bool fill);

        const std::string&      m_tech_name;
        GG::Label*              m_name_text;
        GG::Label*              m_RPs_and_turns_text;
        GG::Label*              m_turns_remaining_text;
        GG::StaticGraphic*      m_icon;
        MultiTurnProgressBar*   m_progress_bar;
        bool                    m_in_progress;
        int                     m_total_turns;
        int                     m_empire_id;
        bool                    m_paused;
    };

    //////////////////////////////////////////////////
    // QueueRow
    //////////////////////////////////////////////////
    struct QueueRow : GG::ListBox::Row {
        QueueRow(GG::X w, const ResearchQueue::Element& queue_element) :
            GG::ListBox::Row(),
            panel(0),
            elem(queue_element)
        {
            const Empire* empire = GetEmpire(queue_element.empire_id);

            const Tech* tech = GetTech(elem.name);
            double per_turn_cost = tech ? tech->PerTurnCost(elem.empire_id) : 1;
            double progress = 0.0;
            if (empire && empire->TechResearched(elem.name))
                progress = tech ? tech->ResearchCost(elem.empire_id) : 0.0;
            else if (empire)
                progress = empire->ResearchProgress(elem.name);

            panel = new QueueTechPanel(w, elem.name, elem.allocated_rp,
                                       elem.turns_left, progress / per_turn_cost,
                                       elem.empire_id, elem.paused);
            Resize(panel->Size());
            push_back(panel);

            SetDragDropDataType("RESEARCH_QUEUE_ROW");

            SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
            SetBrowseInfoWnd(TechPanelRowBrowseWnd(elem.name, elem.empire_id));
        }

        QueueTechPanel*                 panel;
        const ResearchQueue::Element    elem;
    };

    //////////////////////////////////////////////////
    // QueueTechPanel implementation
    //////////////////////////////////////////////////
    QueueTechPanel::QueueTechPanel(GG::X w, const std::string& tech_name, double turn_spending,
                                   int turns_left, double turns_completed, int empire_id, bool paused) :
        GG::Control(GG::X0, GG::Y0, w, GG::Y(10), GG::NO_WND_FLAGS),
        m_tech_name(tech_name),
        m_in_progress(turn_spending),
        m_total_turns(1),
        m_empire_id(empire_id),
        m_paused(paused)
    {
        const int MARGIN = 2;

        const int FONT_PTS = ClientUI::Pts();
        const GG::Y METER_HEIGHT(FONT_PTS);

        const GG::Y HEIGHT = MARGIN + FONT_PTS + MARGIN + METER_HEIGHT + MARGIN + FONT_PTS + MARGIN + 6;

        const int GRAPHIC_SIZE = Value(HEIGHT - 9);    // 9 pixels accounts for border thickness so the sharp-cornered icon doesn't with the rounded panel corner

        const GG::X NAME_WIDTH = w - GRAPHIC_SIZE - 2*MARGIN - 3;
        const GG::X METER_WIDTH = w - GRAPHIC_SIZE - 4*MARGIN - 3;
        const GG::X TURNS_AND_COST_WIDTH = NAME_WIDTH/2 - MARGIN;

        const Tech* tech = GetTech(m_tech_name);
        if (tech)
            m_total_turns = tech->ResearchTime(m_empire_id);

        Resize(GG::Pt(w, HEIGHT));

        GG::Clr clr = m_in_progress
                        ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor())
                        : ClientUI::ResearchableTechTextAndBorderColor();

        GG::Y top(MARGIN);
        GG::X left(MARGIN);


        m_icon = new GG::StaticGraphic(ClientUI::TechIcon(m_tech_name), GG::GRAPHIC_FITGRAPHIC);
        m_icon->MoveTo(GG::Pt(left, top));
        m_icon->Resize(GG::Pt(GG::X(GRAPHIC_SIZE), GG::Y(GRAPHIC_SIZE)));
        m_icon->SetColor(tech ? ClientUI::CategoryColor(tech->Category()) : GG::Clr());
        left += m_icon->Width() + MARGIN;

        m_name_text = new CUILabel(m_paused ? UserString("PAUSED") : UserString(m_tech_name), GG::FORMAT_TOP | GG::FORMAT_LEFT);
        m_name_text->MoveTo(GG::Pt(left, top));
        m_name_text->Resize(GG::Pt(NAME_WIDTH, GG::Y(FONT_PTS + 2*MARGIN)));
        m_name_text->SetTextColor(clr);
        m_name_text->ClipText(true);
        top += m_name_text->Height();

        m_progress_bar = new MultiTurnProgressBar(tech ? tech->ResearchTime(m_empire_id) : 1,
                                                  turns_completed, tech ? tech->ResearchCost(m_empire_id) : 0.0, turn_spending,
                                                  GG::LightColor(ClientUI::TechWndProgressBarBackgroundColor()),
                                                  ClientUI::TechWndProgressBarColor(),
                                                  m_in_progress ? ClientUI::ResearchableTechFillColor() : GG::LightColor(ClientUI::ResearchableTechFillColor()) );
        m_progress_bar->MoveTo(GG::Pt(left, top));
        m_progress_bar->Resize(GG::Pt(METER_WIDTH, METER_HEIGHT));
        top += m_progress_bar->Height() + MARGIN;

        using boost::io::str;

        double max_spending_per_turn = tech->ResearchCost(m_empire_id) / m_total_turns;
        std::string turns_cost_text = str(FlexibleFormat(UserString("TECH_TURN_COST_STR"))
            % DoubleToString(turn_spending, 3, false)
            % DoubleToString(max_spending_per_turn, 3, false));
        m_RPs_and_turns_text = new CUILabel(turns_cost_text, GG::FORMAT_LEFT);
        m_RPs_and_turns_text->MoveTo(GG::Pt(left, top));
        m_RPs_and_turns_text->Resize(GG::Pt(TURNS_AND_COST_WIDTH, GG::Y(FONT_PTS + MARGIN)));
        m_RPs_and_turns_text->SetTextColor(clr);

        left += TURNS_AND_COST_WIDTH;

        std::string turns_left_text = turns_left < 0 ? UserString("TECH_TURNS_LEFT_NEVER")
                                                     : str(FlexibleFormat(UserString("TECH_TURNS_LEFT_STR")) % turns_left);
        m_turns_remaining_text = new CUILabel(turns_left_text, GG::FORMAT_RIGHT);
        m_turns_remaining_text->MoveTo(GG::Pt(left, top));
        m_turns_remaining_text->Resize(GG::Pt(TURNS_AND_COST_WIDTH, GG::Y(FONT_PTS + MARGIN)));
        m_turns_remaining_text->SetTextColor(clr);
        m_turns_remaining_text->ClipText(true);

        AttachChild(m_name_text);
        AttachChild(m_RPs_and_turns_text);
        AttachChild(m_turns_remaining_text);
        AttachChild(m_icon);
        AttachChild(m_progress_bar);
    }

    void QueueTechPanel::Render() {
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

    void QueueTechPanel::Draw(GG::Clr clr, bool fill) {
        const int CORNER_RADIUS = 7;
        glColor(clr);
        PartlyRoundedRect(UpperLeft(), LowerRight(), CORNER_RADIUS, true, false, true, false, fill);
    }
}

//////////////////////////////////////////////////
// ResearchQueueListBox                         //
//////////////////////////////////////////////////
class ResearchQueueListBox : public QueueListBox {
public:
    ResearchQueueListBox(const std::string& drop_type_str, const std::string& prompt_str) :
        QueueListBox(drop_type_str, prompt_str)
    {}

    boost::signals2::signal<void ()>                            ShowPediaSignal;
    boost::signals2::signal<void (GG::ListBox::iterator, bool)> QueueItemPausedSignal;

protected:
    void ItemRightClickedImpl(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
        // mostly duplicated equivalent in QueueListBox, but with an extra command...

        GG::MenuItem menu_contents;
        menu_contents.next_level.push_back(GG::MenuItem(UserString("MOVE_UP_QUEUE_ITEM"),   1, false, false));
        menu_contents.next_level.push_back(GG::MenuItem(UserString("MOVE_DOWN_QUEUE_ITEM"), 2, false, false));
        menu_contents.next_level.push_back(GG::MenuItem(UserString("DELETE_QUEUE_ITEM"),    3, false, false));

        GG::ListBox::Row* row = *it;
        QueueRow* queue_row = row ? dynamic_cast<QueueRow*>(row) : 0;
        if (!queue_row)
            return;

        // pause / resume commands
        if (queue_row->elem.paused) {
            menu_contents.next_level.push_back(GG::MenuItem(UserString("RESUME"),           7, false, false));
        } else {
            menu_contents.next_level.push_back(GG::MenuItem(UserString("PAUSE"),            8, false, false));
        }

        // pedia lookup
        if (queue_row->elem.name.empty()) {
            ErrorLogger() << "Empty tech name referenced during right click";
            return;
        }

        std::string tech_name;
        if (UserStringExists(queue_row->elem.name))
            tech_name = UserString(queue_row->elem.name);

        std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % tech_name);
        menu_contents.next_level.push_back(GG::MenuItem(popup_label, 4, false, false));

        GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                            ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());

        if (popup.Run()) {
            switch (popup.MenuID()) {
            case 1: { // move item to top
                if (GG::ListBox::Row* row = *it)
                    this->QueueItemMovedSignal(row, 0);
                break;
            }
            case 2: { // move item to bottom
                if (GG::ListBox::Row* row = *it)
                    this->QueueItemMovedSignal(row, NumRows());
                break;
            }
            case 3: { // delete item
                this->QueueItemDeletedSignal(it);
                break;
            }
            case 4: { // pedia lookup
                this->LeftClickedSignal(it, pt, modkeys);
                ShowPediaSignal();
                break;
            }


            case 7: { // resume
                this->QueueItemPausedSignal(it, false);
                break;
            }
            case 8: { // pause
                this->QueueItemPausedSignal(it, true);
                break;
            }

            default:
                break;
            }
        }
    }
};

//////////////////////////////////////////////////
// ResearchQueueWnd                             //
//////////////////////////////////////////////////
class ResearchQueueWnd : public CUIWnd {
public:
    /** \name Structors */ //@{
    ResearchQueueWnd(GG::X x, GG::Y y, GG::X w, GG::Y h) :
        CUIWnd("", x, y, w, h, GG::INTERACTIVE | GG::RESIZABLE | GG::DRAGABLE | GG::ONTOP | PINABLE,
               "research.ResearchQueueWnd"),
        m_queue_lb(0)
    {
        Init(HumanClientApp::GetApp()->EmpireID());
    }
    //@}

    /** \name Mutators */ //@{
    virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        GG::Pt sz = Size();
        CUIWnd::SizeMove(ul, lr);
        if (Size() != sz)
            DoLayout();
    }

    ResearchQueueListBox*   GetQueueListBox() { return m_queue_lb; }

    void                SetEmpire(int id) {
        if (const Empire* empire = GetEmpire(id))
            SetName(boost::io::str(FlexibleFormat(UserString("RESEARCH_QUEUE_EMPIRE")) % empire->Name()));
        else
            SetName("");
    }
    //@}

private:
    void DoLayout() {
        m_queue_lb->SizeMove(GG::Pt(GG::X0, GG::Y0),
                                GG::Pt(ClientWidth(), ClientHeight() - GG::Y(CUIWnd::INNER_BORDER_ANGLE_OFFSET)));
    }

    void Init(int empire_id) {
        m_queue_lb = new ResearchQueueListBox("RESEARCH_QUEUE_ROW", UserString("RESEARCH_QUEUE_PROMPT"));
        m_queue_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL | GG::LIST_USERDELETE);
        m_queue_lb->SetName("ResearchQueue ListBox");

        SetEmpire(empire_id);

        AttachChild(m_queue_lb);
        DoLayout();
    }

    ResearchQueueListBox*   m_queue_lb;
};


//////////////////////////////////////////////////
// ResearchWnd                                  //
//////////////////////////////////////////////////
ResearchWnd::ResearchWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE | GG::ONTOP),
    m_research_info_panel(0),
    m_queue_wnd(0),
    m_tech_tree_wnd(0),
    m_enabled(false),
    m_empire_shown_id(ALL_EMPIRES)
{
    GG::X queue_width(GetOptionsDB().Get<int>("UI.queue-width"));
    GG::Pt tech_tree_wnd_size = ClientSize() - GG::Pt(GG::X(GetOptionsDB().Get<int>("UI.queue-width")), GG::Y0);

    m_research_info_panel = new ProductionInfoPanel(UserString("RESEARCH_WND_TITLE"), UserString("RESEARCH_INFO_RP"),
                                                    GG::X0, GG::Y0, GG::X(queue_width), GG::Y(100), "research.InfoPanel");
    m_queue_wnd = new ResearchQueueWnd(GG::X0, GG::Y(100), queue_width, GG::Y(ClientSize().y - 100));
    m_tech_tree_wnd = new TechTreeWnd(tech_tree_wnd_size.x, tech_tree_wnd_size.y);

    GG::Connect(m_queue_wnd->GetQueueListBox()->QueueItemMovedSignal,   &ResearchWnd::QueueItemMoved,               this);
    GG::Connect(m_queue_wnd->GetQueueListBox()->QueueItemDeletedSignal, &ResearchWnd::DeleteQueueItem,              this);
    GG::Connect(m_queue_wnd->GetQueueListBox()->LeftClickedSignal,      &ResearchWnd::QueueItemClickedSlot,         this);
    GG::Connect(m_queue_wnd->GetQueueListBox()->DoubleClickedSignal,    &ResearchWnd::QueueItemDoubleClickedSlot,   this);
    GG::Connect(m_queue_wnd->GetQueueListBox()->ShowPediaSignal,        &ResearchWnd::ShowPedia,                    this);
    GG::Connect(m_queue_wnd->GetQueueListBox()->QueueItemPausedSignal,  &ResearchWnd::QueueItemPaused,              this);
    GG::Connect(m_tech_tree_wnd->AddTechsToQueueSignal,                 &ResearchWnd::AddTechsToQueueSlot,          this);

    AttachChild(m_research_info_panel);
    AttachChild(m_queue_wnd);
    AttachChild(m_tech_tree_wnd);

    SetChildClippingMode(ClipToClient);

    DoLayout();
}

ResearchWnd::~ResearchWnd()
{ m_empire_connection.disconnect(); }

void ResearchWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void ResearchWnd::DoLayout() {
    m_research_info_panel->MoveTo(GG::Pt(GG::X0, GG::Y0));
    m_research_info_panel->Resize(GG::Pt(GG::X(GetOptionsDB().Get<int>("UI.queue-width")),
                                         m_research_info_panel->MinUsableSize().y));
    GG::Pt queue_ul = GG::Pt(GG::X(2), m_research_info_panel->Height());
    GG::Pt queue_size = GG::Pt(m_research_info_panel->Width() - 4,
                               ClientSize().y - 4 - m_research_info_panel->Height());
    m_queue_wnd->SizeMove(queue_ul, queue_ul + queue_size);

    GG::Pt tech_tree_wnd_size = ClientSize() - GG::Pt(m_research_info_panel->Width(), GG::Y0);
    GG::Pt tech_tree_wnd_ul = GG::Pt(m_research_info_panel->Width(), GG::Y0);
    m_tech_tree_wnd->SizeMove(tech_tree_wnd_ul, tech_tree_wnd_ul + tech_tree_wnd_size);
}

void ResearchWnd::Refresh() {
    // useful at start of turn or when loading empire from save.
    // since empire object is recreated based on turn update from server, 
    // connections of signals emitted from the empire must be remade
    m_empire_connection.disconnect();

    if (Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID()))
        m_empire_connection = GG::Connect(empire->GetResearchQueue().ResearchQueueChangedSignal,
                                          &ResearchWnd::ResearchQueueChangedSlot, this);
    Update();
}

void ResearchWnd::Reset() {
    m_tech_tree_wnd->Reset();
    UpdateQueue();
    UpdateInfoPanel();
    m_queue_wnd->GetQueueListBox()->BringRowIntoView(m_queue_wnd->GetQueueListBox()->begin());
}

void ResearchWnd::Update() {
    m_tech_tree_wnd->Update();
    UpdateQueue();
    UpdateInfoPanel();
}

void ResearchWnd::CenterOnTech(const std::string& tech_name)
{ m_tech_tree_wnd->CenterOnTech(tech_name); }

void ResearchWnd::ShowTech(const std::string& tech_name) {
    m_tech_tree_wnd->CenterOnTech(tech_name);
    m_tech_tree_wnd->SetEncyclopediaTech(tech_name);
    m_tech_tree_wnd->SelectTech(tech_name);
}

void ResearchWnd::ShowPedia()
{ m_tech_tree_wnd->ShowPedia(); }

void ResearchWnd::HidePedia()
{ m_tech_tree_wnd->HidePedia(); }

void ResearchWnd::TogglePedia()
{ m_tech_tree_wnd->TogglePedia(); }

bool ResearchWnd::PediaVisible()
{ return m_tech_tree_wnd->PediaVisible(); }

void ResearchWnd::QueueItemMoved(GG::ListBox::Row* row, std::size_t position) {
    if (QueueRow* queue_row = boost::polymorphic_downcast<QueueRow*>(row)) {
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        HumanClientApp::GetApp()->Orders().IssueOrder(
            OrderPtr(new ResearchQueueOrder(empire_id, queue_row->elem.name, static_cast<int>(position))));
        if (Empire* empire = GetEmpire(empire_id))
            empire->UpdateResearchQueue();
    }
}

void ResearchWnd::Sanitize()
{ m_tech_tree_wnd->Clear(); }

void ResearchWnd::Render()
{}

void ResearchWnd::SetEmpireShown(int empire_id) {
    if (empire_id != m_empire_shown_id) {
        m_empire_shown_id = empire_id;
        Refresh();
    }
}

void ResearchWnd::ResearchQueueChangedSlot() {
    UpdateQueue();
    UpdateInfoPanel();
    m_tech_tree_wnd->Update();
}

void ResearchWnd::UpdateQueue() {
    DebugLogger() << "ResearchWnd::UpdateQueue()";
    ScopedTimer timer("ResearchWnd::UpdateQueue");

    m_queue_wnd->SetEmpire(m_empire_shown_id);

    QueueListBox* queue_lb = m_queue_wnd->GetQueueListBox();
    std::size_t first_visible_queue_row = std::distance(queue_lb->begin(), queue_lb->FirstRowShown());
    queue_lb->Clear();

    const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return;

    const ResearchQueue& queue = empire->GetResearchQueue();


    for (ResearchQueue::const_iterator it = queue.begin(); it != queue.end(); ++it) {
        const ResearchQueue::Element& elem = *it;
        QueueRow* row = new QueueRow(queue_lb->RowWidth(), elem);
        queue_lb->Insert(row);
    }

    if (!queue_lb->Empty())
        queue_lb->BringRowIntoView(--queue_lb->end());
    if (first_visible_queue_row < queue_lb->NumRows())
        queue_lb->BringRowIntoView(boost::next(queue_lb->begin(), first_visible_queue_row));
}

void ResearchWnd::UpdateInfoPanel() {
    const Empire* empire = GetEmpire(m_empire_shown_id);
    if (!empire) {
        m_research_info_panel->SetName(UserString("RESEARCH_WND_TITLE"));
        m_research_info_panel->ClearLocalInfo();
        return;
    } else {
        m_research_info_panel->SetEmpireID(m_empire_shown_id);
    }

    const ResearchQueue& queue = empire->GetResearchQueue();
    float RPs = empire->ResourceOutput(RE_RESEARCH);
    float total_queue_cost = queue.TotalRPsSpent();
    m_research_info_panel->SetTotalPointsCost(RPs, total_queue_cost);

    /* Altering research queue may have freed up or required more RP.  Signalling that the
       ResearchResPool has changed causes the MapWnd to be signalled that that pool has changed,
       which causes the resource indicator to be updated (which polls the ResearchQueue to
       determine how many RPs are being spent).  If/when RP are stockpilable, this might matter,
       so then the following line should be uncommented.*/
    //empire->GetResearchResPool().ChangedSignal();
}

void ResearchWnd::AddTechsToQueueSlot(const std::vector<std::string>& tech_vec, int pos) {
    if (!m_enabled)
        return;
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    Empire* empire = GetEmpire(empire_id);
    if (!empire)
        return;
    const ResearchQueue& queue = empire->GetResearchQueue();
    OrderSet& orders = HumanClientApp::GetApp()->Orders();
    for (std::vector<std::string>::const_iterator it = tech_vec.begin(); it != tech_vec.end(); ++it) {
        const std::string& tech_name = *it;
        if (empire->TechResearched(tech_name))
            continue;
        // AddTechsToQueueSlot is currently used for (i) adding a tech and any not-yet-queued prereqs to the
        // end of the queue (but any already-queued prereqs are NOT to be moved to the end of the queue), or 
        // (ii) prioritizing a tech by placing it and any not-yet-completed techs, whether currently queued or not,
        // to the front of the queue.  If at some time this routine is desired to be used to move a group of techs from 
        // early positions in the queue to later positions, the below tests would need to change.
        
        // If we're adding to the end of the queue (pos==-1), we'll need to put in a ResearchQueueOrder iff the tech is 
        // not yet in the queue. Otherwise (adding to beginning) we'll need to put in a ResearchQueueOrder if the tech is
        // not yet in the queue or if the tech's current position in the queue is after the desired position.  When 
        // adding/moving a group of techs to the queue beginning, we increment our insertion point for every tech we add, 
        // or that we skipped because it happened to already be in the right spot.
        if (pos == -1) {
            if (!queue.InQueue(tech_name)) {
                orders.IssueOrder(OrderPtr(new ResearchQueueOrder(empire_id, tech_name, pos)));
            }
        } else if (!queue.InQueue(tech_name) || ((queue.find(tech_name) - queue.begin()) > pos)) {
            orders.IssueOrder(OrderPtr(new ResearchQueueOrder(empire_id, tech_name, pos)));
            pos += 1;
        } else {
            if ((queue.find(tech_name) - queue.begin()) == pos)
                pos += 1;
        }
    }
    empire->UpdateResearchQueue();
}

void ResearchWnd::DeleteQueueItem(GG::ListBox::iterator it) {
    if (!m_enabled)
        return;
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    OrderSet& orders = HumanClientApp::GetApp()->Orders();
    if (QueueRow* queue_row = boost::polymorphic_downcast<QueueRow*>(*it))
        orders.IssueOrder(OrderPtr(new ResearchQueueOrder(empire_id, queue_row->elem.name)));
    if (Empire* empire = GetEmpire(empire_id))
        empire->UpdateResearchQueue();
}

void ResearchWnd::QueueItemClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    if (!m_queue_wnd->GetQueueListBox()->DisplayingValidQueueItems())
        return;

    if (QueueRow* queue_row = boost::polymorphic_downcast<QueueRow*>(*it))
        ShowTech(queue_row->elem.name);
}

void ResearchWnd::QueueItemDoubleClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    if (m_queue_wnd->GetQueueListBox()->DisplayingValidQueueItems())
        DeleteQueueItem(it);
}

void ResearchWnd::QueueItemPaused(GG::ListBox::iterator it, bool pause) {
    if (!m_enabled)
        return;
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    Empire* empire = GetEmpire(client_empire_id);
    if (!empire)
        return;

    // todo: reject action if shown queue is not this client's empire's queue

    if (QueueRow* queue_row = boost::polymorphic_downcast<QueueRow*>(*it))
        HumanClientApp::GetApp()->Orders().IssueOrder(
            OrderPtr(new ResearchQueueOrder(client_empire_id, queue_row->elem.name, pause, -1.0f)));

    empire->UpdateResearchQueue();
}


void ResearchWnd::EnableOrderIssuing(bool enable/* = true*/) {
    m_enabled = enable;
    m_queue_wnd->GetQueueListBox()->EnableOrderIssuing(m_enabled);
}
