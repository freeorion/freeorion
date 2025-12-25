from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    BuildBuilding,
    Capital,
    EffectsGroup,
    Enqueued,
    IsBuilding,
    LocalCandidate,
    Number,
    Object,
    OwnedBy,
    Planet,
    SetMaxStockpile,
    Source,
    Statistic,
    Sum,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_STOCKPILING_CENTER",
    description="BLD_STOCKPILING_CENTER_DESC",
    captureresult=DestroyOnCapture,  # pyrefly: ignore[unbound-name]
    buildcost=100 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    location=(
        Capital
        & OwnedBy(empire=Source.Owner)
        & Number(high=0, condition=IsBuilding(name=["BLD_STOCKPILING_CENTER"]) & OwnedBy(empire=Source.Owner))
    ),
    enqueuelocation=(
        Capital
        & OwnedBy(empire=Source.Owner)
        & Number(high=0, condition=IsBuilding(name=["BLD_STOCKPILING_CENTER"]) & OwnedBy(empire=Source.Owner))
        # can't enqueue if already have an enqueued stockpiling_center anywhere
        & Number(
            high=0,
            condition=(Planet() & Enqueued(type=BuildBuilding, name="BLD_STOCKPILING_CENTER", empire=Source.Owner)),
        )
    ),
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet()),
            accountinglabel="BLD_STOCKPILING_CENTER_LABEL",
            effects=[
                SetMaxStockpile(
                    value=Value
                    + (
                        0.1
                        * Statistic(
                            float, Sum, value=LocalCandidate.Industry, condition=Planet() & OwnedBy(empire=Source.Owner)
                        )
                    )
                )
            ],
        ),
    ],
    icon="icons/building/stockpiling_center.png",
)
