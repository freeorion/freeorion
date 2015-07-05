"""
The TechsListAI module provides functions that describes dependencies between
various technologies to help the AI decide which technologies should be
researched next.
"""

# TODO: further consider alternative implementations for tech priorities
# adrian_broher recommends this whole module (and related
# decision branches within ResearchAI.py) not exist-- that this 
# informations instead go in the FOCS techs.txt file as a priority expression 
# for each respective tech specification, or that it be dynamically calculated 
# by the AI scripts based purely on the info parsed from techs.txt

EXOBOT_TECH_NAME = "PRO_EXOBOTS"

def unusable_techs():
    """
    Returns a list of technologies that shouldn't be researched by the AI.
    """
    return []


def defense_techs_1():
    """
    Returns a list of technologies that implement the first planetary defensive
    measures.
    """
    return [
        "DEF_DEFENSE_NET_1",
        "DEF_GARRISON_1"
    ]


def defense_techs_2():
    """
    Returns a list of technologies that implement additional planetary defensive
    measures. To use use the AI need to have built all defense technologies that
    are provided by defense_techs_1().
    """
    return []


def tech_group_1a(): # early org_hull
    result = [
            "LRN_ALGO_ELEGANCE",
            "GRO_PLANET_ECOL",
            "GRO_SUBTER_HAB",
            "DEF_GARRISON_1",
            "SHP_WEAPON_1_2",
            "LRN_ARTIF_MINDS",
            "SHP_WEAPON_1_3",
            "SHP_WEAPON_1_4",
            "CON_ORBITAL_CON",
    ]
    return result


def tech_group_1b():  # early _lrn_artif_minds and SHP_MIL_ROBO_CONT
    result = [
            "LRN_ALGO_ELEGANCE",
            "DEF_GARRISON_1",
            "LRN_ARTIF_MINDS",
            "SHP_WEAPON_1_2",
            "GRO_PLANET_ECOL",
            "GRO_SUBTER_HAB",
            "SHP_WEAPON_1_3",
            "PRO_ROBOTIC_PROD",
            "SHP_MIL_ROBO_CONT",
            "CON_ORBITAL_CON",
            "SHP_WEAPON_1_4",
    ]
    return result


def tech_group_1_sparse(): # 
    result = [
            "LRN_ALGO_ELEGANCE",
            "GRO_PLANET_ECOL",
            "GRO_SUBTER_HAB",
            "DEF_GARRISON_1",
            "LRN_ARTIF_MINDS",
            "SHP_WEAPON_1_2",
            "PRO_ROBOTIC_PROD",
            "SHP_WEAPON_1_3",
            "SHP_MIL_ROBO_CONT",
            "SHP_WEAPON_1_4",
            "SHP_SPACE_FLUX_DRIVE",
    ]
    return result

def tech_group_1_sparse_b(): # 
    result = [
            "LRN_ALGO_ELEGANCE",
            "GRO_PLANET_ECOL",
            "GRO_SUBTER_HAB",
            "DEF_GARRISON_1",
            "LRN_ARTIF_MINDS",
            "SHP_WEAPON_1_2",
            "CON_ORBITAL_CON",
            "PRO_ROBOTIC_PROD",
            "PRO_FUSION_GEN",
            "PRO_ORBITAL_GEN",
            "GRO_SYMBIOTIC_BIO",
            "SHP_WEAPON_1_3",
            "SHP_MIL_ROBO_CONT",
            "SHP_WEAPON_1_4",
            "SHP_ZORTRIUM_PLATE",
            "SHP_SPACE_FLUX_DRIVE",
    ]
    return result


def tech_group_2a():  # prioritizes growth & spy over weapons
    result = [
            "SHP_ZORTRIUM_PLATE",
            "DEF_DEFENSE_NET_1",
            "PRO_ROBOTIC_PROD",
            "SHP_SPACE_FLUX_DRIVE",
            "PRO_FUSION_GEN",
            "DEF_GARRISON_2",
            "LRN_FORCE_FIELD",
            "GRO_SYMBIOTIC_BIO",
            "SPY_DETECT_2",
            "SHP_WEAPON_2_1",
            "SHP_WEAPON_2_2",
            "SHP_WEAPON_2_3",
            "SHP_WEAPON_2_4",
            "PRO_INDUSTRY_CENTER_I",
    ]
    return result


def tech_group_2b():  # prioritizes weapons over growth & spy
    result = [
            "SHP_ZORTRIUM_PLATE",
            "PRO_ROBOTIC_PROD",
            "SHP_SPACE_FLUX_DRIVE",
            "PRO_FUSION_GEN",
            "DEF_GARRISON_2",
            "LRN_FORCE_FIELD",
            "DEF_DEFENSE_NET_1",
            "SHP_WEAPON_2_1",
            "SHP_WEAPON_2_2",
            "SHP_WEAPON_2_3",
            "SHP_WEAPON_2_4",
            "GRO_SYMBIOTIC_BIO",
            "PRO_INDUSTRY_CENTER_I",
            "SPY_DETECT_2",
    ]
    return result


def tech_group_2_sparse():  # prioritizes growth & defense over weapons
    result = [
            "CON_ORBITAL_CON",
            "PRO_FUSION_GEN",
            "SHP_ZORTRIUM_PLATE",
            "DEF_GARRISON_2",
            "GRO_SYMBIOTIC_BIO",
            "LRN_FORCE_FIELD",
            "DEF_DEFENSE_NET_1",
            "PRO_INDUSTRY_CENTER_I",
            "PRO_ORBITAL_GEN",
            "PRO_MICROGRAV_MAN",
            "SHP_ASTEROID_HULLS",
            "SPY_DETECT_2",
            "SHP_WEAPON_2_1",
            "SHP_WEAPON_2_2",
            "SHP_WEAPON_2_3",
            "SHP_WEAPON_2_4",
            "SHP_DOMESTIC_MONSTER",
    ]
    return result

def tech_group_2_sparse_b():  # prioritizes growth & defense over weapons
    result = [
            "DEF_GARRISON_2",
            "DEF_DEFENSE_NET_1",
            "PRO_INDUSTRY_CENTER_I",
            "SHP_WEAPON_2_1",
            "LRN_FORCE_FIELD",
            "SHP_WEAPON_2_2",
            "SHP_WEAPON_2_3",
            "SHP_WEAPON_2_4",
            "SPY_DETECT_2",
            "PRO_MICROGRAV_MAN",
            "SHP_ASTEROID_HULLS",
            "SHP_DOMESTIC_MONSTER",
    ]
    return result

def tech_group_3a(): # no plasma weaps yet
    result = [
            "SHP_DOMESTIC_MONSTER",
            "SHP_ORG_HULL",
            "GRO_GENETIC_ENG",
            "GRO_GENETIC_MED",
            "DEF_DEFENSE_NET_2",
            "DEF_DEFENSE_NET_REGEN_1",
            "PRO_SENTIENT_AUTOMATION",
            "DEF_PLAN_BARRIER_SHLD_1",
            "PRO_EXOBOTS",
            "GRO_XENO_GENETICS",
            "LRN_PHYS_BRAIN",
            "LRN_TRANSLING_THT",
            "SHP_BASIC_DAM_CONT",
            "DEF_GARRISON_3",
            "PRO_INDUSTRY_CENTER_II",
            "SHP_INTSTEL_LOG",
            "SHP_FLEET_REPAIR",
            "PRO_ORBITAL_GEN",
            "PRO_SOL_ORB_GEN",
            "GRO_NANO_CYBERNET",
            "PRO_MICROGRAV_MAN",
            "SHP_ASTEROID_HULLS",
            "SHP_IMPROVED_ENGINE_COUPLINGS",
            "SHP_ASTEROID_REFORM",
            "SHP_HEAVY_AST_HULL",
            "SHP_DEFLECTOR_SHIELD",
            "LRN_QUANT_NET",
            "SHP_CONTGRAV_MAINT",
            "GRO_XENO_HYBRIDS",
            "SHP_DEUTERIUM_TANK",
            "SPY_DETECT_3",
            "SHP_REINFORCED_HULL",
            "SHP_DIAMOND_PLATE",
            "DEF_DEFENSE_NET_REGEN_2",
            "DEF_PLAN_BARRIER_SHLD_2",
            "DEF_DEFENSE_NET_3",
            "CON_CONTGRAV_ARCH",
            "CON_ORBITAL_HAB",
            "CON_FRC_ENRG_STRC", 
            "DEF_SYST_DEF_MINE_1",
            "DEF_PLAN_BARRIER_SHLD_3",
            "CON_NDIM_STRC", 
            "SHP_N_DIMENSIONAL_ENGINE_MATRIX",
            "GRO_LIFECYCLE_MAN",
            "SHP_MULTICELL_CAST",
            "SHP_ENDOCRINE_SYSTEMS",
            "SHP_CONT_BIOADAPT",
            "SPY_STEALTH_1",
    ]
    return result


def tech_group_3b():  # with plasma weaps
    result = tech_group_3a()
    insert_idx = min(30, len(result)//2) if "GRO_XENO_HYBRIDS" not in result else 1 + result.index("GRO_XENO_HYBRIDS")
    result.insert(insert_idx, "SHP_WEAPON_3_4")
    return result

def tech_group_3_sparse(): # no plasma weaps yet
    result = [
            "SHP_DOMESTIC_MONSTER",
            "SHP_ORG_HULL",
            "GRO_GENETIC_ENG",
            "GRO_GENETIC_MED",
            "DEF_DEFENSE_NET_2",
            "DEF_DEFENSE_NET_REGEN_1",
            "PRO_SENTIENT_AUTOMATION",
            "DEF_PLAN_BARRIER_SHLD_1",
            "PRO_EXOBOTS",
            "GRO_XENO_GENETICS",
            "LRN_PHYS_BRAIN",
            "LRN_TRANSLING_THT",
            "SHP_BASIC_DAM_CONT",
            "PRO_INDUSTRY_CENTER_II",
            "SHP_INTSTEL_LOG",
            "SHP_FLEET_REPAIR",
            "PRO_ORBITAL_GEN",
            "PRO_SOL_ORB_GEN",
            "GRO_NANO_CYBERNET",
            "PRO_MICROGRAV_MAN",
            "SHP_ASTEROID_HULLS",
            "SHP_IMPROVED_ENGINE_COUPLINGS",
            "LRN_QUANT_NET",
            "SHP_ASTEROID_REFORM",
            "SHP_HEAVY_AST_HULL",
            "SHP_CONTGRAV_MAINT",
            "SHP_DEFLECTOR_SHIELD",
            "SHP_WEAPON_3_1",
            "SHP_WEAPON_3_2",
            "SHP_WEAPON_3_3",
            "SHP_WEAPON_3_4",
            "GRO_XENO_HYBRIDS",
            "SHP_DEUTERIUM_TANK",
            "SPY_DETECT_3",
            "SHP_REINFORCED_HULL",
            "SHP_DIAMOND_PLATE",
            "DEF_DEFENSE_NET_REGEN_2",
            "DEF_PLAN_BARRIER_SHLD_2",
            "DEF_DEFENSE_NET_3",
            "CON_CONTGRAV_ARCH",
            "CON_ORBITAL_HAB",
            "CON_FRC_ENRG_STRC", 
            "DEF_SYST_DEF_MINE_1",
            "DEF_GARRISON_3",
            "DEF_PLAN_BARRIER_SHLD_3",
            "CON_NDIM_STRC", 
            "SHP_N_DIMENSIONAL_ENGINE_MATRIX",
            "GRO_LIFECYCLE_MAN",
            "SHP_MULTICELL_CAST",
            "SHP_ENDOCRINE_SYSTEMS",
            "SHP_CONT_BIOADAPT",
            "SPY_STEALTH_1",
    ]
    return result

def tech_group_4a():  # later plasma weaps
    result = [
            "SHP_WEAPON_3_1",
            "SHP_WEAPON_3_2",
            "SHP_WEAPON_3_3",
            "SHP_FRC_ENRG_COMP",
            "SHP_WEAPON_3_4",
            "SHP_MASSPROP_SPEC",
    ]
    return result


def tech_group_4b():
    result = [
            "SHP_FRC_ENRG_COMP",
            "SHP_WEAPON_3_4",
            "SHP_MASSPROP_SPEC",
            ]
    return result


def tech_group_5():
    result = [
            "DEF_GARRISON_4",
            "DEF_PLAN_BARRIER_SHLD_4",
            "LRN_XENOARCH",
            "LRN_GRAVITONICS",
            "LRN_ENCLAVE_VOID",
            "SHP_MONOMOLEC_LATTICE",
            "SHP_SCAT_AST_HULL",
            "SHP_ADV_DAM_CONT",
            "LRN_STELLAR_TOMOGRAPHY",
            "LRN_TIME_MECH",
            "SPY_DETECT_4",
            "SHP_CONT_SYMB",
            "SHP_MONOCELL_EXP",
            "SHP_BIOADAPTIVE_SPEC",
            "SHP_SENT_HULL",
            "SHP_XENTRONIUM_PLATE",
            "GRO_CYBORG",
            "GRO_TERRAFORM",
            "GRO_ENERGY_META",
            "SHP_WEAPON_4_1",
            "SHP_WEAPON_4_2",
            "SHP_WEAPON_4_3",
            "LRN_DISTRIB_THOUGHT",
            "PRO_INDUSTRY_CENTER_III",
            "PRO_SINGULAR_GEN",
            "SHP_QUANT_ENRG_MAG",
            "SHP_PLASMA_SHIELD",
            "SHP_ENRG_BOUND_MAN",
            "PRO_NEUTRONIUM_EXTRACTION",
            "SHP_SOLAR_CONT",
            "CON_CONC_CAMP",
            "LRN_ART_BLACK_HOLE",
            "SPY_STEALTH_2",
            "DEF_SYST_DEF_MINE_2",
            "DEF_SYST_DEF_MINE_3",
            "LRN_PSY_DOM",
            "SPY_STEALTH_3",
            "SHP_WEAPON_4_4",
            "SHP_BLACKSHIELD",
            "SPY_STEALTH_4",
            "DEF_PLAN_BARRIER_SHLD_5",
            "SPY_DETECT_5",
            "GRO_GAIA_TRANS",
            "CON_ART_PLANET",
    ]
    return result

def sparse_galaxy_techs(index):
    #return primary_meta_techs()
    result = []
    print "Choosing Research Techlist Index %d" % index
    if index == 0:
        result = tech_group_1a()  # early org_hull
        result += tech_group_2a()  # prioritizes growth & defense over weapons
        result += tech_group_3a()
        result += tech_group_4a()
        result += tech_group_5()  #
    elif index == 1:
        result = tech_group_1a()  # early _lrn_artif_minds
        result += tech_group_2a()  # prioritizes growth & defense over weapons
        result += tech_group_3a()
        result += tech_group_4a()
        result += tech_group_5()  #
    elif index == 2:
        result = tech_group_1_sparse()  # early _lrn_artif_minds
        result += tech_group_2_sparse()  # prioritizes growth & defense over weapons
        result += tech_group_3a()
        result += tech_group_4a()
        result += tech_group_5()  #
    elif index == 3:
        result = tech_group_1_sparse_b()  # early org_hull
        result += tech_group_2_sparse_b() 
        result += tech_group_3a()
        result += tech_group_4a()
        result += tech_group_5()  #
    elif index == 4:
        result = tech_group_1_sparse_b()  # early _lrn_artif_minds
        result += tech_group_2_sparse_b() 
        result += tech_group_3b()  # faster plasma weaps
        result += tech_group_4b()
        result += tech_group_5()  #
    return result

def primary_meta_techs(index = 0):
    """
    Primary techs for all categories.
    """
    
    #index = 1 - index
    result = []
    
    print "Choosing Research Techlist Index %d" % index
    if index == 0:
        result = tech_group_1a()  # early org_hull
        result += tech_group_2a()  # prioritizes growth & defense over weapons
        result += tech_group_3a()
        result += tech_group_4a()
        result += tech_group_5()  #
    elif index == 1:
        result = tech_group_1a()  # early _lrn_artif_minds
        result += tech_group_2a()  # prioritizes growth & defense over weapons
        result += tech_group_3a()
        result += tech_group_4a()
        result += tech_group_5()  #
    elif index == 2:
        result = tech_group_1a()  # early _lrn_artif_minds
        result += tech_group_2a()  # prioritizes growth & defense over weapons
        result += tech_group_3a()
        result += tech_group_4a()
        result += tech_group_5()  #
    elif index == 3:
        result = tech_group_1b()  # 
        result += tech_group_2b()  # prioritizes weapons over growth & defense
        result += tech_group_3b()  # 3a plus early plasma weaps
        result += tech_group_4b()
        result += tech_group_5()  #
    elif index == 4:
        result = tech_group_1a()  # early _lrn_artif_minds
        result += tech_group_2a()  # prioritizes growth & defense over weapons
        result += tech_group_3b()  # 3a plus early plasma weaps
        result += tech_group_4b()
        result += tech_group_5()  #

    return result

#the following is just for reference
# "CON_ARCH_MONOFILS",
# "CON_ARCH_PSYCH",
# "CON_ART_HEAVENLY",
# "CON_ART_PLANET",
# "CON_ASYMP_MATS",
# "CON_CONC_CAMP",
# "CON_CONTGRAV_ARCH",
# "CON_FRC_ENRG_CAMO",
# "CON_FRC_ENRG_STRC",
# "CON_GAL_INFRA",
# "CON_NDIM_STRC",
# "CON_ORBITAL_CON",
# "CON_ORBITAL_HAB",
# "CON_ORGANIC_STRC",
# "CON_PLANET_DRIVE",
# "CON_STARGATE",
# "CON_TRANS_ARCH",
# "DEF_DEFENSE_NET_1",
# "DEF_DEFENSE_NET_2",
# "DEF_DEFENSE_NET_3",
# "DEF_DEFENSE_NET_REGEN_1",
# "DEF_DEFENSE_NET_REGEN_2",
# "DEF_GARRISON_1",
# "DEF_GARRISON_2",
# "DEF_GARRISON_3",
# "DEF_GARRISON_4",
# "DEF_PLANET_CLOAK",
# "DEF_PLAN_BARRIER_SHLD_1",
# "DEF_PLAN_BARRIER_SHLD_2",
# "DEF_PLAN_BARRIER_SHLD_3",
# "DEF_PLAN_BARRIER_SHLD_4",
# "DEF_PLAN_BARRIER_SHLD_5",
# "DEF_ROOT_DEFENSE",
# "DEF_SYST_DEF_MINE_1",
# "DEF_SYST_DEF_MINE_2",
# "DEF_SYST_DEF_MINE_3",
# "GRO_ADV_ECOMAN",
# "GRO_BIOTERROR",
# "GRO_CYBORG",
# "GRO_ENERGY_META",
# "GRO_GAIA_TRANS",
# "GRO_GENETIC_ENG",
# "GRO_GENETIC_MED",
# "GRO_LIFECYCLE_MAN",
# "GRO_NANOTECH_MED",
# "GRO_NANO_CYBERNET",
# "GRO_PLANET_ECOL",
# "GRO_SUBTER_HAB",
# "GRO_SYMBIOTIC_BIO",
# "GRO_TERRAFORM",
# "GRO_TRANSORG_SENT",
# "GRO_XENO_GENETICS",
# "GRO_XENO_HYBRIDS",
# "LRN_ALGO_ELEGANCE",
# "LRN_ARTIF_MINDS",
# "LRN_ART_BLACK_HOLE",
# "LRN_DISTRIB_THOUGHT",
# "LRN_ENCLAVE_VOID",
# "LRN_EVERYTHING",
# "LRN_FORCE_FIELD",
# "LRN_GATEWAY_VOID",
# "LRN_GRAVITONICS",
# "LRN_MIND_VOID",
# "LRN_NDIM_SUBSPACE",
# "LRN_OBSERVATORY_I",
# "LRN_PHYS_BRAIN",
# "LRN_PSIONICS",
# "LRN_PSY_DOM",
# "LRN_QUANT_NET",
# "LRN_SPATIAL_DISTORT_GEN",
# "LRN_STELLAR_TOMOGRAPHY",
# "LRN_TIME_MECH",
# "LRN_TRANSCEND",
# "LRN_TRANSLING_THT",
# "LRN_UNIF_CONC",
# "LRN_XENOARCH",
# "PRO_EXOBOTS",
# "PRO_FUSION_GEN",
# "PRO_INDUSTRY_CENTER_I",
# "PRO_INDUSTRY_CENTER_II",
# "PRO_INDUSTRY_CENTER_III",
# "PRO_MICROGRAV_MAN",
# "PRO_NANOTECH_PROD",
# "PRO_NDIM_ASSMB",
# "PRO_NEUTRONIUM_EXTRACTION",
# "PRO_ORBITAL_GEN",
# "PRO_ROBOTIC_PROD",
# "PRO_SENTIENT_AUTOMATION",
# "PRO_SINGULAR_GEN",
# "PRO_SOL_ORB_GEN",
# "PRO_ZERO_GEN",
# "SHP_ADV_DAM_CONT",
# "SHP_ANTIMATTER_TANK",
# "SHP_ASTEROID_HULLS",
# "SHP_ASTEROID_REFORM",
# "SHP_BASIC_DAM_CONT",
# "SHP_BIOADAPTIVE_SPEC",
# "SHP_BIOTERM",
# "SHP_BLACKSHIELD",
# "SHP_CAMO_AST_HULL",
# "SHP_CONTGRAV_MAINT",
# "SHP_CONT_BIOADAPT",
# "SHP_CONT_SYMB",
# "SHP_DEATH_SPORE",
# "SHP_DEFLECTOR_SHIELD",
# "SHP_DEUTERIUM_TANK",
# "SHP_DIAMOND_PLATE",
# "SHP_DOMESTIC_MONSTER",
# "SHP_ENDOCRINE_SYSTEMS",
# "SHP_ENDOSYMB_HULL",
# "SHP_ENRG_BOUND_MAN",
# "SHP_FLEET_REPAIR",
# "SHP_FRC_ENRG_COMP",
# "SHP_GAL_EXPLO",
# "SHP_HEAVY_AST_HULL",
# "SHP_IMPROVED_ENGINE_COUPLINGS",
# "SHP_INTSTEL_LOG",
# "SHP_MASSPROP_SPEC",
# "SHP_MIDCOMB_LOG",
# "SHP_MIL_ROBO_CONT",
# "SHP_MINIAST_SWARM",
# "SHP_MONOCELL_EXP",
# "SHP_MONOMOLEC_LATTICE",
# "SHP_MULTICELL_CAST",
# "SHP_MULTISPEC_SHIELD",
# "SHP_NANOROBO_MAINT",
# "SHP_NOVA_BOMB",
# "SHP_N_DIMENSIONAL_ENGINE_MATRIX",
# "SHP_ORG_HULL",
# "SHP_PLASMA_SHIELD",
# "SHP_QUANT_ENRG_MAG",
# "SHP_REINFORCED_HULL",
# "SHP_ROOT_AGGRESSION",
# "SHP_ROOT_ARMOR",
# "SHP_SCAT_AST_HULL",
# "SHP_SENT_HULL",
# "SHP_SINGULARITY_ENGINE_CORE",
# "SHP_SOLAR_CONT",
# "SHP_SPACE_FLUX_DRIVE",
# "SHP_TRANSSPACE_DRIVE",
# "SHP_WEAPON_1_2",
# "SHP_WEAPON_1_3",
# "SHP_WEAPON_1_4",
# "SHP_WEAPON_2_1",
# "SHP_WEAPON_2_2",
# "SHP_WEAPON_2_3",
# "SHP_WEAPON_2_4",
# "SHP_WEAPON_3_1",
# "SHP_WEAPON_3_2",
# "SHP_WEAPON_3_3",
# "SHP_WEAPON_3_4",
# "SHP_WEAPON_4_1",
# "SHP_WEAPON_4_2",
# "SHP_WEAPON_4_3",
# "SHP_WEAPON_4_4",
# "SHP_XENTRONIUM_PLATE",
# "SHP_ZORTRIUM_PLATE",
# "SPY_DETECT_1",
# "SPY_DETECT_2",
# "SPY_DETECT_3",
# "SPY_DETECT_4",
# "SPY_DETECT_5",
# "SPY_DIST_MOD",
# "SPY_LIGHTHOUSE",
# "SPY_PLANET_STEALTH_MOD",
# "SPY_ROOT_DECEPTION",
# "SPY_STEALTH_1",
# "SPY_STEALTH_2",
# "SPY_STEALTH_3",
# "SPY_STEALTH_4",
