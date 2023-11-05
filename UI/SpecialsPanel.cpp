#include "SpecialsPanel.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../universe/Effect.h"
#include "../universe/Special.h"
#include "../universe/ValueRef.h"
#include "../universe/UniverseObject.h"
#include "../client/human/GGHumanClientApp.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "IconTextBrowseWnd.h"

namespace {
    constexpr int   EDGE_PAD = 3;
    constexpr GG::X SPECIAL_ICON_WIDTH{24};
    constexpr GG::Y SPECIAL_ICON_HEIGHT{24};
}

SpecialsPanel::SpecialsPanel(GG::X w, int object_id) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y(32), GG::INTERACTIVE),
    m_object_id(object_id)
{
    SetName("SpecialsPanel"); // TODO: add a Wnd constructor that takes a name so this isn't needed
}

void SpecialsPanel::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();
    Update();
}

bool SpecialsPanel::InWindow(GG::Pt pt) const {
    return std::any_of(m_icons.begin(), m_icons.end(),
                       [pt](const auto& icon) { return icon->InWindow(pt); });
}

void SpecialsPanel::MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void SpecialsPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        Update();
}

void SpecialsPanel::Update() {
    //std::cout << "SpecialsPanel::Update" << std::endl;
    for (auto& icon : m_icons)
        DetachChild(icon);
    m_icons.clear();


    // get specials to display
    auto obj = Objects().get(m_object_id);
    if (!obj) {
        ErrorLogger() << "SpecialsPanel::Update couldn't get object with id " << m_object_id;
        return;
    }
    m_icons.reserve(obj->Specials().size());

    // get specials and use them to create specials icons
    // for specials with a nonzero
    for (auto& [special_name, tc] : obj->Specials()) {
        const auto& [special_added_turn, special_capacity] = tc;
        const Special* special = GetSpecial(special_name);
        if (!special)
            continue;
        std::shared_ptr<StatisticIcon> graphic;
        if (special_capacity > 0.0f)
            graphic = GG::Wnd::Create<StatisticIcon>(ClientUI::SpecialIcon(special_name), special_capacity, 2, false,
                                                     SPECIAL_ICON_WIDTH, SPECIAL_ICON_HEIGHT);
        else
            graphic = GG::Wnd::Create<StatisticIcon>(ClientUI::SpecialIcon(special_name),
                                                     SPECIAL_ICON_WIDTH, SPECIAL_ICON_HEIGHT);

        graphic->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

        std::string desc = special->Description();

        if (special_capacity > 0.0f)
            desc += "\n" + boost::io::str(FlexibleFormat(UserString("SPECIAL_CAPACITY")) % DoubleToString(special_capacity, 2, false));

        if (special_added_turn > 0)
            desc += "\n" + boost::io::str(FlexibleFormat(UserString("ADDED_ON_TURN")) % special_added_turn);
        else
            desc += "\n" + UserString("ADDED_ON_INITIAL_TURN");

        if (GetOptionsDB().Get<bool>("resource.effects.description.shown") && !special->Effects().empty())
            desc += "\n" + Dump(special->Effects());

        graphic->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
            ClientUI::SpecialIcon(special_name), UserString(special_name), desc));
        m_icons.push_back(graphic);

        graphic->RightClickedSignal.connect([name{special_name}](GG::Pt pt) {

            auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
            std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(name));

            auto zoom_action = [name]() { ClientUI::GetClientUI()->ZoomToSpecial(name); };
            popup->AddMenuItem(GG::MenuItem(std::move(popup_label), false, false, zoom_action));

            popup->Run();
        });
    }

    const GG::X AVAILABLE_WIDTH = Width() - EDGE_PAD;
    GG::X x{EDGE_PAD};
    GG::Y y{EDGE_PAD};

    for (auto& icon : m_icons) {
        icon->SizeMove(GG::Pt(x, y), GG::Pt(x,y) + GG::Pt(SPECIAL_ICON_WIDTH, SPECIAL_ICON_HEIGHT));
        AttachChild(icon);

        x += SPECIAL_ICON_WIDTH + EDGE_PAD;

        if (x + SPECIAL_ICON_WIDTH + EDGE_PAD > AVAILABLE_WIDTH) {
            x = GG::X(EDGE_PAD);
            y += SPECIAL_ICON_HEIGHT + EDGE_PAD;
        }
    }

    if (m_icons.empty())
        Resize(GG::Pt(Width(), GG::Y0));
    else
        Resize(GG::Pt(Width(), y + SPECIAL_ICON_HEIGHT + EDGE_PAD*2));
}
