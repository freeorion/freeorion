
metabolimBoostMap= {  "ORGANIC": ["FRUIT_SPECIAL",  "PROBIOTIC_SPECIAL",  "SPICE_SPECIAL"],
                                                        "LITHIC":["CRYSTALS_SPECIAL",  "METALOIDS_SPECIAL", "MINERALS_SPECIAL"],
                                                        "ROBOTIC":["MONOPOLE_SPECIAL",  "POSITRONIUM_SPECIAL",  "SUPERCONDUCTOR_SPECIAL"],
                                                        "SELF_SUSTAINING":[],
                                                    }
metabolims= metabolimBoostMap.keys()
metabolimBoosts={}
for metab in metabolimBoostMap:
    for boost in metabolimBoostMap[metab] :
        metabolimBoosts[boost]=metab

colonyPodCost = 120
colonyPodUpkeep = 0.06
outpostPodCost = 80
shipUpkeep = 0.01

outposting_tech = "CON_ENV_ENCAPSUL"
supply_range_techs = ["CON_ORBITAL_CON", "CON_CONTGRAV_ARCH",  "CON_GAL_INFRA"]


