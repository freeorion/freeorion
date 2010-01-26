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
    system_id(UniverseObject::INVALID_OBJECT_ID)
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

    // find ships and their owners in system
    std::vector<const Ship*> ships = universe_objects.FindObjects<Ship>();
    for (std::vector<const Ship*>::const_iterator it = ships.begin(); it != ships.end(); ++it) {
        const Ship* ship = *it;
        int ship_id = ship->ID();

        // add owners to empires that have assets in this battle
        const std::set<int>& owners = ship->Owners();
        for (std::set<int>::const_iterator owner_it = owners.begin(); owner_it != owners.end(); ++owner_it)
            empire_ids.insert(*owner_it);

        // add copy of ship to full / complete copy of objects in system
        Ship* copy = ship->Clone();
        objects.Insert(ship_id, copy);
    }

    // find planets and their owners in system
    std::vector<const Planet*> planets = universe_objects.FindObjects<Planet>();
    for (std::vector<const Planet*>::const_iterator it = planets.begin(); it != planets.end(); ++it) {
        const Planet* planet = *it;
        int planet_id = planet->ID();

        // add owners to empires that have assets in this battle
        const std::set<int>& owners = planet->Owners();
        for (std::set<int>::const_iterator owner_it = owners.begin(); owner_it != owners.end(); ++owner_it)
            empire_ids.insert(*owner_it);

        // add copy of ship to full / complete copy of objects in system
        Planet* copy = planet->Clone();
        objects.Insert(planet_id, copy);
    }

    // TODO: should buildings be considered separately?

    // now that all participants in the battle have been found, loop through
    // ships and planets again to assemble each participant empire's latest
    // known information about all objects in this battle
    for (std::vector<const Ship*>::const_iterator it = ships.begin(); it != ships.end(); ++it) {
        const Ship* ship = *it;
        int ship_id = ship->ID();

        for (std::set<int>::const_iterator empire_it = empire_ids.begin(); empire_it != empire_ids.end(); ++empire_it) {
            int empire_id = *empire_it;
            if (universe.GetObjectVisibilityByEmpire(ship_id, empire_id) >= VIS_BASIC_VISIBILITY) {
                Ship* visibility_limited_copy = ship->Clone(empire_id);
                empire_known_objects[empire_id].Insert(ship_id, visibility_limited_copy);
            }
        }
    }
    for (std::vector<const Planet*>::const_iterator it = planets.begin(); it != planets.end(); ++it) {
        const Planet* planet = *it;
        int planet_id = planet->ID();

        for (std::set<int>::const_iterator empire_it = empire_ids.begin(); empire_it != empire_ids.end(); ++empire_it) {
            int empire_id = *empire_it;
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
    system_id = UniverseObject::INVALID_OBJECT_ID;
    empire_ids.clear();
    objects.Clear();
    for (std::map<int, ObjectMap>::iterator it = empire_known_objects.begin(); it != empire_known_objects.end(); ++it)
        it->second.Clear();
    destroyed_object_ids.clear();
    destroyed_object_knowers.clear();
}


////////////////////////////////////////////////
// ResolveCombat
////////////////////////////////////////////////
namespace {
    void Attack(Ship* attacker, Ship* target) {
        if (!attacker || ! target) return;

        const ShipDesign* attacker_design = attacker->Design();
        if (!attacker_design)
            return;
        double damage = attacker_design->Attack();

        if (damage <= 0.0)
            return;

        Meter* target_shield = target->GetMeter(METER_SHIELD);
        Meter* target_health = target->GetMeter(METER_HEALTH);
        if (!target_shield || ! target_health) {
            Logger().errorStream() << "couldn't get target health or shield meter";
            return;
        }

        // damage shields, limited by shield current value and damage amount.
        // remaining damage, if any, above shield current value goes to health
        double shield_damage = std::min(target_shield->Current(), damage);
        double health_damage = 0.0;
        if (shield_damage >= target_shield->Current())
            health_damage = std::min(target_health->Current(), damage - shield_damage);

        if (shield_damage >= 0) {
            target->GetMeter(METER_SHIELD)->AdjustCurrent(-shield_damage);
            Logger().debugStream() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << shield_damage << " shield damage to Ship " << target->Name() << " (" << target->ID() << ")";
        }
        if (health_damage >= 0) {
            target->GetMeter(METER_HEALTH)->AdjustCurrent(-health_damage);
            Logger().debugStream() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << health_damage << " health damage to Ship " << target->Name() << " (" << target->ID() << ")";
        }
    }

    void Attack(Ship* attacker, Planet* target) {
        if (!attacker || ! target) return;

        const ShipDesign* attacker_design = attacker->Design();
        if (!attacker_design)
            return;
        double damage = attacker_design->Attack();

        if (damage <= 0.0)
            return;

        Meter* target_shield = target->GetMeter(METER_SHIELD);
        Meter* target_health = target->GetMeter(METER_HEALTH);
        if (!target_shield || ! target_health) {
            Logger().errorStream() << "couldn't get target health or shield meter";
            return;
        }

        // damage shields, limited by shield current value and damage amount.
        // remaining damage, if any, above shield current value goes to health
        double shield_damage = std::min(target_shield->Current(), damage);
        double health_damage = 0.0;
        if (shield_damage >= target_shield->Current())
            health_damage = std::min(target_health->Current(), damage - shield_damage);

        if (shield_damage >= 0) {
            target->GetMeter(METER_SHIELD)->AdjustCurrent(-shield_damage);
            Logger().debugStream() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << shield_damage << " shield damage to Planet " << target->Name() << " (" << target->ID() << ")";
        }
        if (health_damage >= 0) {
            target->GetMeter(METER_HEALTH)->AdjustCurrent(-health_damage);
            Logger().debugStream() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << health_damage << " health damage to Planet " << target->Name() << " (" << target->ID() << ")";
        }
    }

    void Attack(Planet* attacker, Ship* target) {
        if (!attacker || ! target) return;

        double damage = attacker->GetMeter(METER_DEFENSE)->Current();   // planet "Defense" meter is actually its attack power

        if (damage <= 0.0)
            return;

        Meter* target_shield = target->GetMeter(METER_SHIELD);
        Meter* target_health = target->GetMeter(METER_HEALTH);
        if (!target_shield || ! target_health) {
            Logger().errorStream() << "couldn't get target health or shield meter";
            return;
        }

        // damage shields, limited by shield current value and damage amount.
        // remaining damage, if any, above shield current value goes to health
        double shield_damage = std::min(target_shield->Current(), damage);
        double health_damage = 0.0;
        if (shield_damage >= target_shield->Current())
            health_damage = std::min(target_health->Current(), damage - shield_damage);

        if (shield_damage >= 0) {
            target->GetMeter(METER_SHIELD)->AdjustCurrent(-shield_damage);
            Logger().debugStream() << "COMBAT: Planet " << attacker->Name() << " (" << attacker->ID() << ") does " << shield_damage << " shield damage to Ship " << target->Name() << " (" << target->ID() << ")";
        }
        if (health_damage >= 0) {
            target->GetMeter(METER_HEALTH)->AdjustCurrent(-health_damage);
            Logger().debugStream() << "COMBAT: Planet " << attacker->Name() << " (" << attacker->ID() << ") does " << health_damage << " health damage to Ship " << target->Name() << " (" << target->ID() << ")";
        }
    }

    void Attack(Planet* attacker, Planet* target) {
        // intentionally left empty
    }
}

void ResolveCombat(CombatInfo& combat_info) {
    if (combat_info.objects.Empty())
        return;

    System* system = GetObject<System>(combat_info.system_id);
    if (!system) {
        Logger().errorStream() << "ResolveCombat couldn't get system with id " << combat_info.system_id;
    }


    // reasonably unpredictable, but reproducible random seeding
    const UniverseObject* first_object = combat_info.objects.begin()->second;
    int seed = first_object->ID();
    Seed(seed);

    std::vector<UniverseObject*> all_combat_objects = combat_info.objects.FindObjects<UniverseObject>();
    SmallIntDistType object_num_dist = SmallIntDist(0, all_combat_objects.size() - 1);  // to pick an object from the vector


    const int NUM_COMBAT_ROUNDS = 100;
    for (int round = 1; round <= NUM_COMBAT_ROUNDS; ++round) {
        Logger().debugStream() << "Combat at " << system->Name() << " (" << combat_info.system_id << ") Round " << round;

        // select attacking object in battle
        int attacker_index = object_num_dist();

        // check if object is already destroyed.  if so, skip this round
        if (combat_info.destroyed_object_ids.find(attacker_index) != combat_info.destroyed_object_ids.end())
            continue;


        // TODO:: ensure target object is only selected from objects not owned by attacker object's owner


        // select target object
        int target_index = object_num_dist();

        // check if object is already destroyed.  if so, skip this round
        if (combat_info.destroyed_object_ids.find(target_index) != combat_info.destroyed_object_ids.end())
            continue;

        // get objects
        UniverseObject* attacker = 0;
        UniverseObject* target = 0;
        try {
            attacker = all_combat_objects.at(attacker_index);
            target = all_combat_objects.at(target_index);
        } catch (std::out_of_range) {
            Logger().errorStream() << "tried to get out of range combat object?! index " << attacker_index << " or " << target_index;
            continue;
        }

        // check ownership... avoid friendly fire.  Can be removed if above TODO re: ownership is fixed
        const std::set<int>& attacker_owners = attacker->Owners();
        const std::set<int>& target_owners = target->Owners();
        bool abort = false;
        for (std::set<int>::const_iterator it = attacker_owners.begin(); it != attacker_owners.end(); ++it) {
            if (target_owners.find(*it) != target_owners.end()) {
                abort = true;
                break;
            }
        }
        if (abort)
            continue;   // attacker and target had one of the same owners.  skip this combination to avoid friendly fire.

        // hand-written double dispatch?!
        if (Ship* attack_ship = universe_object_cast<Ship*>(attacker)) {
            if (Ship* target_ship = universe_object_cast<Ship*>(target)) {
                Attack(attack_ship, target_ship);
            } else if (Planet* target_planet = universe_object_cast<Planet*>(target)) {
                Attack(attack_ship, target_planet);
            }
        } else if (Planet* attack_planet = universe_object_cast<Planet*>(attacker)) {
            if (Ship* target_ship = universe_object_cast<Ship*>(target)) {
                Attack(attack_planet, target_ship);
            } else if (Planet* target_planet = universe_object_cast<Planet*>(target)) {
                Attack(attack_planet, target_planet);
            }
        }


        // check for destruction
        if (target->GetMeter(METER_HEALTH)->Current() <= 0.0) {
            int target_id = target->ID();

            // object id destroyed
            combat_info.destroyed_object_ids.insert(target_id);
            // all empires in battle know object was destroyed
            for (std::set<int>::const_iterator it = combat_info.empire_ids.begin(); it != combat_info.empire_ids.end(); ++it) {
                int empire_id = *it;
                combat_info.destroyed_object_knowers[empire_id].insert(target_id);
            }
        }
    }
}
