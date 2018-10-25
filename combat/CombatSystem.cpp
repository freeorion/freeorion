#include "CombatSystem.h"
#include "CombatEvents.h"
#include "CombatTargetting.h"

#include "../universe/Universe.h"
#include "../util/GameRules.h"
#include "../util/OptionsDB.h"
#include "../universe/Predicates.h"
#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Fighter.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Enums.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/i18n.h"

#include "../network/Message.h"

#include <iterator>
#include <sstream>

namespace {
    DeclareThreadSafeLogger(combat);
}

////////////////////////////////////////////////
// CombatInfo
////////////////////////////////////////////////
CombatInfo::CombatInfo() :
    turn(INVALID_GAME_TURN),
    system_id(INVALID_OBJECT_ID)
{}

CombatInfo::CombatInfo(int system_id_, int turn_) :
    turn(turn_),
    system_id(system_id_)
{
    auto system = ::GetSystem(system_id);
    if (!system) {
        ErrorLogger() << "CombatInfo constructed with invalid system id: " << system_id;
        return;
    }
    // add system to full / complete objects in combat - NOTE: changed from copy of system
    objects.Insert(system);


    // find ships and their owners in system
    auto ships = Objects().FindObjects<Ship>(system->ShipIDs());

    for (auto& ship : ships) {
        // add owner to empires that have assets in this battle
        empire_ids.insert(ship->Owner());

        objects.Insert(ship);
    }

    // find planets and their owners in system
    auto planets = Objects().FindObjects<Planet>(system->PlanetIDs());

    for (auto& planet : planets) {
        // if planet is populated or has an owner, add owner to empires that have assets in this battle
        if (!planet->Unowned() || planet->InitialMeterValue(METER_POPULATION) > 0.0f)
            empire_ids.insert(planet->Owner());

        objects.Insert(planet);
    }

    // TODO: should buildings be considered separately?

    // now that all participants in the battle have been found, loop through
    // objects again to assemble each participant empire's latest
    // known information about all objects in this battle

    InitializeObjectVisibility();

    // ships
    for (auto& ship : ships) {
        int ship_id = ship->ID();
        auto fleet = GetFleet(ship->FleetID());
        if (!fleet) {
            ErrorLogger() << "CombatInfo::CombatInfo couldn't get fleet with id "
                                   << ship->FleetID() << " in system " << system->Name() << " (" << system_id << ")";
            continue;
        }

        std::string ship_known = "At " + system->Name() + " Ship " + std::to_string(ship_id) + " owned by empire " +
                                    std::to_string(ship->Owner()) + " visible to empires: ";
        for (int empire_id : empire_ids) {
            if (empire_id == ALL_EMPIRES)
                continue;
            if (GetUniverse().GetObjectVisibilityByEmpire(ship_id, empire_id) > VIS_BASIC_VISIBILITY ||
                   (fleet->Aggressive() &&
                       (empire_id == ALL_EMPIRES ||
                        fleet->Unowned() ||
                        Empires().GetDiplomaticStatus(empire_id, fleet->Owner()) == DIPLO_WAR)))
            {
                empire_known_objects[empire_id].Insert(GetEmpireKnownShip(ship->ID(), empire_id));}
                ship_known += std::to_string(empire_id) + ", ";
        }
        DebugLogger(combat) << ship_known;
    }

    // planets
    for (auto& planet : planets) {
        int planet_id = planet->ID();
        std::string planet_known = "At " + system->Name() + " Planet " + std::to_string(planet_id) + " owned by empire " +
                                    std::to_string(planet->Owner()) + " visible to empires: ";

        for (int empire_id : empire_ids) {
            if (empire_id == ALL_EMPIRES)
                continue;
            if (GetUniverse().GetObjectVisibilityByEmpire(planet_id, empire_id) > VIS_BASIC_VISIBILITY) {
                empire_known_objects[empire_id].Insert(GetEmpireKnownPlanet(planet->ID(), empire_id));
                planet_known += std::to_string(empire_id) + ", ";
            }
        }
        DebugLogger(combat) << planet_known;
    }

    // after battle is simulated, any changes to latest known or actual objects
    // will be copied back to the main Universe's ObjectMap and the Universe's
    // empire latest known objects ObjectMap - NOTE: Using the real thing now
}

std::shared_ptr<const System> CombatInfo::GetSystem() const
{ return this->objects.Object<System>(this->system_id); }

std::shared_ptr<System> CombatInfo::GetSystem()
{ return this->objects.Object<System>(this->system_id); }

void CombatInfo::GetEmpireIdsToSerialize(std::set<int>& filtered_empire_ids, int encoding_empire) const {
    if (encoding_empire == ALL_EMPIRES) {
        filtered_empire_ids = this->empire_ids;
        return;
    }
    // TODO: include only empires that the encoding empire knows are present in the system / battle
    filtered_empire_ids = this->empire_ids; // for now, include all empires involved in battle
}

void CombatInfo::GetObjectsToSerialize(ObjectMap& filtered_objects, int encoding_empire) const {
    if (&filtered_objects == &this->objects)
        return;

    filtered_objects.Clear();

    if (encoding_empire == ALL_EMPIRES) {
        filtered_objects = this->objects;
        return;
    }
    // TODO: include only objects that the encoding empire has visibility of
    //       using the combat visibility system.
    filtered_objects = this->objects;       // for now, include all objects in battle / system
}

void CombatInfo::GetEmpireKnownObjectsToSerialize(std::map<int, ObjectMap>& filtered_empire_known_objects,
                                                  int encoding_empire) const
{
    if (&filtered_empire_known_objects == &this->empire_known_objects)
        return;

    for (auto& entry : filtered_empire_known_objects)
    { entry.second.Clear(); }
    filtered_empire_known_objects.clear();

    if (encoding_empire == ALL_EMPIRES) {
        filtered_empire_known_objects = this->empire_known_objects;
        return;
    }

    // include only latest known objects for the encoding empire
    auto it = this->empire_known_objects.find(encoding_empire);
    if (it != this->empire_known_objects.end()) {
        const ObjectMap& map = it->second;
        filtered_empire_known_objects[encoding_empire].Copy(map, ALL_EMPIRES);
    }
}

void CombatInfo::GetDamagedObjectsToSerialize(std::set<int>& filtered_damaged_objects,
                                              int encoding_empire) const
{
    if (encoding_empire == ALL_EMPIRES) {
        filtered_damaged_objects = this->damaged_object_ids;
        return;
    }
    // TODO: decide if some filtering is needed for damaged objects... it may not be.
    filtered_damaged_objects = this->damaged_object_ids;
}

void CombatInfo::GetDestroyedObjectsToSerialize(std::set<int>& filtered_destroyed_objects,
                                                int encoding_empire) const
{
    if (encoding_empire == ALL_EMPIRES) {
        filtered_destroyed_objects = this->destroyed_object_ids;
        return;
    }
    // TODO: decide if some filtering is needed for destroyed objects... it may not be.
    filtered_destroyed_objects = this->destroyed_object_ids;
}

void CombatInfo::GetDestroyedObjectKnowersToSerialize(std::map<int, std::set<int>>&
                                                      filtered_destroyed_object_knowers,
                                                      int encoding_empire) const
{
    if (encoding_empire == ALL_EMPIRES) {
        filtered_destroyed_object_knowers = this->destroyed_object_knowers;
        return;
    }
    // TODO: decide if some filtering is needed for which empires know about which
    // other empires know which objects have been destroyed during the battle.
    filtered_destroyed_object_knowers = this->destroyed_object_knowers;
}

void CombatInfo::GetCombatEventsToSerialize(std::vector<CombatEventPtr>& filtered_combat_events,
                                            int encoding_empire) const
{
    filtered_combat_events = this->combat_events;
}

void CombatInfo::GetEmpireObjectVisibilityToSerialize(Universe::EmpireObjectVisibilityMap&
                                                      filtered_empire_object_visibility,
                                                      int encoding_empire) const
{
    filtered_empire_object_visibility = this->empire_object_visibility;
}

/** Requires system_id, empire_ids are initialized*/
void CombatInfo::InitializeObjectVisibility()
{
    // system and empire visibility of all objects in it
    auto system = ::GetSystem(system_id);
    std::set< int > local_object_ids = system->ContainedObjectIDs();
    for (int empire_id : empire_ids) {
        if (empire_id == ALL_EMPIRES)
            continue;
        empire_known_objects[empire_id].Insert(GetEmpireKnownSystem(system->ID(), empire_id));
        empire_object_visibility[empire_id][system->ID()] = GetUniverse().GetObjectVisibilityByEmpire(system->ID(), empire_id);
        for (int object_id : local_object_ids) {
            Visibility obj_vis = GetUniverse().GetObjectVisibilityByEmpire(object_id, empire_id);
            if (obj_vis > VIS_NO_VISIBILITY)  // to ensure an empire doesn't wrongly get info that an object was present
                empire_object_visibility[empire_id][object_id] = obj_vis;
        }
    }
}

void CombatInfo::ForceAtLeastBasicVisibility(int attacker_id, int target_id)
{
    auto attacker = objects.Object(attacker_id);
    auto target = objects.Object(target_id);
    // Also ensure that attacker (and their fleet if attacker was a ship) are
    // revealed with at least BASIC_VISIBILITY to the target empire
    Visibility old_visibility = empire_object_visibility[target->Owner()][attacker->ID()];
    Visibility new_visibility = std::max(old_visibility, VIS_BASIC_VISIBILITY);
    empire_object_visibility[target->Owner()][attacker->ID()] = new_visibility;
    if (attacker->ObjectType() == OBJ_SHIP && attacker->ContainerObjectID() != INVALID_OBJECT_ID) {
        empire_object_visibility[target->Owner()][attacker->ContainerObjectID()] =
            std::max(empire_object_visibility[target->Owner()][attacker->ContainerObjectID()], VIS_BASIC_VISIBILITY);
    }
}

////////////////////////////////////////////////
// AutoResolveCombat
////////////////////////////////////////////////
namespace {
    struct PartAttackInfo {
        PartAttackInfo(ShipPartClass part_class_, const std::string& part_name_,
                       float part_attack_, const ::Condition::ConditionBase* combat_targets_) :
            part_class(part_class_),
            part_type_name(part_name_),
            part_attack(part_attack_),
            combat_targets(combat_targets_),
            fighters_launched(0),
            fighter_damage(0.0f)
        {}
        PartAttackInfo(ShipPartClass part_class_, const std::string& part_name_,
                       int fighters_launched_, float fighter_damage_, const ::Condition::ConditionBase* combat_targets_) :
            part_class(part_class_),
            part_type_name(part_name_),
            part_attack(0.0f),
            combat_targets(combat_targets_),
            fighters_launched(fighters_launched_),
            fighter_damage(fighter_damage_)
        {}

        ShipPartClass   part_class;
        std::string     part_type_name;
        float           part_attack;        // for direct damage parts
        const ::Condition::ConditionBase* combat_targets;
        int             fighters_launched;  // for fighter bays, input value should be limited by ship available fighters to launch
        float           fighter_damage;     // for fighter bays, input value should be determined by ship fighter weapon setup

    };

    void AttackShipShip(std::shared_ptr<Ship> attacker, const PartAttackInfo& weapon,
                        std::shared_ptr<Ship> target, CombatInfo& combat_info,
                        int bout, int round,
                        WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        if (!attacker || !target) return;

        float power = weapon.part_attack;

        auto& damaged_object_ids = combat_info.damaged_object_ids;

        Meter* target_structure = target->UniverseObject::GetMeter(METER_STRUCTURE);
        if (!target_structure) {
            ErrorLogger() << "couldn't get target structure or shield meter";
            return;
        }

        Meter* target_shield = target->UniverseObject::GetMeter(METER_SHIELD);
        float shield = (target_shield ? target_shield->Current() : 0.0f);

        DebugLogger() << "AttackShipShip: attacker: " << attacker->Name()
                      << "weapon: " << weapon.part_type_name << " power: " << power
                      << "  target: " << target->Name() << " shield: " << target_shield->Current()
                      << " structure: " << target_structure->Current();

        float damage = std::max(0.0f, power - shield);

        if (damage > 0.0f) {
            target_structure->AddToCurrent(-damage);
            damaged_object_ids.insert(target->ID());
            DebugLogger(combat) << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << damage << " damage to Ship " << target->Name() << " (" << target->ID() << ")";
        }

        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.part_type_name, power, shield, damage);

        attacker->SetLastTurnActiveInCombat(CurrentTurn());
        target->SetLastTurnActiveInCombat(CurrentTurn());
    }

    void AttackShipPlanet(std::shared_ptr<Ship> attacker, const PartAttackInfo& weapon,
                          std::shared_ptr<Planet> target, CombatInfo& combat_info,
                          int bout, int round,
                          WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        if (!attacker || !target) return;
        float power = weapon.part_attack;
        if (power <= 0.0f)
            return;

        std::set<int>& damaged_object_ids = combat_info.damaged_object_ids;

        const ShipDesign* attacker_design = attacker->Design();
        if (!attacker_design)
            return;

        Meter* target_shield = target->GetMeter(METER_SHIELD);
        Meter* target_defense = target->UniverseObject::GetMeter(METER_DEFENSE);
        Meter* target_construction = target->UniverseObject::GetMeter(METER_CONSTRUCTION);
        if (!target_shield) {
            ErrorLogger() << "couldn't get target shield meter";
            return;
        }
        if (!target_defense) {
            ErrorLogger() << "couldn't get target defense meter";
            return;
        }
        if (!target_construction) {
            ErrorLogger() << "couldn't get target construction meter";
            return;
        }

        DebugLogger(combat) << "AttackShipPlanet: attacker: " << attacker->Name() << " power: " << power
                            << "\ntarget: " << target->Name() << " shield: " << target_shield->Current()
                            << " defense: " << target_defense->Current() << " infra: " << target_construction->Current();

        // damage shields, limited by shield current value and damage amount.
        // remaining damage, if any, above shield current value goes to defense.
        // remaining damage, if any, above defense current value goes to construction
        float shield_damage = std::min(target_shield->Current(), power);
        float defense_damage = 0.0f;
        float construction_damage = 0.0f;
        if (shield_damage >= target_shield->Current())
            defense_damage = std::min(target_defense->Current(), power - shield_damage);

        if (power > 0)
            damaged_object_ids.insert(target->ID());

        if (defense_damage >= target_defense->Current())
            construction_damage = std::min(target_construction->Current(),
                                           power - shield_damage - defense_damage);

        if (shield_damage >= 0) {
            target_shield->AddToCurrent(-shield_damage);
            DebugLogger(combat) << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does "
                                << shield_damage << " shield damage to Planet " << target->Name() << " ("
                                << target->ID() << ")";
        }
        if (defense_damage >= 0) {
            target_defense->AddToCurrent(-defense_damage);
            DebugLogger(combat) << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does "
                                << defense_damage << " defense damage to Planet " << target->Name() << " ("
                                << target->ID() << ")";
        }
        if (construction_damage >= 0) {
            target_construction->AddToCurrent(-construction_damage);
            DebugLogger(combat) << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does "
                                << construction_damage << " instrastructure damage to Planet " << target->Name()
                                << " (" << target->ID() << ")";
        }

        //TODO report the planet damage details more clearly
        float total_damage = shield_damage + defense_damage + construction_damage;
        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.part_type_name,
                               power, 0.0f, total_damage);

        attacker->SetLastTurnActiveInCombat(CurrentTurn());
        target->SetLastTurnAttackedByShip(CurrentTurn());
    }

    void AttackShipFighter(std::shared_ptr<Ship> attacker, const PartAttackInfo& weapon,
                           std::shared_ptr<Fighter> target, CombatInfo& combat_info,
                           int bout, int round, AttacksEventPtr& attacks_event,
                           WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        float power = weapon.part_attack;

        if (attacker->TotalWeaponsDamage(0.0f, false) > 0.0f) {
            // any damage is enough to kill any fighter
            target->SetDestroyed();
        }
        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.part_type_name,
                               power, 0.0f, 1.0f);
        attacker->SetLastTurnActiveInCombat(CurrentTurn());
    }

    void AttackPlanetShip(std::shared_ptr<Planet> attacker, const PartAttackInfo& weapon,
                          std::shared_ptr<Ship> target, CombatInfo& combat_info,
                          int bout, int round,
                          WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        if (!attacker || !target) return;

        float power = 0.0f;
        const Meter* attacker_damage = attacker->UniverseObject::GetMeter(METER_DEFENSE);
        if (attacker_damage)
            power = attacker_damage->Current();   // planet "Defense" meter is actually its attack power

        std::set<int>& damaged_object_ids = combat_info.damaged_object_ids;

        Meter* target_structure = target->UniverseObject::GetMeter(METER_STRUCTURE);
        if (!target_structure) {
            ErrorLogger() << "couldn't get target structure or shield meter";
            return;
        }

        Meter* target_shield = target->UniverseObject::GetMeter(METER_SHIELD);
        float shield = (target_shield ? target_shield->Current() : 0.0f);

        DebugLogger(combat) << "AttackPlanetShip: attacker: " << attacker->Name() << " power: " << power
                            << "  target: " << target->Name() << " shield: " << target_shield->Current()
                            << " structure: " << target_structure->Current();

        float damage = std::max(0.0f, power - shield);

        if (damage > 0.0f) {
            target_structure->AddToCurrent(-damage);
            damaged_object_ids.insert(target->ID());
            DebugLogger(combat) << "COMBAT: Planet " << attacker->Name() << " (" << attacker->ID()
                                << ") does " << damage << " damage to Ship " << target->Name() << " ("
                                << target->ID() << ")";
        }

        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.part_type_name,
                               power, shield, damage);

        target->SetLastTurnActiveInCombat(CurrentTurn());
    }

    void AttackPlanetFighter(std::shared_ptr<Planet> attacker, const PartAttackInfo& weapon,
                             std::shared_ptr<Fighter> target, CombatInfo& combat_info,
                             int bout, int round, AttacksEventPtr& attacks_event,
                             WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        if (!attacker || !target) return;

        float power = 0.0f;
        const Meter* attacker_damage = attacker->UniverseObject::GetMeter(METER_DEFENSE);
        if (attacker_damage)
            power = attacker_damage->Current();   // planet "Defense" meter is actually its attack power

        if (power > 0.0f) {
            // any damage is enough to destroy any fighter
            target->SetDestroyed();
        }

        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.part_type_name,
                               power, 0.0f, 1.0f);
    }

    void AttackFighterShip(std::shared_ptr<Fighter> attacker, const PartAttackInfo& weapon,
                           std::shared_ptr<Ship> target, CombatInfo& combat_info,
                           int bout, int round, AttacksEventPtr& attacks_event)
    {
        if (!attacker || !target) return;

        float power = attacker->Damage();

        std::set<int>& damaged_object_ids = combat_info.damaged_object_ids;

        Meter* target_structure = target->UniverseObject::GetMeter(METER_STRUCTURE);
        if (!target_structure) {
            ErrorLogger() << "couldn't get target structure or shield meter";
            return;
        }

        //Meter* target_shield = target->UniverseObject::GetMeter(METER_SHIELD);
        float shield = 0.0f; //(target_shield ? target_shield->Current() : 0.0f);

        DebugLogger() << "AttackFighterShip: Fighter of empire " << attacker->Owner() << " power: " << power
                      << "  target: " << target->Name() //<< " shield: " << target_shield->Current()
                      << " structure: " << target_structure->Current();

        float damage = std::max(0.0f, power - shield);

        if (damage > 0.0f) {
            target_structure->AddToCurrent(-damage);
            damaged_object_ids.insert(target->ID());
            DebugLogger(combat) << "COMBAT: Fighter of empire " << attacker->Owner() << " (" << attacker->ID()
                                << ") does " << damage << " damage to Ship " << target->Name() << " ("
                                << target->ID() << ")";
        }

        float pierced_shield_value(-1.0);
        CombatEventPtr attack_event = std::make_shared<WeaponFireEvent>(
            bout, round, attacker->ID(), target->ID(), weapon.part_type_name,
            std::tie(power, pierced_shield_value, damage),
            attacker->Owner(), target->Owner());
        attacks_event->AddEvent(attack_event);
        target->SetLastTurnActiveInCombat(CurrentTurn());
    }

    void AttackFighterFighter(std::shared_ptr<Fighter> attacker, const PartAttackInfo& weapon,
                              std::shared_ptr<Fighter> target, CombatInfo& combat_info,
                              int bout, int round,
                              std::shared_ptr<FightersAttackFightersEvent>& fighter_on_fighter_event)
    {
        if (!attacker || !target) return;

        float damage = attacker->Damage();

        if (damage > 0.0f) {
            // any damage is enough to destroy any fighter
            target->SetDestroyed();
        }

        fighter_on_fighter_event->AddEvent(attacker->Owner(), target->Owner());
    }

    void Attack(std::shared_ptr<UniverseObject>& attacker, const PartAttackInfo& weapon,
                std::shared_ptr<UniverseObject>& target, CombatInfo& combat_info,
                int bout, int round, AttacksEventPtr& attacks_event,
                WeaponsPlatformEvent::WeaponsPlatformEventPtr platform_event,
                std::shared_ptr<FightersAttackFightersEvent>& fighter_on_fighter_event)
    {
        auto attack_ship = std::dynamic_pointer_cast<Ship>(attacker);
        auto attack_planet = std::dynamic_pointer_cast<Planet>(attacker);
        auto attack_fighter = std::dynamic_pointer_cast<Fighter>(attacker);
        auto target_ship = std::dynamic_pointer_cast<Ship>(target);
        auto target_planet = std::dynamic_pointer_cast<Planet>(target);
        auto target_fighter = std::dynamic_pointer_cast<Fighter>(target);

        if (attack_ship && target_ship) {
            AttackShipShip(attack_ship, weapon, target_ship, combat_info, bout, round, platform_event);
        } else if (attack_ship && target_planet) {
            AttackShipPlanet(attack_ship, weapon, target_planet, combat_info, bout, round, platform_event);
        } else if (attack_ship && target_fighter) {
            AttackShipFighter(attack_ship, weapon, target_fighter, combat_info, bout, round, attacks_event, platform_event);
        } else if (attack_planet && target_ship) {
            AttackPlanetShip(attack_planet, weapon, target_ship, combat_info, bout, round, platform_event);
        } else if (attack_planet && target_planet) {
            // Planets don't attack each other, silly
        } else if (attack_planet && target_fighter) {
            AttackPlanetFighter(attack_planet, weapon, target_fighter, combat_info, bout, round, attacks_event, platform_event);
        } else if (attack_fighter && target_ship) {
            AttackFighterShip(attack_fighter, weapon, target_ship, combat_info, bout, round, attacks_event);
        } else if (attack_fighter && target_planet) {
            // Fighters can't attack planets
        } else if (attack_fighter && target_fighter) {
            AttackFighterFighter(attack_fighter, weapon, target_fighter, combat_info, bout, round, fighter_on_fighter_event);
        }
    }

    bool ObjectTypeCanBeAttacked(std::shared_ptr<const UniverseObject> obj) {
        if (!obj)
            return false;

        UniverseObjectType obj_type = obj->ObjectType();

        if (obj_type == OBJ_SHIP || obj_type == OBJ_FIGHTER)
            return true;

        if (obj_type == OBJ_PLANET) {
            if (!obj->Unowned() || obj->InitialMeterValue(METER_POPULATION) > 0.0f)
                return true;
            return false;
        }

        return false;
    }

    bool ObjectDiplomaticallyAttackableByEmpire(std::shared_ptr<const UniverseObject> obj,
                                                int empire_id)
    {
        if (!obj)
            return false;
        if (obj->OwnedBy(empire_id))
            return false;
        if (obj->Unowned() && empire_id == ALL_EMPIRES)
            return false;

        if (empire_id != ALL_EMPIRES && !obj->Unowned() &&
            Empires().GetDiplomaticStatus(empire_id, obj->Owner()) != DIPLO_WAR)
        { return false; }

        return true;
    }

    bool ObjectTargettableByEmpire(std::shared_ptr<const UniverseObject> obj, int empire_id) {
        if (!obj)
            return false;
        if (obj->ObjectType() != OBJ_FIGHTER &&
            GetUniverse().GetObjectVisibilityByEmpire(obj->ID(), empire_id) <= VIS_BASIC_VISIBILITY)
        {
            DebugLogger(combat) << obj->Name() << " not sufficiently visible to empire " << empire_id;
            return false;
        }

        return true;
    }

    bool ObjectAttackableByEmpire(std::shared_ptr<const UniverseObject> obj,
                                  int empire_id)
    {
        return ObjectTypeCanBeAttacked(obj)
            && ObjectDiplomaticallyAttackableByEmpire(obj, empire_id)
            && ObjectTargettableByEmpire(obj, empire_id);
    }

    bool ObjectUnTargettableByEmpire(std::shared_ptr<const UniverseObject> obj,
                                     int empire_id)
    {
        return ObjectTypeCanBeAttacked(obj)
            && ObjectDiplomaticallyAttackableByEmpire(obj, empire_id)
            && !ObjectTargettableByEmpire(obj, empire_id);
    }

    // monsters / natives can attack any planet, but can only attack
    // visible ships or ships that are in aggressive fleets
    bool ObjectDiplomaticallyAttackableByMonsters(std::shared_ptr<const UniverseObject> obj) {
        return (!obj->Unowned());
    }

    bool ObjectTargettableByMonsters(std::shared_ptr<const UniverseObject> obj,
                                     float monster_detection = 0.0f)
    {
        UniverseObjectType obj_type = obj->ObjectType();
        if (obj_type == OBJ_PLANET || obj_type == OBJ_FIGHTER) {
            return true;
        } else if (obj_type == OBJ_SHIP) {
            float stealth = obj->InitialMeterValue(METER_STEALTH);
            if (monster_detection >= stealth)
                return true;
        }
        //DebugLogger() << "... ... is NOT attackable by monsters";
        return false;
    }

    bool ObjectAttackableByMonsters(std::shared_ptr<const UniverseObject> obj,
                                    float monster_detection = 0.0f)
    {
        return ObjectTypeCanBeAttacked(obj)
            && ObjectDiplomaticallyAttackableByMonsters(obj)
            && ObjectTargettableByMonsters(obj, monster_detection);
    }

    bool ObjectUnTargettableByMonsters(std::shared_ptr<const UniverseObject> obj,
                                       float monster_detection = 0.0f)
    {
        return ObjectTypeCanBeAttacked(obj)
            && ObjectDiplomaticallyAttackableByMonsters(obj)
            && !ObjectTargettableByMonsters(obj, monster_detection);
    }

    bool ObjectCanAttack(std::shared_ptr<const UniverseObject> obj) {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(obj)) {
            return ship->IsArmed() || ship->HasFighters();
        } else if (auto planet = std::dynamic_pointer_cast<const Planet>(obj)) {
            return planet->InitialMeterValue(METER_DEFENSE) > 0.0f;
        } else if (auto fighter = std::dynamic_pointer_cast<const Fighter>(obj)) {
            return fighter->Damage() > 0.0f;
        } else {
            return false;
        }
    }

    std::vector<PartAttackInfo> ShipWeaponsStrengths(std::shared_ptr<const Ship> ship) {
        std::vector<PartAttackInfo> retval;
        if (!ship)
            return retval;
        const ShipDesign* design = GetShipDesign(ship->DesignID());
        if (!design)
            return retval;

        std::set<std::string> seen_hangar_part_types;
        int available_fighters = 0;
        float fighter_attack = 0.0f;
        std::map<std::string, int> part_fighter_launch_capacities;
        const ::Condition::ConditionBase* fighter_combat_targets = nullptr;

        // determine what ship does during combat, based on parts and their meters...
        for (const auto& part_name : design->Parts()) {
            const PartType* part = GetPartType(part_name);
            if (!part)
                continue;
            ShipPartClass part_class = part->Class();

            // direct weapon and fighter-related parts all handled differently...
            if (part_class == PC_DIRECT_WEAPON) {
                float part_attack = ship->CurrentPartMeterValue(METER_CAPACITY, part_name);
                int shots = static_cast<int>(ship->CurrentPartMeterValue(METER_SECONDARY_STAT, part_name)); // secondary stat is shots per attack)
                if (part_attack > 0.0f && shots > 0) {
                    // attack for each shot...
                    for (int shot_count = 0; shot_count < shots; ++shot_count)
                        retval.push_back(PartAttackInfo(part_class, part_name, part_attack, part->CombatTargets()));
                }

            } else if (part_class == PC_FIGHTER_HANGAR) {
                // hangar max-capacity-modification effects stack, so only add capacity for each hangar type once
                if (!seen_hangar_part_types.count(part_name)) {
                    available_fighters += ship->CurrentPartMeterValue(METER_CAPACITY, part_name);
                    seen_hangar_part_types.insert(part_name);
                    fighter_combat_targets = part->CombatTargets();

                    // should only be one type of fighter per ship as of this writing
                    fighter_attack = ship->CurrentPartMeterValue(METER_SECONDARY_STAT, part_name);  // secondary stat is fighter damage
                }

            } else if (part_class == PC_FIGHTER_BAY) {
                part_fighter_launch_capacities[part_name] += ship->CurrentPartMeterValue(METER_CAPACITY, part_name);
            }
        }

        if (available_fighters > 0 && !part_fighter_launch_capacities.empty()) {
            for (auto& launch : part_fighter_launch_capacities) {
                int to_launch = std::min(launch.second, available_fighters);

                DebugLogger() << "Ship " << ship->Name() << " launching " << to_launch << " fighters from bay part " << launch.first;

                if (to_launch <= 0)
                    continue;
                retval.push_back(PartAttackInfo(PC_FIGHTER_BAY, launch.first, to_launch,
                                                fighter_attack, fighter_combat_targets)); // attack may be 0; that's ok: decoys
                available_fighters -= to_launch;
                if (available_fighters <= 0)
                    break;
            }
        }

        return retval;
    }

    // Information about a single empire during combat
    struct EmpireCombatInfo {
        std::set<int> attacker_ids;
        std::set<int> target_ids;
        bool HasTargets() const { return !target_ids.empty(); }
        bool HasAttackers() const { return !attacker_ids.empty(); }

        bool HasUnlauchedArmedFighters(const CombatInfo& combat_info) const {
            // check each ship to see if it has any unlaunched armed fighters...
            for (int attacker_id : attacker_ids) {
                if (combat_info.destroyed_object_ids.count(attacker_id))
                    continue;   // destroyed objects can't launch fighters

                auto ship = combat_info.objects.Object<Ship>(attacker_id);
                if (!ship)
                    continue;   // non-ships can't launch fighters

                auto weapons = ShipWeaponsStrengths(ship);
                for (const PartAttackInfo& weapon : weapons) {
                    if (weapon.part_class == PC_FIGHTER_BAY &&
                        weapon.fighters_launched > 0 &&
                        weapon.fighter_damage > 0.0f)
                    { return true; }
                }
            }

            return false;
        }
    };

    // Calculate monster detection strength in system
    float GetMonsterDetection(const CombatInfo& combat_info) {
        float monster_detection = 0.0;
        for (auto it = combat_info.objects.const_begin(); it != combat_info.objects.const_end(); ++it) {
            auto obj = *it;
            if (obj->Unowned() && (obj->ObjectType() == OBJ_SHIP || obj->ObjectType() == OBJ_PLANET )){
                monster_detection = std::max(monster_detection, obj->InitialMeterValue(METER_DETECTION));
            }
        }
        return monster_detection;
    }

    /// A collection of information the autoresolution must keep around
    struct AutoresolveInfo {
        std::set<int>                   valid_target_object_ids;    // all objects that can be attacked
        std::set<int>                   valid_attacker_object_ids;  // all objects that can attack
        std::map<int, EmpireCombatInfo> empire_infos;               // empire specific information, indexed by empire id
        float                           monster_detection;          // monster (non-player combatants) detection strength
        CombatInfo&                     combat_info;                // a reference to the combat info
        int                             next_fighter_id;

        AutoresolveInfo(CombatInfo& combat_info_) :
            valid_target_object_ids(),
            valid_attacker_object_ids(),
            empire_infos(),
            monster_detection(0.0f),
            combat_info(combat_info_),
            next_fighter_id(-10001) // give fighters negative ids so as to avoid clashes with any positiv-id of persistent UniverseObjects
        {
            monster_detection = GetMonsterDetection(combat_info);
            PopulateAttackersAndTargets(combat_info);
            PopulateEmpireTargets(combat_info);
        }

        std::vector<int> AddFighters(int number, float damage, int owner_empire_id,
                                     int from_ship_id, const std::string& species, const ::Condition::ConditionBase* combat_targets)
        {
            std::vector<int> retval;

            for (int n = 0; n < number; ++n) {
                // create / insert fighter into combat objectmap
                auto fighter_ptr = std::make_shared<Fighter>(owner_empire_id, from_ship_id, species, damage, combat_targets);
                fighter_ptr->SetID(next_fighter_id--);
                fighter_ptr->Rename(UserString("OBJ_FIGHTER"));
                combat_info.objects.Insert(fighter_ptr);
                if (!fighter_ptr) {
                    ErrorLogger() << "AddFighters unable to create and insert new Fighter object...";
                    break;
                }

                retval.push_back(fighter_ptr->ID());

                // add fighter to attackers (if it can attack)
                if (damage > 0.0f) {
                    valid_attacker_object_ids.insert(fighter_ptr->ID());
                    empire_infos[fighter_ptr->Owner()].attacker_ids.insert(fighter_ptr->ID());
                    DebugLogger() << "Added fighter id: " << fighter_ptr->ID() << " to attackers sets";
                }

                // and to targets
                valid_target_object_ids.insert(fighter_ptr->ID());
                // add fighter to targets ids for any empire that can attack it
                for (const auto& entry : empire_infos) {
                    if (ObjectAttackableByEmpire(fighter_ptr, entry.first)) {
                        empire_infos[entry.first].target_ids.insert(fighter_ptr->ID());
                        DebugLogger() << "Added fighter " << fighter_ptr->ID()
                                      << " owned by empire " << fighter_ptr->Owner()
                                      << " to targets for empire " << entry.first;
                    }
                }
                DebugLogger() << "Added fighter id: " << fighter_ptr->ID() << " to valid targets set";
            }

            return retval;
        }

        // Return true if some empire that can attack has some targets that it can attack
        bool CanSomeoneAttackSomething() const {
            for (const auto& attacker : empire_infos) {
                if (attacker.second.HasTargets() && (
                        attacker.second.HasAttackers() ||
                        attacker.second.HasUnlauchedArmedFighters(combat_info)
                    ))
                { return true; }
            }
            return false;
        }

        /// Removes dead units from lists of attackers and defenders
        void CullTheDead(int bout,   BoutEvent::BoutEventPtr &bout_event) {
            auto fighters_destroyed_event = std::make_shared<FightersDestroyedEvent>(bout);
            bool at_least_one_fighter_destroyed = false;

            IncapacitationsEventPtr incaps_event =
                std::make_shared<IncapacitationsEvent>();

            for (auto it = combat_info.objects.const_begin();
                 it != combat_info.objects.const_end(); ++it)
            {
                auto obj = *it;

                // There may be objects, for example unowned planets, that were
                // not a part of the battle to start with. Ignore them
                if (!valid_attacker_object_ids.count(obj->ID()) &&
                    !valid_target_object_ids.count(obj->ID()))
                { continue; }

                // Check if the target was destroyed and update lists if yes
                if (!CheckDestruction(obj))
                    continue;

                if (obj->ObjectType() == OBJ_FIGHTER) {
                    fighters_destroyed_event->AddEvent(obj->Owner());
                    at_least_one_fighter_destroyed = true;
                } else {
                    CombatEventPtr incap_event = std::make_shared<IncapacitationEvent>(
                        bout, obj->ID(), obj->Owner());
                    incaps_event->AddEvent(incap_event);
                }
            }

            if (at_least_one_fighter_destroyed)
                bout_event->AddEvent(fighters_destroyed_event);

            if (!incaps_event->AreSubEventsEmpty(ALL_EMPIRES))
                bout_event->AddEvent(incaps_event);
        }

        /// Checks if target is destroyed and if it is, update lists of living objects.
        /// Return true if is incapacitated
        bool CheckDestruction(const std::shared_ptr<const UniverseObject>& target) {
            int target_id = target->ID();
            // check for destruction of target object

            if (target->ObjectType() == OBJ_FIGHTER) {
                auto fighter = std::dynamic_pointer_cast<const Fighter>(target);
                if (fighter && fighter->Destroyed()) {
                    // remove destroyed fighter's ID from lists of valid attackers and targets
                    valid_attacker_object_ids.erase(target_id);
                    valid_target_object_ids.erase(target_id);   // probably not necessary as this set isn't used in this loop
                    DebugLogger() << "Removed destroyed fighter id: " << fighter->ID() << " from attackers and targets sets";

                    for (auto& targeter : empire_infos)
                    { targeter.second.target_ids.erase(target_id); }

                    // Remove target from its empire's list of attackers
                    empire_infos[target->Owner()].attacker_ids.erase(target_id);
                    CleanEmpires();
                    return fighter->Destroyed();
                }

            } else if (target->ObjectType() == OBJ_SHIP) {
                if (target->CurrentMeterValue(METER_STRUCTURE) <= 0.0f) {
                    DebugLogger(combat) << "!! Target Ship " << target_id << " is destroyed!";
                    // object id destroyed
                    combat_info.destroyed_object_ids.insert(target_id);
                    // all empires in battle know object was destroyed
                    for (int empire_id : combat_info.empire_ids) {
                        if (empire_id != ALL_EMPIRES) {
                            DebugLogger(combat) << "Giving knowledge of destroyed object " << target_id
                                                << " to empire " << empire_id;
                            combat_info.destroyed_object_knowers[empire_id].insert(target_id);
                        }
                    }

                    // remove destroyed ship's ID from lists of valid attackers and targets
                    valid_attacker_object_ids.erase(target_id);
                    valid_target_object_ids.erase(target_id);   // probably not necessary as this set isn't used in this loop

                    for (auto& targeter : empire_infos)
                    { targeter.second.target_ids.erase(target_id); }

                    // Remove target from its empire's list of attackers
                    empire_infos[target->Owner()].attacker_ids.erase(target_id);
                    CleanEmpires();
                    return true;
                }

            } else if (target->ObjectType() == OBJ_PLANET) {
                if (!ObjectCanAttack(target) &&
                    valid_attacker_object_ids.count(target_id))
                {
                    DebugLogger(combat) << "!! Target Planet " << target_id << " knocked out, can no longer attack";
                    // remove disabled planet's ID from lists of valid attackers
                    valid_attacker_object_ids.erase(target_id);
                }
                if (target->CurrentMeterValue(METER_SHIELD) <= 0.0f &&
                    target->CurrentMeterValue(METER_DEFENSE) <= 0.0f &&
                    target->CurrentMeterValue(METER_CONSTRUCTION) <= 0.0f)
                {
                    // An outpost can enter combat in essentially an
                    // incapacitated state, but if it is removed from combat
                    // before it has been attacked then it can wrongly get regen
                    // on the next turn, so check that it has been attacked
                    // before excluding it from any remaining battle
                    if (!combat_info.damaged_object_ids.count(target_id)) {
                        DebugLogger(combat) << "!! Planet " << target_id << "has not yet been attacked, "
                                            << "so will not yet be removed from battle, despite being essentially incapacitated";
                        return false;
                    }
                    DebugLogger(combat) << "!! Target Planet " << target_id << " is entirely knocked out of battle";

                    // remove disabled planet's ID from lists of valid targets
                    valid_target_object_ids.erase(target_id);   // probably not necessary as this set isn't used in this loop

                    for (auto& targeter : empire_infos)
                    { targeter.second.target_ids.erase(target_id); }

                    // Remove target from its empire's list of attackers
                    empire_infos[target->Owner()].attacker_ids.erase(target_id);

                    CleanEmpires();
                    return true;
                }
            }

            return false;
        }

        /// check if any empire has no remaining target or attacker objects.
        /// If so, remove that empire's entry
        void CleanEmpires() {
            std::map<int, EmpireCombatInfo> temp = empire_infos;
            for (auto& empire : empire_infos) {
                if (!empire.second.HasTargets() &&
                    !empire.second.HasAttackers() &&
                    !empire.second.HasUnlauchedArmedFighters(combat_info))
                {
                    temp.erase(empire.first);
                    DebugLogger(combat) << "No valid attacking objects left for empire with id: " << empire.first;
                }
            }
            empire_infos = temp;
        }

        /// Returns the list of attacker ids in a random order
        void GetShuffledValidAttackerIDs(std::vector<int>& shuffled) {
            shuffled.clear();
            shuffled.insert(shuffled.begin(), valid_attacker_object_ids.begin(), valid_attacker_object_ids.end());

            const unsigned swaps = shuffled.size();
            for (unsigned i = 0; i < swaps; ++i){
                int pos2 = RandInt(i, swaps - 1);
                std::swap(shuffled[i], shuffled[pos2]);
            }
        }

        /**Report for each empire the stealth objects in the combat. */
        InitialStealthEvent::StealthInvisbleMap ReportInvisibleObjects(const CombatInfo& combat_info_) const {
            DebugLogger(combat) << "Reporting Invisible Objects";
            InitialStealthEvent::StealthInvisbleMap report;
            for (int object_id : valid_target_object_ids) {
                auto obj = combat_info_.objects.Object(object_id);

                // for all empires, can they attack this object?
                for (int attacking_empire_id : combat_info_.empire_ids) {
                    Visibility visibility = VIS_NO_VISIBILITY;
                    DebugLogger(combat) << "Target " << obj->Name() << " by empire = "<< attacking_empire_id;
                    auto target_visible_it = combat_info_.empire_object_visibility.find(obj->Owner());
                    if (target_visible_it != combat_info_.empire_object_visibility.end()) {
                        auto target_attacker_visibility_it = target_visible_it->second.find(obj->ID());
                        if (target_attacker_visibility_it != target_visible_it->second.end()) {
                            visibility = target_attacker_visibility_it->second;
                        }
                    }

                    if (attacking_empire_id == ALL_EMPIRES) {
                        if (ObjectUnTargettableByMonsters(obj, monster_detection)) {
                            // Note: This does put information about invisible and basic visible objects and
                            // trusts that the combat logger only informs player/ai of what they should know
                            DebugLogger(combat) << " Monster "  << obj->Name() << " " << visibility << " to empire " << attacking_empire_id;
                            report[attacking_empire_id][obj->Owner()]
                                .insert({obj->ID(), visibility});
                        }

                    } else {
                        if (ObjectUnTargettableByEmpire(obj, attacking_empire_id)) {
                            // Note: This does put information about invisible and basic visible objects and
                            // trusts that the combat logger only informs player/ai of what they should know
                            DebugLogger(combat) << " Ship " << obj->Name() << " " << visibility << " to empire " << attacking_empire_id;
                            report[attacking_empire_id][obj->Owner()]
                                .insert({obj->ID(), visibility});
                        }
                    }
                }
            }
            return report;
        }

    private:
        typedef std::set<int>::const_iterator const_id_iterator;

        // Populate lists of things that can attack and be attacked. List attackers also by empire.
        void PopulateAttackersAndTargets(const CombatInfo& combat_info_) {
            for (auto it = combat_info_.objects.const_begin();
                 it != combat_info_.objects.const_end(); ++it)
            {
                auto obj = *it;
                bool can_attack{ObjectCanAttack(obj)};
                if (can_attack) {
                    valid_attacker_object_ids.insert(it->ID());
                    empire_infos[obj->Owner()].attacker_ids.insert(it->ID());
                }

                bool can_be_attacked{ObjectTypeCanBeAttacked(obj)};
                if (can_be_attacked) {
                    valid_target_object_ids.insert(it->ID());
                }
                DebugLogger(combat) << "Considerting object " + obj->Name() + " owned by " + std::to_string(obj->Owner())
                                    << (can_attack ? "... can attack" : "")
                                    << (can_be_attacked ? "... can be attacked": "");
            }
        }

        // Get a map from empire to set of IDs of objects that empire's objects
        // could potentially target.
        void PopulateEmpireTargets(const CombatInfo& combat_info_) {
            for (int object_id : valid_target_object_ids) {
                auto obj = combat_info_.objects.Object(object_id);
                for (int attacking_empire_id : combat_info_.empire_ids) {
                    if (attacking_empire_id == ALL_EMPIRES) {
                        if (ObjectAttackableByMonsters(obj, monster_detection))
                            empire_infos[ALL_EMPIRES].target_ids.insert(object_id);
                    } else if (ObjectAttackableByEmpire(obj, attacking_empire_id)) {
                        empire_infos[attacking_empire_id].target_ids.insert(object_id);
                    }
                }

                DebugLogger(combat) << ReportAttackabilityOfTarget(combat_info_, obj);
            }
        }

        // Return a log report of attackability of \p obj
        std::string ReportAttackabilityOfTarget(const CombatInfo& combat_info_,
                                                const std::shared_ptr<const UniverseObject>& obj)
        {
            std::stringstream ss;
            ss << "Considering attackability of object " << obj->Name()
               << " owned by " << std::to_string(obj->Owner())
               << " attackable by ";

            bool no_one{true};
            for (int attacking_empire_id : combat_info_.empire_ids) {
                if (attacking_empire_id == ALL_EMPIRES) {
                    if (ObjectAttackableByMonsters(obj, monster_detection)) {
                        ss << "monsters and ";
                        no_one = false;
                    }
                } else if (ObjectAttackableByEmpire(obj, attacking_empire_id)) {
                    if (!no_one)
                        ss << ", ";
                    ss << std::to_string(attacking_empire_id);
                    no_one = false;
                }
            }
            if (no_one)
                ss << "none of the empires present";
            return ss.str();
        }
    };


    // Return a report of invalid targets
    std::string ReportInvalidTargets(const std::shared_ptr<UniverseObject>& attacker,
                                     const std::set<int>& potential_target_ids,
                                     const AutoresolveInfo& combat_state)
    {
        std::stringstream ss;
        ss << "Attacker " << attacker->ID() << " can't attack potential targets: ";

        for (int target_id : potential_target_ids) {
            auto target = combat_state.combat_info.objects.Object(target_id);
            if (!target)
                continue;

            // planets can only attack ships (not other planets or fighters)
            if (attacker->ObjectType() == OBJ_PLANET && target->ObjectType() != OBJ_SHIP)
                ss << std::to_string(target_id) << " ";

            // fighters can't attack planets
            else if (attacker->ObjectType() == OBJ_FIGHTER && target->ObjectType() == OBJ_PLANET)
                ss << std::to_string(target_id) << " ";
        }

        return ss.str();
    }

    const std::set<int> ValidTargetsForAttackerType(std::shared_ptr<UniverseObject>& attacker,
                                                    AutoresolveInfo& combat_state,
                                                    const std::set<int>& potential_target_ids)
    {
        if (attacker->ObjectType() == OBJ_SHIP)
            return potential_target_ids;    // ships can attack anything!

        std::set<int> valid_target_ids;
        bool any_invalid_targets{false};

        for (int target_id : potential_target_ids) {
            auto target = combat_state.combat_info.objects.Object(target_id);
            if (!target) {
                ErrorLogger() << "AutoResolveCombat couldn't get target object with id " << target_id;
                continue;
            }

            if (attacker->ObjectType() == OBJ_PLANET) {
                // planets can only attack ships (not other planets or fighters)
                if (target->ObjectType() == OBJ_SHIP) {
                    valid_target_ids.insert(target_id);
                } else {
                    any_invalid_targets = true;
                }
            } else if (attacker->ObjectType() == OBJ_FIGHTER) {
                // fighters can't attack planets
                if (target->ObjectType() != OBJ_PLANET) {
                    valid_target_ids.insert(target_id);
                } else {
                    any_invalid_targets = true;
                }
            }
        }

        if (any_invalid_targets)
            DebugLogger(combat) << ReportInvalidTargets(attacker, potential_target_ids, combat_state);

        return valid_target_ids;
    }

    void ShootAllWeapons(std::shared_ptr<UniverseObject>& attacker, const std::vector<PartAttackInfo>& weapons,
                         AutoresolveInfo& combat_state, int bout, int round,
                         AttacksEventPtr &attacks_event,
                         WeaponsPlatformEvent::WeaponsPlatformEventPtr &platform_event,
                         std::shared_ptr<FightersAttackFightersEvent>& fighter_on_fighter_event)
    {
        if (weapons.empty()) {
            DebugLogger(combat) << "no weapons' can't attack";
            return;   // no ability to attack!
        }

        for (const PartAttackInfo& weapon : weapons) {
            // skip non-direct-fire weapons (as only direct fire weapons can "shoot").
            // fighter launches handled separately
            if (weapon.part_class != PC_DIRECT_WEAPON)
                continue;

            // select object from valid targets for this object's owner   TODO: with this weapon...
            DebugLogger(combat) << "Attacking with weapon " << weapon.part_type_name << " with power " << weapon.part_attack;

            // get valid targets set for attacker owner.  need to do this for
            // each weapon that is attacking, as the previous shot might have
            // destroyed something
            int attacker_owner_id = attacker->Owner();

            auto target_vec_it = combat_state.empire_infos.find(attacker_owner_id);
            if (target_vec_it == combat_state.empire_infos.end() || !target_vec_it->second.HasTargets()) {
                DebugLogger(combat) << "No targets for empire: " << attacker_owner_id;
                break;
            }

            auto valid_target_ids = ValidTargetsForAttackerType(attacker, combat_state, target_vec_it->second.target_ids);
            if (valid_target_ids.empty()) {
                DebugLogger(combat) << "No valid targets for attacker " << attacker->ID();
                break;
            }

            DebugLogger(combat) << [&valid_target_ids, &attacker, &attacker_owner_id]() {
                std::stringstream ss;
                ss << "Valid targets for attacker with id: " << attacker->ID()
                   << " owned by empire: " << attacker_owner_id
                   << " :  ";
                for (int target_id : valid_target_ids)
                { ss << std::to_string(target_id) + " "; }
                return ss.str();
            }();


            // select target object
            int target_idx = RandInt(0, valid_target_ids.size() - 1);
            int target_id = *std::next(valid_target_ids.begin(), target_idx);

            auto target = combat_state.combat_info.objects.Object(target_id);
            if (!target) {
                ErrorLogger() << "AutoResolveCombat couldn't get target object with id " << target_id;
                continue;
            }
            DebugLogger(combat) << "Target: " << target->Name();

            // do actual attacks
            Attack(attacker, weapon, target, combat_state.combat_info, bout, round, attacks_event,
                   platform_event, fighter_on_fighter_event);

        } // end for over weapons
    }

    void ReduceStoredFighterCount(std::shared_ptr<Ship>& ship, float launched_fighters) {
        if (!ship || launched_fighters <= 0)
            return;

        // get how many fighters are initialy in each part type...
        // may be multiple hangar part types, each with different capacity (number of stored fighters)
        std::map<std::string, Meter*> part_type_fighter_hangar_capacities;

        const ShipDesign* design = ship->Design();
        if (!design) {
            ErrorLogger() << "ReduceStoredFighterCount couldn't get ship design with id " << ship->DesignID();;
            return;
        }

        // get hangar part meter values
        for (const std::string& part_name : design->Parts()) {
            const PartType* part = GetPartType(part_name);
            if (!part)
                continue;
            if (part->Class() != PC_FIGHTER_HANGAR)
                continue;
            part_type_fighter_hangar_capacities[part_name] = ship->GetPartMeter(METER_CAPACITY, part_name);
        }

        // reduce meters until requested fighter reduction is achived
        // doesn't matter which part's capacity meters are reduced, as all
        // fighters are the same on the ship
        for (auto& part : part_type_fighter_hangar_capacities) {
            if (!part.second)
                continue;
            float reduction = std::min(part.second->Current(), launched_fighters);
            launched_fighters -= reduction;
            part.second->AddToCurrent(-reduction);

            // stop if all fighters launched
            if (launched_fighters <= 0.0f)
                break;
        }
    }

    void LaunchFighters(std::shared_ptr<UniverseObject>& attacker,
                        const std::vector<PartAttackInfo>& weapons,
                        AutoresolveInfo& combat_state, int bout, int round,
                        FighterLaunchesEventPtr &launches_event)
    {
        if (weapons.empty()) {
            DebugLogger(combat) << "no weapons' can't launch figters!";
            return;   // no ability to attack!
        }

        static const std::string EMPTY_STRING;
        auto attacker_ship = std::dynamic_pointer_cast<Ship>(attacker);
        const auto& species_name = attacker_ship ? attacker_ship->SpeciesName() : EMPTY_STRING;

        for (const auto& weapon : weapons) {
            // skip non-fighter weapons
            // direct fire weapons handled separately
            if (weapon.part_class != PC_FIGHTER_BAY)    // passed in weapons container should have already checked for adequate supply of fighters to launch and the available bays, and condensed into a single entry...
                continue;

            int attacker_owner_id = attacker->Owner();

            DebugLogger(combat) << "Launching " << weapon.fighters_launched
                                << " with damage " << weapon.fighter_damage
                                << " for empire id: " << attacker_owner_id
                                << " from ship id: " << attacker->ID();

            std::vector<int> new_fighter_ids =
                combat_state.AddFighters(weapon.fighters_launched, weapon.fighter_damage,
                                         attacker_owner_id, attacker->ID(), species_name, weapon.combat_targets);

            // combat event
            CombatEventPtr launch_event =
                std::make_shared<FighterLaunchEvent>(bout, attacker->ID(), attacker_owner_id, new_fighter_ids.size());
            launches_event->AddEvent(launch_event);


            // reduce hangar capacity (contents) corresponding to launched fighters
            int num_launched = new_fighter_ids.size();
            ReduceStoredFighterCount(attacker_ship, num_launched);

            // launching fighters counts as a ship being active in combat
            if (!new_fighter_ids.empty())
                attacker_ship->SetLastTurnActiveInCombat(CurrentTurn());

            break;  // don't need to check any more weapons, as all fighters launched should have been contained in the single PC_FIGHTER_BAY entry
        } // end for over weapons
    }

    void IncreaseStoredFighterCount(std::shared_ptr<Ship>& ship, float recovered_fighters) {
        if (!ship || recovered_fighters <= 0)
            return;

        // get how many fighters are initialy in each part type...
        // may be multiple hangar part types, each with different capacity (number of stored fighters)
        std::map<std::string, std::pair<Meter*, Meter*>> part_type_fighter_hangar_capacities;

        const ShipDesign* design = ship->Design();
        if (!design) {
            ErrorLogger() << "ReduceStoredFighterCount couldn't get ship design with id " << ship->DesignID();;
            return;
        }

        // get hangar part meter values
        for (const std::string& part_name : design->Parts()) {
            const PartType* part = GetPartType(part_name);
            if (!part)
                continue;
            if (part->Class() != PC_FIGHTER_HANGAR)
                continue;
            part_type_fighter_hangar_capacities[part_name].first = ship->GetPartMeter(METER_CAPACITY, part_name);
            part_type_fighter_hangar_capacities[part_name].second = ship->GetPartMeter(METER_MAX_CAPACITY, part_name);
        }

        // increase capacity meters until requested fighter allocation is
        // achived doesn't matter which part's capacity meters are increased,
        // as allfighters are the same on the ship
        for (auto& part : part_type_fighter_hangar_capacities) {
            if (!part.second.first || !part.second.second)
                continue;
            float space = part.second.second->Current() - part.second.first->Current();
            float increase = std::min(space, recovered_fighters);
            recovered_fighters -= increase;
            part.second.first->AddToCurrent(increase);

            // stop if all fighters launched
            if (recovered_fighters <= 0.0f)
                break;
        }
    }

    void RecoverFighters(CombatInfo& combat_info, int bout,
                         FighterLaunchesEventPtr &launches_event) {
        std::map<int, float> ships_fighters_to_add_back;

        for (auto obj_it = combat_info.objects.begin();
             obj_it != combat_info.objects.end() && obj_it->ID() < 0; ++obj_it)
        {
            auto fighter = std::dynamic_pointer_cast<Fighter>(*obj_it);
            if (!fighter || fighter->Destroyed())
                continue;   // destroyed fighters can't return
            if (combat_info.destroyed_object_ids.count(fighter->LaunchedFrom()))
                continue;   // can't return to a destroyed ship
            ships_fighters_to_add_back[fighter->LaunchedFrom()]++;
        }

        for (auto& entry : ships_fighters_to_add_back) {
            auto ship = combat_info.objects.Object<Ship>(entry.first);
            if (!ship) {
                ErrorLogger() << "Couldn't get ship with id " << entry.first << " for fighter to return to...";
                continue;
            }
            IncreaseStoredFighterCount(ship, entry.second);
            // launching negative ships indicates recovery of them
            CombatEventPtr launch_event = std::make_shared<FighterLaunchEvent>(bout, entry.first, ship->Owner(), -entry.second);
            launches_event->AddEvent(launch_event);
        }
    }

    std::vector<PartAttackInfo> GetWeapons(std::shared_ptr<UniverseObject>& attacker) {
        // loop over weapons of attacking object.  each gets a shot at a
        // randomly selected target object
        std::vector<PartAttackInfo> weapons;

        auto attack_ship = std::dynamic_pointer_cast<Ship>(attacker);
        auto attack_planet = std::dynamic_pointer_cast<Planet>(attacker);
        auto attack_fighter = std::dynamic_pointer_cast<Fighter>(attacker);

        if (attack_ship) {
            weapons = ShipWeaponsStrengths(attack_ship);    // includes info about fighter launches with PC_FIGHTER_BAY part class, and direct fire weapons with PC_DIRECT_WEAPON part class
            for (PartAttackInfo& part : weapons) {
                if (part.part_class == PC_DIRECT_WEAPON) {
                    DebugLogger(combat) << "Attacker Ship direct weapon: " << part.part_type_name
                                        << " attack: " << part.part_attack;
                } else if (part.part_class == PC_FIGHTER_BAY) {
                    DebugLogger(combat) << "Attacker Ship fighter launch: " << part.fighters_launched
                                        << " damage: " << part.fighter_damage;
                }
            }

        } else if (attack_planet) {     // treat planet defenses as direct fire weapon
            weapons.push_back(PartAttackInfo(PC_DIRECT_WEAPON, UserStringNop("DEF_DEFENSE"),
                                             attack_planet->CurrentMeterValue(METER_DEFENSE), Targetting::PreyAsTriggerCondition(Targetting::ShipPrey)));

        } else if (attack_fighter) {    // treat fighter damage as direct fire weapon
            weapons.push_back(PartAttackInfo(PC_DIRECT_WEAPON, UserStringNop("FT_WEAPON_1"),
                                             attack_fighter->Damage(), attack_fighter->CombatTargets()));
        }
        return weapons;
    }

    void CombatRound(int bout, CombatInfo& combat_info, AutoresolveInfo& combat_state) {
        auto bout_event = std::make_shared<BoutEvent>(bout);
        combat_info.combat_events.push_back(bout_event);
        if (combat_state.valid_attacker_object_ids.empty()) {
            DebugLogger(combat) << "Combat bout " << bout << " aborted due to no remaining attackers.";
            return;
        }

        std::vector<int> shuffled_attackers;
        combat_state.GetShuffledValidAttackerIDs(shuffled_attackers);

        DebugLogger() << [&shuffled_attackers](){
            std::stringstream ss;
            ss << "Attacker IDs: [";
            for (int attacker : shuffled_attackers)
            { ss << attacker << " "; }
            ss << "]";
            return ss.str();
        }();

        auto attacks_event = std::make_shared<AttacksEvent>();
        bout_event->AddEvent(attacks_event);

        auto fighter_on_fighter_event = std::make_shared<FightersAttackFightersEvent>(bout);
        bout_event->AddEvent(fighter_on_fighter_event);

        int round = 1;  // counter of events during the current combat bout

        // Planets are processed first so that they still have full power as intended,
        // despite their attack power depending on something (their defence meter)
        // that processing shots at them may reduce.
        for (int attacker_id : shuffled_attackers) {
            auto attacker = combat_info.objects.Object(attacker_id);

            if (!attacker) {
                ErrorLogger() << "CombatRound couldn't get object with id " << attacker_id;
                continue;
            }
            if (attacker->ObjectType() != OBJ_PLANET) {
                continue;   // fighter and ship attacks processed below
            }
            if (!ObjectCanAttack(attacker)) {
                DebugLogger() << "Planet " << attacker->Name() << " could not attack.";
                continue;
            }
            DebugLogger(combat) << "Planet: " << attacker->Name();

            auto weapons = GetWeapons(attacker); // includes info about fighter launches with PC_FIGHTER_BAY part class, and direct fire weapons (ships, planets, or fighters) with PC_DIRECT_WEAPON part class
            auto platform_event = std::make_shared<WeaponsPlatformEvent>(
                bout, attacker_id, attacker->Owner());
            attacks_event->AddEvent(platform_event);
            ShootAllWeapons(attacker, weapons, combat_state, bout, round++,
                            attacks_event, platform_event, fighter_on_fighter_event);
        }

        // now process ship and fighter attacks
        for (int attacker_id : shuffled_attackers) {
            auto attacker = combat_info.objects.Object(attacker_id);

            if (!attacker) {
                ErrorLogger() << "CombatRound couldn't get object with id " << attacker_id;
                continue;
            }
            if (attacker->ObjectType() == OBJ_PLANET) {
                continue;   // planet attacks processed above
            }
            if (!ObjectCanAttack(attacker)) {
                DebugLogger() << "Attacker " << attacker->Name() << " could not attack.";
                continue;
            }
            DebugLogger(combat) << "Attacker: " << attacker->Name();

            // loop over weapons of the attacking object.  each gets a shot at a
            // randomly selected target object
            auto weapons = GetWeapons(attacker);  // includes info about fighter launches with PC_FIGHTER_BAY part class, and direct fire weapons (ships, planets, or fighters) with PC_DIRECT_WEAPON part class
            auto platform_event = std::make_shared<WeaponsPlatformEvent>(
                bout, attacker_id, attacker->Owner());
            ShootAllWeapons(attacker, weapons, combat_state, bout, round++,
                            attacks_event, platform_event, fighter_on_fighter_event);
            if (!platform_event->AreSubEventsEmpty(attacker->Owner()))
                attacks_event->AddEvent(platform_event);
        }

        // now launch fighters (which can attack in any subsequent combat bouts)
        // no point is launching fighters during the last bout, as they will not
        // get any chance to attack during this combat
        if (bout <= GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS") - 1) {
            auto launches_event = std::make_shared<FighterLaunchesEvent>();
            for (int attacker_id : shuffled_attackers) {
                auto attacker = combat_info.objects.Object(attacker_id);

                if (!attacker) {
                    ErrorLogger() << "CombatRound couldn't get object with id " << attacker_id;
                    continue;
                }
                if (attacker->ObjectType() != OBJ_SHIP) {
                    continue;   // currently only ships can launch fighters
                }
                if (!ObjectCanAttack(attacker)) {
                    DebugLogger() << "Attacker " << attacker->Name() << " could not attack.";
                    continue;
                }
                auto weapons = GetWeapons(attacker);  // includes info about fighter launches with PC_FIGHTER_BAY part class, and direct fire weapons (ships, planets, or fighters) with PC_DIRECT_WEAPON part class

                LaunchFighters(attacker, weapons, combat_state, bout, round++,
                               launches_event);

                DebugLogger(combat) << "Attacker: " << attacker->Name();
            }

            if (!launches_event->AreSubEventsEmpty(ALL_EMPIRES))
                bout_event->AddEvent(launches_event);
        }

        // Stealthed attackers have now revealed themselves to their targets.
        // Process this for each new combat event.
        auto stealth_change_event = std::make_shared<StealthChangeEvent>(bout);
        auto attacks_this_bout = attacks_event->SubEvents(ALL_EMPIRES);
        for (auto this_event : attacks_this_bout) {
            // mark attacker as valid target for attacked object's owner, so that regardless
            // of visibility the attacker can be counter-attacked in subsequent rounds if it
            // was not already attackable
            std::vector<std::shared_ptr<const WeaponFireEvent>> weapon_fire_events;
            if (auto naked_fire_event =
                std::dynamic_pointer_cast<const WeaponFireEvent>(this_event))
            {
                weapon_fire_events.push_back(naked_fire_event);

            } else if (auto weapons_platform =
                       std::dynamic_pointer_cast<const WeaponsPlatformEvent>(this_event))
            {
                for (auto more_event : weapons_platform->SubEvents(ALL_EMPIRES)) {
                    if (auto this_attack =
                        std::dynamic_pointer_cast<const WeaponFireEvent>(more_event))
                    {
                        weapon_fire_events.push_back(this_attack);
                    }
                }
            }

            for (auto this_attack : weapon_fire_events) {
                combat_info.ForceAtLeastBasicVisibility(this_attack->attacker_id, this_attack->target_id);
                int target_empire = combat_info.objects.Object(this_attack->target_id)->Owner();
                auto attacker_targettable_it =
                    combat_state.empire_infos[target_empire].target_ids.find(this_attack->attacker_id);
                if (attacker_targettable_it == combat_state.empire_infos[target_empire].target_ids.end())
                {
                    combat_state.empire_infos[target_empire].target_ids.insert(
                        attacker_targettable_it, this_attack->attacker_id);
                    int attacker_empire = combat_info.objects.Object(this_attack->attacker_id)->Owner();
                    stealth_change_event->AddEvent(this_attack->attacker_id, this_attack->target_id,
                                                   attacker_empire, target_empire, VIS_BASIC_VISIBILITY);
                }
            }
        }
        if (!stealth_change_event->AreSubEventsEmpty(ALL_EMPIRES))
            combat_info.combat_events.push_back(stealth_change_event);

        /// Remove all who died in the bout
        combat_state.CullTheDead(bout, bout_event);
    }
}

void AutoResolveCombat(CombatInfo& combat_info) {
    if (combat_info.objects.Empty())
        return;

    auto system = combat_info.objects.Object<System>(combat_info.system_id);
    if (!system)
        ErrorLogger() << "AutoResolveCombat couldn't get system with id " << combat_info.system_id;
    else
        DebugLogger() << "AutoResolveCombat at " << system->Name();

    DebugLogger(combat) << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%";
    DebugLogger(combat) << "AutoResolveCombat objects before resolution: " << combat_info.objects.Dump();

    // reasonably unpredictable but reproducible random seeding
    int base_seed = 0;
    if (GetGameRules().Get<bool>("RULE_RESEED_PRNG_SERVER")) {
        //static boost::hash<std::string> cs_string_hash;
        // todo: salt further with galaxy setup seed
        base_seed = combat_info.objects.begin()->ID() + CurrentTurn();
    }

    // compile list of valid objects to attack or be attacked in this combat
    AutoresolveInfo combat_state(combat_info);

    combat_info.combat_events.push_back(
        std::make_shared<InitialStealthEvent>(
            combat_state.ReportInvisibleObjects(combat_info)));

    // run multiple combat "bouts" during which each combat object can take
    // action(s) such as shooting at target(s) or launching fighters
    int last_bout = 1;
    for (int bout = 1; bout <= GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS"); ++bout) {
        if (GetGameRules().Get<bool>("RULE_RESEED_PRNG_SERVER"))
            Seed(base_seed + bout);    // ensure each combat bout produces different results

        // empires may have valid targets, but nothing to attack with.  If all
        // empires have no attackers or no valid targers, combat is over
        if (!combat_state.CanSomeoneAttackSomething()) {
            DebugLogger(combat) << "No empire has valid targets and something to attack with; combat over.";
            break;
        }

        DebugLogger(combat) << "Combat at " << system->Name() << " (" << combat_info.system_id << ") Bout " << bout;

        CombatRound(bout, combat_info, combat_state);
        last_bout = bout;
    } // end for over combat arounds

    auto launches_event = std::make_shared<FighterLaunchesEvent>();
    combat_info.combat_events.push_back(launches_event);

    RecoverFighters(combat_info, last_bout, launches_event);

    // ensure every participant knows what happened.
    // TODO: assemble list of objects to copy for each empire.  this should
    //       include objects the empire already knows about with standard
    //       visibility system, and also any objects the empire knows are
    //       destroyed during this combat...
    for (auto& entry : combat_info.empire_known_objects)
    { entry.second.Copy(combat_info.objects); }

    DebugLogger(combat) << "AutoResolveCombat objects after resolution: " << combat_info.objects.Dump();

    DebugLogger(combat) << "combat event log start:";
    for (auto event : combat_info.combat_events)
        DebugLogger(combat) << event->DebugString();
    DebugLogger(combat) << "combat event log end:";
}
