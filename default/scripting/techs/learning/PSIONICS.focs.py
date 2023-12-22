from focs._effects import (
    HasTag,
    OwnedBy,
    Planet,
    Ship,
    Source,
    StatisticIf,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_PSIONICS",
    description="LRN_PSIONICS_DESC",
    short_description="POLICY_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=500 * TECH_COST_MULTIPLIER
    - (
        250
        * TECH_COST_MULTIPLIER
        * StatisticIf(
            float,
            # no Source condition here. Empire needs to have a ship or planet with a telepathic species, but it need not be the capital / source object
            condition=(Planet() | Ship) & OwnedBy(empire=Source.Owner) &
            # @content_tag{TELEPATHIC} Decreases research cost of this tech for empires that own any object with this tag
            HasTag(name="TELEPATHIC"),
        )
    ),
    researchturns=4,
    tags=["PEDIA_LEARNING_CATEGORY", "THEORY"],
    prerequisites=["LRN_TRANSLING_THT"],
    unlock=[
        Item(type=UnlockPolicy, name="PLC_INDOCTRINATION"),
        Item(type=UnlockPolicy, name="PLC_CONFORMANCE"),
    ],
    graphic="icons/tech/psionics.png",
)
