"""col_bld_gen.py

Utility script to generate consistent colony building definitions for each species.
This script is not utilized by the game.

When executed, definition files will be generated in the current working directory.

Run black and ruff to fix formatting.
"""

import os
import os.path
import string
from itertools import chain

# List of all colonizable species in game: definition key, graphic file(relative to default/data/art)
from typing import Any

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
buildtime_factor_default = "1.0"

# Species specific overrides to default buildtime factor
species_time_factor = {"SP_HAPPY": "1.2", "SP_PHINNERT": "0.75"}


# Main template
t_main = string.Template(
    """# For long term changes - Do not modify this definition directly
#                     Instead modify and execute col_bld_gen.py and use the result.

from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.upkeep import COLONY_UPKEEP_MULTIPLICATOR, COLONIZATION_POLICY_MULTIPLIER
from macros.misc import MIN_RECOLONIZING_SIZE, LIFECYCLE_MANIP_POPULATION_EFFECTS

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType( # type: ignore[reportUnboundVariable]
    colony=True,
    name="BLD_COL_${name}",
    description="BLD_COL_${name}_DESC",
    buildcost=${cost} * COLONY_UPKEEP_MULTIPLICATOR * BUILDING_COST_MULTIPLIER * COLONIZATION_POLICY_MULTIPLIER,
    buildtime=${time},
    species="${id}",
    tags=[ ${tags} ],
    location =
        Planet() &
        OwnedBy( empire = Source.Owner)&
        Population (high = 0)&
        ~Planet (environment = [Uninhabitable], species = "${id}")
        ${species_condition}
    ,
    enqueuelocation =
        Planet() &
        OwnedBy( empire = Source.Owner)&
        Population (high = 0)&
        ~Planet (environment = [Uninhabitable], species = "${id}")
        ${exclude_parallel_colonies}
        ${species_condition}
    ,
    effectsgroups = [
        *LIFECYCLE_MANIP_POPULATION_EFFECTS("${id}"),

        EffectsGroup(
            scope =
                Object (id = Source.PlanetID)&
                Planet(),

            activation = Turn (low = Source.CreationTurn + 1, high = Source.CreationTurn + 1),
            effects = [
                GenerateSitRepMessage(
                    message = "SITREP_NEW_COLONY_ESTABLISHED",
                    label = "SITREP_NEW_COLONY_ESTABLISHED_LABEL",
                    icon = "${graphic}",
                    parameters = {
                        "species": "${id}",
                        "planet": Target.ID,
                    },
                    empire = Source.Owner)
            ]),

        EffectsGroup(
            scope = IsSource,
            activation = Turn (low = Source.CreationTurn + 2),
            effects = [Destroy]),
    ],
    icon = "${graphic}",
)
"""
)

# Location and Enqueued condition template
t_species_cond = string.Template(
    """&ResourceSupplyConnected( empire = Source.Owner, condition =
            Planet()&
            OwnedBy( empire = Source.Owner)&
            HasSpecies( name = ["${id}"])&
            Population (low = MIN_RECOLONIZING_SIZE)&
            Happiness (low = 5))
        """
)

# Location and Enqueued condition template for extinct species
t_species_cond_extinct = string.Template(
    """&ResourceSupplyConnected (empire = Source.Owner, condition =
            Planet()&
            OwnedBy( empire = Source.Owner)&
            (
                (
                    HasSpecies( name = ["${id}"])&
                    Population( low = MIN_RECOLONIZING_SIZE)&
                    Happiness( low = 5)
                )|
                (
                    OwnerHasTech( name = "${tech_name}")&
                    HasSpecial (name = "EXTINCT_${name}_SPECIAL")&
                    Contains( IsBuilding( name = ["BLD_XENORESURRECTION_LAB"]))
                )
            ))
        """
)

# buildtime statistic condition template
t_buildtime_stat_cond = string.Template(
    """condition =
                Planet() &
                OwnedBy ( empire = Source.Owner) &
                HasSpecies ( name = ["${id}"])&
                Population (low = MIN_RECOLONIZING_SIZE)&
                Happiness (low = 5) &
                ResourceSupplyConnected (empire = Source.Owner, condition = IsTarget)
            """
)

# buildtime statistic condition template for extinct species
t_buildtime_stat_cond_extinct = string.Template(
    """condition =
                Planet() &
                OwnedBy ( empire = Source.Owner) &
                (
                   (
                       HasSpecies( name = ["${id}"])&
                       Population (low = MIN_RECOLONIZING_SIZE)&
                       Happiness (low = 5)
                    ) |
                    (
                        HasSpecial (name = "EXTINCT_${name}_SPECIAL") &
                        Contains( IsBuilding( name = ["BLD_XENORESURRECTION_LAB"]))
                    )
                ) &
                ResourceSupplyConnected (empire = Source.Owner, condition = IsTarget)
            """
)

# buildtime template
t_buildtime = string.Template(
    """${t_factor} * MaxOf(float, 5.0, 1.0 +
        (Statistic(float, Min, value = ShortestPath( Target.SystemID, LocalCandidate.SystemID),
            ${stat_condition}
        )) / (60
             + 20 * (StatisticIf(int, condition =
                 ( IsSource & OwnerHasTech (name = "SHP_MIL_ROBO_CONT")) |
                 ( IsSource & OwnerHasTech (name = "SHP_SPACE_FLUX_BUBBLE")) |
                 ( IsSource & OwnerHasTech (name = "SHP_ORG_HULL")) |
                 ( IsSource & OwnerHasTech (name = "SHP_QUANT_ENRG_MAG"))
             ))
             + 20 * (StatisticIf(int, condition =
                 ( IsSource & OwnerHasTech( name = "SHP_ORG_HULL" )) |
                 ( IsSource & OwnerHasTech( name = "SHP_QUANT_ENRG_MAG"))
             ))
             + 20 * (StatisticIf(int, condition = IsSource & OwnerHasTech( name = "SHP_QUANT_ENRG_MAG")))
             + 10 * (StatisticIf(int, condition = IsSource & OwnerHasTech( name = "SHP_IMPROVED_ENGINE_COUPLINGS")))
             + 10 * (StatisticIf(int, condition = IsSource & OwnerHasTech( name = "SHP_N_DIMENSIONAL_ENGINE_MATRIX")))
             + 10 * (StatisticIf(int, condition = IsSource & OwnerHasTech( name = "SHP_SINGULARITY_ENGINE_CORE")))
             + 10 * (StatisticIf(int, condition = IsSource & OwnerHasTech( name = "SHP_TRANSSPACE_DRIVE")))
             + 10 * (StatisticIf(int, condition = IsSource & OwnerHasTech( name = "SHP_INTSTEL_LOG")))
        )
    )"""
)

exclude_parallel_colonies = ""
for species in chain((x[0] for x in species_list), species_extinct_techs.keys()):
    name = species[3:]
    exclude_parallel_colonies += f"""\
        & ~Contains( IsBuilding (name = ["BLD_COL_{name}"]) & OwnedBy (empire = Source.Owner))
        & ~Enqueued(type = BuildBuilding, name = "BLD_COL_{name}")
"""
# remove indent from first line and newline at the end to match format expected by the template
exclude_parallel_colonies = exclude_parallel_colonies[8:-1]

outpath = os.getcwd()
print("Output folder: %s" % outpath)

for sp_id, sp_graphic in species_list:
    sp_name = sp_id.split("_", 1)[1]
    sp_tags = '"' + sp_id + '"'
    sp_filename = sp_id + ".focs.py"
    sp_gamerule = species_colony_gamerules.get(sp_id, colony_gamerule_default)
    extinct_tech = species_extinct_techs.get(sp_id, "")

    data: dict[str, Any] = {
        "id": sp_id,
        "name": sp_name,
        "tags": sp_tags,
        "graphic": sp_graphic,
        "cost": species_buildcost.get(sp_id, buildcost_default),
        "time": "",
        "exclude_parallel_colonies": exclude_parallel_colonies,
        "species_condition": "",
    }

    if sp_id == "SP_EXOBOT":
        data["tags"] += ', "CTRL_ALWAYS_REPORT"'
        data["time"] = 5
        data["species_condition"] = r"# no existing Exobot colony required!"
    else:
        if extinct_tech != "":
            data["tags"] += ', "CTRL_EXTINCT"'
            data["time"] = t_buildtime.substitute(
                t_factor=species_time_factor.get(sp_id, buildtime_factor_default),
                stat_condition=t_buildtime_stat_cond_extinct.substitute(id=sp_id, name=sp_name),
            )
            data["species_condition"] = t_species_cond_extinct.substitute(
                tech_name=extinct_tech, id=sp_id, name=sp_name
            )
        else:
            data["time"] = t_buildtime.substitute(
                t_factor = species_time_factor.get(sp_id, buildtime_factor_default),
                stat_condition = t_buildtime_stat_cond.substitute(id=sp_id),
            )
            data["species_condition"] = t_species_cond.substitute(id=sp_id)

    if sp_gamerule:
        data["species_condition"] += '\n        &(GameRule(type=int, name = "%s") > 0)' % sp_gamerule

    with open(os.path.join(outpath, sp_filename), "w") as f:
        f.write(t_main.substitute(**data))
