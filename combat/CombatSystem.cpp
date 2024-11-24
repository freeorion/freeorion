#include "CombatSystem.h"
#include "CombatEvents.h"

#include "../universe/Universe.h"
#include "../util/GameRules.h"
#include "../util/OptionsDB.h"
#include "../universe/ConstantsFwd.h"
#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Fighter.h"
#include "../universe/ShipDesign.h"
#include "../universe/ShipPart.h"
#include "../universe/System.h"
#include "../universe/Species.h"
#include "../universe/Conditions.h"
#include "../universe/ValueRefs.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"
#include "../util/Random.h"
#include "../util/i18n.h"

#include "../network/Message.h"

#include <boost/format.hpp>

#include <iterator>
#include <sstream>

namespace {
    DeclareThreadSafeLogger(combat);

#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr const std::string EMPTY_STRING;
#else
    const std::string EMPTY_STRING;
#endif
}

namespace {
    // workaround for eg. boost::unordered_set not having a contains method before Boost 1.79
    inline bool contains(const auto& container, const auto& key) {
        if constexpr (requires { container.contains(key); })
            return container.contains(key);
        else
            return container.find(key) != container.end();
    }
}

////////////////////////////////////////////////
// CombatInfo
////////////////////////////////////////////////
CombatInfo::CombatInfo(int system_id_, int turn_,
                       Universe& universe_mutable_in,
                       EmpireManager& empires_,
                       const DiploStatusMap& diplo_statuses_,
                       const GalaxySetupData& galaxy_setup_data_,
                       SpeciesManager& species_,
                       const SupplyManager& supply_) :
    universe(universe_mutable_in),
    empires(empires_),
    empire_object_vis_turns(universe_mutable_in.GetEmpireObjectVisibilityTurnMap()),
    diplo_statuses(diplo_statuses_),
    galaxy_setup_data(galaxy_setup_data_),
    species(species_),
    supply(supply_),
    empire_object_visibility(universe_mutable_in.GetEmpireObjectVisibility()),
    turn(turn_),
    system_id(system_id_)
{
    auto system = universe_mutable_in.Objects().get<System>(system_id);
    if (!system) {
        ErrorLogger() << "CombatInfo constructed with invalid system id: " << system_id;
        return;
    }
    auto ships = universe_mutable_in.Objects().find<Ship>(system->ShipIDs());
    auto planets = universe_mutable_in.Objects().find<Planet>(system->PlanetIDs());
    auto fleets = universe_mutable_in.Objects().find<Fleet>(system->FleetIDs());

    const auto is_destroyed = [this](int id) { return contains(destroyed_object_ids, id); };

    // add system to objects in combat
    objects.insert(std::move(system), is_destroyed(system_id));

    // find ships and their owners in system
    for (auto& ship : ships) {
        // add owner of ships in system to empires that have assets in this battle
        empire_ids.insert(ship->Owner());
        // add ships to objects in combat
        const int ship_id = ship->ID();
        objects.insert(std::move(ship), is_destroyed(ship_id));
    }

    // find fleets and their owners in system
    for (auto& fleet : fleets) {
        // add owner of fleets in system to empires that have assets in this battle
        empire_ids.insert(fleet->Owner());
        // add fleets to objects in combat
        const int fleet_id = fleet->ID();
        objects.insert(std::move(fleet), is_destroyed(fleet_id));
    }

    // find planets and their owners in system
    for (auto& planet : planets) {
        // if planet is populated or has an owner, add owner to empires that have assets in this battle
        if (!planet->Unowned() || planet->GetMeter(MeterType::METER_POPULATION)->Initial() > 0.0f)
            empire_ids.insert(planet->Owner());
        // add planets to objects in combat
        const int planet_id = planet->ID();
        objects.insert(std::move(planet), is_destroyed(planet_id));
    }

    InitializeObjectVisibility();

    // after battle is simulated, any changes object visibility will be copied
    // to the main gamestate object visibility.
}

std::shared_ptr<const System> CombatInfo::GetSystem() const
{ return this->objects.get<System>(this->system_id); }

std::shared_ptr<System> CombatInfo::GetSystem()
{ return this->objects.get<System>(this->system_id); }

std::shared_ptr<const Empire> CombatInfo::GetEmpire(int id) const
{ return empires.GetEmpire(id); }

std::shared_ptr<Empire> CombatInfo::GetEmpire(int id)
{ return empires.GetEmpire(id); }

namespace {
    constexpr auto not_null = [](const auto* o) noexcept -> bool { return !!o; };
}

void CombatInfo::InitializeObjectVisibility() {
    // visibility in this empire_object_visibility in this CombatInfo should
    // have been initially set in the constructor from the passed-in Universe
    // visibility state.
    // this function adjusts those values

    for (int empire_id : empire_ids) {
        DebugLogger(combat) << "Tweaking CombatInfo object visibility and known objects for empire: " << empire_id;

        auto& empire_vis{empire_object_visibility[empire_id]};

        for (const System* obj : objects.allRaw<System>() | range_filter(not_null)) {
            // systems always visible to empires with objects in them
            empire_vis[obj->ID()] = Visibility::VIS_PARTIAL_VISIBILITY;
            DebugLogger(combat) << "   System " << obj->Name() << " always visible";
        }

        for (const Planet* obj : objects.allRaw<Planet>() | range_filter(not_null)) {
            // planets always at least basically visible to empires with objects in the system
            auto& obj_vis{empire_vis[obj->ID()]};
            DebugLogger(combat) << "   Planet " << obj->Name()
                                << ((obj_vis > Visibility::VIS_BASIC_VISIBILITY) ?
                                    " visible from universe state" : " has default basic visibility");
            obj_vis = std::max(Visibility::VIS_BASIC_VISIBILITY, obj_vis);
        }

        const bool aggressive_rule = GetGameRules().Get<bool>("RULE_AGGRESSIVE_SHIPS_COMBAT_VISIBLE");

        for (const Ship* obj : objects.allRaw<Ship>() | range_filter(not_null)) {
            // ships only initially visible if already detected or if the aggressive fleet rule applies
            auto& obj_vis{empire_vis[obj->ID()]};

            if (aggressive_rule && obj_vis < Visibility::VIS_PARTIAL_VISIBILITY) {
                const auto* fleet = objects.getRaw<const Fleet>(obj->FleetID());
                if (fleet && fleet->Aggressive()) {
                    obj_vis = Visibility::VIS_PARTIAL_VISIBILITY;
                    DebugLogger(combat) << "   Ship " << obj->Name() << " visible due to aggressive fleet rule";
                    continue;
                }
            }
            DebugLogger(combat) << "   Ship " << obj->Name()
                                << ((obj_vis >= Visibility::VIS_BASIC_VISIBILITY) ?
                                    " visible from universe state" : " initially hidden");
        }
    }
}


ScriptingContext::ScriptingContext(CombatInfo& info, Attacker, UniverseObject* attacker_as_source) noexcept :
    source(                 attacker_as_source),
    current_turn(           info.turn),
    combat_bout(            info.bout),
    galaxy_setup_data(      info.galaxy_setup_data),
    species(                info.species),
    supply(                 info.supply),
    const_universe(         info.universe),
    objects(                &info.objects), // not taken from Universe!
    const_objects(          info.objects),
    empire_object_vis(      info.empire_object_visibility), // not taken from Universe!
    empire_object_vis_turns(info.empire_object_vis_turns), // not taken from Universe!
    const_empires(          info.empires),
    diplo_statuses(         info.diplo_statuses)
{}


////////////////////////////////////////////////
// AutoResolveCombat
////////////////////////////////////////////////
namespace {
    // if source is owned by ALL_EMPIRES, match objects owned by an empire
    // if source is owned by an empire, match unowned objects and objects owned by enemies of source's owner empire
    std::unique_ptr<Condition::Condition> VisibleEnemyOfOwnerCondition() {
        return std::make_unique<Condition::Or>(
            // unowned candidate object case
            std::make_unique<Condition::And>(
                std::make_unique<Condition::EmpireAffiliation>(
                    EmpireAffiliationType::AFFIL_NONE),         // unowned candidate object

                std::make_unique<Condition::ValueTest>(         // when source object is owned (ie. not the same owner as the candidate object)
                    std::make_unique<ValueRef::Variable<int>>(
                        ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner"),
                    Condition::ComparisonType::NOT_EQUAL,
                    std::make_unique<ValueRef::Variable<int>>(
                        ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE, "Owner")),

                std::make_unique<Condition::VisibleToEmpire>(   // when source object's owner empire can detect the candidate object
                    std::make_unique<ValueRef::Variable<int>>(  // source's owner empire id
                        ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner"))),

            // owned candidate object case
            std::make_unique<Condition::And>(
                std::make_unique<Condition::EmpireAffiliation>( // candidate is owned by an empire
                    EmpireAffiliationType::AFFIL_ANY),

                std::make_unique<Condition::EmpireAffiliation>( // candidate is owned by enemy of source's owner
                    std::make_unique<ValueRef::Variable<int>>(
                        ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner"),
                        EmpireAffiliationType::AFFIL_ENEMY),

                std::make_unique<Condition::VisibleToEmpire>(     // when source empire can detect the candidate object
                    std::make_unique<ValueRef::Variable<int>>(    // source's owner empire id
                        ValueRef::ReferenceType::SOURCE_REFERENCE, "Owner"))
            ))
        ;
    }

    const std::unique_ptr<Condition::Condition> is_enemy_ship_or_fighter =
        std::make_unique<Condition::And>(
            std::make_unique<Condition::Or>(
                std::make_unique<Condition::And>(
                    std::make_unique<Condition::Type>(UniverseObjectType::OBJ_SHIP),
                    std::make_unique<Condition::Not>(
                        std::make_unique<Condition::MeterValue>(
                            MeterType::METER_STRUCTURE,
                            nullptr,
                            std::make_unique<ValueRef::Constant<double>>(0.0)))),
                std::make_unique<Condition::Type>(UniverseObjectType::OBJ_FIGHTER)),
            VisibleEnemyOfOwnerCondition());

    const std::unique_ptr<Condition::Condition> is_enemy_ship =
        std::make_unique<Condition::And>(
            std::make_unique<Condition::Type>(UniverseObjectType::OBJ_SHIP),

            std::make_unique<Condition::Not>(
                std::make_unique<Condition::MeterValue>(
                    MeterType::METER_STRUCTURE,
                    nullptr,
                    std::make_unique<ValueRef::Constant<double>>(0.0))),

            VisibleEnemyOfOwnerCondition());

    const std::unique_ptr<Condition::Condition> is_enemy_ship_fighter_or_armed_planet =
        std::make_unique<Condition::And>(
            VisibleEnemyOfOwnerCondition(), // enemies
            std::make_unique<Condition::Or>(
                std::make_unique<Condition::Or>(
                    std::make_unique<Condition::And>(
                        std::make_unique<Condition::Type>(UniverseObjectType::OBJ_SHIP),
                        std::make_unique<Condition::Not>(
                            std::make_unique<Condition::MeterValue>(
                                MeterType::METER_STRUCTURE,
                                nullptr,
                                std::make_unique<ValueRef::Constant<double>>(0.0)))),
                    std::make_unique<Condition::Type>(UniverseObjectType::OBJ_FIGHTER)),

                std::make_unique<Condition::And>(
                    std::make_unique<Condition::Type>(UniverseObjectType::OBJ_PLANET),
                    std::make_unique<Condition::Or>(
                        std::make_unique<Condition::Not>(
                            std::make_unique<Condition::MeterValue>(
                                MeterType::METER_DEFENSE,
                                nullptr,
                                std::make_unique<ValueRef::Constant<double>>(0.0))),
                        std::make_unique<Condition::Not>(
                            std::make_unique<Condition::MeterValue>(
                                MeterType::METER_SHIELD,
                                nullptr,
                                std::make_unique<ValueRef::Constant<double>>(0.0))),
                        std::make_unique<Condition::Not>(
                            std::make_unique<Condition::MeterValue>(
                                MeterType::METER_CONSTRUCTION,
                                nullptr,
                                std::make_unique<ValueRef::Constant<double>>(0.0)))))));

    const std::unique_ptr<Condition::Condition> if_source_is_planet_then_ships_else_all =
        std::make_unique<Condition::Or>(
            std::make_unique<Condition::And>(     // if source is a planet, match ships
                std::make_unique<Condition::Number>(
                    std::make_unique<ValueRef::Constant<int>>(1), // minimum objects matching subcondition
                    nullptr,
                    std::make_unique<Condition::And>(             // subcondition: source is a planet
                        std::make_unique<Condition::Source>(),
                        std::make_unique<Condition::Type>(UniverseObjectType::OBJ_PLANET)
                    )
                ),
                std::make_unique<Condition::Type>(UniverseObjectType::OBJ_SHIP)
            ),

            std::make_unique<Condition::Number>(  // if source is not a planet, match anything
                nullptr,
                std::make_unique<ValueRef::Constant<int>>(0),     // maximum objects matching subcondition
                std::make_unique<Condition::And>(                 // subcondition: source is a planet
                    std::make_unique<Condition::Source>(),
                    std::make_unique<Condition::Type>(UniverseObjectType::OBJ_PLANET)
                )
            )
        );

    struct PartAttackInfo {
        PartAttackInfo(ShipPartClass part_class_, const std::string& part_name_,
                       float part_attack_,
                       const ::Condition::Condition* combat_targets_ = nullptr) :
            part_class(part_class_),
            ship_part_name(part_name_),
            part_attack(part_attack_),
            combat_targets(combat_targets_)
        {}
        PartAttackInfo(ShipPartClass part_class_, const std::string& part_name_,
                       int fighters_launched_, float fighter_damage_,
                       const std::string& fighter_type_name_,
                       const ::Condition::Condition* combat_targets_ = nullptr) :
            part_class(part_class_),
            ship_part_name(part_name_),
            combat_targets(combat_targets_),
            fighters_launched(fighters_launched_),
            fighter_damage(fighter_damage_),
            fighter_type_name(fighter_type_name_)
        {}

        ShipPartClass                 part_class = ShipPartClass::INVALID_SHIP_PART_CLASS;
        const std::string             ship_part_name;
        float                         part_attack = 0.0f;    // for direct damage parts
        const ::Condition::Condition* combat_targets = nullptr;
        int                           fighters_launched = 0; // for fighter bays, input value should be limited by ship available fighters to launch
        float                         fighter_damage = 0.0f; // for fighter bays, input value should be determined by ship fighter weapon setup
        const std::string             fighter_type_name;
    };

    void AttackShipShip(Ship* attacker, const PartAttackInfo& weapon,
                        Ship* target, CombatInfo& combat_info,
                        int bout, int round,
                        WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        if (!attacker || !target) return;

        float power = weapon.part_attack;

        auto& damaged_object_ids = combat_info.damaged_object_ids;

        Meter* target_structure = target->UniverseObject::GetMeter(MeterType::METER_STRUCTURE);
        if (!target_structure) {
            ErrorLogger() << "couldn't get target structure or shield meter";
            return;
        }

        Meter* target_shield = target->UniverseObject::GetMeter(MeterType::METER_SHIELD);
        float shield = (target_shield ? target_shield->Current() : 0.0f);

        DebugLogger() << "AttackShipShip: attacker: " << attacker->Name()
                      << "  weapon: " << weapon.ship_part_name << " power: " << power
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

        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.ship_part_name,
                               power, shield, damage);

        attacker->SetLastTurnActiveInCombat(combat_info.turn);
        target->SetLastTurnActiveInCombat(combat_info.turn);
    }

    void AttackShipPlanet(Ship* attacker, const PartAttackInfo& weapon,
                          Planet* target, CombatInfo& combat_info,
                          int bout, int round,
                          WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event,
                          const Universe& universe)
    {
        if (!attacker || !target) return;
        float power = weapon.part_attack;
        if (power <= 0.0f)
            return;

        auto& damaged_object_ids = combat_info.damaged_object_ids;

        const ShipDesign* attacker_design = universe.GetShipDesign(attacker->DesignID());
        if (!attacker_design)
            return;

        Meter* target_shield = target->GetMeter(MeterType::METER_SHIELD);
        Meter* target_defense = target->UniverseObject::GetMeter(MeterType::METER_DEFENSE);
        Meter* target_construction = target->UniverseObject::GetMeter(MeterType::METER_CONSTRUCTION);
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
        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.ship_part_name,
                               power, 0.0f, total_damage);

        attacker->SetLastTurnActiveInCombat(combat_info.turn);
        target->SetLastTurnAttackedByShip(combat_info.turn);
    }

    void AttackShipFighter(Ship* attacker, const PartAttackInfo& weapon,
                           Fighter* target, CombatInfo& combat_info,
                           int bout, int round,
                           WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        float power = weapon.part_attack;

        if (power > 0.0f) {
            // any damage is enough to kill any fighter
            DebugLogger(combat) << "COMBAT: " << attacker->Name() << " of empire " << attacker->Owner()
                                << " (" << attacker->ID() << ") does " << power
                                << " damage to " << target->Name() << " (" << target->ID() << ")";
            target->SetDestroyed();
        }
        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.ship_part_name,
                               power, 0.0f, 1.0f);
        attacker->SetLastTurnActiveInCombat(combat_info.turn);
    }

    void AttackPlanetShip(Planet* attacker, const PartAttackInfo& weapon,
                          Ship* target, CombatInfo& combat_info,
                          int bout, int round,
                          WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        if (!attacker || !target) return;

        float power = 0.0f;
        const Meter* attacker_damage = attacker->UniverseObject::GetMeter(MeterType::METER_DEFENSE);
        if (attacker_damage)
            power = attacker_damage->Current();   // planet "Defense" meter is actually its attack power

        auto& damaged_object_ids = combat_info.damaged_object_ids;

        Meter* target_structure = target->UniverseObject::GetMeter(MeterType::METER_STRUCTURE);
        if (!target_structure) {
            ErrorLogger() << "couldn't get target structure or shield meter";
            return;
        }

        Meter* target_shield = target->UniverseObject::GetMeter(MeterType::METER_SHIELD);
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

        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.ship_part_name,
                               power, shield, damage);

        target->SetLastTurnActiveInCombat(combat_info.turn);
    }

    void AttackPlanetPlanet(Planet* attacker, const PartAttackInfo& weapon,
                            Planet* target, CombatInfo& combat_info,
                            int bout, int round,
                            WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        if (!attacker || !target) return;

        float power = 0.0f;
        const Meter* attacker_damage = attacker->UniverseObject::GetMeter(MeterType::METER_DEFENSE);
        if (attacker_damage)
            power = attacker_damage->Current();   // planet "Defense" meter is actually its attack power

        auto& damaged_object_ids = combat_info.damaged_object_ids;

        Meter* target_shield = target->GetMeter(MeterType::METER_SHIELD);
        Meter* target_defense = target->UniverseObject::GetMeter(MeterType::METER_DEFENSE);
        Meter* target_construction = target->UniverseObject::GetMeter(MeterType::METER_CONSTRUCTION);
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
        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.ship_part_name,
                               power, 0.0f, total_damage);

        //attacker->SetLastTurnActiveInCombat(combat_info.turn);
        //target->SetLastTurnAttackedByShip(combat_info.turn);
    }

    void AttackPlanetFighter(Planet* attacker, const PartAttackInfo& weapon,
                             Fighter* target, CombatInfo& combat_info,
                             int bout, int round,
                             WeaponsPlatformEvent::WeaponsPlatformEventPtr& combat_event)
    {
        if (!attacker || !target) return;

        float power = 0.0f;
        const Meter* attacker_damage = attacker->UniverseObject::GetMeter(MeterType::METER_DEFENSE);
        if (attacker_damage)
            power = attacker_damage->Current();   // planet "Defense" meter is actually its attack power

        if (power > 0.0f) {
            // any damage is enough to destroy any fighter
            DebugLogger(combat) << "COMBAT: " << attacker->Name() << " of empire " << attacker->Owner()
                                << " (" << attacker->ID() << ") does " << power
                                << " damage to " << target->Name() << " (" << target->ID() << ")";
            target->SetDestroyed();
        }

        combat_event->AddEvent(round, target->ID(), target->Owner(), weapon.ship_part_name,
                               power, 0.0f, 1.0f);
    }

    void AttackFighterShip(Fighter* attacker, const PartAttackInfo& weapon,
                           Ship* target, CombatInfo& combat_info,
                           int bout, int round, AttacksEventPtr& attacks_event)
    {
        if (!attacker || !target) return;

        float power = attacker->Damage();

        auto& damaged_object_ids = combat_info.damaged_object_ids;

        Meter* target_structure = target->UniverseObject::GetMeter(MeterType::METER_STRUCTURE);
        if (!target_structure) {
            ErrorLogger() << "couldn't get target structure or shield meter";
            return;
        }

        //Meter* target_shield = target->UniverseObject::GetMeter(MeterType::METER_SHIELD);
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
            bout, round, attacker->ID(), target->ID(), weapon.ship_part_name,
            std::tie(power, pierced_shield_value, damage),
            attacker->Owner(), target->Owner());
        attacks_event->AddEvent(std::move(attack_event));
        target->SetLastTurnActiveInCombat(combat_info.turn);
    }

    void AttackFighterPlanet(Fighter* attacker, const PartAttackInfo& weapon,
                             Planet* target, CombatInfo& combat_info,
                             int bout, int round, AttacksEventPtr& attacks_event)
    {
        if (!attacker || !target) return;

        float power = attacker->Damage();

        auto& damaged_object_ids = combat_info.damaged_object_ids;

        Meter* target_shield = target->GetMeter(MeterType::METER_SHIELD);
        Meter* target_defense = target->UniverseObject::GetMeter(MeterType::METER_DEFENSE);
        Meter* target_construction = target->UniverseObject::GetMeter(MeterType::METER_CONSTRUCTION);
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
        const float total_damage = shield_damage + defense_damage + construction_damage;

        static constexpr float pierced_shield_value(0.0f);
        auto attack_event = std::make_shared<WeaponFireEvent>(
            bout, round, attacker->ID(), target->ID(), weapon.ship_part_name,
            std::tie(power, pierced_shield_value, total_damage),
            attacker->Owner(), target->Owner());
        attacks_event->AddEvent(std::move(attack_event));

        target->SetLastTurnAttackedByShip(combat_info.turn);
    }

    void AttackFighterFighter(Fighter* attacker, const PartAttackInfo& weapon,
                              Fighter* target, CombatInfo& combat_info,
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

    void Attack(UniverseObject* attacker, const PartAttackInfo& weapon,
                UniverseObject* target, CombatInfo& combat_info,
                int bout, int round, AttacksEventPtr& attacks_event,
                WeaponsPlatformEvent::WeaponsPlatformEventPtr platform_event,
                std::shared_ptr<FightersAttackFightersEvent>& fighter_on_fighter_event)
    {
        const auto attack_ship = attacker->ObjectType() == UniverseObjectType::OBJ_SHIP ?
            static_cast<Ship*>(attacker) : nullptr;
        const auto attack_planet = attacker->ObjectType() == UniverseObjectType::OBJ_PLANET ?
            static_cast<Planet*>(attacker) : nullptr;
        const auto attack_fighter = attacker->ObjectType() == UniverseObjectType::OBJ_FIGHTER ?
            static_cast<Fighter*>(attacker) : nullptr;
        const auto target_ship = target->ObjectType() == UniverseObjectType::OBJ_SHIP ?
            static_cast<Ship*>(target) : nullptr;
        const auto target_planet = target->ObjectType() == UniverseObjectType::OBJ_PLANET ?
            static_cast<Planet*>(target) : nullptr;
        const auto target_fighter = target->ObjectType() == UniverseObjectType::OBJ_FIGHTER ?
            static_cast<Fighter*>(target) : nullptr;

        if (attack_ship && target_ship) {
            AttackShipShip(         attack_ship,    weapon, target_ship,    combat_info, bout, round, platform_event);
        } else if (attack_ship && target_planet) {
            AttackShipPlanet(       attack_ship,    weapon, target_planet,  combat_info, bout, round, platform_event, combat_info.universe);
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

    bool ObjectCanAttack(const UniverseObject* obj, const ScriptingContext& context) {
        switch (obj->ObjectType()) {
        case UniverseObjectType::OBJ_SHIP: {
            auto ship = static_cast<const Ship*>(obj);
            if (!ship->IsArmed(context))
                return false;
            const auto fleet = context.ContextObjects().get<Fleet>(ship->FleetID());
            if (!fleet) {
                ErrorLogger(combat) << "ObjectCanAttack unable to find a fleet " << ship->FleetID() << " for ship id " << ship->ID();
                return true;
            }
            return !fleet->Passive();
        }
        case UniverseObjectType::OBJ_PLANET:
            return obj->GetMeter(MeterType::METER_DEFENSE)->Current() > 0.0f;
        case UniverseObjectType::OBJ_FIGHTER:
            return static_cast<const Fighter*>(obj)->Damage() > 0.0f;
        default:
            return false;
        }
    }

    std::vector<PartAttackInfo> ShipWeaponsStrengths(const Ship* ship, const Universe& universe) {
        std::vector<PartAttackInfo> retval;
        if (!ship)
            return retval;
        const ShipDesign* design = universe.GetShipDesign(ship->DesignID());
        if (!design)
            return retval;

        std::set<std::string> seen_hangar_ship_parts;
        int available_fighters = 0;
        float fighter_attack = 0.0f;
        std::string fighter_name = UserString("OBJ_FIGHTER");   // default, may be overridden later
        std::map<std::string, int> part_fighter_launch_capacities;
        const ::Condition::Condition* fighter_combat_targets = nullptr;

        // determine what ship does during combat, based on parts and their meters...
        for (const auto& part_name : design->Parts()) {
            const ShipPart* part = GetShipPart(part_name);
            if (!part)
                continue;
            const ShipPartClass part_class = part->Class();
            const ::Condition::Condition* part_combat_targets = part->CombatTargets();

            // direct weapon and fighter-related parts all handled differently...
            if (part_class == ShipPartClass::PC_DIRECT_WEAPON) {
                const float part_attack = ship->CurrentPartMeterValue(MeterType::METER_CAPACITY, part_name);
                const int shots = static_cast<int>(ship->CurrentPartMeterValue(MeterType::METER_SECONDARY_STAT, part_name)); // secondary stat is shots per attack)
                if (part_attack > 0.0f && shots > 0) {
                    if (!part_combat_targets)
                        part_combat_targets = is_enemy_ship_fighter_or_armed_planet.get();

                    // attack for each shot...
                    for (int shot_count = 0; shot_count < shots; ++shot_count)
                        retval.emplace_back(part_class, part_name, part_attack, part_combat_targets);
                } else {
                    TraceLogger(combat) << "ShipWeaponsStrengths for ship " << ship->Name() << " (" << ship->ID() << ") "
                                        << " direct weapon part " << part->Name() << " has no shots / zero attack, so is skipped";
                }

            } else if (part_class == ShipPartClass::PC_FIGHTER_HANGAR) {
                // hangar max-capacity-modification effects stack, so only add capacity for each hangar type once
                if (!contains(seen_hangar_ship_parts, part_name)) {
                    available_fighters += static_cast<int>(ship->CurrentPartMeterValue(MeterType::METER_CAPACITY, part_name));
                    seen_hangar_ship_parts.insert(part_name);

                    if (!part_combat_targets)
                        part_combat_targets = is_enemy_ship_or_fighter.get();
                    TraceLogger(combat) << "ShipWeaponsStrengths for ship " << ship->Name() << " (" << ship->ID() << ") "
                                        << "when launching fighters, part " << part->Name() << " with targeting condition: "
                                        << part_combat_targets->Dump();
                    if (!fighter_combat_targets)
                        fighter_combat_targets = part_combat_targets;

                    if (UserStringExists(part->Name() + "_FIGHTER"))
                        fighter_name = UserString(part->Name() + "_FIGHTER");

                    // should only be one type of fighter per ship as of this writing
                    fighter_attack = ship->CurrentPartMeterValue(MeterType::METER_SECONDARY_STAT, part_name);  // secondary stat is fighter damage
                }

            } else if (part_class == ShipPartClass::PC_FIGHTER_BAY) {
                part_fighter_launch_capacities[part_name] += static_cast<int>(ship->CurrentPartMeterValue(MeterType::METER_CAPACITY, part_name));
            }
        }

        if (available_fighters > 0 && !part_fighter_launch_capacities.empty()) {
            for (auto& launch : part_fighter_launch_capacities) {
                int to_launch = std::min(launch.second, available_fighters);

                TraceLogger(combat) << "ShipWeaponsStrengths " << ship->Name() << " can launch " << to_launch
                                    << " fighters named \"" << fighter_name << "\" from bay part " << launch.first;
                if (fighter_combat_targets)
                    TraceLogger(combat) << " ... with targeting condition: " << fighter_combat_targets->Dump();
                else
                    TraceLogger(combat) << " ... with no targeting condition: ";

                if (to_launch <= 0)
                    continue;
                retval.emplace_back(ShipPartClass::PC_FIGHTER_BAY, launch.first, to_launch,
                                    fighter_attack, fighter_name,
                                    fighter_combat_targets); // attack may be 0; that's ok: decoys
                available_fighters -= to_launch;
                if (available_fighters <= 0)
                    break;
            }
        }

        return retval;
    }

    // Information about a single empire during combat
    struct EmpireCombatInfo {
        boost::unordered_set<int> attacker_ids;

        bool HasAttackers() const noexcept { return !attacker_ids.empty(); }

        bool HasUnlauchedArmedFighters(const CombatInfo& combat_info) const {
            // check each ship to see if it has any unlaunched armed fighters...
            for (const auto* ship : combat_info.objects.findRaw<Ship>(attacker_ids)) { // TODO: check_if_any
                if (!ship)
                    continue;   // discard invalid ship references
                if (contains(combat_info.destroyed_object_ids, ship->ID()))
                    continue;   // destroyed objects can't launch fighters

                auto weapons = ShipWeaponsStrengths(ship, combat_info.universe);
                for (const PartAttackInfo& weapon : weapons) { // TODO: any_of
                    if (weapon.part_class == ShipPartClass::PC_FIGHTER_BAY &&
                        weapon.fighters_launched > 0 &&
                        weapon.fighter_damage > 0.0f)
                    { return true; }
                }
            }

            return false;
        }
    };

    // how many base-10 digits are needed to represent a number as text
    // note that numeric_limits<>::digits10 is how many base 10 digits can be represented by this type
    template <typename T> requires (std::is_integral_v<T>)
    consteval std::size_t Digits(T t) {
        std::size_t retval = 1;

        if constexpr (std::is_same_v<T, bool>) {
            return 5; // for "false"
        } else {
            if constexpr (std::is_signed_v<T>)
                retval += (t < 0);

            while (t != 0) {
                retval += 1;
                t /= 10;
            }
            return retval;
        }
    }

    /// A collection of information the autoresolution must keep around
    struct AutoresolveInfo {
        boost::unordered_set<int>                         valid_attacker_object_ids;  // all objects that can attack
        boost::container::flat_map<int, EmpireCombatInfo> empire_infos;               // empire specific information, indexed by empire id
        CombatInfo&                                       combat_info;
        int                                               next_fighter_id = -1000001; // give fighters negative ids so as to avoid clashes with any positive-id of persistent UniverseObjects
        boost::container::flat_set<int>                   destroyed_object_ids;       // objects that have been destroyed so far during this combat

        explicit AutoresolveInfo(CombatInfo& combat_info_) :
            combat_info(combat_info_)
        {
            valid_attacker_object_ids.reserve(combat_info.objects.size());
            empire_infos.reserve(combat_info.empire_ids.size());
            for (auto eid : combat_info.empire_ids)
                empire_infos.emplace(std::piecewise_construct, std::forward_as_tuple(eid), std::forward_as_tuple());
            destroyed_object_ids.reserve(combat_info.objects.size());
            PopulateAttackers();
        }

        std::vector<int> AddFighters(int number, float damage, int owner_empire_id,
                                     int from_ship_id, const std::string& species,
                                     std::string fighter_name,
                                     const Condition::Condition* combat_targets)
        {
            std::vector<int> retval;
            retval.reserve(number);

            if (combat_targets)
                TraceLogger(combat) << "Adding " << number << " fighters for empire " << owner_empire_id
                                    << " with targetting condition: " << combat_targets->Dump();
            else
                TraceLogger(combat) << "Adding " << number << " fighters for empire " << owner_empire_id
                                    << " with no targetting condition";

            for (int n = 0; n < number; ++n) {
                // create / insert fighter into combat objectmap

                //Fighter(int empire_id, int launched_from_id, const std::string& species_name,
                //        float damage, const ::Condition::Condition* combat_targets,
                //        int current_turn, const Universe& universe);
                auto fighter_ptr = std::make_shared<Fighter>(owner_empire_id, from_ship_id,
                                                             species, damage, combat_targets);
                if (!fighter_ptr) {
                    ErrorLogger(combat) << "AddFighters unable to create and insert new Fighter object...";
                    break;
                }

                const int new_id = next_fighter_id--;
                fighter_ptr->SetID(new_id);
                fighter_ptr->Rename(fighter_name);
                combat_info.objects.insert(std::move(fighter_ptr), contains(destroyed_object_ids, new_id));
                retval.push_back(new_id);

                // add fighter to attackers (if it can attack)
                if (damage > 0.0f) {
                    valid_attacker_object_ids.insert(new_id);
                    empire_infos[owner_empire_id].attacker_ids.insert(new_id);
                    DebugLogger(combat) << "Added fighter id: " << new_id << " to attackers sets";
                }

                // mark fighter visible to all empire participants
                for (auto viewing_empire_id : combat_info.empire_ids)
                    combat_info.empire_object_visibility[viewing_empire_id][new_id] = Visibility::VIS_PARTIAL_VISIBILITY;
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
            delete_list.reserve(combat_info.objects.size());

            for (const auto* obj : combat_info.objects.allRaw()) { // TODO: rangify
                // Check if object is already noted as destroyed; don't need to re-record this
                if (contains(destroyed_object_ids, obj->ID()))
                    continue;
                // Check if object is destroyed and update lists if yes
                if (!CheckDestruction(obj))
                    continue;
                destroyed_object_ids.insert(obj->ID());
                TraceLogger(combat) << "Added destroyed object id: " << obj->ID();

                if (obj->ObjectType() == UniverseObjectType::OBJ_FIGHTER) {
                    fighters_destroyed_event->AddEvent(obj->Owner());
                    at_least_one_fighter_destroyed = true;
                    // delete actual fighter object so that it can't be targeted
                    // again next round (ships have a minimal structure test instead)
                    delete_list.push_back(obj->ID());
                } else {
                    incaps_event->AddEvent(std::make_shared<IncapacitationEvent>(bout, obj->ID(), obj->Owner()));
                }
            }

            if (at_least_one_fighter_destroyed)
                bout_event->AddEvent(std::move(fighters_destroyed_event));

            if (!incaps_event->AreSubEventsEmpty(ALL_EMPIRES))
                bout_event->AddEvent(std::move(incaps_event));


            for (auto id : delete_list)
                combat_info.objects.erase(id);

            DebugLogger() << "Removed destroyed objects from combat state with ids: " << [&delete_list]() {
                using list_v_t = std::decay_t<decltype(delete_list.front())>;
                static constexpr auto digits_per_id = Digits(std::numeric_limits<list_v_t>::max());
                std::string str;
                str.reserve(delete_list.size() * digits_per_id + 2);
                for (auto id : delete_list)
                    str.append(std::to_string(id)).append(" ");
                return str;
            }();
        }

        /// Checks if target is destroyed and if it is, update lists of living objects.
        /// Return true if is incapacitated
        bool CheckDestruction(const UniverseObject* target) {
            const int target_id = target->ID();
            // check for destruction of target object

            if (target->ObjectType() == UniverseObjectType::OBJ_FIGHTER) {
                auto fighter = static_cast<const Fighter*>(target);
                if (fighter->Destroyed()) {
                    // remove destroyed fighter's ID from lists of valid attackers and targets
                    valid_attacker_object_ids.erase(target_id);
                    DebugLogger(combat) << "Removed destroyed fighter id: " << fighter->ID() << " from attackers";

                    // Remove target from its empire's list of attackers
                    empire_infos[target->Owner()].attacker_ids.erase(target_id);
                    CleanEmpires();
                    return true;
                }

            } else if (target->ObjectType() == UniverseObjectType::OBJ_SHIP) {
                if (target->GetMeter(MeterType::METER_STRUCTURE)->Current() <= 0.0f) {
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

            } else if (target->ObjectType() == UniverseObjectType::OBJ_PLANET) {
                if (!ObjectCanAttack(target, ScriptingContext{combat_info}) &&
                    contains(valid_attacker_object_ids, target_id))
                {
                    DebugLogger(combat) << "!! Target Planet " << target_id << " knocked out, can no longer attack";
                    // remove disabled planet's ID from lists of valid attackers
                    valid_attacker_object_ids.erase(target_id);
                }

                if (target->GetMeter(MeterType::METER_SHIELD)->Current() <= 0.0f &&
                    target->GetMeter(MeterType::METER_DEFENSE)->Current() <= 0.0f &&
                    target->GetMeter(MeterType::METER_CONSTRUCTION)->Current() <= 0.0f)
                {
                    // An outpost can enter combat in essentially an
                    // incapacitated state, but if it is removed from combat
                    // before it has been attacked then it can wrongly get regen
                    // on the next turn, so check that it has been attacked
                    // before excluding it from any remaining battle
                    if (!contains(combat_info.damaged_object_ids, target_id)) {
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
            auto temp{empire_infos};

            boost::container::flat_set<int> empire_ids_with_objects;
            empire_ids_with_objects.reserve(20); // guesstimate, should normally be enough
            for (const auto* obj : combat_info.objects.allRaw()) // TODO: range, range init, and make container const
                empire_ids_with_objects.insert(obj->Owner());

            for (const auto empire_id : empire_infos | range_keys) {
                if (!contains(empire_ids_with_objects, empire_id)) {
                    temp.erase(empire_id);
                    DebugLogger(combat) << "No objects left for empire with id: " << empire_id;
                }
            }
            empire_infos = std::move(temp);

            if (!empire_infos.empty()) {
                DebugLogger(combat) << "Empires with objects remaining:";
                for (const auto& [empire_id, empire_info] : empire_infos) {
                    DebugLogger(combat) << " ... " << empire_id;
                    for (const auto obj_id : empire_info.attacker_ids)
                        TraceLogger(combat) << " ... ... " << obj_id;
                }
            }
        }

        /// Clears and refills \a shuffled with attacker ids in a random order
        auto GetShuffledValidAttackerIDs() {
            std::vector<int> retval{valid_attacker_object_ids.begin(), valid_attacker_object_ids.end()};
            RandomShuffle(retval);
            return retval;
        }

        /** Report for each empire the stealthy objects in the combat. */
        InitialStealthEvent::EmpireToObjectVisibilityMap ReportInvisibleObjects() const {
            DebugLogger(combat) << "Reporting Invisible Objects";
            InitialStealthEvent::EmpireToObjectVisibilityMap report;

            // loop over all objects, noting which is visible by which empire or neutrals
            for (const auto* target : combat_info.objects.allRaw()) {
                // for all empires, can they detect this object?
                for (int viewing_empire_id : combat_info.empire_ids) {
                    // get visibility of target to attacker empire
                    auto empire_vis_info_it = combat_info.empire_object_visibility.find(viewing_empire_id);
                    if (empire_vis_info_it == combat_info.empire_object_visibility.end()) {
                        DebugLogger() << " ReportInvisibleObjects found no visibility info for viewing empire " << viewing_empire_id;
                        report[viewing_empire_id].emplace(target->ID(), Visibility::VIS_NO_VISIBILITY);
                        continue;
                    }
                    auto target_visibility_for_empire_it = empire_vis_info_it->second.find(target->ID());
                    if (target_visibility_for_empire_it == empire_vis_info_it->second.end()) {
                        DebugLogger() << " ReportInvisibleObjects found no visibility record for viewing empire "
                                      << viewing_empire_id << " for object " << target->Name() << " (" << target->ID() << ")";
                        report[viewing_empire_id].emplace(target->ID(), Visibility::VIS_NO_VISIBILITY);
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
                    report[viewing_empire_id].emplace(target->ID(), vis);
                }
            }
            return report;
        }

    private:
        typedef std::set<int>::const_iterator const_id_iterator;

        // Populate lists of things that can attack. List attackers also by empire.
        void PopulateAttackers() {
            auto check_add = [&](auto&& range) {
                for (const auto& obj : range) {
                    bool can_attack{ObjectCanAttack(obj, ScriptingContext{combat_info})};
                    if (can_attack) {
                        valid_attacker_object_ids.insert(obj->ID());
                        empire_infos[obj->Owner()].attacker_ids.insert(obj->ID());
                    }

                    DebugLogger(combat) << "Considering object " << obj->Name() << " (" << obj->ID() << ")"
                                        << " owned by " << std::to_string(obj->Owner())
                                        << (can_attack ? "... can attack" : "");
                }
            };

            check_add(combat_info.objects.allRaw<const Planet>());
            check_add(combat_info.objects.allRaw<const Ship>());
        }
    };

    std::vector<PartAttackInfo> GetWeapons(const UniverseObject* attacker,
                                           const Universe& universe)
    {
        // Loop over weapons of attacking object. Each gets a shot at a
        // randomly selected target object, from the objects in the combat
        // that match the weapon's targetting condition.
        std::vector<PartAttackInfo> weapons;


        if (attacker->ObjectType() == UniverseObjectType::OBJ_SHIP) {
            auto attack_ship = static_cast<const Ship*>(attacker);
            weapons = ShipWeaponsStrengths(attack_ship, universe); // includes info about fighter launches with ShipPartClass::PC_FIGHTER_BAY part class, and direct fire weapons with ShipPartClass::PC_DIRECT_WEAPON part class
            for (PartAttackInfo& part : weapons) {
                if (part.part_class == ShipPartClass::PC_DIRECT_WEAPON) {
                    DebugLogger(combat) << "Attacker Ship has direct weapon: " << part.ship_part_name
                                        << " attack: " << part.part_attack;
                } else if (part.part_class == ShipPartClass::PC_FIGHTER_BAY) {
                    DebugLogger(combat) << "Attacker Ship can fighter launch: " << part.fighters_launched
                                        << " damage: " << part.fighter_damage;
                    if (part.combat_targets)
                        TraceLogger(combat) << " ... fighter targeting condition: " << part.combat_targets->Dump();
                    else
                        TraceLogger(combat) << " ... fighter has no targeting condition";
                }
            }

        } else if (attacker->ObjectType() == UniverseObjectType::OBJ_PLANET) { // treat planet defenses as direct fire weapon that only target ships
            auto attack_planet = static_cast<const Planet*>(attacker);
            weapons.emplace_back(ShipPartClass::PC_DIRECT_WEAPON, UserStringNop("DEF_DEFENSE"),
                                 attack_planet->GetMeter(MeterType::METER_DEFENSE)->Current(),
                                 is_enemy_ship.get());

        } else if (attacker->ObjectType() == UniverseObjectType::OBJ_FIGHTER) { // treat fighter damage as direct fire weapon
            auto attack_fighter = static_cast<const Fighter*>(attacker);
            weapons.emplace_back(ShipPartClass::PC_DIRECT_WEAPON, UserStringNop("FT_WEAPON_1"),
                                 attack_fighter->Damage(),
                                 attack_fighter->CombatTargets());
        }
        return weapons;
    }

    const Condition::Condition* SpeciesTargettingCondition(
        const UniverseObject* attacker, const SpeciesManager& species_manager)
    {
        if (!attacker)
            return if_source_is_planet_then_ships_else_all.get();

        const Species* species = nullptr;
        if (auto attack_ship = dynamic_cast<const Ship*>(attacker))
            species = species_manager.GetSpecies(attack_ship->SpeciesName());
        else if (auto attack_planet = dynamic_cast<const Planet*>(attacker))
            species = species_manager.GetSpecies(attack_planet->SpeciesName());
        else if (auto attack_fighter = dynamic_cast<const Fighter*>(attacker))
            species = species_manager.GetSpecies(attack_fighter->SpeciesName());

        if (!species || !species->CombatTargets())
            return if_source_is_planet_then_ships_else_all.get();

        return species->CombatTargets();
    }

    void AddObjects(ObjectMap& obj_map, Effect::TargetSet& into_set, const auto& exclude_ids) {
        using MapPair = typename ObjectMap::container_type<const UniverseObject>::value_type;
        auto objs = obj_map.findRaw([&exclude_ids](const MapPair& id_obj)
                                    { return !contains(exclude_ids, id_obj.first); });
        into_set.reserve(into_set.size() + objs.size());
        into_set.insert(into_set.end(), objs.begin(), objs.end());
    }

    void ShootAllWeapons(UniverseObject* attacker,
                         AutoresolveInfo& combat_state, int round,
                         AttacksEventPtr& attacks_event,
                         WeaponsPlatformEvent::WeaponsPlatformEventPtr& platform_event,
                         std::shared_ptr<FightersAttackFightersEvent>& fighter_on_fighter_event)
    {
        auto weapons = GetWeapons(attacker, combat_state.combat_info.universe);
        if (weapons.empty()) {
            DebugLogger(combat) << "Attacker " << attacker->Name() << " ("
                                << attacker->ID() << ") has no weapons, so can't attack";
            return;   // no ability to attack!
        }

        const auto* species_targetting_condition = SpeciesTargettingCondition(attacker, combat_state.combat_info.species);
        if (!species_targetting_condition) {
            ErrorLogger(combat) << "Null Species Targetting Condition...!?";
            return;
        }
        TraceLogger(combat) << "Species targeting condition: " << species_targetting_condition->Dump();

        // use combat-specific gamestate info for the ScriptingContext with which
        // to evaluate targetting conditions.
        // attacker is source object for condition evaluation.
        //             const Universe& universe,
        //             ObjectMap& objects_,
        //             const EmpireManager& empires_,
        //             const GalaxySetupData& galaxy_setup_data_,
        //             const SpeciesManager& species_,
        //             const SupplyManager& supply_) 

        ScriptingContext context{combat_state.combat_info, ScriptingContext::Attacker{}, attacker};

        TraceLogger(combat) << "Set up context in ShootAllWeapons: objects: " << context.ContextObjects().size()
                            << "  const objects: " << context.ContextObjects().size()
                            << "  visible objects: empires: " << context.empire_object_vis.size() << "  see: "
                            << [atk_id{attacker->ID()}, objs{context.ContextObjects()}, eov{context.empire_object_vis}]()
        {
            std::stringstream ss;

            for (auto& [empire_id, obj_vis] : eov) {
                ss << "Empire " << empire_id << " sees: ";
                for (auto& [obj_id, vis] : obj_vis) {
                    if (vis > Visibility::VIS_NO_VISIBILITY)
                        ss << obj_id << "  ";
                }
                ss << "\n";
            }

            return ss.str();
        }()
                            << "  empires: " << std::as_const(context).Empires().NumEmpires()
                            << "  diplostatus: " << [ds{context.diplo_statuses}]()
        {
            std::stringstream ss;
            for (auto& s : ds)
                ss << "(" << s.first.first << ", " << s.first.second << "): " << s.second << "    ";
            return ss.str();
        }();


        for (const PartAttackInfo& weapon : weapons) {
            // skip non-direct-fire weapons (as only direct fire weapons can "shoot").
            // fighter launches handled separately
            if (weapon.part_class != ShipPartClass::PC_DIRECT_WEAPON)
                continue;

            // select object from valid targets for this object's owner
            DebugLogger(combat) << "Attacker " << attacker->Name() << " ("
                                << attacker->ID() << ") attacks with weapon "
                                << weapon.ship_part_name << " with power " << weapon.part_attack;

            if (!weapon.combat_targets) {
                DebugLogger(combat) << "Weapon has no targeting condition?? Should have been set when initializing PartAttackInfo";
                continue;
            }


            Effect::TargetSet targets, rejected_targets;
            AddObjects(combat_state.combat_info.objects, targets, combat_state.destroyed_object_ids);

            TraceLogger(combat) << "all candidate targets: " << [&targets]() -> std::string {
                std::stringstream retval;
                for (auto& t : targets)
                    retval << " " << t->ID();
                return retval.str();
            }();

            // apply species targeting condition and then weapon targeting condition
            TraceLogger(combat) << "Species targeting condition: " << species_targetting_condition->Dump();
            species_targetting_condition->Eval(context, targets, rejected_targets, Condition::SearchDomain::MATCHES);
            if (targets.empty()) {
                DebugLogger(combat) << "No objects matched species targeting condition!";
                continue;
            }

            TraceLogger(combat) << "Weapon targeting condition: " << weapon.combat_targets->Dump();
            weapon.combat_targets->Eval(context, targets, rejected_targets, Condition::SearchDomain::MATCHES);
            if (targets.empty()) {
                DebugLogger(combat) << "No objects matched species and weapon targeting condition!";
                continue;
            }

            DebugLogger(combat) << targets.size() << " objects matched targeting condition";
            for (const auto& match : targets)
                TraceLogger(combat) << " ... " << match->Name() << " (" << match->ID() << ")";

            // select target object from matches
            int target_idx = RandInt(0, targets.size() - 1);
            auto* target = *std::next(targets.begin(), target_idx);

            if (!target) {
                ErrorLogger(combat) << "AutoResolveCombat selected null target object?";
                continue;
            }
            DebugLogger(combat) << "Selected target: " << target->Name() << " (" << target->ID() << ")";

            // do actual attacks
            Attack(attacker, weapon, target, combat_state.combat_info,
                   combat_state.combat_info.bout, round,
                   attacks_event, platform_event, fighter_on_fighter_event);

        } // end for over weapons
    }

    void ReduceStoredFighterCount(Ship* ship, float launched_fighters, const Universe& universe) {
        if (!ship || launched_fighters <= 0)
            return;

        // get how many fighters are initialy in each part type...
        // may be multiple hangar part types, each with different capacity (number of stored fighters)
        std::map<std::string, Meter*> ship_part_fighter_hangar_capacities;

        const ShipDesign* design = universe.GetShipDesign(ship->DesignID());
        if (!design) {
            ErrorLogger(combat) << "ReduceStoredFighterCount couldn't get ship design with id " << ship->DesignID();
            return;
        }

        // get hangar part meter values
        for (const std::string& part_name : design->Parts()) {
            const ShipPart* part = GetShipPart(part_name);
            if (!part)
                continue;
            if (part->Class() != ShipPartClass::PC_FIGHTER_HANGAR)
                continue;
            ship_part_fighter_hangar_capacities[part_name] = ship->GetPartMeter(MeterType::METER_CAPACITY, part_name);
        }

        // reduce meters until requested fighter reduction is achived
        // doesn't matter which part's capacity meters are reduced, as all
        // fighters are the same on the ship
        for (auto& part : ship_part_fighter_hangar_capacities) {
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

    void LaunchFighters(UniverseObject* attacker,
                        const std::vector<PartAttackInfo>& weapons,
                        AutoresolveInfo& combat_state, int round,
                        FighterLaunchesEventPtr& launches_event)
    {
        if (weapons.empty()) {
            DebugLogger(combat) << "no weapons' can't launch figters!";
            return;   // no ability to attack!
        }

        auto attacker_ship = dynamic_cast<Ship*>(attacker);
        const auto& species_name = [&attacker, &attacker_ship]() {
            if (attacker_ship)
                return attacker_ship->SpeciesName();
            else if (auto attacker_planet = dynamic_cast<Planet*>(attacker))
                return attacker_planet->SpeciesName();
            else
                return EMPTY_STRING;
        }();

        int attacker_owner_id = attacker->Owner();
        const auto empire = combat_state.combat_info.GetEmpire(attacker_owner_id);
        const auto& empire_name = (empire ? empire->Name() : UserString("ENC_COMBAT_ROGUE"));


        for (const auto& weapon : weapons) {
            // skip non-fighter weapons
            // direct fire weapons handled separately
            if (weapon.part_class != ShipPartClass::PC_FIGHTER_BAY)    // passed in weapons container should have already checked for adequate supply of fighters to launch and the available bays, and condensed into a single entry...
                continue;

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

            auto new_fighter_ids =
                combat_state.AddFighters(weapon.fighters_launched, weapon.fighter_damage,
                                         attacker_owner_id, attacker->ID(), species_name,
                                         fighter_name, weapon.combat_targets);

            // combat event
            launches_event->AddEvent(std::make_shared<FighterLaunchEvent>(
                combat_state.combat_info.bout, attacker->ID(), attacker_owner_id, new_fighter_ids.size()));


            // reduce hangar capacity (contents) corresponding to launched fighters
            const int num_launched = new_fighter_ids.size();
            if (attacker_ship)
                ReduceStoredFighterCount(attacker_ship, static_cast<float>(num_launched),
                                         combat_state.combat_info.universe);

            // launching fighters counts as a ship being active in combat
            if (!new_fighter_ids.empty())
                attacker_ship->SetLastTurnActiveInCombat(combat_state.combat_info.turn);

            break;  // don't need to check any more weapons, as all fighters launched should have been contained in the single ShipPartClass::PC_FIGHTER_BAY entry
        } // end for over weapons
    }

    void IncreaseStoredFighterCount(Ship& ship, float recovered_fighters, const Universe& universe) {
        if (recovered_fighters <= 0)
            return;

        // get how many fighters are initialy in each part type...
        // may be multiple hangar part types, each with different capacity (number of stored fighters)
        std::map<std::string, std::pair<Meter*, Meter*>> ship_part_fighter_hangar_capacities;

        const ShipDesign* design = universe.GetShipDesign(ship.DesignID());
        if (!design) {
            ErrorLogger(combat) << "IncreaseStoredFighterCount couldn't get ship design with id " << ship.DesignID();
            return;
        }

        // get hangar part meter values
        for (const std::string& part_name : design->Parts()) {
            const ShipPart* part = GetShipPart(part_name);
            if (!part || part->Class() != ShipPartClass::PC_FIGHTER_HANGAR)
                continue;
            ship_part_fighter_hangar_capacities.try_emplace(
                part_name,
                ship.GetPartMeter(MeterType::METER_CAPACITY, part_name),
                ship.GetPartMeter(MeterType::METER_MAX_CAPACITY, part_name));
        }

        // increase capacity meters until requested fighter allocation is
        // recovered. ioesn't matter which part's capacity meters are increased,
        // since all fighters are the same on the ship
        for (auto& [part_name, cap_max] : ship_part_fighter_hangar_capacities) {
            if (!cap_max.first || !cap_max.second)
                continue;
            float space = cap_max.second->Current() - cap_max.first->Current();
            float increase = std::min(space, recovered_fighters);
            recovered_fighters -= increase;

            DebugLogger() << "Increasing stored fighter count of ship " << ship.Name()
                          << " (" << ship.ID() << ") from " << cap_max.first->Current()
                          << " by " << increase << " towards max of "
                          << cap_max.second->Current();

            cap_max.first->AddToCurrent(increase);

            // stop if all fighters recovered
            if (recovered_fighters <= 0.0f)
                break;
        }
    }

    void RecoverFighters(CombatInfo& combat_info, int bout, FighterLaunchesEventPtr& launches_event) {
        std::map<int, float> ships_fighters_to_add_back;
        DebugLogger() << "Recovering fighters at end of combat...";

        // count still-existing and not destroyed fighters at end of combat
        for (const auto* obj : combat_info.objects.allRaw()) { // TODO: call this iterate over <Fighter> and avoid the cast ? would require a dedicated Fighter map in ObjectMap
            const int obj_id = obj->ID();
            if (obj_id >= 0 || obj->ObjectType() != UniverseObjectType::OBJ_FIGHTER)
                continue;
            auto fighter = static_cast<const Fighter*>(obj);
            if (fighter->Destroyed())
                continue;   // destroyed fighters can't return
            auto launched_from_id = fighter->LaunchedFrom();
            const auto& cidoi = combat_info.destroyed_object_ids;
            if (std::any_of(cidoi.begin(), cidoi.end(),
                            [launched_from_id](int id) { return launched_from_id == id; }))
            {
                DebugLogger() << " ... Fighter " << fighter->Name() << " (" << obj_id
                              << ") is from destroyed ship id" << launched_from_id
                              << " so can't be recovered";
                continue;   // can't return to a destroyed ship
            }
            ships_fighters_to_add_back[launched_from_id]++;
        }
        DebugLogger() << "Fighters left at end of combat:";
        for (const auto& [ship_id, fighter_count] : ships_fighters_to_add_back)
            DebugLogger() << " ... from ship id " << ship_id << " : " << fighter_count;


        DebugLogger() << "Returning fighters to ships:";
        for (const auto& [ship_id, fighter_count] : ships_fighters_to_add_back) {
            auto ship = combat_info.objects.getRaw<Ship>(ship_id);
            if (!ship) {
                ErrorLogger(combat) << "Couldn't get ship with id " << ship_id << " for fighter to return to...";
                continue;
            }
            IncreaseStoredFighterCount(*ship, fighter_count, combat_info.universe);
            // launching negative ships indicates recovery of them
            launches_event->AddEvent(std::make_shared<FighterLaunchEvent>(
                bout, ship_id, ship->Owner(), -static_cast<int>(fighter_count)));
        }
    }

    void CombatRound(AutoresolveInfo& combat_state) {
        CombatInfo& combat_info = combat_state.combat_info;
        const ScriptingContext context{combat_info};

        auto bout_event = std::make_shared<BoutEvent>(combat_info.bout);
        combat_info.combat_events.push_back(bout_event);
        if (combat_state.valid_attacker_object_ids.empty()) {
            DebugLogger(combat) << "Combat bout " << combat_info.bout << " aborted due to no remaining attackers.";
            return;
        }

        const auto shuffled_attackers = combat_state.GetShuffledValidAttackerIDs();

        DebugLogger() << [&shuffled_attackers](){
            std::stringstream ss;
            ss << "Attacker IDs: [";
            for (int attacker : shuffled_attackers)
                ss << attacker << " ";
            ss << "]";
            return ss.str();
        }();

        auto attacks_event = std::make_shared<AttacksEvent>();
        bout_event->AddEvent(attacks_event);

        auto fighter_on_fighter_event = std::make_shared<FightersAttackFightersEvent>(combat_info.bout);
        bout_event->AddEvent(fighter_on_fighter_event);

        int round = 1;  // counter of events during the current combat bout
        const int NUM_COMBAT_ROUNDS = GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS");

        // TODO: cache results of GetWeapons(attacker) to avoid re-calling multiple times per combat.
        // TODO: and pass into ShootAllWeapons which also calls that function


        // Process planets attacks first so that they still have full power,
        // despite their attack power depending on something (their defence meter)
        // that processing shots at them may reduce.
        for (auto* planet : combat_info.objects.findRaw<Planet>(shuffled_attackers)) {
            if (!planet)
                continue;
            if (!ObjectCanAttack(planet, context)) {
                DebugLogger() << "Planet " << planet->Name() << " could not attack.";
                continue;
            }
            DebugLogger(combat) << "Planet: " << planet->Name();

            auto platform_event = std::make_shared<WeaponsPlatformEvent>(
                combat_info.bout, planet->ID(), planet->Owner());
            attacks_event->AddEvent(platform_event);

            ShootAllWeapons(planet, combat_state, round++,
                            attacks_event, platform_event, fighter_on_fighter_event);
        }


        // Process ship and fighter attacks
        for (auto* attacker : combat_info.objects.findRaw(shuffled_attackers)) {
            if (!attacker)
                continue;
            if (attacker->ObjectType() == UniverseObjectType::OBJ_PLANET)
                continue;   // planet attacks processed above

            if (!ObjectCanAttack(attacker, context)) {
                DebugLogger() << "Attacker " << attacker->ObjectType() << " : "
                              << attacker->Name() << " (" << attacker->ID() << ") could not attack.";
                continue;
            }
            DebugLogger(combat) << "Attacker: " << attacker->ObjectType() << " : "
                                << attacker->Name() << " (" << attacker->ID() << ")";

            auto platform_event = std::make_shared<WeaponsPlatformEvent>(
                combat_info.bout, attacker->ID(), attacker->Owner());

            ShootAllWeapons(attacker, combat_state, round++,
                            attacks_event, platform_event, fighter_on_fighter_event);

            if (!platform_event->AreSubEventsEmpty(attacker->Owner()))
                attacks_event->AddEvent(std::move(platform_event));
        }

        auto stealth_change_event = std::make_shared<StealthChangeEvent>(combat_info.bout);

        // Launch fighters (which can attack in any subsequent combat bouts).
        // There is no point to launching fighters during the last bout, since
        // they won't get any chance to attack during this combat
        if (combat_info.bout < NUM_COMBAT_ROUNDS) {
            auto launches_event = std::make_shared<FighterLaunchesEvent>();
            for (auto* attacker : combat_info.objects.findRaw<Ship>(shuffled_attackers)) {
                if (!attacker)
                    continue;
                if (!ObjectCanAttack(attacker, context)) {
                    DebugLogger() << "Attacker " << attacker->Name() << " could not attack.";
                    continue;
                }
                auto weapons = GetWeapons(attacker, combat_info.universe); // includes info about fighter launches with ShipPartClass::PC_FIGHTER_BAY part class, and direct fire weapons (ships, planets, or fighters) with ShipPartClass::PC_DIRECT_WEAPON part class

                LaunchFighters(attacker, weapons, combat_state, round++, launches_event);

                DebugLogger(combat) << "Attacker: " << attacker->Name();

                // Set launching carrier as at least basically visible to other empires.
                if (launches_event->AreSubEventsEmpty(ALL_EMPIRES))
                    continue;

                for (auto detector_empire_id : combat_info.empire_ids) {
                    Visibility initial_vis = combat_info.empire_object_visibility[detector_empire_id][attacker->ID()];
                    TraceLogger(combat) << "Pre-attack visibility of launching carrier id: " << attacker->ID()
                                        << " by empire: " << detector_empire_id << " was: " << initial_vis;

                    if (initial_vis >= Visibility::VIS_BASIC_VISIBILITY)
                        continue;

                    combat_info.empire_object_visibility[detector_empire_id][attacker->ID()] =
                        Visibility::VIS_BASIC_VISIBILITY;

                    DebugLogger(combat) << " ... Setting post-attack visability to " << Visibility::VIS_BASIC_VISIBILITY;

                    // record visibility change event due to attack
                    // FIXME attacker, TARGET, attacker empire, target empire, visibility
                    stealth_change_event->AddEvent(attacker->ID(), attacker->Owner(), detector_empire_id,
                                                   Visibility::VIS_BASIC_VISIBILITY);
                }
            }

            if (!launches_event->AreSubEventsEmpty(ALL_EMPIRES))
                bout_event->AddEvent(std::move(launches_event));
        }


        // Create weapon fire events and mark attackers as visible to other battle participants
        auto attacks_this_bout = attacks_event->SubEvents(ALL_EMPIRES);
        for (const auto& this_event : attacks_this_bout) {
            // Generate attack events
            std::vector<std::shared_ptr<const WeaponFireEvent>> weapon_fire_events;
            if (auto naked_fire_event = std::dynamic_pointer_cast<const WeaponFireEvent>(this_event)) {
                weapon_fire_events.push_back(std::move(naked_fire_event));

            } else if (auto weapons_platform = std::dynamic_pointer_cast<const WeaponsPlatformEvent>(this_event)) {
                for (auto more_event : weapons_platform->SubEvents(ALL_EMPIRES)) {
                    if (auto this_attack = std::dynamic_pointer_cast<const WeaponFireEvent>(more_event))
                        weapon_fire_events.push_back(std::move(this_attack));
                }
            }

            // Set attacker as at least basically visible to other empires.
            for (const auto& this_attack : weapon_fire_events) {
                for (auto detector_empire_id : combat_info.empire_ids) {
                    Visibility initial_vis = combat_info.empire_object_visibility[detector_empire_id][this_attack->attacker_id];
                    TraceLogger(combat) << "Pre-attack visibility of attacker id: " << this_attack->attacker_id
                                        << " by empire: " << detector_empire_id << " was: " << initial_vis;

                    if (initial_vis >= Visibility::VIS_BASIC_VISIBILITY)
                        continue;

                    combat_info.empire_object_visibility[detector_empire_id][this_attack->attacker_id] = Visibility::VIS_BASIC_VISIBILITY;

                    DebugLogger(combat) << " ... Setting post-attack visability to " << Visibility::VIS_BASIC_VISIBILITY;

                    // record visibility change event due to attack
                    stealth_change_event->AddEvent(this_attack->attacker_id,
                                                   this_attack->target_id,
                                                   this_attack->attacker_owner_id,
                                                   this_attack->target_owner_id,
                                                   Visibility::VIS_BASIC_VISIBILITY);
                }
            }
        }

        if (!stealth_change_event->AreSubEventsEmpty(ALL_EMPIRES))
            combat_info.combat_events.push_back(std::move(stealth_change_event));

        /// Remove all who died in the bout
        combat_state.CullTheDead(combat_info.bout, bout_event);

        // Backpropagate meters so that next round tests can use the results of the previous round
        for (auto* obj : combat_info.objects.allRaw())
            obj->BackPropagateMeters();
    }
}

void AutoResolveCombat(CombatInfo& combat_info) {
    if (combat_info.objects.empty())
        return;

    auto system = combat_info.objects.get<System>(combat_info.system_id);
    if (!system)
        ErrorLogger() << "AutoResolveCombat couldn't get system with id " << combat_info.system_id;
    else
        DebugLogger() << "AutoResolveCombat at " << system->Name();

    DebugLogger(combat) << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%";
    DebugLogger(combat) << "AutoResolveCombat objects before resolution: " << combat_info.objects.Dump();

    // reasonably unpredictable but reproducible random seeding
    int base_seed = 123454321; // arbitrary number
    if (GetGameRules().Get<bool>("RULE_RESEED_PRNG_SERVER")) {
        base_seed += std::hash<std::string>{}(combat_info.galaxy_setup_data.GetSeed()); // probably not consistent across different platforms, but that's OK for this use
        base_seed += combat_info.objects.allWithIDs().begin()->first + combat_info.turn;
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
        combat_info.bout = bout;
        CombatRound(combat_state);
        last_bout = bout;
    } // end for over combat arounds

    auto launches_event = std::make_shared<FighterLaunchesEvent>();
    combat_info.combat_events.push_back(launches_event);

    RecoverFighters(combat_info, last_bout, launches_event);

    DebugLogger(combat) << "AutoResolveCombat objects after resolution: " << combat_info.objects.Dump();

    DebugLogger(combat) << "combat event log start:";
    const ScriptingContext context{combat_info};
    for (const auto& event : combat_info.combat_events)
        DebugLogger(combat) << event->DebugString(context);
    DebugLogger(combat) << "combat event log end:";
}
