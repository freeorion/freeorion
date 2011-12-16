# TechsListsAI module is utilized by ResearchAI module
# AI can not currently add new building types to production queue
# AI can not currently design or build custom ships
# individual techs are to be removed from unusable list as AI programming progresses

def unusableTechsList():
    "techs currently unusable by the AI"

    unusableTechs = [

        'CON_ART_HEAVENLY',
        'CON_ART_MOON',
        'CON_ART_PLANET',
	'CON_CONC_CAMP',
	'CON_FRC_ENRG_CAMO',
	'CON_INFRA_ECOL',
	'CON_MEGALITH',
	'CON_ORGANIC_STRC',
	'CON_PLANET_DRIVE',
        'CON_SPACE_ELEVATOR',
        'CON_STARGATE',
	'CON_TRANS_ARCH',
	'CON_TRANS_STRC',
        'GRO_BIOTERROR',
        'GRO_GENOME_BANK',
	'GRO_GAIA_TRANS',
	'GRO_INDUSTRY_CLONE',
	'GRO_INDUSTRY_FARM',
	'GRO_REMOTE_TERRAFORM',
	'GRO_SUSPENDED_ANIMATION',
	'GRO_TERRAFORM',
        'LRN_ART_BLACK_HOLE',
	'LRN_COLLECTIVE_NET',
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
        'LRN_XENOARCH_RESTORE',
	'PRO_BLACK_HOLE_POW_GEN',
	'PRO_ENERGY_CONV',
	'PRO_ENVIRO_MINING',
	'PRO_GAS_GIANT_GEN',
	'PRO_HEAVY_MINING_I',
	'PRO_HYPER_DAM',
	'PRO_INDUSTRY_CENTER_I',
	'PRO_INDUSTRY_CENTER_II',
	'PRO_INDUSTRY_CENTER_III',
	'PRO_MATENG_REPLIC',
	'PRO_NEUTRONIUM_EXTRACTION',
        'PRO_NDIM_ASSMB',
	'PRO_ORBITAL_GEN',
        'PRO_ORBITAL_MINE',
        'PRO_SINGULAR_GEN',
	'PRO_SOL_ORB_GEN',
        'PRO_ZERO_GEN',
	'SHP_AGREG_AST_HULL',
	'SHP_ANTIMAT_TORP',
	'SHP_ANTIMATTER_TANK',
	'SHP_AST_HULL',
	'SHP_ASTEROID_HULLS',
	'SHP_ASTEROID_REFORM',
        'SHP_BIOADAPT_HULL',
        'SHP_BIOBOMBER',
        'SHP_BIOINTERCEPTOR',
        'SHP_BIONEUR_SPEC',
	'SHP_BOMBER',
	'SHP_CAMO_AST_HULL',
	'SHP_CAMO_AST_PARTS',
	'SHP_COMP_ENRG_HULL',
        'SHP_CONT_BIOADAPT',
        'SHP_CONT_SYMB',
	'SHP_CONTGRAV_MAINT',
	'SHP_CRYSTAL_AST_HULL',
	'SHP_CRYSTAL_PLATE',
        'SHP_DEATH_RAY',
	'SHP_DEFLECTOR',
	'SHP_DEUTERIUM_TANK',
	'SHP_DIST_MOD',
        'SHP_ENDOMORPH_HULL',
        'SHP_ENDOSYMB_HULL',
	'SHP_ENRG_BOUND_MAN',
	'SHP_FLEET_LOGISTICS',
	'SHP_FRAC_ENRG_HULL',
	'SHP_FRC_ENRG_COMP',
        'SHP_HAB_MEGAFAUN',
	'SHP_HEAVY_AST_HULL',
	'SHP_INTERCEPTOR',
	'SHP_LEAD_PLATE',
        'SHP_LIGHTHOUSE',
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
	'SHP_NEUTRONIUM_PLATE',
	'SHP_NEUTRONIUM_PLATE_NUC_MIS',
	'SHP_NEUTRONIUM_PLATE_SPEC_MIS',
	'SHP_NUCLEAR_MISSILE',
        'SHP_ORG_HULL',
        'SHP_PHASOR',
        'SHP_PLANET_CLOAK',
        'SHP_PLASMA_TORP',
	'SHP_PULSE_LASER',
        'SHP_PROTOPLASM_HULL',
	'SHP_QUANT_ENRG_HULL',
	'SHP_QUANT_ENRG_MAG',
        'SHP_RAVEN_HULL',
	'SHP_RECON_FIGHT',
	'SHP_ROBO_HULL',
	'SHP_ROCK_PLATE',
	'SHP_SCAT_AST_HULL',
	'SHP_SELFGRAV_HULL',
        'SHP_SENT_HULL',
	'SHP_SMALL_AST_HULL',
	'SHP_SOLAR_CONT',
	'SHP_SOLAR_HULL',
	'SHP_SPACE_FLUX_DRIVE',
	'SHP_SPACE_FLUX_HULL',
	'SHP_SPECTRAL_MISSILE',
        'SHP_STAT_MULTICELL_HULL',
        'SHP_SYMB_HULL',
        'SHP_TITAN_HULL',
	'SHP_TRANSSPACE_DRIVE',
        'SHP_TRANSSPACE_HULL',
        'SHP_ZORTRIUM_PLATE']

    return unusableTechs

def primaryLearningTechsList():
    "primary learning techs"

    primaryLearningTechs = [

        'LRN_ALGO_ELEGANCE',
        'LRN_ARTIF_MINDS',
	'LRN_AUTOLAB_I']

    return primaryLearningTechs

def primaryGroTechsList():
    "primary growth techs"

    primaryGroTechs = [

        'GRO_ENV_ENCAPSUL',
        'GRO_HABITATION_DOMES']

    return primaryGroTechs

def primaryShipsTechsList():
    "primary ships techs"

    primaryShipsTechs = [

        'SHP_GAL_EXPLO',
        'SHP_SPACE_TACTICS',
        'SHP_SPACE_WEAPON',
	'SHP_ION_CANNON']

    return primaryShipsTechs
