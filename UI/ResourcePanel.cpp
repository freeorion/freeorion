#include "ResourcePanel.h"

#include <GG/Button.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/Effect.h"
#include "../universe/ResourceCenter.h"
#include "../universe/TemporaryPtr.h"
#include "../universe/UniverseObject.h"
#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "MeterBrowseWnd.h"
#include "MultiIconValueIndicator.h"
#include "MultiMeterStatusBar.h"

namespace {
    const int       EDGE_PAD(3);

    const GG::X     METER_BROWSE_LABEL_WIDTH(300);
    const GG::X     METER_BROWSE_VALUE_WIDTH(50);

    /** How big we want meter icons with respect to the current UI font size.
      * Meters should scale along font size, but not below the size for the
      * default 12 points font. */
    GG::Pt MeterIconSize() {
        const int icon_size = std::max(ClientUI::Pts(), 12) * 4/3;
        return GG::Pt(GG::X(icon_size), GG::Y(icon_size));
    }
}

ResourcePanel::ResourcePanel(GG::X w, int object_id) :
    AccordionPanel(w),
    m_rescenter_id(object_id),
    m_meter_stats(),
    m_multi_icon_value_indicator(0),
    m_multi_meter_status_bar(0)
{
    SetName("ResourcePanel");

    TemporaryPtr<const ResourceCenter> res = GetResCenter();
    if (!res)
        throw std::invalid_argument("Attempted to construct a ResourcePanel with an UniverseObject that is not a ResourceCenter");

    SetChildClippingMode(ClipToClient);

    GG::Connect(m_expand_button->LeftClickedSignal, &ResourcePanel::ExpandCollapseButtonPressed, this);

    // small meter indicators - for use when panel is collapsed
    m_meter_stats.push_back(std::make_pair(METER_INDUSTRY, new StatisticIcon(ClientUI::MeterIcon(METER_INDUSTRY), 0, 3, false)));
    m_meter_stats.push_back(std::make_pair(METER_RESEARCH, new StatisticIcon(ClientUI::MeterIcon(METER_RESEARCH), 0, 3, false)));
    m_meter_stats.push_back(std::make_pair(METER_CONSTRUCTION, new StatisticIcon(ClientUI::MeterIcon(METER_CONSTRUCTION), 0, 3, false)));
    m_meter_stats.push_back(std::make_pair(METER_TRADE, new StatisticIcon(ClientUI::MeterIcon(METER_TRADE), 0, 3, false)));

    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType> > meters;

    for (std::vector<std::pair<MeterType, StatisticIcon*> >::iterator it = m_meter_stats.begin(); it != m_meter_stats.end(); ++it) {
        AttachChild(it->second);
        meters.push_back(std::make_pair(it->first, AssociatedMeterType(it->first)));
    }

    // attach and show meter bars and large resource indicators
    m_multi_meter_status_bar =      new MultiMeterStatusBar(Width() - 2*EDGE_PAD,       m_rescenter_id, meters);
    m_multi_icon_value_indicator =  new MultiIconValueIndicator(Width() - 2*EDGE_PAD,   m_rescenter_id, meters);

    // determine if this panel has been created yet.
    std::map<int, bool>::iterator it = s_expanded_map.find(m_rescenter_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_rescenter_id] = false; // if not, default to collapsed state

    Refresh();
}

ResourcePanel::~ResourcePanel() {
    // manually delete all pointed-to controls that may or may not be attached as a child window at time of deletion
    delete m_multi_icon_value_indicator;
    delete m_multi_meter_status_bar;

    for (std::vector<std::pair<MeterType, StatisticIcon*> >::iterator it = m_meter_stats.begin(); it != m_meter_stats.end(); ++it) {
        delete it->second;
    }
}

void ResourcePanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_rescenter_id]) return; // nothing to do
    s_expanded_map[m_rescenter_id] = expanded;

    DoLayout();
}

namespace {
    bool sortByMeterValue(std::pair<MeterType, StatisticIcon*> left, std::pair<MeterType, StatisticIcon*> right) {
        if (left.second->GetValue() == right.second->GetValue()) {
            if (left.first == METER_TRADE && right.first == METER_CONSTRUCTION) {
                // swap order of METER_TRADE and METER_CONSTRUCTION in relation to
                // MeterType enum.
                return false;
            }

            return left.first < right.first;
        }

        return left.second->GetValue() > right.second->GetValue();
    }
}

void ResourcePanel::Update() {
    // remove any old browse wnds
    for (std::vector<std::pair<MeterType, StatisticIcon*> >::iterator it = m_meter_stats.begin(); it != m_meter_stats.end(); ++it) {
        it->second->ClearBrowseInfoWnd();
        m_multi_icon_value_indicator->ClearToolTip(it->first);
    }

    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_rescenter_id);
    if (!obj) {
        ErrorLogger() << "BuildingPanel::Update couldn't get object with id " << m_rescenter_id;
        return;
    }

    // meter bar displays resource stats
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    // tooltips
    boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd;

    for (std::vector<std::pair<MeterType, StatisticIcon*> >::iterator it = m_meter_stats.begin(); it != m_meter_stats.end(); ++it) {
        it->second->SetValue(obj->InitialMeterValue(it->first));

        browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_rescenter_id, it->first, AssociatedMeterType(it->first)));
        it->second->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(it->first, browse_wnd);
    }

    std::sort(m_meter_stats.begin(), m_meter_stats.end(), sortByMeterValue);
}

void ResourcePanel::Refresh() {
    Update();
    DoLayout();
}

void ResourcePanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_rescenter_id]); }

void ResourcePanel::DoLayout() {
    AccordionPanel::DoLayout();

    for (std::vector<std::pair<MeterType, StatisticIcon*> >::iterator it = m_meter_stats.begin(); it != m_meter_stats.end(); ++it) {
        DetachChild(it->second);
    }

    // detach / hide meter bars and large resource indicators
    DetachChild(m_multi_meter_status_bar);
    DetachChild(m_multi_icon_value_indicator);

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_rescenter_id]) {
        // position and reattach icons to be shown
        int n = 0;
        for (std::vector<std::pair<MeterType, StatisticIcon*> >::iterator it = m_meter_stats.begin(); it != m_meter_stats.end(); ++it) {
            GG::X x = MeterIconSize().x*n*7/2;

            if (x > Width() - m_expand_button->Width() - MeterIconSize().x*5/2) break;  // ensure icon doesn't extend past right edge of panel

            StatisticIcon* icon = it->second;
            AttachChild(icon);
            GG::Pt icon_ul(x, GG::Y0);
            GG::Pt icon_lr = icon_ul + MeterIconSize();
            icon->SizeMove(icon_ul, icon_lr);
            icon->Show();

            n++;
        }

        Resize(GG::Pt(Width(), std::max(MeterIconSize().y, m_expand_button->Height())));
    } else {
        // attach and show meter bars and large resource indicators
        GG::Y top = GG::Y0;

        AttachChild(m_multi_icon_value_indicator);
        m_multi_icon_value_indicator->MoveTo(GG::Pt(GG::X(EDGE_PAD), top));
        m_multi_icon_value_indicator->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_icon_value_indicator->Height()));
        top += m_multi_icon_value_indicator->Height() + EDGE_PAD;

        AttachChild(m_multi_meter_status_bar);
        m_multi_meter_status_bar->MoveTo(GG::Pt(GG::X(EDGE_PAD), top));
        m_multi_meter_status_bar->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_meter_status_bar->Height()));
        top += m_multi_icon_value_indicator->Height() + EDGE_PAD;

        MoveChildUp(m_expand_button);

        Resize(GG::Pt(Width(), top));
    }

    m_expand_button->MoveTo(GG::Pt(Width() - m_expand_button->Width(), GG::Y0));

    SetCollapsed(!s_expanded_map[m_rescenter_id]);
}

TemporaryPtr<const ResourceCenter> ResourcePanel::GetResCenter() const {
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_rescenter_id);
    if (!obj) {
        ErrorLogger() << "ResourcePanel tried to get an object with an invalid m_rescenter_id";
        return TemporaryPtr<const ResourceCenter>();
    }
    TemporaryPtr<const ResourceCenter> res = boost::dynamic_pointer_cast<const ResourceCenter>(obj);
    if (!res) {
        ErrorLogger() << "ResourcePanel failed casting an object pointer to a ResourceCenter pointer";
        return TemporaryPtr<const ResourceCenter>();
    }
    return res;
}

std::map<int, bool> ResourcePanel::s_expanded_map;
