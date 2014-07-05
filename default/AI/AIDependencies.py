import freeOrionAIInterface as fo  # pylint: disable=import-error

metabolimBoostMap = {"ORGANIC": ["FRUIT_SPECIAL", "PROBIOTIC_SPECIAL", "SPICE_SPECIAL"],
                     "LITHIC": ["CRYSTALS_SPECIAL", "ELERIUM_SPECIAL", "MINERALS_SPECIAL"],
                     "ROBOTIC": ["MONOPOLE_SPECIAL", "POSITRONIUM_SPECIAL", "SUPERCONDUCTOR_SPECIAL"],
                     "SELF_SUSTAINING": []
                     }

metabolims = metabolimBoostMap.keys()
metabolimBoosts = {}
for metab, boosts in metabolimBoostMap.items():
    for boost in boosts:
        metabolimBoosts[boost] = metab

colonyPodCost = 120
colonyPodUpkeep = 0.06
outpostPodCost = 80
shipUpkeep = 0.01

outposting_tech = "CON_ENV_ENCAPSUL"
supply_range_techs = {"CON_ORBITAL_CON": 1, "CON_CONTGRAV_ARCH": 1, "CON_GAL_INFRA": 1}
supply_by_size = {int(fo.planetSize.tiny):      2,
                  int(fo.planetSize.small):     1,
                  int(fo.planetSize.large):    -1,
                  int(fo.planetSize.huge):     -2,
                  int(fo.planetSize.gasGiant): -1
                  }

# building supply bonuses are keyed by planet size; key -1 stands for any planet size
building_supply = {"BLD_IMPERIAL_PALACE": {-1: 2},
                   "BLD_MEGALITH": {-1: 2},
                   "BLD_SPACE_ELEVATOR": {int(fo.planetSize.tiny):     1,
                                          int(fo.planetSize.small):    2,
                                          int(fo.planetSize.medium):   3,
                                          int(fo.planetSize.large):    4,
                                          int(fo.planetSize.huge):     5,
                                          int(fo.planetSize.gasGiant): 4,
                                          },
                   }

sing_tech_name = "PRO_SINGULAR_GEN"
prod_auto_name = "PRO_SENTIENT_AUTOMATION"
