#include "CombatDamage.h"

#include "CombatSystem.h"
#include "../util/Logger.h"
#include "../util/GameRules.h"
#include "../universe/Condition.h"
#include "../universe/Enums.h"
#include "../universe/Fighter.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/ShipPart.h"
#include "../universe/ValueRef.h"

// This knows about combat bouts, fighter launches, combatTargets conditions,
// damage calculation (shortrange, fighters, shields)

namespace {
    std::vector<float> WeaponDamageCalcImpl(
        const Ship& ship, bool max, bool include_fighters,
        bool target_ships, const ScriptingContext& context)
    {
        std::vector<float> retval;

        const ShipDesign* design = context.ContextUniverse().GetShipDesign(ship.DesignID());
        if (!design)
            return retval;

        const auto& parts = design->Parts();
        if (parts.empty())
            return retval;

        MeterType METER = max ?
            MeterType::METER_MAX_CAPACITY : MeterType::METER_CAPACITY;
        MeterType SECONDARY_METER = max ?
            MeterType::METER_MAX_SECONDARY_STAT : MeterType::METER_SECONDARY_STAT;

        float fighter_damage = 0.0f;
        int fighter_launch_capacity = 0;
        int available_fighters = 0;

        retval.reserve(parts.size() + 1);
        int num_bouts = GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS");
        // for each weapon part, get its damage meter value
        for (const auto& part_name : parts) {
            const ShipPart* part = GetShipPart(part_name);
            if (!part)
                continue;
            ShipPartClass part_class = part->Class();

            // get the attack power for each weapon part.
            if (part_class == ShipPartClass::PC_DIRECT_WEAPON) {
                if (target_ships)
                    retval.push_back(ship.WeaponPartShipDamage(part, context));
                else
                    retval.push_back(ship.WeaponPartFighterDamage(part, context));
            } else if (part_class == ShipPartClass::PC_FIGHTER_BAY && include_fighters) {
                // launch capacity determined by capacity of bay
                fighter_launch_capacity += static_cast<int>(ship.CurrentPartMeterValue(METER, part_name));

            } else if (part_class == ShipPartClass::PC_FIGHTER_HANGAR && include_fighters) {
                // attack strength of a ship's fighters per bout determined by the hangar...
                // assuming all hangars on a ship are the same part type...
                if (target_ships && part->TotalShipDamage()) {
                    retval.push_back(part->TotalShipDamage()->Eval(context));
                    // as TotalShipDamage contains the damage from all fighters, do not further include fighter
                    include_fighters = false;
                } else if (part->TotalFighterDamage() && !target_ships) {
                    retval.push_back(part->TotalFighterDamage()->Eval(context));
                    // as TotalFighterDamage contains the damage from all fighters, do not further include fighter
                    include_fighters = false;
                } else if (part->CombatTargets() && context.effect_target &&
                           part->CombatTargets()->EvalOne(context, context.effect_target))
                {
                    fighter_damage = ship.CurrentPartMeterValue(SECONDARY_METER, part_name);
                    available_fighters = std::max(0, static_cast<int>(
                        ship.CurrentPartMeterValue(METER, part_name)));  // stacked meter
                } else {
                    // target is not of the right type; no damage; stop checking hangars/launch bays
                    fighter_damage = 0.0f;
                    include_fighters = false;
                }
            }
        }

        if (!include_fighters || fighter_damage <= 0.0f || available_fighters <= 0 || fighter_launch_capacity <= 0)
            return retval;

        // Calculate fighter damage manually if not
        int fighter_shots = std::min(available_fighters, fighter_launch_capacity);  // how many fighters launched in bout 1
        available_fighters -= fighter_shots;
        int launched_fighters = fighter_shots;
        int remaining_bouts = num_bouts - 2;  // no attack for first round, second round already added
        while (remaining_bouts > 0) {
            int fighters_launched_this_bout = std::min(available_fighters, fighter_launch_capacity);
            available_fighters -= fighters_launched_this_bout;
            launched_fighters += fighters_launched_this_bout;
            fighter_shots += launched_fighters;
            --remaining_bouts;
        }

        // how much damage does a fighter shot do?
        fighter_damage = std::max(0.0f, fighter_damage);

        if (target_ships)
            retval.push_back(fighter_damage * fighter_shots);
        else
            retval.push_back(static_cast<float>(fighter_shots));
        return retval;
    }

    Ship TempShipForDamageCalcs(const Ship& template_ship, const ScriptingContext& context) {
        // create temporary ship to test targetting condition on...
        static constexpr float shields = 0.0f;
        static constexpr float structure = 100.0f;

        if (template_ship.DesignID() == INVALID_DESIGN_ID)
            ErrorLogger() << "TempShipForDamageCalcs passed template ship with no known design ID";

        // use the given design and species as default enemy.
        Ship target{ALL_EMPIRES, template_ship.DesignID(), template_ship.SpeciesName(),
                    context.ContextUniverse(), context.species, ALL_EMPIRES, context.current_turn};

        // target needs to have an ID != -1 to be visible, stealth should be low enough
        // structure must be higher than zero to be valid target
        target.SetID(TEMPORARY_OBJECT_ID); // also see InsertTemp function usage
        target.GetMeter(MeterType::METER_STRUCTURE)->Set(structure, structure);
        target.GetMeter(MeterType::METER_MAX_STRUCTURE)->Set(structure, structure);
        // Shield value is used for structural damage estimation
        target.GetMeter(MeterType::METER_SHIELD)->Set(shields, shields);

        return target;
    }

    Fighter TempFighterForDamageCalcs(const Ship& template_ship, const ScriptingContext& context) {
        static constexpr auto combat_targets = nullptr;
        Fighter target{ALL_EMPIRES, template_ship.ID(), template_ship.SpeciesName(), 1.0f, combat_targets};
        target.SetID(TEMPORARY_OBJECT_ID);
        return target;
    }
}

std::vector<float> Combat::WeaponDamageImpl(
    const ScriptingContext& context, const Ship& source, float shields,
    bool max, bool launch_fighters, bool target_ships)
{
    if (source.DesignID() == INVALID_DESIGN_ID) {
        if (source.ID() == TEMPORARY_OBJECT_ID) {
            ErrorLogger() << "Combat::WeaponDamageImpl passed TEMPORARY source ship without a valid design ID: " << source.Dump();
        } else {
            ErrorLogger() << "Combat::WeaponDamageImpl passed source ship without a valid design ID: " << source.Dump();
        }
        return {};
    }

    const Universe::EmpireObjectVisibilityMap empire_object_vis{
        {source.Owner(), {{TEMPORARY_OBJECT_ID, Visibility::VIS_FULL_VISIBILITY}}}};
    const Universe::EmpireObjectVisibilityTurnMap empire_object_visibility_turns{
        {source.Owner(), {{TEMPORARY_OBJECT_ID, {{Visibility::VIS_FULL_VISIBILITY, context.current_turn}}}}}};

    if (target_ships) {
        auto temp_ship = TempShipForDamageCalcs(source, context);
        const ScriptingContext temp_ship_context{context, empire_object_vis, empire_object_visibility_turns,
                                                 ScriptingContext::Source{}, &source,
                                                 ScriptingContext::Target{}, &temp_ship};

        return WeaponDamageCalcImpl(source, max, launch_fighters, target_ships, temp_ship_context);

    } else {
        // create temporary fighter to test targetting condition on...
        auto temp_fighter = TempFighterForDamageCalcs(source, context);
        const ScriptingContext temp_fighter_context{context, empire_object_vis, empire_object_visibility_turns,
                                                    ScriptingContext::Source{}, &source,
                                                    ScriptingContext::Target{}, &temp_fighter};

        return WeaponDamageCalcImpl(source, max, launch_fighters, target_ships, temp_fighter_context);
    }
}


/** Populate fighter state quantities and damages for each combat round until @p limit_to_bout */
std::map<int, Combat::FighterBoutInfo> Combat::ResolveFighterBouts(
    const ScriptingContext& context, std::shared_ptr<const Ship> ship,
    const Condition::Condition* combat_targets, int bay_capacity,
    int current_docked, float fighter_damage, int limit_to_bout)
{
    std::map<int, FighterBoutInfo> retval;
    if (!ship)
        return retval;
    const int NUM_BOUTS = GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS");
    int target_bout = limit_to_bout < 1 ? NUM_BOUTS : limit_to_bout;

    Universe::EmpireObjectVisibilityMap empire_object_vis{
        {ship->Owner(), {{TEMPORARY_OBJECT_ID, Visibility::VIS_FULL_VISIBILITY}}}};
    Universe::EmpireObjectVisibilityTurnMap empire_object_visibility_turns{
        {ship->Owner(), {{TEMPORARY_OBJECT_ID, {{Visibility::VIS_FULL_VISIBILITY, context.current_turn}}}}}};
    auto temp_ship = TempShipForDamageCalcs(*ship, context);
    ScriptingContext ship_target_context{context, empire_object_vis, empire_object_visibility_turns,
                                         ScriptingContext::Source{}, ship.get(),
                                         ScriptingContext::Target{}, &temp_ship};

    for (int bout = 1; bout <= target_bout; ++bout) {
        ship_target_context.combat_bout = bout;
        // init current fighters
        if (bout == 1) {
            retval[bout].docked = current_docked;
            retval[bout].attacking = 0;
            retval[bout].launched = 0;
        } else {
            retval[bout] = retval[bout - 1];
            retval[bout].docked = retval[bout - 1].docked;
            retval[bout].attacking = retval[bout - 1].attacking + retval[bout].launched;
            retval[bout].launched = 0;
            retval[bout].total_damage = retval[bout - 1].total_damage;
        }
        // calc damage this bout, apply to total
        int shots_this_bout = retval[bout].attacking;
        if (combat_targets && !combat_targets->EvalOne(ship_target_context, ship_target_context.effect_target))
            shots_this_bout = 0;
        retval[bout].damage = shots_this_bout * fighter_damage;
        retval[bout].total_damage += retval[bout].damage;
        // launch fighters
        if (bout < NUM_BOUTS) {
            retval[bout].launched = std::min(bay_capacity, retval[bout].docked);
            retval[bout].docked -= retval[bout].launched;
        }
    }
    return retval;
}

/** Returns max number of shots a carrier's fighters in a battle. Evals @p sampling_condition for each bout vs @p context */
int Combat::TotalFighterShots(const ScriptingContext& context, const Ship& ship,
                              const Condition::Condition* sampling_condition)
{
    // Iterate over context, but change bout number
    ScriptingContext mut_context{context};
    const int launch_capacity = static_cast<int>(ship.SumCurrentPartMeterValuesForPartClass(MeterType::METER_CAPACITY, ShipPartClass::PC_FIGHTER_BAY, context.ContextUniverse()));
    int hangar_fighters = static_cast<int>(ship.SumCurrentPartMeterValuesForPartClass(MeterType::METER_CAPACITY, ShipPartClass::PC_FIGHTER_HANGAR, context.ContextUniverse()));
    int launched_fighters = 0;
    int shots_total = 0;

    for (int bout = 1; bout <= GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS"); ++bout) {
        mut_context.combat_bout = bout;
        const int launch_this_bout = std::min(launch_capacity, hangar_fighters);
        int shots_this_bout = launched_fighters;
        if (sampling_condition && launched_fighters > 0 && !sampling_condition->EvalAny(mut_context))
            shots_this_bout = 0;
        shots_total += shots_this_bout;
        launched_fighters += launch_this_bout;
        hangar_fighters -= launch_this_bout;
    }

    return shots_total;
}
