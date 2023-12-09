from focs._effects import (
    EffectsGroup,
    InSystem,
    LocalCandidate,
    OwnedBy,
    SetStructure,
    Ship,
    Source,
    Stationary,
    Target,
    Turn,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_ADV_DAM_CONT",
    description="SHP_ADV_DAM_CONT_DESC",
    short_description="STRUCTURE_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=160 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_DAMAGE_CONTROL_PART_TECHS"],
    prerequisites=["SHP_FLEET_REPAIR"],
    effectsgroups=[
        EffectsGroup(
            scope=Ship
            & OwnedBy(empire=Source.Owner)
            & InSystem()
            & Stationary
            & Turn(low=LocalCandidate.System.LastTurnBattleHere + 1),
            effects=SetStructure(value=Value + (Target.MaxStructure / 10)),
        )
    ],
)
