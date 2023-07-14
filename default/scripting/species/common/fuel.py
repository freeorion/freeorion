# usually each "level" of fuel allows two jumps fuel
# (i.e. GREAT_FUEL is similar to GREAT_SUPPLY)
from common.priorities import TARGET_AFTER_SCALING_PRIORITY
from focs._effects import EffectsGroup, IsSource, SetMaxFuel, Ship, Value


def FUEL_EFFECTSGROUP(label: str, fuel_change: float):
    return EffectsGroup(
        description=f"{label}_FUEL_DESC",
        scope=IsSource & Ship,
        accountinglabel="@1@_FUEL_LABEL",
        priority=TARGET_AFTER_SCALING_PRIORITY,
        effects=SetMaxFuel(value=Value + fuel_change),
    )


NO_FUEL = [
    FUEL_EFFECTSGROUP("NO", 0),
]
BAD_FUEL = [
    FUEL_EFFECTSGROUP("BAD", -0.5),
]
GOOD_FUEL = [
    FUEL_EFFECTSGROUP("GOOD", 0.5),
]
AVERAGE_FUEL = []
GREAT_FUEL = [
    FUEL_EFFECTSGROUP("GREAT", 1),
]
ULTIMATE_FUEL = [
    FUEL_EFFECTSGROUP("ULTIMATE", 1.5),
]
