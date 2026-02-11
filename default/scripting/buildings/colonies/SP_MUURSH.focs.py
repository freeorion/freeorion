# For long term changes - Do not modify this definition directly
#                     Instead modify and execute col_bld_gen.py and use the result.

from typing import TYPE_CHECKING

from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.misc import LIFECYCLE_MANIP_POPULATION_EFFECTS, MIN_RECOLONIZING_SIZE
from macros.upkeep import COLONIZATION_POLICY_MULTIPLIER, COLONY_UPKEEP_MULTIPLICATOR

if TYPE_CHECKING:
    # Stub for type checker only
    def BuildingType(*args, **kwargs):
        pass


try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyright: ignore[unbound-name]
    colony=True,
    name="BLD_COL_MUURSH",
    description="BLD_COL_MUURSH_DESC",
    buildcost=50 * COLONY_UPKEEP_MULTIPLICATOR * BUILDING_COST_MULTIPLIER * COLONIZATION_POLICY_MULTIPLIER,
    buildtime=1.0
    * MaxOf(
        float,
        5.0,
        1.0
        + (
            Statistic(
                float,
                Min,
                value=ShortestPath(Target.SystemID, LocalCandidate.SystemID),
                condition=Planet()
                & OwnedBy(empire=Source.Owner)
                & HasSpecies(name=["SP_MUURSH"])
                & Population(low=MIN_RECOLONIZING_SIZE)
                & Happiness(low=5)
                & ResourceSupplyConnected(empire=Source.Owner, condition=IsTarget),
            )
        )
        / (
            60
            + 20
            * (
                StatisticIf(
                    int,
                    condition=(IsSource & OwnerHasTech(name="SHP_MIL_ROBO_CONT"))
                    | (IsSource & OwnerHasTech(name="SHP_SPACE_FLUX_BUBBLE"))
                    | (IsSource & OwnerHasTech(name="SHP_ORG_HULL"))
                    | (IsSource & OwnerHasTech(name="SHP_QUANT_ENRG_MAG")),
                )
            )
            + 20
            * (
                StatisticIf(
                    int,
                    condition=(IsSource & OwnerHasTech(name="SHP_ORG_HULL"))
                    | (IsSource & OwnerHasTech(name="SHP_QUANT_ENRG_MAG")),
                )
            )
            + 20 * (StatisticIf(int, condition=IsSource & OwnerHasTech(name="SHP_QUANT_ENRG_MAG")))
            + 10 * (StatisticIf(int, condition=IsSource & OwnerHasTech(name="SHP_IMPROVED_ENGINE_COUPLINGS")))
            + 10 * (StatisticIf(int, condition=IsSource & OwnerHasTech(name="SHP_N_DIMENSIONAL_ENGINE_MATRIX")))
            + 10 * (StatisticIf(int, condition=IsSource & OwnerHasTech(name="SHP_SINGULARITY_ENGINE_CORE")))
            + 10 * (StatisticIf(int, condition=IsSource & OwnerHasTech(name="SHP_TRANSSPACE_DRIVE")))
            + 10 * (StatisticIf(int, condition=IsSource & OwnerHasTech(name="SHP_INTSTEL_LOG")))
        ),
    ),
    species="SP_MUURSH",
    tags=["SP_MUURSH"],
    location=Planet()
    & OwnedBy(empire=Source.Owner)
    & Population(high=0)
    & ~Planet(environment=[Uninhabitable], species="SP_MUURSH")
    & ResourceSupplyConnected(
        empire=Source.Owner,
        condition=Planet()
        & OwnedBy(empire=Source.Owner)
        & HasSpecies(name=["SP_MUURSH"])
        & Population(low=MIN_RECOLONIZING_SIZE)
        & Happiness(low=5),
    ),
    enqueuelocation=Planet()
    & OwnedBy(empire=Source.Owner)
    & Population(high=0)
    & ~Planet(environment=[Uninhabitable], species="SP_MUURSH")
    & ~Contains(IsBuilding(subtype="colony") & OwnedBy(empire=Source.Owner))
    & ~Enqueued(subtype="colony")
    & ResourceSupplyConnected(
        empire=Source.Owner,
        condition=Planet()
        & OwnedBy(empire=Source.Owner)
        & HasSpecies(name=["SP_MUURSH"])
        & Population(low=MIN_RECOLONIZING_SIZE)
        & Happiness(low=5),
    ),
    effectsgroups=[
        *LIFECYCLE_MANIP_POPULATION_EFFECTS("SP_MUURSH"),
        EffectsGroup(
            scope=Object(id=Source.PlanetID) & Planet(),
            activation=Turn(low=Source.CreationTurn + 1, high=Source.CreationTurn + 1),
            effects=[
                GenerateSitRepMessage(
                    message="SITREP_NEW_COLONY_ESTABLISHED",
                    label="SITREP_NEW_COLONY_ESTABLISHED_LABEL",
                    icon="icons/species/muursh.png",
                    parameters={
                        "species": "SP_MUURSH",
                        "planet": Target.ID,
                    },
                    empire=Source.Owner,
                )
            ],
        ),
        EffectsGroup(scope=IsSource, activation=Turn(low=Source.CreationTurn + 2), effects=[Destroy]),
    ],
    icon="icons/species/muursh.png",
)
