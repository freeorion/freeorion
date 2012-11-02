# AI can not currently add new building types to production queue
# AI can not currently design or build custom ships
# individual techs are to be removed from unusable list as AI programming progresses

def unusableTechsList():
    "techs currently unusable by the AI"

    unusableTechs = []
    ptechs = primaryMetaTechsList()
    for tech in [    

        'CON_ART_HEAVENLY',
        'CON_ART_PLANET',
        'CON_CONC_CAMP',
        'CON_FRC_ENRG_CAMO',
        'CON_INFRA_ECOL',
        'CON_ORGANIC_STRC',
        'CON_PLANET_DRIVE',
        'CON_SPACE_ELEVATOR',
        'CON_STARGATE',
        'CON_TRANS_ARCH',
        'SPY_LIGHTHOUSE',
        'GRO_BIOTERROR',
        'GRO_GENOME_BANK',
        'GRO_GAIA_TRANS',
        'GRO_INDUSTRY_CLONE',
        'GRO_INDUSTRY_FARM',
        'GRO_REMOTE_TERRAFORM',
        'GRO_SUSPENDED_ANIMATION',
        'GRO_TERRAFORM',
        'LRN_ART_BLACK_HOLE',
        'LRN_ENCLAVE_VOID',
        'LRN_GATEWAY_VOID',
        'LRN_MIND_VOID',
        'LRN_OBSERVATORY_I',
        'LRN_PSY_DOM',
        'LRN_SOLAR_MAN',
        'LRN_SPATIAL_DISTORT_GEN',
        'LRN_TIME_MECH',
        'LRN_TRANSCEND',
        'LRN_UNIF_CONC',
        'LRN_XENOARCH',
        'PRO_ENVIRO_MINING',
        'PRO_HEAVY_MINING_I',
        'PRO_INDUSTRY_CENTER_I',
        'PRO_INDUSTRY_CENTER_II',
        'PRO_INDUSTRY_CENTER_III',
        'PRO_NEUTRONIUM_EXTRACTION',
        'PRO_NDIM_ASSMB',
        'PRO_ORBITAL_GEN',
        'PRO_ORBITAL_MINE',
        'PRO_SINGULAR_GEN',
        'PRO_SOL_ORB_GEN',
        'PRO_ZERO_GEN',
        'SHP_ANTIMAT_TORP',
        'SHP_ANTIMATTER_TANK',
        'SHP_ASTEROID_HULLS',
        'SHP_ASTEROID_REFORM',
        'SHP_BIOADAPT_HULL',
        'SHP_BIOBOMBER',
        'SHP_BIOINTERCEPTOR',
        'SHP_BIONEUR_SPEC',
        'SHP_BOMBER',
        'SHP_CAMO_AST_HULL',
        'SHP_CONT_BIOADAPT',
        'SHP_CONT_SYMB',
        'SHP_CONTGRAV_MAINT',
        'SHP_DEATH_RAY',
        'SHP_DEUTERIUM_TANK',
        'SPY_DIST_MOD',
        'SHP_DOMESTIC_MONSTER',
        'SHP_ENDOMORPH_HULL',
        'SHP_ENDOSYMB_HULL',
        'SHP_ENRG_BOUND_MAN',
        'SHP_FRC_ENRG_COMP',
        'SHP_HAB_MEGAFAUN',
        'SHP_INTERCEPTOR',
        'SHP_LEAD_PLATE',
        'SHP_LOGISTICS_FAC',
        'SHP_MASS_DRIVER',
        'SHP_MASSPROP_SPEC',
        'SHP_MIDCOMB_LOG',
        'SHP_MIL_ROBO_CONT',
        'SHP_MINIAST_SWARM',
        'SHP_MONOCELL_EXP',
        'SHP_MONOMOLEC_LATTICE',
        'SHP_MULTICELL_CAST',
        'SHP_MULTISPEC_SHIELD',
        'SHP_NANOROBO_HULL',
        'SHP_NANOROBO_MAINT',
        'SPY_NEUTRON_SCANNER',
        'SHP_NEUTRONIUM_PLATE',
        'SHP_NEUTRONIUM_PLATE_NUC_MIS',
        'SHP_NEUTRONIUM_PLATE_SPEC_MIS',
        'SHP_NUCLEAR_MISSILE',
        'SHP_ORG_HULL',
        'SHP_PHASOR',
        'DEF_PLANET_CLOAK',
        'SHP_PLASMA_TORP',
        'SHP_PULSE_LASER',
        'SHP_PROTOPLASM_HULL',
        'SHP_QUANT_ENRG_MAG',
        'SPY_RADAR',
        'SHP_RECON_FIGHT',
        'SHP_ROCK_PLATE',
        'SHP_SCAT_AST_HULL',
        'SHP_SELFGRAV_HULL',
        'SPY_SENSORS',
        'SHP_SENT_HULL',
        'SHP_SOLAR_CONT',
        'SHP_SPACE_FLUX_DRIVE',
        'SHP_SPECTRAL_MISSILE',
        'SHP_STAT_MULTICELL_HULL',
        'SHP_SYMB_HULL',
        'SHP_TITAN_HULL',
        'SHP_TRANSSPACE_DRIVE',
        'SHP_ZORTRIUM_PLATE',
        'SHP_WEAPON_11',
        'SHP_WEAPON_12',
        'SHP_WEAPON_13',
        'SHP_WEAPON_14',
        'SHP_WEAPON_15',
        'SHP_WEAPON_16',
        'SHP_WEAPON_17'
    ] :
        if tech not in ptechs:
            unusableTechs.append(tech)

    return unusableTechs

def primaryLearningTechsList():
    "primary learning techs"

    primaryLearningTechs = [

        'LRN_ALGO_ELEGANCE',
        'LRN_ARTIF_MINDS']

    return primaryLearningTechs

def primaryGroTechsList():
    "primary growth techs"

    primaryGroTechs = [

        'CON_ENV_ENCAPSUL']
 
    return primaryGroTechs

def primaryShipsTechsList():
    "primary ships techs"

    primaryShipsTechs = [
        'SHP_WEAPON_2',
        'SHP_WEAPON_3',
        'SHP_WEAPON_4',
        'SHP_WEAPON_5',
        'SHP_DEFLECTOR',
        'SHP_WEAPON_6',
        'SHP_WEAPON_7',
        'SHP_WEAPON_8',
        'SHP_WEAPON_9',
        'SHP_WEAPON_10'
        ]

    return primaryShipsTechs

def primaryMetaTechsList():
    "primary techs for al categories"


    primaryMetaTechs = [
        'GRO_SUBTER_HAB', 
        'DEF_GARRISON_1', 
        'SHP_WEAPON_2',
        'SHP_HAB_MEGAFAUN', 
        'SHP_WEAPON_3',
        'SHP_WEAPON_4',
        'SHP_WEAPON_5',
        'GRO_SYMBIOTIC_BIO', 
        'SHP_GAL_EXPLO', 
        'SHP_SPACE_TACTICS', 
        'SHP_DEFLECTOR',
        'GRO_GENETIC_ENG', 
        'CON_ENV_ENCAPSUL', 
        'GRO_XENO_GENETICS', 
        'PRO_MICROGRAV_MAN', 
        'CON_ORBITAL_CON', 
        'LRN_ALGO_ELEGANCE', 
        'LRN_ARTIF_MINDS', 
        'GRO_XENO_HYBRIDS', 
        'PRO_ROBOTIC_PROD', 
        "PRO_EXOBOTS",
        'SHP_WEAPON_6',
        'SHP_WEAPON_7',
        'SHP_WEAPON_8',
        'SHP_DOMESTIC_MONSTER', 
        'SHP_ORG_HULL', 
        'SHP_MULTICELL_CAST', 
        'DEF_DEFENSE_NET', 
        'PRO_FUSION_GEN', 
        'PRO_INDUSTRY_CENTER_I', 
        'LRN_PHYS_BRAIN', 
        'LRN_TRANSLING_THT', 
        'LRN_PSIONICS', 
        'LRN_XENOARCH',
        'LRN_DISTRIB_THOUGHT', 
        'DEF_GARRISON_2', 
        'DEF_GARRISON_3', 
        'DEF_GARRISON_4', 
        'CON_METRO_INFRA', 
        'CON_SUBTER_CONST', 
        'DEF_PLAN_DEFLECT_SHLD', 
        'PRO_INDUSTRY_CENTER_II', 
        'SHP_NAVIGATION', 
        'SPY_RADAR', 
        'SHP_BASIC_DAM_CONT', 
        'SHP_ADV_NAVIGATION', 
        'SHP_ADV_DAM_CONT', 
        'SHP_WEAPON_9',
        'SHP_WEAPON_10', 
        'GRO_GENETIC_MED',
        'PRO_NANOTECH_PROD', 
        'GRO_NANOTECH_MED', 
        'GRO_NANO_CYBERNET', 
        'GRO_CYBORG', 
        'PRO_INDUSTRY_CENTER_III', 
        'PRO_SENTIENT_AUTOMATION', 
        'SHP_WEAPON_11', 
        'SHP_WEAPON_12', 
        'GRO_PLANET_ECOL', 
        'GRO_ADV_ECOMAN', 
        'GRO_TERRAFORM',
        ]
    return primaryMetaTechs
