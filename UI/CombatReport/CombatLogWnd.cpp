#include "CombatLogWnd.h"

#include <GG/Layout.h>
#include "../LinkText.h"

#include "../../client/human/HumanClientApp.h"
#include "../../combat/CombatLogManager.h"
#include "../../combat/CombatEvents.h"
#include "../../universe/System.h"
#include "../../util/AppInterface.h"
#include "../../util/i18n.h"
#include "../../util/Logger.h"
#include "../../universe/UniverseObject.h"
#include "../AccordionPanel.h"
#include "../../Empire/Empire.h"

namespace {
    const std::string EMPTY_STRING;

    std::map<int, int> CountByOwner(const std::set<int>& objects) {
        std::map<int, int> objects_per_owner;
        for (std::set<int>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
            TemporaryPtr<const UniverseObject> object = Objects().Object(*it);
            if (object && (
                    object->ObjectType() == OBJ_SHIP || (
                        object->GetMeter(METER_POPULATION) &&
                        object->CurrentMeterValue(METER_POPULATION) > 0.0)))
            {
                int owner_id = object->Owner();
                if (objects_per_owner.find(owner_id) == objects_per_owner.end())
                    objects_per_owner[owner_id] = 0;
                ++objects_per_owner[owner_id];
            }
        }
        return objects_per_owner;
    }

    std::string CountsToText(const std::map<int, int>& count_per_empire, const std::string& delimiter = ", ") {
        std::stringstream ss;
        for (std::map<int,int>::const_iterator it = count_per_empire.begin(); it != count_per_empire.end(); ) {
            std::string owner_string = UserString("NEUTRAL");
            if (const Empire* owner = GetEmpire(it->first))
                owner_string = GG::RgbaTag(owner->Color()) + owner->Name() + "</rgba>";
            ss << owner_string << ": " << it->second;
            ++it;
            if (it != count_per_empire.end())
                ss << delimiter;
        }
        return ss.str();
    }
}

CombatLogWnd::CombatLogWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
    m_text_format_flags(GG::FORMAT_WORDBREAK| GG::FORMAT_LEFT | GG::FORMAT_TOP)
{
    SetName("CombatLogWnd");
}

void CombatLogWnd::HandleLinkClick(const std::string& link_type, const std::string& data)
{ LinkClickedSignal(link_type, data); }
void CombatLogWnd::HandleLinkDoubleClick(const std::string& link_type, const std::string& data)
{ LinkDoubleClickedSignal(link_type, data); }
void CombatLogWnd::HandleLinkRightClick(const std::string& link_type, const std::string& data)
{ LinkRightClickedSignal(link_type, data); }


LinkText * CombatLogWnd::DecorateLinkText(std::string const & text) {
    LinkText * links = new LinkText(GG::X0, GG::Y0, text, ClientUI::GetFont(), GG::CLR_WHITE);

    links->SetTextFormat(m_text_format_flags);

    links->SetDecorator(VarText::SHIP_ID_TAG, new ColorByOwner());
    links->SetDecorator(VarText::PLANET_ID_TAG, new ColorByOwner());

    GG::Connect(links->LinkClickedSignal,       &CombatLogWnd::HandleLinkClick,          this);
    GG::Connect(links->LinkDoubleClickedSignal, &CombatLogWnd::HandleLinkDoubleClick,    this);
    GG::Connect(links->LinkRightClickedSignal,  &CombatLogWnd::HandleLinkDoubleClick,    this);

    return links;
}

void CombatLogWnd::AddRow(GG::Wnd * wnd) {
    if( GG::Layout * layout = GetLayout())
        layout->Add(wnd, layout->Rows(), 0);
}

void CombatLogWnd::SetLog(int log_id) {
    if (!CombatLogAvailable(log_id)) {
        ErrorLogger() << "Couldn't find combat log with id: " << log_id;
        return;
    }

    DeleteChildren();
    GG::Layout* layout = new GG::Layout(UpperLeft().x, UpperLeft().y, Width(), Height()
                                        , 1, 1 ///< numrows, numcols
                                        , 0, 0 ///< wnd margin, cell margin
                                       );
    SetLayout(layout);

    const CombatLog& log = GetCombatLog(log_id);
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    DebugLogger() << "Setting log with " << log.combat_events.size() << " events";

    TemporaryPtr<const System> system = GetSystem(log.system_id);
    const std::string& sys_name = (system ? system->PublicName(client_empire_id) : UserString("ERROR"));

    AddRow(DecorateLinkText(str(FlexibleFormat(UserString("ENC_COMBAT_LOG_DESCRIPTION_STR"))
                                % LinkTaggedIDText(VarText::SYSTEM_ID_TAG, log.system_id, sys_name)
                                % log.turn) + "\n"
                           ));
    AddRow(DecorateLinkText(UserString("COMBAT_INITIAL_FORCES")));
    AddRow(DecorateLinkText(CountsToText(CountByOwner(log.object_ids))));

    for (std::vector<CombatEventPtr>::const_iterator it = log.combat_events.begin();
         it != log.combat_events.end(); ++it) {
        DebugLogger() << "event debug info: " << it->get()->DebugString();

        AddRow(DecorateLinkText(it->get()->CombatLogDescription(client_empire_id)));
    }

    std::stringstream summary_text;
    summary_text << "\n" + UserString("COMBAT_SUMMARY_DESTROYED")
        + "\n" + CountsToText(CountByOwner(log.destroyed_object_ids));

    AddRow(DecorateLinkText(summary_text.str()));

    //Add a dummy row that the layout manager can use to add space.
    AddRow(DecorateLinkText(""));
    layout->SetRowStretch(layout->Rows() - 1, 1);

}

GG::Pt CombatLogWnd::ClientUpperLeft() const
{ return UpperLeft() + GG::Pt(GG::X(MARGIN), GG::Y(MARGIN)); }

GG::Pt CombatLogWnd::ClientLowerRight() const
{ return LowerRight() - GG::Pt(GG::X(MARGIN), GG::Y(MARGIN)); }

void CombatLogWnd::Render() {
    GG::Clr interior_color =  ClientUI::CtrlColor();
    GG::Clr border_color = ClientUI::CtrlBorderColor();

    GG::Pt ul = UpperLeft(), lr = LowerRight();

    FlatRectangle(ul, lr, interior_color, border_color, 1);
}
