import freeOrionAIInterface as fo  # interface used to interact with FreeOrion AI client  # pylint: disable=import-error

# Note re common dictionary lookup structure, "PlanetSize-Dependent-Lookup":
# Many dictionaries herein (a prime example being the building_supply dictionary) have a primary key (such as
# special_name, or building_type_name), and then provide a sub-dictionary with which to look up the resultant value
# which may in some cases depend upon PlanetSize; if not then the key -1 stands for any planet size.
# So, if the value to be provided by the sub-dictionary does not depend on PlanetSize, then this sub-dictionary
# should be of the form
#    {-1: return_val}
# if the return value *is* dependent on PlanetSize, then the sub-dictionary should be of the form:
#    {fo.planetSize.tiny: return_val_1,
#     fo.planetSize.small: return_val_2,
#     fo.planetSize.medium: return_val_3,
#     fo.planetSize.large: return_val_4,
#     fo.planetSize.huge: return_val_5,
#     fo.planetSize.gasGiant: return_val_6,
#     }
# Please refer to the building_supply dictionary below for an example that displays both of these uses.

#
#  Miscellaneous
#

# TODO look into (enabling) simply retrieving the below values via UserString
INDUSTRY_PER_POP = 0.2

RESEARCH_PER_POP = 0.2

TROOPS_PER_POP = 0.2

TECH_COST_MULTIPLIER = 2.0

#
# Specials details (some specials are instead covered in the section they most directly affect
#
metabolismBoostMap = {
    "ORGANIC": ["FRUIT_SPECIAL", "PROBIOTIC_SPECIAL", "SPICE_SPECIAL"],
    "LITHIC": ["CRYSTALS_SPECIAL", "ELERIUM_SPECIAL", "MINERALS_SPECIAL"],
    "ROBOTIC": ["MONOPOLE_SPECIAL", "POSITRONIUM_SPECIAL", "SUPERCONDUCTOR_SPECIAL"],
    "SELF_SUSTAINING": []
}

metabolismBoosts = {}
for metab, boosts in metabolismBoostMap.items():
    for boost in boosts:
        metabolismBoosts[boost] = metab

HONEYCOMB_IND_MULTIPLIER = 2.5
COMPUTRONIUM_RES_MULTIPLIER = 1.0

#
# Colonization details
#
COLONY_POD_COST = 120
COLONY_POD_UPKEEP = 0.06
OUTPOST_POD_COST = 50
SHIP_UPKEEP = 0.01

OUTPOSTING_TECH = "SHP_GAL_EXPLO"

# Please see the Note at top of this file regarding PlanetSize-Dependent-Lookup
# Regardless of whether the sub-dictionary here has PlanetSize keys, the final
# value will be applied as a *fixed-size mod* to the max population
POP_FIXED_MOD_SPECIALS = {
    'DIM_RIFT_MASTER_SPECIAL': {-1: -4},
}

# Please see the Note at top of this file regarding PlanetSize-Dependent-Lookup
# The return value from the respective sub-dictionary will be applied as a
# population modifier proportional to PlanetSize, i.e.,
#       max_pop += return_val * PlanetSize
# So, most commonly the lookup key for the sub-dictionary here will be
# PlanetSize independent (i.e., -1), although it is possible to use
# PlanetSize keys here for a more complex interaction.
# Regardless of whether the sub-dictionary here has PlanetSize keys, the final
# value will be applied as a fixed-size mod to the max population
POP_PROPORTIONAL_MOD_SPECIALS = {
    'TIDAL_LOCK_SPECIAL': {-1: -1},
    'TEMPORAL_ANOMALY_SPECIAL': {-1: -5},
}

#
#  Supply details
#
supply_range_techs = {"CON_ORBITAL_CON": 1, "CON_CONTGRAV_ARCH": 1, "CON_GAL_INFRA": 1}
supply_by_size = {
    fo.planetSize.tiny: 2,
    fo.planetSize.small: 1,
    fo.planetSize.large: -1,
    fo.planetSize.huge: -2,
    fo.planetSize.gasGiant: -1
}

SUPPLY_MOD_SPECIALS = {
    'WORLDTREE_SPECIAL': {-1: 1},
    'ECCENTRIC_ORBIT_SPECIAL': {-1: -2},
    'ACCRETION_DISC_SPECIAL': {-1: -1},
}

# Please see the Note at top of this file regarding PlanetSize-Dependent-Lookup
# building supply bonuses are keyed by planet size; key -1 stands for any planet size
building_supply = {
    "BLD_IMPERIAL_PALACE": {-1: 2},
    "BLD_MEGALITH": {-1: 2},
    "BLD_SPACE_ELEVATOR": {
        fo.planetSize.tiny: 1,
        fo.planetSize.small: 2,
        fo.planetSize.medium: 3,
        fo.planetSize.large: 4,
        fo.planetSize.huge: 5,
        fo.planetSize.gasGiant: 4,
    },
}

#
# tech names etc.
GRO_LIFE_CYCLE = "GRO_LIFECYCLE_MAN"
PRO_ORBITAL_GEN = "PRO_ORBITAL_GEN"
PRO_SOL_ORB_GEN = "PRO_SOL_ORB_GEN"
PRO_MICROGRAV_MAN = "PRO_MICROGRAV_MAN"
PRO_SINGULAR_GEN = "PRO_SINGULAR_GEN"
PROD_AUTO_NAME = "PRO_SENTIENT_AUTOMATION"
NEST_DOMESTICATION_TECH = "SHP_DOMESTIC_MONSTER"

ART_MINDS = "LRN_ARTIF_MINDS"
LRN_ALGO_ELEGANCE = "LRN_ALGO_ELEGANCE"
GRO_PLANET_ECOL = "GRO_PLANET_ECOL"
LRN_QUANT_NET = "LRN_QUANT_NET"
LRN_XENOARCH = "LRN_XENOARCH"
LRN_ART_BLACK_HOLE = "LRN_ART_BLACK_HOLE"

GRO_XENO_GENETICS = "GRO_XENO_GENETICS"
GRO_GENOME_BANK = "GRO_GENETIC_MED"

CON_CONC_CAMP = "CON_CONC_CAMP"

TECH_EXCLUSION_MAP_1 = {"LRN_TRANSCEND": fo.aggression.typical}  # (k,v) exclude tech k if aggression is less than v
TECH_EXCLUSION_MAP_2 = {}  # (k,v) exclude tech k if aggression is greater than v

FIRST_PLANET_SHIELDS_TECH = "LRN_FORCE_FIELD"
PLANET_BARRIER_I_TECH = "DEF_PLAN_BARRIER_SHLD_1"
DEFENSE_REGEN_1_TECH = "DEF_DEFENSE_NET_REGEN_1"
DEFENSE_DEFENSE_NET_TECHS = ["DEF_DEFENSE_NET_1", "DEF_DEFENSE_NET_2", "DEF_DEFENSE_NET_3"]
DEFENSE_REGEN_TECHS = ["DEF_DEFENSE_NET_REGEN_1", "DEF_DEFENSE_NET_REGEN_2"]
DEFENSE_GARRISON_TECHS = ["DEF_GARRISON_1", "DEF_GARRISON_2", "DEF_GARRISON_3", "DEF_GARRISON_4"]
DEFENSE_SHIELDS_TECHS = [
    "LRN_FORCE_FIELD", "DEF_PLAN_BARRIER_SHLD_1", "DEF_PLAN_BARRIER_SHLD_2",
    "DEF_PLAN_BARRIER_SHLD_3", "DEF_PLAN_BARRIER_SHLD_4", "DEF_PLAN_BARRIER_SHLD_5"]

PROT_FOCUS_MULTIPLIER = 2.0

# TODO obtain this information from techs.txt
UNRESEARCHABLE_TECHS = ("SHP_KRILL_SPAWN", "DEF_PLANET_CLOAK")

UNUSED_TECHS = (
    "LRN_SPATIAL_DISTORT_GEN", "LRN_GATEWAY_VOID", "LRN_PSY_DOM",
    "GRO_TERRAFORM", "GRO_BIOTERROR", "GRO_GAIA_TRANS",
    "PRO_NDIM_ASSMB",
    "CON_ORGANIC_STRC", "CON_PLANET_DRIVE", "CON_STARGATE",
    "CON_ART_HEAVENLY", "CON_ART_PLANET",
    "SHP_NOVA_BOMB",
    "SHP_BOMBARD",
    "SHP_DEATH_SPORE", "SHP_BIOTERM",
    "SHP_EMP", "SHP_EMO",
    "SHP_SONIC", "SHP_GRV",
    "SHP_DARK_RAY", "SHP_VOID_SHADOW",
    "SHP_CHAOS_WAVE"
)

THEORY_TECHS = (
    "LRN_PHYS_BRAIN", "LRN_TRANSLING_THT", "LRN_PSIONICS", "LRN_GRAVITONICS", "LRN_EVERYTHING", "LRN_MIND_VOID",
    "LRN_NDIM_SUBSPACE", "LRN_TIME_MECH",
    "GRO_GENETIC_ENG", "GRO_ADV_ECOMAN", "GRO_NANOTECH_MED", "GRO_TRANSORG_SENT",
    "PRO_NANOTECH_PROD", "PRO_ZERO_GEN",
    "CON_ASYMP_MATS", "CON_ARCH_PSYCH",
    "SHP_GAL_EXPLO"
)

DEFENSE_TECHS_PREFIX = "DEF"

PRODUCTION_BOOST_TECHS = (
    "PRO_ROBOTIC_PROD", "PRO_FUSION_GEN", "PRO_SENTIENT_AUTOMATION",
    "PRO_INDUSTRY_CENTER_I", "PRO_INDUSTRY_CENTER_II", "PRO_INDUSTRY_CENTER_III",
    "PRO_SOL_ORB_GEN"
)

RESEARCH_BOOST_TECHS = (
    "LRN_ALGO_ELEGANCE", "LRN_ARTIF_MINDS", "LRN_DISTRIB_THOUGHT", "LRN_QUANT_NET", "LRN_STELLAR_TOMOGRAPHY",
    "LRN_ENCLAVE_VOID"
)

PRODUCTION_AND_RESEARCH_BOOST_TECHS = ("LRN_UNIF_CONC", "GRO_ENERGY_META")

POPULATION_BOOST_TECHS = (
    "GRO_PLANET_ECOL", "GRO_SYMBIOTIC_BIO", "GRO_XENO_HYBRIDS", "GRO_CYBORG", "GRO_SUBTER_HAB",
    "CON_ORBITAL_HAB", "CON_NDIM_STRC", "PRO_EXOBOTS"
)

# important that the easiest-to-reach supply tech be listed first
SUPPLY_BOOST_TECHS = ("CON_ORBITAL_CON", "CON_ARCH_MONOFILS", "CON_GAL_INFRA", "CON_CONTGRAV_ARCH")

METER_CHANGE_BOOST_TECHS = ("CON_FRC_ENRG_STRC", "CON_TRANS_ARCH")

DETECTION_TECHS = (
    "SPY_DETECT_1", "SPY_DETECT_2", "SPY_DETECT_3", "SPY_DETECT_4",
    "SPY_DETECT_5", "SPY_DIST_MOD", "SPY_LIGHTHOUSE"
)

STEALTH_TECHS = ("SPY_STEALTH_1", "SPY_STEALTH_2", "SPY_STEALTH_3", "SPY_STEALTH_4", "CON_FRC_ENRG_CAMO")

ROBOTIC_HULL_TECHS = (
    "SHP_MIL_ROBO_CONT", "SHP_SPACE_FLUX_DRIVE", "SHP_TRANSSPACE_DRIVE", "SHP_CONTGRAV_MAINT",
    "SHP_MASSPROP_SPEC", "SHP_NANOROBO_MAINT", "SHP_MIDCOMB_LOG"
)

ASTEROID_HULL_TECHS = (
    "SHP_ASTEROID_HULLS", "SHP_SCAT_AST_HULL",
    "SHP_HEAVY_AST_HULL", "SHP_CAMO_AST_HULL", "SHP_MINIAST_SWARM"
)

ORGANIC_HULL_TECHS = (
    "SHP_ORG_HULL", "SHP_MULTICELL_CAST", "SHP_ENDOCRINE_SYSTEMS", "SHP_CONT_BIOADAPT",
    "SHP_MONOCELL_EXP", "SHP_CONT_SYMB", "SHP_BIOADAPTIVE_SPEC", "SHP_ENDOSYMB_HULL", "SHP_SENT_HULL"
)

ENERGY_HULL_TECHS = ("SHP_FRC_ENRG_COMP", "SHP_QUANT_ENRG_MAG", "SHP_ENRG_BOUND_MAN", "SHP_SOLAR_CONT")

MISC_HULL_TECHS = ("SHP_XENTRONIUM_HULL",)

HULL_TECHS = tuple(hull for hulls in (ROBOTIC_HULL_TECHS,
                                      ASTEROID_HULL_TECHS,
                                      ORGANIC_HULL_TECHS,
                                      ENERGY_HULL_TECHS,
                                      MISC_HULL_TECHS) for hull in hulls)

DAMAGE_CONTROL_TECHS = ("SHP_BASIC_DAM_CONT", "SHP_FLEET_REPAIR", "SHP_ADV_DAM_CONT")

WEAPON_PREFIX = "SHP_WEAPON"
ARMOR_TECHS = (
    "SHP_ZORTRIUM_PLATE", "SHP_DIAMOND_PLATE", "SHP_XENTRONIUM_PLATE",
    "SHP_ASTEROID_REFORM", "SHP_MONOMOLEC_LATTICE", "PRO_NEUTRONIUM_EXTRACTION", "SHP_REINFORCED_HULL"
)
ENGINE_TECHS = ("SHP_IMPROVED_ENGINE_COUPLINGS", "SHP_N_DIMENSIONAL_ENGINE_MATRIX", "SHP_SINGULARITY_ENGINE_CORE")
FUEL_TECHS = ("SHP_DEUTERIUM_TANK", "SHP_ANTIMATTER_TANK", "SHP_ZERO_POINT")
SHIELD_TECHS = (
    "LRN_FORCE_FIELD", "SHP_DEFLECTOR_SHIELD", "SHP_PLASMA_SHIELD", "SHP_BLACKSHIELD", "SHP_MULTISPEC_SHIELD"
)
COLONY_POD_TECHS = ("GRO_LIFECYCLE_MAN",)
TROOP_POD_TECHS = ("GRO_NANO_CYBERNET",)

SHIP_TECHS_REQUIRING_BLACK_HOLE = ("SHP_SOLAR_CONT",)

# ship facilities info, dict keyed by building name, value is (min_aggression, prereq_bldg, base_cost, time)
# not currently determined dynamically because it is initially used in a location-independent fashion
# note that BLD_SHIPYARD_BASE is not an absolute prereq for BLD_NEUTRONIUM_FORGE, but is a practical one
SHIP_FACILITIES = {
    "BLD_SHIPYARD_BASE": (0, "", 10, 4),
    "BLD_SHIPYARD_ORBITAL_DRYDOCK": (0, "BLD_SHIPYARD_BASE", 20, 5),
    "BLD_SHIPYARD_CON_NANOROBO": (fo.aggression.aggressive, "BLD_SHIPYARD_ORBITAL_DRYDOCK", 250, 5),
    "BLD_SHIPYARD_CON_GEOINT": (fo.aggression.aggressive, "BLD_SHIPYARD_ORBITAL_DRYDOCK", 750, 5),
    "BLD_SHIPYARD_CON_ADV_ENGINE": (0, "BLD_SHIPYARD_ORBITAL_DRYDOCK", 500, 5),
    "BLD_SHIPYARD_AST": (fo.aggression.typical, "", 75, 5),
    "BLD_SHIPYARD_AST_REF": (fo.aggression.maniacal, "BLD_SHIPYARD_AST", 500, 5),
    "BLD_SHIPYARD_ORG_ORB_INC": (0, "BLD_SHIPYARD_BASE", 40, 8),
    "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB": (fo.aggression.aggressive, "BLD_SHIPYARD_ORG_ORB_INC", 64, 8),
    "BLD_SHIPYARD_ORG_XENO_FAC": (fo.aggression.aggressive, "BLD_SHIPYARD_ORG_ORB_INC", 120, 8),
    "BLD_SHIPYARD_ENRG_COMP": (fo.aggression.aggressive, "BLD_SHIPYARD_BASE", 200, 5),
    "BLD_SHIPYARD_ENRG_SOLAR": (fo.aggression.maniacal, "BLD_SHIPYARD_ENRG_COMP", 1200, 5),
    "BLD_NEUTRONIUM_FORGE": (fo.aggression.cautious, "BLD_SHIPYARD_BASE", 100, 3),
}

# those facilities that need merely be in-system
SYSTEM_SHIP_FACILITIES = frozenset(("BLD_SHIPYARD_AST", "BLD_SHIPYARD_AST_REF"))

PART_KRILL_SPAWNER = "SP_KRILL_SPAWNER"

# known tokens the AI can handle
REPAIR_PER_TURN = "REPAIR_PER_TURN"
FUEL_PER_TURN = "FUEL_PER_TURN"
STEALTH_MODIFIER = "STEALTH_MODIFIER"
ASTEROID_STEALTH = "ASTEROID_STEALTH"
SOLAR_STEALTH = "SOLAR_STEALTH"
SPEED = "SPEED"
FUEL = "FUEL"
SHIELDS = "SHIELDS"
STRUCTURE = "STRUCTURE"
DETECTION = "DETECTION"  # do only specify for hulls if irregular detection
ORGANIC_GROWTH = "ORGANIC_GROWTH"  # structure for value is (per_turn, maximum)
STACKING_RULES = "STACKING_RULES"  # expects a list of stacking rules
# stacking rules
NO_EFFECT_WITH_CLOAKS = "NO_EFFECT_WITH_CLOAKS"

BASE_DETECTION = 25

TECH_EFFECTS = {
    # "TECHNAME": { Token1: Value1, Token2: Value2, ...}
    "SHP_REINFORCED_HULL": {STRUCTURE: 5},
    "SHP_BASIC_DAM_CONT": {REPAIR_PER_TURN: 1},
    "SHP_FLEET_REPAIR": {REPAIR_PER_TURN: (STRUCTURE, 0.1)},  # 10% of max structure
    "SHP_ADV_DAM_CONT": {REPAIR_PER_TURN: (STRUCTURE, 0.1)},  # 10% of max structure
    "SHP_INTSTEL_LOG": {SPEED: 20},  # technically not correct, but as approximation good enough...
    "GRO_ENERGY_META": {FUEL: 2}
}

HULL_EFFECTS = {
    # "HULLNAME": { Token1: Value1, Token2: Value2, ...}
    # Robotic line
    "SH_ROBOTIC": {REPAIR_PER_TURN: 2},
    "SH_SPATIAL_FLUX": {STEALTH_MODIFIER: -30},
    "SH_NANOROBOTIC": {REPAIR_PER_TURN: (STRUCTURE, 1)},  # 100% of max structure
    "SH_LOGISTICS_FACILITATOR": {REPAIR_PER_TURN: (STRUCTURE, 1)},  # 100% of max structure
    # Asteroid line
    "SH_SMALL_ASTEROID": {ASTEROID_STEALTH: 20},
    "SH_ASTEROID": {ASTEROID_STEALTH: 20},
    "SH_HEAVY_ASTEROID": {ASTEROID_STEALTH: 20},
    "SH_SMALL_CAMOUFLAGE_ASTEROID": {ASTEROID_STEALTH: 20},
    "SH_CAMOUFLAGE_ASTEROID": {ASTEROID_STEALTH: 40},
    "SH_CRYSTALLIZED_ASTEROID": {ASTEROID_STEALTH: 20},
    "SH_MINIASTEROID_SWARM": {ASTEROID_STEALTH: 20, SHIELDS: 5},
    "SH_SCATTERED_ASTEROID": {ASTEROID_STEALTH: 40, SHIELDS: 3},
    # Organic line
    "SH_ORGANIC": {REPAIR_PER_TURN: 2, FUEL_PER_TURN: 0.2, DETECTION: 10, ORGANIC_GROWTH: (0.2, 5)},
    "SH_ENDOMORPHIC": {DETECTION: 50, ORGANIC_GROWTH: (0.5, 15)},
    "SH_SYMBIOTIC": {REPAIR_PER_TURN: 2, FUEL_PER_TURN: 0.2, DETECTION: 50, ORGANIC_GROWTH: (0.2, 10)},
    "SH_PROTOPLASMIC": {REPAIR_PER_TURN: 2, FUEL_PER_TURN: 0.2, DETECTION: 50, ORGANIC_GROWTH: (0.5, 25)},
    "SH_ENDOSYMBIOTIC": {REPAIR_PER_TURN: 2, FUEL_PER_TURN: 0.2, DETECTION: 50, ORGANIC_GROWTH: (0.5, 15)},
    "SH_RAVENOUS": {DETECTION: 75, ORGANIC_GROWTH: (0.5, 20)},
    "SH_BIOADAPTIVE": {REPAIR_PER_TURN: (STRUCTURE, 1), FUEL_PER_TURN: 0.2,
                       DETECTION: 75, ORGANIC_GROWTH: (0.5, 25)},
    "SH_SENTIENT": {REPAIR_PER_TURN: 2, FUEL_PER_TURN: 0.2, DETECTION: 70,
                    ORGANIC_GROWTH: (1, 45), STEALTH_MODIFIER: 20},
    # Energy Line
    "SH_SOLAR": {SOLAR_STEALTH: 120, FUEL_PER_TURN: (FUEL, 1)}  # 100% of fuel
}

PART_EFFECTS = {
    # "PARTNAME": { Token1: Value1, Token2: Value2, ...}
    "SH_MULTISPEC": {SOLAR_STEALTH: 60},
    "FU_TRANSPATIAL_DRIVE": {},  # not supported yet
    "FU_RAMSCOOP": {FUEL_PER_TURN: 0.1},
    "FU_ZERO_FUEL": {FUEL_PER_TURN: (FUEL, 1)},  # 100% of fuel
    "SP_DISTORTION_MODULATOR": {},  # not supported yet
    "SH_ROBOTIC_INTERFACE_SHIELDS": {},  # not supported yet
    PART_KRILL_SPAWNER: {STEALTH_MODIFIER: 40, STACKING_RULES: [NO_EFFECT_WITH_CLOAKS]}
}

WEAPON_UPGRADE_DICT = {
    # "PARTNAME": tuple([  (tech_name, dmg_upgrade), (tech_name2, dmg_upgrade2), ... ])
    "SR_WEAPON_1_1": tuple([("SHP_WEAPON_1_%d" % i, 1) for i in [2, 3, 4]]),
    "SR_WEAPON_2_1": tuple([("SHP_WEAPON_2_%d" % i, 2) for i in [2, 3, 4]]),
    "SR_WEAPON_3_1": tuple([("SHP_WEAPON_3_%d" % i, 3) for i in [2, 3, 4]]),
    "SR_WEAPON_4_1": tuple([("SHP_WEAPON_4_%d" % i, 5) for i in [2, 3, 4]]),
    "SR_WEAPON_0_1": tuple([]),
    "SR_SPINAL_ANTIMATTER": tuple([])
}

PILOT_DAMAGE_MODIFIER_DICT = {
    # TRAIT:    {weapon_name: effect, weapon_name2: effect2,...}
    "NO":       {},
    "BAD":      {"SR_WEAPON_1_1": -1, "SR_WEAPON_2_1": -2, "SR_WEAPON_3_1": -3, "SR_WEAPON_4_1": -5},
    "GOOD":     {"SR_WEAPON_1_1":  1, "SR_WEAPON_2_1":  2, "SR_WEAPON_3_1":  3, "SR_WEAPON_4_1": 5},
    "GREAT":    {"SR_WEAPON_1_1":  2, "SR_WEAPON_2_1":  4, "SR_WEAPON_3_1":  6, "SR_WEAPON_4_1": 10},
    "ULTIMATE": {"SR_WEAPON_1_1":  3, "SR_WEAPON_2_1":  6, "SR_WEAPON_3_1":  9, "SR_WEAPON_4_1": 15, "SR_WEAPON_0_1": 1},
}

PILOT_FIGHTERDAMAGE_MODIFIER_DICT = {
    # TRAIT:    {hangar_name: effect, hangar_name2: effect2,...}
    "NO":       {},
    "BAD":      {"FT_HANGAR_1": -1, "FT_HANGAR_2": -2, "FT_HANGAR_3": -3, "FT_HANGAR_4": -4},
    "GOOD":     {"FT_HANGAR_1":  1, "FT_HANGAR_2":  2, "FT_HANGAR_3":  3, "FT_HANGAR_4": 4},
    "GREAT":    {"FT_HANGAR_1":  2, "FT_HANGAR_2":  4, "FT_HANGAR_3":  6, "FT_HANGAR_4": 8},
    "ULTIMATE": {"FT_HANGAR_1":  3, "FT_HANGAR_2":  6, "FT_HANGAR_3":  9, "FT_HANGAR_4": 12},
}

PILOT_ROF_MODIFIER_DICT = {
    # TRAIT:    {weapon_name: effect, weapon_name2: effect2,...}
    "NO":       {},
    "BAD":      {"SR_WEAPON_0_1": -1},
    "GOOD":     {"SR_WEAPON_0_1": 1},
    "GREAT":    {"SR_WEAPON_0_1": 2},
    "ULTIMATE": {"SR_WEAPON_0_1": 3},
}

# DO NOT TOUCH THIS ENTRY BUT UPDATE WEAPON_UPGRADE_DICT INSTEAD!
WEAPON_UPGRADE_TECHS = [tech_name for tups in WEAPON_UPGRADE_DICT.values() for (tech_name, _) in tups]
