import freeOrionAIInterface as fo  # interface used to interact with FreeOrion AI client  # pylint: disable=import-error

metabolismBoostMap = {"ORGANIC": ["FRUIT_SPECIAL", "PROBIOTIC_SPECIAL", "SPICE_SPECIAL"],
                     "LITHIC": ["CRYSTALS_SPECIAL", "ELERIUM_SPECIAL", "MINERALS_SPECIAL"],
                     "ROBOTIC": ["MONOPOLE_SPECIAL", "POSITRONIUM_SPECIAL", "SUPERCONDUCTOR_SPECIAL"],
                     "SELF_SUSTAINING": []
                     }

metabolismBoosts = {}
for metab, boosts in metabolismBoostMap.items():
    for boost in boosts:
        metabolismBoosts[boost] = metab

COLONY_POD_COST = 120
COLONY_POD_UPKEEP = 0.06
OUTPOST_POD_COST = 50
SHIP_UPKEEP = 0.01

OUTPOSTING_TECH = "SHP_GAL_EXPLO"

supply_range_techs = {"CON_ORBITAL_CON": 1, "CON_CONTGRAV_ARCH": 1, "CON_GAL_INFRA": 1}
supply_by_size = {fo.planetSize.tiny: 2,
                  fo.planetSize.small: 1,
                  fo.planetSize.large: -1,
                  fo.planetSize.huge: -2,
                  fo.planetSize.gasGiant: -1
                  }

SUPPLY_MOD_SPECIALS = {'WORLDTREE_SPECIAL':{-1:1}}

# building supply bonuses are keyed by planet size; key -1 stands for any planet size
building_supply = {"BLD_IMPERIAL_PALACE": {-1: 2},
                   "BLD_MEGALITH": {-1: 2},
                   "BLD_SPACE_ELEVATOR": {fo.planetSize.tiny: 1,
                                          fo.planetSize.small: 2,
                                          fo.planetSize.medium: 3,
                                          fo.planetSize.large: 4,
                                          fo.planetSize.huge: 5,
                                          fo.planetSize.gasGiant: 4,
                                          },
                   }

PRO_ORBITAL_GEN = "PRO_ORBITAL_GEN"
PRO_SOL_ORB_GEN = "PRO_SOL_ORB_GEN"
PRO_MICROGRAV_MAN = "PRO_MICROGRAV_MAN"
PRO_SINGULAR_GEN = "PRO_SINGULAR_GEN"
PROD_AUTO_NAME = "PRO_SENTIENT_AUTOMATION"

LRN_ALGO_ELEGANCE = "LRN_ALGO_ELEGANCE"
LRN_QUANT_NET = "LRN_QUANT_NET"

TECH_EXCLUSION_MAP_1 = {"LRN_TRANSCEND": fo.aggression.typical}  # (k,v) exclude tech k if aggression is less than v
TECH_EXCLUSION_MAP_2 = {}  # (k,v) exclude tech k if aggression is greater than v

FIRST_PLANET_SHIELDS_TECH = "LRN_FORCE_FIELD"
PLANET_BARRIER_I_TECH = "DEF_PLAN_BARRIER_SHLD_1"
DEFENSE_REGEN_1_TECH = "DEF_DEFENSE_NET_REGEN_1"
PROT_FOCUS_MULTIPLIER = 2.0


# ship facilities info, dict keyed by building name, value is (min_aggression, prereq_bldg, base_cost, time, system)
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
SYSTEM_SHIP_FACILITIES = {
    "BLD_SHIPYARD_AST",
    "BLD_SHIPYARD_AST_REF",
}


