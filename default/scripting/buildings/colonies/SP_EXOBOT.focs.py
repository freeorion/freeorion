# For long term changes - Do not modify this definition directly
#                     Instead modify and execute col_bld_gen.py and use the result.

from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.misc import LIFECYCLE_MANIP_POPULATION_EFFECTS
from macros.upkeep import COLONIZATION_POLICY_MULTIPLIER, COLONY_UPKEEP_MULTIPLICATOR

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_COL_EXOBOT",
    description="BLD_COL_EXOBOT_DESC",
    buildcost=60 * COLONY_UPKEEP_MULTIPLICATOR * BUILDING_COST_MULTIPLIER * COLONIZATION_POLICY_MULTIPLIER,
    buildtime=5,
    tags=["SP_EXOBOT", "CTRL_ALWAYS_REPORT"],
    location=Planet()
    & OwnedBy(empire=Source.Owner)
    & Population(high=0)
    & ~Planet(environment=[Uninhabitable], species="SP_EXOBOT"),
    # no existing Exobot colony required!
    enqueuelocation=Planet()
    & OwnedBy(empire=Source.Owner)
    & Population(high=0)
    & ~Planet(environment=[Uninhabitable], species="SP_EXOBOT")
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
    & ~Enqueued(type=BuildBuilding, name="BLD_COL_MISIORLA"),
    # no existing Exobot colony required!
    effectsgroups=[
        *LIFECYCLE_MANIP_POPULATION_EFFECTS("SP_EXOBOT"),
        EffectsGroup(
            scope=Object(id=Source.PlanetID) & Planet(),
            activation=Turn(low=Source.CreationTurn + 1, high=Source.CreationTurn + 1),
            effects=[
                GenerateSitRepMessage(
                    message="SITREP_NEW_COLONY_ESTABLISHED",
                    label="SITREP_NEW_COLONY_ESTABLISHED_LABEL",
                    icon="icons/species/robotic-01.png",
                    parameters={
                        "species": "SP_EXOBOT",
                        "planet": Target.ID,
                    },
                    empire=Source.Owner,
                )
            ],
        ),
        EffectsGroup(scope=IsSource, activation=Turn(low=Source.CreationTurn + 2), effects=[Destroy]),
    ],
    icon="icons/species/robotic-01.png",
)
