#include "PopulationPanel.h"

#include <GG/Button.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/UniverseObject.h"
#include "../universe/PopCenter.h"
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
    m_multi_icon_value_indicator(0),
    m_multi_meter_status_bar(0)
{
    SetName("PopulationPanel");

    TemporaryPtr<const PopCenter> pop = GetPopCenter();
    if (!pop)
        throw std::invalid_argument("Attempted to construct a PopulationPanel with an object id is not a PopCenter");

    GG::Connect(m_expand_button->LeftClickedSignal, &PopulationPanel::ExpandCollapseButtonPressed, this);

    // small meter indicators - for use when panel is collapsed
    m_meter_stats.push_back(
        std::make_pair(METER_POPULATION, new StatisticIcon(ClientUI::SpeciesIcon(pop->SpeciesName()), 0, 3, false,
                                                           GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y)));
    m_meter_stats.push_back(
        std::make_pair(METER_HAPPINESS, new StatisticIcon(ClientUI::MeterIcon(METER_HAPPINESS), 0, 3, false,
                                                          GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y)));
    m_meter_stats.push_back(
        std::make_pair(METER_CONSTRUCTION, new StatisticIcon(ClientUI::MeterIcon(METER_CONSTRUCTION), 0, 3, false,
                                                             GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y)));

    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType> > meters;

    for (std::vector<std::pair<MeterType, StatisticIcon*> >::iterator it = m_meter_stats.begin(); it != m_meter_stats.end(); ++it) {
        it->second->InstallEventFilter(this);
        AttachChild(it->second);
        meters.push_back(std::make_pair(it->first, AssociatedMeterType(it->first)));
    }

    // attach and show meter bars and large resource indicators
    m_multi_icon_value_indicator =  new MultiIconValueIndicator(Width() - 2*EDGE_PAD,   m_popcenter_id, meters);
    m_multi_meter_status_bar =      new MultiMeterStatusBar(Width() - 2*EDGE_PAD,       m_popcenter_id, meters);

    // determine if this panel has been created yet.
    std::map<int, bool>::iterator it = s_expanded_map.find(m_popcenter_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_popcenter_id] = false; // if not, default to collapsed state

    Refresh();
}

PopulationPanel::~PopulationPanel() {
    // manually delete all pointed-to controls that may or may not be attached as a child window at time of deletion
    delete m_multi_icon_value_indicator;
    delete m_multi_meter_status_bar;

    for (std::vector<std::pair<MeterType, StatisticIcon*> >::iterator it = m_meter_stats.begin(); it != m_meter_stats.end(); ++it) {
        delete it->second;
    }
}

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
    for (std::vector<std::pair<MeterType, StatisticIcon*> >::iterator stat_it = m_meter_stats.begin();
         stat_it != m_meter_stats.end(); ++stat_it)
    {
        if ((*stat_it).second == w) {
            meter_type = (*stat_it).first;
            break;
        }
    }
    if (meter_type == INVALID_METER_TYPE)
        return false;

    std::string meter_string = EnumToString(meter_type);
    std::string meter_title;
    if (UserStringExists(meter_string))
        meter_title = UserString(meter_string);

    GG::MenuItem menu_contents;
    std::string species_name;

    TemporaryPtr<const PopCenter> pc = GetPopCenter();
    if (meter_type == METER_POPULATION && pc) {
        species_name = pc->SpeciesName();
        if (!species_name.empty()) {
            std::string species_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(species_name));
            menu_contents.next_level.push_back(GG::MenuItem(species_label, 1, false, false));
        }
    }

    if (!meter_title.empty()) {
        std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % meter_title);
        menu_contents.next_level.push_back(GG::MenuItem(popup_label, 2, false, false));
    }

    CUIPopupMenu popup(pt.x, pt.y, menu_contents);

    bool retval = false;

    if (popup.Run()) {
        switch (popup.MenuID()) {
            case 1: {
                retval = ClientUI::GetClientUI()->ZoomToSpecies(species_name);
                break;
            }
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

void PopulationPanel::Update() {
    // remove any old browse wnds
    for (std::vector<std::pair<MeterType, StatisticIcon*> >::iterator it = m_meter_stats.begin(); it != m_meter_stats.end(); ++it) {
        it->second->ClearBrowseInfoWnd();
        m_multi_icon_value_indicator->ClearToolTip(it->first);
    }

    TemporaryPtr<const PopCenter> pop = GetPopCenter();
    if (!pop) {
        ErrorLogger() << "PopulationPanel::Update couldn't get PopCenter or couldn't get UniverseObject";
        return;
    }

    // meter bar displays population stats
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    // tooltips
    boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd;

    for (std::vector<std::pair<MeterType, StatisticIcon*> >::iterator it = m_meter_stats.begin(); it != m_meter_stats.end(); ++it) {
        it->second->SetValue(pop->InitialMeterValue(it->first));

        browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_popcenter_id, it->first, AssociatedMeterType(it->first)));
        it->second->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(it->first, browse_wnd);
    }
}

void PopulationPanel::Refresh() {
    Update();
    DoLayout();
}

void PopulationPanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_popcenter_id]); }

void PopulationPanel::DoLayout() {
    AccordionPanel::DoLayout();

    for (std::vector<std::pair<MeterType, StatisticIcon*> >::iterator it = m_meter_stats.begin(); it != m_meter_stats.end(); ++it) {
        DetachChild(it->second);
    }

    // detach / hide meter bars and large resource indicators
    DetachChild(m_multi_meter_status_bar);
    DetachChild(m_multi_icon_value_indicator);

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_popcenter_id]) {
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

    SetCollapsed(!s_expanded_map[m_popcenter_id]);
}

TemporaryPtr<const PopCenter> PopulationPanel::GetPopCenter() const {
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_popcenter_id);
    if (!obj) {
        ErrorLogger() << "PopulationPanel tried to get an object with an invalid m_popcenter_id";
        return TemporaryPtr<const PopCenter>();
    }
    TemporaryPtr<const PopCenter> pop = boost::dynamic_pointer_cast<const PopCenter>(obj);
    if (!pop) {
        ErrorLogger() << "PopulationPanel failed casting an object pointer to a PopCenter pointer";
        return TemporaryPtr<const PopCenter>();
    }
    return pop;
}

std::map<int, bool> PopulationPanel::s_expanded_map;
