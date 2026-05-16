#include "CombatEvents.h"

#include "../universe/ScriptingContext.h"
#include "../universe/Universe.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../util/VarText.h"
#include "../util/GameRules.h"
#include "../util/GameRuleRanks.h"

#include "../Empire/Empire.h"
#include "../universe/Fighter.h"
#include "../universe/Planet.h"
#include "../universe/Ship.h"

#include <sstream>

namespace {
    // would be better in CombatSystem, but that is server-only, and rules need
    // to exist on client and server.
    void AddRules(GameRules& rules) {
        rules.Add<int>(UserStringNop("RULE_NUM_COMBAT_ROUNDS"),
                       UserStringNop("RULE_NUM_COMBAT_ROUNDS_DESC"),
                       GameRuleCategories::GameRuleCategory::GENERAL,
                       4, true,
                       GameRuleRanks::RULE_NUM_COMBAT_ROUNDS_RANK,
                       RangedValidator<int>(2, 20));
        rules.Add<bool>(UserStringNop("RULE_AGGRESSIVE_SHIPS_COMBAT_VISIBLE"),
                        UserStringNop("RULE_AGGRESSIVE_SHIPS_COMBAT_VISIBLE_DESC"),
                        GameRuleCategories::GameRuleCategory::GENERAL,
                        false, true,
                        GameRuleRanks::RULE_AGGRESSIVE_SHIPS_COMBAT_VISIBLE_RANK);

    }
    bool temp_bool = RegisterGameRules(&AddRules);


    // TODO: Move this code into a common non UI linked location, so that
    // there is no duplicated code between server and clientUI.
    constexpr std::string_view LinkTag(UniverseObjectType obj_type) {
        switch (obj_type) {
        case UniverseObjectType::OBJ_SHIP:
            return VarText::SHIP_ID_TAG;
        case UniverseObjectType::OBJ_FLEET:
            return VarText::FLEET_ID_TAG;
        case UniverseObjectType::OBJ_PLANET:
            return VarText::PLANET_ID_TAG;
        case UniverseObjectType::OBJ_BUILDING:
            return VarText::BUILDING_ID_TAG;
        case UniverseObjectType::OBJ_SYSTEM:
            return VarText::SYSTEM_ID_TAG;
        case UniverseObjectType::OBJ_FIELD:
        case UniverseObjectType::OBJ_FIGHTER:
            [[fallthrough]];
        default:
            return "";
        }
    }

    std::string WrapWithTagAndId(std::string_view meat, std::string_view tag, int id) {
        std::string retval;
        retval.reserve(1  + 2* tag.size() + 1 + 10 + 1 + meat.size() + 2 + 1); // guesstimate
        retval.append("<").append(tag).append(" ").append(std::to_string(id)).append(">")
              .append(meat).append("</").append(tag).append(">");
        return retval;
    }

    std::string WrapUserStringWithTag(std::string_view table_id, std::string_view tag) {
        std::string retval;
        const auto& us{UserString(table_id)};
        retval.reserve(1 + 2*tag.size() + 1 + table_id.size() + 1 + us.size() + 2 + 1 + 5);
        retval.append("<").append(tag).append(" ").append(table_id).append(">")
              .append(us).append("</").append(tag).append(">");
        return retval;
    }

    constexpr auto* UInt8ToChars(std::string::value_type* out_it, const uint8_t num) noexcept {
        const uint8_t hundreds = num / 100u;
        const uint8_t less_than_100 = num - hundreds*100u;
        const uint8_t tens = less_than_100 / 10u;
        const uint8_t ones = less_than_100 - tens*10u;
        if (hundreds > 0)
            *out_it++ = (hundreds + '0');
        if (tens > 0 || hundreds > 0)
            *out_it++ = (tens + '0');
        *out_it++ = (ones + '0');
        return out_it;
    };

    constexpr auto* ToChars(std::string::value_type* it, EmpireColor c) noexcept {
        it = UInt8ToChars(it, c[0]);
        *it++ = ' ';
        it = UInt8ToChars(it, c[1]);
        *it++ = ' ';
        it = UInt8ToChars(it, c[2]);
        *it++ = ' ';
        it = UInt8ToChars(it, c[3]);
        return it;
    }

    //                                 <rgba  255    ' '  '>'   0
    constexpr std::size_t rgba_tag_sz = 6u + 4u*3u + 3u + 1u + 1u;
    constexpr std::array<std::string::value_type, rgba_tag_sz> rgba_tag_container_with_prefix{
        "<rgba \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"};
    static_assert(rgba_tag_container_with_prefix.back() == 0); // verify array fill with 0 to end
    constexpr std::string_view rgba_close_tag{"</rgba>"};


    CONSTEXPR_STRING std::string WrapColorTag(std::string_view text, EmpireColor c) {
        static_assert(std::numeric_limits<EmpireColor::value_type>::max() < 1000); // ensure no more than 3 characters will be consumed per number
        static_assert(c.size() >= 4);
        auto rgba_tag(rgba_tag_container_with_prefix); // copy from constant

        auto next_it = ToChars(std::next(rgba_tag.data(), 6), c);
        *next_it++ = '>';
        const auto tag_view = std::string_view(rgba_tag.data(), std::distance(rgba_tag.data(), next_it));

        std::string retval;
        retval.reserve(text.size() + rgba_tag_sz + rgba_close_tag.size());
        retval.append(tag_view).append(text).append(rgba_close_tag);

        return retval;
    }

#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 11))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934)))
    static_assert(WrapColorTag("A", EmpireColor{}).size() == 22);
    static_assert([]() { return WrapColorTag("A", EmpireColor{1, 2, 3, 255}) == "<rgba 1 2 3 255>A</rgba>"; }());
#endif

    std::string EmpireColorWrappedText(const auto& empire, std::string_view text)
    { return WrapColorTag(text, empire ? empire->Color() : EmpireColor{{80, 255, 128, 255}}); }

    std::string EmpireColorWrappedText(int empire_id, std::string_view text, const ScriptingContext& context)
    { return EmpireColorWrappedText(context.GetEmpire(empire_id), text); }

    /// Creates a link tag of the appropriate type for object_id,
    /// with the content being the public name from the point of view of empire_id.
    /// Returns UserString("ENC_COMBAT_UNKNOWN_OBJECT") if object_id is not found.
    std::string PublicNameLink(int empire_id, const auto& object, const Universe& u) {
        const auto& name = object.PublicName(empire_id, u);
        const auto tag = LinkTag(object.ObjectType());
        return WrapWithTagAndId(name, tag, object.ID());
    }

    std::string PublicNameLink(int empire_id, int object_id, const Universe& u) {
        if (const auto* object = u.Objects().getRaw(object_id))
            return PublicNameLink(empire_id, *object, u);
        else
            return UserString("ENC_COMBAT_UNKNOWN_OBJECT");
    }

    std::string EmpireColouredFighterLink(int empire_id, const ScriptingContext& context)
    { return EmpireColorWrappedText(empire_id, UserString("OBJ_FIGHTER"), context); }

    /// Creates a link tag of the appropriate type for either a fighter or another object.
    std::string FighterOrPublicNameLink(int viewing_empire_id, int object_id,
                                        int object_empire_id, const ScriptingContext& context)
    {
        if (object_id >= 0)   // ship
            return PublicNameLink(viewing_empire_id, object_id, context.ContextUniverse());
        else                  // fighter
            return EmpireColouredFighterLink(object_empire_id, context);
    }

    std::string EmpireLink(int empire_id, const ScriptingContext& context) {
        if (empire_id == ALL_EMPIRES) {
            return UserString("NEUTRAL");
        } else if (auto empire = context.GetEmpire(empire_id)) {
            return EmpireColorWrappedText(empire,
                                          WrapWithTagAndId(empire->Name(), VarText::EMPIRE_ID_TAG, empire_id));
        } else {
            return UserString("ENC_COMBAT_UNKNOWN_OBJECT");
        }
    }

    std::string ShipPartLink(std::string_view part) {
        return part.empty() ?
            UserString("ENC_COMBAT_UNKNOWN_OBJECT") : WrapUserStringWithTag(part, VarText::SHIP_PART_TAG);
    }


    constexpr auto not_null = [](const auto& o) noexcept -> bool { return !!o; };
}


/////////////////////////////////////
/////////// BoutEvent ///////////////
/////////////////////////////////////
std::string BoutEvent::DebugString(const ScriptingContext&) const {
    std::stringstream ss;
    ss << "Bout " << bout << " has " << weapon_firings.SubEvents(ALL_EMPIRES).size() << " weapon firings, "
       << weapons_platform_firings.SubEvents(ALL_EMPIRES).size() << " weapon platform firings, "
       << fighter_launches.SubEvents(ALL_EMPIRES).size() << " launches, "
       << fighter_launches2.SubEvents(ALL_EMPIRES).size() << " launches, "
       << fighters_destroyed.SubEvents(ALL_EMPIRES).size() << " fighters destroyed, "
       << fighters_attack_fighters.SubEvents(ALL_EMPIRES).size() << " fighters on fighter attack, "
       << ship_incapacitations.SubEvents(ALL_EMPIRES).size() << " ship incapacitations, "
       << planet_incapacitations.SubEvents(ALL_EMPIRES).size() << " planet incapacitations, "
       << other_incapacitations.SubEvents(ALL_EMPIRES).size() << " other incapacitations";
    return ss.str();
}

std::string BoutEvent::CombatLogDescription(int, const ScriptingContext&) const
{ return str(FlexibleFormat(UserString("ENC_ROUND_BEGIN")) % bout); }

std::vector<const CombatEvent*> BoutEvent::SubEvents(int) const {
    std::vector<const CombatEvent*> retval;
    retval.reserve(8);
    for (const auto* subevent : std::array<const CombatEvent*, 9>{
        std::addressof(weapon_firings),         std::addressof(weapons_platform_firings),
        std::addressof(fighter_launches),       std::addressof(fighter_launches2),
        std::addressof(fighters_attack_fighters),
        std::addressof(fighters_destroyed),     std::addressof(ship_incapacitations),
        std::addressof(planet_incapacitations), std::addressof(other_incapacitations)})
    {
        if (subevent && !subevent->IsEmpty())
            retval.push_back(subevent);
    }
    return retval;
}


//////////////////////////////////////////
///////// SimultaneousEvents /////////////
//////////////////////////////////////////
std::string SimultaneousEvents::DebugString(const ScriptingContext&) const
{ return "SimultaneousEvents has " + std::to_string(events.size()) + " events"; }

std::vector<const CombatEvent*> SimultaneousEvents::SubEvents(int viewing_empire_id) const {
    // Sort the events by viewing empire, then ALL_EMPIRES, and then other empires.
    const auto to_faction_event = [viewing_empire_id](const auto& event) -> std::pair<int, const CombatEvent*> {
        auto fac = event->PrincipalFaction(viewing_empire_id);
        return std::pair{fac.has_value() ? *fac : ALL_EMPIRES, event.get()};
    };

    auto empire_to_event = events | range_filter(not_null) | range_transform(to_faction_event) | range_to_vec;

    // put viewer events first
    const auto viewer_events_end_it = std::stable_partition(empire_to_event.begin(), empire_to_event.end(),
                                                            [viewing_empire_id](const auto& e) { return e.first == viewing_empire_id; });
    // sort remaining events by viewer id. ALL_EMPIRES should be lower than other empire IDs so appears next
    std::stable_sort(viewer_events_end_it, empire_to_event.end(),
                     [](const auto& lhs, const auto& rhs) noexcept { return lhs.first < rhs.first; });

    return empire_to_event | range_values | range_to_vec;
}

//////////////////////////////////////////
///////// InitialStealthEvent ////////////
//////////////////////////////////////////
std::string InitialStealthEvent::DebugString(const ScriptingContext& context) const {
    auto get_obj_id_owner = [&context](const auto id) -> std::pair<int, int> {
        const auto* obj = context.ContextObjects().getRaw(id);
        return obj ? std::pair(obj->ID(), obj->Owner()) : std::pair(INVALID_OBJECT_ID, ALL_EMPIRES);
    };

    static constexpr auto is_empire_id = [](const auto& x_oid) noexcept { return x_oid.second != ALL_EMPIRES; };

    std::stringstream ss;
    ss << "InitialStealthEvent: ";
    for (auto& [empire_id, empire_object_vis] : empire_object_visibility) {
        ss << " Viewing Empire: " << EmpireLink(empire_id, context) << "\n";

        for (const auto [object_id, owner_id] : empire_object_vis
             | range_keys | range_transform(get_obj_id_owner) | range_filter(is_empire_id))
        { ss << FighterOrPublicNameLink(ALL_EMPIRES, object_id, owner_id, context); }
        ss << "\n";
    }
    return ss.str();
}

std::string InitialStealthEvent::CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const {
    DebugLogger() << "CombatLogDescription for InitialStealthEvent viewing empire empire: " << viewing_empire_id;

    std::string desc;

    for (auto& [detector_empire_id, visible_objects] : empire_object_visibility) {
        DebugLogger() << "CombatLogDescription for InitialStealthEvent for detector empire: " << detector_empire_id;

        if (visible_objects.empty()) {
            DebugLogger() << " ... no object info recorded for detector empire: " << detector_empire_id;
            continue;
        }

        // Check Visibility of objects, report those that are not visible.
        std::vector<std::string> cloaked_attackers;
        for (auto& [object_id, object_vis] : visible_objects) {
            const auto obj = context.ContextObjects().get(object_id);
            const auto& name = obj ? obj->Name() : UserString("UNKNOWN");
            DebugLogger() << " ... object: " << name << " (" << object_id << ") has vis: " << object_vis;
            if (object_vis > Visibility::VIS_NO_VISIBILITY)
                continue;

            // ignore fleets from mention of being initially unable to detect - individual ships are mentioned already
            if (obj && obj->ObjectType() == UniverseObjectType::OBJ_FLEET)
                continue;

            // all empires specifies empire to use for link color if this is a fighter
            cloaked_attackers.push_back(FighterOrPublicNameLink(
                viewing_empire_id, object_id, ALL_EMPIRES, context));
        }

        if (!cloaked_attackers.empty()) {
            desc += "\n"; //< Add \n at start of the report and between each empire
            if (detector_empire_id != ALL_EMPIRES) {
                std::vector<std::string> detector_empire_link{EmpireLink(detector_empire_id, context)};
                desc += FlexibleFormatList(detector_empire_link, cloaked_attackers,
                                           UserString("ENC_COMBAT_INITIAL_STEALTH_LIST")).str();
            } else {
                desc += FlexibleFormatList(cloaked_attackers, UserString("ENC_COMBAT_NEUTRAL_INITIAL_STEALTH_LIST")).str();
            }
        }
    }

    return desc;
}


//////////////////////////////////////////
///////// StealthChangeEvent /////////////
//////////////////////////////////////////
std::string StealthChangeEvent::StealthChangeEventDetail::DebugString(const ScriptingContext& context) const {
    std::stringstream ss;
    ss << "StealthChangeDetailEvent " << (is_fighter_launch ? " observer: " : " target: ")
       << target_observer_empire_id << " "
       << FighterOrPublicNameLink(ALL_EMPIRES, attacker_id, attacker_empire_id, context)
       << " -> " << visibility << " ";
    return ss.str();
}

std::string StealthChangeEvent::StealthChangeEventDetail::CombatLogDescription(
    int viewing_empire_id, const ScriptingContext& context) const
{
    std::string attacker_link = FighterOrPublicNameLink(viewing_empire_id, attacker_id, attacker_empire_id, context);
    std::string target_empire_link = EmpireLink(target_observer_empire_id, context);

    if (is_fighter_launch) {
        const std::string& template_str = UserString("ENC_COMBAT_STEALTH_DECLOAK_LAUNCH");
        return str(FlexibleFormat(template_str)
                   % attacker_link
                   % target_empire_link);

    } else {
        std::string target_link = FighterOrPublicNameLink(viewing_empire_id, target_id, target_observer_empire_id, context);
        const std::string& template_str = UserString("ENC_COMBAT_STEALTH_DECLOAK_ATTACK");
        return str(FlexibleFormat(template_str)
                   % attacker_link
                   % target_link
                   % target_empire_link);
    }
}

std::string StealthChangeEvent::DebugString(const ScriptingContext& context) const {
    std::stringstream ss;
    ss << "StealthChangeEvent: ";
    for (const auto& event : events)
        ss << event.DebugString(context);
    return ss.str();
}

std::string StealthChangeEvent::CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const {
    std::string desc;
    if (events.empty())
        return desc;

    std::map<int, std::vector<std::string>> target_empire_uncloaked_attackers;

    for (const auto& event : events) {
        target_empire_uncloaked_attackers[event.target_observer_empire_id].push_back(
            FighterOrPublicNameLink(viewing_empire_id, event.attacker_id, event.attacker_empire_id, context));
    }

    for (const auto& [toeid, attacker_links] : target_empire_uncloaked_attackers) {
        if (attacker_links.empty())
            continue;
        if (!desc.empty())
            desc += "\n";
        std::vector<std::string> target_empire_link{EmpireLink(toeid, context)};

        desc += FlexibleFormatList(target_empire_link, attacker_links,
                                   UserString("ENC_COMBAT_STEALTH_DECLOAK_ATTACK_MANY_EVENTS"),
                                   UserString("ENC_COMBAT_STEALTH_DECLOAK_ATTACK_1_EVENTS")).str();
    }

    return desc;
}

std::vector<const CombatEvent*> StealthChangeEvent::SubEvents(int) const {
    static constexpr auto to_combatevent_ptr = [](const StealthChangeEventDetail& p) noexcept -> const CombatEvent*
    { return std::addressof(p); };
    return events | range_transform(to_combatevent_ptr) | range_to_vec;
}


//////////////////////////////////////////
///////// Attack Event////////////////////
//////////////////////////////////////////
std::string WeaponFireEvent::DebugString(const ScriptingContext&) const {
    std::stringstream ss;
    ss << attacker_id << " -> " << target_id << " : " << weapon_name << " "
       << power << " - " << shield << " = " << damage << "   attacker owner: " << attacker_owner_id;
    return ss.str();
}

std::string WeaponFireEvent::CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const {
    std::string attacker_link = FighterOrPublicNameLink(viewing_empire_id, attacker_id, attacker_owner_id, context);
    std::string target_link = FighterOrPublicNameLink(viewing_empire_id, target_id, target_owner_id, context);

    const std::string& template_str = UserString("ENC_COMBAT_ATTACK_STR");

    return str(FlexibleFormat(template_str)
               % attacker_link
               % target_link
               % damage);
}

std::string WeaponFireEvent::CombatLogDetails(int) const {
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


///////////////////////////////////////////
////////// IncapacitationsEvent ///////////
///////////////////////////////////////////
namespace {
    constexpr auto to_combatevent_ptr = [](const auto& p) noexcept -> const CombatEvent* { return std::addressof(p); };
    constexpr auto to_second_size = [](const auto& id_vec) noexcept { return id_vec.second.size(); };

    std::vector<const CombatEvent*> ExtractedNestedSubEvents(auto& events) {
        auto sizes_rng = events | range_transform(to_second_size);
        const std::size_t count = std::accumulate(sizes_rng.begin(), sizes_rng.end(), std::size_t{0});

        std::vector<const CombatEvent*> retval;
        retval.reserve(count);

        for (const auto& vec : events | range_values) {
            auto subevents_rng = vec | range_transform(to_combatevent_ptr);
            retval.insert(retval.end(), subevents_rng.begin(), subevents_rng.end());
        }

        return retval;
    }

    std::string NestedCountDescription(const auto& events, UniverseObjectType objects_type) {
        if (objects_type == UniverseObjectType::NUM_OBJ_TYPES || events.empty())
            return "";

        auto sizes_rng = events | range_transform(to_second_size);
        const std::size_t count = std::accumulate(sizes_rng.begin(), sizes_rng.end(), std::size_t{0});

        const std::string_view msg =
            (objects_type == UniverseObjectType::OBJ_PLANET) ?  "ENC_COMBAT_MANY_INCAPACITATED_PLANETS" :
            (objects_type == UniverseObjectType::OBJ_SHIP) ?    "ENC_COMBAT_MANY_DESTROYED_SHIPS" :
            (objects_type == UniverseObjectType::OBJ_FIGHTER) ? "ENC_COMBAT_MANY_DESTROYED_FIGHTERS" :
                                                                "ENC_COMBAT_MANY_DESTROYED_MISC";
        return str(FlexibleFormat(UserString(msg)) % count);
    }

    std::string NestedVecsDebugString(const auto& events, std::string_view string_key) {
        std::string retval;
        retval.reserve(50 * events.size()); // guesstimate
        retval.append(string_key);
        for (const auto& [owner_id, vec] : events) {
            retval.append("  ").append(std::to_string(owner_id)).append(":");
            for (const auto& id : vec)
                retval.append(" ").append(std::to_string(static_cast<int>(id)));
        }
        return retval;
    }

    template <typename ObjectType = UniverseObject>
    std::string EmpireNameIncapacitationDescription(int viewing_empire_id, int object_id,
                                                    const ScriptingContext& context)
    {
        static_assert(std::is_base_of_v<UniverseObject, ObjectType>);

        const ObjectType* const object = [object_id, &context]() {
            if constexpr (std::is_same_v<ObjectType, Fighter>)
                return dynamic_cast<const Fighter*>(context.ContextObjects().getRaw(object_id));
            else
                return context.ContextObjects().getRaw<ObjectType>(object_id);
        }();
        if (!object)
            return UserString("ENC_COMBAT_UNKNOWN_OBJECT");

        auto object_name = PublicNameLink(viewing_empire_id, *object, context.ContextUniverse());
        auto owner_name = EmpireLink(object->Owner(), context);

        const std::string_view template_str = [object]() {
            if constexpr (std::is_same_v<ObjectType, Ship>) {
                return "ENC_COMBAT_DESTROYED_STR";
            } else if constexpr (std::is_same_v<ObjectType, Planet>) {
                return "ENC_COMBAT_PLANET_INCAPACITATED_STR";
            } else if constexpr (std::is_same_v<ObjectType, Fighter>) {
                return "ENC_COMBAT_FIGHTER_INCAPACITATED_STR";
            } else {
                switch (object->ObjectType()) {
                case UniverseObjectType::OBJ_SHIP:    return "ENC_COMBAT_DESTROYED_STR";
                case UniverseObjectType::OBJ_PLANET:  return "ENC_COMBAT_PLANET_INCAPACITATED_STR";
                case UniverseObjectType::OBJ_FIGHTER: return "ENC_COMBAT_FIGHTER_INCAPACITATED_STR";
                default:                              return "ENC_COMBAT_UNKNOWN_DESTROYED_STR";
                }
            }
        }();

        return str(FlexibleFormat(UserString(template_str)) % owner_name % object_name);
    }

    void AddDetailEvent(std::vector<std::pair<int, std::vector<IncapacitationsEvent::IncapacitationDetail>>>& events,
                        int object_id, int owner_id, UniverseObjectType obj_type)
    {
        const auto is_owner = [owner_id](const auto& owner_vec) noexcept { return owner_vec.first == owner_id; };
        auto it = std::find_if(events.begin(), events.end(), is_owner);
        if (it == events.end()) {
            auto& vec = events.emplace_back(std::piecewise_construct,
                                            std::forward_as_tuple(owner_id),
                                            std::forward_as_tuple()).second;
            vec.emplace_back(object_id, obj_type);
        } else {
            auto& vec = it->second;
            vec.emplace_back(object_id, obj_type);
        }
    }
}

std::string IncapacitationsEvent::CombatLogDescription(int, const ScriptingContext&) const
{ return NestedCountDescription(events, UniverseObjectType::NUM_OBJ_TYPES); } // pass in this->objects_type for count + type indication

std::string IncapacitationsEvent::DebugString(const ScriptingContext&) const {
    const std::string_view msg =
        (objects_type == UniverseObjectType::OBJ_PLANET) ?  "planet incapacitations for empires:" :
        (objects_type == UniverseObjectType::OBJ_SHIP) ?    "ship destructions for empires:" :
        (objects_type == UniverseObjectType::OBJ_FIGHTER) ? "fighter destructions for empires:" :
                                                            "incapacitations for empires:";
    return NestedVecsDebugString(events, msg);
}

std::vector<const CombatEvent*> IncapacitationsEvent::SubEvents(int) const
{ return ExtractedNestedSubEvents(events); }

void IncapacitationsEvent::AddEvent(int object_id, int owner_id)
{ AddDetailEvent(events, object_id, owner_id, objects_type); }

std::string IncapacitationsEvent::IncapacitationDetail::CombatLogDescription(
    int viewing_empire_id, const ScriptingContext& context) const
{
    switch (object_type) {
    case UniverseObjectType::OBJ_PLANET:  return EmpireNameIncapacitationDescription<Planet>(viewing_empire_id, id, context);
    case UniverseObjectType::OBJ_SHIP:    return EmpireNameIncapacitationDescription<Ship>(viewing_empire_id, id, context);
    case UniverseObjectType::OBJ_FIGHTER: return EmpireNameIncapacitationDescription<Fighter>(viewing_empire_id, id, context);
    default:                              return EmpireNameIncapacitationDescription<UniverseObject>(viewing_empire_id, id, context);
    }    
}


//////////////////////////////////////////
///////// FightersAttackFightersEvent ////
//////////////////////////////////////////
std::string FightersAttackFightersEvent::DebugString(const ScriptingContext&) const {
    std::stringstream ss;
    ss << "FightersAttackFightersEvent: ";
    for (auto& [index, event]: events) {
        ss << event << " repeated fighters from empire " << index.first
           << " attacking fighters from empire " << index.second << ", ";
    }
    return ss.str();
}

std::string FightersAttackFightersEvent::CombatLogDescription(int viewing_empire_id,
                                                              const ScriptingContext& context) const {
    if (events.empty())
        return "";

    const auto& events_to_show = events;
    auto num_events_remaining = events.size();
    std::stringstream ss;

    // Use show_events_for_empire to show events in this order: viewing empire, ALL_EMPIRES and
    // then the remainder.
    auto show_events_for_empire =
        [&ss, &num_events_remaining, &events_to_show, &viewing_empire_id, &context]
        (std::optional<int> show_attacker)
    {
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
                const auto attacker_link = FighterOrPublicNameLink(
                    viewing_empire_id, INVALID_OBJECT_ID, attacker_empire, context);
                const auto target_link = FighterOrPublicNameLink(
                    viewing_empire_id, INVALID_OBJECT_ID, target_empire, context);
                const auto& template_str = UserString("ENC_COMBAT_ATTACK_REPEATED_STR");

                ss << str(FlexibleFormat(template_str) % count % attacker_link % target_link);
                if (--num_events_remaining > 0)
                    ss << "\n";
            }
        };

    // Sort the events by viewing empire, then ALL_EMPIRES and then other empires.
    show_events_for_empire(viewing_empire_id);
    show_events_for_empire(ALL_EMPIRES);
    show_events_for_empire(std::nullopt);

    return ss.str();
}


//////////////////////////////////////////
/////////// FighterLaunchEvent ///////////
//////////////////////////////////////////
std::string FighterLaunchEvent::DebugString(const ScriptingContext&) const {
    std::stringstream ss;
    ss << "launch from object " << launched_from_id
       << " of " << number_launched
       << " fighter(s) of empire " << fighter_owner_empire_id;
    return ss.str();
}

namespace {
    std::string FighterLaunchLogDescription(int viewing_empire_id, int launched_from_id,
                                            int owner_empire_id, int number_launched,
                                            const ScriptingContext& context)
    {
        std::string launched_from_link = PublicNameLink(viewing_empire_id, launched_from_id,
                                                        context.ContextUniverse());
        std::string empire_coloured_fighter = EmpireColorWrappedText(
            owner_empire_id, UserString("OBJ_FIGHTER"), context);

        // launching negative fighters indicates recovery of them by the ship
        const std::string& template_str = (number_launched >= 0 ?
                                           UserString("ENC_COMBAT_LAUNCH_STR") :
                                           UserString("ENC_COMBAT_RECOVER_STR"));

        return str(FlexibleFormat(template_str)
                   % launched_from_link
                   % empire_coloured_fighter
                   % std::abs(number_launched));
    }
}

std::string FighterLaunchEvent::CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const
{ return FighterLaunchLogDescription(viewing_empire_id, launched_from_id, fighter_owner_empire_id, number_launched, context); }


////////////////////////////////////////////
/////////// FighterLaunchesEvent ///////////
////////////////////////////////////////////
namespace {
    auto TallyEmpireLaunches(const std::vector<std::pair<int, std::vector<FighterLaunchesEvent::FighterLaunchDetail>>>& events) {
        static constexpr auto to_count_sum = [](const std::vector<FighterLaunchesEvent::FighterLaunchDetail>& details) {
            auto counts_rng = details | range_transform([](const auto& d) noexcept { return d.count; });
            return std::accumulate(counts_rng.begin(), counts_rng.end(), 0);
        };

        static constexpr auto to_eid_sum = [](const auto& eid_vec) noexcept
        { return std::pair{eid_vec.first, to_count_sum(eid_vec.second)}; };

        return events | range_transform(to_eid_sum) | range_to_vec;
    }
}

std::string FighterLaunchesEvent::DebugString(const ScriptingContext&) const {
    const auto empire_tallies = TallyEmpireLaunches(events);
    std::string retval = "empire fighter launches:";
    for (auto& [eid, count] : empire_tallies)
        retval.append("  ").append(std::to_string(eid)).append(": ").append(std::to_string(count));
    return retval;
}

std::string FighterLaunchesEvent::CombatLogDescription(int, const ScriptingContext& context) const {
    std::string retval;
    if (events.empty())
        return retval;

    const auto empire_tallies = TallyEmpireLaunches(events);
    if (empire_tallies.empty())
        return retval;

    const auto& launch_str = UserString("ENC_COMBAT_EMPIRE_LAUNCHES_FIGHTERS");
    const auto& recover_str = UserString("ENC_COMBAT_EMPIRE_RECOVERS_FIGHTERS");

    for (auto& [eid, count] : empire_tallies) {
        if (count == 0)
            continue;
        if (!retval.empty())
            retval.append("\n");

        const auto empire_link = EmpireLink(eid, context);
        const auto fighter_link = EmpireColouredFighterLink(eid, context);
        const auto& template_str = (count > 0) ? launch_str : recover_str;

        retval.append(str(FlexibleFormat(template_str) % empire_link % std::abs(count) % fighter_link));
    }

    return retval;
}

std::vector<const CombatEvent*> FighterLaunchesEvent::SubEvents(int) const
{ return ExtractedNestedSubEvents(events); }

void FighterLaunchesEvent::AddEvent(int from_id, int fighter_owner_empire_id, int count) {
    const auto is_empire = [fighter_owner_empire_id](const auto& eid_vec) noexcept { return eid_vec.first == fighter_owner_empire_id; };
    auto empire_it = std::find_if(events.begin(), events.end(), is_empire);
    if (empire_it == events.end()) {
        auto& vec = events.emplace_back(std::piecewise_construct,
                                        std::forward_as_tuple(fighter_owner_empire_id),
                                        std::forward_as_tuple()).second;
        vec.emplace_back(from_id, count);
    } else {
        auto& vec = empire_it->second;
        vec.emplace_back(from_id, count);
    }
}

std::string FighterLaunchesEvent::FighterLaunchDetail::CombatLogDescription(
    int viewing_empire_id, const ScriptingContext& context) const
{
    const auto* launcher_obj = context.ContextObjects().getRaw(from_id);
    const int launcher_empire_id = launcher_obj ? launcher_obj->Owner() : ALL_EMPIRES;
    return FighterLaunchLogDescription(viewing_empire_id, from_id, launcher_empire_id, count, context);
}


//////////////////////////////////////////
///////// FightersDestroyedEvent ////
//////////////////////////////////////////
std::string FightersDestroyedEvent::DebugString(const ScriptingContext&) const {
    std::stringstream ss;
    ss << "FightersDestroyedEvent: ";
    for (auto& [empire_id, count] : events)
        ss << count << " repeated fighters from empire " << empire_id << " destroyed.";
    return ss.str();
}

std::string FightersDestroyedEvent::CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const {
    if (events.empty())
        return "";

    const auto& events_to_show = events;
    auto num_events_remaining = events.size();
    std::stringstream ss;

    // Use show_events_for_empire to show events in this order: viewing empire,
    // ALL_EMPIRES and then the remainder.
    auto show_events_for_empire =
        [&ss, &num_events_remaining, &events_to_show, &viewing_empire_id, &context]
        (std::optional<int> show_empire_id)
    {
            for (auto& [target_empire_id, count] : events_to_show) {
                // Skip if this is not the particular attacker requested
                if (show_empire_id && *show_empire_id != target_empire_id)
                    continue;

                // Skip if no particular empire was requested and this empire is the viewing
                // empire or ALL_EMPIRES
                if (!show_empire_id && (target_empire_id == viewing_empire_id || target_empire_id == ALL_EMPIRES))
                    continue;

                auto count_str{std::to_string(count)};
                auto target_empire_link{EmpireLink(target_empire_id, context)};
                const auto target_link{FighterOrPublicNameLink(
                    viewing_empire_id, INVALID_OBJECT_ID, target_empire_id, context)};

                if (count == 1) {
                    const std::string& template_str = UserString("ENC_COMBAT_FIGHTER_INCAPACITATED_STR");
                    ss << str(FlexibleFormat(template_str) % target_empire_link % target_link);
                } else {
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
    show_events_for_empire(std::nullopt);

    return ss.str();
}


//////////////////////////////////////////
///////// WeaponsPlatformEvent /////////////
//////////////////////////////////////////
std::string WeaponsPlatformEvent::DebugString(const ScriptingContext& context) const {
    std::stringstream desc;
    desc << "WeaponsPlatformEvent attacker_id = " << attacker_id << " attacker_owner = " << attacker_owner_id;
    for (auto& event_vec : events | range_values) {
        for (const auto& attack : event_vec)
            desc << "\n" << attack.DebugString(context);
    }
    return desc.str();
}

std::string WeaponsPlatformEvent::CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const {
    if (events.empty())
        return "";

    std::vector<std::string> damaged_target_links;
    std::vector<std::string> undamaged_target_links;
    damaged_target_links.reserve(events.size()); // underestimate
    undamaged_target_links.reserve(events.size()); // underestimate

    for (const auto& [target_id, weapon_fire_events] : events) {
        if (weapon_fire_events.empty())
            continue;

        std::string target_public_name{
            FighterOrPublicNameLink(viewing_empire_id, target_id, weapon_fire_events.front().target_owner_id, context)};

        double damage = 0.0f;
        for (const auto& weapon_fire_event : weapon_fire_events)
            damage += weapon_fire_event.damage;

        if (damage <= 0.0f) {
            undamaged_target_links.push_back(std::move(target_public_name));
        } else {
            damaged_target_links.push_back(
                str(FlexibleFormat(UserString("ENC_COMBAT_PLATFORM_TARGET_AND_DAMAGE"))
                    % target_public_name % damage));
        }
    }


    const std::vector<std::string> attacker_link{FighterOrPublicNameLink(
        viewing_empire_id, attacker_id, attacker_owner_id, context)};

    std::string desc;
    desc.reserve(200); // guesstimate based on test game resulting size

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

std::vector<const CombatEvent*> WeaponsPlatformEvent::SubEvents(int) const {
    auto sz_rng = events | range_values | range_transform([](const auto& c) noexcept { return c.size(); });
    const auto sz_sum = std::accumulate(sz_rng.begin(), sz_rng.end(), std::size_t{0});
    std::vector<const CombatEvent*> retval;
    retval.reserve(sz_sum);

    for (const auto& sub_event_vec : events | range_values) {
        auto raw_sub_events_rng = sub_event_vec | range_transform(to_combatevent_ptr);
        retval.insert(retval.end(), raw_sub_events_rng.begin(), raw_sub_events_rng.end());
    }

    return retval;
}
