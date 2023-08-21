# Increase the fuel capacity of ships with FU_BASIC_TANK parts.
# Adds the given increase amount times the number of FU_BASIC_TANK parts in the design to the max fuel meter.
# @1@ Accounting label to present this effect with
# @2@ Fuel capacity increase per fuel part
from focs._effects import (
    DesignHasPart,
    EffectsGroup,
    NamedReal,
    OwnedBy,
    PartsInShipDesign,
    SetMaxFuel,
    Ship,
    Source,
    Target,
    Value,
)


def PART_UPGRADE_MAXFUEL_EFFECTS(effect_name: str, value: float):
    return EffectsGroup(
        scope=Ship & OwnedBy(empire=Source.Owner) & DesignHasPart(name="FU_BASIC_TANK"),
        accountinglabel=effect_name,
        effects=[
            SetMaxFuel(
                value=Value
                + NamedReal(name=effect_name + "_MULT", value=value)
                * PartsInShipDesign(name="FU_BASIC_TANK", design=Target.DesignID)
            )
        ],
    )
