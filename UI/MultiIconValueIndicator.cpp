#include "MultiIconValueIndicator.h"

#include <GG/Texture.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/UniverseObject.h"
#include "../universe/Planet.h"
#include "../universe/Enums.h"
#include "../client/human/GGHumanClientApp.h"
#include "ClientUI.h"
#include "CUIControls.h"

namespace {
    constexpr int EDGE_PAD(3);

    int IconSpacing() { return ClientUI::Pts(); }
    GG::X IconWidth() { return GG::X(IconSpacing()*2); }
    GG::Y IconHeight() { return GG::Y(IconSpacing()*2); }
}

MultiIconValueIndicator::MultiIconValueIndicator(GG::X w) :
    MultiIconValueIndicator(w, {}, {})
{}

MultiIconValueIndicator::MultiIconValueIndicator(
    GG::X w, int object_id,
    std::vector<std::pair<MeterType, MeterType>> meter_types) :
    MultiIconValueIndicator(w, std::vector<int>{object_id}, std::move(meter_types))
{}

MultiIconValueIndicator::MultiIconValueIndicator(
    GG::X w, const std::vector<int>& object_ids,
    std::vector<std::pair<MeterType, MeterType>> meter_types) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y1, GG::INTERACTIVE),
    m_meter_types(std::move(meter_types)),
    m_object_ids(object_ids)
{}

void MultiIconValueIndicator::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    SetName("MultiIconValueIndicator");

    GG::X x{EDGE_PAD};
    for (const auto& meter_type : m_meter_types) {
        const MeterType PRIMARY_METER_TYPE = meter_type.first;
        // get icon texture.
        auto texture = ClientUI::MeterIcon(PRIMARY_METER_TYPE);

        // special case for population meter for an indicator showing only a
        // single popcenter: icon is species icon, rather than generic pop icon
        if (PRIMARY_METER_TYPE == MeterType::METER_POPULATION && m_object_ids.size() == 1) {
            if (auto pc = Objects().get<Planet>(m_object_ids.front()))
                texture = ClientUI::SpeciesIcon(pc->SpeciesName());
        }

        m_icons.push_back(GG::Wnd::Create<StatisticIcon>(std::move(texture), 0.0, 3, false,
                                                         IconWidth(), IconHeight()));
        const GG::Pt icon_ul(x, GG::Y(EDGE_PAD));
        const GG::Pt icon_lr = icon_ul + GG::Pt(IconWidth(), IconHeight() + ClientUI::Pts()*3/2);
        m_icons.back()->SizeMove(icon_ul, icon_lr);
        const auto meter = meter_type.first;
        const auto meter_string = to_string(meter_type.first);
        m_icons.back()->RightClickedSignal.connect([this, meter, meter_string](GG::Pt pt) {
            auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

            auto pc = Objects().get<Planet>(this->m_object_ids.front());
            if (meter == MeterType::METER_POPULATION && pc && this->m_object_ids.size() == 1) {
                auto species_name = pc->SpeciesName();  // intentionally making a copy for use in lambda
                if (!species_name.empty()) {
                    auto zoom_species_action = [species_name]() { ClientUI::GetClientUI()->ZoomToSpecies(species_name); };
                    std::string species_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) %
                                                                              UserString(species_name));
                    popup->AddMenuItem(GG::MenuItem(std::move(species_label), false, false,
                                                    zoom_species_action));
                }
            }


            auto zoom_article_action = [meter_string]() { ClientUI::GetClientUI()->ZoomToMeterTypeArticle(std::string{meter_string});}; // TODO: avoid copy
            std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) %
                                                                    UserString(meter_string));
            popup->AddMenuItem(GG::MenuItem(std::move(popup_label), false, false,
                                            zoom_article_action));
            popup->Run();
        });
        AttachChild(m_icons.back());
        x += IconWidth() + IconSpacing();
    }
    if (!m_icons.empty())
        Resize(GG::Pt(Width(), EDGE_PAD + IconHeight() + ClientUI::Pts()*3/2));
    Update();
}

bool MultiIconValueIndicator::Empty() const
{ return m_object_ids.empty(); }

void MultiIconValueIndicator::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // outline of whole control
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}

void MultiIconValueIndicator::MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void MultiIconValueIndicator::Update() {
    if (m_icons.size() != m_meter_types.size()) {
        ErrorLogger() << "MultiIconValueIndicator::Update has inconsitent numbers of icons and meter types";
        return;
    }

    for (std::size_t i = 0; i < m_icons.size(); ++i) {
        assert(m_icons[i]);
        double total = 0.0;
        for (const auto& obj : Objects().find<UniverseObject>(m_object_ids)) {
            if (!obj)
                continue;
            auto type = m_meter_types[i].first;
            const auto* meter{obj->GetMeter(type)};
            if (!meter) {
                ErrorLogger() << "MultiIconValueIndicator::Update couldn't get meter type " << type << " for object " << obj->Name() << " (" << obj->ID() << ")";
                continue;
            }
            double value = obj->GetMeter(type)->Initial();
            // Supply is a special case: the only thing that matters is the highest value.
            if (type == MeterType::METER_SUPPLY)
                total = std::max(total, value);
            else
                total += value;
        }
        m_icons[i]->SetValue(total);
    }
}

void MultiIconValueIndicator::SetToolTip(MeterType meter_type, const std::shared_ptr<GG::BrowseInfoWnd>& browse_wnd) {
    for (unsigned int i = 0; i < m_icons.size(); ++i)
        if (m_meter_types.at(i).first == meter_type)
            m_icons.at(i)->SetBrowseInfoWnd(browse_wnd);
}

void MultiIconValueIndicator::ClearToolTip(MeterType meter_type) {
    for (unsigned int i = 0; i < m_icons.size(); ++i)
        if (m_meter_types.at(i).first == meter_type)
            m_icons.at(i)->ClearBrowseInfoWnd();
}
