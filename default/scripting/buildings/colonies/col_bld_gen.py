"""col_bld_gen.py

Utility script to generate consistent colony building definitions for each species.
This script is not utilized by the game.

When executed, definition files will be generated in the current working directory.
"""

import os
import os.path
import string


# List of all species in game: definition key, graphic file(relative to default/data/art)
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
    ("SP_EXOBOT", "icons/species/robotic-01.png")
]

# Species which start as extinct and tech enabling colonization without a suitable colony in supply range
species_extinct_techs = {
    "SP_BANFORO": "TECH_COL_BANFORO",
    "SP_KILANDOW": "TECH_COL_KILANDOW",
    "SP_MISIORLA": "TECH_COL_MISIORLA"
}

# Default gamerule to toggle availablity of colony buildings for a species (no rule)
colony_gamerule_default = ""

# Species specific gamerule enabling colony building
species_colony_gamerules = {
    "SP_SUPER_TEST": "RULE_ENABLE_SUPER_TESTER"
}

# default base buildcost
buildcost_default = 50

# Species specific overrides to base buildcost
species_buildcost = {
    "SP_EXOBOT": 70
}

# default buildtime factor
buildtime_factor_default = "1.0"

# Species specific overrides to default buildtime factor
species_time_factor = {
    "SP_HAPPY": "1.2",
    "SP_PHINNERT": "0.75"
}


# Main template
t_main = string.Template('''// For long term changes - Do not modify this definition directly
//                     Instead modify and execute col_bld_gen.py and use the result.
BuildingType
    name = "BLD_COL_${name}"
    description = "BLD_COL_${name}_DESC"
    buildcost = ${cost} * [[COLONY_UPKEEP_MULTIPLICATOR]] * [[BUILDING_COST_MULTIPLIER]]
    buildtime = ${time}
    tags = [ ${tags} ]
    location = And [
        Planet
        OwnedBy empire = Source.Owner
        Population high = 0
        Not Planet environment = Uninhabitable species = "${id}"
        Not Contains Building name = "BLD_COL_${name}"
        ${species_condition}
    ]
    EnqueueLocation = And [
        Planet
        OwnedBy empire = Source.Owner
        Population high = 0
        Not Planet environment = Uninhabitable species = "${id}"
        Not Contains Building name = "BLD_COL_${name}"
        Not Enqueued type = Building name = "BLD_COL_${name}"
        ${species_condition}
    ]
    effectsgroups = [
        [[LIFECYCLE_MANIP_POPULATION_EFFECTS("${id}")]]
        
        EffectsGroup
            scope = And [
                Object id = Source.PlanetID
                Planet
            ]
            activation = Turn low = Source.CreationTurn + 1 high = Source.CreationTurn + 1
            effects = [
                GenerateSitRepMessage
                    message = "SITREP_NEW_COLONY_ESTABLISHED"
                    label = "SITREP_NEW_COLONY_ESTABLISHED_LABEL"
                    icon = "${graphic}"
                    parameters = [
                        tag = "species" data = "${id}"
                        tag = "planet" data = Target.ID
                    ]
                    empire = Source.Owner
            ]
            
        EffectsGroup
            scope = Source
            activation = Turn low = Source.CreationTurn + 2
            effects = Destroy
    ]
    icon = "${graphic}"

#include "/scripting/common/misc.macros"
#include "/scripting/common/upkeep.macros"
#include "/scripting/common/priorities.macros"
#include "/scripting/common/base_prod.macros"
#include "/scripting/species/common/population.macros"
''')

# Location and Enqueued condition template
t_species_cond = string.Template(
    '''ResourceSupplyConnected empire = Source.Owner condition = And [
            Planet
            OwnedBy empire = Source.Owner
            Species name = "${id}"
            Population low = [[MIN_RECOLONIZING_SIZE]]
            Happiness low = 5
        ]''')

# Location and Enqueued condition template for extinct species
t_species_cond_extinct = string.Template(
    '''ResourceSupplyConnected empire = Source.Owner condition = And [
            Planet
            OwnedBy empire = Source.Owner
            Or [
                And [
                    Species name = "${id}"
                    Population low = [[MIN_RECOLONIZING_SIZE]]
                    Happiness low = 5
                ]
                And [
                    OwnerHasTech name = "${tech_name}"
                    HasSpecial name = "EXTINCT_${name}_SPECIAL"
                    Contains Building name = "BLD_XENORESURRECTION_LAB"
                ]
            ]
        ]''')

# buildtime statistic condition template
t_buildtime_stat_cond = string.Template(
    '''Condition = And [
                Planet
                OwnedBy empire = Source.Owner
                Species name = "${id}"
                Population low = [[MIN_RECOLONIZING_SIZE]]
                Happiness low = 5
                ResourceSupplyConnected empire = Source.Owner condition = Target
            ]''')

# buildtime statistic condition template for extinct species
t_buildtime_stat_cond_extinct = string.Template(
    '''Condition = And [
                Planet
                OwnedBy empire = Source.Owner
            Or [
               And [
                   Species name = "${id}"
                   Population low = [[MIN_RECOLONIZING_SIZE]]
                   Happiness low = 5
                ]
                And [
                    HasSpecial name = "EXTINCT_${name}_SPECIAL"
                    Contains Building name = "BLD_XENORESURRECTION_LAB"
                ]
            ]
                ResourceSupplyConnected empire = Source.Owner condition = Target
            ]''')

# buildtime template
t_buildtime = string.Template('''${t_factor} * max(5.0, 1.0 +
        (Statistic Min Value = ShortestPath Object = Target.SystemID Object = LocalCandidate.SystemID
            ${stat_condition}
        ) / (60
             + 20 * (Statistic If Condition = Or [
                 OwnerHasTech name = "SHP_MIL_ROBO_CONT"
                 OwnerHasTech name = "SHP_ORG_HULL"
                 OwnerHasTech name = "SHP_QUANT_ENRG_MAG"
             ])
             + 20 * (Statistic If Condition = Or [
                 OwnerHasTech name = "SHP_ORG_HULL"
                 OwnerHasTech name = "SHP_QUANT_ENRG_MAG"
             ])
             + 20 * (Statistic If Condition = OwnerHasTech name = "SHP_QUANT_ENRG_MAG")
             + 10 * (Statistic If Condition = OwnerHasTech name = "SHP_IMPROVED_ENGINE_COUPLINGS")
             + 10 * (Statistic If Condition = OwnerHasTech name = "SHP_N_DIMENSIONAL_ENGINE_MATRIX")
             + 10 * (Statistic If Condition = OwnerHasTech name = "SHP_SINGULARITY_ENGINE_CORE")
             + 10 * (Statistic If Condition = OwnerHasTech name = "SHP_TRANSSPACE_DRIVE")
             + 10 * (Statistic If Condition = OwnerHasTech name = "SHP_INTSTEL_LOG")
        )
    )''')

outpath = os.getcwd()
print("Output folder: %s" % outpath)

for sp_id, sp_graphic in species_list:
    sp_name = sp_id.split("_", 1)[1]
    sp_tags = '"' + sp_id + '"'
    sp_filename = sp_id + ".focs.txt"
    sp_gamerule = species_colony_gamerules.get(sp_id, colony_gamerule_default)
    extinct_tech = species_extinct_techs.get(sp_id, '')

    data = {
        'id': sp_id,
        'name': sp_name,
        'tags': sp_tags,
        'graphic': sp_graphic,
        'cost': species_buildcost.get(sp_id, buildcost_default),
        'time': '',
        'species_condition': ''
    }

    if sp_id == "SP_EXOBOT":
        data['tags'] += ' "CTRL_ALWAYS_REPORT"'
        data['time'] = 5
        data['species_condition'] = r"// no existing Exobot colony required!"
    else:
        if extinct_tech != '':
            data['tags'] += ' "CTRL_EXTINCT"'
            data['time'] = t_buildtime.substitute(t_factor=species_time_factor.get(sp_id, buildtime_factor_default),
                                                  stat_condition=t_buildtime_stat_cond_extinct.substitute(id=sp_id,
                                                                                                          name=sp_name))
            data['species_condition'] = t_species_cond_extinct.substitute(tech_name=extinct_tech, id=sp_id,
                                                                          name=sp_name)
        else:
            data['time'] = t_buildtime.substitute(t_factor=species_time_factor.get(sp_id, buildtime_factor_default),
                                                  stat_condition=t_buildtime_stat_cond.substitute(id=sp_id))
            data['species_condition'] = t_species_cond.substitute(id=sp_id)

    if sp_gamerule:
        data['species_condition'] += ("\n        ((GameRule name = \"%s\") > 0)" % sp_gamerule)

    with open(os.path.join(outpath, sp_filename), "w") as f:
        f.write(t_main.substitute(**data))
