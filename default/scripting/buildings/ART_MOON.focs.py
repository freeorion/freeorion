from focs._effects import (
    AddSpecial,
    AsteroidsType,
    Contains,
    Destroy,
    EffectsGroup,
    HasSpecial,
    IsBuilding,
    IsSource,
    Object,
    OwnedBy,
    Planet,
    Source,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # type: ignore[reportUnboundVariable]
    name="BLD_ART_MOON",
    description="BLD_ART_MOON_DESC",
    buildcost=250 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_ART_MOON"]))
        & OwnedBy(empire=Source.Owner)
        & ~Planet(type=[AsteroidsType])
        & ~HasSpecial(name="RESONANT_MOON_SPECIAL")
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        EffectsGroup(
            scope=Object(id=Source.PlanetID) & Planet(),
            effects=AddSpecial(name="RESONANT_MOON_SPECIAL"),
        ),
        EffectsGroup(scope=IsSource, effects=Destroy),
    ],
    icon="icons/specials_huge/resonant_moon.png",
)
