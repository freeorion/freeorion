#include "CombatLogWnd.h"

#include <GG/GUI.h>
#include <GG/DeferredLayout.h>
#include <GG/Scroll.h>
#include <GG/ScrollPanel.h>

#include "../LinkText.h"

#include "../../client/human/HumanClientApp.h"
#include "../../combat/CombatLogManager.h"
#include "../../combat/CombatEvents.h"
#include "../../universe/System.h"
#include "../../util/AppInterface.h"
#include "../../util/i18n.h"
#include "../../util/Logger.h"
#include "../../util/VarText.h"
#include "../../universe/UniverseObject.h"
#include "../AccordionPanel.h"
#include "../../Empire/Empire.h"

class CombatLogWnd::CombatLogWndImpl {
public:
    CombatLogWndImpl(CombatLogWnd& _log);

    /** \name Accessors */ ///@{
    GG::Pt MinUsableSize() const;
    //@}

    /** \name Mutators */ //@{
    void SetFont(boost::shared_ptr<GG::Font> font);
    /// Set which log to show
    void SetLog(int log_id);
    /** Add a row at the end of the combat report*/
    void AddRow(GG::Wnd* wnd);
    //@}

    /** When windows changes forces a re-layout */
    void HandleWndChanged();

    /** DecorateLinkText creates a CUILinkTextMultiEdit using \a text and attaches it to handlers
        \a and_flags are anded to the default flags. */
    LinkText* DecorateLinkText(std::string const& text);

    /** Fill \p new_logs with pointers to the flat log contents of \p
        event using the pre-calculated \p details.*/
    void PopulateWithFlatLogs(
        GG::X w, int viewing_empire_id, std::vector<GG::Wnd *>& new_logs,
        ConstCombatEventPtr& event, std::string& details);

    // Returns either a simple LinkText for a simple log or a CombatLogAccordionPanel for a complex log
    std::vector<GG::Wnd*> MakeCombatLogPanel(
        GG::X w, int viewing_empire_id, ConstCombatEventPtr event);

    // public interface
    CombatLogWnd& m_wnd;

    // default flags for a text link log segment
    GG::Flags<GG::TextFormat> m_text_format_flags;
    boost::shared_ptr<GG::Font> m_font;

};

namespace {
    typedef boost::shared_ptr<LinkText> LinkTextPtr;

    const std::string EMPTY_STRING;

    std::map<int, int> CountByOwner(const std::set<int>& owners, const std::set<int>& objects) {
        std::map<int, int> objects_per_owner;
        for (int owner_id : owners)
            objects_per_owner[owner_id] = 0;
        for (int obj_id : objects) {
            TemporaryPtr<const UniverseObject> object = Objects().Object(obj_id);
            if (object && (
                    object->ObjectType() == OBJ_SHIP || (
                        object->GetMeter(METER_POPULATION) &&
                        object->CurrentMeterValue(METER_POPULATION) > 0.0)))
            {
                int owner_id = object->Owner();
                if (objects_per_owner.find(owner_id) == objects_per_owner.end())
                    objects_per_owner[owner_id] = 0;
                ++objects_per_owner[owner_id];
            }
        }
        return objects_per_owner;
    }

    std::string CountsToText(const std::map<int, int>& count_per_empire, const std::string& delimiter = ", ") {
        std::stringstream ss;
        for (std::map<int,int>::const_iterator it = count_per_empire.begin(); it != count_per_empire.end(); ) {
            std::string owner_string = UserString("NEUTRAL");
            if (const Empire* owner = GetEmpire(it->first))
                owner_string = GG::RgbaTag(owner->Color()) + "<" + VarText::EMPIRE_ID_TAG + " "
                    + boost::lexical_cast<std::string>(owner->EmpireID()) + ">" + owner->Name()
                    + "</" + VarText::EMPIRE_ID_TAG + ">" + "</rgba>";
            ss << owner_string << ": " << it->second;
            ++it;
            if (it != count_per_empire.end())
                ss << delimiter;
        }
        return ss.str();
    }

    /// A Single section in the CombatLog showing general outline of a ship's combat
    /// and expanding into specifics.
    class CombatLogAccordionPanel : public AccordionPanel {
        public:
        CombatLogAccordionPanel(GG::X w, CombatLogWnd::CombatLogWndImpl &log_,
                                int viewing_empire_id_, ConstCombatEventPtr event_);
        ~CombatLogAccordionPanel();

        private:
        /** toggles panel expanded or collapsed */
        void ToggleExpansion();

        CombatLogWnd::CombatLogWndImpl & log;
        int viewing_empire_id;
        ConstCombatEventPtr event;
        LinkText* title;
        std::vector<GG::Wnd *> details;

        // distance between expansion symbol and text
        static const unsigned int BORDER_MARGIN = 5;

    };

    CombatLogAccordionPanel::CombatLogAccordionPanel(GG::X w, CombatLogWnd::CombatLogWndImpl& log_,
                                                     int viewing_empire_id_, ConstCombatEventPtr event_) :
        AccordionPanel(w, GG::Y(ClientUI::Pts()), true),
        log(log_),
        viewing_empire_id(viewing_empire_id_),
        event(event_),
        title(log.DecorateLinkText(event->CombatLogDescription(viewing_empire_id))),
        details()
    {
        AccordionPanel::SetInteriorColor(ClientUI::CtrlColor());

        GG::Connect(m_expand_button->LeftClickedSignal, &CombatLogAccordionPanel::ToggleExpansion, this);
        GG::Connect(this->ExpandCollapseSignal, &CombatLogWnd::CombatLogWndImpl::HandleWndChanged, &log);

        SetBorderMargin(BORDER_MARGIN);

        SetLayout(new GG::DeferredLayout(UpperLeft().x, UpperLeft().y, Width(), Height(), 1, 1));
        GetLayout()->Add(title, 0, 0, 1, 1);
        SetCollapsed(true);
        RequirePreRender();
    }

    CombatLogAccordionPanel::~CombatLogAccordionPanel() {
        if (!IsCollapsed() && !details.empty()) {
            for (GG::Wnd* wnd : details) {
                delete wnd;
            }
        }
    }

    void CombatLogAccordionPanel::ToggleExpansion() {
        DebugLogger() << "Expand/Collapse of detailed combat log.";
        bool new_collapsed = !IsCollapsed();
        if (new_collapsed) {
            for (GG::Wnd* wnd : details) {
                GetLayout()->Remove(wnd);
            }
        } else {
            if (details.empty()) {
                std::string detail_text = event->CombatLogDetails(viewing_empire_id);
                log.PopulateWithFlatLogs(Width(), viewing_empire_id, details, event, detail_text);
            }

            for (GG::Wnd* wnd : details) {
                GetLayout()->Add(wnd, GetLayout()->Rows(), 0);
            }
        }
        SetCollapsed(new_collapsed);
    }

}

CombatLogWnd::CombatLogWndImpl::CombatLogWndImpl(CombatLogWnd& _wnd) :
    m_wnd(_wnd),
    m_text_format_flags(GG::FORMAT_WORDBREAK| GG::FORMAT_LEFT | GG::FORMAT_TOP),
    m_font(ClientUI::GetFont())
{ }

GG::Pt CombatLogWnd::CombatLogWndImpl::MinUsableSize() const {
    return GG::Pt(m_font->SpaceWidth()*20, m_font->Lineskip()*10);
}

void CombatLogWnd::CombatLogWndImpl::HandleWndChanged()
{ m_wnd.RequirePreRender(); }


namespace {
    /**Find a parent of type T*/
    template <typename T>
    T const* FindParentOfType(GG::Wnd const* parent) {
        GG::Wnd const * iwnd = parent;
        T const* type_T = 0;
        while (iwnd && !(type_T = dynamic_cast<const T*>(iwnd))){
            iwnd = iwnd->Parent();
        }
        return type_T;
    }


    /**LazyScrollerLinkText is a link text that initially populates
       itself with an ellipsis.

       As a one time effect, when it comes into view it populates itself
       with the text which will be processed by DetermineLines and
       flowed by the Layout.

       This works around problems with Font::DetermineLinesImpl() which is
       too costly to run on 100 combat reports without freezing the UI
       for an extended portion of time.

       This assumes:
       + CombatLogWnd is in a TabWnd as the second tab.
       + CombatLogWnd is in a ScrollPanel
    */

    class LazyScrollerLinkText : public LinkText {
        public:

        mutable boost::signals2::signal<void ()> ChangedSignal;

        LazyScrollerLinkText(
            GG::Wnd & parent, GG::X x, GG::Y y, const std::string& str,
            const boost::shared_ptr<GG::Font>& font, GG::Clr color = GG::CLR_BLACK) :
            LinkText(x, y, UserString("ELLIPSIS"), font, color),
            m_text( new std::string(str)),
            m_signals()
        {

            // Register for signals that might bring the text into view

            if (CombatLogWnd const* log = FindParentOfType<CombatLogWnd>(&parent)) {
                m_signals.push_back(
                    GG::Connect(log->WndChangedSignal, &LazyScrollerLinkText::HandleMaybeVisible, this));
            }

            if (GG::ScrollPanel const * scroll_panel = FindParentOfType<GG::ScrollPanel>(&parent)) {
                GG::Scroll const* scroll = scroll_panel->GetScroll();
                m_signals.push_back(
                    GG::Connect(scroll->ScrolledAndStoppedSignal, &LazyScrollerLinkText::HandleScrolledAndStopped, this));

            }

            // Parent doesn't contain any of the expected parents so just
            // show the text.
            if (m_signals.empty()) {
                SetText(str);
                m_text.reset();
            } else {
                HandleMaybeVisible();
            }
        }

        void HandleMaybeVisible() {

            // Assumes the log is the second tab.
            GG::OverlayWnd const* tab = FindParentOfType<const GG::OverlayWnd>(Parent());
            if (tab && (tab->CurrentWndIndex() != 1))
                return;

            // Check if any part of text is in the scrollers visible area
            GG::ScrollPanel const* scroll_panel = FindParentOfType<GG::ScrollPanel>(Parent());
            if (scroll_panel && (scroll_panel->InClient(UpperLeft())
                                 || scroll_panel->InClient(LowerRight())
                                 || scroll_panel->InClient(GG::Pt(Right(), Top()))
                                 || scroll_panel->InClient(GG::Pt(Left(), Bottom())))) {
                for (boost::signals2::connection& signal : m_signals) {
                    signal.disconnect();
                }
                m_signals.clear();

                SetText(*m_text);
                m_text.reset();

                ChangedSignal();
            }
        }

        void HandleScrolledAndStopped(int start_pos, int end_post, int min_pos, int max_pos) {
            HandleMaybeVisible();
        }

        virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            LinkText::SizeMove(ul, lr);
            if (! m_signals.empty())
                HandleMaybeVisible();
        }

        boost::scoped_ptr<std::string> m_text;
        std::vector<boost::signals2::connection> m_signals;
    };

}

LinkText * CombatLogWnd::CombatLogWndImpl::DecorateLinkText(std::string const & text) {
    LazyScrollerLinkText * links = new LazyScrollerLinkText(m_wnd, GG::X0, GG::Y0, text, m_font, GG::CLR_WHITE);

    links->SetTextFormat(m_text_format_flags);

    links->SetDecorator(VarText::SHIP_ID_TAG, new ColorByOwner());
    links->SetDecorator(VarText::PLANET_ID_TAG, new ColorByOwner());
    links->SetDecorator(VarText::SYSTEM_ID_TAG, new ColorByOwner());
    links->SetDecorator(VarText::EMPIRE_ID_TAG, new ColorByOwner());

    links->LinkClickedSignal.connect(m_wnd.LinkClickedSignal);
    links->LinkDoubleClickedSignal.connect(m_wnd.LinkDoubleClickedSignal);
    links->LinkRightClickedSignal.connect(m_wnd.LinkRightClickedSignal);
    GG::Connect(links->ChangedSignal,           &CombatLogWnd::CombatLogWndImpl::HandleWndChanged,         this);

    return links;
}

/** Fill \p new_logs with pointers to the flat log contents of \p
    event using the pre-calculated \p details.*/
void CombatLogWnd::CombatLogWndImpl::PopulateWithFlatLogs(GG::X w, int viewing_empire_id, std::vector<GG::Wnd*> &new_logs,
                                                          ConstCombatEventPtr& event, std::string& details)
{
    if (!details.empty()) {
        new_logs.push_back(DecorateLinkText(details));
    }

    if (!event->AreSubEventsEmpty(viewing_empire_id)) {
        for (ConstCombatEventPtr sub_event : event->SubEvents(viewing_empire_id)) {
            std::vector<GG::Wnd*> flat_logs =
                MakeCombatLogPanel(w, viewing_empire_id, sub_event);
            new_logs.insert(new_logs.end(), flat_logs.begin(), flat_logs.end());
        }
    }
}


// Returns either a simple LinkText for a simple log or a CombatLogAccordionPanel for a complex log
std::vector<GG::Wnd*> CombatLogWnd::CombatLogWndImpl::MakeCombatLogPanel(GG::X w, int viewing_empire_id,
                                                                         ConstCombatEventPtr event)
{
    std::vector<GG::Wnd *> new_logs;

    // Create an accordion log if there are detail or sub events and
    // the log isn't explicitly flattened.  Otherwise, flatten the log,
    // details and sub events.

    if (!event->FlattenSubEvents() && !event->AreSubEventsEmpty(viewing_empire_id) ) {
        new_logs.push_back(new CombatLogAccordionPanel(w, *this, viewing_empire_id, event));
        return new_logs;
    }

    if (!event->FlattenSubEvents() && !event->AreDetailsEmpty(viewing_empire_id)) {
        new_logs.push_back(new CombatLogAccordionPanel(w, *this, viewing_empire_id, event));
        return new_logs;
    }

    std::string title = event->CombatLogDescription(viewing_empire_id);
    if (!(event->FlattenSubEvents() && title.empty()))
        new_logs.push_back(DecorateLinkText(title));

    std::string details = event->CombatLogDetails(viewing_empire_id);
    PopulateWithFlatLogs(w, viewing_empire_id, new_logs, event, details);

    return new_logs;
}


void CombatLogWnd::CombatLogWndImpl::AddRow(GG::Wnd* wnd) {
    if (GG::Layout* layout = m_wnd.GetLayout())
        layout->Add(wnd, layout->Rows(), 0);
}

void CombatLogWnd::CombatLogWndImpl::SetFont(boost::shared_ptr<GG::Font> font)
{ m_font = font; }

void CombatLogWnd::CombatLogWndImpl::SetLog(int log_id) {
    if (!CombatLogAvailable(log_id)) {
        ErrorLogger() << "Couldn't find combat log with id: " << log_id;
        return;
    }

    m_wnd.DeleteChildren();
    GG::Layout* layout = new GG::DeferredLayout(m_wnd.UpperLeft().x, m_wnd.UpperLeft().y
                                        , m_wnd.Width(), m_wnd.Height()
                                        , 1, 1 ///< numrows, numcols
                                        , 0, 0 ///< wnd margin, cell margin
                                       );
    m_wnd.SetLayout(layout);

    const CombatLog& log = GetCombatLog(log_id);
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    // Write Header text
    DebugLogger() << "Setting log with " << log.combat_events.size() << " events";

    TemporaryPtr<const System> system = GetSystem(log.system_id);
    const std::string& sys_name = (system ? system->PublicName(client_empire_id) : UserString("ERROR"));

    AddRow(DecorateLinkText(str(FlexibleFormat(UserString("ENC_COMBAT_LOG_DESCRIPTION_STR"))
                                % LinkTaggedIDText(VarText::SYSTEM_ID_TAG, log.system_id, sys_name)
                                % log.turn) + "\n"
                           ));
    AddRow(DecorateLinkText(UserString("COMBAT_INITIAL_FORCES")));
    AddRow(DecorateLinkText(CountsToText(CountByOwner(log.empire_ids, log.object_ids))));

    std::stringstream summary_text;
    summary_text << std::endl << UserString("COMBAT_SUMMARY_DESTROYED")
                 << std::endl << CountsToText(CountByOwner(log.empire_ids, log.destroyed_object_ids));
    AddRow(DecorateLinkText(summary_text.str()));

    // Write Logs
    for (CombatEventPtr event : log.combat_events) {
        DebugLogger() << "event debug info: " << event->DebugString();

        for (GG::Wnd* wnd : MakeCombatLogPanel(m_font->SpaceWidth()*10, client_empire_id, event)) {
            AddRow(wnd);
        }
    }

    // Add a dummy row that the layout manager can use to add space.
    AddRow(DecorateLinkText(""));
    layout->SetRowStretch(layout->Rows() - 1, 1);

    HandleWndChanged();
}


// Forward request to private implementation
CombatLogWnd::CombatLogWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
    pimpl(new CombatLogWndImpl(*this))
{
    SetName("CombatLogWnd");
}

// This virtual destructor must exist to ensure that the pimpl is destroyed.
CombatLogWnd::~CombatLogWnd()
{}

void CombatLogWnd::SetFont(boost::shared_ptr<GG::Font> font)
{ pimpl->SetFont(font); }

void CombatLogWnd::SetLog(int log_id)
{ pimpl->SetLog(log_id); }

GG::Pt CombatLogWnd::ClientUpperLeft() const
{ return UpperLeft() + GG::Pt(GG::X(MARGIN), GG::Y(MARGIN)); }

GG::Pt CombatLogWnd::ClientLowerRight() const
{ return LowerRight() - GG::Pt(GG::X(MARGIN), GG::Y(MARGIN)); }

GG::Pt CombatLogWnd::MinUsableSize() const
{ return pimpl->MinUsableSize(); }

void CombatLogWnd::HandleMadeVisible()
{ return pimpl->HandleWndChanged(); }

void CombatLogWnd::PreRender() {
    GG::Wnd::PreRender();

    /* Workaround

     Problem: CombatLogWnd is incorrectly initialized with a width of 30 which causes the combat
     accordion windows to incorrectly size themselves as if the title text were 30 wide.

    This fix forces the combat accordion window to correctly resize itself.

    TODO: Fix intial size of CombatReport from (30,15) to its actual first displayed size.*/
    GG::Pt size = Size();
    Resize(size + GG::Pt(2*pimpl->m_font->SpaceWidth(), GG::Y0));
    GG::GUI::PreRenderWindow(this);
    Resize(size);
    GG::GUI::PreRenderWindow(this);
    /* End workaround. */

    WndChangedSignal();
}
