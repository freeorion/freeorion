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

std::map<int, bool> PopulationPanel::s_expanded_map = std::map<int, bool>();

PopulationPanel::PopulationPanel(GG::X w, int object_id) :
    AccordionPanel(w),
    m_popcenter_id(object_id),
    m_pop_stat(0),
    m_happiness_stat(0),
    m_multi_icon_value_indicator(0),
    m_multi_meter_status_bar(0)
{
    SetName("PopulationPanel");

    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_popcenter_id);
    if (!obj)
        throw std::invalid_argument("Attempted to construct a PopulationPanel with an invalid object id");
    TemporaryPtr<const PopCenter> pop = boost::dynamic_pointer_cast<const PopCenter>(obj);
    if (!pop)
        throw std::invalid_argument("Attempted to construct a PopulationPanel with an object id is not a PopCenter");

    GG::Connect(m_expand_button->LeftClickedSignal, &PopulationPanel::ExpandCollapseButtonPressed, this);

    m_pop_stat = new StatisticIcon(ClientUI::SpeciesIcon(pop->SpeciesName()), 0, 3, false);
    AttachChild(m_pop_stat);
    m_pop_stat->InstallEventFilter(this);

    m_happiness_stat = new StatisticIcon(ClientUI::MeterIcon(METER_HAPPINESS), 0, 3, false);
    AttachChild(m_happiness_stat);


    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType> > meters;
    meters.push_back(std::make_pair(METER_POPULATION,   METER_TARGET_POPULATION));
    meters.push_back(std::make_pair(METER_HAPPINESS,    METER_TARGET_HAPPINESS));

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
    delete m_pop_stat;
    delete m_happiness_stat;
    delete m_multi_icon_value_indicator;
    delete m_multi_meter_status_bar;
}

void PopulationPanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void PopulationPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    AccordionPanel::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoLayout();
}

void PopulationPanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_popcenter_id]); }

void PopulationPanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_popcenter_id]) return; // nothing to do
    s_expanded_map[m_popcenter_id] = expanded;

    DoLayout();
}

void PopulationPanel::DoLayout() {
    // initially detach most things.  Some will be reattached later.
    DetachChild(m_pop_stat);
    DetachChild(m_happiness_stat);

    // detach / hide meter bars and large resource indicators
    DetachChild(m_multi_meter_status_bar);
    DetachChild(m_multi_icon_value_indicator);


    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_popcenter_id]) {

        std::vector<StatisticIcon*> icons;
        icons.push_back(m_pop_stat);
        icons.push_back(m_happiness_stat);

        // position and reattach icons to be shown
        for (int n = 0; n < static_cast<int>(icons.size()); ++n) {
            GG::X x = MeterIconSize().x*n*7/2;

            if (x > Width() - m_expand_button->Width() - MeterIconSize().x*5/2) break;  // ensure icon doesn't extend past right edge of panel

            StatisticIcon* icon = icons[n];
            AttachChild(icon);
            GG::Pt icon_ul(x, GG::Y0);
            GG::Pt icon_lr = icon_ul + MeterIconSize();
            icon->SizeMove(icon_ul, icon_lr);
            icon->Show();
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

void PopulationPanel::Render() {
    // Draw outline and background...
    GG::FlatRectangle(UpperLeft(), LowerRight(), ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}

void PopulationPanel::Update() {
    // remove any old browse wnds
    m_pop_stat->ClearBrowseInfoWnd();
    m_multi_icon_value_indicator->ClearToolTip(METER_POPULATION);

    m_happiness_stat->ClearBrowseInfoWnd();
    m_multi_icon_value_indicator->ClearToolTip(METER_HAPPINESS);


    TemporaryPtr<const PopCenter>        pop = GetPopCenter();
    TemporaryPtr<const UniverseObject>   obj = GetUniverseObject(m_popcenter_id);

    if (!pop || !obj) {
        ErrorLogger() << "PopulationPanel::Update couldn't get PopCenter or couldn't get UniverseObject";
        return;
    }

    // meter bar displays and stat icons
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    m_pop_stat->SetValue(pop->InitialMeterValue(METER_POPULATION));
    m_happiness_stat->SetValue(pop->InitialMeterValue(METER_HAPPINESS));


    // create an attach browse info wnds for each meter type on the icon + number stats used when collapsed and
    // for all meter types shown in the multi icon value indicator.  this replaces any previous-present
    // browse wnd on these indicators
    boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd;

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_popcenter_id, METER_POPULATION, METER_TARGET_POPULATION));
    m_pop_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_POPULATION, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_popcenter_id, METER_HAPPINESS, METER_TARGET_HAPPINESS));
    m_happiness_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_HAPPINESS, browse_wnd);
}

void PopulationPanel::Refresh() {
    Update();
    DoLayout();
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

void PopulationPanel::EnableOrderIssuing(bool enable/* = true*/)
{}

bool PopulationPanel::EventFilter(GG::Wnd* w, const GG::WndEvent& event) {
    if (event.Type() != GG::WndEvent::RClick)
        return false;
    const GG::Pt& pt = event.Point();

    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_popcenter_id);
    if (!obj)
        return false;

    TemporaryPtr<const PopCenter> pc = boost::dynamic_pointer_cast<const PopCenter>(obj);
    if (!pc)
        return false;

    const std::string& species_name = pc->SpeciesName();
    if (species_name.empty())
        return false;

    if (m_pop_stat != w)
        return false;

    GG::MenuItem menu_contents;

    std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(species_name));
    menu_contents.next_level.push_back(GG::MenuItem(popup_label, 1, false, false));
    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                        ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());

    if (!popup.Run() || popup.MenuID() != 1)
        return false;

    ClientUI::GetClientUI()->ZoomToSpecies(species_name);
    return true;
}
