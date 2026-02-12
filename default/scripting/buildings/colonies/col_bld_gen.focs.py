"""col_bld_gen.py

Utility script to generate consistent colony building definitions for each species.
This script is not utilized by the game.

When executed, definition files will be generated in the current working directory.

Run black and ruff to fix formatting.
"""

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


# List of all colonizable species in game: definition key, graphic file(relative to default/data/art)

species_list = [
    ("SP_SUPER_TEST", "icons/species/other-04.png"),
    ("SP_ABADDONI", "icons/species/abaddonnian.png"),
    ("SP_BANFORO", "icons/species/banforo.png"),
    ("SP_CHATO", "icons/species/chato-matou-gormoshk.png"),
    ("SP_CRAY", "icons/species/cray.png"),
    ("SP_DERTHREAN", "icons/species/derthrean.png"),
    ("SP_EAXAW", "icons/species/eaxaw.png"),
    ("SP_EGASSEM", "icons/species/egassem.png"),
    ("SP_ETTY", "icons/species/etty.png"),
    ("SP_FULVER", "icons/species/insectoid-01.png"),
    ("SP_FURTHEST", "icons/species/furthest.png"),
    ("SP_GEORGE", "icons/species/george.png"),
    ("SP_GYSACHE", "icons/species/gysache.png"),
    ("SP_HAPPY", "icons/species/ichthyoid-06.png"),
    ("SP_HHHOH", "icons/species/hhhoh.png"),
    ("SP_HUMAN", "icons/species/human.png"),
    ("SP_KILANDOW", "icons/species/kilandow.png"),
    ("SP_KOBUNTURA", "icons/species/intangible-04.png"),
    ("SP_LAENFA", "icons/species/laenfa.png"),
    ("SP_MISIORLA", "icons/species/misiorla.png"),
    ("SP_MUURSH", "icons/species/muursh.png"),
    ("SP_PHINNERT", "icons/species/phinnert.png"),
    ("SP_SCYLIOR", "icons/species/scylior.png"),
    ("SP_SETINON", "icons/species/amorphous-02.png"),
    ("SP_SILEXIAN", "icons/species/robotic-06.png"),
    ("SP_SLY", "icons/species/amorphous-05.png"),
    ("SP_SSLITH", "icons/species/sslith.png"),
    ("SP_TAEGHIRUS", "icons/species/t-aeghirus.png"),
    ("SP_TRITH", "icons/species/trith.png"),
    ("SP_REPLICON", "icons/species/replicon.png"),
    ("SP_UGMORS", "icons/species/amorphous-06.png"),
    ("SP_EXOBOT", "icons/species/robotic-01.png"),
]

# Species which start as extinct and tech enabling colonization without a suitable colony in supply range
species_extinct_techs = {
    "SP_BANFORO": "TECH_COL_BANFORO",
    "SP_KILANDOW": "TECH_COL_KILANDOW",
    "SP_MISIORLA": "TECH_COL_MISIORLA",
}

# Default gamerule to toggle availablity of colony buildings for a species (no rule)
colony_gamerule_default = ""

# Species specific gamerule enabling colony building
species_colony_gamerules = {"SP_SUPER_TEST": "RULE_ENABLE_SUPER_TESTER"}

# default base buildcost
buildcost_default = 50

# Species specific overrides to base buildcost
species_buildcost = {"SP_EXOBOT": 60}

# default buildtime factor
buildtime_factor_default = 1.0

# Species specific overrides to default buildtime factor
species_time_factor = {"SP_HAPPY": 1.2, "SP_PHINNERT": 0.75}

for sp_id, sp_graphic in species_list:
    sp_name = sp_id.split("_", 1)[1]

    gamerule = species_colony_gamerules.get(sp_id, colony_gamerule_default)

    sp_cost = (
        species_buildcost.get(sp_id, buildcost_default)
        * COLONY_UPKEEP_MULTIPLICATOR
        * BUILDING_COST_MULTIPLIER
        * COLONIZATION_POLICY_MULTIPLIER
    )

    extinct_tech = species_extinct_techs.get(sp_id, "")

    sp_tags = [sp_id]
    if sp_id == ["SP_EXOBOT"]:
        sp_tags += ["CTRL_ALWAYS_REPORT"]
    elif extinct_tech != "":
        sp_tags += ["CTRL_EXTINCT"]

    if sp_id == "SP_EXOBOT":
        species_condition = None
    if extinct_tech == "":
        species_condition = ResourceSupplyConnected(
            empire=Source.Owner,
            condition=Planet()
            & OwnedBy(empire=Source.Owner)
            & HasSpecies(name=sp_id)
            & Population(low=MIN_RECOLONIZING_SIZE)
            & Happiness(low=5),
        )
    else:
        species_condition = ResourceSupplyConnected(
            empire=Source.Owner,
            condition=Planet()
            & OwnedBy(empire=Source.Owner)
            & (
                (HasSpecies(name=sp_id) & Population(low=MIN_RECOLONIZING_SIZE) & Happiness(low=5))
                | (
                    OwnerHasTech(name=extinct_tech)
                    & HasSpecial(name=f"EXTINCT_{sp_name}_SPECIAL")
                    & Contains(IsBuilding(name=["BLD_XENORESURRECTION_LAB"]))
                )
            ),
        )

    if gamerule:
        species_condition = species_condition & (GameRule(type=int, name=gamerule) > 0)

    if sp_id == "SP_EXOBOT":
        sp_time = 5
    else:
        if extinct_tech == "":
            buildtime_stat_condition = (
                Planet()
                & OwnedBy(empire=Source.Owner)
                & HasSpecies(name=sp_id)
                & Population(low=MIN_RECOLONIZING_SIZE)
                & Happiness(low=5)
                & ResourceSupplyConnected(empire=Source.Owner, condition=IsTarget)
            )
        else:
            buildtime_stat_condition = (
                Planet()
                & OwnedBy(empire=Source.Owner)
                & (
                    (HasSpecies(name=sp_id) & Population(low=MIN_RECOLONIZING_SIZE) & Happiness(low=5))
                    | (
                        HasSpecial(name=f"EXTINCT_{sp_name}_SPECIAL")
                        & Contains(IsBuilding(name="BLD_XENORESURRECTION_LAB"))
                    )
                )
                & ResourceSupplyConnected(empire=Source.Owner, condition=IsTarget)
            )

        shortest_shortest_path_to_supply_connected = Statistic(
            float, Min, value=ShortestPath(Target.SystemID, LocalCandidate.SystemID), condition=buildtime_stat_condition
        )

        species_tf = species_time_factor.get(sp_id, buildtime_factor_default)

        sp_time = species_tf * MaxOf(
            float,
            5.0,
            1.0
            + shortest_shortest_path_to_supply_connected
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
        )

    BuildingType(  # pyright: ignore[unbound-name]
        colony=True,
        name=f"BLD_COL_{sp_name}",
        description=f"BLD_COL_{sp_name}_DESC",
        buildcost=sp_cost,
        buildtime=sp_time,
        species=sp_id,
        tags=sp_tags,
        location=Planet()
        & OwnedBy(empire=Source.Owner)
        & Population(high=0)
        & ~Planet(environment=[Uninhabitable], species=sp_id)
        & species_condition,
        enqueuelocation=Planet()
        & OwnedBy(empire=Source.Owner)
        & Population(high=0)
        & ~Planet(environment=[Uninhabitable], species=sp_id)
        & ~Contains(IsBuilding(subtype="colony") & OwnedBy(empire=Source.Owner))
        & ~Enqueued(subtype="colony")
        & species_condition,
        effectsgroups=[
            *LIFECYCLE_MANIP_POPULATION_EFFECTS(sp_id),
            EffectsGroup(
                scope=Object(id=Source.PlanetID) & Planet(),
                activation=Turn(low=Source.CreationTurn + 1, high=Source.CreationTurn + 1),
                effects=[
                    GenerateSitRepMessage(
                        message="SITREP_NEW_COLONY_ESTABLISHED",
                        label="SITREP_NEW_COLONY_ESTABLISHED_LABEL",
                        icon=sp_graphic,
                        parameters={
                            "species": sp_id,
                            "planet": Target.ID,
                        },
                        empire=Source.Owner,
                    )
                ],
            ),
            EffectsGroup(scope=IsSource, activation=Turn(low=Source.CreationTurn + 2), effects=[Destroy]),
        ],
        icon=sp_graphic,
    )
