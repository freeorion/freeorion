from focs._effects import (
    Destroy,
    EffectsGroup,
    GameRule,
    HasSpecies,
    IsSource,
    MaxOf,
    Object,
    Planet,
    SetPopulation,
    SetSpecies,
    Source,
    Target,
)
from macros.priorities import POPULATION_OVERRIDE_PRIORITY

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_SUPER_TEST",
    description="BLD_SUPER_TEST_DESC",
    buildcost=1,
    buildtime=1,
    location=(Planet() & (GameRule(type=int, name="RULE_ENABLE_SUPER_TESTER") > 0)),
    effectsgroups=[
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet()),
            priority=POPULATION_OVERRIDE_PRIORITY,
            effects=[
                SetSpecies(name="SP_SUPER_TEST"),
                SetPopulation(value=MaxOf(float, Target.Population, 1)),
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(HasSpecies(name=["SP_SUPER_TEST"])),
            effects=[Destroy],
        ),
    ],
    icon="icons/species/other-04.png",
)
