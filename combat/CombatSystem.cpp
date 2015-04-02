#include "CombatSystem.h"
#include "CombatEvents.h"

#include "../universe/Universe.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../universe/Predicates.h"

#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../Empire/Empire.h"

#include "../util/Logger.h"
#include "../util/Random.h"

#include "../server/ServerApp.h"
#include "../network/Message.h"

#include <boost/make_shared.hpp>

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
    TemporaryPtr<System> system = ::GetSystem(system_id);
    if (!system) {
        ErrorLogger() << "CombatInfo constructed with invalid system id: " << system_id;
        return;
    }

    // add system to full / complete objects in combat - NOTE: changed from copy of system
    objects.Insert(system);


    // find ships and their owners in system
    std::vector<TemporaryPtr<Ship> > ships =
        Objects().FindObjects<Ship>(system->ShipIDs());

    for (std::vector<TemporaryPtr<Ship> >::const_iterator ship_it = ships.begin();
         ship_it != ships.end(); ++ship_it)
    {
        TemporaryPtr<Ship> ship = *ship_it;
        // add owner to empires that have assets in this battle
        empire_ids.insert(ship->Owner());

        objects.Insert(ship);
    }

    // find planets and their owners in system
    std::vector<TemporaryPtr<Planet> > planets =
        Objects().FindObjects<Planet>(system->PlanetIDs());

    for (std::vector<TemporaryPtr<Planet> >::const_iterator planet_it = planets.begin();
         planet_it != planets.end(); ++planet_it)
    {
        TemporaryPtr<Planet> planet = *planet_it;
        // if planet is populated, add owner to empires that have assets in this battle
        if (planet->CurrentMeterValue(METER_POPULATION) > 0.0)
            empire_ids.insert(planet->Owner());

        objects.Insert(planet);
    }

    // TODO: should buildings be considered separately?

    // now that all participants in the battle have been found, loop through
    // objects again to assemble each participant empire's latest
    // known information about all objects in this battle

    // system
    for (std::set<int>::const_iterator empire_it = empire_ids.begin();
         empire_it != empire_ids.end(); ++empire_it)
    {
        int empire_id = *empire_it;
        if (empire_id == ALL_EMPIRES)
            continue;
        empire_known_objects[empire_id].Insert(GetEmpireKnownSystem(system->ID(), empire_id));
    }

    // ships
    for (std::vector<TemporaryPtr<Ship> >::const_iterator it = ships.begin();
         it != ships.end(); ++it)
    {
        TemporaryPtr<Ship> ship = *it;
        int ship_id = ship->ID();
        TemporaryPtr<const Fleet> fleet = GetFleet(ship->FleetID());
        if (!fleet) {
            ErrorLogger() << "CombatInfo::CombatInfo couldn't get fleet with id "
                                   << ship->FleetID() << " in system " << system->Name() << " (" << system_id << ")";
            continue;
        }

        for (std::set<int>::const_iterator empire_it = empire_ids.begin();
             empire_it != empire_ids.end(); ++empire_it)
        {
            int empire_id = *empire_it;
            if (empire_id == ALL_EMPIRES)
                continue;
            if (GetUniverse().GetObjectVisibilityByEmpire(ship_id, empire_id) >= VIS_BASIC_VISIBILITY ||
                   (fleet->Aggressive() &&
                       (empire_id == ALL_EMPIRES ||
                        fleet->Unowned() ||
                        Empires().GetDiplomaticStatus(empire_id, fleet->Owner()) == DIPLO_WAR)))
            { empire_known_objects[empire_id].Insert(GetEmpireKnownShip(ship->ID(), empire_id));}
        }
    }

    // planets
    for (std::vector<TemporaryPtr<Planet> >::const_iterator it = planets.begin();
         it != planets.end(); ++it)
    {
        TemporaryPtr<Planet> planet = *it;
        int planet_id = planet->ID();

        for (std::set<int>::const_iterator empire_it = empire_ids.begin(); empire_it != empire_ids.end(); ++empire_it) {
            int empire_id = *empire_it;
            if (empire_id == ALL_EMPIRES)
                continue;
            if (GetUniverse().GetObjectVisibilityByEmpire(planet_id, empire_id) >= VIS_BASIC_VISIBILITY) {
                empire_known_objects[empire_id].Insert(GetEmpireKnownPlanet(planet->ID(), empire_id));
            }
        }
    }

    // after battle is simulated, any changes to latest known or actual objects
    // will be copied back to the main Universe's ObjectMap and the Universe's
    // empire latest known objects ObjectMap - NOTE: Using the real thing now
}

TemporaryPtr<const System> CombatInfo::GetSystem() const
{ return this->objects.Object<System>(this->system_id); }

TemporaryPtr<System> CombatInfo::GetSystem()
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

    for (std::map<int, ObjectMap>::iterator it = filtered_empire_known_objects.begin();
         it != filtered_empire_known_objects.end(); ++it)
    { it->second.Clear(); }
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

void CombatInfo::GetDestroyedObjectKnowersToSerialize(std::map<int, std::set<int> >&
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

////////////////////////////////////////////////
// AutoResolveCombat
////////////////////////////////////////////////
namespace {
    struct PartAttackInfo {
        PartAttackInfo(ShipPartClass part_class_, const std::string& part_name_, float part_attack_) :
        part_class(part_class_),
        part_type_name(part_name_),
        part_attack(part_attack_)
        {}
        ShipPartClass   part_class;
        std::string     part_type_name;
        float           part_attack;
    };

    void AttackShipShip(TemporaryPtr<Ship> attacker, float damage, TemporaryPtr<Ship> target, CombatInfo& combat_info, int bout, int round) {
        if (!attacker || ! target) return;

        std::set<int>& damaged_object_ids = combat_info.damaged_object_ids;

        Meter* target_structure = target->UniverseObject::GetMeter(METER_STRUCTURE);
        if (!target_structure) {
            ErrorLogger() << "couldn't get target structure or shield meter";
            return;
        }

        Meter* target_shield = target->UniverseObject::GetMeter(METER_SHIELD);
        float shield = (target_shield ? target_shield->Current() : 0.0f);

        DebugLogger() << "AttackShipShip: attacker: " << attacker->Name() << " damage: " << damage
                               << "  target: " << target->Name() << " shield: " << target_shield->Current()
                                                                 << " structure: " << target_structure->Current();

        damage = std::max(0.0f, damage - shield);

        if (damage > 0.0f) {
            target_structure->AddToCurrent(-damage);
            damaged_object_ids.insert(target->ID());
            if (GetOptionsDB().Get<bool>("verbose-logging"))
                DebugLogger() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << damage << " damage to Ship " << target->Name() << " (" << target->ID() << ")";
        }

        combat_info.combat_events.push_back(boost::make_shared<AttackEvent>(bout, round, attacker->ID(), target->ID(), damage));

        attacker->SetLastTurnActiveInCombat(CurrentTurn());
        target->SetLastTurnActiveInCombat(CurrentTurn());
    }

    void AttackShipPlanet(TemporaryPtr<Ship> attacker, float damage, TemporaryPtr<Planet> target, CombatInfo& combat_info, int bout, int round) {
        if (!attacker || ! target) return;
        if (damage <= 0.0f)
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

        if (GetOptionsDB().Get<bool>("verbose-logging")) {
            DebugLogger() << "AttackShipPlanet: attacker: " << attacker->Name() << " damage: " << damage
                               << "\ntarget: " << target->Name() << " shield: " << target_shield->Current()
                                                                 << " defense: " << target_defense->Current()
                                                                 << " infra: " << target_construction->Current();
        }

        // damage shields, limited by shield current value and damage amount.
        // remaining damage, if any, above shield current value goes to defense.
        // remaining damage, if any, above defense current value goes to construction
        float shield_damage = std::min(target_shield->Current(), damage);
        float defense_damage = 0.0f;
        float construction_damage = 0.0f;
        if (shield_damage >= target_shield->Current())
            defense_damage = std::min(target_defense->Current(), damage - shield_damage);

        if (shield_damage > 0 || defense_damage > 0 || construction_damage > 0)
            damaged_object_ids.insert(target->ID());

        if (defense_damage >= target_defense->Current())
            construction_damage = std::min(target_construction->Current(), damage - shield_damage - defense_damage);

        if (shield_damage >= 0) {
            target_shield->AddToCurrent(-shield_damage);
            if (GetOptionsDB().Get<bool>("verbose-logging"))
                DebugLogger() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << shield_damage << " shield damage to Planet " << target->Name() << " (" << target->ID() << ")";
        }
        if (defense_damage >= 0) {
            target_defense->AddToCurrent(-defense_damage);
            if (GetOptionsDB().Get<bool>("verbose-logging"))
                DebugLogger() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << defense_damage << " defense damage to Planet " << target->Name() << " (" << target->ID() << ")";
        }
        if (construction_damage >= 0) {
            target_construction->AddToCurrent(-construction_damage);
            if (GetOptionsDB().Get<bool>("verbose-logging"))
                DebugLogger() << "COMBAT: Ship " << attacker->Name() << " (" << attacker->ID() << ") does " << construction_damage << " instrastructure damage to Planet " << target->Name() << " (" << target->ID() << ")";
        }

        combat_info.combat_events.push_back(boost::make_shared<AttackEvent>(bout, round, attacker->ID(), target->ID(), damage));

        attacker->SetLastTurnActiveInCombat(CurrentTurn());
        target->SetLastTurnAttackedByShip(CurrentTurn());
    }

    void AttackPlanetShip(TemporaryPtr<Planet> attacker, TemporaryPtr<Ship> target, CombatInfo& combat_info, int bout, int round) {
        if (!attacker || ! target) return;

        float damage = 0.0f;
        const Meter* attacker_damage = attacker->UniverseObject::GetMeter(METER_DEFENSE);
        if (attacker_damage)
            damage = attacker_damage->Current();   // planet "Defense" meter is actually its attack power

        std::set<int>& damaged_object_ids = combat_info.damaged_object_ids;

        Meter* target_structure = target->UniverseObject::GetMeter(METER_STRUCTURE);
        if (!target_structure) {
            ErrorLogger() << "couldn't get target structure or shield meter";
            return;
        }

        Meter* target_shield = target->UniverseObject::GetMeter(METER_SHIELD);
        float shield = (target_shield ? target_shield->Current() : 0.0f);

        if (GetOptionsDB().Get<bool>("verbose-logging")) {
            DebugLogger() << "AttackPlanetShip: attacker: " << attacker->Name() << " damage: " << damage
                               << "  target: " << target->Name() << " shield: " << target_shield->Current()
                                                                 << " structure: " << target_structure->Current();
        }

        damage = std::max(0.0f, damage - shield);

        if (damage > 0.0f) {
            target_structure->AddToCurrent(-damage);
            damaged_object_ids.insert(target->ID());
            if (GetOptionsDB().Get<bool>("verbose-logging"))
                DebugLogger() << "COMBAT: Planet " << attacker->Name() << " (" << attacker->ID() << ") does " << damage << " damage to Ship " << target->Name() << " (" << target->ID() << ")";
        }

        combat_info.combat_events.push_back(boost::make_shared<AttackEvent>(bout, round, attacker->ID(), target->ID(), damage));

        target->SetLastTurnActiveInCombat(CurrentTurn());
    }

    void Attack(TemporaryPtr<UniverseObject>& attacker, const PartAttackInfo& weapon, TemporaryPtr<UniverseObject>& target, CombatInfo& combat_info, int bout, int round){
        TemporaryPtr<Ship>      attack_ship =   boost::dynamic_pointer_cast<Ship>(attacker);
        TemporaryPtr<Planet>    attack_planet = boost::dynamic_pointer_cast<Planet>(attacker);
        TemporaryPtr<Ship>      target_ship =   boost::dynamic_pointer_cast<Ship>(target);
        TemporaryPtr<Planet>    target_planet = boost::dynamic_pointer_cast<Planet>(target);

        if (attack_ship && target_ship) {
            AttackShipShip(attack_ship, weapon.part_attack, target_ship, combat_info, bout, round);
        } else if (attack_ship && target_planet) {
            AttackShipPlanet(attack_ship, weapon.part_attack, target_planet, combat_info, bout, round);
        } else if (attack_planet && target_ship) {
            AttackPlanetShip(attack_planet, target_ship, combat_info, bout, round);
        } else if (attack_planet && target_planet) {
            // Planets don't attack each other, silly
        }
    }

    bool ObjectCanBeAttacked(TemporaryPtr<const UniverseObject> obj) {
        if (!obj)
            return false;

        UniverseObjectType obj_type = obj->ObjectType();

        if (obj_type == OBJ_SHIP)
            return true;

        if (obj_type == OBJ_PLANET) {
            if (!obj->Unowned() || obj->CurrentMeterValue(METER_POPULATION) > 0.0f)
                return true;
            return false;
        }

        return false;
    }

    bool ObjectAttackableByEmpire(TemporaryPtr<const UniverseObject> obj, int empire_id) {
        if (obj->OwnedBy(empire_id))
            return false;
        if (obj->Unowned() && empire_id == ALL_EMPIRES)
            return false;

        if (empire_id != ALL_EMPIRES && !obj->Unowned() &&
            Empires().GetDiplomaticStatus(empire_id, obj->Owner()) != DIPLO_WAR)
        { return false; }

        if (GetUniverse().GetObjectVisibilityByEmpire(obj->ID(), empire_id) < VIS_BASIC_VISIBILITY)
            return false;

        return ObjectCanBeAttacked(obj);
    }

    // monsters / natives can attack any planet, but can only attack
    // visible ships or ships that are in aggressive fleets
    bool ObjectAttackableByMonsters(TemporaryPtr<const UniverseObject> obj, float monster_detection = 0.0) {
        if (obj->Unowned())
            return false;

        //DebugLogger() << "Testing if object " << obj->Name() << " is attackable by monsters";

        UniverseObjectType obj_type = obj->ObjectType();
        if (obj_type == OBJ_PLANET) {
            return true;
        } else if (obj_type == OBJ_SHIP) {
            float stealth = obj->CurrentMeterValue(METER_STEALTH);
            if (monster_detection >= stealth)
                return true;
        }
        //DebugLogger() << "... ... is NOT attackable by monsters";
        return false;
    }

    bool ObjectCanAttack(TemporaryPtr<const UniverseObject> obj) {
        if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(obj)) {
            return ship->IsArmed();
        } else if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(obj)) {
            return planet->CurrentMeterValue(METER_DEFENSE) > 0.0;
        } else {
            return false;
        }
    }

    std::vector<PartAttackInfo> ShipWeaponsStrengths(TemporaryPtr<const Ship> ship) {
        std::vector<PartAttackInfo> retval;
        if (!ship)
            return retval;
        const ShipDesign* design = GetShipDesign(ship->DesignID());
        if (!design)
            return retval;
        const std::vector<std::string>& parts = design->Parts();

        // for each weapon part, get its damage meter value
        for (std::vector<std::string>::const_iterator part_it = parts.begin();
             part_it != parts.end(); ++part_it)
        {
            const std::string& part_name = *part_it;
            const PartType* part = GetPartType(part_name);
            if (!part)
                continue;
            ShipPartClass part_class = part->Class();

            // get the attack power for each weapon part
            float part_attack = 0.0f;

            if (part_class == PC_SHORT_RANGE || part_class == PC_POINT_DEFENSE || part_class == PC_MISSILES || part_class == PC_FIGHTERS)
                part_attack = ship->CurrentPartMeterValue(METER_DAMAGE, part_name);

            if (part_attack > 0.0f)
                retval.push_back(PartAttackInfo(part_class, part_name, part_attack));
        }
        return retval;
    }

    // Information about a single empire during combat
    struct EmpireCombatInfo{
        std::set<int> attacker_ids;
        std::set<int> target_ids;
        bool HasTargets() const     { return !target_ids.empty(); }
        bool HasAttackers() const   { return !attacker_ids.empty(); }
    };

    // Populate lists of things that can attack and be attacked. List attackers also by empire.
    void GetAttackersAndTargets(const CombatInfo& combat_info, std::set<int>& valid_target_object_ids,
                                std::set<int>& valid_attacker_object_ids,
                                std::map<int, EmpireCombatInfo>& empire_infos)
    {
        for (ObjectMap::const_iterator<> it = combat_info.objects.const_begin(); it != combat_info.objects.const_end(); ++it) {
            TemporaryPtr<const UniverseObject> obj = *it;
            //DebugLogger() << "Considerting object " << obj->Name() << " owned by " << obj->Owner();
            if (ObjectCanAttack(obj)) {
                //DebugLogger() << "... can attack";
                valid_attacker_object_ids.insert(it->ID());
                empire_infos[obj->Owner()].attacker_ids.insert(it->ID());
            }
            if (ObjectCanBeAttacked(obj)) {
                //DebugLogger() << "... can be attacked";
                valid_target_object_ids.insert(it->ID());
            }
        }
    }

    // Calculate monster detection strength in system
    float GetMonsterDetection(const CombatInfo& combat_info) {
        float monster_detection = 0.0;
        for (ObjectMap::const_iterator<> it = combat_info.objects.const_begin(); it != combat_info.objects.const_end(); ++it) {
            TemporaryPtr<const UniverseObject> obj = *it;
            if (obj->Unowned() && (obj->ObjectType() == OBJ_SHIP || obj->ObjectType() == OBJ_PLANET )){
                monster_detection = std::max(monster_detection, obj->CurrentMeterValue(METER_DETECTION));
            }
        }
        return monster_detection;
    }

    /// A collection of information the autoresolution must keep around
    struct AutoresolveInfo {
        typedef std::map<int, EmpireCombatInfo>::iterator empire_it;

        std::set<int>                   valid_target_object_ids;    // all objects that can be attacked
        std::set<int>                   valid_attacker_object_ids;  // all objects that can attack
        std::map<int, EmpireCombatInfo> empire_infos;               // empire specific information
        float                           monster_detection;          // monster's detections strength
        CombatInfo&                     combat_info;                // a reference to the combat info

        AutoresolveInfo(CombatInfo& combat_info):
        combat_info(combat_info)
        {
            monster_detection = GetMonsterDetection(combat_info);
            PopulateAttackersAndTargets(combat_info);
            PopulateEmpireTargets(combat_info);
        }

        // Return true if some empire that can attack has some targets that it can attack
        bool CanSomeoneAttackSomething() const {
            for (std::map<int, EmpireCombatInfo>::const_iterator attacker_it = empire_infos.begin();
                 attacker_it != empire_infos.end(); ++attacker_it)
            {
                if (attacker_it->second.HasTargets() && attacker_it->second.HasAttackers())
                    return true;
            }
            return false;
        }

        /// Removes dead units from lists of attackers and defenders
        void CullTheDead(int bout) {
            for (ObjectMap::const_iterator<> it = combat_info.objects.const_begin();
                 it != combat_info.objects.const_end(); ++it)
            {
                TemporaryPtr<const UniverseObject> obj = *it;

                // There may be objects, for example unowned planets, that were
                // not a part of the battle to start with. Ignore them
                if (valid_attacker_object_ids.find(obj->ID()) == valid_attacker_object_ids.end() &&
                    valid_target_object_ids.find(obj->ID()) == valid_target_object_ids.end())
                { continue; }

                // Check if the target was destroyed and update lists if yes
                bool destroyed = CheckDestruction(obj);
                if (destroyed)
                    combat_info.combat_events.push_back(boost::make_shared<IncapacitationEvent>(bout, obj->ID()));
            }
        }

        /// Checks if target is destroyed and if it is, update lists of living objects.
        /// Return true if is incapacitated
        bool CheckDestruction(const TemporaryPtr<const UniverseObject>& target) {
            int target_id = target->ID();
            // check for destruction of target object
            if (target->ObjectType() == OBJ_SHIP) {
                if (target->CurrentMeterValue(METER_STRUCTURE) <= 0.0) {
                    if (GetOptionsDB().Get<bool>("verbose-logging"))
                        DebugLogger() << "!! Target Ship is destroyed!";
                    // object id destroyed
                    combat_info.destroyed_object_ids.insert(target_id);
                    // all empires in battle know object was destroyed
                    for (std::set<int>::const_iterator it = combat_info.empire_ids.begin();
                         it != combat_info.empire_ids.end(); ++it)
                    {
                        int empire_id = *it;
                        if (empire_id != ALL_EMPIRES) {
                            if (GetOptionsDB().Get<bool>("verbose-logging"))
                                DebugLogger() << "Giving knowledge of destroyed object " << target_id << " to empire " << empire_id;
                            combat_info.destroyed_object_knowers[empire_id].insert(target_id);
                        }
                    }
                    
                    // remove destroyed ship's ID from lists of valid attackers and targets
                    valid_attacker_object_ids.erase(target_id);
                    valid_target_object_ids.erase(target_id);   // probably not necessary as this set isn't used in this loop
                    
                    for (empire_it targeter_it = empire_infos.begin();
                        targeter_it != empire_infos.end(); ++targeter_it)
                    { targeter_it->second.target_ids.erase(target_id); }
                        
                    // Remove target from its empire's list of attackers
                    empire_infos[target->Owner()].attacker_ids.erase(target_id);
                    CleanEmpires();
                    return true;
                }

            } else if (target->ObjectType() == OBJ_PLANET) {
                if (!ObjectCanAttack(target) &&
                    valid_attacker_object_ids.find(target_id) != valid_attacker_object_ids.end())
                {
                    if (GetOptionsDB().Get<bool>("verbose-logging"))
                        DebugLogger() << "!! Target Planet defenses knocked out, can no longer attack";
                    // remove disabled planet's ID from lists of valid attackers
                    valid_attacker_object_ids.erase(target_id);
                }
                if (target->CurrentMeterValue(METER_SHIELD) <= 0.0 &&
                    target->CurrentMeterValue(METER_DEFENSE) <= 0.0 &&
                    target->CurrentMeterValue(METER_CONSTRUCTION) <= 0.0)
                {
                    if (GetOptionsDB().Get<bool>("verbose-logging")) {
                        DebugLogger() << "!! Target Planet is entirely knocked out of battle";
                    }

                    // remove disabled planet's ID from lists of valid targets
                    valid_target_object_ids.erase(target_id);   // probably not necessary as this set isn't used in this loop

                    for (empire_it targeter_it = empire_infos.begin();
                         targeter_it != empire_infos.end(); ++targeter_it)
                    { targeter_it->second.target_ids.erase(target_id); }

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
            for (empire_it empire_it = empire_infos.begin();
                 empire_it != empire_infos.end(); ++empire_it)
            {
                if (!empire_it->second.HasTargets() && ! empire_it->second.HasAttackers()) {
                    temp.erase(empire_it->first);
                    if (GetOptionsDB().Get<bool>("verbose-logging"))
                        DebugLogger() << "No valid attacking objects left for empire with id: " << empire_it->first;
                }
            }
            empire_infos = temp;
        }

        /// Returns the list of attacker ids in a random order
        void GiveAttackersShuffled(std::vector<int>& shuffled) {
            shuffled.clear();
            shuffled.insert(shuffled.begin(), valid_attacker_object_ids.begin(), valid_attacker_object_ids.end());

            const unsigned swaps = shuffled.size();
            for (unsigned i = 0; i < swaps; ++i){
                int pos1 = RandInt(0, shuffled.size() - 1);
                int pos2 = RandInt(0, shuffled.size() - 1);
                std::swap(shuffled[pos1], shuffled[pos2]);
            }
        }
    private:
        typedef std::set<int>::const_iterator const_id_iterator;

        // Populate lists of things that can attack and be attacked. List attackers also by empire.
        void PopulateAttackersAndTargets(const CombatInfo& combat_info) {
            for (ObjectMap::const_iterator<> it = combat_info.objects.const_begin(); it != combat_info.objects.const_end(); ++it) {
                TemporaryPtr<const UniverseObject> obj = *it;
                //DebugLogger() << "Considerting object " << obj->Name() << " owned by " << obj->Owner();
                if (ObjectCanAttack(obj)) {
                    //DebugLogger() << "... can attack";
                    valid_attacker_object_ids.insert(it->ID());
                    empire_infos[obj->Owner()].attacker_ids.insert(it->ID());
                }
                if (ObjectCanBeAttacked(obj)) {
                    //DebugLogger() << "... can be attacked";
                    valid_target_object_ids.insert(it->ID());
                }
            }
        }

        // Get a map from empire to set of IDs of objects that empire's objects
        // could potentially target.
        void PopulateEmpireTargets(const CombatInfo& combat_info) {
            for (std::set<int>::const_iterator target_it = valid_target_object_ids.begin();
                 target_it != valid_target_object_ids.end(); ++target_it)
            {
                int object_id = *target_it;
                TemporaryPtr<const UniverseObject> obj = combat_info.objects.Object(object_id);
                // DebugLogger() << "Considering attackability of object " << obj->Name() << " owned by " << obj->Owner();

                // for all empires, can they attack this object?
                for (std::set<int>::const_iterator empire_it = combat_info.empire_ids.begin();
                     empire_it != combat_info.empire_ids.end(); ++empire_it)
                {
                    int attacking_empire_id = *empire_it;
                    if (attacking_empire_id == ALL_EMPIRES) {
                        if (ObjectAttackableByMonsters(obj, monster_detection)) {
                            // DebugLogger() << "object: " << obj->Name() << " attackable by monsters";
                            empire_infos[ALL_EMPIRES].target_ids.insert(object_id);
                        }

                    } else {
                        if (ObjectAttackableByEmpire(obj, attacking_empire_id)) {
                            // DebugLogger() << "object: " << obj->Name() << " attackable by empire " << attacking_empire_id;
                            empire_infos[attacking_empire_id].target_ids.insert(object_id);
                        }
                    }
                }
            }
        }
    };

    void ShootAllWeapons(TemporaryPtr<UniverseObject>& attacker,
                         const std::vector<PartAttackInfo>& weapons,
                         AutoresolveInfo& combat_state,
                         int bout, int round)
    {
        if (weapons.empty()) {
            if (GetOptionsDB().Get<bool>("verbose-logging"))
                DebugLogger() << "no weapons' can't attack";
            return;   // no ability to attack!
        }

        for (std::vector<PartAttackInfo>::const_iterator weapon_it = weapons.begin();
             weapon_it != weapons.end(); ++weapon_it)
        {
            // select object from valid targets for this object's owner   TODO: with this weapon...
            if (GetOptionsDB().Get<bool>("verbose-logging"))
                DebugLogger() << "Attacking with weapon " << weapon_it->part_type_name << " with power " << weapon_it->part_attack;

            // get valid targets set for attacker owner.  need to do this for
            // each weapon that is attacking, as the previous shot might have
            // destroyed something
            int attacker_owner_id = attacker->Owner();

            std::map<int, EmpireCombatInfo >::iterator target_vec_it = combat_state.empire_infos.find(attacker_owner_id);
            if (target_vec_it == combat_state.empire_infos.end() || !target_vec_it->second.HasTargets()) {
                if (GetOptionsDB().Get<bool>("verbose-logging"))
                    DebugLogger() << "No targets for attacker with id: " << attacker_owner_id;
                break;
            }

            const std::set<int>& valid_target_ids = target_vec_it->second.target_ids;

            // DEBUG
            std::string id_list;
            for (std::set<int>::const_iterator target_it = valid_target_ids.begin();
                 target_it != valid_target_ids.end(); ++target_it)
            { id_list += boost::lexical_cast<std::string>(*target_it) + " "; }

            if (GetOptionsDB().Get<bool>("verbose-logging")) { 
                DebugLogger() << "Valid targets for attacker with id: " << attacker_owner_id
                << " owned by empire: " << attacker_owner_id
                << " :  " << id_list;
            }
            // END DEBUG

            // select target object
            int target_idx = RandInt(0, valid_target_ids.size() - 1);
            if (GetOptionsDB().Get<bool>("verbose-logging"))
                DebugLogger() << " ... target index: " << target_idx << " of " << valid_target_ids.size() - 1;
            std::set<int>::const_iterator target_it = valid_target_ids.begin();
            std::advance(target_it, target_idx);
            assert(target_it != valid_target_ids.end());
            int target_id = *target_it;

            TemporaryPtr<UniverseObject> target = combat_state.combat_info.objects.Object(target_id);
            if (!target) {
                ErrorLogger() << "AutoResolveCombat couldn't get target object with id " << target_id;
                continue;
            }
            if (GetOptionsDB().Get<bool>("verbose-logging"))
                DebugLogger() << "Target: " << target->Name();

            // do actual attacks
            Attack(attacker, *weapon_it, target, combat_state.combat_info, bout, round);

            // mark attackers as valid targets for attacked object's owners, so
            // attacker they can be counter-attacked in subsequent rounds if it
            // was not already attackable
            combat_state.empire_infos[target->Owner()].target_ids.insert(attacker->ID());
        } // end for over weapons
    }

    std::vector<PartAttackInfo> GetWeapons(TemporaryPtr<UniverseObject>& attacker) {
        // loop over weapons of attacking object.  each gets a shot at a
        // randomly selected target object
        std::vector<PartAttackInfo> weapons;

        TemporaryPtr<Ship> attack_ship = boost::dynamic_pointer_cast<Ship>(attacker);
        TemporaryPtr<Planet> attack_planet = boost::dynamic_pointer_cast<Planet>(attacker);

        if (attack_ship) {
            weapons = ShipWeaponsStrengths(attack_ship);
            for (std::vector<PartAttackInfo>::const_iterator part_it = weapons.begin();
                 part_it != weapons.end(); ++part_it)
            {
                if (GetOptionsDB().Get<bool>("verbose-logging")) {
                    DebugLogger() << "weapon: " << part_it->part_type_name
                                           << " attack: " << part_it->part_attack;
                }
            }
        } else if (attack_planet) { // treat planet defenses as short range
            weapons.push_back(PartAttackInfo(PC_SHORT_RANGE, "", attack_planet->CurrentMeterValue(METER_DEFENSE)));
        }
        return weapons;
    }

    void CombatRound(int bout, CombatInfo& combat_info, AutoresolveInfo& combat_state) {
        combat_info.combat_events.push_back(boost::make_shared<BoutBeginEvent>(bout));

        std::vector<int> shuffled_attackers;
        combat_state.GiveAttackersShuffled(shuffled_attackers);

        int round = 1;

        // Planets are processed first so that they still have full power as intended,
        // despite their attack power depending on a thing (defence meter)
        // processing shots at them may reduce.
        for (std::vector<int>::iterator attacker_it = shuffled_attackers.begin();
             attacker_it != shuffled_attackers.end(); ++attacker_it)
        {
            int attacker_id = *attacker_it;

            TemporaryPtr<UniverseObject> attacker = combat_info.objects.Object(attacker_id);

            if (!attacker) {
                ErrorLogger() << "CombatRound couldn't get object with id " << attacker_id;
                return;
            }
            if (attacker->ObjectType() != OBJ_PLANET) {
                continue;
            }
            if (!ObjectCanAttack(attacker)) {
                DebugLogger() << "Planet " << attacker->Name() << " could not attack.";
                continue;
            }
            if (GetOptionsDB().Get<bool>("verbose-logging"))
                DebugLogger() << "Planet: " << attacker->Name();

            std::vector<PartAttackInfo> weapons = GetWeapons(attacker);
            ShootAllWeapons(attacker, weapons, combat_state, bout, round++);
        }

        for (std::vector<int>::iterator attacker_it = shuffled_attackers.begin();
             attacker_it != shuffled_attackers.end(); ++attacker_it)
        {
            int attacker_id = *attacker_it;

            TemporaryPtr<UniverseObject> attacker = combat_info.objects.Object(attacker_id);

            if (!attacker) {
                ErrorLogger() << "CombatRound couldn't get object with id " << attacker_id;
                return;
            }
            if (attacker->ObjectType() == OBJ_PLANET) {
                continue;
            }
            if (!ObjectCanAttack(attacker)) {
                DebugLogger() << "Attacker " << attacker->Name() << " could not attack.";
                continue;
            }
            if (GetOptionsDB().Get<bool>("verbose-logging"))
                DebugLogger() << "Attacker: " << attacker->Name();

            // loop over weapons of the attacking object.  each gets a shot at a
            // randomly selected target object
            std::vector<PartAttackInfo> weapons = GetWeapons(attacker);
            ShootAllWeapons(attacker, weapons, combat_state, bout, round++);
        }

        /// Remove all who died in the bout
        combat_state.CullTheDead(bout);
    }
}

void AutoResolveCombat(CombatInfo& combat_info) {
    if (combat_info.objects.Empty())
        return;

    TemporaryPtr<const System> system = combat_info.objects.Object<System>(combat_info.system_id);
    if (!system)
        ErrorLogger() << "AutoResolveCombat couldn't get system with id " << combat_info.system_id;
    else
        DebugLogger() << "AutoResolveCombat at " << system->Name();

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        DebugLogger() << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%";
        DebugLogger() << "AutoResolveCombat objects before resolution: " << combat_info.objects.Dump();
    }

    // reasonably unpredictable but reproducible random seeding
    const int base_seed = combat_info.objects.begin()->ID() + CurrentTurn();


    // compile list of valid objects to attack or be attacked in this combat
    AutoresolveInfo combat_state(combat_info);


    // Each combat "bout" all attackers attack one target (if any are available)
    const int NUM_COMBAT_BOUTS = 3;

    for (int bout = 1; bout <= NUM_COMBAT_BOUTS; ++bout) {
        Seed(base_seed + bout);    // ensure each combat bout produces different results

        // empires may have valid targets, but nothing to attack with.  If all
        // empires have no attackers or no valid targers, combat is over
        if (!combat_state.CanSomeoneAttackSomething()) {
            if (GetOptionsDB().Get<bool>("verbose-logging"))
                DebugLogger() << "No empire has valid targets and something to attack with; combat over.";
            break;
        }

        if (GetOptionsDB().Get<bool>("verbose-logging"))
            DebugLogger() << "Combat at " << system->Name() << " (" << combat_info.system_id << ") Bout " << bout;

        CombatRound(bout, combat_info, combat_state);
    } // end for over combat arounds

    // ensure every participant knows what happened.
    // TODO: assemble list of objects to copy for each empire.  this should
    //       include objects the empire already knows about with standard
    //       visibility system, and also any objects the empire knows are
    //       destroyed during this combat...
    for (std::map<int, ObjectMap>::iterator it = combat_info.empire_known_objects.begin();
         it != combat_info.empire_known_objects.end(); ++it)
    { it->second.Copy(combat_info.objects); }

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        DebugLogger() << "AutoResolveCombat objects after resolution: " << combat_info.objects.Dump();

        DebugLogger() << "combat event log:";
        for (std::vector<CombatEventPtr>::const_iterator it = combat_info.combat_events.begin();
             it != combat_info.combat_events.end(); ++it)
        { DebugLogger() << (*it)->DebugString(); }
    }
}
