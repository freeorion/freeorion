#include "MilitaryPanel.h"

#include <GG/Button.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/UniverseObject.h"
#include "../universe/Planet.h"
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
    m_multi_icon_value_indicator(0),
    m_multi_meter_status_bar(0)
{
    SetName("MilitaryPanel");

    TemporaryPtr<const Planet> planet = GetPlanet();
    if (!planet)
        throw std::invalid_argument("Attempted to construct a MilitaryPanel with an object id is not a Planet");

    GG::Connect(m_expand_button->LeftClickedSignal, &MilitaryPanel::ExpandCollapseButtonPressed, this);

    // small meter indicators - for use when panel is collapsed
    m_meter_stats.push_back(
        std::make_pair(METER_SHIELD, new StatisticIcon(ClientUI::MeterIcon(METER_SHIELD), 0, 3, false,
                                                       GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y)));
    m_meter_stats.push_back(
        std::make_pair(METER_DEFENSE, new StatisticIcon(ClientUI::MeterIcon(METER_DEFENSE), 0, 3, false,
                                                        GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y)));
    m_meter_stats.push_back(
        std::make_pair(METER_TROOPS, new StatisticIcon(ClientUI::MeterIcon(METER_TROOPS), 0, 3, false,
                                                       GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y)));
    m_meter_stats.push_back(
        std::make_pair(METER_DETECTION, new StatisticIcon(ClientUI::MeterIcon(METER_DETECTION), 0, 3, false,
                                                          GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y)));
    m_meter_stats.push_back(
        std::make_pair(METER_STEALTH, new StatisticIcon(ClientUI::MeterIcon(METER_STEALTH), 0, 3, false,
                                                        GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y)));

    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType> > meters;

    for (std::pair<MeterType, StatisticIcon*>& meter_stat : m_meter_stats) {
        meter_stat.second->InstallEventFilter(this);
        AttachChild(meter_stat.second);
        meters.push_back(std::make_pair(meter_stat.first, AssociatedMeterType(meter_stat.first)));
    }

    // attach and show meter bars and large resource indicators
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
    delete m_multi_icon_value_indicator;
    delete m_multi_meter_status_bar;

    for (std::pair<MeterType, StatisticIcon*>& meter_stat : m_meter_stats) {
        delete meter_stat.second;
    }
}

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
    for (std::pair<MeterType, StatisticIcon*>& meter_stat : m_meter_stats) {
        if (meter_stat.second == w) {
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

    GG::MenuItem menu_contents;

    std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % meter_title);
    menu_contents.next_level.push_back(GG::MenuItem(popup_label, 2, false, false));

    CUIPopupMenu popup(pt.x, pt.y, menu_contents);

    bool retval = false;

    if (popup.Run()) {
        switch (popup.MenuID()) {
            case 2: {
                retval = ClientUI::GetClientUI()->ZoomToMeterTypeArticle(meter_string);
                break;
            }
            default:
                break;
        }
    }

    return retval;
}

void MilitaryPanel::Update() {
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_planet_id);
    if (!obj) {
        ErrorLogger() << "MilitaryPanel::Update coudln't get object with id  " << m_planet_id;
        return;
    }

    // meter bar displays military stats
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    // tooltips
    boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd;

    for (std::pair<MeterType, StatisticIcon*>& meter_stat : m_meter_stats) {
        meter_stat.second->SetValue(obj->InitialMeterValue(meter_stat.first));

        browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, meter_stat.first, AssociatedMeterType(meter_stat.first)));
        meter_stat.second->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(meter_stat.first, browse_wnd);
    }
}

void MilitaryPanel::Refresh() {
    Update();
    DoLayout();
}

void MilitaryPanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_planet_id]); }

void MilitaryPanel::DoLayout() {
    AccordionPanel::DoLayout();

    for (std::pair<MeterType, StatisticIcon*>& meter_stat : m_meter_stats) {
        DetachChild(meter_stat.second);
    }

    // detach / hide meter bars and large resource indicators
    DetachChild(m_multi_meter_status_bar);
    DetachChild(m_multi_icon_value_indicator);

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_planet_id]) {
        // position and reattach icons to be shown
        int n = 0;
        for (std::pair<MeterType, StatisticIcon*>& meter_stat : m_meter_stats) {
            GG::X x = MeterIconSize().x*n*7/2;

            if (x > Width() - m_expand_button->Width() - MeterIconSize().x*5/2) break;  // ensure icon doesn't extend past right edge of panel

            StatisticIcon* icon = meter_stat.second;
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

TemporaryPtr<const Planet> MilitaryPanel::GetPlanet() const {
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_planet_id);
    if (!obj) {
        ErrorLogger() << "MilitaryPanel tried to get an object with an invalid m_planet_id";
        return TemporaryPtr<const Planet>();
    }
    TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(obj);
    if (!planet) {
        ErrorLogger() << "MilitaryPanel failed casting an object pointer to a Planet pointer";
        return TemporaryPtr<const Planet>();
    }
    return planet;
}

std::map<int, bool> MilitaryPanel::s_expanded_map;
