#include "CombatLogWnd.h"

#include "../../client/human/HumanClientApp.h"
#include "../../combat/CombatLogManager.h"
#include "../../combat/CombatEvents.h"
#include "../../universe/System.h"
#include "../../util/AppInterface.h"
#include "../../util/i18n.h"
#include "../../util/Logger.h"
#include "../../universe/UniverseObject.h"
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

std::string CountsToText(const std::map<int, int>& count_per_empire, std::string delimiter = ", ") {
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

const std::string& LinkTag(UniverseObjectType obj_type) {
    switch (obj_type) {
    case OBJ_SHIP:
        return VarText::SHIP_ID_TAG;
        break;
    case OBJ_FLEET:
        return VarText::FLEET_ID_TAG;
        break;
    case OBJ_PLANET:
        return VarText::PLANET_ID_TAG;
        break;
    case OBJ_BUILDING:
        return VarText::BUILDING_ID_TAG;
        break;
    case OBJ_SYSTEM:
        return VarText::SYSTEM_ID_TAG;
        break;
    case OBJ_FIELD:
    default:
        return EMPTY_STRING;
    }
}

/// Creates a link tag of the appropriate type for object_id,
/// with the content being the public name from the point of view of client_empire_id.
/// Returns not_found if object_id is not found.
std::string PublicNameLink(int client_empire_id, int object_id, std::string not_found) {
    TemporaryPtr<const UniverseObject> object = GetUniverseObject(object_id);
    if (object) {
        const std::string& name = object->PublicName(client_empire_id);
        const std::string& tag = LinkTag(object->ObjectType());
        return LinkTaggedIDText(tag, object_id, name);
    } else {
        return not_found;
    }
}

class ColorByOwner: public LinkDecorator {
public:
    virtual std::string Decorate(const std::string& object_id_str, const std::string& content) const {
        GG::Clr color = ClientUI::DefaultLinkColor();
        const Empire* empire = 0;

        // get object indicated by object_id, and then get object's owner, if any
        int object_id = CastStringToInt(object_id_str);
        TemporaryPtr<const UniverseObject> object = Objects().Object(object_id);
        if (object && !object->Unowned())
            empire = GetEmpire(object->Owner());
        if (empire)
            color = empire->Color();

        return GG::RgbaTag(color) + content + "</rgba>";
    }
};

}


CombatLogWnd::CombatLogWnd():
    CUILinkTextMultiEdit("", GG::MULTI_WORDBREAK | GG::MULTI_READ_ONLY)
{
    SetDecorator(VarText::SHIP_ID_TAG, new ColorByOwner());
    SetDecorator(VarText::PLANET_ID_TAG, new ColorByOwner());
}

void CombatLogWnd::SetLog(int log_id) {
    std::stringstream detailed_description;
    bool available = CombatLogAvailable(log_id);
    if (!available) {
        Logger().errorStream() << "EncyclopediaDetailPanel::Refresh couldn't find combat log with id: " << log_id;
        return;
    }
    const CombatLog& log = GetCombatLog(log_id);
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    std::string name = UserString("ENC_COMBAT_LOG");
    boost::shared_ptr<GG::Texture> texture = ClientUI::GetTexture(ClientUI::ArtDir() / "/icons/sitrep/combat.png", true);
    std::string general_type = UserString("ENC_COMBAT_LOG");

    TemporaryPtr<const System> system = GetSystem(log.system_id);
    const std::string& sys_name = (system ? system->PublicName(client_empire_id) : UserString("ERROR"));

    detailed_description << str(FlexibleFormat(UserString("ENC_COMBAT_LOG_DESCRIPTION_STR"))
                                % LinkTaggedIDText(VarText::SYSTEM_ID_TAG, log.system_id, sys_name)
                                % log.turn) + "\n";

    detailed_description <<"\n"+ UserString("COMBAT_INITIAL_FORCES") + "\n" + CountsToText(CountByOwner(log.object_ids))+"\n";

    for ( std::vector<CombatEventPtr>::const_iterator it = log.combat_events.begin();
            it != log.combat_events.end(); ++it )
    {
        const BoutBeginEvent* bout_begin = dynamic_cast<BoutBeginEvent*>(it->get());
        const AttackEvent* attack = dynamic_cast<AttackEvent*>(it->get());
        const IncapacitationEvent* incapacitation = dynamic_cast<IncapacitationEvent*>(it->get());
        if ( bout_begin ) {
            detailed_description << str(FlexibleFormat(UserString("ENC_ROUND_BEGIN")) % bout_begin->bout) + "\n";
        } else if ( attack ) {
            std::string attacker_link = PublicNameLink(client_empire_id, attack->attacker_id, UserString("ENC_COMBAT_UNKNOWN_OBJECT"));
            std::string target_link = PublicNameLink(client_empire_id, attack->target_id, UserString("ENC_COMBAT_UNKNOWN_OBJECT"));

            const std::string& template_str = UserString("ENC_COMBAT_ATTACK_STR");

            detailed_description << str(FlexibleFormat(template_str)
                                        % attacker_link
                                        % target_link
                                        % attack->damage
                                        % attack->bout
                                        % attack->round) + "\n";
        } else if ( incapacitation ) {
            TemporaryPtr<const UniverseObject> object = GetUniverseObject(incapacitation->object_id);
            std::string  template_str;
            if ( !object ) {
                template_str = UserString("ENC_COMBAT_UNKNOWN_DESTROYED_STR");
            } else if ( object->ObjectType() == OBJ_PLANET ) {
                template_str = UserString("ENC_COMBAT_PLANET_INCAPACITATED_STR");
            } else {
                template_str = UserString("ENC_COMBAT_DESTROYED_STR");
            }
            const std::string object_link = PublicNameLink ( client_empire_id, incapacitation->object_id, UserString ( "ENC_COMBAT_UNKNOWN_OBJECT" ) );

            int owner_id = object?object->Owner():ALL_EMPIRES;
            std::string owner_string = " ";
            if ( owner_id != ALL_EMPIRES ) {
                Empire* owner = GetEmpire(owner_id);
                if ( owner ) {
                    owner_string += owner->Name() + " ";
                }
            }

            detailed_description << str(FlexibleFormat(template_str) % owner_string % object_link) + "\n";
        }
    }

    detailed_description << "\n" + UserString("COMBAT_SUMMARY_DESTROYED") + "\n" + CountsToText(CountByOwner(log.destroyed_object_ids));

    SetText(detailed_description.str());
}
