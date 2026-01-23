#include "ResourcePanel.h"

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

ResourcePanel::ResourcePanel(GG::X w, int object_id) :
    AccordionPanel(w, GG::Y(ClientUI::Pts()*2)),
    m_rescenter_id(object_id)
{}

void ResourcePanel::CompleteConstruction() {
    AccordionPanel::CompleteConstruction();

    SetName("ResourcePanel");

    auto& app = GetApp();
    auto obj = app.GetContext().ContextObjects().get(m_rescenter_id);
    if (!obj) {
        ErrorLogger() << "ResourcePanel::CompleteConstruction couldn't get object with id  " << m_rescenter_id;
        return;
    }

    m_expand_button->LeftPressedSignal.connect(boost::bind(&ResourcePanel::ExpandCollapseButtonPressed, this));

    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType>> meters;
    meters.reserve(5);

    // small meter indicators - for use when panel is collapsed
    for (MeterType meter : {MeterType::METER_INDUSTRY, MeterType::METER_RESEARCH,
                            MeterType::METER_INFLUENCE, MeterType::METER_SUPPLY,
                            MeterType::METER_STOCKPILE})
    {
        const auto* p_meter = obj->GetMeter(meter);
        if (!p_meter) {
            ErrorLogger() << "ResourcePanel constructed with object " << obj->Dump()
                          << " with no " << to_string(meter) << " meter";
            continue;
        }
        const auto assoc_meter = AssociatedMeterType(meter);
        const auto* p_assoc_meter = obj->GetMeter(assoc_meter);
        if (!p_assoc_meter) {
            ErrorLogger() << "ResourcePanel constructed with object " << obj->Dump()
                          << " with no " << to_string(assoc_meter) << " meter";
            continue;
        }

        auto stat = GG::Wnd::Create<StatisticIcon>(
            app.GetUI().MeterIcon(meter), obj->GetMeter(meter)->Initial(),
            3, false, MeterIconSize().x, MeterIconSize().y);
        AttachChild(stat);
        m_meter_stats.emplace_back(meter, stat);
        meters.emplace_back(meter, assoc_meter);
        stat->RightClickedSignal.connect([meter](GG::Pt pt) {
            auto meter_string = to_string(meter);

            auto pedia_zoom_to_article_action = [meter_string]() {
                GetApp().GetUI().ZoomToMeterTypeArticle(std::string{meter_string}); };

            auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
            std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) %
                                                                    UserString(meter_string));
            popup->AddMenuItem(GG::MenuItem(std::move(popup_label), false, false,
                                            pedia_zoom_to_article_action));
            popup->Run();
        });
    }

    // attach and show meter bars and large resource indicators
    m_multi_meter_status_bar =
        GG::Wnd::Create<MultiMeterStatusBar>(Width() - 2*EDGE_PAD, m_rescenter_id, meters);
    m_multi_icon_value_indicator =
        GG::Wnd::Create<MultiIconValueIndicator>(Width() - 2*EDGE_PAD, m_rescenter_id, std::move(meters));

    // if this panel has been created yet, default to collapsed state
    s_expanded_map.try_emplace(m_rescenter_id, false);

    Refresh();
}

void ResourcePanel::ExpandCollapse(bool expanded) {
    if (expanded != std::exchange(s_expanded_map[m_rescenter_id], expanded))
        RequirePreRender();
}



namespace {
    bool SortByMeterValue(std::pair<MeterType, std::shared_ptr<StatisticIcon>> left,
                          std::pair<MeterType, std::shared_ptr<StatisticIcon>> right)
    {
        // TODO: pass by ref and avoid repeated ->GetValue()
        if (std::abs(left.second->GetValue()) == std::abs(right.second->GetValue())) {
            if (left.first == MeterType::METER_INFLUENCE && right.first == MeterType::METER_CONSTRUCTION) {
                // swap order of MeterType::METER_INFLUENCE and MeterType::METER_CONSTRUCTION in relation to
                // MeterType enum.
                return false;
            }

            return left.first < right.first;
        }

        return std::abs(left.second->GetValue()) > std::abs(right.second->GetValue());
    }
}

void ResourcePanel::Update(const ObjectMap& objects) {
    // remove any old browse wnds
    for (auto& [meter_type, stat_icon] : m_meter_stats) {
        stat_icon->ClearBrowseInfoWnd();
        m_multi_icon_value_indicator->ClearToolTip(meter_type);
    }

    auto obj = objects.get(m_rescenter_id);
    if (!obj) {
        ErrorLogger() << "BuildingPanel::Update couldn't get object with id " << m_rescenter_id;
        return;
    }

    // meter bar displays resource stats
    m_multi_meter_status_bar->Update(objects);
    m_multi_icon_value_indicator->Update(objects);

    // tooltips
    for (auto& [meter_type, stat_icon] : m_meter_stats) {
        if (!stat_icon)
            continue;
        auto meter = obj->GetMeter(meter_type);
        auto init = meter ? meter->Initial() : 0.0f;
        stat_icon->SetValue(init);

        auto assoc_meter_type = AssociatedMeterType(meter_type);
        auto browse_wnd = GG::Wnd::Create<MeterBrowseWnd>(m_rescenter_id, meter_type, assoc_meter_type);
        stat_icon->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(meter_type, browse_wnd);
    }

    std::sort(m_meter_stats.begin(), m_meter_stats.end(), SortByMeterValue);
}

void ResourcePanel::Refresh() {
    for (auto& stat_icon: m_meter_stats | range_values)
        stat_icon->RequirePreRender();

    RequirePreRender();
}

void ResourcePanel::PreRender() {
    AccordionPanel::PreRender();
    Update(GetApp().GetContext().ContextObjects());
    DoLayout();
}

void ResourcePanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_rescenter_id]); }

void ResourcePanel::DoLayout() {
    AccordionPanel::DoLayout();

    for (auto& stat : m_meter_stats | range_values)
        DetachChild(stat);

    // detach / hide meter bars and large resource indicators
    DetachChild(m_multi_meter_status_bar);
    DetachChild(m_multi_icon_value_indicator);

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_rescenter_id]) {
        // position and reattach icons to be shown
        int n = 0;
        GG::X stride = MeterIconSize().x * 7/2;
        for (auto& icon : m_meter_stats | range_values) {
            GG::X x = n * stride;

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

    SetCollapsed(!s_expanded_map[m_rescenter_id]);
}

std::map<int, bool> ResourcePanel::s_expanded_map;
