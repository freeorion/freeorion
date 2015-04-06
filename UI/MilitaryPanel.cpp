#include "MilitaryPanel.h"

#include <GG/Button.h>

#include "../util/Logger.h"
#include "../universe/UniverseObject.h"
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

std::map<int, bool> MilitaryPanel::s_expanded_map;

MilitaryPanel::MilitaryPanel(GG::X w, int planet_id) :
    AccordionPanel(w),
    m_planet_id(planet_id),
    m_fleet_supply_stat(0),
    m_shield_stat(0),
    m_defense_stat(0),
    m_troops_stat(0),
    m_detection_stat(0),
    m_stealth_stat(0),
    m_multi_icon_value_indicator(0),
    m_multi_meter_status_bar(0)
{
    SetName("MilitaryPanel");

    GG::Connect(m_expand_button->LeftClickedSignal, &MilitaryPanel::ExpandCollapseButtonPressed, this);

    // small meter indicators - for use when panel is collapsed
    m_fleet_supply_stat = new StatisticIcon(ClientUI::MeterIcon(METER_SUPPLY), 0, 3, false);
    AttachChild(m_fleet_supply_stat);

    m_shield_stat = new StatisticIcon(ClientUI::MeterIcon(METER_SHIELD), 0, 3, false);
    AttachChild(m_shield_stat);

    m_defense_stat = new StatisticIcon(ClientUI::MeterIcon(METER_DEFENSE), 0, 3, false);
    AttachChild(m_defense_stat);

    m_troops_stat = new StatisticIcon(ClientUI::MeterIcon(METER_TROOPS), 0, 3, false);
    AttachChild(m_troops_stat);

    m_detection_stat = new StatisticIcon(ClientUI::MeterIcon(METER_DETECTION), 0, 3, false);
    AttachChild(m_detection_stat);

    m_stealth_stat = new StatisticIcon(ClientUI::MeterIcon(METER_STEALTH), 0, 3, false);
    AttachChild(m_stealth_stat);


    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType> > meters;
    meters.push_back(std::make_pair(METER_SHIELD, METER_MAX_SHIELD));
    meters.push_back(std::make_pair(METER_DEFENSE, METER_MAX_DEFENSE));
    meters.push_back(std::make_pair(METER_TROOPS, METER_MAX_TROOPS));
    meters.push_back(std::make_pair(METER_DETECTION, INVALID_METER_TYPE));
    meters.push_back(std::make_pair(METER_STEALTH, INVALID_METER_TYPE));
    meters.push_back(std::make_pair(METER_SUPPLY, METER_MAX_SUPPLY));


    m_multi_meter_status_bar =      new MultiMeterStatusBar(Width() - 2*EDGE_PAD,       m_planet_id, meters);
    m_multi_icon_value_indicator =  new MultiIconValueIndicator(Width() - 2*EDGE_PAD,   m_planet_id, meters);

    // determine if this panel has been created yet.
    std::map<int, bool>::iterator it = s_expanded_map.find(m_planet_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_planet_id] = false; // if not, default to collapsed state

    Refresh();
}

MilitaryPanel::~MilitaryPanel() {
    // manually delete all pointed-to controls that may or may not be attached as a child window at time of deletion
    delete m_fleet_supply_stat;
    delete m_shield_stat;
    delete m_defense_stat;
    delete m_troops_stat;
    delete m_detection_stat;
    delete m_stealth_stat;

    delete m_multi_icon_value_indicator;
    delete m_multi_meter_status_bar;
}

void MilitaryPanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_planet_id]) return; // nothing to do
    s_expanded_map[m_planet_id] = expanded;

    DoLayout();
}

void MilitaryPanel::Render() {
    if (Height() < 1) return;   // don't render if empty

    // Draw outline and background...
    GG::FlatRectangle(UpperLeft(), LowerRight(), ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}

void MilitaryPanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void MilitaryPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    AccordionPanel::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoLayout();
}

void MilitaryPanel::Update() {
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_planet_id);
    if (!obj) {
        ErrorLogger() << "MilitaryPanel::Update coudln't get object with id  " << m_planet_id;
        return;
    }


    // meter bar displays and production stats
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    m_fleet_supply_stat->SetValue(obj->InitialMeterValue(METER_SUPPLY));
    m_shield_stat->SetValue(obj->InitialMeterValue(METER_SHIELD));
    m_defense_stat->SetValue(obj->InitialMeterValue(METER_DEFENSE));
    m_troops_stat->SetValue(obj->InitialMeterValue(METER_TROOPS));
    m_detection_stat->SetValue(obj->InitialMeterValue(METER_DETECTION));
    m_stealth_stat->SetValue(obj->InitialMeterValue(METER_STEALTH));

    // tooltips
    boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, METER_SUPPLY, METER_MAX_SUPPLY));
    m_fleet_supply_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_SUPPLY, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, METER_SHIELD, METER_MAX_SHIELD));
    m_shield_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_SHIELD, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, METER_DEFENSE, METER_MAX_DEFENSE));
    m_defense_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_DEFENSE, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, METER_TROOPS, METER_MAX_TROOPS));
    m_troops_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_TROOPS, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, METER_DETECTION));
    m_detection_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_DETECTION, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, METER_STEALTH));
    m_stealth_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_STEALTH, browse_wnd);
}

void MilitaryPanel::Refresh() {
    Update();
    DoLayout();
}

void MilitaryPanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_planet_id]); }

void MilitaryPanel::DoLayout() {
    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_planet_id]) {

        // detach / hide meter bars and large resource indicators
        DetachChild(m_multi_meter_status_bar);
        DetachChild(m_multi_icon_value_indicator);


        // determine which two resource icons to display while collapsed: the two with the highest production

        // sort by insereting into multimap keyed by production amount, then taking the first two icons therein
        std::vector<StatisticIcon*> meter_icons;
        meter_icons.push_back(m_shield_stat);
        meter_icons.push_back(m_defense_stat);
        meter_icons.push_back(m_troops_stat);
        meter_icons.push_back(m_detection_stat);
        meter_icons.push_back(m_stealth_stat);
        meter_icons.push_back(m_fleet_supply_stat);

        // initially detach all...
        for (std::vector<StatisticIcon*>::iterator it = meter_icons.begin(); it != meter_icons.end(); ++it)
            DetachChild(*it);

        // position and reattach icons to be shown
        int n = 0;
        for (std::vector<StatisticIcon*>::iterator it = meter_icons.begin(); it != meter_icons.end(); ++it) {
            GG::X x = MeterIconSize().x*n*7/2;

            if (x > Width() - m_expand_button->Width() - MeterIconSize().x*5/2) break;  // ensure icon doesn't extend past right edge of panel

            StatisticIcon* icon = *it;
            AttachChild(icon);
            GG::Pt icon_ul(x, GG::Y0);
            GG::Pt icon_lr = icon_ul + MeterIconSize();
            icon->SizeMove(icon_ul, icon_lr);
            icon->Show();

            n++;
        }

        Resize(GG::Pt(Width(), std::max(MeterIconSize().y, m_expand_button->Height())));
    } else {
        // detach statistic icons
        DetachChild(m_shield_stat);     DetachChild(m_defense_stat);    DetachChild(m_troops_stat);
        DetachChild(m_detection_stat);  DetachChild(m_stealth_stat);    DetachChild(m_fleet_supply_stat);

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

    m_expand_button->MoveTo(GG::Pt(Width() - m_expand_button->Width(), GG::Y0));

    SetCollapsed(!s_expanded_map[m_planet_id]);
}

void MilitaryPanel::EnableOrderIssuing(bool enable/* = true*/)
{}
