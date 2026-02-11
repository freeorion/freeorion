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
    name="BLD_COL_EXOBOT",
    description="BLD_COL_EXOBOT_DESC",
    buildcost=60 * COLONY_UPKEEP_MULTIPLICATOR * BUILDING_COST_MULTIPLIER * COLONIZATION_POLICY_MULTIPLIER,
    buildtime=5,
    species="SP_EXOBOT",
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
    & ~Contains(IsBuilding(subtype="colony") & OwnedBy(empire=Source.Owner))
    & ~Enqueued(subtype="colony"),
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
