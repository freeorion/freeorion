# usually each "level" of fuel allows two jumps fuel
# (i.e. GREAT_FUEL is similar to GREAT_SUPPLY)
from focs._effects import EffectsGroup, IsSource, SetMaxFuel, Ship, Value
from macros.priorities import TARGET_AFTER_SCALING_PRIORITY


def _fuel(label: str, fuel_change: float):
    return EffectsGroup(
        description=f"{label}_FUEL_DESC",
        scope=IsSource & Ship,
        accountinglabel=f"{label}_FUEL_LABEL",
        priority=TARGET_AFTER_SCALING_PRIORITY,
        effects=SetMaxFuel(value=fuel_change),
    )


NO_FUEL = [
    _fuel("NO", Value + 0),
]
BAD_FUEL = [
    _fuel("BAD", Value - 0.5),
]
GOOD_FUEL = [
    _fuel("GOOD", Value + 0.5),
]
AVERAGE_FUEL = []
GREAT_FUEL = [
    _fuel("GREAT", Value + 1),
]
ULTIMATE_FUEL = [
    _fuel("ULTIMATE", Value + 1.5),
]
