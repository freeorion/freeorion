import os
import os.path
import string


species_list = [
    ("SP_ABADDONI", "Abaddoni", "icons/species/abaddonnian.png"),
    ("SP_CHATO", "Chato", "icons/species/chato-matou-gormoshk.png"),
    ("SP_CRAY", "Cray", "icons/species/cray.png"),
    ("SP_DERTHREAN", "Derthrean", "icons/species/derthrean.png"),
    ("SP_EAXAW", "Eaxaw", "icons/species/eaxaw.png"),
    ("SP_EGASSEM", "Egassem", "icons/species/egassem.png"),
    ("SP_ETTY", "Etty", "icons/species/etty.png"),
    ("SP_GEORGE", "George", "icons/species/george.png"),
    ("SP_GYSACHE", "Gysache", "icons/species/gysache.png"),
    ("SP_HHHOH", "Hhhoh", "icons/species/hhhoh.png"),
    ("SP_HUMAN", "Human", "icons/species/human.png"),
    ("SP_KOBUNTURA", "Kobuntura", "icons/species/intangible-04.png"),
    ("SP_LAENFA", "Laenfa", "icons/species/laenfa.png"),
    ("SP_MUURSH", "Mu Ursh", "icons/species/muursh.png"),
    ("SP_SCYLIOR", "Scylior", "icons/species/scylior.png"),
    ("SP_SETINON", "Setinon", "icons/species/amorphous-02.png"),
    ("SP_SSLITH", "Sslith", "icons/species/sslith.png"),
    ("SP_TAEGHIRUS", "Tae Ghirus", "icons/species/t-aeghirus.png"),
    ("SP_TRITH", "Trith", "icons/species/trith.png"),
    ("SP_UGMORS", "Ugmors", "icons/species/amorphous-06.png"),
    ("SP_EXOBOT", "Exobot", "icons/species/robotic-01.png")
]


# the following currently causes a crash on Linux
#
#     buildtime = max( 5,
#                         min value = JumpsBetween object = target.SystemID object = LocalCandidate.SystemID
#                             condition = And [
#                                             Planet
#                                             OwnedBy empire = Source.Owner
#                                             Species name = "SP_HUMAN"
#                                             Population low = 3
#                                             Happiness low = 5
#                                             ResourceSupplyConnected empire = Source.Owner condition = Target
#                                         ]
#                     )
#
# the following currently work(s) on Linux
#     buildtime = 1 + JumpsBetween object = target.SystemID object = Source.SystemID
#
#     buildtime = max( 5.0 , 2.0 * count                            condition = And [
#                                             Planet
#                                             OwnedBy empire = Source.Owner
#                                             Species name = "SP_HUMAN"
#                                             Population low = 3
#                                             Happiness low = 5
#                                             ResourceSupplyConnected empire = Source.Owner condition = Target
#                                         ]
#                     )


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
            activation = Turn low = Source.CreationTurn + 1 high = Source.CreationTurn + 1
            effects = [
                SetSpecies name = "${id}"
                SetPopulation value = 1
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
    icon = "icons/building/generic_building.png"''')

t_species_condition = string.Template('''ResourceSupplyConnected empire = Source.Owner condition = And [
            Planet
            OwnedBy empire = Source.Owner
            Species name = "${id}"
            Population low = 3
            Happiness low = 5
        ]''')

outpath = os.getcwd()
print "Output folder:", outpath

with open(os.path.join(outpath, "col_buildings.txt"), "w") as f:
    for species in species_list:
        sp_id = species[0]
        sp_name = sp_id.split("_")[1]
        sp_desc_name = species[1]
        sp_graphic = species[2]
        if sp_id == "SP_EXOBOT":
            f.write(t_main.substitute(id=sp_id, name=sp_name, graphic=sp_graphic, cost=70, time=5,
                    species_condition=r"// no existing Exobot colony required!") + "\n\n")
        else:
            f.write(t_main.substitute(id=sp_id, name=sp_name, graphic=sp_graphic, cost=50, time=5,
                    species_condition=t_species_condition.substitute(id=sp_id)) + "\n\n")
