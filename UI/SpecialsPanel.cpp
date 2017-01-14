#include "SpecialsPanel.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../universe/Effect.h"
#include "../universe/Special.h"
#include "../universe/UniverseObject.h"
#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "IconTextBrowseWnd.h"

namespace {
    const int       EDGE_PAD(3);

    const GG::X     SPECIAL_ICON_WIDTH(24);
    const GG::Y     SPECIAL_ICON_HEIGHT(24);
}

SpecialsPanel::SpecialsPanel(GG::X w, int object_id) : 
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y(32), GG::INTERACTIVE),
    m_object_id(object_id),
    m_icons()
{
    SetName("SpecialsPanel");
    Update();
}

bool SpecialsPanel::InWindow(const GG::Pt& pt) const {
    bool retval = false;
    for (const std::map<std::string, StatisticIcon*>::value_type& entry : m_icons) {
        if (entry.second->InWindow(pt)) {
            retval = true;
            break;
        }
    }
    return retval;
}

void SpecialsPanel::Render()
{}

void SpecialsPanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void SpecialsPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        Update();
}

void SpecialsPanel::Update() {
    //std::cout << "SpecialsPanel::Update" << std::endl;
    for (std::map<std::string, StatisticIcon*>::value_type& entry : m_icons)
        DeleteChild(entry.second);
    m_icons.clear();


    // get specials to display
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_object_id);
    if (!obj) {
        ErrorLogger() << "SpecialsPanel::Update couldn't get object with id " << m_object_id;
        return;
    }

    // get specials and use them to create specials icons
    // for specials with a nonzero
    for (const std::map<std::string, std::pair<int, float>>::value_type& entry : obj->Specials()) {
        const Special* special = GetSpecial(entry.first);
        StatisticIcon* graphic = 0;
        if (entry.second.second > 0.0f)
            graphic = new StatisticIcon(ClientUI::SpecialIcon(special->Name()), entry.second.second, 2, false,
                                        GG::X0, GG::Y0, SPECIAL_ICON_WIDTH, SPECIAL_ICON_HEIGHT);
        else
            graphic = new StatisticIcon(ClientUI::SpecialIcon(special->Name()),
                                        GG::X0, GG::Y0, SPECIAL_ICON_WIDTH, SPECIAL_ICON_HEIGHT);

        graphic->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

        std::string desc = special->Description();

        if (entry.second.second > 0.0f)
            desc += "\n" + boost::io::str(FlexibleFormat(UserString("SPECIAL_CAPACITY")) % DoubleToString(entry.second.second, 2, false));

        if (entry.second.first > 0)
            desc += "\n" + boost::io::str(FlexibleFormat(UserString("ADDED_ON_TURN")) % boost::lexical_cast<std::string>(entry.second.first));
        else
            desc += "\n" + UserString("ADDED_ON_INITIAL_TURN");

        if (GetOptionsDB().Get<bool>("UI.dump-effects-descriptions") && !special->Effects().empty()) {
            desc += "\n" + Dump(special->Effects());
        }

        graphic->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
            new IconTextBrowseWnd(ClientUI::SpecialIcon(special->Name()),
                                  UserString(special->Name()),
                                  desc)));
        m_icons[entry.first] = graphic;

        graphic->InstallEventFilter(this);
    }

    const GG::X AVAILABLE_WIDTH = Width() - EDGE_PAD;
    GG::X x(EDGE_PAD);
    GG::Y y(EDGE_PAD);

    for (std::map<std::string, StatisticIcon*>::value_type& entry : m_icons) {
        StatisticIcon* icon = entry.second;
        icon->SizeMove(GG::Pt(x, y), GG::Pt(x,y) + GG::Pt(SPECIAL_ICON_WIDTH, SPECIAL_ICON_HEIGHT));
        AttachChild(icon);

        x += SPECIAL_ICON_WIDTH + EDGE_PAD;

        if (x + SPECIAL_ICON_WIDTH + EDGE_PAD > AVAILABLE_WIDTH) {
            x = GG::X(EDGE_PAD);
            y += SPECIAL_ICON_HEIGHT + EDGE_PAD;
        }
    }

    if (m_icons.empty()) {
        Resize(GG::Pt(Width(), GG::Y0));
    } else {
        Resize(GG::Pt(Width(), y + SPECIAL_ICON_HEIGHT + EDGE_PAD*2));
    }
}

bool SpecialsPanel::EventFilter(GG::Wnd* w, const GG::WndEvent& event) {
    if (event.Type() != GG::WndEvent::RClick)
        return false;
    const GG::Pt& pt = event.Point();

    for (const std::map<std::string, StatisticIcon*>::value_type& entry : m_icons)
    {
        if (entry.second != w)
            continue;

        std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(entry.first));

        GG::MenuItem menu_contents;
        menu_contents.next_level.push_back(GG::MenuItem(popup_label, 1, false, false));
        CUIPopupMenu popup(pt.x, pt.y, menu_contents);

        if (!popup.Run() || popup.MenuID() != 1) {
            return false;
            break;
        }

        ClientUI::GetClientUI()->ZoomToSpecial(entry.first);
        return true;
    }
    return false;
}
