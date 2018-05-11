#include "PopulationPanel.h"

#include <GG/Button.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/UniverseObject.h"
#include "../universe/PopCenter.h"
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

PopulationPanel::PopulationPanel(GG::X w, int object_id) :
    AccordionPanel(w, GG::Y(ClientUI::Pts()*2)),
    m_popcenter_id(object_id),
    m_meter_stats(),
    m_multi_icon_value_indicator(nullptr),
    m_multi_meter_status_bar(nullptr)
{}

void PopulationPanel::CompleteConstruction() {
    AccordionPanel::CompleteConstruction();
    SetName("PopulationPanel");

    auto pop = GetPopCenter();
    if (!pop)
        throw std::invalid_argument("Attempted to construct a PopulationPanel with an object id is not a PopCenter");

    m_expand_button->LeftPressedSignal.connect(
        boost::bind(&PopulationPanel::ExpandCollapseButtonPressed, this));

    const auto obj = GetUniverseObject(m_popcenter_id);
    if (!obj) {
        ErrorLogger() << "Invalid object id " << m_popcenter_id;
        return;
    }

    // small meter indicators - for use when panel is collapsed
    m_meter_stats.push_back({
        METER_POPULATION,
        GG::Wnd::Create<StatisticIcon>(ClientUI::SpeciesIcon(pop->SpeciesName()),
                                       obj->InitialMeterValue(METER_POPULATION), 3, false,
                                       MeterIconSize().x, MeterIconSize().y)});
    m_meter_stats.push_back({
        METER_HAPPINESS,
        GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(METER_HAPPINESS),
                                       obj->InitialMeterValue(METER_HAPPINESS), 3, false,
                                       MeterIconSize().x, MeterIconSize().y)});
    m_meter_stats.push_back({
        METER_CONSTRUCTION,
        GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(METER_CONSTRUCTION),
                                       obj->InitialMeterValue(METER_CONSTRUCTION), 3, false,
                                       MeterIconSize().x, MeterIconSize().y)});

    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType>> meters;

    for (auto& meter_stat : m_meter_stats) {
        meter_stat.second->InstallEventFilter(shared_from_this());
        AttachChild(meter_stat.second);
        meters.push_back({meter_stat.first, AssociatedMeterType(meter_stat.first)});
    }

    // attach and show meter bars and large resource indicators
    m_multi_icon_value_indicator =  GG::Wnd::Create<MultiIconValueIndicator>(Width() - 2*EDGE_PAD,   m_popcenter_id, meters);
    m_multi_meter_status_bar =      GG::Wnd::Create<MultiMeterStatusBar>(Width() - 2*EDGE_PAD,       m_popcenter_id, meters);

    // determine if this panel has been created yet.
    std::map<int, bool>::iterator it = s_expanded_map.find(m_popcenter_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_popcenter_id] = false; // if not, default to collapsed state

    Refresh();
}

PopulationPanel::~PopulationPanel()
{}

void PopulationPanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_popcenter_id]) return; // nothing to do
    s_expanded_map[m_popcenter_id] = expanded;

    DoLayout();
}

bool PopulationPanel::EventFilter(GG::Wnd* w, const GG::WndEvent& event) {
    if (event.Type() != GG::WndEvent::RClick)
        return false;
    const GG::Pt& pt = event.Point();

    MeterType meter_type = INVALID_METER_TYPE;
    for (const auto& meter_stat : m_meter_stats) {
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

    std::string species_name;
    bool retval = false;

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    auto pc = GetPopCenter();
    if (meter_type == METER_POPULATION && pc) {
        species_name = pc->SpeciesName();
        if (!species_name.empty()) {
            auto zoom_species_action = [&retval, &species_name]() { retval = ClientUI::GetClientUI()->ZoomToSpecies(species_name); };
            std::string species_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(species_name));
            popup->AddMenuItem(GG::MenuItem(species_label, false, false, zoom_species_action));
        }
    }

    if (!meter_title.empty()) {
        auto pedia_meter_type_action = [&retval, &meter_string]() { retval = ClientUI::GetClientUI()->ZoomToMeterTypeArticle(meter_string); };
        std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % meter_title);
        popup->AddMenuItem(GG::MenuItem(popup_label, false, false, pedia_meter_type_action));
    }

    popup->Run();
    return retval;
}

void PopulationPanel::Update() {
    // remove any old browse wnds
    for (auto& meter_stat : m_meter_stats) {
        meter_stat.second->ClearBrowseInfoWnd();
        m_multi_icon_value_indicator->ClearToolTip(meter_stat.first);
    }

    auto pop = GetPopCenter();
    if (!pop) {
        ErrorLogger() << "PopulationPanel::Update couldn't get PopCenter or couldn't get UniverseObject";
        return;
    }

    // meter bar displays population stats
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    // tooltips
    for (auto& meter_stat : m_meter_stats) {
        meter_stat.second->SetValue(pop->InitialMeterValue(meter_stat.first));

        auto browse_wnd = GG::Wnd::Create<MeterBrowseWnd>(m_popcenter_id, meter_stat.first, AssociatedMeterType(meter_stat.first));
        meter_stat.second->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(meter_stat.first, browse_wnd);
    }
}

void PopulationPanel::Refresh() {
    for (auto& meter_stat : m_meter_stats)
        meter_stat.second->RequirePreRender();

    RequirePreRender();
}

void PopulationPanel::PreRender() {
    AccordionPanel::PreRender();
    Update();
    DoLayout();
}

void PopulationPanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_popcenter_id]); }

void PopulationPanel::DoLayout() {
    AccordionPanel::DoLayout();

    for (const auto& meter_stat : m_meter_stats) {
        DetachChild(meter_stat.second);
    }

    // detach / hide meter bars and large resource indicators
    DetachChild(m_multi_meter_status_bar);
    DetachChild(m_multi_icon_value_indicator);

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_popcenter_id]) {
        // position and reattach icons to be shown
        int n = 0;
        GG::X stride = MeterIconSize().x * 7/2;
        for (const auto& meter_stat : m_meter_stats) {
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

    SetCollapsed(!s_expanded_map[m_popcenter_id]);
}

std::shared_ptr<const PopCenter> PopulationPanel::GetPopCenter() const {
    auto pop = ::GetPopCenter(m_popcenter_id);
    if (!pop) {
        ErrorLogger() << "PopulationPanel tried to get an object with an invalid m_popcenter_id";
        return nullptr;
    }
    return pop;
}

std::map<int, bool> PopulationPanel::s_expanded_map;
