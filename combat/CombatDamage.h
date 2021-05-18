#ifndef _CombatDamage_h_
#define _CombatDamage_h_

#include "../universe/Condition.h"
#include "../universe/EnumsFwd.h"
#include "../universe/Ship.h"

namespace Combat {
    /** Container for return values of ResolveFighterBouts */
    struct FighterBoutInfo {
        struct StateQty {
            int         docked;
            int         attacking;
            int         launched;       // transitioning between docked and attacking
        };
        float           damage;
        float           total_damage;   // damage from all previous bouts, including this bout
        StateQty        qty;
    };

    /** Populate fighter state quantities and damages for each combat round until @p limit_to_bout */
    FO_COMMON_API std::map<int, FighterBoutInfo> ResolveFighterBouts(std::shared_ptr<const Ship> ship,
                                                                     const Condition::Condition* combat_targets,
                                                                     int bay_capacity, int current_docked,
                                                                     float fighter_damage, int limit_to_bout = -1);

    FO_COMMON_API int TotalFighterShots(const ScriptingContext& context, const Ship& ship, const Condition::Condition* sampling_condition);

    FO_COMMON_API std::vector<float> WeaponDamageImpl(std::shared_ptr<const Ship> ship, float target_shields, bool use_max_meters,
                                                      bool launch_fighters, bool target_ships = true);


}

#endif
