#include "CombatSystem.h"

#include "../universe/Universe.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Predicates.h"

#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../Empire/Empire.h"

#include "../util/Random.h"
#include "../util/AppInterface.h"

#include "../server/ServerApp.h"
#include "../network/Message.h"

////////////////////////////////////////////////
// CombatInfo
////////////////////////////////////////////////
CombatInfo::CombatInfo() :
    system_id(INVALID_OBJECT_ID)
{}

CombatInfo::CombatInfo(int system_id_) :
    system_id(system_id_)
{
    const Universe& universe = GetUniverse();
    const ObjectMap& universe_objects = universe.Objects();

    const System* system = universe_objects.Object<System>(system_id);
    if (!system) {
        Logger().errorStream() << "CombatInfo constructed with invalid system id: " << system_id;
        return;
    }

    // add copy of system to full / complete objects in combat
    System* copy_system = system->Clone();
    objects.Insert(system_id, copy_system);


    // find ships and their owners in system
    std::vector<int> ship_ids = system->FindObjectIDs<Ship>();
    for (std::vector<int>::const_iterator it = ship_ids.begin(); it != ship_ids.end(); ++it) {
        int ship_id = *it;
        const Ship* ship = GetShip(ship_id);
        if (!ship) {
            Logger().errorStream() << "CombatInfo::CombatInfo couldn't get ship with id " << ship_id << " in system " << system->Name() << " (" << system_id << ")";
            continue;
        }

        // add owner to empires that have assets in this battle
        empire_ids.insert(ship->Owner());

        // add copy of ship to full / complete copy of objects in system
        Ship* copy = ship->Clone();
        objects.Insert(ship_id, copy);
    }

    // find planets and their owners in system
    std::vector<int> planet_ids = system->FindObjectIDs<Planet>();
    for (std::vector<int>::const_iterator it = planet_ids.begin(); it != planet_ids.end(); ++it) {
        int planet_id = *it;
        const Planet* planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "CombatInfo::CombatInfo couldn't get planet with id " << planet_id << " in system " << system->Name() << " (" << system_id << ")";
            continue;
        }

        // if planet is populated, add owner to empires that have assets in this battle
        if (planet->CurrentMeterValue(METER_POPULATION) > 0.0)
            empire_ids.insert(planet->Owner());

        // add copy of ship to full / complete copy of objects in system
        Planet* copy = planet->Clone();
        objects.Insert(planet_id, copy);
    }

    // TODO: should buildings be considered separately?

    // now that all participants in the battle have been found, loop through
    // objects again to assemble each participant empire's latest
    // known information about all objects in this battle

    // system
    for (std::set<int>::const_iterator empire_it = empire_ids.begin(); empire_it != empire_ids.end(); ++empire_it) {
        int empire_id = *empire_it;
        if (empire_id == ALL_EMPIRES)
            continue;
        System* visibility_limited_copy = system->Clone(empire_id);
        empire_known_objects[empire_id].Insert(system_id, visibility_limited_copy);
    }
    // ships
    for (std::vector<int>::const_iterator it = ship_ids.begin(); it != ship_ids.end(); ++it) {
        int ship_id = *it;
        const Ship* ship = GetShip(ship_id);
        if (!ship) {
            Logger().errorStream() << "CombatInfo::CombatInfo couldn't get ship with id " << ship_id << " in system " << system->Name() << " (" << system_id << ")";
            continue;
        }

        for (std::set<int>::const_iterator empire_it = empire_ids.begin(); empire_it != empire_ids.end(); ++empire_it) {
            int empire_id = *empire_it;
            if (empire_id == ALL_EMPIRES)
                continue;
            if (universe.GetObjectVisibilityByEmpire(ship_id, empire_id) >= VIS_BASIC_VISIBILITY) {
                Ship* visibility_limited_copy = ship->Clone(empire_id);
                empire_known_objects[empire_id].Insert(ship_id, visibility_limited_copy);
            }
        }
    }
    // planets
    for (std::vector<int>::const_iterator it = planet_ids.begin(); it != planet_ids.end(); ++it) {
        int planet_id = *it;
        const Planet* planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "CombatInfo::CombatInfo couldn't get planet with id " << planet_id << " in system " << system->Name() << " (" << system_id << ")";
            continue;
        }

        for (std::set<int>::const_iterator empire_it = empire_ids.begin(); empire_it != empire_ids.end(); ++empire_it) {
            int empire_id = *empire_it;
            if (empire_id == ALL_EMPIRES)
                continue;
            if (universe.GetObjectVisibilityByEmpire(planet_id, empire_id) >= VIS_BASIC_VISIBILITY) {
                Planet* visibility_limited_copy = planet->Clone(empire_id);
                empire_known_objects[empire_id].Insert(planet_id, visibility_limited_copy);
            }
        }
    }

    // after battle is simulated, any changes to latest known or actual objects
    // will be copied back to the main Universe's ObjectMap and the Universe's
    // empire latest known objects ObjectMap
}

void CombatInfo::Clear() {
    system_id = INVALID_OBJECT_ID;
    empire_ids.clear();
    objects.Clear();
    for (std::map<int, ObjectMap>::iterator it = empire_known_objects.begin(); it != empire_known_objects.end(); ++it)
        it->second.Clear();
    destroyed_object_ids.clear();
    destroyed_object_knowers.clear();
}

const System* CombatInfo::GetSystem() const
{ return this->objects.Object<System>(this->system_id); }

System* CombatInfo::GetSystem()
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

void CombatInfo::GetEmpireKnownObjectsToSerialize(std::map<int, ObjectMap>& filtered_empire_known_objects, int encoding_empire) const {
    if (&filtered_empire_known_objects == &this->empire_known_objects)
        return;

    for (std::map<int, ObjectMap>::iterator it = filtered_empire_known_objects.begin(); it != filtered_empire_known_objects.end(); ++it)
        it->second.Clear();
    filtered_empire_known_objects.clear();

    if (encoding_empire == ALL_EMPIRES) {
        filtered_empire_known_objects = this->empire_known_objects;
        return;
    }

    // include only latest known objects for the encoding empire
    std::map<int, ObjectMap>::const_iterator it = this->empire_known_objects.find(encoding_empire);
    if (it != this->empire_known_objects.end()) {
        const ObjectMap& map = it->second;
        filtered_empire_known_objects[encoding_empire].Copy(map, ALL_EMPIRES);
    }
}

void CombatInfo::GetDestroyedObjectsToSerialize(std::set<int>& filtered_destroyed_objects, int encoding_empire) const {
    if (encoding_empire == ALL_EMPIRES) {
        filtered_destroyed_objects = this->destroyed_object_ids;
        return;
    }
    // TODO: decide if some filtering is needed for destroyed objects... it may not be.
    filtered_destroyed_objects = this->destroyed_object_ids;
}

void CombatInfo::GetDestroyedObjectKnowersToSerialize(std::map<int, std::set<int> >&
    filtered_destroyed_object_knowers, int encoding_empire) const
{
    if (encoding_empire == ALL_EMPIRES) {
        filtered_destroyed_object_knowers = this->destroyed_object_knowers;
        return;
    }
    // TODO: decide if some filtering is needed for which empires know about which
    // other empires know which objects have been destroyed during the battle.
    filtered_destroyed_object_knowers = this->destroyed_object_knowers;
}

////////////////////////////////////////////////
// AutoResolveCombat
////////////////////////////////////////////////
namespace {
    void AttackShipShip(Ship* attacker, Ship* target) {
        if (!attacker || ! target) return;

        const ShipDesign* attacker_design = attacker->Design();
        if (!attacker_design)
            return;
        double damage = attacker_design->Attack();
        if (damage <= 0.0)
            return;

        Meter* target_shield = target->UniverseObject::GetMeter(METER_SHIELD);
        Meter* target_structure = target->UniverseObject::GetMeter(METER_STRUCTURE);
        if (!target_shield || ! target_structure) {
            Logger().errorStream() << "couldn't get target structure or shield meter";
            return;
        }

        Logger().debugStream() << "AttackShipShip: attacker: " << attacker->Name() << " damage: " << damage
                               << "\ntarget: " << target->Name() << " shield: " << target_shield->Current()
                                                                 << " structure: " << target_structure->Current();

        // damage shields, limited by shield current value and damage amount.
        // remaining damage, if any, above shield current value goes to structure
        double shield_damage = std::min(target_shield->Current(), damage);
        double structure_damage = 0.0;
        if (shield_damage >= target_shield->Current())
            structure_damage = std::min(target_structure->Current(), damage - shield_damage);

        if (shield_damage > 0) {
            target_shield->AddToCurrent(-shield_damage);
            Logger().debugStream() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << shield_damage << " shield damage to Ship " << target->Name() << " (" << target->ID() << ")";
        }
        if (structure_damage > 0) {
            target_structure->AddToCurrent(-structure_damage);
            Logger().debugStream() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << structure_damage << " structure damage to Ship " << target->Name() << " (" << target->ID() << ")";
        }

        attacker->SetLastTurnActiveInCombat(CurrentTurn());
        target->SetLastTurnActiveInCombat(CurrentTurn());
    }

    void AttackShipPlanet(Ship* attacker, Planet* target) {
        if (!attacker || ! target) return;

        const ShipDesign* attacker_design = attacker->Design();
        if (!attacker_design)
            return;
        double damage = attacker_design->Attack();
        if (damage <= 0.0)
            return;

        Meter* target_shield = target->GetMeter(METER_SHIELD);
        Meter* target_defense = target->UniverseObject::GetMeter(METER_DEFENSE);
        Meter* target_construction = target->UniverseObject::GetMeter(METER_CONSTRUCTION);
        if (!target_shield) {
            Logger().errorStream() << "couldn't get target shield meter";
            return;
        }
        if (!target_defense) {
            Logger().errorStream() << "couldn't get target defense meter";
            return;
        }
        if (!target_construction) {
            Logger().errorStream() << "couldn't get target construction meter";
            return;
        }

        Logger().debugStream() << "AttackShipPlanet: attacker: " << attacker->Name() << " damage: " << damage
                               << "\ntarget: " << target->Name() << " shield: " << target_shield->Current()
                                                                 << " defense: " << target_defense->Current()
                                                                 << " infra: " << target_construction->Current();

        // damage shields, limited by shield current value and damage amount.
        // remaining damage, if any, above shield current value goes to defense.
        // remaining damage, if any, above defense current value goes to construction
        double shield_damage = std::min(target_shield->Current(), damage);
        double defense_damage = 0.0;
        double construction_damage = 0.0;
        if (shield_damage >= target_shield->Current())
            defense_damage = std::min(target_defense->Current(), damage - shield_damage);

        if (defense_damage >= target_defense->Current())
            construction_damage = std::min(target_construction->Current(), damage - shield_damage - defense_damage);

        if (shield_damage >= 0) {
            target_shield->AddToCurrent(-shield_damage);
            Logger().debugStream() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << shield_damage << " shield damage to Planet " << target->Name() << " (" << target->ID() << ")";
        }
        if (defense_damage >= 0) {
            target_defense->AddToCurrent(-defense_damage);
            Logger().debugStream() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << defense_damage << " defense damage to Planet " << target->Name() << " (" << target->ID() << ")";
        }

        if (construction_damage >= 0) {
            target_construction->AddToCurrent(-construction_damage);
            Logger().debugStream() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << construction_damage << " instrastructure damage to Planet " << target->Name() << " (" << target->ID() << ")";
        }

        attacker->SetLastTurnActiveInCombat(CurrentTurn());
        target->SetLastTurnAttackedByShip(CurrentTurn());
    }

    void AttackPlanetShip(Planet* attacker, Ship* target) {
        if (!attacker || ! target) return;

        double damage = attacker->UniverseObject::GetMeter(METER_DEFENSE)->Current();   // planet "Defense" meter is actually its attack power
        if (damage <= 0.0)
            return;

        Meter* target_shield = target->UniverseObject::GetMeter(METER_SHIELD);
        Meter* target_structure = target->UniverseObject::GetMeter(METER_STRUCTURE);
        if (!target_shield || ! target_structure) {
            Logger().errorStream() << "couldn't get target structure or shield meter";
            return;
        }

        Logger().debugStream() << "AttackPlanetShip: attacker: " << attacker->Name() << " damage: " << damage
                               << "\ntarget: " << target->Name() << " shield: " << target_shield->Current()
                                                                 << " structure: " << target_structure->Current();

        // damage shields, limited by shield current value and damage amount.
        // remaining damage, if any, above shield current value goes to structure
        double shield_damage = std::min(target_shield->Current(), damage);
        double structure_damage = 0.0;
        if (shield_damage >= target_shield->Current())
            structure_damage = std::min(target_structure->Current(), damage - shield_damage);

        if (shield_damage >= 0) {
            target_shield->AddToCurrent(-shield_damage);
            Logger().debugStream() << "COMBAT: Planet " << attacker->Name() << " (" << attacker->ID() << ") does " << shield_damage << " shield damage to Ship " << target->Name() << " (" << target->ID() << ")";
        }
        if (structure_damage >= 0) {
            target_structure->AddToCurrent(-structure_damage);
            Logger().debugStream() << "COMBAT: Planet " << attacker->Name() << " (" << attacker->ID() << ") does " << structure_damage << " structure damage to Ship " << target->Name() << " (" << target->ID() << ")";
        }

        target->SetLastTurnActiveInCombat(CurrentTurn());
    }

    void AttackPlanetPlanet(Planet* attacker, Planet* target) {
        Logger().debugStream() << "AttackPlanetPlanet does nothing!";
        // intentionally left empty
    }

    bool ObjectCanBeAttacked(const UniverseObject* obj) {
        if (const Ship* ship = universe_object_cast<const Ship*>(obj)) {
            return true;
        } else if (const Planet* planet = universe_object_cast<const Planet*>(obj)) {
            if (planet->CurrentMeterValue(METER_POPULATION) > 0.0)
                return true;
            else
                return false;
        } else {
            return false;
        }
    }

    bool ObjectAttackableByEmpire(const UniverseObject* obj, int empire_id) {
        if (obj->OwnedBy(empire_id))
            return false;
        if (obj->Unowned() && empire_id == ALL_EMPIRES)
            return false;
        return ObjectCanBeAttacked(obj);
    }

    bool ObjectCanAttack(const UniverseObject* obj) {
        if (const Ship* ship = universe_object_cast<const Ship*>(obj)) {
            return ship->IsArmed();
        } else if (const Planet* planet = universe_object_cast<const Planet*>(obj)) {
            return planet->CurrentMeterValue(METER_POPULATION) > 0.0 &&
                   planet->CurrentMeterValue(METER_DEFENSE) > 0.0;
        } else {
            return false;
        }
    }
}

void AutoResolveCombat(CombatInfo& combat_info) {
    if (combat_info.objects.Empty())
        return;

    const System* system = combat_info.objects.Object<System>(combat_info.system_id);
    if (!system)
        Logger().errorStream() << "AutoResolveCombat couldn't get system with id " << combat_info.system_id;


    Logger().debugStream() << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%";
    Logger().debugStream() << "AutoResolveCombat objects before resolution: " << combat_info.objects.Dump();

    // reasonably unpredictable but reproducible random seeding
    const int base_seed = combat_info.objects.begin()->first + CurrentTurn();


    // compile list of valid objects to attack or be attacked in this combat
    std::set<int> valid_target_object_ids;                          // all objects that can be attacked
    std::set<int> valid_attacker_object_ids;                        // all objects that can attack
    std::map<int, std::set<int> > empire_valid_attacker_object_ids; // objects that can attack that each empire owns

    for (ObjectMap::iterator it = combat_info.objects.begin(); it != combat_info.objects.end(); ++it) {
        const UniverseObject* obj = it->second;
        if (ObjectCanAttack(obj)) {
            valid_attacker_object_ids.insert(it->first);
            empire_valid_attacker_object_ids[obj->Owner()].insert(it->first);
        }
        if (ObjectCanBeAttacked(obj))
            valid_target_object_ids.insert(it->first);
    }


    // map from empire to set of IDs of objects that empire's objects
    // could potentially target.
    std::map<int, std::set<int> > empire_valid_target_object_ids;   // objects that each empire can attack
    for (std::set<int>::const_iterator target_it = valid_target_object_ids.begin();
         target_it != valid_target_object_ids.end(); ++target_it)
    {
        int object_id = *target_it;
        const UniverseObject* obj = combat_info.objects.Object(object_id);

        // which empire owns this object
        int owner = obj->Owner();

        // for all empires, can they attack this object?
        for (std::set<int>::const_iterator empire_it = combat_info.empire_ids.begin();
             empire_it != combat_info.empire_ids.end(); ++empire_it)
        {
            int empire_id = *empire_it;
            if (ObjectAttackableByEmpire(obj, empire_id))
                empire_valid_target_object_ids[empire_id].insert(object_id);
        }
    }


    // Each combat "round" a randomly-selected object in the battle attacks
    // something, if it is able to do so.  The number of rounds scales with the
    // number of objects, so the total actions per object is the same for
    // battles, roughly independent of number of objects in the battle
    const int NUM_COMBAT_ROUNDS = 10*valid_attacker_object_ids.size();

    for (int round = 1; round <= NUM_COMBAT_ROUNDS; ++round) {
        Seed(base_seed + round);    // ensure each combat round produces different results

        // ensure something can attack and something can be attacked
        if (valid_attacker_object_ids.empty()) {
            Logger().debugStream() << "Nothing left can attack; combat over";
            break;
        }
        if (empire_valid_target_object_ids.empty()) {
            Logger().debugStream() << "Nothing left can be attacked; combat over";
            break;
        }
        // empires may have valid targets, but nothing to attack with.  If all
        // empires have no attackers or no valid targers, combat is over
        bool someone_can_attack_something = false;
        for (std::map<int, std::set<int> >::const_iterator attacker_it = empire_valid_attacker_object_ids.begin();
             attacker_it != empire_valid_attacker_object_ids.end(); ++attacker_it)
        {
            if (empire_valid_target_object_ids.find(attacker_it->first) != empire_valid_target_object_ids.end()) {
                someone_can_attack_something = true;
                break;
            }
        }
        if (!someone_can_attack_something) {
            Logger().debugStream() << "No empire has valid targets and something to attack with; combat over.";
            break;
        }

        Logger().debugStream() << "Combat at " << system->Name() << " (" << combat_info.system_id << ") Round " << round;

        // select attacking object in battle
        SmallIntDistType attacker_id_num_dist = SmallIntDist(0, valid_attacker_object_ids.size() - 1);
        std::set<int>::const_iterator attacker_it = valid_attacker_object_ids.begin();
        std::advance(attacker_it, attacker_id_num_dist());
        assert(attacker_it != valid_attacker_object_ids.end());
        int attacker_id = *attacker_it;

        UniverseObject* attacker = combat_info.objects.Object(attacker_id);
        if (!attacker) {
            Logger().errorStream() << "AutoResolveCombat couldn't get object with id " << attacker_id;
            continue;
        }
        Logger().debugStream() << "Attacker: " << attacker->Name();


        // select object from valid targets for this object's owner.
        // get attacker owner id
        int attacker_owner_id = attacker->Owner();

        // get valid targets set for attacker owner
        std::map<int, std::set<int> >::iterator target_vec_it = empire_valid_target_object_ids.find(attacker_owner_id);
        if (target_vec_it == empire_valid_target_object_ids.end()) {
            Logger().debugStream() << "No targets for attacker with id: " << attacker_owner_id;
            continue;
        }
        const std::set<int>& valid_target_ids = target_vec_it->second;
        if (valid_target_ids.empty()) continue; // should be redundant with this entry being erased when emptied


        // select target object
        SmallIntDistType target_id_num_dist = SmallIntDist(0, valid_target_ids.size() - 1);
        std::set<int>::const_iterator target_it = valid_target_ids.begin();
        std::advance(target_it, target_id_num_dist());
        assert(target_it != valid_target_ids.end());
        int target_id = *target_it;

        UniverseObject* target = combat_info.objects.Object(target_id);
        if (!target) {
            Logger().errorStream() << "AutoResolveCombat couldn't get target object with id " << target_id;
            continue;
        }
        Logger().debugStream() << "Target: " << target->Name();


        // do actual attack
        if (Ship* attack_ship = universe_object_cast<Ship*>(attacker)) {
            if (Ship* target_ship = universe_object_cast<Ship*>(target)) {
                AttackShipShip(attack_ship, target_ship);
            } else if (Planet* target_planet = universe_object_cast<Planet*>(target)) {
                AttackShipPlanet(attack_ship, target_planet);
            }
        } else if (Planet* attack_planet = universe_object_cast<Planet*>(attacker)) {
            if (Ship* target_ship = universe_object_cast<Ship*>(target)) {
                AttackPlanetShip(attack_planet, target_ship);
            } else if (Planet* target_planet = universe_object_cast<Planet*>(target)) {
                AttackPlanetPlanet(attack_planet, target_planet);
            }
        }


        // check for destruction of ships
        if (universe_object_cast<Ship*>(target)) {
            if (target->CurrentMeterValue(METER_STRUCTURE) <= 0.0) {
                Logger().debugStream() << "!! Target Ship is destroyed!";
                // object id destroyed
                combat_info.destroyed_object_ids.insert(target_id);
                // all empires in battle know object was destroyed
                for (std::set<int>::const_iterator it = combat_info.empire_ids.begin();
                     it != combat_info.empire_ids.end(); ++it)
                {
                    int empire_id = *it;
                    if (empire_id != ALL_EMPIRES)
                        combat_info.destroyed_object_knowers[empire_id].insert(target_id);
                }
                // remove destroyed ship's ID from lists of valid attackers and targets
                valid_attacker_object_ids.erase(target_id);
                valid_target_object_ids.erase(target_id);   // probably not necessary as this set isn't used in this loop
                for (target_vec_it = empire_valid_target_object_ids.begin();
                     target_vec_it != empire_valid_target_object_ids.end(); ++target_vec_it)
                { target_vec_it->second.erase(target_id); }
                for (target_vec_it = empire_valid_attacker_object_ids.begin();
                     target_vec_it != empire_valid_attacker_object_ids.end(); ++target_vec_it)
                { target_vec_it->second.erase(target_id); }
            }
        } else if (universe_object_cast<Planet*>(target)) {
            if (target->CurrentMeterValue(METER_SHIELD) <= 0.0 &&
                target->CurrentMeterValue(METER_DEFENSE) <= 0.0 &&
                target->CurrentMeterValue(METER_CONSTRUCTION) <= 0.0)
            {
                Logger().debugStream() << "!! Target Planet is knocked out of battle";
                // remove disabled planet's ID from lists of valid attackers and targets
                valid_attacker_object_ids.erase(target_id);
                valid_target_object_ids.erase(target_id);   // probably not necessary as this set isn't used in this loop
                for (target_vec_it = empire_valid_target_object_ids.begin();
                     target_vec_it != empire_valid_target_object_ids.end(); ++target_vec_it)
                { target_vec_it->second.erase(target_id); }
            }
        }

        // check if any empire has no remaining target or attacker objects.
        // If so, remove that empire's entry
        std::map<int, std::set<int> > temp = empire_valid_target_object_ids;
        for (target_vec_it = empire_valid_target_object_ids.begin();
             target_vec_it != empire_valid_target_object_ids.end(); ++target_vec_it)
        {
            if (target_vec_it->second.empty()) {
                temp.erase(target_vec_it->first);
                Logger().debugStream() << "No valid targets left for empire with id: " << target_vec_it->first;
            }
        }
        empire_valid_target_object_ids = temp;

        temp = empire_valid_attacker_object_ids;
        for (target_vec_it = empire_valid_attacker_object_ids.begin();
             target_vec_it != empire_valid_attacker_object_ids.end(); ++target_vec_it)
        {
            if (target_vec_it->second.empty()) {
                temp.erase(target_vec_it->first);
                Logger().debugStream() << "No valid attacking objects left for empire with id: " << target_vec_it->first;
            }
        }
        empire_valid_attacker_object_ids = temp;
    }

    // ensure every participant knows what happened.  TODO: this should probably
    // be more discriminating about what info is or isn't copied.
    for (std::map<int, ObjectMap>::iterator it = combat_info.empire_known_objects.begin();
         it != combat_info.empire_known_objects.end(); ++it)
    {
        it->second.Copy(combat_info.objects);
    }

    Logger().debugStream() << "AutoResolveCombat objects after resolution: " << combat_info.objects.Dump();
}
