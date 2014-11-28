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
OUTPOST_POD_COST = 80
SHIP_UPKEEP = 0.01

OUTPOSTING_TECH = "CON_ENV_ENCAPSUL"

supply_range_techs = {"CON_ORBITAL_CON": 1, "CON_CONTGRAV_ARCH": 1, "CON_GAL_INFRA": 1}
supply_by_size = {fo.planetSize.tiny: 2,
                  fo.planetSize.small: 1,
                  fo.planetSize.large: -1,
                  fo.planetSize.huge: -2,
                  fo.planetSize.gasGiant: -1
                  }

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

