#ifndef _CombatDamage_h_
#define _CombatDamage_h_

#include "../universe/Condition.h"
#include "../universe/EnumsFwd.h"
#include "../universe/Ship.h"

namespace Combat {
    /** Container for return values of ResolveFighterBouts */
    struct FighterBoutInfo {
        float damage       = 0.0f;
        float total_damage = 0.0f; // damage from all previous bouts, including this bout
        int docked    = 0;
        int attacking = 0;
        int launched  = 0; // transitioning between docked and attacking
    };

    /** Populate fighter state quantities and damages for each combat round until @p limit_to_bout */
    [[nodiscard]] FO_COMMON_API std::map<int, FighterBoutInfo> ResolveFighterBouts(
        const ScriptingContext& context, std::shared_ptr<const Ship> ship,
        const Condition::Condition* combat_targets, int bay_capacity,
        int current_docked, float fighter_damage, int limit_to_bout = -1);

    /** Returns the maximum number of shots the fighters launched by carrier @p ship shoot in a battle.
     * If a @p sampling_condition is given, shots are counted for a bout iff the @p sampling_condition
     * evals to true in the given @p context.
     * Note that a copy of the context modified with current combat bout is used so the
     * @p sampling_condition can consider the bout */
    [[nodiscard]] FO_COMMON_API int TotalFighterShots(const ScriptingContext& context, const Ship& ship,
                                                      const Condition::Condition* sampling_condition);

    [[nodiscard]] FO_COMMON_API std::vector<float> WeaponDamageImpl(
        const ScriptingContext& context,
        const Ship& ship, float target_shields,
        bool use_max_meters, bool launch_fighters, bool target_ships = true);
}

#endif
