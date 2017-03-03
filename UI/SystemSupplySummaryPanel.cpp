#include "SystemSupplySummaryPanel.h"

#include <GG/Texture.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/PopCenter.h"
#include "../universe/UniverseObject.h"
#include "../universe/System.h"
#include "../universe/Enums.h"
#include "../client/human/HumanClientApp.h"
#include "../Empire/Empire.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"

#include <iomanip>

namespace {
    constexpr double    TWO_PI = 2.0*3.1415926536;

    constexpr int       EDGE_PAD(3);

    int         IconSpacing()
    { return ClientUI::Pts(); }
    GG::X       IconWidth()
    { return GG::X(IconSpacing()*2); }
    GG::Y       IconHeight()
    { return GG::Y(IconSpacing()*2); }

    // Copied pasted from CombatEvents.cpp
    // TODO find this code a home.
    std::string WrapColorTag(std::string const & text, const GG::Clr& c) {
        std::stringstream stream;
        stream << "<rgba "
               << std::setw(3) << static_cast<int>(c.r) << " "
               << std::setw(3) << static_cast<int>(c.g) << " "
               << std::setw(3) << static_cast<int>(c.b) << " "
               << std::setw(3) << static_cast<int>(c.a)
               << ">" << text << "</rgba>";
        return stream.str();
    }

    std::string EmpireColorWrappedText(const Empire* empire, const std::string& text) {
        // TODO: refactor this to somewhere that links with the UI code.
        GG::Clr c = (empire ? empire->Color() : ClientUI::DefaultLinkColor());
        return WrapColorTag(text, c);
    }



    /** Gives information about all supply in system. */
    class SystemSupplyBrowseWnd : public GG::BrowseInfoWnd {
        public:
        SystemSupplyBrowseWnd(int system_id);

        bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override
        { return !m_is_empty; }

        bool IsEmpty() const
        { return m_is_empty; }

        void Render() override;

        private:
        bool m_is_empty;
        CUILabel* m_label;
    };


    SystemSupplyBrowseWnd::SystemSupplyBrowseWnd(int system_id) :
        GG::BrowseInfoWnd(GG::X0, GG::Y0, GG::X1, GG::Y1),
        m_label(nullptr)
    {
        const auto system_supply_it = GetSupplyManager().SystemSupply(system_id);
        m_is_empty = !system_supply_it;
        if (m_is_empty)
            return;

        const auto system = GetSystem(system_id);

        // Get detection strength
        int viewing_empire_id = HumanClientApp::GetApp()->EmpireID();
        const Empire* client_empire = GetEmpire(viewing_empire_id);
        const auto detection_meter = client_empire->GetMeter("METER_DETECTION_STRENGTH");
        const auto detection_strength =  detection_meter ? detection_meter->Current() : 0.0f;

        std::stringstream ss;
        ss << std::left << (system ? system->Name() : UserString("UNKNOWN"))
           << " (" << system_id << ") supply details." <<std::endl;
        ss << std::setw(50) << EmpireColorWrappedText(nullptr, "Empire")
           << std::setw(50) << EmpireColorWrappedText(nullptr, "Source")
           << std::setw(7) <<"(id)"
           << std::setw(7) << "Range"
           << std::setw(7) << "Dist" << std::setw(9) << "Stealth"
           << std::setw(9) << "Bonus =" << std::setw(10) << "(Visible"
           << std::setw(10) << "+ Ship +" << std::setw(8) << "Colony)" <<std::endl;

        bool at_least_one_visible = false;
        int num_rows = 1;
        DebugLogger() << " Supply in system " << system->Name() << "(" << system_id << ") ";
        for (const auto& empire_and_supply_fields : (*system_supply_it)->second) {
            auto empire_id = empire_and_supply_fields.first;
            auto empire = GetEmpire(empire_id);
            for (const auto& supply_tuple : empire_and_supply_fields.second) {
                num_rows += 1;
                const auto& source_id = std::get<sseSource>(supply_tuple);
                const auto& range = std::get<sseRange>(supply_tuple);
                const auto& bonus_tuple = std::get<sseBonus>(supply_tuple);
                const auto& distance = std::get<sseDistance>(supply_tuple);
                const auto& stealth = std::get<sseStealth>(supply_tuple);
                const auto& bonus = std::get<ssbBonus>(bonus_tuple);
                const auto& visibility = std::get<ssbVisibilityBonus>(bonus_tuple);
                const auto& ships = std::get<ssbShipBonus>(bonus_tuple);
                const auto& colony = std::get<ssbColonyBonus>(bonus_tuple);

                if (stealth >= detection_strength && empire_id != viewing_empire_id)
                    continue;
                at_least_one_visible = true;

                auto source = Objects().Object(source_id);
                ss << std::setw(50) << EmpireColorWrappedText(empire, (empire ? empire->Name() : UserString("UNKNOWN")));
                ss << std::setw(50) << EmpireColorWrappedText(empire, (source ? source->Name() : UserString("UNKNOWN")));

                ss << std::setw(7) << std::to_string(source_id);
                ss << std::setw(7) << std::setprecision(2) << range;
                ss << std::setw(7) << std::setprecision(3) << distance;
                ss << std::setw(9) << std::setprecision(2) << stealth;
                ss << std::setw(9) << std::setprecision(2) << bonus <<"= (";
                ss << std::setw(10) << std::setprecision(2) << visibility << " + ";
                ss << std::setw(10) << std::setprecision(2) << ships << " + ";
                ss << std::setw(8) << std::setprecision(2) << colony << ")";
                ss << std::endl;

                DebugLogger() << "Harrow for empire = "<< empire_and_supply_fields.first <<" supply "
                              << SupplySystemEmpireTupleString(supply_tuple);
            }
        }

        m_label = new CUILabel(ss.str(), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP);
        m_label->MoveTo(GG::Pt(GG::X0, GG::Y0));
        m_label->SetFont(ClientUI::GetBoldFont());
        Resize(m_label->TextLowerRight());
        AttachChild(m_label);

        if (!at_least_one_visible)
            m_is_empty = true;
    }

    void SystemSupplyBrowseWnd::Render() {
        GG::Pt ul = UpperLeft();
        GG::Pt lr = LowerRight();
        // main background
        GG::FlatRectangle(ul, lr, OpaqueColor(ClientUI::WndColor()), ClientUI::WndOuterBorderColor(), 1);

    }
}

class SystemSupplySummaryPanel::Impl {
    public:
    Impl(SystemSupplySummaryPanel& wnd, const int system_id);

    bool            IsEmpty() const;

    void            Update();

    private:
    SystemSupplySummaryPanel&  m_wnd;

    int m_system_id;

    StatisticIcon* m_icon;
    std::shared_ptr<SystemSupplyBrowseWnd> m_browse_window;
};

SystemSupplySummaryPanel::Impl::Impl(SystemSupplySummaryPanel& wnd, const int system_id) :
    m_wnd(wnd),
    m_system_id(system_id),
    m_icon(),
    m_browse_window()
{
    const auto height = EDGE_PAD + IconHeight() + ClientUI::Pts()*3/2;
    const auto width = EDGE_PAD + IconWidth() + ClientUI::Pts()*3/2;
    m_wnd.Resize(GG::Pt(width, height));
    DebugLogger() << "size = " << m_wnd.Size();

    std::shared_ptr<GG::Texture> texture = ClientUI::MeterIcon(METER_SUPPLY);

    m_icon = new StatisticIcon(texture, GG::X0, GG::Y0, IconWidth(), IconHeight());
    GG::Pt icon_ul = GG::Pt(GG::X(EDGE_PAD), GG::Y(EDGE_PAD));
    GG::Pt icon_lr = icon_ul + GG::Pt(IconWidth(), IconHeight() + ClientUI::Pts()*3/2);
    m_icon->SizeMove(icon_ul, icon_lr);
    // TODO an a context menu that goes to Pedia Supply info
    //m_icon->InstallEventFilter(this);
    m_wnd.AttachChild(m_icon);

    Update();
}

bool SystemSupplySummaryPanel::Impl::IsEmpty() const
{ return !m_browse_window || m_browse_window->IsEmpty(); }

void SystemSupplySummaryPanel::Impl::Update() {
    m_browse_window = std::make_shared<SystemSupplyBrowseWnd>(m_system_id);
    m_icon->SetBrowseInfoWnd(m_browse_window);
}


SystemSupplySummaryPanel::SystemSupplySummaryPanel(const int system_id) :
    GG::Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE),
    pimpl(new Impl(*this, system_id))
{}

SystemSupplySummaryPanel::~SystemSupplySummaryPanel()
{}

void SystemSupplySummaryPanel::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // outline of whole control
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}

void SystemSupplySummaryPanel::Update()
{ return pimpl->Update(); }

bool SystemSupplySummaryPanel::IsEmpty()
{ return pimpl->IsEmpty(); }
