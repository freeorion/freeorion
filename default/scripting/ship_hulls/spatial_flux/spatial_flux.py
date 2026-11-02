from focs._conditions import Aggressive, IsSource, OwnerHasTech
from focs._effects import EffectsGroup, SetStealth
from focs._sources import Source
from focs._value_refs import CurrentTurn, NamedRealLookup, StatisticIf, Value

SPATIAL_FLUX_STEALTH_EFFECTS = [
    EffectsGroup(
        scope=IsSource,
        activation=(Source.ArrivedOnTurn >= CurrentTurn - 1),
        accountinglabel="SPATIAL_FLUX_MALUS",
        effects=SetStealth(value=Value - NamedRealLookup(name="SPATIAL_FLUX_STEALTH_ARRIVAL_MALUS")),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=~Aggressive,
        accountinglabel="SPATIAL_FLUX_BONUS",
        effects=SetStealth(
            value=Value
            + NamedRealLookup(name="SPATIAL_FLUX_STEALTH_NON_AGGRESSIVE_BONUS")
            + NamedRealLookup(name="SPATIAL_FLUX_STEALTH_TECH_BONUS")
            * (
                StatisticIf(float, condition=IsSource & OwnerHasTech(name="SPY_STEALTH_PART_1"))
                + StatisticIf(float, condition=IsSource & OwnerHasTech(name="SPY_STEALTH_PART_2"))
                + StatisticIf(float, condition=IsSource & OwnerHasTech(name="SPY_STEALTH_PART_3"))
                + StatisticIf(float, condition=IsSource & OwnerHasTech(name="SPY_STEALTH_4"))
            ),
        ),
    ),
]
