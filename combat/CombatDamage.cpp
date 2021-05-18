#include "CombatDamage.h"

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
    ScriptingContext CombatContextWithShipTarget(std::shared_ptr<const Ship> source, float shields = 0.0f)
    {
        // use the given design and species as default enemy. at least those least exists
        auto target = std::make_shared<Ship>(ALL_EMPIRES, source->DesignID(), source->SpeciesName());
        // target needs to have an ID != -1 to be visible, stealth should be low enough
        // structure must be higher than zero to be valid target
        target->SetID(-1000000); // XXX magic number AutoresolveInfo.next_fighter_id starts at -1000001 counting down
        target->GetMeter(MeterType::METER_STRUCTURE)->Set(100.0f, 100.0f);
        target->GetMeter(MeterType::METER_MAX_STRUCTURE)->Set(100.0f, 100.0f);
        // Shield value is used for structural damage estimation
        target->GetMeter(MeterType::METER_SHIELD)->Set(shields, shields);
        GetUniverse().SetEmpireObjectVisibility(source->Owner(), target->ID(), Visibility::VIS_FULL_VISIBILITY);
        return ScriptingContext{source, target};
    }

    ScriptingContext CombatContextWithFighterTarget(std::shared_ptr<const Ship> source)
    {
        auto target = std::make_shared<Fighter>();
        target->SetID(-1000000); // XXX magic number AutoresolveInfo.next_fighter_id starts at -1000001 counting down
        GetUniverse().SetEmpireObjectVisibility(source->Owner(), target->ID(), Visibility::VIS_FULL_VISIBILITY);
        return ScriptingContext{source, target};
    }

    std::vector<float> WeaponDamageCalcImpl(std::shared_ptr<const Ship> ship, bool max, bool include_fighters,
                                            bool target_ships, const ScriptingContext&& context)
    {
        std::vector<float> retval;
        if (!ship)
            return retval;

        const ShipDesign* design = ship->Design();
        if (!design)
            return retval;

        const std::vector<std::string>& parts = design->Parts();
        if (parts.empty())
            return retval;

        MeterType METER = max ? MeterType::METER_MAX_CAPACITY : MeterType::METER_CAPACITY;
        MeterType SECONDARY_METER = max ? MeterType::METER_MAX_SECONDARY_STAT : MeterType::METER_SECONDARY_STAT;

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
                    retval.push_back(ship->WeaponPartShipDamage(part, context));
                else
                    retval.push_back(ship->WeaponPartFighterDamage(part, context));
            } else if (part_class == ShipPartClass::PC_FIGHTER_BAY && include_fighters) {
                // launch capacity determined by capacity of bay
                fighter_launch_capacity += static_cast<int>(ship->CurrentPartMeterValue(METER, part_name));

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
                } else if (part->CombatTargets() && context.effect_target && part->CombatTargets()->Eval(context, context.effect_target)) {
                    fighter_damage = ship->CurrentPartMeterValue(SECONDARY_METER, part_name);
                    available_fighters = std::max(0, static_cast<int>(ship->CurrentPartMeterValue(METER, part_name)));  // stacked meter
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
            retval.push_back(fighter_shots);
        return retval;
    }
}

std::vector<float> Combat::WeaponDamageImpl(std::shared_ptr<const Ship> source, float shields,
                                            bool max, bool launch_fighters, bool target_ships)
{
    if (target_ships)
        return WeaponDamageCalcImpl(source, max, launch_fighters, target_ships, CombatContextWithShipTarget(source, shields));
    else
        return WeaponDamageCalcImpl(source, max, launch_fighters, target_ships, CombatContextWithFighterTarget(source));
}


/** Populate fighter state quantities and damages for each combat round until @p limit_to_bout */
std::map<int, Combat::FighterBoutInfo> Combat::ResolveFighterBouts(std::shared_ptr<const Ship> ship,
                                                           const Condition::Condition* combat_targets,
                                                   int bay_capacity, int current_docked,
                                                   float fighter_damage, int limit_to_bout)
{
        // FIXME this should be moved to combat probably
        std::map<int, FighterBoutInfo> retval;
        const int NUM_BOUTS = GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS");
        int target_bout = limit_to_bout < 1 ? NUM_BOUTS : limit_to_bout;

        ScriptingContext context = CombatContextWithShipTarget(ship);

        for (int bout = 1; bout <= target_bout; ++bout) {
            context.combat_bout = bout;
            // init current fighters
            if (bout == 1) {
                retval[bout].qty.docked = current_docked;
                retval[bout].qty.attacking = 0;
                retval[bout].qty.launched = 0;
            } else {
                retval[bout].qty = retval[bout - 1].qty;
                retval[bout].qty.attacking += retval[bout].qty.launched;
                retval[bout].qty.launched = 0;
                retval[bout].total_damage = retval[bout - 1].total_damage;
            }
            // calc damage this bout, apply to total
            int shots_this_bout = retval[bout].qty.attacking;
            if (combat_targets && !combat_targets->Eval(context, context.effect_target)) {
                shots_this_bout = 0;
            }
            retval[bout].damage = shots_this_bout * fighter_damage;
            retval[bout].total_damage += retval[bout].damage;
            // launch fighters
            if (bout < NUM_BOUTS) {
                retval[bout].qty.launched = std::min(bay_capacity, retval[bout].qty.docked);
                retval[bout].qty.docked -= retval[bout].qty.launched;
            }
        }
        return retval;
}

int Combat::TotalFighterShots(const ScriptingContext& context, const Ship& ship, const Condition::Condition* sampling_condition)
{

    // Iterate over context, but change bout number
    // XXX probably should rather take ship ID as argument as well
    // XXX else i get a fighter and have to fetch me the launching ship from that
    ScriptingContext mut_context(context);
    int launch_capacity = ship.SumCurrentPartMeterValuesForPartClass(MeterType::METER_CAPACITY, ShipPartClass::PC_FIGHTER_BAY);
    int hangar_fighters = ship.SumCurrentPartMeterValuesForPartClass(MeterType::METER_CAPACITY, ShipPartClass::PC_FIGHTER_HANGAR);
    int launched_fighters = 0;
    int shots_total = 0;
    Condition::ObjectSet condition_matches;

    for (int bout = 1; bout <= GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS"); ++bout) {
        mut_context.combat_bout = bout;
        int launch_this_bout = std::min(launch_capacity,hangar_fighters);
        int shots_this_bout = launched_fighters;
        if (sampling_condition && launched_fighters > 0) {
            // check if not shooting
            condition_matches.clear();
            sampling_condition->Eval(mut_context, condition_matches);
            if (condition_matches.size() == 0) {
                shots_this_bout = 0;
            }
        }
        shots_total += shots_this_bout;
        launched_fighters += launch_this_bout;
        hangar_fighters -= launch_this_bout;
    }

    return shots_total;
}
