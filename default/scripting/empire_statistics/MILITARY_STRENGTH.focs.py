from focs._effects import LocalCandidate, MaxOf, OwnedBy, Ship, Source, Statistic, Sum
from focs._empire_statistics import EmpireStatistic
from macros.misc import FIGHTER_DAMAGE_FACTOR

EmpireStatistic(
    name="MILITARY_STRENGTH_STAT",
    # Damage prevention by shields: each shield is hit twice and a half times by non-shield-piercing weapons.
    #   Or often it prevents a third of the damage (going against similar tech level direct weapons)
    #   as a shield usefulness scales with structure, we "split" the effect into a relative and an absolute part
    # Damage prevention by shooting down fighters: each part shot prevents 2/3 of a hit by a laser fighter (4+2 base damage) in bout two.
    #      With four bouts a bomber destructed in bout two prevents 2 bomber shots, but flak and interceptors are usually idle 2/3 of their total destruction estimation so that evens out
    #      Usefulness of most anti-fighter weapons scales with the number of enemy fighters
    # Note that weapons targeting fighters and ships will be counted twice (once for doing structural damage, once for preventing fighter damage)
    value=Statistic(
        float, Sum, value=LocalCandidate.DamageStructurePerBattleMax, condition=Ship & OwnedBy(empire=Source.Owner)
    )
    * Statistic(
        float,
        Sum,
        value=(
            LocalCandidate.Structure
            + (FIGHTER_DAMAGE_FACTOR * 4 * LocalCandidate.DestroyFightersPerBattleMax)
            + (1.5 * LocalCandidate.MaxShield)
            + (MaxOf(float, LocalCandidate.MaxShield, 0.0) * LocalCandidate.Structure / 4)
        ),
        condition=Ship & OwnedBy(empire=Source.Owner),
    ),
)
