from buildings.buildings import BORE_POSSIBLE, DO_STARLANE_BORE, SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
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
    name="BLD_STARLANE_BORE",
    description="BLD_STARLANE_BORE_DESC",
    buildcost=(200 + 50 * Target.System.NumStarlanes) * BUILDING_COST_MULTIPLIER,
    buildtime=Target.System.NumStarlanes + 1,
    location=Planet(),
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=Object(id=Source.SystemID),
            activation=~BORE_POSSIBLE,
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_STARLANE_BORE_FAILED",
                    label="EFFECT_STARLANE_BORE_LABEL",
                    icon="icons/tech/n-dimensional_structures.png",
                    parameters={"system": Target.ID},
                    empire=Source.Owner,
                )
            ],
        ),
        EffectsGroup(
            scope=Object(id=Source.SystemID),
            activation=BORE_POSSIBLE,
            effects=[
                DO_STARLANE_BORE,
                GenerateSitRepMessage(
                    message="EFFECT_STARLANE_BORE",
                    label="EFFECT_STARLANE_BORE_LABEL",
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
