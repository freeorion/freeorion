#include "CombatEvents.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>

#include "../util/Serialize.ipp"
#include "../util/Serialize.h"
#include "../universe/Universe.h"
#include "../universe/Enums.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../util/VarText.h"
#include "../util/GameRules.h"

#include "../Empire/Empire.h"

#include <sstream>

namespace {
    // would be better in CombatSystem, but that is server-only, and rules need
    // to exist on client and server.
    void AddRules(GameRules& rules) {
        // makes all buildings cost 1 PP and take 1 turn to produce
        rules.Add<int>("RULE_NUM_COMBAT_ROUNDS", "RULE_NUM_COMBAT_ROUNDS_DESC",
                       "", 3, true, RangedValidator<int>(1, 20));
    }
    bool temp_bool = RegisterGameRules(&AddRules);


    //TODO: Move this code into a common non UI linked location, so that
    //there is no duplicated code between server and clientUI.
    const std::string EMPTY_STRING("");
    const std::string& LinkTag(UniverseObjectType obj_type) {
        switch (obj_type) {
        case OBJ_SHIP:
            return VarText::SHIP_ID_TAG;
        case OBJ_FLEET:
            return VarText::FLEET_ID_TAG;
        case OBJ_PLANET:
            return VarText::PLANET_ID_TAG;
        case OBJ_BUILDING:
            return VarText::BUILDING_ID_TAG;
        case OBJ_SYSTEM:
            return VarText::SYSTEM_ID_TAG;
        case OBJ_FIELD:
        case OBJ_FIGHTER:
        default:
            return EMPTY_STRING;
        }
    }

    std::string WrapWithTagAndId(const std::string& meat, const std::string& tag, int id) {
        std::stringstream ss;
        ss << "<" << tag << " " << std::to_string(id) << ">" << meat << "</" << tag << ">";
        return ss.str();
    }

    std::string WrapUserStringWithTag(const std::string& table_id, const std::string& tag) {
        std::stringstream ss;
        ss << "<" << tag << " " << table_id << ">" << UserString(table_id) << "</" << tag << ">";
        return ss.str();
    }

    //Copied pasted from Font.cpp due to Font not being linked into AI and server code
    std::string WrapColorTag(std::string const & text, const GG::Clr& c) {
        std::stringstream stream;
        stream << "<rgba "
               << static_cast<int>(c.r) << " "
               << static_cast<int>(c.g) << " "
               << static_cast<int>(c.b) << " "
               << static_cast<int>(c.a)
               << ">" << text << "</rgba>";
        return stream.str();
    }

    std::string EmpireColorWrappedText(int empire_id, const std::string& text) {
        // TODO: refactor this to somewhere that links with the UI code.
        // Hardcoded default color becauses not linked with UI code.
        const Empire* empire = GetEmpire(empire_id);
        GG::Clr c = (empire ? empire->Color() : GG::Clr(80,255,128,255));
        return WrapColorTag(text, c);
    }

    /// Creates a link tag of the appropriate type for object_id,
    /// with the content being the public name from the point of view of empire_id.
    /// Returns UserString("ENC_COMBAT_UNKNOWN_OBJECT") if object_id is not found.
    std::string PublicNameLink(int empire_id, int object_id) {
        auto object = GetUniverseObject(object_id);
        if (object) {
            const std::string& name = object->PublicName(empire_id);
            const std::string& tag = LinkTag(object->ObjectType());
            return WrapWithTagAndId(name, tag, object_id);
        } else {
            return UserString("ENC_COMBAT_UNKNOWN_OBJECT");
        }
    }

    /// Creates a link tag of the appropriate type for either a fighter or another object.
    std::string FighterOrPublicNameLink(
        int viewing_empire_id, int object_id, int object_empire_id) {
        if (object_id >= 0)   // ship
            return PublicNameLink(viewing_empire_id, object_id);
        else                  // fighter
            return EmpireColorWrappedText(object_empire_id, UserString("OBJ_FIGHTER"));
    }

    std::string EmpireLink(int empire_id) {
        const Empire* empire = GetEmpire(empire_id);
        if (empire) {
            const std::string& tag = VarText::EMPIRE_ID_TAG;
            std::string empire_wrapped = WrapWithTagAndId(empire->Name(), tag, empire_id);
            return EmpireColorWrappedText(empire_id, empire_wrapped);
        } else {
            return UserString("ENC_COMBAT_UNKNOWN_OBJECT");
        }
    }

    std::string ShipPartLink(std::string const & part) {
        return part.empty() ? UserString("ENC_COMBAT_UNKNOWN_OBJECT")
            : WrapUserStringWithTag(part, VarText::SHIP_PART_TAG);
    }
}

//////////////////////////////////////////
///////// BoutBeginEvent//////////////////
//////////////////////////////////////////
BoutBeginEvent::BoutBeginEvent() :
    bout(-1)
{}

BoutBeginEvent::BoutBeginEvent(int bout_) :
    bout(bout_)
{}

std::string BoutBeginEvent::DebugString() const {
    std::stringstream ss;
    ss << "Bout " << bout << " begins.";
    return ss.str();
}

std::string BoutBeginEvent::CombatLogDescription(int viewing_empire_id) const {
    return str(FlexibleFormat(UserString("ENC_ROUND_BEGIN")) % bout);
}

template <class Archive>
void BoutBeginEvent::serialize(Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout);
}

BOOST_CLASS_EXPORT(BoutBeginEvent)

template
void BoutBeginEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void BoutBeginEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void BoutBeginEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void BoutBeginEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);


//////////////////////////////////////////
///////// BoutEvent /////////////
//////////////////////////////////////////
BoutEvent::BoutEvent():
    bout(-1), events() {}

BoutEvent::BoutEvent(int _bout):
    bout(_bout), events() {}

void BoutEvent::AddEvent(const CombatEventPtr& event)
{ events.push_back(event); }

std::string BoutEvent::DebugString() const {
    std::stringstream ss;
    ss << "Bout " << bout << " has " << events.size() << " events";
    return ss.str();
}

std::string BoutEvent::CombatLogDescription(int viewing_empire_id) const {
    return str(FlexibleFormat(UserString("ENC_ROUND_BEGIN")) % bout);
}

std::vector<ConstCombatEventPtr> BoutEvent::SubEvents(int viewing_empire_id) const {
    std::vector<ConstCombatEventPtr> all_events;
    for (CombatEventPtr event : events) {
        all_events.push_back(event);
    }
    return all_events;
}

template <class Archive>
void BoutEvent::serialize(Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout)
        & BOOST_SERIALIZATION_NVP(events);
}

BOOST_CLASS_VERSION(BoutEvent, 4)
BOOST_CLASS_EXPORT(BoutEvent)

template
void BoutEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void BoutEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void BoutEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void BoutEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);


//////////////////////////////////////////
///////// SimultaneousEvents ///////////////////
//////////////////////////////////////////

SimultaneousEvents::SimultaneousEvents() :
    events() {}

void SimultaneousEvents::AddEvent(const CombatEventPtr& event)
{ events.push_back(event); }

std::string SimultaneousEvents::DebugString() const {
    std::stringstream ss;
    ss << "SimultaneousEvents has " << events.size() << " events";
    return ss.str();
}

std::string SimultaneousEvents::CombatLogDescription(int viewing_empire_id) const
{ return ""; }

std::vector<ConstCombatEventPtr> SimultaneousEvents::SubEvents(int viewing_empire_id) const {
    // Sort the events by viewing empire, then ALL_EMPIRES and then
    // other empires.
    std::multimap<int, ConstCombatEventPtr> empire_to_event;
    typedef std::multimap<int, ConstCombatEventPtr>::iterator iterator;
    typedef std::pair<iterator, iterator> range;

    for (CombatEventPtr event : events) {
        boost::optional<int> maybe_faction = event->PrincipalFaction(viewing_empire_id);
        int faction = maybe_faction.get_value_or(ALL_EMPIRES);
        empire_to_event.insert({faction, event});
    }

    std::vector<ConstCombatEventPtr> ordered_events;

    range viewing_empire_events = empire_to_event.equal_range(viewing_empire_id);
    range all_empire_events = empire_to_event.equal_range(ALL_EMPIRES);

    for (iterator it = viewing_empire_events.first;
         it != viewing_empire_events.second; ++it) {
        ordered_events.push_back(it->second);
    }

    for (iterator it = all_empire_events.first;
         it != all_empire_events.second; ++it) {
        ordered_events.push_back(it->second);
    }

    for (auto& entry : empire_to_event) {
        if (entry.first != viewing_empire_id && entry.first != ALL_EMPIRES)
            ordered_events.push_back(entry.second);
    }

    return ordered_events;
}


template <class Archive>
void SimultaneousEvents::serialize(Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(events);
}

BOOST_CLASS_VERSION(SimultaneousEvents, 4)
BOOST_CLASS_EXPORT(SimultaneousEvents)

template
void SimultaneousEvents::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void SimultaneousEvents::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void SimultaneousEvents::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void SimultaneousEvents::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);


//////////////////////////////////////////
///////// InitialStealthEvent /////////////
//////////////////////////////////////////

InitialStealthEvent::InitialStealthEvent()
{}

InitialStealthEvent::InitialStealthEvent(const EmpireToObjectVisibilityMap& x) :
    empire_to_object_visibility(x)
{}

std::string InitialStealthEvent::DebugString() const {
    std::stringstream ss;
    ss << "InitialStealthEvent: ";
    for (const auto& empire_object_vis : empire_to_object_visibility) {
        ss << " Viewing Empire: " << EmpireLink(empire_object_vis.first) << "\n";

        for (const auto& viewed_object : empire_object_vis.second) {
            const auto obj = GetUniverseObject(viewed_object.first);
            int owner_id = obj ? obj->Owner() : ALL_EMPIRES;
            ss << FighterOrPublicNameLink(ALL_EMPIRES, viewed_object.first, owner_id);
        }
        ss << "\n";
    }
    return ss.str();
}

std::string InitialStealthEvent::CombatLogDescription(int viewing_empire_id) const {
    DebugLogger() << "CombatLogDescription for InitialStealthEvent viewing empire empire: " << viewing_empire_id;

    std::string desc = "";

    for (const auto& detector_empire : empire_to_object_visibility) {
        int detector_empire_id = detector_empire.first;
        DebugLogger() << "CombatLogDescription for InitialStealthEvent for detector empire: " << detector_empire_id;

        const auto& visible_objects = detector_empire.second;
        if (visible_objects.empty()) {
            DebugLogger() << " ... no object info recorded for detector empire: " << detector_empire_id;
            continue;
        }

        // Check Visibility of objects, report those that are not visible.
        std::vector<std::string> cloaked_attackers;
        for (auto& object_vis : visible_objects) {
            const auto obj = GetUniverseObject(object_vis.first);
            const auto name = obj ? obj->Name() : UserString("UNKNOWN");
            DebugLogger() << " ... object: " << name << " (" << object_vis.first << ") has vis: " << object_vis.second;
            if (object_vis.second > VIS_NO_VISIBILITY)
                continue;
            std::string attacker_link = FighterOrPublicNameLink(
                viewing_empire_id, object_vis.first, ALL_EMPIRES); // all empires specifies empire to use for link color if this is a fighter
            cloaked_attackers.push_back(attacker_link);
        }

        if (!cloaked_attackers.empty()) {
            desc += "\n"; //< Add \n at start of the report and between each empire
            std::vector<std::string> detector_empire_link(1, EmpireLink(detector_empire.first));
            desc += FlexibleFormatList(detector_empire_link, cloaked_attackers,
                                       UserString("ENC_COMBAT_INITIAL_STEALTH_LIST")).str();
        }
    }

    return desc;
}

template <class Archive>
void InitialStealthEvent::serialize(Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(empire_to_object_visibility);
}

BOOST_CLASS_VERSION(InitialStealthEvent, 4)
BOOST_CLASS_EXPORT(InitialStealthEvent)

template
void InitialStealthEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void InitialStealthEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void InitialStealthEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void InitialStealthEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);


//////////////////////////////////////////
///////// StealthChangeEvent /////////////
//////////////////////////////////////////

StealthChangeEvent::StealthChangeEvent() :
    bout(-1),
    events()
{}

StealthChangeEvent::StealthChangeEvent(int bout_) :
    bout(bout_),
    events()
{}

StealthChangeEvent::StealthChangeEventDetail::StealthChangeEventDetail() :
    attacker_id(INVALID_OBJECT_ID),
    target_id(INVALID_OBJECT_ID),
    attacker_empire_id(INVALID_OBJECT_ID),
    target_empire_id(INVALID_OBJECT_ID),
    visibility()
{}

StealthChangeEvent::StealthChangeEventDetail::StealthChangeEventDetail(
    int attacker_id_, int target_id_, int attacker_empire_, int target_empire_, Visibility new_visibility_) :
    attacker_id(attacker_id_),
    target_id(target_id_),
    attacker_empire_id(attacker_empire_),
    target_empire_id(target_empire_),
    visibility(new_visibility_)
{}

std::string StealthChangeEvent::StealthChangeEventDetail::DebugString() const {
    std::stringstream ss;
    ss << "StealthChangeDetailEvent"
       <<  FighterOrPublicNameLink(ALL_EMPIRES, attacker_id, attacker_empire_id)
       << "->" << visibility << " ";
    return ss.str();
}

std::string StealthChangeEvent::StealthChangeEventDetail::CombatLogDescription(int viewing_empire_id) const {

    std::string attacker_link = FighterOrPublicNameLink(viewing_empire_id, attacker_id, attacker_empire_id);
    std::string target_link = FighterOrPublicNameLink(viewing_empire_id, target_id, target_empire_id);
    std::string empire_link = EmpireLink(target_empire_id);
    const std::string& template_str = UserString("ENC_COMBAT_STEALTH_DECLOAK_ATTACK");

    return str(FlexibleFormat(template_str)
               % attacker_link
               % target_link
               % empire_link);
}


void StealthChangeEvent::AddEvent(int attacker_id_, int target_id_, int attacker_empire_
                             , int target_empire_, Visibility new_visibility_) {
    events[target_empire_].push_back(
        std::make_shared<StealthChangeEventDetail>(
            attacker_id_, target_id_, attacker_empire_, target_empire_, new_visibility_));
}

std::string StealthChangeEvent::DebugString() const {
    std::stringstream ss;
    ss << "StealthChangeEvent";
    if (events.size() > 4) {
        ss << events.size() << " empires.";
    } else {
        for (const auto& target : events) {
            ss << "Target Empire: " << EmpireLink(target.first) << "\n";

            if (target.second.size() > 4) {
                ss << target.second.size() << " events.";
            } else {
                for (const auto& event : target.second) {
                    ss << event->DebugString();
                }
            }
        }
    }
    return ss.str();
}

std::string StealthChangeEvent::CombatLogDescription(int viewing_empire_id) const {
    if (events.empty())
        return "";

    std::string desc = "";
    for (const auto& target : events) {
        std::vector<std::string> uncloaked_attackers;
        for (const auto event : target.second) {
            uncloaked_attackers.push_back(FighterOrPublicNameLink(viewing_empire_id, event->attacker_id, event->attacker_empire_id));
        }

        if (!uncloaked_attackers.empty()) {
            if (!desc.empty())
                desc += "\n";
            std::vector<std::string> target_empire_link(1, EmpireLink(target.first));

            desc += FlexibleFormatList(target_empire_link, uncloaked_attackers,
                                       UserString("ENC_COMBAT_STEALTH_DECLOAK_ATTACK_MANY_EVENTS"),
                                       UserString("ENC_COMBAT_STEALTH_DECLOAK_ATTACK_1_EVENTS")).str();
        }
    }

    return desc;
}

bool StealthChangeEvent::AreSubEventsEmpty(int viewing_empire_id) const {
    return events.empty();
}
std::vector<ConstCombatEventPtr> StealthChangeEvent::SubEvents(int viewing_empire_id) const {
    std::vector<ConstCombatEventPtr> all_events;
    for (const auto& target : events) {

        for (const auto event : target.second){
            all_events.push_back(std::dynamic_pointer_cast<CombatEvent>(event));
        }
    }
    return all_events;
}

template <class Archive>
void StealthChangeEvent::serialize(Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout)
       & BOOST_SERIALIZATION_NVP(events);
}

BOOST_CLASS_VERSION(StealthChangeEvent, 4)
BOOST_CLASS_EXPORT(StealthChangeEvent)

template
void StealthChangeEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void StealthChangeEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void StealthChangeEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void StealthChangeEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

template <class Archive>
void StealthChangeEvent::StealthChangeEventDetail::serialize(Archive& ar, const unsigned int version) {
    ar  & BOOST_SERIALIZATION_NVP(attacker_id)
        & BOOST_SERIALIZATION_NVP(target_id)
        & BOOST_SERIALIZATION_NVP(attacker_empire_id)
        & BOOST_SERIALIZATION_NVP(target_empire_id)
        & BOOST_SERIALIZATION_NVP(visibility);
}

BOOST_CLASS_VERSION(StealthChangeEvent::StealthChangeEventDetail, 4)
BOOST_CLASS_EXPORT(StealthChangeEvent::StealthChangeEventDetail)

template
void StealthChangeEvent::StealthChangeEventDetail::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void StealthChangeEvent::StealthChangeEventDetail::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void StealthChangeEvent::StealthChangeEventDetail::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void StealthChangeEvent::StealthChangeEventDetail::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

//////////////////////////////////////////
///////// Attack Event////////////////////
//////////////////////////////////////////
WeaponFireEvent::WeaponFireEvent() :
    bout(-1),
    round(-1),
    attacker_id(INVALID_OBJECT_ID),
    target_id(INVALID_OBJECT_ID),
    weapon_name(),
    power(0.0f),
    shield(0.0f),
    damage(0.0f),
    attacker_owner_id(ALL_EMPIRES),
    target_owner_id(INVALID_OBJECT_ID)
{}

WeaponFireEvent::WeaponFireEvent(
    int bout_, int round_, int attacker_id_, int target_id_, const std::string& weapon_name_,
    const std::tuple<float, float, float>& power_shield_damage,
    int attacker_owner_id_, int target_owner_id_) :
    bout(bout_),
    round(round_),
    attacker_id(attacker_id_),
    target_id( target_id_),
    weapon_name(weapon_name_),
    power(),
    shield(),
    damage(),
    attacker_owner_id(attacker_owner_id_),
    target_owner_id(target_owner_id_)
{ std::tie(power, shield, damage) = power_shield_damage; }

std::string WeaponFireEvent::DebugString() const {
    std::stringstream ss;
    ss << "rnd: " << round << " : "
       << attacker_id << " -> " << target_id << " : " << weapon_name << " "
       << power << " - " << shield << " = " << damage << "   attacker owner: " << attacker_owner_id;
    return ss.str();
}

std::string WeaponFireEvent::CombatLogDescription(int viewing_empire_id) const {
    std::string attacker_link = FighterOrPublicNameLink(viewing_empire_id, attacker_id, attacker_owner_id);
    std::string target_link = FighterOrPublicNameLink(viewing_empire_id, target_id, target_owner_id);

    const std::string& template_str = UserString("ENC_COMBAT_ATTACK_STR");

    return str(FlexibleFormat(template_str)
               % attacker_link
               % target_link
               % damage
               % bout
               % round);
}

std::string WeaponFireEvent::CombatLogDetails(int viewing_empire_id) const {
    const std::string& template_str = UserString("ENC_COMBAT_ATTACK_DETAILS");

    if (shield >= 0)
        return str(FlexibleFormat(template_str)
                   % ShipPartLink(weapon_name)
                   % power
                   % shield
                   % damage);
    else
        return str(FlexibleFormat(template_str)
                   % ShipPartLink(weapon_name)
                   % power
                   % UserString("ENC_COMBAT_SHIELD_PIERCED")
                   % damage);
}

boost::optional<int> WeaponFireEvent::PrincipalFaction(int viewing_empire_id) const {
    return attacker_id;
}


template <class Archive>
void WeaponFireEvent::serialize(Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout)
       & BOOST_SERIALIZATION_NVP(round)
       & BOOST_SERIALIZATION_NVP(attacker_id)
       & BOOST_SERIALIZATION_NVP(target_id)
       & BOOST_SERIALIZATION_NVP(weapon_name)
       & BOOST_SERIALIZATION_NVP(power)
       & BOOST_SERIALIZATION_NVP(shield)
       & BOOST_SERIALIZATION_NVP(damage)
       & BOOST_SERIALIZATION_NVP(target_owner_id)
       & BOOST_SERIALIZATION_NVP(attacker_owner_id);

    if (version < 3) {
        int target_destroyed = 0;
        ar & BOOST_SERIALIZATION_NVP (target_destroyed);
    }
}

BOOST_CLASS_VERSION(WeaponFireEvent, 4)
BOOST_CLASS_EXPORT(WeaponFireEvent)

template
void WeaponFireEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void WeaponFireEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void WeaponFireEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void WeaponFireEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

//////////////////////////////////////////
///////// IncapacitationEvent/////////////
//////////////////////////////////////////
IncapacitationEvent::IncapacitationEvent() :
    bout(-1),
    object_id(INVALID_OBJECT_ID),
    object_owner_id(ALL_EMPIRES)
{}

IncapacitationEvent::IncapacitationEvent(int bout_, int object_id_, int object_owner_id_) :
    bout(bout_),
    object_id(object_id_),
    object_owner_id(object_owner_id_)
{}

std::string IncapacitationEvent::DebugString() const {
    std::stringstream ss;
    ss << "incapacitation of " << object_id << " owned by " << object_owner_id << " at bout " << bout;
    return ss.str();
}


std::string IncapacitationEvent::CombatLogDescription(int viewing_empire_id) const {
    auto object = GetUniverseObject(object_id);
    std::string template_str, object_str;
    int owner_id = object_owner_id;

    if (!object && object_id < 0) {
        template_str = UserString("ENC_COMBAT_FIGHTER_INCAPACITATED_STR");
        object_str = UserString("OBJ_FIGHTER");

    } else if (!object) {
        template_str = UserString("ENC_COMBAT_UNKNOWN_DESTROYED_STR");
        object_str = UserString("ENC_COMBAT_UNKNOWN_OBJECT");

    } else if (object->ObjectType() == OBJ_PLANET) {
        template_str = UserString("ENC_COMBAT_PLANET_INCAPACITATED_STR");
        object_str = PublicNameLink(viewing_empire_id, object_id);

    } else {    // ships or other to-be-determined objects...
        template_str = UserString("ENC_COMBAT_DESTROYED_STR");
        object_str = PublicNameLink(viewing_empire_id, object_id);
    }

    std::string owner_string = " ";
    if (const Empire* owner = GetEmpire(owner_id))
        owner_string += owner->Name() + " ";

    std::string object_link = FighterOrPublicNameLink(viewing_empire_id, object_id, object_owner_id);

    return str(FlexibleFormat(template_str) % owner_string % object_link);
}

boost::optional<int> IncapacitationEvent::PrincipalFaction(int viewing_empire_id) const {
    return object_owner_id;
}


template <class Archive>
void IncapacitationEvent::serialize (Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout)
       & BOOST_SERIALIZATION_NVP(object_id)
       & BOOST_SERIALIZATION_NVP(object_owner_id);
}

BOOST_CLASS_EXPORT(IncapacitationEvent)

template
void IncapacitationEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void IncapacitationEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void IncapacitationEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void IncapacitationEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);


//////////////////////////////////////////
///////// FightersAttackFightersEvent ////
//////////////////////////////////////////

FightersAttackFightersEvent::FightersAttackFightersEvent() :
    bout(-1),
    events()
{}

FightersAttackFightersEvent::FightersAttackFightersEvent(int bout_) :
    bout(bout_),
    events()
{}

void FightersAttackFightersEvent::AddEvent(int attacker_empire_, int target_empire_)
{ events[{attacker_empire_, target_empire_}] += 1; }

std::string FightersAttackFightersEvent::DebugString() const {
    std::stringstream ss;
    ss << "FightersAttackFightersEvent: ";
    for (const auto& index_and_event: events) {
        ss << index_and_event.second << " repeated fighters from empire " << index_and_event.first.first
           << " attacking fighters from empire " << index_and_event.first.second << ", ";
    }
    return ss.str();
}

std::string FightersAttackFightersEvent::CombatLogDescription(int viewing_empire_id) const {
    if (events.empty())
        return "";

    const auto& events_to_show = events;
    auto num_events_remaining = events.size();
    std::stringstream ss;

    // Use show_events_for_empire to show events in this order: viewing empire, ALL_EMPIRES and
    // then the remainder.
    auto show_events_for_empire =
        [&ss, &num_events_remaining, &events_to_show, &viewing_empire_id]
        (boost::optional<int> show_attacker) {
            int attacker_empire;
            int target_empire;
            for (const auto& index_and_event : events_to_show) {
                std::tie(attacker_empire, target_empire) = index_and_event.first;

                // Skip if this is not the particular attacker requested
                if (show_attacker && *show_attacker != attacker_empire)
                    continue;

                // Skip if no particular attacker was requested and this empire is the viewing
                // empire or ALL_EMPIRES
                if (!show_attacker && (attacker_empire == viewing_empire_id || attacker_empire == ALL_EMPIRES))
                    continue;

                auto count = std::to_string(index_and_event.second);
                const auto&& attacker_link = FighterOrPublicNameLink(
                    viewing_empire_id, INVALID_OBJECT_ID, attacker_empire);
                const auto&& target_link = FighterOrPublicNameLink(
                    viewing_empire_id, INVALID_OBJECT_ID, target_empire);
                const std::string& template_str = UserString("ENC_COMBAT_ATTACK_REPEATED_STR");

                ss << str(FlexibleFormat(template_str) % count % attacker_link % target_link);
                if (--num_events_remaining > 0)
                    ss << "\n";
            }
        };

    // Sort the events by viewing empire, then ALL_EMPIRES and then other empires.
    show_events_for_empire(viewing_empire_id);
    show_events_for_empire(ALL_EMPIRES);
    show_events_for_empire(boost::none);

    return ss.str();
}


template <class Archive>
void FightersAttackFightersEvent::serialize(Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout)
       & BOOST_SERIALIZATION_NVP(events);
}

BOOST_CLASS_VERSION(FightersAttackFightersEvent, 4)
BOOST_CLASS_EXPORT(FightersAttackFightersEvent)

template
void FightersAttackFightersEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void FightersAttackFightersEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void FightersAttackFightersEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void FightersAttackFightersEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);



//////////////////////////////////////////
/////////// FighterLaunchEvent ///////////
//////////////////////////////////////////
FighterLaunchEvent::FighterLaunchEvent() :
    bout(-1),
    fighter_owner_empire_id(ALL_EMPIRES),
    launched_from_id(INVALID_OBJECT_ID),
    number_launched(0)
{}

FighterLaunchEvent::FighterLaunchEvent(int bout_, int launched_from_id_, int fighter_owner_empire_id_, int number_launched_) :
    bout(bout_),
    fighter_owner_empire_id(fighter_owner_empire_id_),
    launched_from_id(launched_from_id_),
    number_launched(number_launched_)
{}

std::string FighterLaunchEvent::DebugString() const {
    std::stringstream ss;
    ss << "launch from object " << launched_from_id
       << " of " << number_launched
       << " fighter(s) of empire " << fighter_owner_empire_id
       << " at bout " << bout;
    return ss.str();
}

std::string FighterLaunchEvent::CombatLogDescription(int viewing_empire_id) const {
    std::string launched_from_link = PublicNameLink(viewing_empire_id, launched_from_id);
    std::string empire_coloured_fighter = EmpireColorWrappedText(fighter_owner_empire_id, UserString("OBJ_FIGHTER"));

    // launching negative fighters indicates recovery of them by the ship
    const std::string& template_str = (number_launched >= 0 ?
                                       UserString("ENC_COMBAT_LAUNCH_STR") :
                                       UserString("ENC_COMBAT_RECOVER_STR"));

   return str(FlexibleFormat(template_str)
              % launched_from_link
              % empire_coloured_fighter
              % std::abs(number_launched));
}

boost::optional<int> FighterLaunchEvent::PrincipalFaction(int viewing_empire_id) const {
    return fighter_owner_empire_id;
}

template <class Archive>
void FighterLaunchEvent::serialize (Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout)
       & BOOST_SERIALIZATION_NVP(fighter_owner_empire_id)
       & BOOST_SERIALIZATION_NVP(launched_from_id)
       & BOOST_SERIALIZATION_NVP(number_launched);
}

BOOST_CLASS_EXPORT(FighterLaunchEvent)

template
void FighterLaunchEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void FighterLaunchEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void FighterLaunchEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void FighterLaunchEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);



//////////////////////////////////////////
///////// FightersDestroyedEvent ////
//////////////////////////////////////////

FightersDestroyedEvent::FightersDestroyedEvent() :
    bout(-1),
    events()
{}

FightersDestroyedEvent::FightersDestroyedEvent(int bout_) :
    bout(bout_),
    events()
{}

void FightersDestroyedEvent::AddEvent(int target_empire_)
{ events[target_empire_] += 1; }

std::string FightersDestroyedEvent::DebugString() const {
    std::stringstream ss;
    ss << "FightersDestroyedEvent: ";
    for (const auto& index_and_event : events) {
        ss << index_and_event.second << " repeated fighters from empire "
           << index_and_event.first << " destroyed.";
    }
    return ss.str();
}

std::string FightersDestroyedEvent::CombatLogDescription(int viewing_empire_id) const {
    if (events.empty())
        return "";

    const auto& events_to_show = events;
    auto num_events_remaining = events.size();
    std::stringstream ss;

    // Use show_events_for_empire to show events in this order: viewing empire,
    // ALL_EMPIRES and then the remainder.
    auto show_events_for_empire =
        [&ss, &num_events_remaining, &events_to_show, &viewing_empire_id]
        (boost::optional<int> show_empire_id) {
            int count;
            int target_empire_id;
            for (const auto& index_and_event : events_to_show) {
                std::tie(target_empire_id, count) = index_and_event;

                // Skip if this is not the particular attacker requested
                if (show_empire_id && *show_empire_id != target_empire_id)
                    continue;

                // Skip if no particular empire was requested and this empire is the viewing
                // empire or ALL_EMPIRES
                if (!show_empire_id && (target_empire_id == viewing_empire_id || target_empire_id == ALL_EMPIRES))
                    continue;

                auto count_str = std::to_string(index_and_event.second);
                auto target_empire_link = EmpireLink(target_empire_id);
                const auto&& target_link = FighterOrPublicNameLink(
                    viewing_empire_id, INVALID_OBJECT_ID, target_empire_id);

                if (count == 1) {
                    const std::string& template_str = UserString("ENC_COMBAT_FIGHTER_INCAPACITATED_STR");
                    ss << str(FlexibleFormat(template_str) % target_empire_link % target_link);
                }else {
                    const std::string& template_str = UserString("ENC_COMBAT_FIGHTER_INCAPACITATED_REPEATED_STR");
                    ss << str(FlexibleFormat(template_str) % count_str % target_empire_link % target_link);
                }
                if (--num_events_remaining > 0)
                    ss << "\n";
            }
        };

    // Sort the events by viewing empire, then ALL_EMPIRES and then other empires.
    show_events_for_empire(viewing_empire_id);
    show_events_for_empire(ALL_EMPIRES);
    show_events_for_empire(boost::none);

    return ss.str();
}


template <class Archive>
void FightersDestroyedEvent::serialize(Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout)
       & BOOST_SERIALIZATION_NVP(events);
}

BOOST_CLASS_VERSION(FightersDestroyedEvent, 4)
BOOST_CLASS_EXPORT(FightersDestroyedEvent)

template
void FightersDestroyedEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void FightersDestroyedEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void FightersDestroyedEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void FightersDestroyedEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);



//////////////////////////////////////////
///////// WeaponsPlatformEvent /////////////
//////////////////////////////////////////

WeaponsPlatformEvent::WeaponsPlatformEvent() :
    bout(-1),
    attacker_id(INVALID_OBJECT_ID),
    attacker_owner_id(INVALID_OBJECT_ID),
    events()
{}

WeaponsPlatformEvent::WeaponsPlatformEvent(int bout_, int attacker_id_, int attacker_owner_id_) :
    bout(bout_),
    attacker_id(attacker_id_),
    attacker_owner_id(attacker_owner_id_),
    events()
{}

void WeaponsPlatformEvent::AddEvent(
    int round_, int target_id_, int target_owner_id_, std::string const & weapon_name_,
    float power_, float shield_, float damage_) {
    events[target_id_].push_back(
        std::make_shared<WeaponFireEvent>(
            bout, round_, attacker_id, target_id_, weapon_name_,
            std::tie(power_, shield_, damage_),
            attacker_owner_id, target_owner_id_));
}

std::string WeaponsPlatformEvent::DebugString() const {
    std::stringstream desc;
    desc << "WeaponsPlatformEvent bout = " << bout << " attacker_id = "
        << attacker_id << " attacker_owner = "<< attacker_owner_id;
    for (const auto& target : events) {
        for (const auto attack : target.second) {
            desc << std::endl << attack->DebugString();
        }
    }
    return desc.str();
}

std::string WeaponsPlatformEvent::CombatLogDescription(int viewing_empire_id) const {
    if (events.empty())
        return "";

    std::vector<std::string> damaged_target_links;
    std::vector<std::string> undamaged_target_links;

    for (const auto& target : events) {
        if (target.second.empty())
            continue;

        const auto& fire_event(*target.second.begin());
        std::string target_public_name(
            FighterOrPublicNameLink(viewing_empire_id, target.first,
                                    fire_event->target_owner_id));

        double damage = 0.0f;
        for (auto attack_it : target.second) {
            damage += attack_it->damage;
        }

        if (damage <= 0.0f) {
            undamaged_target_links.push_back(target_public_name);

        } else {
            damaged_target_links.push_back(
                str(FlexibleFormat(UserString("ENC_COMBAT_PLATFORM_TARGET_AND_DAMAGE"))
                    % target_public_name % damage));
        }
    }

    std::string desc = "";

    const std::vector<std::string> attacker_link(
        1, FighterOrPublicNameLink(viewing_empire_id, attacker_id, attacker_owner_id));

    if (!damaged_target_links.empty() ) {
        desc += FlexibleFormatList(attacker_link, damaged_target_links,
                                   UserString("ENC_COMBAT_PLATFORM_DAMAGE_MANY_EVENTS"),
                                   UserString("ENC_COMBAT_PLATFORM_DAMAGE_1_EVENTS")).str();

        if (!undamaged_target_links.empty())
            desc += "\n";
    }
    if (!undamaged_target_links.empty()) {
        desc += FlexibleFormatList(attacker_link, undamaged_target_links,
                                   UserString("ENC_COMBAT_PLATFORM_NO_DAMAGE_MANY_EVENTS"),
                                   UserString("ENC_COMBAT_PLATFORM_NO_DAMAGE_1_EVENTS")).str();
    }
    return desc;
}

bool WeaponsPlatformEvent::AreSubEventsEmpty(int viewing_empire_id) const {
    return events.empty();
}

std::vector<ConstCombatEventPtr> WeaponsPlatformEvent::SubEvents(int viewing_empire_id) const {
    std::vector<ConstCombatEventPtr> all_events;
    for (const auto& target : events) {
        for (auto event : target.second) {
            all_events.push_back(std::dynamic_pointer_cast<CombatEvent>(event));
        }
    }
    return all_events;
}

boost::optional<int> WeaponsPlatformEvent::PrincipalFaction(int viewing_empire_id) const {
    return attacker_owner_id;
}

template <class Archive>
void WeaponsPlatformEvent::serialize(Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout)
        & BOOST_SERIALIZATION_NVP(attacker_id)
        & BOOST_SERIALIZATION_NVP(attacker_owner_id)
        & BOOST_SERIALIZATION_NVP(events);
}

BOOST_CLASS_VERSION(WeaponsPlatformEvent, 4)
BOOST_CLASS_EXPORT(WeaponsPlatformEvent)

template
void WeaponsPlatformEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void WeaponsPlatformEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void WeaponsPlatformEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void WeaponsPlatformEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);
