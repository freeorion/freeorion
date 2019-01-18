#include "CombatLogWnd.h"

#include <GG/DeferredLayout.h>
#include <GG/GUI.h>
#include <GG/Scroll.h>
#include <GG/ScrollPanel.h>

#include "../LinkText.h"

#include "../../client/human/HumanClientApp.h"
#include "../../combat/CombatLogManager.h"
#include "../../universe/System.h"
#include "../../universe/UniverseObject.h"
#include "../../universe/Enums.h"
#include "../../util/AppInterface.h"
#include "../../util/i18n.h"
#include "../../util/Logger.h"
#include "../../util/VarText.h"
#include "../AccordionPanel.h"
#include "../../Empire/Empire.h"

namespace {
    DeclareThreadSafeLogger(combat_log);
}

class CombatLogWnd::Impl {
public:
    Impl(CombatLogWnd& _log);

    /** \name Accessors */ ///@{
    GG::Pt MinUsableSize() const;
    //@}

    /** \name Mutators */ //@{
    void SetFont(std::shared_ptr<GG::Font> font);
    /// Set which log to show
    void SetLog(int log_id);
    /** Add a row \p wnd at the end of the combat report. */
    void AddRow(std::shared_ptr<GG::Wnd> wnd);
    //@}

    /** When windows changes forces a re-layout */
    void HandleWndChanged();

    /** DecorateLinkText creates a CUILinkTextMultiEdit using \a text and attaches it to handlers
        \a and_flags are anded to the default flags. */
    std::shared_ptr<LinkText> DecorateLinkText(const std::string& text);

    /** Fill \p new_logs with pointers to the flat log contents of \p
        event using the pre-calculated \p details.*/
    void PopulateWithFlatLogs(
        GG::X w, int viewing_empire_id, std::vector<std::shared_ptr<GG::Wnd>>& new_logs,
        ConstCombatEventPtr& event, std::string& details);

    // Returns either a simple LinkText for a simple log or a CombatLogAccordionPanel for a complex log
    std::vector<std::shared_ptr<GG::Wnd>> MakeCombatLogPanel(
        GG::X w, int viewing_empire_id, ConstCombatEventPtr event);

    // public interface
    CombatLogWnd& m_wnd;

    // default flags for a text link log segment
    GG::Flags<GG::TextFormat> m_text_format_flags;
    std::shared_ptr<GG::Font> m_font;

};

namespace {
    typedef std::shared_ptr<LinkText> LinkTextPtr;

    const std::string EMPTY_STRING;

    // Counts how many ships or populated planets in \a objects are owned by
    // each empire. Assumes ownership and populations now are consistent with
    // whenever the result is supposed to indicate, not accounting for 
    std::map<int, int> CountByOwner(const std::set<int>& owners, const std::set<int>& objects) {
        std::map<int, int> objects_per_owner;
        for (int owner_id : owners)
            objects_per_owner[owner_id] = 0;
        for (int obj_id : objects) {
            auto object = Objects().Object(obj_id); // gets destroyed objects, so this function can be used for initial combat forces even if some are destroyed during the combat
            if (object && (
                    object->ObjectType() == OBJ_SHIP || (
                        object->GetMeter(METER_POPULATION) &&
                        object->InitialMeterValue(METER_POPULATION) > 0.0f)))
            {
                int owner_id = object->Owner();
                if (!objects_per_owner.count(owner_id))
                    objects_per_owner[owner_id] = 0;
                ++objects_per_owner[owner_id];
            }
        }
        return objects_per_owner;
    }

    // converts input empire ids and numbers of objects to text with names and
    // links to empires and numbers-as-text of how many objects they own
    std::string CountsToText(const std::map<int, int>& count_per_empire,
                             const std::string& delimiter = ", ")
    {
        std::stringstream ss;
        for (auto it = count_per_empire.begin(); it != count_per_empire.end(); ) {
            std::string owner_string;
            if (const Empire* owner = GetEmpire(it->first))
                owner_string = GG::RgbaTag(owner->Color()) + "<" + VarText::EMPIRE_ID_TAG + " "
                    + std::to_string(owner->EmpireID()) + ">" + owner->Name()
                    + "</" + VarText::EMPIRE_ID_TAG + ">" + "</rgba>";
            else
                owner_string = GG::RgbaTag(ClientUI::DefaultLinkColor()) + UserString("NEUTRAL")  + "</rgba>";
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
        CombatLogAccordionPanel(GG::X w, CombatLogWnd::Impl &log_,
                                int viewing_empire_id_, ConstCombatEventPtr event_);
        ~CombatLogAccordionPanel();
        void CompleteConstruction() override;

    private:
        /** toggles panel expanded or collapsed */
        void ToggleExpansion();

        CombatLogWnd::Impl&                     log;
        int                                     viewing_empire_id;
        ConstCombatEventPtr                     event;
        std::shared_ptr<LinkText>               title;
        std::vector<std::shared_ptr<GG::Wnd>>   details;

        // distance between expansion symbol and text
        static const unsigned int BORDER_MARGIN = 5;
    };

    CombatLogAccordionPanel::CombatLogAccordionPanel(GG::X w, CombatLogWnd::Impl& log_,
                                                     int viewing_empire_id_, ConstCombatEventPtr event_) :
        AccordionPanel(w, GG::Y(ClientUI::Pts()), true),
        log(log_),
        viewing_empire_id(viewing_empire_id_),
        event(event_),
        title(log.DecorateLinkText(event->CombatLogDescription(viewing_empire_id))),
        details()
    {}

    void CombatLogAccordionPanel::CompleteConstruction() {
        AccordionPanel::CompleteConstruction();
        AccordionPanel::SetInteriorColor(ClientUI::CtrlColor());

        m_expand_button->LeftPressedSignal.connect(
            boost::bind(&CombatLogAccordionPanel::ToggleExpansion, this));
        this->ExpandCollapseSignal.connect(
            boost::bind(&CombatLogWnd::Impl::HandleWndChanged, &log));

        SetBorderMargin(BORDER_MARGIN);

        SetLayout(GG::Wnd::Create<GG::DeferredLayout>(UpperLeft().x, UpperLeft().y, Width(), Height(), 1, 1));
        GetLayout()->Add(title, 0, 0, 1, 1);
        SetCollapsed(true);
        RequirePreRender();
    }

    CombatLogAccordionPanel::~CombatLogAccordionPanel()
    {}

    void CombatLogAccordionPanel::ToggleExpansion() {
        bool new_collapsed = !IsCollapsed();
        if (new_collapsed) {
            for (auto& wnd : details) {
                GetLayout()->Remove(wnd.get());
            }
        } else {
            if (details.empty()) {
                std::string detail_text = event->CombatLogDetails(viewing_empire_id);
                log.PopulateWithFlatLogs(Width(), viewing_empire_id, details, event, detail_text);
            }

            for (auto& wnd : details) {
                GetLayout()->Add(wnd, GetLayout()->Rows(), 0);
            }
        }
        SetCollapsed(new_collapsed);
    }

}

CombatLogWnd::Impl::Impl(CombatLogWnd& _wnd) :
    m_wnd(_wnd),
    m_text_format_flags(GG::FORMAT_WORDBREAK| GG::FORMAT_LEFT | GG::FORMAT_TOP),
    m_font(ClientUI::GetFont())
{ }

GG::Pt CombatLogWnd::Impl::MinUsableSize() const {
    return GG::Pt(m_font->SpaceWidth()*20, m_font->Lineskip()*10);
}

void CombatLogWnd::Impl::HandleWndChanged()
{ m_wnd.RequirePreRender(); }


namespace {
    /**Find a parent of type T*/
    template <typename T>
    T const* FindParentOfType(GG::Wnd const* parent) {
        GG::Wnd const * iwnd = parent;
        T const* type_T = nullptr;
        while (iwnd && !(type_T = dynamic_cast<const T*>(iwnd))){
            iwnd = iwnd->Parent().get();
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
            const std::shared_ptr<GG::Font>& font, GG::Clr color = GG::CLR_BLACK) :
            LinkText(x, y, UserString("ELLIPSIS"), font, color),
            m_text( new std::string(str)),
            m_signals()
        {
            // Register for signals that might bring the text into view
            if (const auto* log = FindParentOfType<CombatLogWnd>(&parent)) {
                m_signals.push_back(log->WndChangedSignal.connect(
                    boost::bind(&LazyScrollerLinkText::HandleMaybeVisible, this)));
            }

            if (const auto* scroll_panel = FindParentOfType<GG::ScrollPanel>(&parent)) {
                const auto* scroll = scroll_panel->GetScroll();
                m_signals.push_back(scroll->ScrolledAndStoppedSignal.connect(
                    boost::bind(&LazyScrollerLinkText::HandleScrolledAndStopped,
                                this, _1, _2, _3, _4)));

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
            const auto& tab = FindParentOfType<const GG::OverlayWnd>(Parent().get());
            if (tab && (tab->CurrentWndIndex() != 1))
                return;

            // Check if any part of text is in the scrollers visible area
            const auto* scroll_panel = FindParentOfType<GG::ScrollPanel>(Parent().get());
            if (scroll_panel && (scroll_panel->InClient(UpperLeft())
                             || scroll_panel->InClient(LowerRight())
                             || scroll_panel->InClient(GG::Pt(Right(), Top()))
                             || scroll_panel->InClient(GG::Pt(Left(), Bottom()))))
            {
                for (boost::signals2::connection& signal : m_signals)
                    signal.disconnect();

                m_signals.clear();

                SetText(*m_text);
                m_text.reset();

                ChangedSignal();
            }
        }

        void HandleScrolledAndStopped(int start_pos, int end_post, int min_pos, int max_pos)
        { HandleMaybeVisible(); }

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr)  override {
            LinkText::SizeMove(ul, lr);
            if (! m_signals.empty())
                HandleMaybeVisible();
        }

        std::unique_ptr<std::string> m_text;
        std::vector<boost::signals2::connection> m_signals;
    };

}

std::shared_ptr<LinkText> CombatLogWnd::Impl::DecorateLinkText(const std::string& text) {
    auto links = GG::Wnd::Create<LazyScrollerLinkText>(m_wnd, GG::X0, GG::Y0,
                                                       text, m_font, GG::CLR_WHITE);

    links->SetTextFormat(m_text_format_flags);

    links->SetDecorator(VarText::SHIP_ID_TAG, new ColorByOwner());
    links->SetDecorator(VarText::PLANET_ID_TAG, new ColorByOwner());
    links->SetDecorator(VarText::SYSTEM_ID_TAG, new ColorByOwner());
    links->SetDecorator(VarText::EMPIRE_ID_TAG, new ColorByOwner());

    links->LinkClickedSignal.connect(m_wnd.LinkClickedSignal);
    links->LinkDoubleClickedSignal.connect(m_wnd.LinkDoubleClickedSignal);
    links->LinkRightClickedSignal.connect(m_wnd.LinkRightClickedSignal);
    links->ChangedSignal.connect(boost::bind(&CombatLogWnd::Impl::HandleWndChanged, this));

    return links;
}

/** Fill \p new_logs with pointers to the flat log contents of \p
    event using the pre-calculated \p details.*/
void CombatLogWnd::Impl::PopulateWithFlatLogs(GG::X w, int viewing_empire_id,
                                              std::vector<std::shared_ptr<GG::Wnd>>& new_logs,
                                              ConstCombatEventPtr& event, std::string& details)
{
    if (!details.empty())
        new_logs.push_back(DecorateLinkText(details));

    if (!event->AreSubEventsEmpty(viewing_empire_id)) {
        for (auto& sub_event : event->SubEvents(viewing_empire_id)) {
            auto&& flat_logs = MakeCombatLogPanel(w, viewing_empire_id, sub_event);
            new_logs.insert(new_logs.end(), flat_logs.begin(), flat_logs.end());
        }
    }
}

// Returns either a simple LinkText for a simple log or a CombatLogAccordionPanel for a complex log
std::vector<std::shared_ptr<GG::Wnd>> CombatLogWnd::Impl::MakeCombatLogPanel(
    GG::X w, int viewing_empire_id, ConstCombatEventPtr event)
{
    std::vector<std::shared_ptr<GG::Wnd>> new_logs;

    // Create an accordion log if there are detail or sub events and
    // the log isn't explicitly flattened.  Otherwise, flatten the log,
    // details and sub events.

    if (!event->FlattenSubEvents() && !event->AreSubEventsEmpty(viewing_empire_id) ) {
        new_logs.push_back(GG::Wnd::Create<CombatLogAccordionPanel>(w, *this, viewing_empire_id, event));
        return new_logs;
    }

    if (!event->FlattenSubEvents() && !event->AreDetailsEmpty(viewing_empire_id)) {
        new_logs.push_back(GG::Wnd::Create<CombatLogAccordionPanel>(w, *this, viewing_empire_id, event));
        return new_logs;
    }

    std::string title = event->CombatLogDescription(viewing_empire_id);
    if (!(event->FlattenSubEvents() && title.empty()))
        new_logs.push_back(DecorateLinkText(title));

    std::string details = event->CombatLogDetails(viewing_empire_id);
    PopulateWithFlatLogs(w, viewing_empire_id, new_logs, event, details);

    return new_logs;
}


void CombatLogWnd::Impl::AddRow(std::shared_ptr<GG::Wnd> wnd) {
    if (auto&& layout = m_wnd.GetLayout())
        layout->Add(std::move(wnd), layout->Rows(), 0);
}

void CombatLogWnd::Impl::SetFont(std::shared_ptr<GG::Font> font)
{ m_font = font; }

void CombatLogWnd::Impl::SetLog(int log_id) {
    boost::optional<const CombatLog&> log = GetCombatLog(log_id);
    if (!log) {
        ErrorLogger() << "Couldn't find combat log with id: " << log_id;
        return;
    }

    m_wnd.DetachChildren();
    auto layout = GG::Wnd::Create<GG::DeferredLayout>(m_wnd.RelativeUpperLeft().x, m_wnd.RelativeUpperLeft().y,
                                                      m_wnd.Width(), m_wnd.Height(),
                                                      1, 1, ///< numrows, numcols
                                                      0, 0 ///< wnd margin, cell margin
                                                     );
    m_wnd.SetLayout(layout);

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    // Write Header text
    auto system = GetSystem(log->system_id);
    const std::string& sys_name = (system ? system->PublicName(client_empire_id) : UserString("ERROR"));
    DebugLogger(combat_log) << "Showing combat log #" << log_id << " at " << sys_name << " (" << log->system_id
                            << ") with " << log->combat_events.size() << " events";

    AddRow(DecorateLinkText(str(FlexibleFormat(UserString("ENC_COMBAT_LOG_DESCRIPTION_STR"))
                                % LinkTaggedIDText(VarText::SYSTEM_ID_TAG, log->system_id, sys_name)
                                % log->turn) + "\n"));
    AddRow(DecorateLinkText(UserString("COMBAT_INITIAL_FORCES")));
    AddRow(DecorateLinkText(CountsToText(CountByOwner(log->empire_ids, log->object_ids))));

    std::stringstream summary_text;
    summary_text << std::endl << UserString("COMBAT_SUMMARY_DESTROYED")
                 << std::endl << CountsToText(CountByOwner(log->empire_ids, log->destroyed_object_ids));
    AddRow(DecorateLinkText(summary_text.str()));

    // Write Logs
    for (CombatEventPtr event : log->combat_events) {
        DebugLogger(combat_log) << "event debug info: " << event->DebugString();
        for (auto&& wnd : MakeCombatLogPanel(m_font->SpaceWidth()*10, client_empire_id, event))
            AddRow(std::move(wnd));
    }

    // Add a dummy row that the layout manager can use to add space.
    AddRow(DecorateLinkText(""));
    layout->SetRowStretch(layout->Rows() - 1, 1);

    HandleWndChanged();
}


// Forward request to private implementation
CombatLogWnd::CombatLogWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
    m_impl(new Impl(*this))
{
    SetName("CombatLogWnd");
}

// This virtual destructor must exist to ensure that the m_impl is destroyed.
CombatLogWnd::~CombatLogWnd()
{}

void CombatLogWnd::SetFont(std::shared_ptr<GG::Font> font)
{ m_impl->SetFont(font); }

void CombatLogWnd::SetLog(int log_id)
{ m_impl->SetLog(log_id); }

GG::Pt CombatLogWnd::ClientUpperLeft() const
{ return UpperLeft() + GG::Pt(GG::X(MARGIN), GG::Y(MARGIN)); }

GG::Pt CombatLogWnd::ClientLowerRight() const
{ return LowerRight() - GG::Pt(GG::X(MARGIN), GG::Y(MARGIN)); }

GG::Pt CombatLogWnd::MinUsableSize() const
{ return m_impl->MinUsableSize(); }

void CombatLogWnd::HandleMadeVisible()
{ return m_impl->HandleWndChanged(); }

void CombatLogWnd::PreRender() {
    GG::Wnd::PreRender();

    /* Workaround

     Problem: CombatLogWnd is incorrectly initialized with a width of 30 which causes the combat
     accordion windows to incorrectly size themselves as if the title text were 30 wide.

    This fix forces the combat accordion window to correctly resize itself.

    TODO: Fix intial size of CombatReport from (30,15) to its actual first displayed size.*/
    GG::Pt size = Size();
    Resize(size + GG::Pt(2 * m_impl->m_font->SpaceWidth(), GG::Y0));
    GG::GUI::PreRenderWindow(this);
    Resize(size);
    GG::GUI::PreRenderWindow(this);
    /* End workaround. */

    WndChangedSignal();
}
