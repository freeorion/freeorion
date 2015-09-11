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
    ("SP_KILANDOW", "Kilandow", "icons/species/insectoid-03.png"),
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
    ("SP_UGMORS", "Ugmors", "icons/species/amorphous-06.png"),
    ("SP_EXOBOT", "Exobot", "icons/species/robotic-01.png")
]

time_factor = {"SP_HAPPY": "1.2", "SP_PHINNERT": "0.75"}


t_main = string.Template('''BuildingType
    name = "BLD_COL_${name}"
    description = "BLD_COL_${name}_DESC"
    buildcost = ${cost} * [[COLONY_UPKEEP_MULTIPLICATOR]]
    buildtime = ${time}
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
    icon = "${graphic}"''')

t_species_condition = string.Template('''ResourceSupplyConnected empire = Source.Owner condition = And [
            Planet
            OwnedBy empire = Source.Owner
            Species name = "${id}"
            Population low = [[MIN_RECOLONIZING_SIZE]]
            Happiness low = 5
        ]''')

t_species_condition_extinct = string.Template('''ResourceSupplyConnected empire = Source.Owner condition = And [
            Planet
            OwnedBy empire = Source.Owner
            Or [
                And [
                    Species name = "${id}"
                    Population low = [[MIN_RECOLONIZING_SIZE]]
                    Happiness low = 5
                ]
                And [
                    HasSpecial name = "ANCIENT_RUINS_DEPLETED_${name}_SPECIAL"
                    Contains Building name = "BLD_XENORESURRECTION_LAB"
                ]
            ]
        ]''')

t_buildtime = string.Template('''${t_factor} * max(5.0, 1.0 +
        (min value = ShortestPath object = Target.SystemID object = LocalCandidate.SystemID
            condition = And [
                Planet
                OwnedBy empire = Source.Owner
                Species name = "${id}"
                Population low = [[MIN_RECOLONIZING_SIZE]]
                Happiness low = 5
                ResourceSupplyConnected empire = Source.Owner condition = Target
            ]
        ) / (60
             + 20 * (If condition = Or [
                 OwnerHasTech name = "SHP_MIL_ROBO_CONT"
                 OwnerHasTech name = "SHP_ORG_HULL"
                 OwnerHasTech name = "SHP_QUANT_ENRG_MAG"
             ])
             + 20 * (If condition = Or [
                 OwnerHasTech name = "SHP_ORG_HULL"
                 OwnerHasTech name = "SHP_QUANT_ENRG_MAG"
             ])
             + 20 * (If condition = OwnerHasTech name = "SHP_QUANT_ENRG_MAG")
             + 10 * (If condition = OwnerHasTech name = "SHP_IMPROVED_ENGINE_COUPLINGS")
             + 10 * (If condition = OwnerHasTech name = "SHP_N_DIMENSIONAL_ENGINE_MATRIX")
             + 10 * (If condition = OwnerHasTech name = "SHP_SINGULARITY_ENGINE_CORE")
             + 10 * (If condition = OwnerHasTech name = "SHP_TRANSSPACE_DRIVE")
             + 10 * (If condition = OwnerHasTech name = "SHP_INTSTEL_LOG")
        )
    )''')

t_buildtime_extinct = string.Template('''${t_factor} * max(5.0, 1.0 +
        (min value = ShortestPath object = Target.SystemID object = LocalCandidate.SystemID
            condition = And [
                Planet
                OwnedBy empire = Source.Owner
            Or [
               And [
                   Species name = "${id}"
                   Population low = [[MIN_RECOLONIZING_SIZE]]
                   Happiness low = 5
                ]
                And [
                    HasSpecial name = "ANCIENT_RUINS_DEPLETED_${name}_SPECIAL"
                    Contains Building name = "BLD_XENORESURRECTION_LAB"
                ]
            ]
                ResourceSupplyConnected empire = Source.Owner condition = Target
            ]
        ) / (60
             + 20 * (If condition = Or [
                 OwnerHasTech name = "SHP_MIL_ROBO_CONT"
                 OwnerHasTech name = "SHP_ORG_HULL"
                 OwnerHasTech name = "SHP_QUANT_ENRG_MAG"
             ])
             + 20 * (If condition = Or [
                 OwnerHasTech name = "SHP_ORG_HULL"
                 OwnerHasTech name = "SHP_QUANT_ENRG_MAG"
             ])
             + 20 * (If condition = OwnerHasTech name = "SHP_QUANT_ENRG_MAG")
             + 10 * (If condition = OwnerHasTech name = "SHP_IMPROVED_ENGINE_COUPLINGS")
             + 10 * (If condition = OwnerHasTech name = "SHP_N_DIMENSIONAL_ENGINE_MATRIX")
             + 10 * (If condition = OwnerHasTech name = "SHP_SINGULARITY_ENGINE_CORE")
             + 10 * (If condition = OwnerHasTech name = "SHP_TRANSSPACE_DRIVE")
             + 10 * (If condition = OwnerHasTech name = "SHP_INTSTEL_LOG")
        )
    )''')

outpath = os.getcwd()
print ("Output folder: %s" % outpath)

with open(os.path.join(outpath, "col_buildings.txt"), "w") as f:
    for species in species_list:
        sp_id = species[0]
        sp_name = sp_id.split("_", 1)[1]
        sp_desc_name = species[1]
        sp_graphic = species[2]
        if sp_id == "SP_EXOBOT":
            f.write(t_main.substitute(id=sp_id, name=sp_name, graphic=sp_graphic, cost=70, time=5,
                    species_condition=r"// no existing Exobot colony required!") + "\n\n")
        elif sp_id == "SP_BANFORO" or sp_id == "SP_KILANDOW" or sp_id == "SP_MISIORLA":
            this_time_factor = time_factor.get(sp_id, "1.0")
            f.write(t_main.substitute(id=sp_id, name=sp_name, graphic=sp_graphic, cost=50,
                                      time=t_buildtime_extinct.substitute(id=sp_id, name=sp_name, t_factor=this_time_factor),
                                      species_condition=t_species_condition_extinct.substitute(id=sp_id, name=sp_name)) + "\n\n")
        else:
            this_time_factor = time_factor.get(sp_id, "1.0")
            f.write(t_main.substitute(id=sp_id, name=sp_name, graphic=sp_graphic, cost=50,
                                      time=t_buildtime.substitute(id=sp_id, t_factor=this_time_factor),
                                      species_condition=t_species_condition.substitute(id=sp_id)) + "\n\n")
