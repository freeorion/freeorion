#include "MultiIconValueIndicator.h"

#include <GG/Texture.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/PopCenter.h"
#include "../universe/UniverseObject.h"
#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"
#include "CUIControls.h"

namespace {
    const int       EDGE_PAD(3);

    const int       MULTI_INDICATOR_ICON_SPACING(12);
    const GG::X     MULTI_INDICATOR_ICON_WIDTH(24);
    const GG::Y     MULTI_INDICATOR_ICON_HEIGHT(24);
}

MultiIconValueIndicator::MultiIconValueIndicator(GG::X w, int object_id,
                                                 const std::vector<std::pair<MeterType, MeterType> >& meter_types) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y1, GG::INTERACTIVE),
    m_icons(),
    m_meter_types(meter_types),
    m_object_ids()
{
    m_object_ids.push_back(object_id);
    Init();
}

MultiIconValueIndicator::MultiIconValueIndicator(GG::X w, const std::vector<int>& object_ids,
                                                 const std::vector<std::pair<MeterType, MeterType> >& meter_types) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y1, GG::INTERACTIVE),
    m_icons(),
    m_meter_types(meter_types),
    m_object_ids(object_ids)
{ Init(); }

MultiIconValueIndicator::MultiIconValueIndicator(GG::X w) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y1, GG::INTERACTIVE),
    m_icons(),
    m_meter_types(),
    m_object_ids()
{ Init(); }

MultiIconValueIndicator::~MultiIconValueIndicator()
{}  // nothing needs deleting, as all contained indicators are childs and auto deleted

void MultiIconValueIndicator::Init() {
    SetName("MultiIconValueIndicator");

    GG::X x(EDGE_PAD);
    for (std::vector<std::pair<MeterType, MeterType> >::const_iterator it = m_meter_types.begin(); it != m_meter_types.end(); ++it) {
        const MeterType PRIMARY_METER_TYPE = it->first;
        // get icon texture.
        boost::shared_ptr<GG::Texture> texture = ClientUI::MeterIcon(PRIMARY_METER_TYPE);

        // special case for population meter for an indicator showing only a
        // single popcenter: icon is species icon, rather than generic pop icon
        if (PRIMARY_METER_TYPE == METER_POPULATION && m_object_ids.size() == 1) {
            if (TemporaryPtr<const UniverseObject> obj = GetUniverseObject(*m_object_ids.begin()))
                if (TemporaryPtr<const PopCenter> pc = boost::dynamic_pointer_cast<const PopCenter>(obj))
                    texture = ClientUI::SpeciesIcon(pc->SpeciesName());
        }

        m_icons.push_back(new StatisticIcon(texture, 0.0, 3, false));
        GG::Pt icon_ul(x, GG::Y(EDGE_PAD));
        GG::Pt icon_lr = icon_ul + GG::Pt(MULTI_INDICATOR_ICON_WIDTH, MULTI_INDICATOR_ICON_HEIGHT + ClientUI::Pts()*3/2);
        m_icons.back()->SizeMove(icon_ul, icon_lr);
        m_icons.back()->InstallEventFilter(this);
        AttachChild(m_icons.back());
        x += MULTI_INDICATOR_ICON_WIDTH + MULTI_INDICATOR_ICON_SPACING;
    }
    if (!m_icons.empty())
        Resize(GG::Pt(Width(), EDGE_PAD + MULTI_INDICATOR_ICON_HEIGHT + ClientUI::Pts()*3/2));
    Update();
}

bool MultiIconValueIndicator::Empty()
{ return m_object_ids.empty(); }

void MultiIconValueIndicator::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // outline of whole control
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}

void MultiIconValueIndicator::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void MultiIconValueIndicator::Update() {
    if (m_icons.size() != m_meter_types.size()) {
        ErrorLogger() << "MultiIconValueIndicator::Update has inconsitent numbers of icons and meter types";
        return;
    }

    for (std::size_t i = 0; i < m_icons.size(); ++i) {
        assert(m_icons[i]);
        double sum = 0.0;
        for (std::size_t j = 0; j < m_object_ids.size(); ++j) {
            int object_id = m_object_ids[j];
            TemporaryPtr<const UniverseObject> obj = GetUniverseObject(object_id);
            if (!obj) {
                ErrorLogger() << "MultiIconValueIndicator::Update couldn't get object with id " << object_id;
                continue;
            }
            //DebugLogger() << "MultiIconValueIndicator::Update object:";
            //DebugLogger() << obj->Dump();
            sum += obj->InitialMeterValue(m_meter_types[i].first);
        }
        m_icons[i]->SetValue(sum);
    }
}

void MultiIconValueIndicator::SetToolTip(MeterType meter_type, const boost::shared_ptr<GG::BrowseInfoWnd>& browse_wnd) {
    for (unsigned int i = 0; i < m_icons.size(); ++i)
        if (m_meter_types.at(i).first == meter_type)
            m_icons.at(i)->SetBrowseInfoWnd(browse_wnd);
}

void MultiIconValueIndicator::ClearToolTip(MeterType meter_type) {
    for (unsigned int i = 0; i < m_icons.size(); ++i)
        if (m_meter_types.at(i).first == meter_type)
            m_icons.at(i)->ClearBrowseInfoWnd();
}

bool MultiIconValueIndicator::EventFilter(GG::Wnd* w, const GG::WndEvent& event) {
    if (event.Type() != GG::WndEvent::RClick)
        return false;
    const GG::Pt& pt = event.Point();

    if (m_object_ids.size() != 1)
        return false;
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(*m_object_ids.begin());
    if (!obj)
        return false;

    TemporaryPtr<const PopCenter> pc = boost::dynamic_pointer_cast<const PopCenter>(obj);
    if (!pc)
        return false;

    const std::string& species_name = pc->SpeciesName();
    if (species_name.empty())
        return false;

    for (unsigned int i = 0; i < m_icons.size(); ++i) {
        if (m_icons.at(i) != w)
            continue;
        MeterType meter_type = m_meter_types.at(i).first;
        if (meter_type != METER_POPULATION)
            continue;

        GG::MenuItem menu_contents;

        std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(species_name));
        menu_contents.next_level.push_back(GG::MenuItem(popup_label, 1, false, false));
        GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                            ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());

        if (!popup.Run() || popup.MenuID() != 1) {
            return false;
            break;
        }

        ClientUI::GetClientUI()->ZoomToSpecies(species_name);
        return true;
    }

    return false;
}
