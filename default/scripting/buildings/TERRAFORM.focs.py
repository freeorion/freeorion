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


def TARGET_TERRAFORMING(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8):
    """
    // @1@ clockwise previous type
    // @2@ target type
    // @3@ clockwise next type
    // @4-8@ cw-2 - cw+2 in upper case
    :return:
    """

    BuildingType(  # type: ignore[reportUnboundVariable]
        name=f"BLD_TERRAFORM_{arg6}",
        description=f"BLD_TERRAFORM_{arg6}_DESC",
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
                * MaxOf(int, 1, PlanetTypeDifference(from_=Target.OriginalType, to=arg2))
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
                Planet(type=[arg1])
                | Planet(type=[arg3])
                | LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED("BLD_TERRAFORM_" + arg5)
                | LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED("BLD_TERRAFORM_" + arg7)
            )
        ),
        enqueuelocation=(
            ENQUEUE_BUILD_ONE_PER_PLANET
            & ~Planet(type=[arg2])
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
                Planet(type=[arg1])
                & (
                    Contains(IsBuilding(name=[f"BLD_TERRAFORM_{arg4}"]))
                    | Enqueued(type=BuildBuilding, name=f"BLD_TERRAFORM_{arg4}")
                )
            )
            & ~(
                Planet(type=[arg3])
                & (
                    Contains(IsBuilding(name=[f"BLD_TERRAFORM_{arg8}"]))
                    | Enqueued(type=BuildBuilding, name=f"BLD_TERRAFORM_{arg8}")
                )
            )
        ),
        effectsgroups=[
            EffectsGroup(
                scope=Object(id=Source.PlanetID) & Planet(),
                effects=[
                    SetPlanetType(type=arg2),
                    GenerateSitRepMessage(
                        message="EFFECT_TERRAFORM",
                        label="EFFECT_TERRAFORM_LABEL",
                        icon="icons/building/terraform.png",
                        parameters={
                            "planet": Target.ID,
                            "planettype": f"PT_{arg6}",
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


TARGET_TERRAFORMING(Desert, Terran, Ocean, "TUNDRA", "DESERT", "TERRAN", "OCEAN", "SWAMP")
TARGET_TERRAFORMING(Terran, Ocean, Swamp, "DESERT", "TERRAN", "OCEAN", "SWAMP", "TOXIC")
TARGET_TERRAFORMING(Ocean, Swamp, Toxic, "TERRAN", "OCEAN", "SWAMP", "TOXIC", "INFERNO")
TARGET_TERRAFORMING(Swamp, Toxic, Inferno, "OCEAN", "SWAMP", "TOXIC", "INFERNO", "RADIATED")
TARGET_TERRAFORMING(Toxic, Inferno, Radiated, "SWAMP", "TOXIC", "INFERNO", "RADIATED", "BARREN")
TARGET_TERRAFORMING(Inferno, Radiated, Barren, "TOXIC", "INFERNO", "RADIATED", "BARREN", "TUNDRA")
TARGET_TERRAFORMING(Radiated, Barren, Tundra, "INFERNO", "RADIATED", "BARREN", "TUNDRA", "DESERT")
TARGET_TERRAFORMING(Barren, Tundra, Desert, "RADIATED", "BARREN", "TUNDRA", "DESERT", "TERRAN")
TARGET_TERRAFORMING(Tundra, Desert, Terran, "BARREN", "TUNDRA", "DESERT", "TERRAN", "OCEAN")

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

BuildingType(  # type: ignore[reportUnboundVariable]
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
