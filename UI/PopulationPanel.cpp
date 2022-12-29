#include "PopulationPanel.h"

#include <GG/Button.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/UniverseObject.h"
#include "../universe/Enums.h"
#include "../universe/Planet.h"
#include "../client/human/GGHumanClientApp.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "MeterBrowseWnd.h"
#include "MultiIconValueIndicator.h"
#include "MultiMeterStatusBar.h"

namespace {
    constexpr int EDGE_PAD(3);

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
    m_popcenter_id(object_id)
{}

void PopulationPanel::CompleteConstruction() {
    AccordionPanel::CompleteConstruction();
    SetName("PopulationPanel");

    m_expand_button->LeftPressedSignal.connect(
        boost::bind(&PopulationPanel::ExpandCollapseButtonPressed, this));

    auto pop = Objects().get<Planet>(m_popcenter_id);
    if (!pop) {
        ErrorLogger() << "Attempted to construct a PopulationPanel with an object id that is not a planet: " << m_popcenter_id;
        return;
    }

    // small meter indicators - for use when panel is collapsed
    m_meter_stats.emplace_back(
        MeterType::METER_POPULATION,
        GG::Wnd::Create<StatisticIcon>(ClientUI::SpeciesIcon(pop->SpeciesName()),
                                       pop->GetMeter(MeterType::METER_POPULATION)->Initial(), 3, false,
                                       MeterIconSize().x, MeterIconSize().y));
    m_meter_stats.emplace_back(
        MeterType::METER_HAPPINESS,
        GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(MeterType::METER_HAPPINESS),
                                       pop->GetMeter(MeterType::METER_HAPPINESS)->Initial(), 3, false,
                                       MeterIconSize().x, MeterIconSize().y));
    m_meter_stats.emplace_back(
        MeterType::METER_CONSTRUCTION,
        GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(MeterType::METER_CONSTRUCTION),
                                       pop->GetMeter(MeterType::METER_CONSTRUCTION)->Initial(), 3, false,
                                       MeterIconSize().x, MeterIconSize().y));
    m_meter_stats.emplace_back(
        MeterType::METER_REBEL_TROOPS,
        GG::Wnd::Create<StatisticIcon>(ClientUI::MeterIcon(MeterType::METER_REBEL_TROOPS),
                                       pop->GetMeter(MeterType::METER_REBEL_TROOPS)->Initial(), 3, false,
                                       MeterIconSize().x, MeterIconSize().y));

    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType>> meters;
    meters.reserve(m_meter_stats.size());

    for (const auto& meter_stat : m_meter_stats) {
        MeterType meter_type = meter_stat.first;

        meter_stat.second->RightClickedSignal.connect([this, meter_type](GG::Pt pt) {
            auto meter_string = to_string(meter_type);

            auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
            auto* pc = Objects().getRaw<Planet>(m_popcenter_id);
            if (meter_type == MeterType::METER_POPULATION && pc) {
                std::string species_name = pc->SpeciesName();
                if (!species_name.empty()) {
                    auto zoom_species_action = [species_name]() { ClientUI::GetClientUI()->ZoomToSpecies(species_name); };
                    std::string species_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) %
                                                                              UserString(species_name));
                    popup->AddMenuItem(GG::MenuItem(std::move(species_label), false, false,
                                                    zoom_species_action));
                }
            }

            auto pedia_meter_type_action = [meter_string]() { ClientUI::GetClientUI()->ZoomToMeterTypeArticle(std::string{meter_string}); };
            std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) %
                                                                    UserString(meter_string));
            popup->AddMenuItem(GG::MenuItem(std::move(popup_label), false, false,
                                            pedia_meter_type_action));

            popup->Run();
        });
        AttachChild(meter_stat.second);
        meters.emplace_back(meter_stat.first, AssociatedMeterType(meter_stat.first));
    }

    // attach and show meter bars and large resource indicators
    m_multi_icon_value_indicator = GG::Wnd::Create<MultiIconValueIndicator>(Width() - 2*EDGE_PAD, m_popcenter_id, meters);
    m_multi_meter_status_bar =     GG::Wnd::Create<MultiMeterStatusBar>(Width() - 2*EDGE_PAD,     m_popcenter_id, meters);

    // determine if this panel has been created yet.
    auto it = s_expanded_map.find(m_popcenter_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_popcenter_id] = false; // if not, default to collapsed state

    Refresh();
}

void PopulationPanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_popcenter_id]) return; // nothing to do
    s_expanded_map[m_popcenter_id] = expanded;

    DoLayout();
}

void PopulationPanel::Update() {
    // remove any old browse wnds
    for (auto& [meter_name, old_stat_icon] : m_meter_stats) {
        old_stat_icon->ClearBrowseInfoWnd();
        m_multi_icon_value_indicator->ClearToolTip(meter_name);
    }

    auto pop = Objects().get<Planet>(m_popcenter_id);
    if (!pop) {
        ErrorLogger() << "PopulationPanel::Update couldn't get Planet";
        return;
    }

    // meter bar displays population stats
    if (m_multi_meter_status_bar)
        m_multi_meter_status_bar->Update();
    if (m_multi_icon_value_indicator)
        m_multi_icon_value_indicator->Update();

    // tooltips
    for (auto& [meter_name, old_stat_icon] : m_meter_stats) {
        old_stat_icon->SetValue(pop->GetMeter(meter_name)->Initial());

        auto browse_wnd = GG::Wnd::Create<MeterBrowseWnd>(m_popcenter_id, meter_name, AssociatedMeterType(meter_name));
        old_stat_icon->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(meter_name, browse_wnd);
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

    for (const auto& meter_stat : m_meter_stats)
        DetachChild(meter_stat.second);

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
    } else if (m_multi_icon_value_indicator && m_multi_meter_status_bar) {
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

std::map<int, bool> PopulationPanel::s_expanded_map;
