import os
import os.path
import string


species_list = [
    ("SP_SUPER_TEST", "Super Tester", "icons/species/other-04.png"),
    ("SP_ABADDONI", "Abaddoni", "icons/species/abaddonnian.png"),
    ("SP_BANFORO", "Banforo", "icons/species/banforo.png"),
    ("SP_CHATO", "Chato", "icons/species/chato-matou-gormoshk.png"),
    ("SP_CRAY", "Cray", "icons/species/cray.png"),
    ("SP_DERTHREAN", "Derthrean", "icons/species/derthrean.png"),
    ("SP_EAXAW", "Eaxaw", "icons/species/eaxaw.png"),
    ("SP_EGASSEM", "Egassem", "icons/species/egassem.png"),
    ("SP_ETTY", "Etty", "icons/species/etty.png"),
    ("SP_FURTHEST", "Furthest", "icons/species/furthest.png"),
    ("SP_GEORGE", "George", "icons/species/george.png"),
    ("SP_GYSACHE", "Gysache", "icons/species/gysache.png"),
    ("SP_HAPPY", "Happybirthday", "icons/species/ichthyoid-06.png"),
    ("SP_HHHOH", "Hhhoh", "icons/species/hhhoh.png"),
    ("SP_HUMAN", "Human", "icons/species/human.png"),
    ("SP_KILANDOW", "Kilandow", "icons/species/kilandow.png"),
    ("SP_KOBUNTURA", "Kobuntura", "icons/species/intangible-04.png"),
    ("SP_LAENFA", "Laenfa", "icons/species/laenfa.png"),
    ("SP_MISIORLA", "Misiorla", "icons/species/misiorla.png"),
    ("SP_MUURSH", "Mu Ursh", "icons/species/muursh.png"),
    ("SP_PHINNERT", "Phinnert", "icons/species/phinnert.png"),
    ("SP_SCYLIOR", "Scylior", "icons/species/scylior.png"),
    ("SP_SETINON", "Setinon", "icons/species/amorphous-02.png"),
    ("SP_SILEXIAN", "Silexian", "icons/species/robotic-06.png"),
    ("SP_SSLITH", "Sslith", "icons/species/sslith.png"),
    ("SP_TAEGHIRUS", "Tae Ghirus", "icons/species/t-aeghirus.png"),
    ("SP_TRITH", "Trith", "icons/species/trith.png"),
    ("SP_RADON", "Radons", "icons/species/robotic-05.png"),
    ("SP_UGMORS", "Ugmors", "icons/species/amorphous-06.png"),
    ("SP_EXOBOT", "Exobot", "icons/species/robotic-01.png")
]

time_factor = {"SP_HAPPY": "1.2", "SP_PHINNERT": "0.75"}


t_main = string.Template('''BuildingType
    name = "BLD_COL_${name}"
    description = "BLD_COL_${name}_DESC"
    buildcost = ${cost} * [[COLONY_UPKEEP_MULTIPLICATOR]]
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
        EffectsGroup
            scope = And [
                Object id = Source.PlanetID
                Planet
            ]
            activation = And [
                Not OwnerHasTech name = "GRO_LIFECYCLE_MAN"
                Turn low = Source.CreationTurn + 1 high = Source.CreationTurn + 1
            ]
            effects = [
                SetSpecies name = "${id}"
                SetPopulation value = 1
            ]
        EffectsGroup
            scope = And [
                Object id = Source.PlanetID
                Planet
            ]
            activation = And [
                OwnerHasTech name = "GRO_LIFECYCLE_MAN"
                Turn low = Source.CreationTurn + 1 high = Source.CreationTurn + 1
            ]
            effects = [
                SetSpecies name = "${id}"
                SetPopulation value = [[MIN_RECOLONIZING_SIZE]]
            ]
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
''')

t_species_condition = string.Template(
    '''ResourceSupplyConnected empire = Source.Owner condition = And [
            Planet
            OwnedBy empire = Source.Owner
            Species name = "${id}"
            Population low = [[MIN_RECOLONIZING_SIZE]]
            Happiness low = 5
        ]''')

t_species_condition_extinct = string.Template(
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
                    HasSpecial name = "EXTINCT_${name}_SPECIAL"
                    Contains Building name = "BLD_XENORESURRECTION_LAB"
                ]
            ]
        ]''')

t_buildtime = string.Template('''${t_factor} * max(5.0, 1.0 +
        (Statistic Min Value = ShortestPath Object = Target.SystemID Object = LocalCandidate.SystemID
            Condition = And [
                Planet
                OwnedBy empire = Source.Owner
                Species name = "${id}"
                Population low = [[MIN_RECOLONIZING_SIZE]]
                Happiness low = 5
                ResourceSupplyConnected empire = Source.Owner condition = Target
            ]
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

t_buildtime_extinct = string.Template('''${t_factor} * max(5.0, 1.0 +
        (Statistic Min Value = ShortestPath Object = Target.SystemID Object = LocalCandidate.SystemID
            Condition = And [
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
            ]
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
             + 20 * (Statistic If Condition = OwnerHasTech Name = "SHP_QUANT_ENRG_MAG")
             + 10 * (Statistic If Condition = OwnerHasTech Name = "SHP_IMPROVED_ENGINE_COUPLINGS")
             + 10 * (Statistic If Condition = OwnerHasTech Name = "SHP_N_DIMENSIONAL_ENGINE_MATRIX")
             + 10 * (Statistic If Condition = OwnerHasTech Name = "SHP_SINGULARITY_ENGINE_CORE")
             + 10 * (Statistic If Condition = OwnerHasTech Name = "SHP_TRANSSPACE_DRIVE")
             + 10 * (Statistic If Condition = OwnerHasTech Name = "SHP_INTSTEL_LOG")
        )
    )''')

outpath = os.getcwd()
print ("Output folder: %s" % outpath)

for sp_id, sp_desc_name, sp_graphic in species_list:
    sp_name = sp_id.split("_", 1)[1]
    sp_tags = '"' + sp_id + '"'
    sp_filename = sp_id + ".focs.txt"

    data = {
        'id': sp_id,
        'name': sp_name,
        'tags': sp_tags,
        'graphic': sp_graphic,
        'cost': 50,
        'time': t_buildtime.substitute(id=sp_id, t_factor=time_factor.get(sp_id, "1.0")),
        'species_condition': t_species_condition.substitute(id=sp_id)
    }

    if sp_id == "SP_EXOBOT":
        data['tags'] += ' "CTRL_ALWAYS_REPORT"'
        data['cost'] = 70
        data['time'] = 5
        data['species_condition'] = r"// no existing Exobot colony required!"
    elif sp_id in ("SP_BANFORO", "SP_KILANDOW", "SP_MISIORLA"):
        data['tags'] += ' "CTRL_EXTINCT"'
        data['species_condition'] = t_species_condition_extinct.substitute(id=sp_id, name=sp_name)
        data['time'] = t_buildtime_extinct.substitute(id=sp_id, name=sp_name, t_factor=time_factor.get(sp_id, "1.0"))

    with open(os.path.join(outpath, sp_filename), "w") as f:
        f.write(t_main.substitute(**data))
