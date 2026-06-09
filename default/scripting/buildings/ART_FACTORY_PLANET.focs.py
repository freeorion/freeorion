from focs._buildings import BuildingType
from focs._conditions import Contains, IsBuilding, IsSource, Object, OwnedBy, OwnerHasTech, Planet, Population
from focs._effects_new import Destroy, EffectsGroup, GenerateSitRepMessage, SetPlanetType, SetPopulation, SetSpecies
from focs._enums import AsteroidsType, Barren, GasGiantType
from focs._sources import Source, Target
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_ARTIFICIAL_PLANET_EXCLUSION
from macros.priorities import POPULATION_OVERRIDE_PRIORITY
from macros.upkeep import COLONY_UPKEEP_MULTIPLICATOR

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_ART_FACTORY_PLANET",
    description="BLD_ART_FACTORY_PLANET_DESC",
    buildcost=200 * Target.HabitableSize + (70 * COLONY_UPKEEP_MULTIPLICATOR * BUILDING_COST_MULTIPLIER),
    buildtime=12,
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_ART_PLANET", "BLD_ART_FACTORY_PLANET", "BLD_ART_PARADISE_PLANET"]))
        & OwnedBy(empire=Source.Owner)
        & Planet(type=[AsteroidsType, GasGiantType])
        & OwnerHasTech(name="PRO_EXOBOTS")
        & Population(high=0)
    ),
    enqueuelocation=ENQUEUE_ARTIFICIAL_PLANET_EXCLUSION,
    effectsgroups=[
        EffectsGroup(
            scope=Object(id=Source.PlanetID) & Planet(),
            priority=POPULATION_OVERRIDE_PRIORITY,
            effects=[
                SetPlanetType(type=Barren),
                SetSpecies(name="SP_EXOBOT"),
                SetPopulation(value=1),
                GenerateSitRepMessage(
                    message="EFFECT_ART_PLANET",
                    label="EFFECT_ART_PLANET_LABEL",
                    icon="icons/species/robotic-01.png",
                    parameters={
                        "planet": Target.ID,
                    },
                    empire=Source.Owner,
                ),
            ],
        ),
        EffectsGroup(scope=IsSource, effects=Destroy),
    ],
    icon="icons/species/robotic-01.png",
)
