from buildings.buildings_macros import (
    BORE_POSSIBLE,
    DO_STARLANE_BORE,
    SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
)
from focs._effects import (
    Destroy,
    EffectsGroup,
    GenerateSitRepMessage,
    IsSource,
    Object,
    Planet,
    Source,
    Target,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # type: ignore[reportUnboundVariable]
    name="BLD_STARLANE_NEXUS",
    description="BLD_STARLANE_NEXUS_DESC",
    buildcost=1000 * BUILDING_COST_MULTIPLIER,
    buildtime=8,
    location=(Planet()),
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=Object(id=Source.SystemID),
            activation=(~BORE_POSSIBLE),
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_STARLANE_NEXUS_FAILED",
                    label="EFFECT_STARLANE_NEXUS_LABEL",
                    icon="icons/tech/n-dimensional_structures.png",
                    parameters={"system": Target.ID},
                    empire=Source.Owner,
                ),
            ],
        ),
        EffectsGroup(
            scope=Object(id=Source.SystemID),
            activation=(BORE_POSSIBLE),
            # // Maximum is 11 , since they must be at least 30 degrees apart
            # // (very theoretical 12, if it triggers in the experimentor system while it has no links)
            effects=[
                *([DO_STARLANE_BORE] * 12),
                GenerateSitRepMessage(
                    message="EFFECT_STARLANE_NEXUS",
                    label="EFFECT_STARLANE_NEXUS_LABEL",
                    icon="icons/tech/n-dimensional_structures.png",
                    parameters={"system": Target.ID},
                    empire=Source.Owner,
                ),
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            effects=[Destroy],
        ),
    ],
    icon="icons/tech/n-dimensional_structures.png",
)
