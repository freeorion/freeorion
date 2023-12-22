from focs._effects import (
    EffectsGroup,
    NamedRealLookup,
    OwnedBy,
    SetMaxStructure,
    Ship,
    Source,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_REINFORCED_HULL",
    description="SHP_REINFORCED_HULL_DESC",
    short_description="STRUCTURE_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=72 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_DAMAGE_CONTROL_PART_TECHS"],
    prerequisites=["CON_ARCH_MONOFILS"],
    effectsgroups=[
        EffectsGroup(
            scope=Ship & OwnedBy(empire=Source.Owner),
            effects=SetMaxStructure(value=Value + NamedRealLookup(name="SHP_REINFORCED_HULL_BONUS")),
        )
    ],
    graphic="icons/tech/structural_integrity_fields.png",
)
