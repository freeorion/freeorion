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
import freeOrionAIInterface as fo

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
HOMEWORLD_INFLUENCE_COST = 1  # homeworld independency movement

STABILITY_PER_LIKED_FOCUS = 2.0
STABILITY_HOMEWORLD_BONUS = 5.0
STABILITY_PER_LIKED_BUILDING_ON_PLANET = 4.0
STABILITY_PER_LIKED_BUILDING_IN_SYSTEM = 1.0
STABILITY_BASE_LIKED_BUILDING_ELSEWHERE = 0.5  # bonus is this time sqrt(number)
STABILITY_BY_WORLDTREE = 1.0
# specials (dis)likes are not affected by PlanetUtilsAI.dislike_factor() and also not by who owns them!
STABILITY_PER_LIKED_SPECIAL_ON_PLANET = 3.0
STABILITY_PER_LIKED_SPECIAL_IN_SYSTEM = 1.0


SHIP_STRUCTURE_FACTOR = fo.getGameRules().getDouble("RULE_SHIP_STRUCTURE_FACTOR")
SHIP_WEAPON_DAMAGE_FACTOR = fo.getGameRules().getDouble("RULE_SHIP_WEAPON_DAMAGE_FACTOR")
FIGHTER_DAMAGE_FACTOR = fo.getGameRules().getDouble("RULE_FIGHTER_DAMAGE_FACTOR")

# Colonization details
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

# Monster fleet plans
MINIMUM_GUARD_DISTANCE_TO_HOME_SYSTEM = 2


class Tags:
    POPULATION = "POPULATION"
    INDUSTRY = "INDUSTRY"
    INFLUENCE = "INFLUENCE"
    WEAPONS = "WEAPONS"
    RESEARCH = "RESEARCH"
    SUPPLY = "SUPPLY"
    STABILITY = "HAPPINESS"
    ATTACKTROOPS = "OFFENSE_TROOPS"
    STEALTH = "STEALTH"
    SHIP_SHIELDS = "SHIP_SHIELDS"
    FUEL = "FUEL"
    DETECTION = "DETECTION"
    INDEPENDENT = "INDEPENDENT_HAPPINESS"
    ARTISTIC = "ARTISTIC"
    XENOPHOBIC = "XENOPHOBIC"


# </editor-fold>


# <editor-fold desc="Environmental population modifiers">
#  Population modifiers that are based on environment

# Population modifiers scaling with planet size
#  - [[EARLY_PRIORITY]]: Affected by species modifiers
POP_SIZE_MOD_MAP_MODIFIED_BY_SPECIES = {
    # "Effect": [uninhab, hostile, poor, adequate, good]
    "environment_bonus": [0, -4, -2, 0, 3],
    "GRO_SYMBIOTIC_BIO": [0, 0, 1, 1, 1],
    "GRO_XENO_GENETICS": [0, 1, 2, 2, 0],
    "GRO_XENO_HYBRIDS": [0, 2, 1, 0, 0],
}

# Population modifiers scaling with planet size
#  - [[LATE_PRIORITY]] or later:
POP_SIZE_MOD_MAP_NOT_MODIFIED_BY_SPECIES = {
    "GRO_SUBTER_HAB": [0, 1, 1, 1, 1],
    "CON_NDIM_STRC": [0, 2, 2, 2, 2],
    "CON_ORBITAL_HAB": [0, 1, 1, 1, 1],
}

# flat boni not dependent on size
POP_CONST_MOD_MAP = {
    # "Source": [uninhab, hostile, poor, adequate, good]
    "GRO_PLANET_ECOL": [0, 0, 0, 1, 1],
    "GRO_SYMBIOTIC_BIO": [0, 0, 0, -1, -1],
}

# phototrophic star coefficients
POP_MOD_PHOTOTROPHIC_STAR_MAP = {
    fo.starType.blue: 3,
    fo.starType.white: 1.5,
    fo.starType.red: -1,
    fo.starType.neutron: -1,
    fo.starType.blackHole: -10,
    fo.starType.noStar: -10,
}

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
    "BAD": -PRIMARY_FOCS_STEALTH_LEVELS["LOW_STEALTH"],
    "GOOD": PRIMARY_FOCS_STEALTH_LEVELS["LOW_STEALTH"],
    "GREAT": PRIMARY_FOCS_STEALTH_LEVELS["MEDIUM_STEALTH"],
    "ULTIMATE": PRIMARY_FOCS_STEALTH_LEVELS["HIGH_STEALTH"],
    "INFINITE": 500,  # used by super testers
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
    "SELF_SUSTAINING": [],
}

# stores which metabolism profits from each special - autogenerated DO NOT EDIT
metabolismBoosts = {}
for metab, boosts in metabolismBoostMap.items():
    for boost in boosts:
        metabolismBoosts[boost] = metab
# </editor-fold>

# <editor-fold desc="Industry boosting specials">
# Each adds INDUSTRY_PER_POP before production is multiplied by species skill modifier
industry_boost_specials_modified = {
    "TIDAL_LOCK_SPECIAL",
}
# Each adds INDUSTRY_PER_POP after all multipliers have been applied
industry_boost_specials_unmodified = {
    "CRYSTALS_SPECIAL",
    "ELERIUM_SPECIAL",
    "MINERALS_SPECIAL",
    "MONOPOLE_SPECIAL",
    "POSITRONIUM_SPECIAL",
    "SUPERCONDUCTOR_SPECIAL",
}
# </editor-fold>

luxury_specials = {
    "FRACTAL_GEODES_SPECIAL",
    "MIMETIC_ALLOY_SPECIAL",
    "SHIMMER_SILK_SPECIAL",
    "SPARK_FOSSILS_SPECIAL",
    "SUCCULENT_BARNACLES_SPECIAL",
}

# <editor-fold desc="Other Population changing specials">
# Please see the Note at top of this file regarding PlanetSize-Dependent-Lookup
# Regardless of whether the sub-dictionary here has PlanetSize keys, the final
# value will be applied as a *fixed-size mod* to the max population
WORLDTREE_SPECIAL = "WORLDTREE_SPECIAL"
POP_FIXED_MOD_SPECIALS = {
    WORLDTREE_SPECIAL: 1,  # Not for SP_KHAKTURIAN...
}
GAIA_SPECIAL = "GAIA_SPECIAL"


def not_affect_by_special(special: str, species: str) -> bool:
    if special == WORLDTREE_SPECIAL:
        return species == "SP_KHAKTURIAN"
    # They do not have a good environment on normal planets (or none at all).
    # TBD: could determine that list when there is a way to iterate over all species
    if special == GAIA_SPECIAL:
        return species in ("SP_EXOBOT", "SP_SLY", "SP_THENIAN")


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
    "TIDAL_LOCK_SPECIAL": -1,
    "TEMPORAL_ANOMALY_SPECIAL": -5,
}
# </editor-fold>

# <editor-fold desc="Industry related specials">
HONEYCOMB_SPECIAL = "HONEYCOMB_SPECIAL"
# </editor-fold>

# <editor-fold desc="Research related specials">
COMPUTRONIUM_SPECIAL = "COMPUTRONIUM_SPECIAL"

ANCIENT_RUINS_SPECIAL = "ANCIENT_RUINS_SPECIAL"
ANCIENT_RUINS_SPECIAL2 = "ANCIENT_RUINS_DEPLETED_SPECIAL"  # nothing to excavate anymore, but some research bonus
ASTEROID_COATING_OWNED_SPECIAL = "ASTEROID_COATING_OWNED_SPECIAL"
ASTEROID_COATING_SPECIAL = "ASTEROID_COATING_SPECIAL"
# </editor-fold>

# <editor-fold desc="Supply related specials">
SUPPLY_MOD_SPECIALS = {
    WORLDTREE_SPECIAL: {
        -1: 1,
    },
    "ECCENTRIC_ORBIT_SPECIAL": {
        -1: -2,
    },
    "ACCRETION_DISC_SPECIAL": {
        -1: -1,
    },
}
# </editor-fold>

# <editor-fold desc="Native Tech/Defense related specials">
# (special_name, defense&shield_bonus)
TECH_NATIVE_SPECIALS = {
    "MODERATE_TECH_NATIVES_SPECIAL": {"defense": 10, "shield": 10},
    "HIGH_TECH_NATIVES_SPECIAL": {"defense": 30, "shield": 30},
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
PRO_AUTO_1 = "PRO_ADAPTIVE_AUTOMATION"
PRO_AUTO_2 = "PRO_SENTIENT_AUTOMATION"
PRO_NEUTRONIUM_EXTRACTION = "PRO_NEUTRONIUM_EXTRACTION"
NEST_DOMESTICATION_TECH = "SHP_DOMESTIC_MONSTER"

LRN_ARTIF_MINDS_1 = "LRN_NASCENT_AI"
LRN_ALGO_ELEGANCE = "LRN_ALGO_ELEGANCE"
GRO_PLANET_ECOL = "GRO_PLANET_ECOL"
LRN_QUANT_NET = "LRN_QUANT_NET"
LRN_XENOARCH = "LRN_XENOARCH"
LRN_EVERYTHING = "LRN_EVERYTHING"
LRN_ART_BLACK_HOLE = "LRN_ART_BLACK_HOLE"

SR_FLUX_LANCE = "SR_FLUX_LANCE"
SPY_STEALTH_1 = "SPY_STEALTH_1"
SPY_STEALTH_2 = "SPY_STEALTH_2"

EXOBOT_TECH_NAME = "PRO_EXOBOTS"
SHP_ASTEROID_HULLS = "SHP_ASTEROID_HULLS"

# </editor-fold>


# Tech required to build outpost pods
OUTPOSTING_TECH = "SHP_GAL_EXPLO"

# <editor-fold desc="Defense techs">
# TODO are these 2 necessary? Could rely on ordered DEFENSE_SHIELD_TECHS (Morlic)
PLANET_BARRIER_I_TECH = "DEF_PLAN_BARRIER_SHLD_1"
DEFENSE_REGEN_1_TECH = "DEF_DEFENSE_NET_REGEN_1"
# </editor-fold>

# <editor-fold desc="Fuel techs">
FUEL_TANK_UPGRADE_DICT = {
    # "PARTNAME": tuple((tech_name, fuel_upgrade), (tech_name2, fuel_upgrade2), ...)
    "FU_BASIC_TANK": (("SHP_DEUTERIUM_TANK", 0.5), ("SHP_ANTIMATTER_TANK", 1.5)),
    "FU_RAMSCOOP": (),
    "FU_ZERO_FUEL": (),
}

# DO NOT TOUCH THIS ENTRY BUT UPDATE FUEL_TANK_UPGRADE_DICT INSTEAD!
FUEL_UPGRADE_TECHS = frozenset(
    tech_name for _dict in (FUEL_TANK_UPGRADE_DICT,) for tups in _dict.values() for (tech_name, _) in tups
)
# </editor-fold>

# <editor-fold desc="Weapon techs">
# TODO (Morlic): Consider using only 1 dict with (capacity, secondaryStat) tuple as entries
WEAPON_UPGRADE_DICT = {
    part_name: tuple(
        {tech_name: damage * SHIP_WEAPON_DAMAGE_FACTOR for tech_name, damage, in techs_damage.items()}.items()
    )
    for part_name, techs_damage in {
        # Output "PARTNAME": tuple((tech_name, dmg_upgrade), (tech_name2, dmg_upgrade2), ...)
        #  Input "PARTNAME": {tech_name: dmg_upgrade, tech_name2, dmg_upgrade2, ...}
        "SR_WEAPON_0_1": {},
        "SR_WEAPON_1_1": {"SHP_WEAPON_1_%d" % i: 1 for i in [2, 3, 4]},
        "SR_WEAPON_2_1": {"SHP_WEAPON_2_%d" % i: 2 for i in [2, 3, 4]},
        "SR_WEAPON_3_1": {"SHP_WEAPON_3_%d" % i: 3 for i in [2, 3, 4]},
        "SR_WEAPON_4_1": {"SHP_WEAPON_4_%d" % i: 5 for i in [2, 3, 4]},
        "SR_ARC_DISRUPTOR": {"SHP_WEAPON_ARC_DISRUPTOR_%d" % i: i for i in [2, 3]},
        SR_FLUX_LANCE: {},
        "SR_SPINAL_ANTIMATTER": {},
    }.items()
}

# ROF == rate of fire == weapon shots
WEAPON_ROF_UPGRADE_DICT = {
    # "PARTNAME": tuple((tech_name, rof_upgrade), (tech_name2, rof_upgrade2), ...)
    "SR_WEAPON_0_1": (("SHP_WEAPON_1_3", 1), ("SHP_WEAPON_2_3", 1), ("SHP_WEAPON_3_3", 1), ("SHP_WEAPON_4_3", 1)),
    "SR_WEAPON_1_1": (),
    "SR_WEAPON_2_1": (),
    "SR_WEAPON_3_1": (),
    "SR_WEAPON_4_1": (),
    "SR_ARC_DISRUPTOR": (),
    SR_FLUX_LANCE: {},
    "SR_SPINAL_ANTIMATTER": (),
}

FIGHTER_DAMAGE_UPGRADE_DICT = {
    part_name: tuple({tech_name: damage * FIGHTER_DAMAGE_FACTOR for tech_name, damage, in techs_damage.items()}.items())
    for part_name, techs_damage in {
        # Output "PARTNAME": tuple((tech_name, dmg_upgrade), (tech_name2, dmg_upgrade2), ...)
        #  Input "PARTNAME": {tech_name: dmg_upgrade, tech_name2: dmg_upgrade2, ...}
        "FT_HANGAR_1": {"SHP_FIGHTERS_2": 0, "SHP_FIGHTERS_3": 0, "SHP_FIGHTERS_4": 0},
        "FT_HANGAR_2": {"SHP_FIGHTERS_2": 2, "SHP_FIGHTERS_3": 2, "SHP_FIGHTERS_4": 2},
        "FT_HANGAR_3": {"SHP_FIGHTERS_2": 3, "SHP_FIGHTERS_3": 3, "SHP_FIGHTERS_4": 3},
        "FT_HANGAR_4": {"SHP_FIGHTERS_2": 6, "SHP_FIGHTERS_3": 6, "SHP_FIGHTERS_4": 6},
    }.items()
}

FIGHTER_CAPACITY_UPGRADE_DICT = {
    # "PARTNAME": tuple((tech_name, capacity_upgrade), (tech_name2, capacity_upgrade2), ...)
    "FT_HANGAR_1": (("SHP_FIGHTERS_2", 1), ("SHP_FIGHTERS_3", 1), ("SHP_FIGHTERS_4", 1)),
    "FT_HANGAR_2": (),
    "FT_HANGAR_3": (),
    "FT_HANGAR_4": (),
}

# DO NOT TOUCH THIS ENTRY BUT UPDATE WEAPON_UPGRADE_DICT INSTEAD!
WEAPON_UPGRADE_TECHS = frozenset(
    tech_name
    for _dict in (WEAPON_UPGRADE_DICT, WEAPON_ROF_UPGRADE_DICT)
    for tups in _dict.values()
    for (tech_name, _) in tups
)
FIGHTER_UPGRADE_TECHS = frozenset(
    tech_name
    for _dict in (FIGHTER_DAMAGE_UPGRADE_DICT, FIGHTER_CAPACITY_UPGRADE_DICT)
    for tups in _dict.values()
    for (tech_name, _) in tups
)
# </editor-fold>

# <editor-fold desc="Tech Groups for Automated Research Approach">
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
# </editor-fold>

# </editor-fold>


# <editor-fold desc="Species">
# species names
SP_LEMBALALAM = "SP_LEMBALALAM"

# Species modifiers
SPECIES_RESEARCH_MODIFIER = {"NO": 0.0, "VERY_BAD": 0.5, "BAD": 0.75, "GOOD": 1.5, "GREAT": 2.0, "ULTIMATE": 3.0}
SPECIES_INDUSTRY_MODIFIER = {"NO": 0.0, "VERY_BAD": 0.5, "BAD": 0.75, "GOOD": 1.5, "GREAT": 2.0, "ULTIMATE": 3.0}
SPECIES_INFLUENCE_MODIFIER = {"NO": 0.0, "VERY_BAD": 0.5, "BAD": 0.75, "GOOD": 1.5, "GREAT": 2.0, "ULTIMATE": 3.0}
SPECIES_POPULATION_MODIFIER = {"EXTREMELY_BAD": 0.25, "VERY_BAD": 0.5, "BAD": 0.75, "GOOD": 1.25}
SPECIES_SUPPLY_MODIFIER = {"VERY_BAD": -1, "BAD": 0, "AVERAGE": 1, "GREAT": 2, "ULTIMATE": 3}
SPECIES_STABILITY_MODIFIER = {"VERY_BAD": -5.0, "BAD": -2.5, "AVERAGE": 0, "GOOD": 2.5, "GREAT": 5.0, "ULTIMATE": 7.5}
SPECIES_FUEL_MODIFIER = {"NO": -100, "BAD": -0.5, "AVERAGE": 0, "GOOD": 0.5, "GREAT": 1, "ULTIMATE": 1.5}
# the actual effect depends on the type of shield, see CombatRatingsAI.species_shield_bonus
SPECIES_SHIP_SHIELD_MODIFIER = {"BAD": -0.5, "AVERAGE": 0, "GOOD": 0.5, "GREAT": 1, "ULTIMATE": 1.5}
# values for both offense and defense
SPECIES_TROOP_MODIFIER = {"NO": 0.0, "BAD": 0.5, "": 1.0, "GOOD": 1.5, "GREAT": 2.0, "ULTIMATE": 3.0}
# missing: HAPPINESS, ...

# <editor-fold desc="XenoResurrectionSpecies">
EXTINCT_SPECIES = ["BANFORO", "KILANDOW", "MISIORLA"]
# </editor-fold>

# <editor-fold desc="Piloting traits">
# TODO (Morlic): Consider using only 1 dict with tuple for (Capacity, SecondaryStat) effects
PILOT_DAMAGE_UNSCALED_MODIFIER_DICT = {
    # TRAIT:    {weapon_name: effect, weapon_name2: effect2,...}
    "NO": {},
    "BAD": {"SR_WEAPON_1_1": -1, "SR_WEAPON_2_1": -2, "SR_WEAPON_3_1": -3, "SR_WEAPON_4_1": -5},
    "GOOD": {"SR_WEAPON_1_1": 1, "SR_WEAPON_2_1": 2, "SR_WEAPON_3_1": 3, "SR_WEAPON_4_1": 5},
    "GREAT": {"SR_WEAPON_1_1": 2, "SR_WEAPON_2_1": 4, "SR_WEAPON_3_1": 6, "SR_WEAPON_4_1": 10},
    "ULTIMATE": {"SR_WEAPON_1_1": 3, "SR_WEAPON_2_1": 6, "SR_WEAPON_3_1": 9, "SR_WEAPON_4_1": 15},
}

PILOT_ROF_MODIFIER_DICT = {
    # TRAIT:    {weapon_name: effect, weapon_name2: effect2,...}
    "NO": {},
    "BAD": {"SR_WEAPON_0_1": -1},
    "GOOD": {"SR_WEAPON_0_1": 1},
    "GREAT": {"SR_WEAPON_0_1": 2},
    "ULTIMATE": {"SR_WEAPON_0_1": 3},
}

PILOT_FIGHTERDAMAGE_UNSCALED_MODIFIER_DICT = {
    # TRAIT:    {hangar_name: effect, hangar_name2: effect2,...}
    # Note: FT_HANGAR_1 fighters are not able to attack ships so pilot damage modifier does not apply
    "NO": {},
    "BAD": {"FT_HANGAR_1": 0, "FT_HANGAR_2": -1, "FT_HANGAR_3": -1, "FT_HANGAR_4": -1},
    "GOOD": {"FT_HANGAR_1": 0, "FT_HANGAR_2": 1.5, "FT_HANGAR_3": 1.5, "FT_HANGAR_4": 1.5},
    "GREAT": {"FT_HANGAR_1": 0, "FT_HANGAR_2": 3, "FT_HANGAR_3": 3, "FT_HANGAR_4": 3},
    "ULTIMATE": {"FT_HANGAR_1": 0, "FT_HANGAR_2": 4.5, "FT_HANGAR_3": 4.5, "FT_HANGAR_4": 4.5},
}

PILOT_FIGHTER_CAPACITY_MODIFIER_DICT = {
    # TRAIT:    {hangar_name: effect, hangar_name2: effect2,...}
    "NO": {},
    "BAD": {},
    "GOOD": {},
    "GREAT": {},
    "ULTIMATE": {},
}

HANGAR_LAUNCH_CAPACITY_MODIFIER_DICT = {
    # hangar_name: {bay_name: ((tech_name, effect), ...), bay_name2: ((tech_name, effect), ...}
    "FT_HANGAR_1": {
        "FT_BAY_1": (("SHP_FIGHTERS_1", 1), ("SHP_FIGHTERS_2", 1), ("SHP_FIGHTERS_3", 1), ("SHP_FIGHTERS_4", 1))
    },
}


def _scale_part_damage(part_damage: dict[str, int], factor: float) -> dict[str, float]:
    scaled_part_damage = {weapon_name: (damage * factor) for weapon_name, damage in part_damage.items()}
    return scaled_part_damage


PILOT_DAMAGE_MODIFIER_DICT = {
    trait: _scale_part_damage(part_damage, SHIP_WEAPON_DAMAGE_FACTOR)
    for trait, part_damage in PILOT_DAMAGE_UNSCALED_MODIFIER_DICT.items()
}

PILOT_FIGHTERDAMAGE_MODIFIER_DICT = {
    trait: _scale_part_damage(part_damage, FIGHTER_DAMAGE_FACTOR)
    for trait, part_damage in PILOT_FIGHTERDAMAGE_UNSCALED_MODIFIER_DICT.items()
}

# </editor-fold>

# <editor-fold desc="Extraordinary Species Rules">
# tag used if species self-destructs on conquest
TAG_DESTROYED_ON_CONQUEST = "DESTROYED_ON_CONQUEST"

# some species have a fixed population
SPECIES_FIXED_POPULATION = {
    # species_name: fixed_population_size
    SP_LEMBALALAM: 5.0,
}

# techs that are unlocked if conquering a planet of a species
SPECIES_TECH_UNLOCKS = {
    # species: [tech1, tech2, ...]
    SP_LEMBALALAM: [GRO_LIFE_CYCLE, SPY_STEALTH_1, SPY_STEALTH_2]
}
# </editor-fold>

# </editor-fold>


# <editor-fold desc="Buildings">
# <editor-fold desc="Supply">
# Please see the Note at top of this file regarding PlanetSize-Dependent-Lookup
# building supply bonuses are keyed by planet size; key -1 stands for any planet size
building_supply = {
    # Palace also gives a bonus, but it is destroyed when being conquered, so we wouldn't get it.
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

# </editor-fold>


# <editor-fold desc="Shipdesign/-parts">
PART_KRILL_SPAWNER = "SP_KRILL_SPAWNER"


HULL_EXCLUDED_SHIP_PART_CLASSES = {"SH_COLONY_BASE": (fo.shipPartClass.fuel, fo.shipPartClass.speed)}


# <editor-fold desc="Allowed Combat Targets">
# TODO: Inherit from enum.Flag after switch to Python 3.6+
class CombatTarget:
    NONE = 0
    SHIP = 1 << 0
    PLANET = 1 << 1
    FIGHTER = 1 << 2
    ANY = SHIP | PLANET | FIGHTER

    # map from weapon to allowed targets
    PART_ALLOWED_TARGETS = {
        "SR_WEAPON_0_1": FIGHTER,
        "SR_WEAPON_1_1": SHIP | PLANET,
        "SR_WEAPON_2_1": SHIP | PLANET,
        "SR_WEAPON_3_1": SHIP | PLANET,
        "SR_WEAPON_4_1": SHIP | PLANET,
        "SR_ARC_DISRUPTOR": ANY,
        SR_FLUX_LANCE: SHIP | PLANET,
        "SR_SPINAL_ANTIMATTER": SHIP | PLANET,
        "FT_HANGAR_0": NONE,
        "FT_HANGAR_1": FIGHTER,
        "FT_HANGAR_2": SHIP | FIGHTER,
        "FT_HANGAR_3": SHIP,
        "FT_HANGAR_4": SHIP | PLANET,
        # monster weapons
        "SR_GRAV_PULSE": ANY,
        "SR_ICE_BEAM": ANY,
        "SR_JAWS": SHIP | PLANET,
        "SR_PLASMA_DISCHARGE": ANY,
        "SR_SPINES": SHIP | PLANET,
        "SR_TENTACLE": ANY,  # note that SHIPs are primary targets
        "SR_THRASHING_BODY": SHIP | FIGHTER,
        "FT_HANGAR_KRILL": SHIP | FIGHTER,
    }


# </editor-fold>


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
FUEL_EFFICIENCY = "FUEL_EFFICIENCY"
SHIELDS = "SHIELDS"
STRUCTURE = "STRUCTURE"
DETECTION = "DETECTION"  # do only specify for hulls if irregular detection
ORGANIC_GROWTH = "ORGANIC_GROWTH"  # structure for value is (per_turn, maximum)
STACKING_RULES = "STACKING_RULES"  # expects a list of stacking rules
# <editor-fold desc="Stacking rules">
NO_EFFECT_WITH_CLOAKS = "NO_EFFECT_WITH_CLOAKS"  # part does not stack with cloak parts
# </editor-fold>
# </editor-fold>

BASE_DETECTION = 25

TECH_EFFECTS = {
    # "TECHNAME": { Token1: Value1, Token2: Value2, ...}
    "SHP_REINFORCED_HULL": {STRUCTURE: 5 * SHIP_STRUCTURE_FACTOR},
    "SHP_BASIC_DAM_CONT": {REPAIR_PER_TURN: SHIP_STRUCTURE_FACTOR},
    "SHP_FLEET_REPAIR": {REPAIR_PER_TURN: (STRUCTURE, 0.1)},  # 10% of max structure
    "SHP_ADV_DAM_CONT": {REPAIR_PER_TURN: (STRUCTURE, 0.1)},  # 10% of max structure
    "SHP_INTSTEL_LOG": {SPEED: 20},  # technically not correct, but as approximation good enough...
    "GRO_ENERGY_META": {FUEL: 1},
}

DEFAULT_FUEL_EFFICIENCY = 1
HULL_TAG_EFFECTS = {
    "GREAT_FUEL_EFFICIENCY": {
        FUEL_EFFICIENCY: 4,
    },
    "GOOD_FUEL_EFFICIENCY": {
        FUEL_EFFICIENCY: 2,
    },
    "AVERAGE_FUEL_EFFICIENCY": {
        FUEL_EFFICIENCY: 1,
    },
    "BAD_FUEL_EFFICIENCY": {
        FUEL_EFFICIENCY: 0.6,
    },
}

HULL_EFFECTS = {
    # "HULLNAME": { Token1: Value1, Token2: Value2, ...}
    # Spatial Flux line
    "SH_SPACE_FLUX_BUBBLE": {
        STEALTH_MODIFIER: -30,
    },
    "SH_SPACE_FLUX_COMPOSITE": {
        STEALTH_MODIFIER: -30,
    },
    "SH_SPATIAL_FLUX": {
        STEALTH_MODIFIER: -30,
    },
    # Robotic line
    "SH_ROBOTIC": {
        REPAIR_PER_TURN: 2 * SHIP_STRUCTURE_FACTOR,
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
        SHIELDS: 5 * SHIP_WEAPON_DAMAGE_FACTOR,
    },
    "SH_SCATTERED_ASTEROID": {
        ASTEROID_STEALTH: 40,
        SHIELDS: 3 * SHIP_WEAPON_DAMAGE_FACTOR,
    },
    # Organic line
    "SH_ORGANIC": {
        REPAIR_PER_TURN: 2 * SHIP_STRUCTURE_FACTOR,
        FUEL_PER_TURN: 0.2,
        ORGANIC_GROWTH: (0.2 * SHIP_STRUCTURE_FACTOR, 3 * SHIP_STRUCTURE_FACTOR),
    },
    "SH_ENDOMORPHIC": {
        DETECTION: 50,
        ORGANIC_GROWTH: (0.5 * SHIP_STRUCTURE_FACTOR, 15 * SHIP_STRUCTURE_FACTOR),
    },
    "SH_SYMBIOTIC": {
        REPAIR_PER_TURN: 2 * SHIP_STRUCTURE_FACTOR,
        FUEL_PER_TURN: 0.2,
        DETECTION: 40,
        ORGANIC_GROWTH: (0.2 * SHIP_STRUCTURE_FACTOR, 10 * SHIP_STRUCTURE_FACTOR),
    },
    "SH_PROTOPLASMIC": {
        REPAIR_PER_TURN: 2 * SHIP_STRUCTURE_FACTOR,
        FUEL_PER_TURN: 0.2,
        DETECTION: 40,
        ORGANIC_GROWTH: (0.5 * SHIP_STRUCTURE_FACTOR, 25 * SHIP_STRUCTURE_FACTOR),
    },
    "SH_ENDOSYMBIOTIC": {
        REPAIR_PER_TURN: 2 * SHIP_STRUCTURE_FACTOR,
        FUEL_PER_TURN: 0.2,
        DETECTION: 40,
        ORGANIC_GROWTH: (0.5 * SHIP_STRUCTURE_FACTOR, 15 * SHIP_STRUCTURE_FACTOR),
    },
    "SH_RAVENOUS": {
        DETECTION: 75,
        ORGANIC_GROWTH: (0.5 * SHIP_STRUCTURE_FACTOR, 20 * SHIP_STRUCTURE_FACTOR),
    },
    "SH_BIOADAPTIVE": {
        REPAIR_PER_TURN: (STRUCTURE, 1),
        FUEL_PER_TURN: 0.2,
        DETECTION: 75,
        ORGANIC_GROWTH: (0.5 * SHIP_STRUCTURE_FACTOR, 25 * SHIP_STRUCTURE_FACTOR),
    },
    "SH_SENTIENT": {
        REPAIR_PER_TURN: 2 * SHIP_STRUCTURE_FACTOR,
        FUEL_PER_TURN: 0.2,
        DETECTION: 90,
        ORGANIC_GROWTH: (1 * SHIP_STRUCTURE_FACTOR, 45 * SHIP_STRUCTURE_FACTOR),
        STEALTH_MODIFIER: 20,
    },
    # Energy Line
    "SH_SOLAR": {
        SOLAR_STEALTH: 120,
        FUEL_PER_TURN: (FUEL, 1),
    },  # 100% of fuel
}

PART_EFFECTS = {
    # "PARTNAME": { Token1: Value1, Token2: Value2, ...}
    "SH_MULTISPEC": {
        SOLAR_STEALTH: 60,
    },
    "FU_TRANSPATIAL_DRIVE": {},  # not supported yet
    "FU_RAMSCOOP": {
        FUEL_PER_TURN: 0.1,
    },
    "FU_ZERO_FUEL": {
        FUEL_PER_TURN: (FUEL, 1),
    },  # 100% of fuel
    "SP_DISTORTION_MODULATOR": {},  # not supported yet
    "SH_ROBOTIC_INTERFACE_SHIELDS": {},  # not supported yet
    PART_KRILL_SPAWNER: {
        STEALTH_MODIFIER: 40,
        STACKING_RULES: [NO_EFFECT_WITH_CLOAKS],
    },
}

# </editor-fold>

# </editor-fold>
