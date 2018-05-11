#include "MilitaryPanel.h"

#include <GG/Button.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/UniverseObject.h"
#include "../universe/Planet.h"
#include "../universe/Enums.h"
#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "MeterBrowseWnd.h"
#include "MultiIconValueIndicator.h"
#include "MultiMeterStatusBar.h"

namespace {
    const int       EDGE_PAD(3);

    /** How big we want meter icons with respect to the current UI font size.
      * Meters should scale along font size, but not below the size for the
      * default 12 points font. */
    GG::Pt MeterIconSize() {
        const int icon_size = std::max(ClientUI::Pts(), 12) * 4/3;
        return GG::Pt(GG::X(icon_size), GG::Y(icon_size));
    }
}

MilitaryPanel::MilitaryPanel(GG::X w, int planet_id) :
    AccordionPanel(w, GG::Y(ClientUI::Pts()*2)),
    m_planet_id(planet_id),
    m_meter_stats(),
    m_multi_icon_value_indicator(nullptr),
    m_multi_meter_status_bar(nullptr)
{}

void MilitaryPanel::CompleteConstruction() {
    AccordionPanel::CompleteConstruction();

    SetName("MilitaryPanel");

    auto planet = this->GetPlanet();
    if (!planet)
        throw std::invalid_argument("Attempted to construct a MilitaryPanel with an object id is not a Planet");

    m_expand_button->LeftPressedSignal.connect(
        boost::bind(&MilitaryPanel::ExpandCollapseButtonPressed, this));

    const auto obj = GetUniverseObject(m_planet_id);
    if (!obj) {
        ErrorLogger() << "Invalid object id " << m_planet_id;
        return;
    }

    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType>> meters;

    // small meter indicators - for use when panel is collapsed
    for (MeterType meter : {METER_SHIELD, METER_DEFENSE, METER_TROOPS, METER_DETECTION, METER_STEALTH})
    {
        auto stat = GG::Wnd::Create<StatisticIcon>(
            ClientUI::MeterIcon(meter), obj->InitialMeterValue(meter),
            3, false, MeterIconSize().x, MeterIconSize().y);
        stat->InstallEventFilter(shared_from_this());
        AttachChild(stat);
        m_meter_stats.push_back({meter, stat});
        meters.push_back({meter, AssociatedMeterType(meter)});
    }

    // attach and show meter bars and large resource indicators
    m_multi_meter_status_bar =      GG::Wnd::Create<MultiMeterStatusBar>(Width() - 2*EDGE_PAD,       m_planet_id, meters);
    m_multi_icon_value_indicator =  GG::Wnd::Create<MultiIconValueIndicator>(Width() - 2*EDGE_PAD,   m_planet_id, meters);

    // determine if this panel has been created yet.
    auto it = s_expanded_map.find(m_planet_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_planet_id] = false; // if not, default to collapsed state

    Refresh();
}

MilitaryPanel::~MilitaryPanel()
{}

void MilitaryPanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_planet_id]) return; // nothing to do
    s_expanded_map[m_planet_id] = expanded;

    DoLayout();
}

bool MilitaryPanel::EventFilter(GG::Wnd* w, const GG::WndEvent& event) {
    if (event.Type() != GG::WndEvent::RClick)
        return false;
    const GG::Pt& pt = event.Point();

    MeterType meter_type = INVALID_METER_TYPE;
    for (auto& meter_stat : m_meter_stats) {
        if (meter_stat.second.get() == w) {
            meter_type = meter_stat.first;
            break;
        }
    }

    if (meter_type == INVALID_METER_TYPE)
        return false;

    std::string meter_string = boost::lexical_cast<std::string>(meter_type);
    std::string meter_title;
    if (UserStringExists(meter_string))
        meter_title = UserString(meter_string);
    if (meter_title.empty())
        return false;

    bool retval = false;
    auto zoom_action = [&retval, &meter_string]() { retval = ClientUI::GetClientUI()->ZoomToMeterTypeArticle(meter_string); };

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % meter_title);
    popup->AddMenuItem(GG::MenuItem(popup_label, false, false, zoom_action));

    return retval;
}

void MilitaryPanel::Update() {
    auto obj = GetUniverseObject(m_planet_id);
    if (!obj) {
        ErrorLogger() << "MilitaryPanel::Update coudln't get object with id  " << m_planet_id;
        return;
    }

    // meter bar displays military stats
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    // tooltips
    for (auto& meter_stat : m_meter_stats) {
        meter_stat.second->SetValue(obj->InitialMeterValue(meter_stat.first));

        auto browse_wnd = GG::Wnd::Create<MeterBrowseWnd>(m_planet_id, meter_stat.first, AssociatedMeterType(meter_stat.first));
        meter_stat.second->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(meter_stat.first, browse_wnd);
    }
}

void MilitaryPanel::Refresh() {
    for (auto& meter_stat : m_meter_stats)
        meter_stat.second->RequirePreRender();

    RequirePreRender();
}

void MilitaryPanel::PreRender() {
    AccordionPanel::PreRender();
    Update();
    DoLayout();
}

void MilitaryPanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_planet_id]); }

void MilitaryPanel::DoLayout() {
    AccordionPanel::DoLayout();

    for (auto& meter_stat : m_meter_stats) {
        DetachChild(meter_stat.second);
    }

    // detach / hide meter bars and large resource indicators
    DetachChild(m_multi_meter_status_bar);
    DetachChild(m_multi_icon_value_indicator);

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_planet_id]) {
        // position and reattach icons to be shown
        int n = 0;
        GG::X stride = MeterIconSize().x * 7/2;
        for (auto& meter_stat : m_meter_stats) {
            GG::X x = n * stride;

            auto& icon = meter_stat.second;
            GG::Pt icon_ul(x, GG::Y0);
            GG::Pt icon_lr = icon_ul + MeterIconSize();
            icon->SizeMove(icon_ul, icon_lr);

            if (x + icon->MinUsableSize().x >= ClientWidth())
                break;

            AttachChild(icon);
            icon->Show();

            n++;
        }

        Resize(GG::Pt(Width(), std::max(MeterIconSize().y, m_expand_button->Height())));
    } else {
        // attach and show meter bars and large resource indicators
        GG::Y top = Top();

        AttachChild(m_multi_icon_value_indicator);
        m_multi_icon_value_indicator->MoveTo(GG::Pt(GG::X(EDGE_PAD), GG::Y(EDGE_PAD)));
        m_multi_icon_value_indicator->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_icon_value_indicator->Height()));

        AttachChild(m_multi_meter_status_bar);
        m_multi_meter_status_bar->MoveTo(GG::Pt(GG::X(EDGE_PAD), m_multi_icon_value_indicator->Bottom() + EDGE_PAD - top));
        m_multi_meter_status_bar->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_meter_status_bar->Height()));

        MoveChildUp(m_expand_button);

        Resize(GG::Pt(Width(), m_multi_meter_status_bar->Bottom() + EDGE_PAD - top));
    }

    SetCollapsed(!s_expanded_map[m_planet_id]);
}

std::shared_ptr<const Planet> MilitaryPanel::GetPlanet() const {
    auto obj = GetUniverseObject(m_planet_id);
    if (!obj) {
        ErrorLogger() << "MilitaryPanel tried to get an object with an invalid m_planet_id";
        return nullptr;
    }
    auto planet = std::dynamic_pointer_cast<const Planet>(obj);
    if (!planet) {
        ErrorLogger() << "MilitaryPanel failed casting an object pointer to a Planet pointer";
        return nullptr;
    }
    return planet;
}

std::map<int, bool> MilitaryPanel::s_expanded_map;
