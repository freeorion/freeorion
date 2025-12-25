from focs._effects import (
    AsteroidsType,
    Barren,
    BuildBuilding,
    Contains,
    Desert,
    Destroy,
    EffectsGroup,
    EmpireHasAdoptedPolicy,
    Enqueued,
    GasGiantType,
    GenerateSitRepMessage,
    Good,
    HasSpecial,
    HasTag,
    Inferno,
    IsBuilding,
    IsSource,
    LocalCandidate,
    MaxOf,
    Object,
    Ocean,
    OwnedBy,
    Planet,
    PlanetTypeDifference,
    Radiated,
    SetPlanetType,
    Source,
    StatisticIf,
    Swamp,
    Target,
    TargetPopulation,
    Terran,
    Toxic,
    Tundra,
    Uninhabitable,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import (
    DO_NOT_CONTAIN_FOR_ALL_TERRAFORM_PLANET_TYPES,
    ENQUEUE_BUILD_ONE_PER_PLANET,
    LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED,
)

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass


def AGGREGATED_STEPS_FROM_TO(from_, to):
    return (1 + PlanetTypeDifference(from_=from_, to=to)) * (PlanetTypeDifference(from_=from_, to=to) / 2)


_terraforming_order = [
    Desert,
    Terran,
    Ocean,
    Swamp,
    Toxic,
    Inferno,
    Radiated,
    Barren,
    Tundra,
]

_mapping = {
    Desert: "DESERT",
    Terran: "TERRAN",
    Ocean: "OCEAN",
    Swamp: "SWAMP",
    Toxic: "TOXIC",
    Inferno: "INFERNO",
    Radiated: "RADIATED",
    Barren: "BARREN",
    Tundra: "TUNDRA",
}


def get_related(current, step: int):
    curren_index = _terraforming_order.index(current)
    # keep ne index as list index.
    # To support negative steps add full length and divide for full length by module
    # l = 10, start = 0
    # step = 1:  0 + 1 + 10 => 11;  11 % 10 = 1  # second element in the list
    # step = -1:  0 - 1 + 10 => 9;  9 % 10 = 9  # last element in the list
    new_index = (curren_index + step + len(_terraforming_order)) % len(_terraforming_order)
    return _terraforming_order[new_index]


def _get_name(planet_type):
    return f"BLD_TERRAFORM_{_mapping[planet_type]}"


def TARGET_TERRAFORMING(*, current_type):
    before_source = get_related(current_type, -1)
    target_type = get_related(current_type, +1)

    pre_before_source = get_related(current_type, -2)
    post_target = get_related(current_type, +2)

    BuildingType(  # pyrefly: ignore[unbound-name]
        name=_get_name(current_type),
        description=f"{_get_name(current_type)}_DESC",
        buildcost=(
            100
            * (
                1
                - 0.4
                * StatisticIf(
                    float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_TERRAFORMING")
                )
            )
            * (
                Target.HabitableSize
                * MaxOf(int, 1, PlanetTypeDifference(from_=Target.OriginalType, to=current_type))
                * BUILDING_COST_MULTIPLIER
                * (
                    1
                    + 2
                    * StatisticIf(
                        float,
                        condition=IsSource & EmpireHasAdoptedPolicy(name="PLC_ENVIRONMENTALISM", empire=Source.Owner),
                    )
                )
            )
        ),
        buildtime=(
            12
            - 6
            * StatisticIf(
                int, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_TERRAFORMING")
            )
        ),
        location=(
            Planet()
            & OwnedBy(empire=Source.Owner)
            & ~Planet(type=[AsteroidsType, GasGiantType])
            & (
                Planet(type=[before_source])
                | Planet(type=[target_type])
                | LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED(_get_name(before_source))
                | LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED(_get_name(target_type))
            )
        ),
        enqueuelocation=(
            ENQUEUE_BUILD_ONE_PER_PLANET
            & ~Planet(type=[current_type])
            & ~Contains(IsBuilding(name=["BLD_GAIA_TRANS"]))
            & ~Enqueued(type=BuildBuilding, name="BLD_GAIA_TRANS")
            & ~HasSpecial(name="GAIA_SPECIAL")
            & ~Contains(IsBuilding(name=["BLD_TERRAFORM_BEST"]))
            & ~Enqueued(type=BuildBuilding, name="BLD_TERRAFORM_BEST")
            # Options should go down to one after a player started going into one direction,
            # E.g. after enqueuing Ocean on a Terran planet, Desert should be removed.
            # We can however not simply exclude Desert when Ocean is enqueued, since someone
            # may want to convert Swamp to Desert.
            & ~(
                Planet(type=[before_source])
                & (
                    Contains(IsBuilding(name=[_get_name(pre_before_source)]))
                    | Enqueued(type=BuildBuilding, name=_get_name(pre_before_source))
                )
            )
            & ~(
                Planet(type=[target_type])
                & (
                    Contains(IsBuilding(name=[_get_name(post_target)]))
                    | Enqueued(type=BuildBuilding, name=_get_name(post_target))
                )
            )
        ),
        effectsgroups=[
            EffectsGroup(
                scope=Object(id=Source.PlanetID) & Planet(),
                effects=[
                    SetPlanetType(type=current_type),
                    GenerateSitRepMessage(
                        message="EFFECT_TERRAFORM",
                        label="EFFECT_TERRAFORM_LABEL",
                        icon="icons/building/terraform.png",
                        parameters={
                            "planet": Target.ID,
                            "planettype": f"PT_{_mapping[current_type]}",
                            "environment": Target.ID,
                        },
                        empire=Source.Owner,
                    ),
                ],
            ),
            EffectsGroup(scope=IsSource, effects=[Destroy]),
        ],
        icon="icons/building/terraform.png",
    )


# Generate all terraform buildings for plant types
for planet_type in _terraforming_order:
    TARGET_TERRAFORMING(current_type=planet_type)


_build_cost = (
    100
    * (
        1
        - 0.4
        * StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_TERRAFORMING"))
    )
    * Target.HabitableSize
    # calculating multiple steps: stepwise-cost(A,B) = 1+2+..+N == (1+N)*N/2  iff difference between A and B is N
    # if one uses cost(OrigType, BestType) it completely ignores if you changed the type inbetween
    # if one uses cost(CurrentType, BestType) the cost growth is too little (at least) if CurrentType is between Orig and Best on the wheel
    # if one uses cost(OrigType, BestType) - cost(OrigType, CurrentType) you get a lot of corner cases (depending where the three types are in relation to another on the wheel)
    #    a) CurrentType is somewhere on the way from Orig to Best: cost(OrigType, BestType) - cost(OrigType, CurrentType) should be right
    #    b) CurrentType is not on the way from Orig to Best: ??? actually dont know - one could go back to Orig, so: cost(OrigType, BestType) + cost(OrigType, CurrentType)
    #                                                        or one could go around the other direction: other_cost(OrigType, BestType) - other_cost(OrigType, CurrentType)
    #                                                        also there could be a different best planet closest as well
    # aggregating a sum stepwise with the right step formula would be straightforward and correct, but needs better backend support
    # so we only support the standard case (i.e. CurrentType == OrigType) 100% and make sure the cost is not too low in other cases by taking a maximum
    * MaxOf(
        float,
        1,
        AGGREGATED_STEPS_FROM_TO(Target.OriginalType, Target.NextBestPlanetType),
        AGGREGATED_STEPS_FROM_TO(Target.PlanetType, Target.NextBestPlanetType),
    )
    # Giving a discount to amend opportunity cost by not multi-stepping
    # per extra step 10%, fast linear formula
    * (
        1
        - 0.1
        * (PlanetTypeDifference(from_=Target.PlanetType, to=Target.NextBestPlanetType) - 1)
        * BUILDING_COST_MULTIPLIER
        * (
            1
            + (
                2
                * StatisticIf(
                    float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_ENVIRONMENTALISM")
                )
            )
        )
    )
)

_build_time = (
    PlanetTypeDifference(from_=Target.PlanetType, to=Target.NextBestPlanetType)
    * (
        12
        - (
            6
            * StatisticIf(
                int, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_TERRAFORMING")
            )
        )
    )
    # Keep the division operation last in order to keep rounding to integer errors at minimum
    * (100 - 10 * (PlanetTypeDifference(from_=Target.PlanetType, to=Target.NextBestPlanetType) - 1))
    / 100
)  # end build time

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_TERRAFORM_BEST",
    description="BLD_TERRAFORM_BEST_DESC",
    buildcost=_build_cost,
    buildtime=_build_time,
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & TargetPopulation(low=1)
        & ~Planet(type=[AsteroidsType, GasGiantType])
        & ~Planet(environment=[Uninhabitable, Good])
        & ~Contains(IsBuilding(name=["BLD_GAIA_TRANS"]))
        & ~Enqueued(type=BuildBuilding, name="BLD_GAIA_TRANS")
        & ~HasSpecial(name="GAIA_SPECIAL")
        # no targeted terraforming buildings contained or enqueued
        & DO_NOT_CONTAIN_FOR_ALL_TERRAFORM_PLANET_TYPES()
        # Hide building if the species can not improve (Check species' highest available target environment tag)
        & ~HasTag(name="NO_TERRAFORM")
        & (LocalCandidate.PlanetType != LocalCandidate.NextBestPlanetType)
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet()),
            effects=[
                SetPlanetType(type=Target.NextBestPlanetType),
                GenerateSitRepMessage(
                    message="EFFECT_TERRAFORM",
                    label="EFFECT_TERRAFORM_LABEL",
                    icon="icons/building/terraform.png",
                    parameters={
                        "planet": Target.ID,
                        "planettype": Target.ID,
                        "environment": Target.ID,
                    },
                    empire=Source.Owner,
                ),
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            effects=[Destroy],
        ),
    ],
    icon="icons/building/terraform.png",
)
