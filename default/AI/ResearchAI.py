import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import TechsListsAI
import AIDependencies
import AIstate
import traceback
import ColonisationAI
import ShipDesignAI
import random

from freeorion_tools import tech_is_complete, chat_human

inProgressTechs = {}


def get_research_index():
    empire_id = fo.empireID()
    research_index = empire_id % 2
    if foAI.foAIstate.aggression >= fo.aggression.aggressive:  # maniacal
        research_index = 2 + (empire_id % 3)  # so indices [2,3,4]
    elif foAI.foAIstate.aggression >= fo.aggression.typical:
        research_index += 1
    return research_index

def has_only_bad_colonizers():
    most_adequate = 0
    for specName in ColonisationAI.empire_colonizers:
        environs = {}
        this_spec = fo.getSpecies(specName)
        if not this_spec:
            continue
        for ptype in [fo.planetType.swamp, fo.planetType.radiated, fo.planetType.toxic, fo.planetType.inferno, fo.planetType.barren, fo.planetType.tundra, fo.planetType.desert, fo.planetType.terran, fo.planetType.ocean, fo.planetType.asteroids]:
            environ = this_spec.getPlanetEnvironment(ptype)
            environs.setdefault(environ, []).append(ptype)
        most_adequate = max(most_adequate, len(environs.get(fo.planetEnvironment.adequate, [])))
    return most_adequate == 0

def get_priority(tech_name):
    """
    Get tech priority. 1 is default. 0 if not useful (but doesn't hurt to research),
    < 0 to prevent AI to research it
    """

    rng = random.Random()
    rng.seed(fo.getEmpire().name + fo.getGalaxySetupData().seed)

    IMMEDIATE = 999 # i.e. research this tech NOW!
    USELESS = 0 # useless for AI, mostly because the AI doesn't know how to use them
    DEFAULT = 1
    UNRESEARCHABLE = -1 # anything unresearchable should use this TODO obtain this information from techs.txt

    theory = 0 # theories has 0 priority as they don't unlock anything. If it ends up prereq for any other tech those tech will add the theories in.

    growth = 2 # maximum population growth has higher priority.
    production_boost = 1.5 # production boosts have higher priority. This include buildings that boosts production
    research_boost = 2 # research boosts have higher priority. This include buildings that boosts research
    meter_boost = 1
    shield = 1.25 # shields are more powerful than armors or weapons
    # TODO reduce priority at very early stage for defense techs, until enemies seen
    defensive = 2 if foAI.foAIstate.aggression <= fo.aggression.cautious else (1 if fo.currentTurn() > 10 else 0.5)
    # TODO consider starlane density and planet density instead
    supply = 2 if ColonisationAI.galaxy_is_sparse() else 0.5

    has_red_star = len(AIstate.empireStars.get(fo.starType.red, [])) != 0
    has_blue_star = len(AIstate.empireStars.get(fo.starType.blue, [])) != 0
    has_black_hole = len(AIstate.empireStars.get(fo.starType.blackHole, [])) != 0
    has_neutron_star = len(AIstate.empireStars.get(fo.starType.neutron , [])) != 0

    hull = 1
    offtrack_hull = 0.05
    offtrack_subhull = 0.25
    # select one hull and specialize it, but some AI may want to have more hulls, by random
    chosen_hull = rng.randrange(4)
    org = hull if chosen_hull % 2 == 0 or rng.random() < 0.05 else offtrack_hull
    robotic = hull if chosen_hull % 2 == 1 or rng.random() < 0.05 else offtrack_hull
    if ColonisationAI.got_ast:
        extra = rng.random() < 0.05
        asteroid = hull if chosen_hull == 2 or extra else offtrack_hull
        if asteroid == hull and not extra:
            org = offtrack_hull
            robotic = offtrack_hull
    else:
        asteroid = offtrack_hull
    if has_blue_star or has_black_hole:
        extra = rng.random() < 0.05
        energy = hull if chosen_hull == 3 or extra else offtrack_hull
        if energy == hull and not extra:
            org = offtrack_hull
            robotic = offtrack_hull
            asteroid = offtrack_hull
    else:
        energy = offtrack_hull

    # TODO consider subhulls, weapons, armors, engines and fuels by estimating their strengths
    # https://github.com/freeorion/freeorion/pull/178
    # further specialization for robotic hulls
    chosen_subhull = rng.randrange(2)
    contgrav = hull if robotic == hull and chosen_subhull == 0 else offtrack_subhull
    nanorobo = hull if robotic == hull and chosen_subhull == 1 else offtrack_subhull
    flux = hull if robotic == hull and rng.random() < 0.5 else offtrack_subhull
    # further specialization for asteroid hulls
    chosen_subhull = rng.randrange(3)
    astheavy = hull if asteroid == hull and chosen_subhull == 0 else offtrack_subhull
    astswarm = hull if asteroid == hull and chosen_subhull == 1 else offtrack_subhull
    astcamo = hull if asteroid == hull and chosen_subhull == 2 else offtrack_subhull
    # further specialization for organic hulls
    chosen_subhull = rng.randrange(2)
    orgneural = hull if org == hull and chosen_subhull == 0 else offtrack_subhull
    orgraven = hull if org == hull and chosen_subhull == 1 else offtrack_subhull
    orgendosym = hull if org == hull and rng.random() < 0.5 else offtrack_subhull
    # further specialization for energy hulls
    chosen_subhull = rng.randrange(2)
    energyfrac = hull if energy == hull and chosen_subhull == 0 else offtrack_subhull
    energymag = hull if energy == hull and chosen_subhull == 1 else offtrack_subhull
    # repair techs may be skipped if AI decides to go for nanorobotic hull which full-repairs
    repair = 1 if not nanorobo == hull or rng.random() < 0.75 else 0.3

    # (Disabled) AI may skip weapon lines
    weapon = 1
    # massdriver = weapon if rng.random() < 0.75 else 0
    # laser = weapon if (rng.random() < 0.4 if massdriver == weapon else rng.random() < 0.75) else 0
    # plasmacannon = weapon if (rng.random() < 0.4 if laser == weapon else rng.random() < 0.75) else 0
    massdriver = weapon
    laser = weapon
    plasmacannon = weapon
    deathray = weapon

    armor = 1

    engine = 1 if (rng.random() < 0.8 if ColonisationAI.galaxy_is_sparse() else rng.random() < 0.4) else 0
    fuel = 1 if (rng.random() < 0.8 if ColonisationAI.galaxy_is_sparse() else rng.random() < 0.4) else 0

    detection = 1 # TODO Consider stealth of enemies
    stealth = 2.2 if rng.random() < 0.1 else 0.2 # TODO stealthy species probably want more stealthy techs, and base one enemies detection

    if tech_name in ["SHP_KRILL_SPAWN", "DEF_PLANET_CLOAK"]:
        return UNRESEARCHABLE

    # defense techs
    if tech_name in ["DEF_GARRISON_1", "DEF_GARRISON_2", "DEF_GARRISON_3", "DEF_GARRISON_4"]:
        return defensive - 0.01 # garrison are less important/powerful among all defenses
    if tech_name in ["DEF_DEFENSE_NET_1", "DEF_DEFENSE_NET_2", "DEF_DEFENSE_NET_3"]:
        return defensive
    if tech_name in ["DEF_DEFENSE_NET_REGEN_1", "DEF_DEFENSE_NET_REGEN_2"]:
        return defensive
    if tech_name in ["DEF_PLAN_BARRIER_SHLD_1", "DEF_PLAN_BARRIER_SHLD_2", "DEF_PLAN_BARRIER_SHLD_3", "DEF_PLAN_BARRIER_SHLD_4", "DEF_PLAN_BARRIER_SHLD_5"]:
        return defensive
    if tech_name in ["DEF_SYST_DEF_MINE_1", "DEF_SYST_DEF_MINE_2", "DEF_SYST_DEF_MINE_3"]:
        return defensive + 0.01 # mines are more important/powerful among all defenses

    # learning
    if tech_name in ["LRN_PHYS_BRAIN", "LRN_TRANSLING_THT", "LRN_PSIONICS", "LRN_GRAVITONICS", "LRN_EVERYTHING", "LRN_MIND_VOID", "LRN_NDIM_SUBSPACE", "LRN_TIME_MECH"]:
        return theory
    if tech_name in ["LRN_ALGO_ELEGANCE", "LRN_ARTIF_MINDS", "LRN_DISTRIB_THOUGHT", "LRN_QUANT_NET", "LRN_STELLAR_TOMOGRAPHY"]:
        return research_boost
    if tech_name in ["LRN_ENCLAVE_VOID", "LRN_UNIF_CONC"]:
        return research_boost
    if tech_name in ["LRN_XENOARCH"]:
        if foAI.foAIstate.aggression < fo.aggression.typical:
            return DEFAULT
        return 5 if ColonisationAI.gotRuins else USELESS # get xenoarcheology when we have ruins, otherwise it is useless
    if tech_name in ["LRN_FORCE_FIELD"]:
        return shield
    if tech_name in ["LRN_SPATIAL_DISTORT_GEN", "LRN_GATEWAY_VOID", "LRN_PSY_DOM"]:
        return USELESS
    if tech_name in ["LRN_ART_BLACK_HOLE"]:
        if has_black_hole or not has_red_star:
            return USELESS
        if tech_is_complete("SHP_SOLAR_CONT"):
            return IMMEDIATE
        return DEFAULT
    if tech_name in ["LRN_TRANSCEND"]:
        return DEFAULT # Transcendence requires so much RP anyway it will be higher and only higher than techs considered useless lol

    # growth
    if tech_name in ["GRO_PLANET_ECOL", "GRO_GENETIC_ENG", "GRO_ADV_ECOMAN", "GRO_NANOTECH_MED", "GRO_TRANSORG_SENT"]:
        return theory
    if tech_name in ["GRO_SYMBIOTIC_BIO", "GRO_XENO_HYBRIDS", "GRO_CYBORG", "GRO_SUBTER_HAB"]:
        return growth
    if tech_name in ["GRO_XENO_GENETICS"]:
        if foAI.foAIstate.aggression < fo.aggression.cautious:
            return DEFAULT
        if has_only_bad_colonizers():
            return growth * 3 # Empire only have lousy colonisers, xeno-genetics are really important for them
        else:
            return growth
    if tech_name in ["GRO_GENETIC_MED"]:
        return DEFAULT # TODO boost this to get the genome bank if enemy is using bioterror?
    if tech_name in ["GRO_LIFECYCLE_MAN", "GRO_NANO_CYBERNET"]: # TODO consider ship part simulation calculation
        return DEFAULT
    if tech_name in ["GRO_ENERGY_META"]:
        return production_boost + research_boost
    if tech_name in ["GRO_TERRAFORM", "GRO_BIOTERROR", "GRO_GAIA_TRANS"]:
        return USELESS

    # Production
    if tech_name in ["PRO_NANOTECH_PROD", "PRO_ZERO_GEN"]:
        return theory
    if tech_name in ["PRO_ROBOTIC_PROD", "PRO_FUSION_GEN", "PRO_SENTIENT_AUTOMATION"]:
        return production_boost
    if tech_name in ["PRO_INDUSTRY_CENTER_I", "PRO_INDUSTRY_CENTER_II", "PRO_INDUSTRY_CENTER_III", "PRO_SOL_ORB_GEN"]:
        return production_boost
    if tech_name in ["PRO_MICROGRAV_MAN"]:
        return production_boost if ColonisationAI.got_ast else USELESS # useless if we have no plan for asteroids
    if tech_name in ["PRO_ORBITAL_GEN"]:
        return production_boost if ColonisationAI.got_gg else USELESS # useless if we have no plan for gas giants
    if tech_name in ["PRO_EXOBOTS"]:
        return DEFAULT
    if tech_name in ["PRO_NDIM_ASSMB"]:
        return DEFAULT # TODO does AI use/value Hyperspatial Dam?
    if tech_name in ["PRO_SINGULAR_GEN"]:
        return production_boost if has_black_hole else USELESS
    if tech_name in ["PRO_NEUTRONIUM_EXTRACTION"]:
        return armor if has_neutron_star else USELESS # application of neutronium extraction is armor only for now

    # Construction
    if tech_name in ["CON_ASYMP_MATS", "CON_ARCH_PSYCH"]:
        return theory
    if tech_name in ["CON_ARCH_MONOFILS", "CON_GAL_INFRA", "CON_CONTGRAV_ARCH", "CON_ORBITAL_CON"]:
        return supply
    if tech_name in ["CON_FRC_ENRG_STRC"]:
        return meter_boost
    if tech_name in ["CON_ORBITAL_HAB", "CON_NDIM_STRC"]:
        return growth
    if tech_name in ["CON_TRANS_ARCH"]:
        return meter_boost
    if tech_name in ["CON_ORGANIC_STRC", "CON_PLANET_DRIVE", "CON_STARGATE"]:
        return USELESS # AI does not use stargate, planetary starline drive, spatial distortion nor bioterror
    if tech_name in ["CON_ART_HEAVENLY", "CON_ART_PLANET"]:
        return USELESS # AI does not create artificial moons or planets
    if tech_name in ["CON_CONC_CAMP"]:
        return 0 # TODO Concentration camps are now disabled
    if tech_name in ["CON_FRC_ENRG_CAMO"]:
        return stealth

    # Ships
    if tech_name in ["SHP_GAL_EXPLO"]:
        return theory
    if tech_name in ["SHP_DOMESTIC_MONSTER"]:
        return USELESS # TODO does "Tames space monsters" really work?!?! Does AI know this? Anyway tamable space monsters are rare...
    if tech_name in ["SHP_INTSTEL_LOG"]:
        return DEFAULT # interstellar logistics increases ship speed

    # Robotic hulls
    if tech_name in ["SHP_MIL_ROBO_CONT", "SHP_MIDCOMB_LOG"]:
        return robotic
    if tech_name in ["SHP_SPACE_FLUX_DRIVE", "SHP_TRANSSPACE_DRIVE"]:
        return min(robotic, flux)
    if tech_name in ["SHP_CONTGRAV_MAINT", "SHP_MASSPROP_SPEC"]:
        return min(robotic, contgrav)
    if tech_name in ["SHP_NANOROBO_MAINT"]:
        return min(robotic, nanorobo)
    if tech_name in ["SHP_XENTRONIUM_HULL"]:
        return DEFAULT # this is not a robotic hull!

    # Asteroid hulls
    if tech_name in ["SHP_ASTEROID_HULLS", "SHP_ASTEROID_REFORM", "SHP_MONOMOLEC_LATTICE", "SHP_SCAT_AST_HULL"]:
        return asteroid
    if tech_name in ["SHP_HEAVY_AST_HULL"]:
        return min(asteroid, astheavy)
    if tech_name in ["SHP_CAMO_AST_HULL"]:
        return min(asteroid, astcamo)
    if tech_name in ["SHP_MINIAST_SWARM"]:
        return min(asteroid, astswarm)

    # Organic hulls
    if tech_name in ["SHP_ORG_HULL", "SHP_SENT_HULL"]:
        return org
    if tech_name in ["SHP_MULTICELL_CAST", "SHP_ENDOCRINE_SYSTEMS", "SHP_CONT_BIOADAPT"]:
        return min(org, orgraven)
    if tech_name in ["SHP_MONOCELL_EXP", "SHP_CONT_SYMB", "SHP_BIOADAPTIVE_SPEC"]:
        return min(org, orgneural)
    if tech_name in ["SHP_ENDOSYMB_HULL"]:
        return min(org, orgendosym)

    # energy hulls
    if tech_name in ["SHP_FRC_ENRG_COMP"]:
        return energy
    if tech_name in ["SHP_QUANT_ENRG_MAG"]:
        return min(energy, energymag, IMMEDIATE if has_blue_star or has_black_hole else USELESS)
    if tech_name in ["SHP_ENRG_BOUND_MAN"]:
        return min(energy, energyfrac, IMMEDIATE if has_blue_star or has_black_hole else USELESS)
    if tech_name in ["SHP_SOLAR_CONT"]:
        return min(energy, IMMEDIATE if has_red_star or has_black_hole else USELESS) # red star can be turned into has_black_hole by Artificial black hole

    # damage control
    if tech_name in ["SHP_REINFORCED_HULL"]:
        return armor
    if tech_name in ["SHP_BASIC_DAM_CONT", "SHP_FLEET_REPAIR", "SHP_ADV_DAM_CONT"]:
        return repair

    # ship parts
    if tech_name in ["SHP_IMPROVED_ENGINE_COUPLINGS", "SHP_N_DIMENSIONAL_ENGINE_MATRIX", "SHP_SINGULARITY_ENGINE_CORE"]:
        return engine
    if tech_name in ["SHP_DEUTERIUM_TANK", "SHP_ANTIMATTER_TANK", "SHP_ZERO_POINT"]:
        return fuel
    if tech_name in ["SHP_NOVA_BOMB", "SHP_DEATH_SPORE", "SHP_BIOTERM"]:
        return USELESS # AI does not use Nova bomb nor bioterror

    # ship shields
    if tech_name in ["SHP_DEFLECTOR_SHIELD", "SHP_PLASMA_SHIELD", "SHP_BLACKSHIELD"]:
        return shield
    if tech_name in ["SHP_MULTISPEC_SHIELD"]:
        return max(shield, stealth)

    # ship armor TODO include these branches into ship-design calculation
    if tech_name in ["SHP_ROOT_ARMOR", "SHP_ZORTRIUM_PLATE"]:
        return armor
    if tech_name in ["SHP_DIAMOND_PLATE", "SHP_XENTRONIUM_PLATE"]:
        return USELESS if asteroid == hull or has_neutron_star else armor # asteroid hull line and neutronium extraction includes better armors

    # weapons TODO include these branches into ship-design calculation
    if tech_name in ["SHP_ROOT_AGGRESSION", "SHP_WEAPON_1_2", "SHP_WEAPON_1_3", "SHP_WEAPON_1_4"]:
        return massdriver if not tech_is_complete("SHP_WEAPON_4_1") else USELESS # don't research obsolete weapons if get deathray from ruins
    if tech_name in ["SHP_WEAPON_2_1", "SHP_WEAPON_2_2", "SHP_WEAPON_2_3", "SHP_WEAPON_2_4"]:
        return laser if not tech_is_complete("SHP_WEAPON_4_1") else USELESS
    if tech_name in ["SHP_WEAPON_3_1", "SHP_WEAPON_3_2", "SHP_WEAPON_3_3", "SHP_WEAPON_3_4"]:
        return plasmacannon if not tech_is_complete("SHP_WEAPON_4_1") else USELESS
    if tech_name in ["SHP_WEAPON_4_1", "SHP_WEAPON_4_2", "SHP_WEAPON_4_3", "SHP_WEAPON_4_4"]:
        return deathray

    # detection
    if tech_name in ["SPY_DETECT_1", "SPY_DETECT_2", "SPY_DETECT_3", "SPY_DETECT_4", "SPY_DETECT_5", "SPY_DIST_MOD", "SPY_LIGHTHOUSE"]:
        return detection

    # stealth
    if tech_name in ["SPY_ROOT_DECEPTION", "SPY_STEALTH_1", "SPY_STEALTH_2", "SPY_STEALTH_3", "SPY_STEALTH_4"]:
        return stealth


    # default priority for unseen techs
    return 1


def calculate_research_requirements(empire):
    """calculate RPs and prerequisties of every tech"""
    result = {}

    # TODO subtract already spent RPs from research projects
    completed_techs = get_completed_techs()
    for tech_name in fo.techs():
        this_tech = fo.getTech(tech_name)
        prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire.empireID) if preReq not in completed_techs]
        cost = this_tech.researchCost(empire.empireID)
        for prereq in prereqs:
            cost += fo.getTech(prereq).researchCost(empire.empireID)
        result[tech_name] = (prereqs, cost)

    return result

def generate_classic_research_orders():
    """generate research orders"""
    report_adjustments = False
    empire = fo.getEmpire()
    empire_id = empire.empireID
    enemies_sighted = foAI.foAIstate.misc.get('enemies_sighted', {})
    galaxy_is_sparse = ColonisationAI.galaxy_is_sparse()
    print "Research Queue Management:"
    resource_production = empire.resourceProduction(fo.resourceType.research)
    print "\nTotal Current Research Points: %.2f\n" % resource_production
    print "Techs researched and available for use:"
    completed_techs = sorted(list(get_completed_techs()))
    tlist = completed_techs+3*[" "]
    tlines = zip(tlist[0::3], tlist[1::3], tlist[2::3])
    for tline in tlines:
        print "%25s %25s %25s" % tline
    print
    
    #
    # report techs currently at head of research queue
    #
    research_queue = empire.researchQueue
    research_queue_list = get_research_queue_techs()
    total_rp = empire.resourceProduction(fo.resourceType.research)
    inProgressTechs.clear()
    tech_turns_left = {}
    if research_queue_list:
        print "Techs currently at head of Research Queue:"
        for element in list(research_queue)[:10]:
            tech_turns_left[element.tech] = element.turnsLeft
            if element.allocation > 0.0:
                inProgressTechs[element.tech] = True
            this_tech = fo.getTech(element.tech)
            if not this_tech:
                print "Error: can't retrieve tech ", element.tech
                continue
            missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq not in completed_techs]
            # unlocked_items = [(uli.name, uli.type) for uli in this_tech.unlocked_items]
            unlocked_items = [uli.name for uli in this_tech.unlockedItems]
            if not missing_prereqs:
                print "    %25s allocated %6.2f RP -- unlockable items: %s " % (element.tech, element.allocation, unlocked_items)
            else:
                print "    %25s allocated %6.2f RP -- missing preReqs: %s -- unlockable items: %s " % (element.tech, element.allocation, missing_prereqs, unlocked_items)
        print
    #
    # set starting techs, or after turn 100 add any additional default techs
    #
    if (fo.currentTurn() == 1) or ((total_rp - research_queue.totalSpent) > 0):
        research_index = get_research_index()
        if fo.currentTurn() == 1:
            # do only this one on first turn, to facilitate use of a turn-1 savegame for testing of alternate
            # research strategies
            new_tech = ["GRO_PLANET_ECOL", "LRN_ALGO_ELEGANCE"]
        else:
            new_tech = TechsListsAI.sparse_galaxy_techs(research_index) if galaxy_is_sparse else TechsListsAI.primary_meta_techs(research_index)
        print "Empire %s (%d) is selecting research index %d" % (empire.name, empire_id, research_index)
        # techs_to_enqueue = (set(new_tech)-(set(completed_techs)|set(research_queue_list)))
        techs_to_enqueue = new_tech[:]
        tech_base = set(completed_techs+research_queue_list)
        techs_to_add = []
        for tech in techs_to_enqueue:
            if tech not in tech_base:
                this_tech = fo.getTech(tech)
                if this_tech is None:
                    print "Error: desired tech '%s' appears to not exist" % tech
                    continue
                missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq not in tech_base]
                techs_to_add.extend(missing_prereqs + [tech])
                tech_base.update(missing_prereqs+[tech])
        cum_cost = 0
        print "  Enqueued Tech: %20s \t\t %8s \t %s" % ("Name", "Cost", "CumulativeCost")
        for name in techs_to_add:
            try:
                enqueue_res = fo.issueEnqueueTechOrder(name, -1)
                if enqueue_res == 1:
                    this_tech = fo.getTech(name)
                    this_cost = 0
                    if this_tech:
                        this_cost = this_tech.researchCost(empire_id)
                        cum_cost += this_cost
                    print "    Enqueued Tech: %20s \t\t %8.0f \t %8.0f" % (name, this_cost, cum_cost)
                else:
                    print "    Error: failed attempt to enqueued Tech: " + name
            except:
                print "    Error: failed attempt to enqueued Tech: " + name
                print "    Error: exception triggered and caught: ", traceback.format_exc()


        print "\n\nAll techs:"
        alltechs = fo.techs()  # returns names of all techs
        for tname in alltechs:
            print tname
        print "\n-------------------------------\nAll unqueued techs:"
        # coveredTechs = new_tech+completed_techs
        for tname in [tn for tn in alltechs if tn not in tech_base]:
            print tname

        if fo.currentTurn() == 1:
            return
        if foAI.foAIstate.aggression <= fo.aggression.cautious:
            research_queue_list = get_research_queue_techs()
            def_techs = TechsListsAI.defense_techs_1()
            for def_tech in def_techs:
                if def_tech not in research_queue_list[:5] and not tech_is_complete(def_tech):
                    res = fo.issueEnqueueTechOrder(def_tech, min(3, len(research_queue_list)))
                    print "Empire is very defensive, so attempted to fast-track %s, got result %d" % (def_tech, res)
        if False and foAI.foAIstate.aggression >= fo.aggression.aggressive:  # with current stats of Conc Camps, disabling this fast-track
            research_queue_list = get_research_queue_techs()
            if "CON_CONC_CAMP" in research_queue_list:
                insert_idx = min(40, research_queue_list.index("CON_CONC_CAMP"))
            else:
                insert_idx = max(0, min(40, len(research_queue_list)-10))
            if "SHP_DEFLECTOR_SHIELD" in research_queue_list:
                insert_idx = min(insert_idx, research_queue_list.index("SHP_DEFLECTOR_SHIELD"))
            for cc_tech in ["CON_ARCH_PSYCH", "CON_CONC_CAMP"]:
                if cc_tech not in research_queue_list[:insert_idx + 1] and not tech_is_complete(cc_tech):
                    res = fo.issueEnqueueTechOrder(cc_tech, insert_idx)
                    msg = "Empire is very aggressive, so attempted to fast-track %s, got result %d" % (cc_tech, res)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        print msg

    elif fo.currentTurn() > 100:
        generate_default_research_order()

    research_queue_list = get_research_queue_techs()
    num_techs_accelerated = 1  # will ensure leading tech doesn't get dislodged
    got_ggg_tech = tech_is_complete("PRO_ORBITAL_GEN")
    got_sym_bio = tech_is_complete("GRO_SYMBIOTIC_BIO")
    got_xeno_gen = tech_is_complete("GRO_XENO_GENETICS")
    #
    # Consider accelerating techs; priority is
    # Supply/Detect range
    # xeno arch
    # ast / GG
    # gro xeno gen
    # distrib thought
    # quant net
    # pro sing gen
    # death ray 1 cleanup

    nest_tech = AIDependencies.NEST_DOMESTICATION_TECH
    artif_minds = AIDependencies.ART_MINDS
    if ColonisationAI.got_nest and not tech_is_complete(nest_tech):
        if artif_minds in research_queue_list:
            insert_idx = 1 + research_queue_list.index(artif_minds)
        else:
            insert_idx = 1
        res = fo.issueEnqueueTechOrder(nest_tech, insert_idx)
        num_techs_accelerated += 1
        msg = "Have a monster nest, so attempted to fast-track %s, got result %d" % (nest_tech, res)
        if report_adjustments:
            chat_human(msg)
        else:
            print msg
        research_queue_list = get_research_queue_techs()

    #
    # Supply range and detection range
    if False:  # disabled for now, otherwise just to help with cold-folding / organization
        if len(foAI.foAIstate.colonisablePlanetIDs) == 0:
            best_colony_site_score = 0
        else:
            best_colony_site_score = foAI.foAIstate.colonisablePlanetIDs.items()[0][1]
        if len(foAI.foAIstate.colonisableOutpostIDs) == 0:
            best_outpost_site_score = 0
        else:
            best_outpost_site_score = foAI.foAIstate.colonisableOutpostIDs.items()[0][1]
        need_improved_scouting = (best_colony_site_score < 150 or best_outpost_site_score < 200)

        if need_improved_scouting:
            if not tech_is_complete("CON_ORBITAL_CON"):
                num_techs_accelerated += 1
                if ("CON_ORBITAL_CON" not in research_queue_list[:1 + num_techs_accelerated]) and (
                        tech_is_complete("PRO_FUSION_GEN") or ("PRO_FUSION_GEN" in research_queue_list[:1 + num_techs_accelerated])):
                    res = fo.issueEnqueueTechOrder("CON_ORBITAL_CON", num_techs_accelerated)
                    msg = "Empire has poor colony/outpost prospects, so attempted to fast-track %s, got result %d" % ("CON_ORBITAL_CON", res)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        print msg
            elif not tech_is_complete("CON_CONTGRAV_ARCH"):
                num_techs_accelerated += 1
                if ("CON_CONTGRAV_ARCH" not in research_queue_list[:1+num_techs_accelerated]) and (
                        tech_is_complete("CON_METRO_INFRA")):
                    for supply_tech in [_s_tech for _s_tech in ["CON_ARCH_MONOFILS", "CON_CONTGRAV_ARCH"] if not tech_is_complete(_s_tech)]:
                        res = fo.issueEnqueueTechOrder(supply_tech, num_techs_accelerated)
                        msg = "Empire has poor colony/outpost prospects, so attempted to fast-track %s, got result %d" % (supply_tech, res)
                        if report_adjustments:
                            chat_human(msg)
                        else:
                            print msg
            elif not tech_is_complete("CON_GAL_INFRA"):
                num_techs_accelerated += 1
                if ("CON_GAL_INFRA" not in research_queue_list[:1+num_techs_accelerated]) and (
                        tech_is_complete("PRO_SINGULAR_GEN")):
                    res = fo.issueEnqueueTechOrder("CON_GAL_INFRA", num_techs_accelerated)
                    msg = "Empire has poor colony/outpost prospects, so attempted to fast-track %s, got result %d" % ("CON_GAL_INFRA", res)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        print msg
            else:
                pass
            research_queue_list = get_research_queue_techs()
            # could add more supply tech

            if False and not tech_is_complete("SPY_DETECT_2"):  # disabled for now, detect2
                num_techs_accelerated += 1
                if "SPY_DETECT_2" not in research_queue_list[:2+num_techs_accelerated] and tech_is_complete("PRO_FUSION_GEN"):
                    if "CON_ORBITAL_CON" not in research_queue_list[:1+num_techs_accelerated]:
                        res = fo.issueEnqueueTechOrder("SPY_DETECT_2", num_techs_accelerated)
                    else:
                        co_idx = research_queue_list.index("CON_ORBITAL_CON")
                        res = fo.issueEnqueueTechOrder("SPY_DETECT_2", co_idx + 1)
                    msg = "Empire has poor colony/outpost prospects, so attempted to fast-track %s, got result %d" % ("CON_ORBITAL_CON", res)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        print msg
                research_queue_list = get_research_queue_techs()

    #
    # check to accelerate xeno_arch
    if True:  # just to help with cold-folding /  organization
        if (ColonisationAI.gotRuins and not tech_is_complete("LRN_XENOARCH") and
                foAI.foAIstate.aggression >= fo.aggression.typical):
            if artif_minds in research_queue_list:
                insert_idx = 7 + research_queue_list.index(artif_minds)
            elif "GRO_SYMBIOTIC_BIO" in research_queue_list:
                insert_idx = research_queue_list.index("GRO_SYMBIOTIC_BIO") + 1
            else:
                insert_idx = num_techs_accelerated
            if "LRN_XENOARCH" not in research_queue_list[:insert_idx]:
                for xenoTech in ["LRN_XENOARCH", "LRN_TRANSLING_THT", "LRN_PHYS_BRAIN", "LRN_ALGO_ELEGANCE"]:
                    if not tech_is_complete(xenoTech) and xenoTech not in research_queue_list[:(insert_idx + 4)]:
                        res = fo.issueEnqueueTechOrder(xenoTech, insert_idx)
                        num_techs_accelerated += 1
                        msg = "ANCIENT_RUINS: have an ancient ruins, so attempted to fast-track %s to enable LRN_XENOARCH, got result %d" % (xenoTech, res)
                        if report_adjustments:
                            chat_human(msg)
                        else:
                            print msg
                research_queue_list = get_research_queue_techs()

    if False and not enemies_sighted:  # curently disabled
        # params = [ (tech, gate, target_slot, add_tech_list), ]
        params = [("GRO_XENO_GENETICS", "PRO_EXOBOTS", "PRO_EXOBOTS", ["GRO_GENETIC_MED", "GRO_XENO_GENETICS"]),
                  ("PRO_EXOBOTS", "PRO_SENTIENT_AUTOMATION", "PRO_SENTIENT_AUTOMATION", ["PRO_EXOBOTS"]),
                  ("PRO_SENTIENT_AUTOMATION", "PRO_NANOTECH_PROD", "PRO_NANOTECH_PROD", ["PRO_SENTIENT_AUTOMATION"]),
                  ("PRO_INDUSTRY_CENTER_I", "GRO_SYMBIOTIC_BIO", "GRO_SYMBIOTIC_BIO", ["PRO_ROBOTIC_PROD", "PRO_FUSION_GEN", "PRO_INDUSTRY_CENTER_I"]),
                  ("GRO_SYMBIOTIC_BIO", "SHP_ORG_HULL", "SHP_ZORTRIUM_PLATE", ["GRO_SYMBIOTIC_BIO"]),
                  ]
        for (tech, gate, target_slot, add_tech_list) in params:
            if tech_is_complete(tech):
                break
            if tech_turns_left.get(gate, 0) not in [0, 1, 2]:  # needs to exclude -1, the flag for no predicted completion
                continue
            if target_slot in research_queue_list:
                target_index = 1 + research_queue_list.index(target_slot)
            else:
                target_index = num_techs_accelerated
            for move_tech in add_tech_list:
                print "for tech %s, target_slot %s, target_index:%s ; num_techs_accelerated:%s" % (move_tech, target_slot, target_index, num_techs_accelerated)
                if tech_is_complete(move_tech):
                    continue
                if target_index <= num_techs_accelerated:
                    num_techs_accelerated += 1
                if move_tech not in research_queue_list[:1 + target_index]:
                    res = fo.issueEnqueueTechOrder(move_tech, target_index)
                    msg = "Research: To prioritize %s, have advanced %s to slot %d" % (tech, move_tech, target_index)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        print msg
                    target_index += 1
    #
    # check to accelerate asteroid or GG tech
    if True:  # just to help with cold-folding / organization
        if ColonisationAI.got_ast:
            insert_idx = num_techs_accelerated if "GRO_SYMBIOTIC_BIO" not in research_queue_list else research_queue_list.index("GRO_SYMBIOTIC_BIO")
            ast_tech = "PRO_MICROGRAV_MAN"
            if not (tech_is_complete(ast_tech) or ast_tech in research_queue_list[:(1 + insert_idx)]):
                res = fo.issueEnqueueTechOrder(ast_tech, insert_idx)
                num_techs_accelerated += 1
                msg = "Asteroids: plan to colonize an asteroid belt, so attempted to fast-track %s , got result %d" % (ast_tech, res)
                if report_adjustments:
                    chat_human(msg)
                else:
                    print msg
                research_queue_list = get_research_queue_techs()
            elif tech_is_complete("SHP_ZORTRIUM_PLATE"):
                insert_idx = (1 + insert_idx) if "LRN_FORCE_FIELD" not in research_queue_list else max(1 + insert_idx, research_queue_list.index("LRN_FORCE_FIELD") - 1)
                for ast_tech in ["SHP_ASTEROID_HULLS", "SHP_IMPROVED_ENGINE_COUPLINGS"]:
                    if not tech_is_complete(ast_tech) and ast_tech not in research_queue_list[:insert_idx + 1]:
                        res = fo.issueEnqueueTechOrder(ast_tech, insert_idx)
                        num_techs_accelerated += 1
                        insert_idx += 1
                        msg = "Asteroids: plan to colonize an asteroid belt, so attempted to fast-track %s , got result %d" % (ast_tech, res)
                        print msg
                        if report_adjustments:
                            chat_human(msg)
                research_queue_list = get_research_queue_techs()
        if ColonisationAI.got_gg and not tech_is_complete("PRO_ORBITAL_GEN"):
            fusion_idx = 0 if "PRO_FUSION_GEN" not in research_queue_list else (1 + research_queue_list.index("PRO_FUSION_GEN"))
            forcefields_idx = 0 if "LRN_FORCE_FIELD" not in research_queue_list else (1 + research_queue_list.index("LRN_FORCE_FIELD"))
            insert_idx = max(fusion_idx, forcefields_idx) if enemies_sighted else fusion_idx
            if "PRO_ORBITAL_GEN" not in research_queue_list[:insert_idx+1]:
                res = fo.issueEnqueueTechOrder("PRO_ORBITAL_GEN", insert_idx)
                num_techs_accelerated += 1
                msg = "GasGiant: plan to colonize a gas giant, so attempted to fast-track %s, got result %d" % ("PRO_ORBITAL_GEN", res)
                print msg
                if report_adjustments:
                    chat_human(msg)
                research_queue_list = get_research_queue_techs()
    #
    # assess if our empire has any non-lousy colonizers, & boost gro_xeno_gen if we don't
    if True:  # just to help with cold-folding / organization
        if got_ggg_tech and got_sym_bio and (not got_xeno_gen) and foAI.foAIstate.aggression >= fo.aggression.cautious:
            most_adequate = 0
            for specName in ColonisationAI.empire_colonizers:
                environs = {}
                this_spec = fo.getSpecies(specName)
                if not this_spec:
                    continue
                for ptype in [fo.planetType.swamp, fo.planetType.radiated, fo.planetType.toxic, fo.planetType.inferno, fo.planetType.barren, fo.planetType.tundra, fo.planetType.desert, fo.planetType.terran, fo.planetType.ocean, fo.planetType.asteroids]:
                    environ = this_spec.getPlanetEnvironment(ptype)
                    environs.setdefault(environ, []).append(ptype)
                most_adequate = max(most_adequate, len(environs.get(fo.planetEnvironment.adequate, [])))
            if most_adequate == 0:
                insert_idx = num_techs_accelerated
                for xg_tech in ["GRO_XENO_GENETICS", "GRO_GENETIC_ENG"]:
                    if xg_tech not in research_queue_list[:1+num_techs_accelerated] and not tech_is_complete(xg_tech):
                        res = fo.issueEnqueueTechOrder(xg_tech, insert_idx)
                        num_techs_accelerated += 1
                        msg = "Empire has poor colonizers, so attempted to fast-track %s, got result %d" % (xg_tech, res)
                        print msg
                        if report_adjustments:
                            chat_human(msg)
                research_queue_list = get_research_queue_techs()
    #
    # check to accelerate distrib thought
    if True:  # just to help with cold-folding / organization
        if not tech_is_complete("LRN_DISTRIB_THOUGHT"):
            got_telepathy = False
            for specName in ColonisationAI.empire_species:
                this_spec = fo.getSpecies(specName)
                if this_spec and ("TELEPATHIC" in list(this_spec.tags)):
                    got_telepathy = True
                    break
            if (foAI.foAIstate.aggression > fo.aggression.cautious) and (empire.population() > ([300, 100][got_telepathy])):
                insert_idx = num_techs_accelerated
                for dt_ech in ["LRN_PHYS_BRAIN", "LRN_TRANSLING_THT", "LRN_PSIONICS", "LRN_DISTRIB_THOUGHT"]:
                    if dt_ech not in research_queue_list[:insert_idx + 2] and not tech_is_complete(dt_ech):
                        res = fo.issueEnqueueTechOrder(dt_ech, insert_idx)
                        num_techs_accelerated += 1
                        insert_idx += 1
                        fmt_str = "Empire has a telepathic race, so attempted to fast-track %s (got result %d)"
                        fmt_str += " with current target_RP %.1f and current pop %.1f, on turn %d"
                        msg = fmt_str % (dt_ech, res, resource_production, empire.population(), fo.currentTurn())
                        print msg
                        if report_adjustments:
                            chat_human(msg)
                research_queue_list = get_research_queue_techs()
    #
    # check to accelerate quant net
    if False:  # disabled for now, otherwise just to help with cold-folding / organization
        if (foAI.foAIstate.aggression > fo.aggression.cautious) and (ColonisationAI.empire_status.get('researchers', 0) >= 40):
            if not tech_is_complete("LRN_QUANT_NET"):
                insert_idx = num_techs_accelerated  # TODO determine min target slot if reenabling
                for qnTech in ["LRN_NDIM_SUBSPACE", "LRN_QUANT_NET"]:
                    if qnTech not in research_queue_list[:insert_idx + 2] and not tech_is_complete(qnTech):
                        res = fo.issueEnqueueTechOrder(qnTech, insert_idx)
                        num_techs_accelerated += 1
                        insert_idx += 1
                        msg = "Empire has many researchers, so attempted to fast-track %s (got result %d) on turn %d" % (qnTech, res, fo.currentTurn())
                        print msg
                        if report_adjustments:
                            chat_human(msg)
                research_queue_list = get_research_queue_techs()

    #
    # if we own a blackhole, accelerate sing_gen and conc camp
    if True:  # just to help with cold-folding / organization
        if (fo.currentTurn() > 50 and len(AIstate.empireStars.get(fo.starType.blackHole, [])) != 0 and
                foAI.foAIstate.aggression > fo.aggression.cautious and not tech_is_complete(AIDependencies.PRO_SINGULAR_GEN) and
                tech_is_complete(AIDependencies.PRO_SOL_ORB_GEN)):
            # sing_tech_list = [ "LRN_GRAVITONICS" , "PRO_SINGULAR_GEN"]  # formerly also "CON_ARCH_PSYCH", "CON_CONC_CAMP",
            sing_gen_tech = fo.getTech(AIDependencies.PRO_SINGULAR_GEN)
            sing_tech_list = [pre_req for pre_req in sing_gen_tech.recursivePrerequisites(empire_id) if not tech_is_complete(pre_req)]
            sing_tech_list += [AIDependencies.PRO_SINGULAR_GEN]
            for singTech in sing_tech_list:
                if singTech not in research_queue_list[:num_techs_accelerated+1]:
                    res = fo.issueEnqueueTechOrder(singTech, num_techs_accelerated)
                    num_techs_accelerated += 1
                    msg = "have a black hole star outpost/colony, so attempted to fast-track %s, got result %d" % (singTech, res)
                    print msg
                    if report_adjustments:
                        chat_human(msg)
            research_queue_list = get_research_queue_techs()

    #
    # if got deathray from Ruins, remove most prereqs from queue
    if True:  # just to help with cold-folding / organization
        if tech_is_complete("SHP_WEAPON_4_1"):
            this_tech = fo.getTech("SHP_WEAPON_4_1")
            if this_tech:
                missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq in research_queue_list]
                if len(missing_prereqs) > 2:  # leave plasma 4 and 3 if up to them already
                    for preReq in missing_prereqs:  # sorted(missing_prereqs, reverse=True)[2:]
                        if preReq in research_queue_list:
                            fo.issueDequeueTechOrder(preReq)
                    research_queue_list = get_research_queue_techs()
                    if "SHP_WEAPON_4_2" in research_queue_list:  # (should be)
                        idx = research_queue_list.index("SHP_WEAPON_4_2")
                        fo.issueEnqueueTechOrder("SHP_WEAPON_4_2", max(0, idx-18))

    # TODO: Remove the following example code
    # Example/Test code for the new ShipDesigner functionality
    techs = ["SHP_WEAPON_4_2", "SHP_TRANSSPACE_DRIVE", "SHP_INTSTEL_LOG", "SHP_ASTEROID_HULLS", ""]
    for tech in techs:
        this_tech = fo.getTech(tech)
        if not this_tech:
            print "Invalid Tech specified"
            continue
        unlocked_items = this_tech.unlockedItems
        unlocked_hulls = []
        unlocked_parts = []
        for item in unlocked_items:
            if item.type == fo.unlockableItemType.shipPart:
                print "Tech %s unlocks a ShipPart: %s" % (tech, item.name)
                unlocked_parts.append(item.name)
            elif item.type == fo.unlockableItemType.shipHull:
                print "Tech %s unlocks a ShipHull: %s" % (tech, item.name)
                unlocked_hulls.append(item.name)
        if not (unlocked_parts or unlocked_hulls):
            print "No new ship parts/hulls unlocked by tech %s" % tech
            continue
        old_designs = ShipDesignAI.MilitaryShipDesigner().optimize_design(consider_fleet_count=False)
        new_designs = ShipDesignAI.MilitaryShipDesigner().optimize_design(additional_hulls=unlocked_hulls, additional_parts=unlocked_parts, consider_fleet_count=False)
        if not (old_designs and new_designs):
            # AI is likely defeated; don't bother with logging error message
            continue
        old_rating, old_pid, old_design_id, old_cost = old_designs[0]
        old_design = fo.getShipDesign(old_design_id)
        new_rating, new_pid, new_design_id, new_cost = new_designs[0]
        new_design = fo.getShipDesign(new_design_id)
        if new_rating > old_rating:
            print "Tech %s gives access to a better design!" % tech
            print "old best design: Rating %.5f" % old_rating
            print "old design specs: %s - " % old_design.hull, list(old_design.parts)
            print "new best design: Rating %.5f" % new_rating
            print "new design specs: %s - " % new_design.hull, list(new_design.parts)
        else:
            print "Tech %s gives access to new parts or hulls but there seems to be no military advantage." % tech

def use_classic_research_approach():
    # TODO: make research approach dependent on AI Config
    return True


def generate_research_orders():
    """generate research orders"""

    if use_classic_research_approach():
        generate_classic_research_orders()
        return

    report_adjustments = False
    empire = fo.getEmpire()
    empire_id = empire.empireID
    enemies_sighted = foAI.foAIstate.misc.get('enemies_sighted', {})
    galaxy_is_sparse = ColonisationAI.galaxy_is_sparse()
    print "Research Queue Management:"
    resource_production = empire.resourceProduction(fo.resourceType.research)
    print "\nTotal Current Research Points: %.2f\n" % resource_production
    print "Techs researched and available for use:"
    completed_techs = sorted(list(get_completed_techs()))
    tlist = completed_techs+3*[" "]
    tlines = zip(tlist[0::3], tlist[1::3], tlist[2::3])
    for tline in tlines:
        print "%25s %25s %25s" % tline
    print

    #
    # report techs currently at head of research queue
    #
    research_queue = empire.researchQueue
    research_queue_list = get_research_queue_techs()
    tech_turns_left = {}
    if research_queue_list:
        print "Techs currently at head of Research Queue:"
        for element in list(research_queue)[:10]:
            tech_turns_left[element.tech] = element.turnsLeft
            this_tech = fo.getTech(element.tech)
            if not this_tech:
                print "Error: can't retrieve tech ", element.tech
                continue
            missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq not in completed_techs]
            # unlocked_items = [(uli.name, uli.type) for uli in this_tech.unlocked_items]
            unlocked_items = [uli.name for uli in this_tech.unlockedItems]
            if not missing_prereqs:
                print "    %25s allocated %6.2f RP -- unlockable items: %s " % (element.tech, element.allocation, unlocked_items)
            else:
                print "    %25s allocated %6.2f RP -- missing preReqs: %s -- unlockable items: %s " % (element.tech, element.allocation, missing_prereqs, unlocked_items)
        print

    #
    # calculate all research priorities, as in get_priority(tech) / total cost of tech (including prereqs)
    #
    research_reqs = calculate_research_requirements(empire)
    priorities = {}
    for tech_name in fo.techs():
        priority = get_priority(tech_name)
        if not tech_is_complete(tech_name) and priority >= 0:
            priorities[tech_name] = float(priority) / research_reqs[tech_name][1]

    #
    # put in highest priority techs until all RP spent
    #
    possible = sorted(priorities.keys(), key=priorities.__getitem__)

    print "Research priorities"
    print "    %25s %8s %8s %s" % ("Name", "Priority", "Cost", "Missing Prerequisties")
    for tech_name in possible[-10:]:
        print "    %25s %8.6f %8.2f %s" % (tech_name, priorities[tech_name], research_reqs[tech_name][1], research_reqs[tech_name][0])
    print

    # TODO: Remove the following example code
    # Example/Test code for the new ShipDesigner functionality
    techs = ["SHP_WEAPON_4_2", "SHP_TRANSSPACE_DRIVE", "SHP_INTSTEL_LOG", "SHP_ASTEROID_HULLS", ""]
    for tech in techs:
        this_tech = fo.getTech(tech)
        if not this_tech:
            print "Invalid Tech specified"
            continue
        unlocked_items = this_tech.unlockedItems
        unlocked_hulls = []
        unlocked_parts = []
        for item in unlocked_items:
            if item.type == fo.unlockableItemType.shipPart:
                print "Tech %s unlocks a ShipPart: %s" % (tech, item.name)
                unlocked_parts.append(item.name)
            elif item.type == fo.unlockableItemType.shipHull:
                print "Tech %s unlocks a ShipHull: %s" % (tech, item.name)
                unlocked_hulls.append(item.name)
        if not (unlocked_parts or unlocked_hulls):
            print "No new ship parts/hulls unlocked by tech %s" % tech
            continue
        old_designs = ShipDesignAI.MilitaryShipDesigner().optimize_design(consider_fleet_count=False)
        new_designs = ShipDesignAI.MilitaryShipDesigner().optimize_design(additional_hulls=unlocked_hulls, additional_parts=unlocked_parts, consider_fleet_count=False)
        if not (old_designs and new_designs):
            # AI is likely defeated; don't bother with logging error message
            continue
        old_rating, old_pid, old_design_id, old_cost = old_designs[0]
        old_design = fo.getShipDesign(old_design_id)
        new_rating, new_pid, new_design_id, new_cost = new_designs[0]
        new_design = fo.getShipDesign(new_design_id)
        if new_rating > old_rating:
            print "Tech %s gives access to a better design!" % tech
            print "old best design: Rating %.5f" % old_rating
            print "old design specs: %s - " % old_design.hull, list(old_design.parts)
            print "new best design: Rating %.5f" % new_rating
            print "new design specs: %s - " % new_design.hull, list(new_design.parts)
        else:
            print "Tech %s gives access to new parts or hulls but there seems to be no military advantage." % tech

    total_rp = empire.resourceProduction(fo.resourceType.research)
    print "enqueuing techs. already spent RP: %s total RP: %s" % (fo.getEmpire().researchQueue.totalSpent, total_rp)

    if fo.currentTurn() == 1:
        fo.issueEnqueueTechOrder("LRN_ALGO_ELEGANCE", -1)
    else:
        # some floating point issues can cause AI to enqueue every tech......
        while empire.resourceProduction(fo.resourceType.research) - empire.researchQueue.totalSpent > 0.001 and possible:
            queued_techs = get_research_queue_techs()

            to_research = possible.pop()  # get tech with highest priority
            prereqs, cost = research_reqs[to_research]
            prereqs += [to_research]

            for prereq in prereqs:
                if prereq not in queued_techs:
                    fo.issueEnqueueTechOrder(prereq, -1)
                    print "    enqueued tech " + prereq + "  : cost: " + str(fo.getTech(prereq).researchCost(empire.empireID)) + "RP"

            fo.updateResearchQueue()
        print



def generate_default_research_order():
    """
    Generate default research orders.
    Add cheapest technology from possible researches
    until current turn point totally spent.
    """

    empire = fo.getEmpire()
    total_rp = empire.resourceProduction(fo.resourceType.research)

    # get all usable researchable techs not already completed or queued for research

    queued_techs = get_research_queue_techs()

    def is_possible(tech_name):
        return all([empire.getTechStatus(tech_name) == fo.techStatus.researchable,
                   not tech_is_complete(tech_name),
                   not exclude_tech(tech_name),
                   tech_name not in queued_techs])

    # (cost, name) for all tech that possible to add to queue, cheapest last
    possible = sorted(
        [(fo.getTech(tech).researchCost(empire.empireID), tech) for tech in fo.techs() if is_possible(tech)],
        reverse=True)

    print "Techs in possible list after enqueues to Research Queue:"
    for _, tech in possible:
        print "    " + tech
    print

    # iterate through techs in order of cost
    fo.updateResearchQueue()
    total_spent = fo.getEmpire().researchQueue.totalSpent
    print "enqueuing techs. already spent RP: %s total RP: %s" % (total_spent, total_rp)

    while total_rp > 0 and possible:
        cost, name = possible.pop()  # get chipest
        total_rp -= cost
        fo.issueEnqueueTechOrder(name, -1)
        print "    enqueued tech " + name + "  : cost: " + str(cost) + "RP"
    print


def get_possible_projects():
    """get possible projects"""
    preliminary_projects = []
    empire = fo.getEmpire()
    for tech_name in fo.techs():
        if empire.getTechStatus(tech_name) == fo.techStatus.researchable:
            preliminary_projects.append(tech_name)
    return set(preliminary_projects)-set(TechsListsAI.unusable_techs())


def get_completed_techs():
    """get completed and available for use techs"""
    return [tech for tech in fo.techs() if tech_is_complete(tech)]


def get_research_queue_techs():
    """ Get list of techs in research queue."""
    return [element.tech for element in fo.getEmpire().researchQueue]

def exclude_tech(tech_name):
    return ((foAI.foAIstate.aggression < AIDependencies.TECH_EXCLUSION_MAP_1.get(tech_name, fo.aggression.invalid)) or
            (foAI.foAIstate.aggression > AIDependencies.TECH_EXCLUSION_MAP_2.get(tech_name, fo.aggression.maniacal)) or
            tech_name in TechsListsAI.unusable_techs())


