from focs._buildings import BuildingType
from focs._conditions import Contains, IsBuilding, IsSource, Object, OwnedBy, OwnerHasTech, Planet, Population
from focs._effects_new import AddSpecial, Destroy, EffectsGroup, GenerateSitRepMessage, SetPlanetType
from focs._enums import AsteroidsType, Barren, GasGiantType
from focs._sources import Source, Target
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_ARTIFICIAL_PLANET_EXCLUSION

try:
    pass
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_ART_PARADISE_PLANET",
    description="BLD_ART_PARADISE_PLANET_DESC",
    buildcost=(200 * Target.HabitableSize + 300) * BUILDING_COST_MULTIPLIER,
    buildtime=10,
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_ART_PLANET", "BLD_ART_FACTORY_PLANET", "BLD_ART_PARADISE_PLANET"]))
        & OwnedBy(empire=Source.Owner)
        & Planet(type=[AsteroidsType, GasGiantType])
        & OwnerHasTech(name="GRO_GAIA_TRANS")
        & Population(high=0)
    ),
    enqueuelocation=ENQUEUE_ARTIFICIAL_PLANET_EXCLUSION,
    effectsgroups=[
        EffectsGroup(
            scope=Object(id=Source.PlanetID) & Planet(),
            effects=[
                SetPlanetType(type=Barren),
                AddSpecial(name="GAIA_SPECIAL"),
                GenerateSitRepMessage(
                    message="EFFECT_ART_PLANET",
                    label="EFFECT_ART_PLANET_LABEL",
                    icon="icons/specials_large/gaia.png",
                    parameters={"planet": Target.ID},
                    empire=Source.Owner,
                ),
            ],
        ),
        EffectsGroup(scope=IsSource, effects=Destroy),
    ],
    icon="icons/building/paradise_planet.png",
)
