import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import AIDependencies
import AIstate
import traceback
import ColonisationAI
import ShipDesignAI
from freeorion_tools import tech_is_complete, chat_human

def get_empire_hash():
    """
	Get hash for empire name, so to create random tech choices in every game, 
	yet deterministic across loading of same game
    """
    ename = fo.getEmpire().name
    hash = 0
    for char in ename:
        hash += ord(char)
    research_index = fo.getEmpire().empireID + hash
    return research_index

def get_priority(tech_name, empire):
    """
	Get tech priority. 1 is default. 0 if not useful (but doesn't hurt to research), 
	< 0 to prevent AI to research it
    """

    index = get_empire_hash()

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
    defensive = 2 if foAI.foAIstate.aggression <= fo.aggression.cautious else (1 if fo.currentTurn() > 10 else 0.5)
    supply = 2 if ColonisationAI.galaxy_is_sparse() else 0.5

    has_red_star = len(AIstate.empireStars.get(fo.starType.red, [])) != 0
    has_blue_star = len(AIstate.empireStars.get(fo.starType.blue, [])) != 0
    has_black_hole = len(AIstate.empireStars.get(fo.starType.blackHole, [])) != 0
    has_neutron_star = len(AIstate.empireStars.get(fo.starType.neutron , [])) != 0

    hull = 1
    offtrack_hull = 0.05
    offtrack_subhull = 0.25
    # select one hull and specialize it, but some AI may want to have more hulls, by random
    org = hull if index & 1 == 0 or (index + 97) % 227 < 21 else offtrack_hull
    robotic = hull if index & 1 == 1 or (index + 107) % 229 < 22 else offtrack_hull
    if ColonisationAI.got_ast:
        extra = (index + 113) % 223 < 11
        asteroid = hull if index & 2 == 0 or extra else offtrack_hull
        if asteroid == hull and not extra:
            org = offtrack_hull
            robotic = offtrack_hull
    else:
        asteroid = offtrack_hull
    if has_blue_star or has_black_hole:
        extra = (index + 103) % 211 < 10
        energy = hull if index & 4 == 0 or extra else offtrack_hull
        if energy == hull and not extra:
            org = offtrack_hull
            robotic = offtrack_hull
            asteroid = offtrack_hull
    else:
        energy = offtrack_hull

    # further specialization for robotic hulls
    contgrav = hull if robotic == hull and index & 8 == 0 else offtrack_subhull
    nanorobo = hull if robotic == hull and index & 8 == 1 else offtrack_subhull
    flux = hull if robotic == hull and (index + 17) % 43 < 22 else offtrack_subhull
    # further specialization for asteroid hulls
    astheavy = hull if asteroid == hull and index % 3 == 0 else offtrack_subhull
    astswarm = hull if asteroid == hull and index % 3 == 1 else offtrack_subhull
    astcamo = hull if asteroid == hull and index % 3 == 2 else offtrack_subhull
    # further specialization for organic hulls
    orgneural = hull if org == hull and index & 8 == 0 else offtrack_subhull
    orgraven = hull if org == hull and index & 8 == 1 else offtrack_subhull
    orgendosym = hull if org == hull and (index + 17) % 43 < 22 else offtrack_subhull
    # further specialization for energy hulls
    energyfrac = hull if energy == hull and index & 8 == 0 else offtrack_subhull
    energymag = hull if energy == hull and index & 8 == 1 else offtrack_subhull
    # repair techs may be skipped if AI decides to go for nanorobotic hull which full-repairs
    repair = 1 if not nanorobo == hull or (index + 37) % 103 < 77 else 0.3

    # AI may skip weapon lines
    weapon = 1
    massdriver = weapon if index % 47 < 36 else 0
    laser = weapon if (index % 83 < 36 if massdriver == weapon else index % 47 < 41) else 0
    plasmacannon = weapon if (index % 83 < 36 if laser == weapon else index % 47 < 41) else 0
    deathray = weapon

    armor = 1

    engine = 1 if ((index + 13) % 31 < 26 if ColonisationAI.galaxy_is_sparse() else (index + 13) % 31 < 5) else 0
    fuel = 1 if ((index + 17) % 37 < 31 if ColonisationAI.galaxy_is_sparse() else (index + 17) % 37 < 3) else 0

    detection = 1
    stealth = 2.2 if (index + 37) % 67 < 9 else 0.2 # TODO stealthy species probably want more stealthy techs

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
        return IMMEDIATE if ColonisationAI.gotRuins else USELESS # get xenoarcheology ASAP when we have ruins, otherwise it is useless
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
            return growth * 3 # Empire only have lousy colonisers, xeno-genetics are really important for them
        else:
            return growth
    if tech_name in ["GRO_GENETIC_MED"]:
        return DEFAULT # TODO boost this to get the genome bank if enemy is using bioterror?
    if tech_name in ["GRO_LIFECYCLE_MAN", "GRO_NANO_CYBERNET"]:
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
    if tech_name in ["PRO_has_neutron_starIUM_EXTRACTION"]:
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
        return DEFAULT
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
        return stealth

    # ship armor
    if tech_name in ["SHP_ROOT_ARMOR", "SHP_ZORTRIUM_PLATE"]:
        return armor
    if tech_name in ["SHP_DIAMOND_PLATE", "SHP_XENTRONIUM_PLATE"]:
        return useless if asteroid or neutron else armor # asteroid hull line and neutronium extraction includes better armors

    # weapons
    if tech_name in ["SHP_ROOT_AGGRESSION", "SHP_WEAPON_1_2", "SHP_WEAPON_1_3", "SHP_WEAPON_1_4"]:
        return massdriver and not tech_is_complete("SHP_WEAPON_4_1") # don't research obsolete weapons if get deathray from ruins
    if tech_name in ["SHP_WEAPON_2_1", "SHP_WEAPON_2_2", "SHP_WEAPON_2_3", "SHP_WEAPON_2_4"]:
        return laser and not tech_is_complete("SHP_WEAPON_4_1")
    if tech_name in ["SHP_WEAPON_3_1", "SHP_WEAPON_3_2", "SHP_WEAPON_3_3", "SHP_WEAPON_3_4"]:
        return plasmacannon and not tech_is_complete("SHP_WEAPON_4_1")
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

def generate_research_orders():
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
        priority = get_priority(tech_name, empire)
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


def get_completed_techs():
    """get completed and available for use techs"""
    return [tech for tech in fo.techs() if tech_is_complete(tech)]

def get_research_queue_techs():
    """ Get list of techs in research queue."""
    return [element.tech for element in fo.getEmpire().researchQueue]
