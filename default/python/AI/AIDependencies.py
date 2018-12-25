"""
This module stores information about game rules and content that can not be dynamically accessed.

If other modules rely on information regarding the game content that can not be queried from the server and thus
 needs to be hardcoded, the information should be implemented here and referenced by the other module.

The game content is sorted by categories:
    - Basic Game Rules
    - Species
    - Specials
    - Techs
    - Buildings
    - Shipdesign / -parts
Each category is wrapped by <editor-fold desc="CATEGORY">   </editor-fold> tags which allow code-folding in IDE.
Categories may have subcategories which are also wrapped by these tags. When adding new content, try to keep it
structured in the aforementioned way. Use the category names to find the appropriate section of the file.

Example usage:
    import AIDependencies
    my_industry = AIDependencies.INDUSTRY_PER_POP * my_population
"""
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

# <editor-fold desc="Basic Game Rules">
# TODO look into (enabling) simply retrieving the below values via UserString
INDUSTRY_PER_POP = 0.2
RESEARCH_PER_POP = 0.2
TROOPS_PER_POP = 0.2

PROT_FOCUS_MULTIPLIER = 2.0
TECH_COST_MULTIPLIER = 2.0
FOCUS_CHANGE_PENALTY = 1

# Colonization details
COLONY_POD_COST = 120  # TODO Query directly from part
OUTPOST_POD_COST = 50  # TODO Query directly from part
COLONY_POD_UPKEEP = 0.06
SHIP_UPKEEP = 0.01

#  Supply details
supply_by_size = {
    fo.planetSize.tiny: 2,
    fo.planetSize.small: 1,
    fo.planetSize.large: -1,
    fo.planetSize.huge: -2,
    fo.planetSize.gasGiant: -1,
}

# Drydock repair details
DRYDOCK_HAPPINESS_THRESHOLD = 5

# Constants defined by the C++ game engine
INVALID_ID = -1
ALL_EMPIRES = -1

# </editor-fold>


# <editor-fold desc="Environmental population modifiers">
#  Population modifiers that are based on environment

# Population modifiers scaling with planet size
#  - [[EARLY_PRIORITY]]: Affected by species modifiers
POP_SIZE_MOD_MAP_MODIFIED_BY_SPECIES = {
    # "Effect": [uninhab, hostile, poor, adequate, good]
    "environment_bonus": [0, -4, -2, 0, 3],
    "GRO_SYMBIOTIC_BIO": [0,  0,  1, 1, 1],
    "GRO_XENO_GENETICS": [0,  1,  2, 2, 0],
    "GRO_XENO_HYBRIDS":   [0,  2,  1, 0, 0],
    "GRO_CYBORG":        [0,  2,  0, 0, 0],
}

# Population modifiers scaling with planet size
#  - [[LATE_PRIORITY]] or later:
POP_SIZE_MOD_MAP_NOT_MODIFIED_BY_SPECIES = {
    "GRO_SUBTER_HAB":  [0, 1, 1, 1, 1],
    "CON_NDIM_STRC":  [0, 2, 2, 2, 2],
    "CON_ORBITAL_HAB": [0, 1, 1, 1, 1],
}

# flat boni not dependent on size
POP_CONST_MOD_MAP = {
    # "Source": [uninhab, hostile, poor, adequate, good]
    "GRO_PLANET_ECOL":   [0, 0, 0,  1,  1],
    "GRO_SYMBIOTIC_BIO": [0, 0, 0, -1, -1],
}

# phototrophic star coefficients
POP_MOD_PHOTOTROPHIC_STAR_MAP = {fo.starType.blue: 3, fo.starType.white: 1.5, fo.starType.red: -1,
                                 fo.starType.neutron: -1, fo.starType.blackHole: -10, fo.starType.noStar: -10}

# lightsensitive star coefficients
TAG_LIGHT_SENSITIVE = "LIGHT_SENSITIVE"
POP_MOD_LIGHTSENSITIVE_STAR_MAP = {fo.starType.blue: -2, fo.starType.white: -1}

# the percentage of normal population that a species with Gaseous tag has
GASEOUS_POP_FACTOR = 0.50

# </editor-fold>

# <editor-fold desc="Detection Strengths">
DETECTION_TECH_STRENGTHS = {
    "SPY_DETECT_1": 10,
    "SPY_DETECT_2": 30,
    "SPY_DETECT_3": 50,
    "SPY_DETECT_4": 70,
    "SPY_DETECT_5": 200,
}
# </editor-fold>

# <editor-fold desc="Stealth Strengths">
PRIMARY_FOCS_STEALTH_LEVELS = {
    "LOW_STEALTH": 20,
    "MEDIUM_STEALTH": 40,
    "HIGH_STEALTH": 60,
    "VERY_HIGH_STEALTH": 80,
}

STEALTH_SPECIAL_STRENGTHS = {
    "CLOUD_COVER_SLAVE_SPECIAL": PRIMARY_FOCS_STEALTH_LEVELS["LOW_STEALTH"],
    "VOLCANIC_ASH_SLAVE_SPECIAL": PRIMARY_FOCS_STEALTH_LEVELS["MEDIUM_STEALTH"],
    "DIM_RIFT_SLAVE_SPECIAL": PRIMARY_FOCS_STEALTH_LEVELS["HIGH_STEALTH"],
    "VOID_SLAVE_SPECIAL": PRIMARY_FOCS_STEALTH_LEVELS["VERY_HIGH_STEALTH"],
}

BASE_PLANET_STEALTH = 5

STEALTH_STRENGTHS_BY_SPECIES_TAG = {
    "BAD_STEALTH": -PRIMARY_FOCS_STEALTH_LEVELS["LOW_STEALTH"],
    "GOOD_STEALTH": PRIMARY_FOCS_STEALTH_LEVELS["LOW_STEALTH"],
    "GREAT_STEALTH": PRIMARY_FOCS_STEALTH_LEVELS["MEDIUM_STEALTH"],
    "ULTIMATE_STEALTH": PRIMARY_FOCS_STEALTH_LEVELS["HIGH_STEALTH"],
    "INFINITE_STEALTH": 500,  # used by super testers
}
# </editor-fold>

# <editor-fold desc="Specials">
# <editor-fold desc="Growth Focus specials">
# stores growth special per metabolism
metabolismBoostMap = {
    "ORGANIC": [
        "FRUIT_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "SPICE_SPECIAL",
    ],
    "LITHIC": [
        "CRYSTALS_SPECIAL",
        "ELERIUM_SPECIAL",
        "MINERALS_SPECIAL",
    ],
    "ROBOTIC": [
        "MONOPOLE_SPECIAL",
        "POSITRONIUM_SPECIAL",
        "SUPERCONDUCTOR_SPECIAL",
    ],
    "SELF_SUSTAINING": [

    ],
}

# stores which metabolism profits from each special - autogenerated DO NOT EDIT
metabolismBoosts = {}
for metab, boosts in metabolismBoostMap.items():
    for boost in boosts:
        metabolismBoosts[boost] = metab
# </editor-fold>

# <editor-fold desc="Other Population changing specials">
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
    'TIDAL_LOCK_SPECIAL': {
        -1: -1,
    },
    'TEMPORAL_ANOMALY_SPECIAL': {
        -1: -5,
    },
}
# </editor-fold>

# <editor-fold desc="Industry related specials">
HONEYCOMB_IND_MULTIPLIER = 2.5
# </editor-fold>

# <editor-fold desc="Research related specials">
COMPUTRONIUM_SPECIAL = "COMPUTRONIUM_SPECIAL"
COMPUTRONIUM_RES_MULTIPLIER = 1.0

ANCIENT_RUINS_SPECIAL = "ANCIENT_RUINS_SPECIAL"
# </editor-fold>

# <editor-fold desc="Supply related specials">
SUPPLY_MOD_SPECIALS = {
    'WORLDTREE_SPECIAL': {
        -1: 1,
    },
    'ECCENTRIC_ORBIT_SPECIAL': {
        -1: -2,
    },
    'ACCRETION_DISC_SPECIAL': {
        -1: -1,
    },
}
# </editor-fold>

# <editor-fold desc="Detection related specials">
PANOPTICON_SPECIAL = "PANOPTICON_SPECIAL"
PANOPTICON_DETECTION_BONUS = 10
# </editor-fold>

# <editor-fold desc="Native Tech/Defense related specials">
# (special_name, defense&shield_bonus)
TECH_NATIVE_SPECIALS = {
    "MODERATE_TECH_NATIVES_SPECIAL": {'defense': 10, 'shield': 10},
    "HIGH_TECH_NATIVES_SPECIAL": {'defense': 30, 'shield': 30},
}
# </editor-fold>


# </editor-fold>


# <editor-fold desc="Techs">
# <editor-fold desc="Various tech names">
# For IDE support and to avoid typos and easier editing, define tech names here and import for scripts
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
GRO_SUBTER_HAB = "GRO_SUBTER_HAB"
LRN_PHYS_BRAIN = "LRN_PHYS_BRAIN"
LRN_QUANT_NET = "LRN_QUANT_NET"
LRN_XENOARCH = "LRN_XENOARCH"
LRN_ART_BLACK_HOLE = "LRN_ART_BLACK_HOLE"

GRO_XENO_GENETICS = "GRO_XENO_GENETICS"
GRO_GENOME_BANK = "GRO_GENETIC_MED"

CON_CONC_CAMP = "CON_CONC_CAMP"

SPY_STEALTH_1 = "SPY_STEALTH_1"
SPY_STEALTH_2 = "SPY_STEALTH_2"

EXOBOT_TECH_NAME = "PRO_EXOBOTS"

# </editor-fold>


# Tech required to build outpost pods
OUTPOSTING_TECH = "SHP_GAL_EXPLO"

# <editor-fold desc="Supply techs">
supply_range_techs = {
    "CON_ORBITAL_CON": 1,
    "CON_CONTGRAV_ARCH": 1,
    "CON_GAL_INFRA": 1,
}
# </editor-fold>

# <editor-fold desc="Industry bossting techs">
# Industry modifiers per population - [[EARLY_PRIORITY]]: Affected by species modifiers
INDUSTRY_EFFECTS_PER_POP_MODIFIED_BY_SPECIES = {
    "PRO_FUSION_GEN": 1.0,
    "GRO_ENERGY_META": 1.0,
    "PRO_INDUSTRY_CENTER_I": 1.0,
    "PRO_INDUSTRY_CENTER_II": 1.0,
    "PRO_INDUSTRY_CENTER_III": 1.0,
}
# Industry modifiers per population - [[LATE_PRIORITY]] or later: Not affected by species modifiers
INDUSTRY_EFFECTS_PER_POP_NOT_MODIFIED_BY_SPECIES = {
    "PRO_ROBOTIC_PROD": 0.5,
    "PRO_SOL_ORB_GEN": 2.0,  # TODO don't assume will build a gen at a blue/white star
    PRO_SINGULAR_GEN: 5.0,
}
# </editor-fold>


# <editor-fold desc="Defense techs">
# TODO are these 3 necessary? Could rely on ordered DEFENSE_SHIELD_TECHS (Morlic)
FIRST_PLANET_SHIELDS_TECH = "LRN_FORCE_FIELD"
PLANET_BARRIER_I_TECH = "DEF_PLAN_BARRIER_SHLD_1"
DEFENSE_REGEN_1_TECH = "DEF_DEFENSE_NET_REGEN_1"

DEFENSE_DEFENSE_NET_TECHS = ["DEF_DEFENSE_NET_1", "DEF_DEFENSE_NET_2", "DEF_DEFENSE_NET_3"]
DEFENSE_REGEN_TECHS = [
    "DEF_DEFENSE_NET_REGEN_1",
    "DEF_DEFENSE_NET_REGEN_2",
]
DEFENSE_GARRISON_TECHS = [
    "DEF_GARRISON_1",
    "DEF_GARRISON_2",
    "DEF_GARRISON_3",
    "DEF_GARRISON_4",
]
DEFENSE_SHIELDS_TECHS = [
    "LRN_FORCE_FIELD",
    "DEF_PLAN_BARRIER_SHLD_1",
    "DEF_PLAN_BARRIER_SHLD_2",
    "DEF_PLAN_BARRIER_SHLD_3",
    "DEF_PLAN_BARRIER_SHLD_4",
    "DEF_PLAN_BARRIER_SHLD_5",
]
# </editor-fold>

# <editor-fold desc="Weapon techs">
# TODO (Morlic): Consider using only 1 dict with (capacity, secondaryStat) tuple as entries
WEAPON_UPGRADE_DICT = {
    # "PARTNAME": tuple((tech_name, dmg_upgrade), (tech_name2, dmg_upgrade2), ...)
    "SR_WEAPON_0_1": (),
    "SR_WEAPON_1_1": tuple(("SHP_WEAPON_1_%d" % i, 1) for i in [2, 3, 4]),
    "SR_WEAPON_2_1": tuple(("SHP_WEAPON_2_%d" % i, 2) for i in [2, 3, 4]),
    "SR_WEAPON_3_1": tuple(("SHP_WEAPON_3_%d" % i, 3) for i in [2, 3, 4]),
    "SR_WEAPON_4_1": tuple(("SHP_WEAPON_4_%d" % i, 5) for i in [2, 3, 4]),
    "SR_SPINAL_ANTIMATTER": (),
}

WEAPON_ROF_UPGRADE_DICT = {
    # "PARTNAME": tuple((tech_name, rof_upgrade), (tech_name2, rof_upgrade2), ...)
    "SR_WEAPON_0_1": (),
    "SR_WEAPON_1_1": (),
    "SR_WEAPON_2_1": (),
    "SR_WEAPON_3_1": (),
    "SR_WEAPON_4_1": (),
    "SR_SPINAL_ANTIMATTER": (),
}

FIGHTER_DAMAGE_UPGRADE_DICT = {
    # "PARTNAME": tuple((tech_name, dmg_upgrade), (tech_name2, dmg_upgrade2), ...)
    "FT_HANGAR_1": (("SHP_FIGHTERS_2", 1), ("SHP_FIGHTERS_3", 1), ("SHP_FIGHTERS_4", 1)),
    "FT_HANGAR_2": (("SHP_FIGHTERS_2", 2), ("SHP_FIGHTERS_3", 3), ("SHP_FIGHTERS_4", 5)),
    "FT_HANGAR_3": (("SHP_FIGHTERS_2", 3), ("SHP_FIGHTERS_3", 4), ("SHP_FIGHTERS_4", 7)),
    "FT_HANGAR_4": (),
}

FIGHTER_CAPACITY_UPGRADE_DICT = {
    # "PARTNAME": tuple((tech_name, capacity_upgrade), (tech_name2, capacity_upgrade2), ...)
    "FT_HANGAR_1": (),
    "FT_HANGAR_2": (),
    "FT_HANGAR_3": (),
    "FT_HANGAR_4": (),
}

# DO NOT TOUCH THIS ENTRY BUT UPDATE WEAPON_UPGRADE_DICT INSTEAD!
WEAPON_UPGRADE_TECHS = [tech_name for tups in WEAPON_UPGRADE_DICT.values() for (tech_name, _) in tups]
# </editor-fold>

# <editor-fold desc="Tech Groups for Automated Research Approach">

# TODO: I don't think this should be in AIDependencies... (Morlic)
# (k,v) exclude tech k if aggression is less than v
TECH_EXCLUSION_MAP_1 = {
    "LRN_TRANSCEND": fo.aggression.typical,
}
# (k,v) exclude tech k if aggression is greater than v
TECH_EXCLUSION_MAP_2 = {
}

# TODO obtain this information from techs
UNRESEARCHABLE_TECHS = (
    "SHP_KRILL_SPAWN",
    "DEF_PLANET_CLOAK",
    "TECH_COL_BANFORO",
    "TECH_COL_KILANDOW",
    "TECH_COL_MISIORLA",
    "SPY_DETECT_1",
    "SPY_PLANET_STEALTH_MOD",
)

UNUSED_TECHS = (
    "LRN_SPATIAL_DISTORT_GEN",
    "LRN_GATEWAY_VOID",
    "LRN_PSY_DOM",
    "GRO_TERRAFORM",
    "GRO_BIOTERROR",
    "GRO_GAIA_TRANS",
    "PRO_NDIM_ASSMB",
    "CON_ORGANIC_STRC",
    "CON_PLANET_DRIVE",
    "CON_STARGATE",
    "CON_ART_HEAVENLY",
    "CON_ART_PLANET",
    "SHP_NOVA_BOMB",
    "SHP_BOMBARD",
    "SHP_DEATH_SPORE",
    "SHP_BIOTERM",
    "SHP_EMP",
    "SHP_EMO",
    "SHP_SONIC",
    "SHP_GRV",
    "SHP_DARK_RAY",
    "SHP_VOID_SHADOW",
    "SHP_CHAOS_WAVE",
)

THEORY_TECHS = (
    "LRN_TRANSLING_THT",
    "LRN_PSIONICS",
    "LRN_GRAVITONICS",
    "LRN_EVERYTHING",
    "LRN_MIND_VOID",
    "LRN_NDIM_SUBSPACE",
    "LRN_TIME_MECH",
    "GRO_GENETIC_ENG",
    "GRO_ADV_ECOMAN",
    "GRO_NANOTECH_MED",
    "GRO_TRANSORG_SENT",
    "PRO_NANOTECH_PROD",
    "PRO_ZERO_GEN",
    "CON_ASYMP_MATS",
    "CON_ARCH_PSYCH",
    "SHP_GAL_EXPLO",
)

DEFENSE_TECHS_PREFIX = "DEF"

PRODUCTION_BOOST_TECHS = (
    "PRO_ROBOTIC_PROD",
    "PRO_FUSION_GEN",
    "PRO_SENTIENT_AUTOMATION",
    "PRO_INDUSTRY_CENTER_I",
    "PRO_INDUSTRY_CENTER_II",
    "PRO_INDUSTRY_CENTER_III",
    "PRO_SOL_ORB_GEN",
)
RESEARCH_BOOST_TECHS = (
    "LRN_ALGO_ELEGANCE",
    "LRN_ARTIF_MINDS",
    "LRN_PHYS_BRAIN",
    "LRN_DISTRIB_THOUGHT",
    "LRN_QUANT_NET",
    "LRN_STELLAR_TOMOGRAPHY",
    "LRN_ENCLAVE_VOID",
)
PRODUCTION_AND_RESEARCH_BOOST_TECHS = (
    "LRN_UNIF_CONC",
    "GRO_ENERGY_META"
)
POPULATION_BOOST_TECHS = (
    "GRO_PLANET_ECOL",
    "GRO_SYMBIOTIC_BIO",
    "GRO_XENO_HYBRIDS",
    "GRO_CYBORG",
    "GRO_SUBTER_HAB",
    "CON_ORBITAL_HAB",
    "CON_NDIM_STRC",
    "PRO_EXOBOTS",
)
# important that the easiest-to-reach supply tech be listed first
SUPPLY_BOOST_TECHS = (
    "CON_ORBITAL_CON",
    "CON_ARCH_MONOFILS",
    "CON_GAL_INFRA",
    "CON_CONTGRAV_ARCH",
)
METER_CHANGE_BOOST_TECHS = (
    "CON_FRC_ENRG_STRC",
    "CON_TRANS_ARCH",
)
DETECTION_TECHS = (
    "SPY_DETECT_1",
    "SPY_DETECT_2",
    "SPY_DETECT_3",
    "SPY_DETECT_4",
    "SPY_DETECT_5",
    "SPY_DIST_MOD",
    "SPY_LIGHTHOUSE",
)
STEALTH_TECHS = (
    "SPY_STEALTH_1",
    "SPY_STEALTH_2",
    "SPY_STEALTH_3",
    "SPY_STEALTH_4",
    "CON_FRC_ENRG_CAMO",
)
ROBOTIC_HULL_TECHS = (
    "SHP_MIL_ROBO_CONT",
    "SHP_SPACE_FLUX_DRIVE",
    "SHP_TRANSSPACE_DRIVE",
    "SHP_CONTGRAV_MAINT",
    "SHP_MASSPROP_SPEC",
    "SHP_NANOROBO_MAINT",
    "SHP_MIDCOMB_LOG",
)
ASTEROID_HULL_TECHS = (
    "SHP_ASTEROID_HULLS",
    "SHP_SCAT_AST_HULL",
    "SHP_HEAVY_AST_HULL",
    "SHP_CAMO_AST_HULL",
    "SHP_MINIAST_SWARM",
)
ORGANIC_HULL_TECHS = (
    "SHP_ORG_HULL",
    "SHP_MULTICELL_CAST",
    "SHP_ENDOCRINE_SYSTEMS",
    "SHP_CONT_BIOADAPT",
    "SHP_MONOCELL_EXP",
    "SHP_CONT_SYMB",
    "SHP_BIOADAPTIVE_SPEC",
    "SHP_ENDOSYMB_HULL",
    "SHP_SENT_HULL",
)
ENERGY_HULL_TECHS = (
    "SHP_FRC_ENRG_COMP",
    "SHP_QUANT_ENRG_MAG",
    "SHP_ENRG_BOUND_MAN",
    "SHP_SOLAR_CONT",
)
MISC_HULL_TECHS = (
    "SHP_XENTRONIUM_HULL",
)
HULL_TECHS = tuple(hull for hulls in (ROBOTIC_HULL_TECHS,
                                      ASTEROID_HULL_TECHS,
                                      ORGANIC_HULL_TECHS,
                                      ENERGY_HULL_TECHS,
                                      MISC_HULL_TECHS) for hull in hulls)

DAMAGE_CONTROL_TECHS = (
    "SHP_BASIC_DAM_CONT",
    "SHP_FLEET_REPAIR",
    "SHP_ADV_DAM_CONT",
)

WEAPON_PREFIX = "SHP_WEAPON"

ARMOR_TECHS = (
    "SHP_ZORTRIUM_PLATE",
    "SHP_DIAMOND_PLATE",
    "SHP_XENTRONIUM_PLATE",
    "SHP_ASTEROID_REFORM",
    "SHP_MONOMOLEC_LATTICE",
    "PRO_NEUTRONIUM_EXTRACTION",
    "SHP_REINFORCED_HULL",
)
ENGINE_TECHS = (
    "SHP_IMPROVED_ENGINE_COUPLINGS",
    "SHP_N_DIMENSIONAL_ENGINE_MATRIX",
    "SHP_SINGULARITY_ENGINE_CORE",
)
FUEL_TECHS = (
    "SHP_DEUTERIUM_TANK",
    "SHP_ANTIMATTER_TANK",
    "SHP_ZERO_POINT",
)
SHIELD_TECHS = (
    "LRN_FORCE_FIELD",
    "SHP_DEFLECTOR_SHIELD",
    "SHP_PLASMA_SHIELD",
    "SHP_BLACKSHIELD",
    "SHP_MULTISPEC_SHIELD",
)
COLONY_POD_TECHS = (
    "GRO_LIFECYCLE_MAN",
)
TROOP_POD_TECHS = (
    "GRO_NANO_CYBERNET",
)

SHIP_TECHS_REQUIRING_BLACK_HOLE = (
    "SHP_SOLAR_CONT",
)

# </editor-fold>

# </editor-fold>


# <editor-fold desc="Species">
# species names
SP_LAMBALALAM = "SP_LEMBALALAM"

# Species modifiers
SPECIES_RESEARCH_MODIFIER = {'NO': 0.0, 'BAD': 0.75, 'GOOD': 1.5, 'GREAT': 2.0, 'ULTIMATE': 3.0}
SPECIES_INDUSTRY_MODIFIER = {'NO': 0.0, 'BAD': 0.75, 'GOOD': 1.5, 'GREAT': 2.0, 'ULTIMATE': 3.0}
SPECIES_POPULATION_MODIFIER = {'BAD': 0.75, 'GOOD': 1.25}
SPECIES_SUPPLY_MODIFIER = {'VERY_BAD': -1, 'BAD': 0, 'AVERAGE': 1, 'GREAT': 2, 'ULTIMATE': 3}

# <editor-fold desc="XenoResurrectionSpecies">
EXTINCT_SPECIES = [
    "BANFORO",
    "KILANDOW",
    "MISIORLA"
]
# </editor-fold>

# <editor-fold desc="Piloting traits">
# TODO (Morlic): Consider using only 1 dict with tuple for (Capacity, SecondaryStat) effects
PILOT_DAMAGE_MODIFIER_DICT = {
    # TRAIT:    {weapon_name: effect, weapon_name2: effect2,...}
    "NO":       {},
    "BAD":      {"SR_WEAPON_1_1": -1, "SR_WEAPON_2_1": -2, "SR_WEAPON_3_1": -3, "SR_WEAPON_4_1": -5},
    "GOOD":     {"SR_WEAPON_1_1":  1, "SR_WEAPON_2_1":  2, "SR_WEAPON_3_1":  3, "SR_WEAPON_4_1": 5},
    "GREAT":    {"SR_WEAPON_1_1":  2, "SR_WEAPON_2_1":  4, "SR_WEAPON_3_1":  6, "SR_WEAPON_4_1": 10},
    "ULTIMATE": {"SR_WEAPON_1_1":  3, "SR_WEAPON_2_1":  6, "SR_WEAPON_3_1":  9, "SR_WEAPON_4_1": 15, "SR_WEAPON_0_1": 1},
}

PILOT_ROF_MODIFIER_DICT = {
    # TRAIT:    {weapon_name: effect, weapon_name2: effect2,...}
    "NO":       {},
    "BAD":      {"SR_WEAPON_0_1": -1},
    "GOOD":     {"SR_WEAPON_0_1": 1},
    "GREAT":    {"SR_WEAPON_0_1": 2},
    "ULTIMATE": {"SR_WEAPON_0_1": 3},
}

PILOT_FIGHTERDAMAGE_MODIFIER_DICT = {
    # TRAIT:    {hangar_name: effect, hangar_name2: effect2,...}
    "NO":       {},
    "BAD":      {"FT_HANGAR_1": -1, "FT_HANGAR_2": -2, "FT_HANGAR_3": -3, "FT_HANGAR_4": -4},
    "GOOD":     {"FT_HANGAR_1":  1, "FT_HANGAR_2":  2, "FT_HANGAR_3":  3, "FT_HANGAR_4": 4},
    "GREAT":    {"FT_HANGAR_1":  2, "FT_HANGAR_2":  4, "FT_HANGAR_3":  6, "FT_HANGAR_4": 8},
    "ULTIMATE": {"FT_HANGAR_1":  3, "FT_HANGAR_2":  6, "FT_HANGAR_3":  9, "FT_HANGAR_4": 12},
}

PILOT_FIGHTER_CAPACITY_MODIFIER_DICT = {
    # TRAIT:    {hangar_name: effect, hangar_name2: effect2,...}
    "NO":       {},
    "BAD":      {},
    "GOOD":     {},
    "GREAT":    {},
    "ULTIMATE": {},
}

HANGAR_LAUNCH_CAPACITY_MODIFIER_DICT = {
    # hangar_name: {bay_name: effect, bay_name2: effect, ...}
    "FT_HANGAR_1": {"FT_BAY_1": 2},
}
# </editor-fold>

# <editor-fold desc="Extraordinary Species Rules">
# tag used if species self-destructs on conquest
TAG_DESTROYED_ON_CONQUEST = "DESTROYED_ON_CONQUEST"

# some species have a fixed population
SPECIES_FIXED_POPULATION = {
    # species_name: fixed_population_size
    SP_LAMBALALAM: 5.0,
}

# techs that are unlocked if conquering a planet of a species
SPECIES_TECH_UNLOCKS = {
    # species: [tech1, tech2, ...]
    SP_LAMBALALAM: [GRO_LIFE_CYCLE, SPY_STEALTH_1, SPY_STEALTH_2]
}
# </editor-fold>

# </editor-fold>


# <editor-fold desc="Buildings">
# <editor-fold desc="Supply">
# Please see the Note at top of this file regarding PlanetSize-Dependent-Lookup
# building supply bonuses are keyed by planet size; key -1 stands for any planet size
building_supply = {
    "BLD_IMPERIAL_PALACE": {
        -1: 2,
    },
    "BLD_MEGALITH": {
        -1: 2,
    },
    "BLD_SPACE_ELEVATOR": {
        fo.planetSize.tiny: 1,
        fo.planetSize.small: 2,
        fo.planetSize.medium: 3,
        fo.planetSize.large: 4,
        fo.planetSize.huge: 5,
        fo.planetSize.gasGiant: 3,
    },
}
# </editor-fold>

# <editor-fold desc="Shipyards">
BLD_SHIPYARD_ORBITAL_DRYDOCK = "BLD_SHIPYARD_ORBITAL_DRYDOCK"
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
SYSTEM_SHIP_FACILITIES = frozenset((
    "BLD_SHIPYARD_AST",
    "BLD_SHIPYARD_AST_REF",
))

# </editor-fold>

# </editor-fold>


# <editor-fold desc="Shipdesign/-parts">
PART_KRILL_SPAWNER = "SP_KRILL_SPAWNER"

# <editor-fold desc="Effect Scripting for Shipdesigns">
# <editor-fold desc="Tokens">
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
DETECTION = "DETECTION"            # do only specify for hulls if irregular detection
ORGANIC_GROWTH = "ORGANIC_GROWTH"  # structure for value is (per_turn, maximum)
STACKING_RULES = "STACKING_RULES"  # expects a list of stacking rules
# <editor-fold desc="Stacking rules">
NO_EFFECT_WITH_CLOAKS = "NO_EFFECT_WITH_CLOAKS"  # part does not stack with cloak parts
# </editor-fold>
# </editor-fold>

BASE_DETECTION = 25

TECH_EFFECTS = {
    # "TECHNAME": { Token1: Value1, Token2: Value2, ...}
    "SHP_REINFORCED_HULL": {STRUCTURE: 5},
    "SHP_BASIC_DAM_CONT": {REPAIR_PER_TURN: 1},
    "SHP_FLEET_REPAIR": {REPAIR_PER_TURN: (STRUCTURE, 0.1)},  # 10% of max structure
    "SHP_ADV_DAM_CONT": {REPAIR_PER_TURN: (STRUCTURE, 0.1)},  # 10% of max structure
    "SHP_INTSTEL_LOG": {SPEED: 20},  # technically not correct, but as approximation good enough...
    "GRO_ENERGY_META": {FUEL: 2},
}

HULL_EFFECTS = {
    # "HULLNAME": { Token1: Value1, Token2: Value2, ...}
    # Robotic line
    "SH_ROBOTIC": {
        REPAIR_PER_TURN: 2,
    },
    "SH_SPATIAL_FLUX": {
        STEALTH_MODIFIER: -30,
    },
    "SH_NANOROBOTIC": {
        REPAIR_PER_TURN: (STRUCTURE, 1),
    },  # 100% of max structure
    "SH_LOGISTICS_FACILITATOR": {
        REPAIR_PER_TURN: (STRUCTURE, 1),
    },  # 100% of max structure
    # Asteroid line
    "SH_SMALL_ASTEROID": {
        ASTEROID_STEALTH: 20,
    },
    "SH_ASTEROID": {
        ASTEROID_STEALTH: 20,
    },
    "SH_HEAVY_ASTEROID": {
        ASTEROID_STEALTH: 20,
    },
    "SH_SMALL_CAMOUFLAGE_ASTEROID": {
        ASTEROID_STEALTH: 20,
    },
    "SH_CAMOUFLAGE_ASTEROID": {
        ASTEROID_STEALTH: 40,
    },
    "SH_CRYSTALLIZED_ASTEROID": {
        ASTEROID_STEALTH: 20,
    },
    "SH_MINIASTEROID_SWARM": {
        ASTEROID_STEALTH: 20,
        SHIELDS: 5,
    },
    "SH_SCATTERED_ASTEROID": {
        ASTEROID_STEALTH: 40,
        SHIELDS: 3,
    },
    # Organic line
    "SH_ORGANIC": {
        REPAIR_PER_TURN: 2,
        FUEL_PER_TURN: 0.2,
        DETECTION: 10,
        ORGANIC_GROWTH: (0.2, 5),
    },
    "SH_ENDOMORPHIC": {
        DETECTION: 50,
        ORGANIC_GROWTH: (0.5, 15),
    },
    "SH_SYMBIOTIC": {
        REPAIR_PER_TURN: 2,
        FUEL_PER_TURN: 0.2,
        DETECTION: 50,
        ORGANIC_GROWTH: (0.2, 10),
    },
    "SH_PROTOPLASMIC": {
        REPAIR_PER_TURN: 2,
        FUEL_PER_TURN: 0.2,
        DETECTION: 50,
        ORGANIC_GROWTH: (0.5, 25),
    },
    "SH_ENDOSYMBIOTIC": {
        REPAIR_PER_TURN: 2,
        FUEL_PER_TURN: 0.2,
        DETECTION: 50,
        ORGANIC_GROWTH: (0.5, 15),
    },
    "SH_RAVENOUS": {
        DETECTION: 75,
        ORGANIC_GROWTH: (0.5, 20),
    },
    "SH_BIOADAPTIVE": {
        REPAIR_PER_TURN: (STRUCTURE, 1),
        FUEL_PER_TURN: 0.2,
        DETECTION: 75,
        ORGANIC_GROWTH: (0.5, 25),
    },
    "SH_SENTIENT": {
        REPAIR_PER_TURN: 2,
        FUEL_PER_TURN: 0.2,
        DETECTION: 70,
        ORGANIC_GROWTH: (1, 45),
        STEALTH_MODIFIER: 20,
    },
    # Energy Line
    "SH_SOLAR": {
        SOLAR_STEALTH: 120,
        FUEL_PER_TURN: (FUEL, 1),
    }  # 100% of fuel
}

PART_EFFECTS = {
    # "PARTNAME": { Token1: Value1, Token2: Value2, ...}
    "SH_MULTISPEC": {
        SOLAR_STEALTH: 60,
    },
    "FU_TRANSPATIAL_DRIVE": {

    },  # not supported yet
    "FU_RAMSCOOP": {
        FUEL_PER_TURN: 0.1,
    },
    "FU_ZERO_FUEL": {
        FUEL_PER_TURN: (FUEL, 1),
    },  # 100% of fuel
    "SP_DISTORTION_MODULATOR": {

    },  # not supported yet
    "SH_ROBOTIC_INTERFACE_SHIELDS": {

    },  # not supported yet
    PART_KRILL_SPAWNER: {
        STEALTH_MODIFIER: 40,
        STACKING_RULES: [NO_EFFECT_WITH_CLOAKS],
    }
}

# </editor-fold>

# </editor-fold>
