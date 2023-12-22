from focs._effects import (
    EffectsGroup,
    InSystem,
    LocalCandidate,
    OwnedBy,
    SetStructure,
    Ship,
    Source,
    Structure,
    Turn,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.misc import SHIP_STRUCTURE_FACTOR

Tech(
    name="SHP_BASIC_DAM_CONT",
    description="SHP_BASIC_DAM_CONT_DESC",
    short_description="STRUCTURE_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=80 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_DAMAGE_CONTROL_PART_TECHS"],
    prerequisites=["SHP_MIL_ROBO_CONT"],
    unlock=Item(type=UnlockPolicy, name="PLC_ENGINEERING"),
    effectsgroups=[
        EffectsGroup(
            scope=Ship
            & OwnedBy(empire=Source.Owner)
            & (~InSystem() | InSystem() & Turn(low=LocalCandidate.System.LastTurnBattleHere + 1))
            & Structure(high=LocalCandidate.MaxStructure - 0.001),
            effects=SetStructure(value=Value + SHIP_STRUCTURE_FACTOR),
        )
    ],
)
