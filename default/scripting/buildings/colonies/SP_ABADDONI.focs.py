# For long term changes - Do not modify this definition directly
#                     Instead modify and execute col_bld_gen.py and use the result.

from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.misc import LIFECYCLE_MANIP_POPULATION_EFFECTS, MIN_RECOLONIZING_SIZE
from macros.upkeep import COLONIZATION_POLICY_MULTIPLIER, COLONY_UPKEEP_MULTIPLICATOR

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    colony=True,
    name="BLD_COL_ABADDONI",
    description="BLD_COL_ABADDONI_DESC",
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
                & HasSpecies(name=["SP_ABADDONI"])
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
    tags=["SP_ABADDONI"],
    location=Planet()
    & OwnedBy(empire=Source.Owner)
    & Population(high=0)
    & ~Planet(environment=[Uninhabitable], species="SP_ABADDONI")
    & ResourceSupplyConnected(
        empire=Source.Owner,
        condition=Planet()
        & OwnedBy(empire=Source.Owner)
        & HasSpecies(name=["SP_ABADDONI"])
        & Population(low=MIN_RECOLONIZING_SIZE)
        & Happiness(low=5),
    ),
    enqueuelocation=Planet()
    & OwnedBy(empire=Source.Owner)
    & Population(high=0)
    & ~Planet(environment=[Uninhabitable], species="SP_ABADDONI")
    & ~Contains(IsBuilding(name=["BLD_COL_SUPER_TEST"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_SUPER_TEST")
    & ~Contains(IsBuilding(name=["BLD_COL_ABADDONI"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_ABADDONI")
    & ~Contains(IsBuilding(name=["BLD_COL_BANFORO"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_BANFORO")
    & ~Contains(IsBuilding(name=["BLD_COL_CHATO"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_CHATO")
    & ~Contains(IsBuilding(name=["BLD_COL_CRAY"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_CRAY")
    & ~Contains(IsBuilding(name=["BLD_COL_DERTHREAN"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_DERTHREAN")
    & ~Contains(IsBuilding(name=["BLD_COL_EAXAW"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_EAXAW")
    & ~Contains(IsBuilding(name=["BLD_COL_EGASSEM"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_EGASSEM")
    & ~Contains(IsBuilding(name=["BLD_COL_ETTY"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_ETTY")
    & ~Contains(IsBuilding(name=["BLD_COL_FULVER"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_FULVER")
    & ~Contains(IsBuilding(name=["BLD_COL_FURTHEST"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_FURTHEST")
    & ~Contains(IsBuilding(name=["BLD_COL_GEORGE"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_GEORGE")
    & ~Contains(IsBuilding(name=["BLD_COL_GYSACHE"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_GYSACHE")
    & ~Contains(IsBuilding(name=["BLD_COL_HAPPY"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_HAPPY")
    & ~Contains(IsBuilding(name=["BLD_COL_HHHOH"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_HHHOH")
    & ~Contains(IsBuilding(name=["BLD_COL_HUMAN"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_HUMAN")
    & ~Contains(IsBuilding(name=["BLD_COL_KILANDOW"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_KILANDOW")
    & ~Contains(IsBuilding(name=["BLD_COL_KOBUNTURA"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_KOBUNTURA")
    & ~Contains(IsBuilding(name=["BLD_COL_LAENFA"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_LAENFA")
    & ~Contains(IsBuilding(name=["BLD_COL_MISIORLA"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_MISIORLA")
    & ~Contains(IsBuilding(name=["BLD_COL_MUURSH"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_MUURSH")
    & ~Contains(IsBuilding(name=["BLD_COL_PHINNERT"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_PHINNERT")
    & ~Contains(IsBuilding(name=["BLD_COL_SCYLIOR"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_SCYLIOR")
    & ~Contains(IsBuilding(name=["BLD_COL_SETINON"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_SETINON")
    & ~Contains(IsBuilding(name=["BLD_COL_SILEXIAN"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_SILEXIAN")
    & ~Contains(IsBuilding(name=["BLD_COL_SLY"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_SLY")
    & ~Contains(IsBuilding(name=["BLD_COL_SSLITH"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_SSLITH")
    & ~Contains(IsBuilding(name=["BLD_COL_TAEGHIRUS"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_TAEGHIRUS")
    & ~Contains(IsBuilding(name=["BLD_COL_TRITH"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_TRITH")
    & ~Contains(IsBuilding(name=["BLD_COL_REPLICON"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_REPLICON")
    & ~Contains(IsBuilding(name=["BLD_COL_UGMORS"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_UGMORS")
    & ~Contains(IsBuilding(name=["BLD_COL_EXOBOT"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_EXOBOT")
    & ~Contains(IsBuilding(name=["BLD_COL_BANFORO"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_BANFORO")
    & ~Contains(IsBuilding(name=["BLD_COL_KILANDOW"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_KILANDOW")
    & ~Contains(IsBuilding(name=["BLD_COL_MISIORLA"]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_MISIORLA")
    & ResourceSupplyConnected(
        empire=Source.Owner,
        condition=Planet()
        & OwnedBy(empire=Source.Owner)
        & HasSpecies(name=["SP_ABADDONI"])
        & Population(low=MIN_RECOLONIZING_SIZE)
        & Happiness(low=5),
    ),
    effectsgroups=[
        *LIFECYCLE_MANIP_POPULATION_EFFECTS("SP_ABADDONI"),
        EffectsGroup(
            scope=Object(id=Source.PlanetID) & Planet(),
            activation=Turn(low=Source.CreationTurn + 1, high=Source.CreationTurn + 1),
            effects=[
                GenerateSitRepMessage(
                    message="SITREP_NEW_COLONY_ESTABLISHED",
                    label="SITREP_NEW_COLONY_ESTABLISHED_LABEL",
                    icon="icons/species/abaddonnian.png",
                    parameters={
                        "species": "SP_ABADDONI",
                        "planet": Target.ID,
                    },
                    empire=Source.Owner,
                )
            ],
        ),
        EffectsGroup(scope=IsSource, activation=Turn(low=Source.CreationTurn + 2), effects=[Destroy]),
    ],
    icon="icons/species/abaddonnian.png",
)
