#include "CombatSystem.h"
#include "CombatEvents.h"

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
#include "../universe/Species.h"
#include "../universe/Enums.h"
#include "../universe/Condition.h"
#include "../universe/ValueRef.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/i18n.h"

#include "../network/Message.h"

//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>
#include <boost/format.hpp>

#include <iterator>
#include <sstream>

namespace {
    DeclareThreadSafeLogger(combat);
}

////////////////////////////////////////////////
// CombatInfo
////////////////////////////////////////////////
CombatInfo::CombatInfo(int system_id_, int turn_) :
    turn(turn_),
    system_id(system_id_)
{
    auto system = ::GetSystem(system_id);
    if (!system) {
        ErrorLogger() << "CombatInfo constructed with invalid system id: " << system_id;
        return;
    }
    // add system to objects in combat
    objects.Insert(system);

    // find ships and their owners in system
    auto ships = Objects().FindObjects<Ship>(system->ShipIDs());
    for (auto& ship : ships) {
        // add owner of ships in system to empires that have assets in this battle
        empire_ids.insert(ship->Owner());
        // add ships to objects in combat
        objects.Insert(ship);
    }

    // find planets and their owners in system
    auto planets = Objects().FindObjects<Planet>(system->PlanetIDs());
    for (auto& planet : planets) {
        // if planet is populated or has an owner, add owner to empires that have assets in this battle
        if (!planet->Unowned() || planet->InitialMeterValue(METER_POPULATION) > 0.0f)
            empire_ids.insert(planet->Owner());
        // add planets to objects in combat
        objects.Insert(planet);
    }

    InitializeObjectVisibility();

    // after battle is simulated, any changes to latest known or actual objects
    // will be copied back to the main Universe's ObjectMap and the Universe's
    // empire latest known objects ObjectMap
}

std::shared_ptr<const System> CombatInfo::GetSystem() const
{ return this->objects.Object<System>(this->system_id); }

std::shared_ptr<System> CombatInfo::GetSystem()
{ return this->objects.Object<System>(this->system_id); }

float CombatInfo::GetMonsterDetection() const {
    float monster_detection = 0.0;
    for (auto it = objects.const_begin(); it != objects.const_end(); ++it) {
        auto obj = *it;
        if (obj->Unowned() && (obj->ObjectType() == OBJ_SHIP || obj->ObjectType() == OBJ_PLANET ))
            monster_detection = std::max(monster_detection, obj->InitialMeterValue(METER_DETECTION));
    }
    return monster_detection;
}

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
{ filtered_combat_events = this->combat_events; }

void CombatInfo::GetEmpireObjectVisibilityToSerialize(Universe::EmpireObjectVisibilityMap&
                                                      filtered_empire_object_visibility,
                                                      int encoding_empire) const
{
    filtered_empire_object_visibility = this->empire_object_visibility;
}

namespace {
    // collect detection strengths of all empires (and neutrals) in \a combat_info
    std::map<int, float> GetEmpiresDetectionStrengths(const CombatInfo& combat_info) {
        std::map<int, float> retval;
        for (auto empire_id : combat_info.empire_ids) {
            const auto empire = GetEmpire(empire_id);
            if (!empire)
                continue;
            const Meter* meter = empire->GetMeter("METER_DETECTION_STRENGTH");
            float strength = meter ? meter->Current() : 0.0f;
            retval[empire_id] = strength;
        }
        retval[ALL_EMPIRES] =  combat_info.GetMonsterDetection();
        return retval;
    }
}

void CombatInfo::InitializeObjectVisibility() {
    // initialize combat-local visibility of objects by empires and combat-local
    // empire ObjectMaps with object state info that empires know at start of battle
    auto det_strengths = GetEmpiresDetectionStrengths(*this);

    for (int empire_id : empire_ids) {
        DebugLogger() << "Initializing CombatInfo object visibility and known objects for empire: " << empire_id;

        float empire_detection = det_strengths[empire_id];

        for (auto obj : objects) {

            if (obj->ObjectType() == OBJ_SYSTEM) {
                // systems always visible to empires with objects in them
                empire_object_visibility[empire_id][obj->ID()] = VIS_PARTIAL_VISIBILITY;
                DebugLogger() << "System " << obj->Name() << " always visible";

            } else if (obj->ObjectType() == OBJ_PLANET) {
                // planets always at least basically visible to empires with objects in them
                Visibility vis = VIS_BASIC_VISIBILITY;
                if (empire_id != ALL_EMPIRES) {
                    Visibility vis_univ = GetUniverse().GetObjectVisibilityByEmpire(obj->ID(), empire_id);
                    if (vis_univ > vis) {
                        vis = vis_univ;
                        DebugLogger() << "Planet " << obj->Name() << " visible from universe state";
                    }
                }
                if (vis < VIS_PARTIAL_VISIBILITY && empire_detection >= obj->CurrentMeterValue(METER_STEALTH)) {
                    vis = VIS_PARTIAL_VISIBILITY;
                    DebugLogger() << "Planet " << obj->Name() << " visible empire stealth check: " << empire_detection << " >= " << obj->CurrentMeterValue(METER_STEALTH);
                }
                if (vis == VIS_BASIC_VISIBILITY) {
                    DebugLogger() << "Planet " << obj->Name() << " has just basic visibility by default";
                }

                empire_object_visibility[empire_id][obj->ID()] = vis;

            } else if (obj->ObjectType() == OBJ_SHIP) {
                // ships only visible if detected or they attack later in combat
                Visibility vis = VIS_NO_VISIBILITY;
                if (empire_id != ALL_EMPIRES) {
                    Visibility vis_univ = GetUniverse().GetObjectVisibilityByEmpire(obj->ID(), empire_id);
                    if (vis_univ > vis) {
                        vis = vis_univ;
                        DebugLogger() << "Ship " << obj->Name() << " visible from universe state";
                    }
                }
                if (vis < VIS_PARTIAL_VISIBILITY && empire_detection >= obj->CurrentMeterValue(METER_STEALTH)) {
                    vis = VIS_PARTIAL_VISIBILITY;
                    DebugLogger() << "Ship " << obj->Name() << " visible empire stealth check: " << empire_detection << " >= " << obj->CurrentMeterValue(METER_STEALTH);
                }
                if (vis < VIS_PARTIAL_VISIBILITY) {
                    if (auto ship = std::dynamic_pointer_cast<Ship>(obj)) {
                        if (auto fleet = ::GetFleet(ship->FleetID())) {
                            if (fleet->Aggressive()) {
                                vis = VIS_PARTIAL_VISIBILITY;
                                DebugLogger() << "Ship " << obj->Name() << " visible from aggressive fleet";
                            }
                        }
                    }
                }

                if (vis > VIS_NO_VISIBILITY) {
                    empire_object_visibility[empire_id][obj->ID()] = vis;
                } else {
                    DebugLogger() << "Ship " << obj->Name() << " initially hidden";
                }
            }
        }
    }
}


////////////////////////////////////////////////
// AutoResolveCombat
////////////////////////////////////////////////
namespace {
    // if source is owned by ALL_EMPIRES, match objects owned by an empire
    // if source is owned by an empire, match unowned objects and objects owned by enemies of source's owner empire
    Condition::ConditionBase* VisibleEnemyOfOwnerCondition() {
        return new Condition::Or(
                // unowned candidate object case
                boost::make_unique<Condition::And>(
                    boost::make_unique<Condition::EmpireAffiliation>(AFFIL_NONE),   // unowned candidate object

                    boost::make_unique<Condition::ValueTest>(           // when source object is owned (ie. not the same owner as the candidate object)
                        boost::make_unique<ValueRef::Variable<int>>(
                            ValueRef::SOURCE_REFERENCE, "Owner"),
                        Condition::NOT_EQUAL,
                        boost::make_unique<ValueRef::Variable<int>>(
                            ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE, "Owner")),

                    boost::make_unique<Condition::VisibleToEmpire>(     // when source object's owner empire can detect the candidate object
                        boost::make_unique<ValueRef::Variable<int>>(    // source's owner empire id
                            ValueRef::SOURCE_REFERENCE, "Owner"))),

                // owned candidate object case
                boost::make_unique<Condition::And>(
                    boost::make_unique<Condition::EmpireAffiliation>(AFFIL_ANY),    // candidate is owned by an empire

                    boost::make_unique<Condition::EmpireAffiliation>(               // candidate is owned by enemy of source's owner
                        boost::make_unique<ValueRef::Variable<int>>(
                            ValueRef::SOURCE_REFERENCE, "Owner"), AFFIL_ENEMY),

                    boost::make_unique<Condition::VisibleToEmpire>(     // when source empire can detect the candidate object
                        boost::make_unique<ValueRef::Variable<int>>(    // source's owner empire id
                            ValueRef::SOURCE_REFERENCE, "Owner"))
                ))
            ;
    }

    const std::unique_ptr<Condition::ConditionBase> is_enemy_ship_or_fighter =
        boost::make_unique<Condition::And>(
            boost::make_unique<Condition::Or>(
                boost::make_unique<Condition::And>(
                    boost::make_unique<Condition::Type>(OBJ_SHIP),
                    boost::make_unique<Condition::Not>(
                        boost::make_unique<Condition::MeterValue>(
                            METER_STRUCTURE,
                            nullptr,
                            boost::make_unique<ValueRef::Constant<double>>(0.0)))),
                boost::make_unique<Condition::Type>(OBJ_FIGHTER)),
            std::unique_ptr<Condition::ConditionBase>{VisibleEnemyOfOwnerCondition()});

    const std::unique_ptr<Condition::ConditionBase> is_enemy_ship =
        boost::make_unique<Condition::And>(
            boost::make_unique<Condition::Type>(OBJ_SHIP),

            boost::make_unique<Condition::Not>(
                boost::make_unique<Condition::MeterValue>(
                    METER_STRUCTURE,
                    nullptr,
                    boost::make_unique<ValueRef::Constant<double>>(0.0))),

            std::unique_ptr<Condition::ConditionBase>{VisibleEnemyOfOwnerCondition()});

    const std::unique_ptr<Condition::ConditionBase> is_enemy_ship_fighter_or_armed_planet =
        boost::make_unique<Condition::And>(
            std::unique_ptr<Condition::ConditionBase>{VisibleEnemyOfOwnerCondition()},  // enemies
            boost::make_unique<Condition::Or>(
                boost::make_unique<Condition::Or>(
                    boost::make_unique<Condition::And>(
                        boost::make_unique<Condition::Type>(OBJ_SHIP),
                        boost::make_unique<Condition::Not>(
                            boost::make_unique<Condition::MeterValue>(
                                METER_STRUCTURE,
                                nullptr,
                                boost::make_unique<ValueRef::Constant<double>>(0.0)))),
                    boost::make_unique<Condition::Type>(OBJ_FIGHTER)),

                boost::make_unique<Condition::And>(
                    boost::make_unique<Condition::Type>(OBJ_PLANET),
                    boost::make_unique<Condition::Or>(
                        boost::make_unique<Condition::Not>(
                            boost::make_unique<Condition::MeterValue>(
                                METER_DEFENSE,
                                nullptr,
                                boost::make_unique<ValueRef::Constant<double>>(0.0))),
                        boost::make_unique<Condition::Not>(
                            boost::make_unique<Condition::MeterValue>(
                                METER_SHIELD,
                                nullptr,
                                boost::make_unique<ValueRef::Constant<double>>(0.0)))))));

    const std::unique_ptr<Condition::ConditionBase> if_source_is_planet_then_ships_else_all =
        boost::make_unique<Condition::Or>(
            boost::make_unique<Condition::And>(     // if source is a planet, match ships
                boost::make_unique<Condition::Number>(
                    boost::make_unique<ValueRef::Constant<int>>(1), // minimum objects matching subcondition
                    nullptr,
                    boost::make_unique<Condition::And>(             // subcondition: source is a planet
                        boost::make_unique<Condition::Source>(),
                        boost::make_unique<Condition::Type>(OBJ_PLANET)
                    )
                ),
                boost::make_unique<Condition::Type>(OBJ_SHIP)
            ),

            boost::make_unique<Condition::Number>(  // if source is not a planet, match anything
                nullptr,
                boost::make_unique<ValueRef::Constant<int>>(0),     // maximum objects matching subcondition
                boost::make_unique<Condition::And>(                 // subcondition: source is a planet
                    boost::make_unique<Condition::Source>(),
                    boost::make_unique<Condition::Type>(OBJ_PLANET)
                )
            )
        );

    struct PartAttackInfo {
        PartAttackInfo(ShipPartClass part_class_, const std::string& part_name_,
                       float part_attack_,
                       const ::Condition::ConditionBase* combat_targets_ = nullptr) :
            part_class(part_class_),
            part_type_name(part_name_),
            part_attack(part_attack_),
            combat_targets(combat_targets_)
        {}
        PartAttackInfo(ShipPartClass part_class_, const std::string& part_name_,
                       int fighters_launched_, float fighter_damage_,
                       const std::string& fighter_type_name_,
                       const ::Condition::ConditionBase* combat_targets_ = nullptr) :
            part_class(part_class_),
            part_type_name(part_name_),
            combat_targets(combat_targets_),
            fighters_launched(fighters_launched_),
            fighter_damage(fighter_damage_),
            fighter_type_name(fighter_type_name_)
        {}

        ShipPartClass                       part_class;
        std::string                         part_type_name;
        float                               part_attack = 0.0f;     // for direct damage parts
        const ::Condition::ConditionBase*   combat_targets = nullptr;
        int                                 fighters_launched = 0;  // for fighter bays, input value should be limited by ship available fighters to launch
        float                               fighter_damage = 0.0f;  // for fighter bays, input value should be determined by ship fighter weapon setup
        std::string                         fighter_type_name;
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
                      << "  weapon: " << weapon.part_type_name << " power: " << power
                      << "  target: " << target->Name() << " shield: " << target_shield->Current()
                      << " structure: " << target_structure->Current();

        float damage = std::max(0.0f, power - shield);

        if (damage > 0.0f) {
            target_structure->AddToCurrent(-damage);
            damaged_object_ids.insert(target->ID());
            DebugLogger(combat) << "COMBAT: Ship " << attacker->Name() << " ("
                                << attacker->ID() << ") does " << damage << " damage to Ship "
                                << target->Name() << " (" << target->ID() << ")";
        }

        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.part_type_name,
                               power, shield, damage);

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
                           int bout, int round,
                           WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        float power = weapon.part_attack;

        if (attacker->TotalWeaponsDamage(0.0f, false) > 0.0f) {
            // any damage is enough to kill any fighter
            DebugLogger(combat) << "COMBAT: " << attacker->Name() << " of empire " << attacker->Owner()
                                << " (" << attacker->ID() << ") does " << power
                                << " damage to " << target->Name() << " (" << target->ID() << ")";
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

    void AttackPlanetPlanet(std::shared_ptr<Planet> attacker, const PartAttackInfo& weapon,
                            std::shared_ptr<Planet> target, CombatInfo& combat_info,
                            int bout, int round,
                            WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        if (!attacker || !target) return;

        float power = 0.0f;
        const Meter* attacker_damage = attacker->UniverseObject::GetMeter(METER_DEFENSE);
        if (attacker_damage)
            power = attacker_damage->Current();   // planet "Defense" meter is actually its attack power

        std::set<int>& damaged_object_ids = combat_info.damaged_object_ids;

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

        DebugLogger(combat) << "AttackPlanetPlanet: attacker: " << attacker->Name() << " power: " << power
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
            DebugLogger(combat) << "COMBAT: Planet " << attacker->Name() << " (" << attacker->ID() << ") does "
                                << shield_damage << " shield damage to Planet " << target->Name() << " ("
                                << target->ID() << ")";
        }
        if (defense_damage >= 0) {
            target_defense->AddToCurrent(-defense_damage);
            DebugLogger(combat) << "COMBAT: Planet " << attacker->Name() << " (" << attacker->ID() << ") does "
                                << defense_damage << " defense damage to Planet " << target->Name() << " ("
                                << target->ID() << ")";
        }
        if (construction_damage >= 0) {
            target_construction->AddToCurrent(-construction_damage);
            DebugLogger(combat) << "COMBAT: Planet " << attacker->Name() << " (" << attacker->ID() << ") does "
                                << construction_damage << " instrastructure damage to Planet " << target->Name()
                                << " (" << target->ID() << ")";
        }

        //TODO report the planet damage details more clearly
        float total_damage = shield_damage + defense_damage + construction_damage;
        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.part_type_name,
                               power, 0.0f, total_damage);

        //attacker->SetLastTurnActiveInCombat(CurrentTurn());
        //target->SetLastTurnAttackedByShip(CurrentTurn());
    }

    void AttackPlanetFighter(std::shared_ptr<Planet> attacker, const PartAttackInfo& weapon,
                             std::shared_ptr<Fighter> target, CombatInfo& combat_info,
                             int bout, int round,
                             WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        if (!attacker || !target) return;

        float power = 0.0f;
        const Meter* attacker_damage = attacker->UniverseObject::GetMeter(METER_DEFENSE);
        if (attacker_damage)
            power = attacker_damage->Current();   // planet "Defense" meter is actually its attack power

        if (power > 0.0f) {
            // any damage is enough to destroy any fighter
            DebugLogger(combat) << "COMBAT: " << attacker->Name() << " of empire " << attacker->Owner()
                                << " (" << attacker->ID() << ") does " << power
                                << " damage to " << target->Name() << " (" << target->ID() << ")";
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

        DebugLogger() << "AttackFighterShip: " << attacker->Name() << " of empire " << attacker->Owner()
                      << " power: " << power << "  target: " << target->Name() //<< " shield: " << target_shield->Current()
                      << " structure: " << target_structure->Current();

        float damage = std::max(0.0f, power - shield);

        if (damage > 0.0f) {
            target_structure->AddToCurrent(-damage);
            damaged_object_ids.insert(target->ID());
            DebugLogger(combat) << "COMBAT: " << attacker->Name() << " of empire " << attacker->Owner()
                                << " (" << attacker->ID() << ") does " << damage
                                << " damage to Ship " << target->Name() << " (" << target->ID() << ")";
        }

        float pierced_shield_value(-1.0);
        CombatEventPtr attack_event = std::make_shared<WeaponFireEvent>(
            bout, round, attacker->ID(), target->ID(), weapon.part_type_name,
            std::tie(power, pierced_shield_value, damage),
            attacker->Owner(), target->Owner());
        attacks_event->AddEvent(attack_event);
        target->SetLastTurnActiveInCombat(CurrentTurn());
    }

    void AttackFighterPlanet(std::shared_ptr<Fighter> attacker, const PartAttackInfo& weapon,
                             std::shared_ptr<Planet> target, CombatInfo& combat_info,
                             int bout, int round, AttacksEventPtr& attacks_event)
    {
        if (!attacker || !target) return;

        float power = attacker->Damage();

        std::set<int>& damaged_object_ids = combat_info.damaged_object_ids;

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

        DebugLogger(combat) << "AttackFighterPlanet: attacker: " << attacker->Name() << " power: " << power
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

        float pierced_shield_value(0.0);
        CombatEventPtr attack_event = std::make_shared<WeaponFireEvent>(
            bout, round, attacker->ID(), target->ID(), weapon.part_type_name,
            std::tie(power, pierced_shield_value, total_damage),
            attacker->Owner(), target->Owner());
        attacks_event->AddEvent(attack_event);

        target->SetLastTurnAttackedByShip(CurrentTurn());
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
            DebugLogger(combat) << "COMBAT: " << attacker->Name() << " of empire " << attacker->Owner()
                                << " (" << attacker->ID() << ") does " << damage
                                << " damage to " << target->Name() << " (" << target->ID() << ")";
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
            AttackShipShip(         attack_ship,    weapon, target_ship,    combat_info, bout, round, platform_event);
        } else if (attack_ship && target_planet) {
            AttackShipPlanet(       attack_ship,    weapon, target_planet,  combat_info, bout, round, platform_event);
        } else if (attack_ship && target_fighter) {
            AttackShipFighter(      attack_ship,    weapon, target_fighter, combat_info, bout, round, platform_event);
        } else if (attack_planet && target_ship) {
            AttackPlanetShip(       attack_planet,  weapon, target_ship,    combat_info, bout, round, platform_event);
        } else if (attack_planet && target_planet) {
            AttackPlanetPlanet(     attack_planet,  weapon, target_planet,  combat_info, bout, round, platform_event);
        } else if (attack_planet && target_fighter) {
            AttackPlanetFighter(    attack_planet,  weapon, target_fighter, combat_info, bout, round, platform_event);
        } else if (attack_fighter && target_ship) {
            AttackFighterShip(      attack_fighter, weapon, target_ship,    combat_info, bout, round, attacks_event);
        } else if (attack_fighter && target_planet) {
            AttackFighterPlanet(    attack_fighter, weapon, target_planet,  combat_info, bout, round, attacks_event);
        } else if (attack_fighter && target_fighter) {
            AttackFighterFighter(   attack_fighter, weapon, target_fighter, combat_info, bout, round, fighter_on_fighter_event);
        }
    }

    bool ObjectCanAttack(std::shared_ptr<const UniverseObject> obj) {
        if (auto ship = std::dynamic_pointer_cast<const Ship>(obj)) {
            return ship->IsArmed() || ship->HasFighters();
        } else if (auto planet = std::dynamic_pointer_cast<const Planet>(obj)) {
            return planet->CurrentMeterValue(METER_DEFENSE) > 0.0f;
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
        std::string fighter_name = UserString("OBJ_FIGHTER");
        std::map<std::string, int> part_fighter_launch_capacities;
        const ::Condition::ConditionBase* part_combat_targets = nullptr;

        // determine what ship does during combat, based on parts and their meters...
        for (const auto& part_name : design->Parts()) {
            const PartType* part = GetPartType(part_name);
            if (!part)
                continue;
            ShipPartClass part_class = part->Class();
            part_combat_targets = part->CombatTargets();

            // direct weapon and fighter-related parts all handled differently...
            if (part_class == PC_DIRECT_WEAPON) {
                float part_attack = ship->CurrentPartMeterValue(METER_CAPACITY, part_name);
                int shots = static_cast<int>(ship->CurrentPartMeterValue(METER_SECONDARY_STAT, part_name)); // secondary stat is shots per attack)
                if (part_attack > 0.0f && shots > 0) {
                    if (!part_combat_targets)
                        part_combat_targets = is_enemy_ship_fighter_or_armed_planet.get();

                    // attack for each shot...
                    for (int shot_count = 0; shot_count < shots; ++shot_count)
                        retval.push_back(PartAttackInfo(part_class, part_name, part_attack,
                                                        part_combat_targets));
                } else {
                    TraceLogger(combat) << "ShipWeaponsStrengths for ship " << ship->Name() << " (" << ship->ID() << ") "
                                        << " direct weapon part " << part->Name() << " has no shots / zero attack, so is skipped";
                }

            } else if (part_class == PC_FIGHTER_HANGAR) {
                // hangar max-capacity-modification effects stack, so only add capacity for each hangar type once
                if (!seen_hangar_part_types.count(part_name)) {
                    available_fighters += ship->CurrentPartMeterValue(METER_CAPACITY, part_name);
                    seen_hangar_part_types.insert(part_name);

                    if (!part_combat_targets)
                        part_combat_targets = is_enemy_ship_or_fighter.get();
                    TraceLogger(combat) << "ShipWeaponsStrengths for ship " << ship->Name() << " (" << ship->ID() << ") "
                                        << "when launching fighters, part " << part->Name() << " with targeting condition: "
                                        << part_combat_targets->Dump();

                    if (UserStringExists(part->Name() + "_FIGHTER"))
                        fighter_name = UserString(part->Name() + "_FIGHTER");

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

                TraceLogger(combat) << "ShipWeaponsStrengths " << ship->Name() << " can launch " << to_launch
                                    << " fighters named \"" << fighter_name << "\" from bay part " << launch.first
                                    << " ... with targeting condition: " << part_combat_targets->Dump();

                if (to_launch <= 0)
                    continue;
                retval.push_back(PartAttackInfo(PC_FIGHTER_BAY, launch.first, to_launch,
                                                fighter_attack, fighter_name,
                                                part_combat_targets)); // attack may be 0; that's ok: decoys
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

    /// A collection of information the autoresolution must keep around
    struct AutoresolveInfo {
        std::set<int>                   valid_attacker_object_ids;  // all objects that can attack
        std::map<int, EmpireCombatInfo> empire_infos;               // empire specific information, indexed by empire id
        CombatInfo&                     combat_info;
        int                             next_fighter_id = -10001;   // give fighters negative ids so as to avoid clashes with any positive-id of persistent UniverseObjects
        std::set<int>                   destroyed_object_ids;       // objects that have been destroyed so far during this combat

        explicit AutoresolveInfo(CombatInfo& combat_info_) :
            combat_info(combat_info_)
        {
            PopulateAttackers();
        }

        std::vector<int> AddFighters(int number, float damage, int owner_empire_id,
                                     int from_ship_id, const std::string& species,
                                     const std::string& fighter_name,
                                     const Condition::ConditionBase* combat_targets)
        {
            std::vector<int> retval;

            if (combat_targets)
                TraceLogger(combat) << "Adding " << number << " fighters for empire " << owner_empire_id
                                    << " with targetting condition: " << combat_targets->Dump();
            else
                TraceLogger(combat) << "Adding " << number << " fighters for empire " << owner_empire_id
                                    << " with no targetting condition";

            for (int n = 0; n < number; ++n) {
                // create / insert fighter into combat objectmap
                auto fighter_ptr = std::make_shared<Fighter>(owner_empire_id, from_ship_id,
                                                             species, damage, combat_targets);
                fighter_ptr->SetID(next_fighter_id--);
                fighter_ptr->Rename(fighter_name);
                combat_info.objects.Insert(fighter_ptr);
                if (!fighter_ptr) {
                    ErrorLogger(combat) << "AddFighters unable to create and insert new Fighter object...";
                    break;
                }

                retval.push_back(fighter_ptr->ID());

                // add fighter to attackers (if it can attack)
                if (damage > 0.0f) {
                    valid_attacker_object_ids.insert(fighter_ptr->ID());
                    empire_infos[fighter_ptr->Owner()].attacker_ids.insert(fighter_ptr->ID());
                    DebugLogger(combat) << "Added fighter id: " << fighter_ptr->ID() << " to attackers sets";
                }

                // mark fighter visible to all empire participants
                for (auto viewing_empire_id : combat_info.empire_ids)
                    combat_info.empire_object_visibility[viewing_empire_id][fighter_ptr->ID()] = VIS_PARTIAL_VISIBILITY;
            }

            return retval;
        }

        // Return true if some empire has ships or fighters that can attack
        // Doesn't consider diplomacy or other empires, as parts have arbitrary
        // targeting conditions, including friendly fire
        bool CanSomeoneAttackSomething() const {
            for (const auto& attacker_info : empire_infos) {
                // does empire have something to attack with?
                const EmpireCombatInfo& attacker_empire_info = attacker_info.second;
                if (!attacker_empire_info.HasAttackers() && !attacker_empire_info.HasUnlauchedArmedFighters(combat_info))
                    continue;

                // TODO: check if any of these ships or fighters have targeting
                // conditions that match anything present in this combat
                return true;
            }
            return false;
        }

        /// Removes dead units from lists of attackers and defenders
        void CullTheDead(int bout, BoutEvent::BoutEventPtr& bout_event) {
            auto fighters_destroyed_event = std::make_shared<FightersDestroyedEvent>(bout);
            bool at_least_one_fighter_destroyed = false;

            IncapacitationsEventPtr incaps_event = std::make_shared<IncapacitationsEvent>();

            std::vector<int> delete_list;
            delete_list.reserve(combat_info.objects.NumObjects());

            for (auto it = combat_info.objects.const_begin();
                 it != combat_info.objects.const_end(); ++it)
            {
                auto obj = *it;

                // Check if object is already noted as destroyed; don't need to re-record this
                if (destroyed_object_ids.count(obj->ID()))
                    continue;
                // Check if object is destroyed and update lists if yes
                if (!CheckDestruction(obj))
                    continue;
                destroyed_object_ids.insert(obj->ID());

                if (obj->ObjectType() == OBJ_FIGHTER) {
                    fighters_destroyed_event->AddEvent(obj->Owner());
                    at_least_one_fighter_destroyed = true;
                    // delete actual fighter object so that it can't be targeted
                    // again next round (ships have a minimal structure test instead)
                    delete_list.push_back(obj->ID());
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

            std::stringstream ss;
            for (auto id : delete_list) {
                ss << id << " ";
                combat_info.objects.Remove(id);
            }
            DebugLogger() << "Removed destroyed objects from combat state with ids: " << ss.str();
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
                    DebugLogger(combat) << "Removed destroyed fighter id: " << fighter->ID() << " from attackers";

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
                        DebugLogger(combat) << "!! Planet " << target_id << " has not yet been attacked, "
                                            << "so will not yet be removed from battle, despite being essentially incapacitated";
                        return false;
                    }
                    DebugLogger(combat) << "!! Target Planet " << target_id << " is entirely knocked out of battle";

                    // Remove target from its empire's list of attackers
                    empire_infos[target->Owner()].attacker_ids.erase(target_id);

                    CleanEmpires();
                    return true;
                }
            }

            return false;
        }

        /// check if any empire has no remaining objects.
        /// If so, remove that empire's entry
        void CleanEmpires() {
            DebugLogger(combat) << "CleanEmpires";
            auto temp = empire_infos;

            std::set<int> empire_ids_with_objects;
            for (const auto obj : combat_info.objects)
                empire_ids_with_objects.insert(obj->Owner());

            for (auto& empire : empire_infos) {
                if (!empire_ids_with_objects.count(empire.first)) {
                    temp.erase(empire.first);
                    DebugLogger(combat) << "No objects left for empire with id: " << empire.first;
                }
            }
            empire_infos = std::move(temp);

            if (!empire_infos.empty()) {
                DebugLogger(combat) << "Empires with objects remaining:";
                for (auto empire : empire_infos) {
                    DebugLogger(combat) << " ... " << empire.first;
                    for (auto obj_id : empire.second.attacker_ids) {
                        TraceLogger(combat) << " ... ... " << obj_id;
                    }
                }
            }
        }

        /// Clears and refills \a shuffled with attacker ids in a random order
        void GetShuffledValidAttackerIDs(std::vector<int>& shuffled) {
            shuffled.clear();
            shuffled.insert(shuffled.begin(), valid_attacker_object_ids.begin(), valid_attacker_object_ids.end());

            const unsigned swaps = shuffled.size();
            for (unsigned i = 0; i < swaps; ++i) {
                int pos2 = RandInt(i, swaps - 1);
                std::swap(shuffled[i], shuffled[pos2]);
            }
        }

        float EmpireDetectionStrength(int empire_id) {
            if (const auto* empire = GetEmpire(empire_id))
                return empire->GetMeter("METER_DETECTION_STRENGTH")->Current();
            return 0.0f;
        }

        /** Report for each empire the stealthy objects in the combat. */
        InitialStealthEvent::EmpireToObjectVisibilityMap ReportInvisibleObjects() const {
            DebugLogger(combat) << "Reporting Invisible Objects";
            InitialStealthEvent::EmpireToObjectVisibilityMap report;

            // loop over all objects, noting which is visible by which empire or neutrals
            for (const auto target : combat_info.objects) {
                // for all empires, can they detect this object?
                for (int viewing_empire_id : combat_info.empire_ids) {
                    // get visibility of target to attacker empire
                    auto empire_vis_info_it = combat_info.empire_object_visibility.find(viewing_empire_id);
                    if (empire_vis_info_it == combat_info.empire_object_visibility.end()) {
                        DebugLogger() << " ReportInvisibleObjects found no visibility info for viewing empire " << viewing_empire_id;
                        report[viewing_empire_id][target->ID()] = VIS_NO_VISIBILITY;
                        continue;
                    }
                    auto target_visibility_for_empire_it = empire_vis_info_it->second.find(target->ID());
                    if (target_visibility_for_empire_it == empire_vis_info_it->second.end()) {
                        DebugLogger() << " ReportInvisibleObjects found no visibility record for viewing empire "
                                      << viewing_empire_id << " for object " << target->Name() << " (" << target->ID() << ")";
                        report[viewing_empire_id][target->ID()] = VIS_NO_VISIBILITY;
                        continue;
                    }

                    Visibility vis = target_visibility_for_empire_it->second;
                    if (viewing_empire_id == ALL_EMPIRES) {
                        DebugLogger(combat) << " Target " << target->Name() << " (" << target->ID() << "): "
                                            << vis << " to monsters and neutrals";
                    } else {
                        DebugLogger(combat) << " Target " << target->Name() << " (" << target->ID() << "): "
                                            << vis << " to empire " << viewing_empire_id;
                    }

                    // This adds information about invisible and basic visible objects and
                    // trusts that the combat logger only informs player/ai of what they should know
                    report[viewing_empire_id][target->ID()] = vis;
                }
            }
            return report;
        }

    private:
        typedef std::set<int>::const_iterator const_id_iterator;

        // Populate lists of things that can attack. List attackers also by empire.
        void PopulateAttackers() {
            for (auto it = combat_info.objects.const_begin();
                 it != combat_info.objects.const_end(); ++it)
            {
                auto obj = *it;
                bool can_attack{ObjectCanAttack(obj)};
                if (can_attack) {
                    valid_attacker_object_ids.insert(it->ID());
                    empire_infos[obj->Owner()].attacker_ids.insert(it->ID());
                }

                DebugLogger(combat) << "Considering object " << obj->Name() << " (" << obj->ID() << ")"
                                    << " owned by " << std::to_string(obj->Owner())
                                    << (can_attack ? "... can attack" : "");
            }
        }
    };

    std::vector<PartAttackInfo> GetWeapons(std::shared_ptr<UniverseObject>& attacker) {
        // Loop over weapons of attacking object. Each gets a shot at a
        // randomly selected target object, from the objects in the combat
        // that match the weapon's targetting condition.
        std::vector<PartAttackInfo> weapons;

        auto attack_ship = std::dynamic_pointer_cast<Ship>(attacker);
        auto attack_planet = std::dynamic_pointer_cast<Planet>(attacker);
        auto attack_fighter = std::dynamic_pointer_cast<Fighter>(attacker);

        if (attack_ship) {
            weapons = ShipWeaponsStrengths(attack_ship);    // includes info about fighter launches with PC_FIGHTER_BAY part class, and direct fire weapons with PC_DIRECT_WEAPON part class
            for (PartAttackInfo& part : weapons) {
                if (part.part_class == PC_DIRECT_WEAPON) {
                    DebugLogger(combat) << "Attacker Ship has direct weapon: " << part.part_type_name
                                        << " attack: " << part.part_attack;
                } else if (part.part_class == PC_FIGHTER_BAY) {
                    DebugLogger(combat) << "Attacker Ship can fighter launch: " << part.fighters_launched
                                        << " damage: " << part.fighter_damage;
                    if (part.combat_targets)
                        TraceLogger(combat) << " ... fighter targeting condition: " << part.combat_targets->Dump();
                    else
                        TraceLogger(combat) << " ... fighter has no targeting condition";
                }
            }

        } else if (attack_planet) {     // treat planet defenses as direct fire weapon that only target ships
            weapons.push_back(PartAttackInfo(PC_DIRECT_WEAPON, UserStringNop("DEF_DEFENSE"),
                                             attack_planet->CurrentMeterValue(METER_DEFENSE),
                                             is_enemy_ship.get()));

        } else if (attack_fighter) {    // treat fighter damage as direct fire weapon
            weapons.push_back(PartAttackInfo(PC_DIRECT_WEAPON, UserStringNop("FT_WEAPON_1"),
                                             attack_fighter->Damage(),
                                             attack_fighter->CombatTargets()));
        }
        return weapons;
    }

    const Condition::ConditionBase* SpeciesTargettingCondition(const std::shared_ptr<UniverseObject>& attacker) {
        if (!attacker)
            return if_source_is_planet_then_ships_else_all.get();

        const Species* species = nullptr;
        if (auto attack_ship = std::dynamic_pointer_cast<Ship>(attacker))
            species = GetSpecies(attack_ship->SpeciesName());
        else if (auto attack_planet = std::dynamic_pointer_cast<Planet>(attacker))
            species = GetSpecies(attack_planet->SpeciesName());
        else if (auto attack_fighter = std::dynamic_pointer_cast<Fighter>(attacker))
            species = GetSpecies(attack_fighter->SpeciesName());

        if (!species || !species->CombatTargets())
            return if_source_is_planet_then_ships_else_all.get();

        return species->CombatTargets();
    }

    void AddAllObjectsSet(ObjectMap& obj_map, Condition::ObjectSet& condition_non_targets) {
        condition_non_targets.reserve(condition_non_targets.size() + obj_map.ExistingObjects().size());
        std::transform(obj_map.ExistingObjects().begin(), obj_map.ExistingObjects().end(),  // ExistingObjects() here does not consider whether objects have been destroyed during this combat
                       std::back_inserter(condition_non_targets),
                       boost::bind(&std::map<int, std::shared_ptr<UniverseObject>>::value_type::second, _1));
    }

    void ShootAllWeapons(std::shared_ptr<UniverseObject>& attacker,
                         AutoresolveInfo& combat_state, int bout, int round,
                         AttacksEventPtr& attacks_event,
                         WeaponsPlatformEvent::WeaponsPlatformEventPtr& platform_event,
                         std::shared_ptr<FightersAttackFightersEvent>& fighter_on_fighter_event)
    {
        auto weapons = GetWeapons(attacker);
        if (weapons.empty()) {
            DebugLogger(combat) << "Attacker " << attacker->Name() << " ("
                                << attacker->ID() << ") has no weapons, so can't attack";
            return;   // no ability to attack!
        }

        const auto* species_targetting_condition = SpeciesTargettingCondition(attacker);
        if (!species_targetting_condition) {
            ErrorLogger(combat) << "Null Species Targetting Condition...!?";
            return;
        }
        TraceLogger(combat) << "Species targeting condition: " << species_targetting_condition->Dump();

        for (const PartAttackInfo& weapon : weapons) {
            // skip non-direct-fire weapons (as only direct fire weapons can "shoot").
            // fighter launches handled separately
            if (weapon.part_class != PC_DIRECT_WEAPON)
                continue;

            // select object from valid targets for this object's owner
            DebugLogger(combat) << "Attacker " << attacker->Name() << " ("
                                << attacker->ID() << ") attacks with weapon "
                                << weapon.part_type_name << " with power " << weapon.part_attack;

            if (!weapon.combat_targets) {
                DebugLogger(combat) << "Weapon has no targeting condition?? Should have been set when initializing PartAttackInfo";
                continue;
            }
            TraceLogger(combat) << "Weapon targeting condition: " << weapon.combat_targets->Dump();


            Condition::ObjectSet targets, rejected_targets;
            AddAllObjectsSet(combat_state.combat_info.objects, targets);

            // attacker is source object for condition evaluation. use combat-specific vis info.
            ScriptingContext context(attacker, combat_state.combat_info.empire_object_visibility);

            // apply species targeting condition and then weapon targeting condition
            species_targetting_condition->Eval(context, targets, rejected_targets, Condition::MATCHES);
            weapon.combat_targets->Eval(context, targets, rejected_targets, Condition::MATCHES);


            if (targets.empty()) {
                DebugLogger(combat) << "No objects matched targeting condition!";
                continue;
            }
            DebugLogger(combat) << targets.size() << " objects matched targeting condition";
            for (const auto& match : targets)
                TraceLogger(combat) << " ... " << match->Name() << " (" << match->ID() << ")";

            // select target object from matches
            int target_idx = RandInt(0, targets.size() - 1);
            auto target = *std::next(targets.begin(), target_idx);

            if (!target) {
                ErrorLogger(combat) << "AutoResolveCombat selected null target object?";
                continue;
            }
            DebugLogger(combat) << "Selected target: " << target->Name() << " (" << target->ID() << ")";
            auto targetx = std::const_pointer_cast<UniverseObject>(target);

            // do actual attacks
            Attack(attacker, weapon, targetx, combat_state.combat_info, bout, round,
                   attacks_event, platform_event, fighter_on_fighter_event);

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
            ErrorLogger(combat) << "ReduceStoredFighterCount couldn't get ship design with id " << ship->DesignID();;
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
                        FighterLaunchesEventPtr& launches_event)
    {
        if (weapons.empty()) {
            DebugLogger(combat) << "no weapons' can't launch figters!";
            return;   // no ability to attack!
        }

        std::string species_name;
        auto attacker_ship = std::dynamic_pointer_cast<Ship>(attacker);
        if (attacker_ship)
            species_name = attacker_ship->SpeciesName();
        else if (auto attacker_planet = std::dynamic_pointer_cast<Planet>(attacker))
            species_name = attacker_planet->SpeciesName();

        for (const auto& weapon : weapons) {
            // skip non-fighter weapons
            // direct fire weapons handled separately
            if (weapon.part_class != PC_FIGHTER_BAY)    // passed in weapons container should have already checked for adequate supply of fighters to launch and the available bays, and condensed into a single entry...
                continue;

            int attacker_owner_id = attacker->Owner();
            const auto empire = GetEmpire(attacker_owner_id);
            const auto& empire_name = (empire ? empire->Name() : UserString("ENC_COMBAT_ROGUE"));

            std::string fighter_name = boost::io::str(FlexibleFormat(UserString("ENC_COMBAT_EMPIRE_FIGHTER_NAME"))
                % empire_name % species_name % weapon.fighter_type_name);

            DebugLogger(combat) << "Launching " << weapon.fighters_launched
                                << " with damage " << weapon.fighter_damage
                                << " for empire id: " << attacker_owner_id
                                << " from ship id: " << attacker->ID();
            if (weapon.combat_targets)
                TraceLogger(combat) << " ... with targeting condition: " << weapon.combat_targets->Dump();
            else
                TraceLogger(combat) << " ... with no targeting condition!";

            std::vector<int> new_fighter_ids =
                combat_state.AddFighters(weapon.fighters_launched, weapon.fighter_damage,
                                         attacker_owner_id, attacker->ID(), species_name,
                                         fighter_name, weapon.combat_targets);

            // combat event
            CombatEventPtr launch_event = std::make_shared<FighterLaunchEvent>(
                bout, attacker->ID(), attacker_owner_id, new_fighter_ids.size());
            launches_event->AddEvent(launch_event);


            // reduce hangar capacity (contents) corresponding to launched fighters
            int num_launched = new_fighter_ids.size();
            if (attacker_ship)
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
            ErrorLogger(combat) << "IncreaseStoredFighterCount couldn't get ship design with id " << ship->DesignID();;
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
        // recovered. ioesn't matter which part's capacity meters are increased,
        // since all fighters are the same on the ship
        for (auto& part : part_type_fighter_hangar_capacities) {
            if (!part.second.first || !part.second.second)
                continue;
            float space = part.second.second->Current() - part.second.first->Current();
            float increase = std::min(space, recovered_fighters);
            recovered_fighters -= increase;

            DebugLogger() << "Increasing stored fighter count of ship " << ship->Name()
                          << " (" << ship->ID() << ") from " << part.second.first->Current()
                          << " by " << increase << " towards max of "
                          << part.second.second->Current();

            part.second.first->AddToCurrent(increase);

            // stop if all fighters recovered
            if (recovered_fighters <= 0.0f)
                break;
        }
    }

    void RecoverFighters(CombatInfo& combat_info, int bout,
                         FighterLaunchesEventPtr& launches_event)
    {
        std::map<int, float> ships_fighters_to_add_back;
        DebugLogger() << "Recovering fighters at end of combat...";

        // count still-existing and not destroyed fighters at end of combat
        for (auto obj_it = combat_info.objects.begin();
             obj_it != combat_info.objects.end() && obj_it->ID() < 0; ++obj_it)
        {
            auto fighter = std::dynamic_pointer_cast<Fighter>(*obj_it);
            if (!fighter || fighter->Destroyed())
                continue;   // destroyed fighters can't return
            if (combat_info.destroyed_object_ids.count(fighter->LaunchedFrom())) {
                DebugLogger() << " ... Fighter " << fighter->Name() << " (" << fighter->ID()
                              << ") is from destroyed ship id" << fighter->LaunchedFrom()
                              << " so can't be recovered";
                continue;   // can't return to a destroyed ship
            }
            ships_fighters_to_add_back[fighter->LaunchedFrom()]++;
        }
        DebugLogger() << "Fighters left at end of combat:";
        for (auto ship_fighter_count_pair : ships_fighters_to_add_back)
            DebugLogger() << " ... from ship id " << ship_fighter_count_pair.first << " : " << ship_fighter_count_pair.second;


        DebugLogger() << "Returning fighters to ships:";
        for (auto& entry : ships_fighters_to_add_back) {
            auto ship = combat_info.objects.Object<Ship>(entry.first);
            if (!ship) {
                ErrorLogger(combat) << "Couldn't get ship with id " << entry.first << " for fighter to return to...";
                continue;
            }
            IncreaseStoredFighterCount(ship, entry.second);
            // launching negative ships indicates recovery of them
            CombatEventPtr launch_event = std::make_shared<FighterLaunchEvent>(bout, entry.first, ship->Owner(), -entry.second);
            launches_event->AddEvent(launch_event);
        }
    }

    void CombatRound(int bout, AutoresolveInfo& combat_state) {
        CombatInfo& combat_info = combat_state.combat_info;

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


        // Process planets attacks first so that they still have full power,
        // despite their attack power depending on something (their defence meter)
        // that processing shots at them may reduce.
        for (int attacker_id : shuffled_attackers) {
            auto attacker = combat_info.objects.Object(attacker_id);

            if (!attacker) {
                ErrorLogger() << "CombatRound couldn't get object with id " << attacker_id;
                continue;
            }
            if (attacker->ObjectType() != OBJ_PLANET)
                continue;   // fighter and ship attacks processed below
            if (!ObjectCanAttack(attacker)) {
                DebugLogger() << "Planet " << attacker->Name() << " could not attack.";
                continue;
            }
            DebugLogger(combat) << "Planet: " << attacker->Name();

            auto platform_event = std::make_shared<WeaponsPlatformEvent>(
                bout, attacker_id, attacker->Owner());
            attacks_event->AddEvent(platform_event);

            ShootAllWeapons(attacker, combat_state, bout, round++,
                            attacks_event, platform_event, fighter_on_fighter_event);
        }


        // Process ship and fighter attacks
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

            auto platform_event = std::make_shared<WeaponsPlatformEvent>(
                bout, attacker_id, attacker->Owner());

            ShootAllWeapons(attacker, combat_state, bout, round++,
                            attacks_event, platform_event, fighter_on_fighter_event);

            if (!platform_event->AreSubEventsEmpty(attacker->Owner()))
                attacks_event->AddEvent(platform_event);
        }


        // Launch fighters (which can attack in any subsequent combat bouts).
        // There is no point to launching fighters during the last bout, since
        // they won't get any chance to attack during this combat
        if (bout < GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS")) {
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


        // Create weapon fire events and mark attackers as visible to other battle participants
        auto attacks_this_bout = attacks_event->SubEvents(ALL_EMPIRES);
        auto stealth_change_event = std::make_shared<StealthChangeEvent>(bout);
        for (auto this_event : attacks_this_bout) {
            // Generate attack events
            std::vector<std::shared_ptr<const WeaponFireEvent>> weapon_fire_events;
            if (auto naked_fire_event = std::dynamic_pointer_cast<const WeaponFireEvent>(this_event)) {
                weapon_fire_events.push_back(naked_fire_event);

            } else if (auto weapons_platform = std::dynamic_pointer_cast<const WeaponsPlatformEvent>(this_event)) {
                for (auto more_event : weapons_platform->SubEvents(ALL_EMPIRES)) {
                    if (auto this_attack = std::dynamic_pointer_cast<const WeaponFireEvent>(more_event))
                        weapon_fire_events.push_back(this_attack);
                }
            }

            // Set attacker as at least basically visible to other empires.
            for (auto this_attack : weapon_fire_events) {
                for (auto detector_empire_id : combat_info.empire_ids) {
                    Visibility initial_vis = combat_info.empire_object_visibility[detector_empire_id][this_attack->attacker_id];
                    TraceLogger(combat) << "Pre-attack visibility of attacker id: " << this_attack->attacker_id
                                        << " by empire: " << detector_empire_id << " was: " << initial_vis;

                    if (initial_vis >= VIS_BASIC_VISIBILITY)
                        continue;

                    combat_info.empire_object_visibility[detector_empire_id][this_attack->attacker_id] = VIS_BASIC_VISIBILITY;

                    DebugLogger(combat) << " ... Setting post-attack visability to " << VIS_BASIC_VISIBILITY;

                    // record visibility change event due to attack
                    stealth_change_event->AddEvent(this_attack->attacker_id,
                                                   this_attack->target_id,
                                                   this_attack->attacker_owner_id,
                                                   this_attack->target_owner_id,
                                                   VIS_BASIC_VISIBILITY);
                }
            }
        }

        if (!stealth_change_event->AreSubEventsEmpty(ALL_EMPIRES))
            combat_info.combat_events.push_back(stealth_change_event);

        /// Remove all who died in the bout
        combat_state.CullTheDead(bout, bout_event);

        // Backpropagate meters so that next round tests can use the results of the previous round
        for (auto obj : combat_info.objects)
            obj->BackPropagateMeters();
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
        // TODO: salt further with galaxy setup seed
        base_seed = combat_info.objects.begin()->ID() + CurrentTurn();
    }

    // compile list of valid objects to attack or be attacked in this combat
    AutoresolveInfo combat_state(combat_info);

    combat_info.combat_events.push_back(
        std::make_shared<InitialStealthEvent>(
            combat_state.ReportInvisibleObjects()));

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

        DebugLogger(combat) << "Combat at " << system->Name() << " ("
                            << combat_info.system_id << ") Bout " << bout;

        CombatRound(bout, combat_state);
        last_bout = bout;
    } // end for over combat arounds

    auto launches_event = std::make_shared<FighterLaunchesEvent>();
    combat_info.combat_events.push_back(launches_event);

    RecoverFighters(combat_info, last_bout, launches_event);

    DebugLogger(combat) << "AutoResolveCombat objects after resolution: " << combat_info.objects.Dump();

    DebugLogger(combat) << "combat event log start:";
    for (auto event : combat_info.combat_events)
        DebugLogger(combat) << event->DebugString();
    DebugLogger(combat) << "combat event log end:";
}
