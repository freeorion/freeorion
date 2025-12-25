from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    AddSpecial,
    ContainedBy,
    Contains,
    Destroy,
    EffectsGroup,
    EmpireHasAdoptedPolicy,
    GenerateSitRepMessage,
    HasSpecial,
    IsBuilding,
    IsSource,
    MinOf,
    NamedReal,
    Object,
    OwnedBy,
    Planet,
    Population,
    ProducedByEmpire,
    SetIndustry,
    SetPopulation,
    SetSpeciesOpinion,
    SetTargetIndustry,
    Source,
    Target,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER, INDUSTRY_PER_POP
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET
from macros.priorities import (
    CONCENTRATION_CAMP_PRIORITY,
    TARGET_AFTER_SCALING_PRIORITY,
)

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_CONC_CAMP",
    description="BLD_CONC_CAMP_DESC",
    buildcost=5 * BUILDING_COST_MULTIPLIER,
    buildtime=1,
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_CONC_CAMP"]))
        & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_RACIAL_PURITY")
        & Population(low=3)
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=Object(id=Source.PlanetID) & Planet(),
            activation=(ContainedBy(Object(id=Source.PlanetID) & Planet() & OwnedBy(empire=Source.Owner))),
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=[
                SetTargetIndustry(
                    value=Value
                    + Target.Population
                    * NamedReal(name="BLD_CONC_CAMP_TARGET_INDUSTRY_PERPOP", value=3.75 * INDUSTRY_PER_POP)
                )
            ],
        ),
        EffectsGroup(
            scope=Object(id=Source.PlanetID) & Planet(),
            activation=(
                ~EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_RACIAL_PURITY")
                | ContainedBy(
                    Object(id=Source.PlanetID)
                    & OwnedBy(empire=Source.Owner)
                    & Population(low=0.0001)
                    & ~HasSpecial(name="CONC_CAMP_MASTER_SPECIAL")
                )
            ),
            effects=[
                AddSpecial(name="CONC_CAMP_MASTER_SPECIAL"),
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(ContainedBy(Object(id=Source.PlanetID) & Population(high=0))),
            effects=[
                Destroy,
                GenerateSitRepMessage(
                    message="EFFECT_CONC_CAMP_COMLETE",
                    label="EFFECT_CONC_CAMP_COMLETE_LABEL",
                    icon="icons/building/concentration-camp.png",
                    parameters={"planet": Source.PlanetID},
                    empire=Source.Owner,
                ),
            ],
        ),
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet()),
            activation=(
                ContainedBy(
                    Object(id=Source.PlanetID)
                    & OwnedBy(empire=Source.Owner)
                    & HasSpecial(name="CONC_CAMP_MASTER_SPECIAL")
                )
            ),
            priority=CONCENTRATION_CAMP_PRIORITY,
            effects=[
                SetSpeciesOpinion(
                    species=Target.Species,
                    empire=Source.Owner,
                    opinion=Value - NamedReal(name="BLD_CONC_CAMP_TARGET_SPECIES_OPINION_PENALTY", value=3.0),
                ),
                SetPopulation(
                    value=MinOf(
                        float, Value, Target.Population + NamedReal(name="BLD_CONC_CAMP_POPULATION_FLAT", value=-3)
                    )
                ),
                SetIndustry(value=Target.TargetIndustry),
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(
                ~EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_RACIAL_PURITY")
                | ~ProducedByEmpire(empire=Source.Owner)
            ),
            effects=[Destroy],
        ),
    ],
    icon="icons/building/concentration-camp.png",
)
