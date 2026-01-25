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

    auto& app = GetApp();
    const auto& objects = app.GetContext().ContextObjects();
    auto& ui = app.GetUI();

    auto planet = objects.get<Planet>(m_popcenter_id);
    if (!planet) {
        ErrorLogger() << "Attempted to construct a PopulationPanel with an object id that is not a planet: " << m_popcenter_id;
        return;
    }
    const auto& species_name = planet->SpeciesName();

    const auto make_stat_icon_tex = [font{ui.GetFont()}, sz{MeterIconSize()}](auto tex, std::string name) {
        auto si = GG::Wnd::Create<StatisticIcon>(std::move(tex), font, sz.x, sz.y, 3,
                                                 StatisticIcon::IndicateChangeColour::INDICATE_FOR_OTHER,
                                                 StatisticIcon::ShowSign::HIDE_IF_NON_NEGATIVE);
        if (si) si->SetName(std::move(name));
        return si;
    };
    const auto make_stat_icon = [&make_stat_icon_tex, &ui](MeterType mt, std::string name)
    { return make_stat_icon_tex(ui.MeterIcon(mt), std::move(name)); };

    // small meter indicators - for use when panel is collapsed
    m_meter_stats.emplace_back(MeterType::METER_POPULATION, make_stat_icon_tex(ui.SpeciesIcon(species_name), "Population StatisticIcon"));
    m_meter_stats.emplace_back(MeterType::METER_HAPPINESS, make_stat_icon(MeterType::METER_HAPPINESS, "Stability StatisticIcon"));
    m_meter_stats.emplace_back(MeterType::METER_CONSTRUCTION, make_stat_icon(MeterType::METER_CONSTRUCTION, "Infrastructure StatisticIcon"));
    m_meter_stats.emplace_back(MeterType::METER_REBEL_TROOPS, make_stat_icon(MeterType::METER_REBEL_TROOPS, "Rebels StatisticIcon"));

    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType>> meters;
    meters.reserve(m_meter_stats.size());

    for (const auto& [meter_type, stat_icon] : m_meter_stats) {
        stat_icon->RightClickedSignal.connect([this, meter_type{meter_type}](GG::Pt pt) {
            auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
            if (meter_type == MeterType::METER_POPULATION) {
                const auto& objects = GetApp().GetContext().ContextObjects();
                if (const auto* planet = objects.getRaw<Planet>(m_popcenter_id)) {
                    const auto& species_name = planet->SpeciesName();
                    if (!species_name.empty()) {
                        auto zoom_species_action = [species_name]() { GetApp().GetUI().ZoomToSpecies(species_name); };
                        std::string species_label = boost::io::str(
                            FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(species_name));
                        popup->AddMenuItem(std::move(species_label), false, false, zoom_species_action);
                    }
                }
            }

            auto pedia_meter_type_action = [meter_type]() { GetApp().GetUI().ZoomToMeterTypeArticle(std::string{to_string(meter_type)}); };
            std::string popup_label = boost::io::str(
                FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(to_string(meter_type)));
            popup->AddMenuItem(std::move(popup_label), false, false, pedia_meter_type_action);

            popup->Run();
        });
        AttachChild(stat_icon);
        meters.emplace_back(meter_type, AssociatedMeterType(meter_type));
    }

    // attach and show meter bars and large resource indicators
    m_multi_icon_value_indicator =
        GG::Wnd::Create<MultiIconValueIndicator>(Width() - 2*EDGE_PAD, m_popcenter_id, meters);
    m_multi_meter_status_bar =
        GG::Wnd::Create<MultiMeterStatusBar>(Width() - 2*EDGE_PAD, m_popcenter_id, std::move(meters));

    // determine if this panel has been created yet. if not, default to collapsed state
    s_expanded_map.try_emplace(m_popcenter_id, false);

    Refresh();
}

void PopulationPanel::ExpandCollapse(bool expanded) {
    if (expanded != std::exchange(s_expanded_map[m_popcenter_id], expanded))
        DoLayout();
}

void PopulationPanel::Update(const ObjectMap& objects) {
    // remove any old browse wnds
    for (auto& [meter_name, stat_icon] : m_meter_stats) {
        stat_icon->ClearBrowseInfoWnd();
        m_multi_icon_value_indicator->ClearToolTip(meter_name);
    }

    auto pop = objects.getRaw<Planet>(m_popcenter_id);
    if (!pop) {
        ErrorLogger() << "PopulationPanel::Update couldn't get Planet";
        return;
    }

    // meter bar displays population stats
    if (m_multi_meter_status_bar)
        m_multi_meter_status_bar->Update(objects);
    if (m_multi_icon_value_indicator)
        m_multi_icon_value_indicator->Update(objects);

    // tooltips
    for (auto& [meter_name, stat_icon] : m_meter_stats) {
        const auto* meter = pop->GetMeter(meter_name);
        stat_icon->SetValue(meter->Initial());
        stat_icon->SetValue(meter->Current() - meter->Initial(), 1);

        auto browse_wnd = GG::Wnd::Create<MeterBrowseWnd>(m_popcenter_id, meter_name, AssociatedMeterType(meter_name));
        stat_icon->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(meter_name, browse_wnd);
    }
}

void PopulationPanel::Refresh() {
    for (auto& stat_icon : m_meter_stats | range_values)
        stat_icon->RequirePreRender();

    RequirePreRender();
}

void PopulationPanel::PreRender() {
    AccordionPanel::PreRender();
    Update(GetApp().GetContext().ContextObjects());
    DoLayout();
}

void PopulationPanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_popcenter_id]); }

void PopulationPanel::DoLayout() {
    AccordionPanel::DoLayout();

    for (auto& stat_icon : m_meter_stats | range_values)
        DetachChild(stat_icon);

    // detach / hide meter bars and large resource indicators
    DetachChild(m_multi_meter_status_bar);
    DetachChild(m_multi_icon_value_indicator);

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_popcenter_id]) {
        // position and reattach icons to be shown
        int n = 0;
        GG::X stride = MeterIconSize().x * 7/2;
        for (const auto& stat_icon : m_meter_stats | range_values) {
            GG::X x = n * stride;

            GG::Pt icon_ul(x, GG::Y0);
            GG::Pt icon_lr = icon_ul + MeterIconSize();
            stat_icon->SizeMove(icon_ul, icon_lr);

            if (x + stat_icon->MinUsableSize().x >= ClientWidth())
                break;

            AttachChild(stat_icon);
            stat_icon->Show();

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
