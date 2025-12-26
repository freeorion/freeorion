from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    ContainedBy,
    Contains,
    Destroy,
    EffectsGroup,
    HasSpecial,
    Industry,
    IsAnyObject,
    IsBuilding,
    IsSource,
    Object,
    Planet,
    SetIndustry,
    Source,
    Target,
)
from macros.priorities import (
    CONCENTRATION_CAMP_PRIORITY,
)

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_CONC_CAMP_REMNANT",
    description="BLD_CONC_CAMP_REMNANT_DESC",
    buildcost=1,
    buildtime=1,
    location=~IsAnyObject,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet()),
            activation=(
                ContainedBy(Object(id=Source.PlanetID) & Planet() & ~Contains(IsBuilding(name=["BLD_CONC_CAMP"])))
            ),
            priority=CONCENTRATION_CAMP_PRIORITY,
            effects=[SetIndustry(value=Target.TargetIndustry)],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(
                ContainedBy(
                    Object(id=Source.PlanetID)
                    & ~HasSpecial(name="CONC_CAMP_MASTER_SPECIAL")
                    & ~HasSpecial(name="CONC_CAMP_SLAVE_SPECIAL")
                    & Industry(high=Source.Planet.TargetIndustry + 2)
                )
            ),
            effects=[Destroy],
        ),
    ],
    icon="icons/building/concentration-camp.png",
)
