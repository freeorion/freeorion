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

#include <numeric>

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

MultiIconValueIndicator::MultiIconValueIndicator(GG::X w, std::vector<int> object_ids,
                                                 std::vector<std::pair<MeterType, MeterType>> meter_types) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y1, GG::INTERACTIVE),
    m_meter_types(std::move(meter_types)),
    m_object_ids(std::move(object_ids))
{}

void MultiIconValueIndicator::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    SetName("MultiIconValueIndicator");

    auto& app = GetApp();
    const auto& objects = app.GetContext().ContextObjects();
    auto& ui = app.GetUI();

    GG::X x{EDGE_PAD};
    for (const auto primary_meter_type : m_meter_types | range_keys) {
        {
            // get icon texture.
            auto texture = ui.MeterIcon(primary_meter_type);

            // special case for population meter for an indicator showing only a
            // single popcenter: icon is species icon, rather than generic pop icon
            if (primary_meter_type == MeterType::METER_POPULATION && m_object_ids.size() == 1) {
                if (auto pc = objects.get<Planet>(m_object_ids.front()))
                    texture = ui.SpeciesIcon(pc->SpeciesName());
            }

            m_icons.push_back(GG::Wnd::Create<StatisticIcon>(std::move(texture), 0.0, 3, false,
                                                             IconWidth(), IconHeight()));
        }
        const GG::Pt icon_ul(x, GG::Y(EDGE_PAD));
        const GG::Pt icon_lr = icon_ul + GG::Pt(IconWidth(), IconHeight() + ClientUI::Pts()*3/2);
        m_icons.back()->SizeMove(icon_ul, icon_lr);

        m_icons.back()->RightClickedSignal.connect([this, primary_meter_type](GG::Pt pt) {

            const auto meter_string = to_string(primary_meter_type);
            auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

            if (primary_meter_type == MeterType::METER_POPULATION && this->m_object_ids.size() == 1) {
                const auto& objects = GetApp().GetContext().ContextObjects();
                if (auto planet = objects.get<Planet>(this->m_object_ids.front())) {
                    auto species_name{planet->SpeciesName()}; // intentional copy for use in lambda
                    if (species_name.empty()) {
                        std::string species_label =
                            boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(species_name));
                        auto zoom_species_action = [species_name{std::move(species_name)}]()
                        { GetApp().GetUI().ZoomToSpecies(species_name); };
                        popup->AddMenuItem(GG::MenuItem(std::move(species_label), false, false, zoom_species_action));
                    }
                }
            }


            auto zoom_article_action = [meter_string]() { GetApp().GetUI().ZoomToMeterTypeArticle(std::string{meter_string});}; // TODO: avoid copy
            std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) %
                                                                    UserString(meter_string));
            popup->AddMenuItem(GG::MenuItem(std::move(popup_label), false, false, zoom_article_action));
            popup->Run();
        });
        AttachChild(m_icons.back());
        x += IconWidth() + IconSpacing();
    }

    if (!m_icons.empty())
        Resize(GG::Pt(Width(), EDGE_PAD + IconHeight() + ClientUI::Pts()*3/2));

    Update(objects);
}

void MultiIconValueIndicator::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // outline of whole control
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}

void MultiIconValueIndicator::MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void MultiIconValueIndicator::Update(const ObjectMap& objects) {
    if (m_icons.size() != m_meter_types.size()) {
        ErrorLogger() << "MultiIconValueIndicator::Update has inconsitent numbers of icons and meter types";
        return;
    }

    const auto objs = objects.findRaw<UniverseObject>(m_object_ids);
    if (objs.empty()) {
        for (auto& icon : m_icons)
            if (icon) icon->SetValue(0);
        return;
    }


    for (std::size_t i = 0; i < m_icons.size(); ++i) {
        auto& icon = m_icons[i];
        if (!icon) continue;
        const auto type = m_meter_types[i].first;

        const auto to_value = [type](const auto* obj) -> float {
            if (!obj) return 0.0f;
            const auto* meter = obj->GetMeter(type);
            if (!meter) return 0.0f;
            return meter->Initial();
        };

        auto values = objs | range_transform(to_value);
        if (type == MeterType::METER_SUPPLY) {
            auto max_it = range_max_element(values);
            auto value = (max_it == values.end()) ? 0.0 : *max_it;
            icon->SetValue(value);
        } else {
            auto value = std::accumulate(values.begin(), values.end(), 0.0);
            icon->SetValue(value);
        }
    }
}

void MultiIconValueIndicator::SetToolTip(MeterType meter_type,
                                         const std::shared_ptr<GG::BrowseInfoWnd>& browse_wnd)
{
    for (unsigned int i = 0; i < m_icons.size(); ++i)
        if (m_meter_types.at(i).first == meter_type)
            m_icons.at(i)->SetBrowseInfoWnd(browse_wnd);
}

void MultiIconValueIndicator::ClearToolTip(MeterType meter_type) {
    for (unsigned int i = 0; i < m_icons.size(); ++i)
        if (m_meter_types.at(i).first == meter_type)
            m_icons.at(i)->ClearBrowseInfoWnd();
}
