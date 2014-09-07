from freeOrionAIInterface import planetSize  # pylint: disable=import-error

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
supply_by_size = {planetSize.tiny: 2,
                  planetSize.small: 1,
                  planetSize.large: -1,
                  planetSize.huge: -2,
                  planetSize.gasGiant: -1
                  }

# building supply bonuses are keyed by planet size; key -1 stands for any planet size
building_supply = {"BLD_IMPERIAL_PALACE": {-1: 2},
                   "BLD_MEGALITH": {-1: 2},
                   "BLD_SPACE_ELEVATOR": {planetSize.tiny: 1,
                                          planetSize.small: 2,
                                          planetSize.medium: 3,
                                          planetSize.large: 4,
                                          planetSize.huge: 5,
                                          planetSize.gasGiant: 4,
                                          },
                   }

PRO_ORBITAL_GEN = "PRO_ORBITAL_GEN"
PRO_SOL_ORB_GEN = "PRO_SOL_ORB_GEN"
PRO_MICROGRAV_MAN = "PRO_MICROGRAV_MAN"
PRO_SINGULAR_GEN = "PRO_SINGULAR_GEN"
PROD_AUTO_NAME = "PRO_SENTIENT_AUTOMATION"

LRN_ALGO_ELEGANCE = "LRN_ALGO_ELEGANCE"
LRN_QUANT_NET = "LRN_QUANT_NET"
