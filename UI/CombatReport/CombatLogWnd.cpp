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
    // TODO: Function adapted from CombatEvents.cpp, will need to be extracted to a common library
    const std::string& LinkTag(UniverseObjectType obj_type) {
        static const std::string EMPTY_STRING("");

        switch (obj_type) {
        case OBJ_SHIP:
            return VarText::SHIP_ID_TAG;
        case OBJ_FLEET:
            return VarText::FLEET_ID_TAG;
        case OBJ_PLANET:
            return VarText::PLANET_ID_TAG;
        case OBJ_BUILDING:
            return VarText::BUILDING_ID_TAG;
        case OBJ_SYSTEM:
            return VarText::SYSTEM_ID_TAG;
        case OBJ_FIELD:
        case OBJ_FIGHTER:
        default:
            return EMPTY_STRING;
        }
    }

    // TODO: Function adapted from CombatEvents.cpp, will need to beextracted to a common library
    std::string WrapWithTagAndId(const std::string& meat, const std::string& tag, int id)
    { return boost::str(boost::format("<%1% %2%>%3%</%1%>") % tag % id % meat); }

    /// Segregates \a objects into categories based on \a categories and ownership;
    /// only applies to objects owned by one of \a owners;
    /// within a category, sorts by \a order
    std::map<int, std::vector<std::vector<std::shared_ptr<UniverseObject>>>> SegregateForces(
        const std::set<int>& owners,
        const std::set<int>& objects,
        std::vector<std::function<bool(std::shared_ptr<UniverseObject>)>> categories,
        std::function<bool(std::shared_ptr<UniverseObject>, std::shared_ptr<UniverseObject>)> order)
    {
        decltype(SegregateForces(owners, objects, categories, order)) forces;

        for (const auto& object: Objects().find(objects)) {
            if (!object)
                continue;

            if (owners.count(object->Owner()) == 0)
                continue;

            auto& owner_forces = forces[object->Owner()];
            if (owner_forces.empty())
                owner_forces.resize(categories.size());

            for (size_t i = 0; i < categories.size(); ++i) {
                const auto& category = categories[i];
                if (category(object)) {
                    auto& owner_forces_category = owner_forces[i];
                    owner_forces_category.insert(
                        std::upper_bound(owner_forces_category.begin(), owner_forces_category.end(),
                                         object, order),
                        object);  // Insertion sort
                    break;        // Place only in one category
                }
            }
        }

        return forces;
    }

    bool IsShip(std::shared_ptr<UniverseObject> object)
    { return object->ObjectType() == UniverseObjectType::OBJ_SHIP; }

    bool HasPopulation(std::shared_ptr<UniverseObject> object) {
        const auto* m = object->GetMeter(METER_POPULATION);
        return m && m->Initial() > 0.0f;
    }

    std::string EmpireIdToText(int empire_id) {
        if (const Empire* empire = GetEmpire(empire_id))
            return GG::RgbaTag(empire->Color()) + "<" + VarText::EMPIRE_ID_TAG + " " +
                   std::to_string(empire->EmpireID()) + ">" + empire->Name() + "</" +
                   VarText::EMPIRE_ID_TAG + ">" + "</rgba>";
        else
            return GG::RgbaTag(ClientUI::DefaultLinkColor()) + UserString("NEUTRAL") + "</rgba>";
    }

    /// converts to "Empire_name: n" text
    std::string CountToText(int empire_id, int forces_count) {
        std::stringstream ss;
        ss << EmpireIdToText(empire_id) << ": " << forces_count;
        return ss.str();
    }

    class OrderByNameAndId {
    public:
        explicit OrderByNameAndId(int viewing_empire_id_ = ALL_EMPIRES) :
            viewing_empire_id(viewing_empire_id_)
        {}

        bool operator()(const std::shared_ptr<UniverseObject>& lhs,
                        const std::shared_ptr<UniverseObject>& rhs)
        {
            const auto& lhs_public_name = lhs->PublicName(viewing_empire_id);
            const auto& rhs_public_name = rhs->PublicName(viewing_empire_id);
            if (lhs_public_name != rhs_public_name) {
#if defined(FREEORION_MACOSX)
                // Collate on OSX seemingly ignores greek characters, resulting in sort order: X α
                // I, X β I, X α II
                return lhs_public_name < rhs_public_name;
#else
                return GetLocale("en_US.UTF-8").operator()(lhs_public_name, rhs_public_name);
#endif
            }

            // ID used for stable sorting of objects with non-unique name
            return lhs->ID() < rhs->ID();
        }

    private:
        const int viewing_empire_id;
    };

    std::string ForcesToText(
        int viewing_empire_id,
        const std::vector<std::vector<std::shared_ptr<UniverseObject>>>& forces,
        const std::string& delimiter = ", ",
        const std::string& category_delimiter = "\n-\n")
    {
        std::stringstream ss;

        bool first_category = true;
        for (const auto& category : forces) {
            if (category.empty())
                continue;
            if (first_category)
                first_category = false;
            else
                ss << category_delimiter;

            bool first_in_category = true;
            for (const auto& object : category) {
                if (first_in_category )
                    first_in_category = false;
                else
                    ss << delimiter;
                ss << WrapWithTagAndId(object->PublicName(viewing_empire_id),
                                       LinkTag(object->ObjectType()), object->ID());
            }
        }

        // TODO: This appends unicode character U+200B (zero-width space); this is a workaround for
        // a bug in LinkText class, which prevents linkifying of the last entry when it's at the end
        // of the string
        if (!first_category)
            ss << "​";

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
        int                                     viewing_empire_id = ALL_EMPIRES;
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

        m_expand_button->LeftPressedSignal.connect(boost::bind(&CombatLogAccordionPanel::ToggleExpansion, this));
        this->ExpandCollapseSignal.connect(boost::bind(&CombatLogWnd::Impl::HandleWndChanged, &log));

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

    /// A section used in initial forces and destroyed forces, displays empire's name and forces count;
    /// and can be expanded to enumerate all forces of this empire that take part in the combat.
    class EmpireForcesAccordionPanel : public AccordionPanel {
    public:
        EmpireForcesAccordionPanel(GG::X w,
                                   CombatLogWnd::Impl& log_,
                                   int viewing_empire_id_,
                                   int empire_id,
                                   std::vector<std::vector<std::shared_ptr<UniverseObject>>> forces_);

        ~EmpireForcesAccordionPanel();

        void CompleteConstruction() override;

    private:
        void ToggleExpansion();
        static size_t CountForces(std::vector<std::vector<std::shared_ptr<UniverseObject>>> forces);

        CombatLogWnd::Impl& log;
        const int viewing_empire_id = ALL_EMPIRES;
        std::shared_ptr<LinkText> title;
        std::shared_ptr<LinkText> details;
        std::vector<std::vector<std::shared_ptr<UniverseObject>>> forces;

        // distance between expansion symbol and text
        static const unsigned int BORDER_MARGIN = 5;
    };

    EmpireForcesAccordionPanel::EmpireForcesAccordionPanel(GG::X w,
                                                           CombatLogWnd::Impl& log_,
                                                           int viewing_empire_id_,
                                                           int empire_id,
                                                           std::vector<std::vector<std::shared_ptr<UniverseObject>>> forces_) :
        AccordionPanel(w, GG::Y(ClientUI::Pts()), true),
        log(log_),
        viewing_empire_id(viewing_empire_id_),
        title(log.DecorateLinkText(CountToText(empire_id, CountForces(forces_)))),
        forces(std::move(forces_))
    {}

    void EmpireForcesAccordionPanel::CompleteConstruction() {
        AccordionPanel::CompleteConstruction();
        AccordionPanel::SetInteriorColor(ClientUI::CtrlColor());

        m_expand_button->LeftPressedSignal.connect(boost::bind(&EmpireForcesAccordionPanel::ToggleExpansion, this));
        this->ExpandCollapseSignal.connect(boost::bind(&CombatLogWnd::Impl::HandleWndChanged, &log));

        SetBorderMargin(BORDER_MARGIN);

        SetLayout(GG::Wnd::Create<GG::DeferredLayout>(UpperLeft().x, UpperLeft().y, Width(),
                                                      Height(), 1, 1));
        GetLayout()->Add(title, 0, 0, 1, 1);
        SetCollapsed(true);
        RequirePreRender();
    }

    EmpireForcesAccordionPanel::~EmpireForcesAccordionPanel() {}

    void EmpireForcesAccordionPanel::ToggleExpansion() {
        bool new_collapsed = !IsCollapsed();

        if (new_collapsed) {
            GetLayout()->Remove(details.get());
        } else {
            if (!details)
                details = log.DecorateLinkText(ForcesToText(viewing_empire_id, forces));

            GetLayout()->Add(details, GetLayout()->Rows(), 0);
        }

        SetCollapsed(new_collapsed);
    }

    size_t EmpireForcesAccordionPanel::CountForces(
        std::vector<std::vector<std::shared_ptr<UniverseObject>>> forces) {
        size_t n = 0;
        for (const auto& owner_forces : forces)
            n += owner_forces.size();
        return n;
    }
}

CombatLogWnd::Impl::Impl(CombatLogWnd& _wnd) :
    m_wnd(_wnd),
    m_text_format_flags(GG::FORMAT_WORDBREAK| GG::FORMAT_LEFT | GG::FORMAT_TOP),
    m_font(ClientUI::GetFont())
{}

GG::Pt CombatLogWnd::Impl::MinUsableSize() const
{ return GG::Pt(m_font->SpaceWidth()*20, m_font->Lineskip()*10); }

void CombatLogWnd::Impl::HandleWndChanged()
{ m_wnd.RequirePreRender(); }


namespace {
    /**Find a parent of type T*/
    template <typename T>
    T const* FindParentOfType(GG::Wnd const* parent)
    {
        GG::Wnd const * iwnd = parent;
        T const* type_T = dynamic_cast<const T*>(iwnd);
        while (iwnd && !type_T) {
            iwnd = iwnd->Parent().get();
            if (iwnd)
                type_T = dynamic_cast<const T*>(iwnd);
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
            m_text(new std::string(str)),
            m_signals()
        {
            // Register for signals that might bring the text into view
            if (const auto* log = FindParentOfType<CombatLogWnd>(&parent)) {
                m_signals.push_back(log->WndChangedSignal.connect(
                    boost::bind(&LazyScrollerLinkText::HandleMaybeVisible, this)));
            }

            namespace ph = boost::placeholders;

            if (const auto* scroll_panel = FindParentOfType<GG::ScrollPanel>(&parent)) {
                const auto* scroll = scroll_panel->GetScroll();
                m_signals.push_back(scroll->ScrolledAndStoppedSignal.connect(
                    boost::bind(&LazyScrollerLinkText::HandleScrolledAndStopped,
                              this, ph::_1, ph::_2, ph::_3, ph::_4)));
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
    auto system = Objects().get<System>(log->system_id);
    const std::string& sys_name = (system ? system->PublicName(client_empire_id) : UserString("ERROR"));
    DebugLogger(combat_log) << "Showing combat log #" << log_id << " at " << sys_name << " (" << log->system_id
                            << ") with " << log->combat_events.size() << " events";

    AddRow(DecorateLinkText(str(FlexibleFormat(UserString("ENC_COMBAT_LOG_DESCRIPTION_STR"))
                                % LinkTaggedIDText(VarText::SYSTEM_ID_TAG, log->system_id, sys_name)
                                % log->turn) + "\n"));


    AddRow(DecorateLinkText(UserString("COMBAT_INITIAL_FORCES")));
    const auto initial_forces =
        SegregateForces(log->empire_ids, log->object_ids, {IsShip, HasPopulation},
                        OrderByNameAndId(client_empire_id));
    for (const auto& empire_forces : initial_forces)
        AddRow(GG::Wnd::Create<EmpireForcesAccordionPanel>(
            GG::X0, *this, client_empire_id, empire_forces.first, empire_forces.second));

    AddRow(DecorateLinkText("\n" + UserString("COMBAT_SUMMARY_DESTROYED")));
    const auto destroyed_forces =
        SegregateForces(log->empire_ids, log->destroyed_object_ids, {IsShip, HasPopulation},
                        OrderByNameAndId(client_empire_id));
    for (const auto& empire_forces : destroyed_forces)
        AddRow(GG::Wnd::Create<EmpireForcesAccordionPanel>(
            GG::X0, *this, client_empire_id, empire_forces.first, empire_forces.second));

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
