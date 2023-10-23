# usually each "level" of fuel allows two jumps fuel
# (i.e. GREAT_FUEL is similar to GREAT_SUPPLY)
from common.priorities import TARGET_AFTER_SCALING_PRIORITY
from focs._effects import EffectsGroup, IsSource, SetMaxFuel, Ship, Value


def FUEL_EFFECTSGROUP(label: str, fuel_change: float):
    return EffectsGroup(
        description=f"{label}_FUEL_DESC",
        scope=IsSource & Ship,
        accountinglabel=f"{label}_FUEL_LABEL",
        priority=TARGET_AFTER_SCALING_PRIORITY,
        effects=SetMaxFuel(value=fuel_change),
    )


NO_FUEL = [
    FUEL_EFFECTSGROUP("NO", Value + 0),
]
BAD_FUEL = [
    FUEL_EFFECTSGROUP("BAD", Value - 0.5),
]
GOOD_FUEL = [
    FUEL_EFFECTSGROUP("GOOD", Value + 0.5),
]
AVERAGE_FUEL = []
GREAT_FUEL = [
    FUEL_EFFECTSGROUP("GREAT", Value + 1),
]
ULTIMATE_FUEL = [
    FUEL_EFFECTSGROUP("ULTIMATE", Value + 1.5),
]
