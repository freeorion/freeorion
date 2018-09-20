import math
import random
from functools import partial
from logging import warn, debug

import freeOrionAIInterface as fo  # pylint: disable=import-error
from common.print_utils import print_in_columns

import AIDependencies as Dep
import AIstate
import ColonisationAI
from aistate_interface import get_aistate
import ShipDesignAI
import TechsListsAI
from freeorion_tools import chat_human, get_ai_tag_grade, tech_is_complete
from turn_state import state

inProgressTechs = {}


class Choices(object):
    # Cannot construct on import, because fo.getEmpire() is None at this time
    def init(self):
        rng = random.Random()
        rng.seed(fo.getEmpire().name + fo.getGalaxySetupData().seed)
        self.engine = rng.random() < 0.7
        self.fuel = rng.random() < 0.7
        self.hull = rng.randrange(4)
        self.extra_organic_hull = rng.random() < 0.05
        self.extra_robotic_hull = rng.random() < 0.05
        self.extra_asteroid_hull = rng.random() < 0.05
        self.extra_energy_hull = rng.random() < 0.05


empire_stars = {}
research_reqs = {}
choices = Choices()
MAIN_SHIP_DESIGNER_LIST = []

# keys are individual full tech names
priority_funcs = {}

REQS_PREREQS_IDX = 0
REQS_COST_IDX = 1
REQS_TIME_IDX = 2
REQS_PER_TURN_COST_IDX = 3

MIL_IDX = 0
TROOP_IDX = 1
COLONY_IDX = 2

# Priorities
ZERO = 0.0
LOW = 0.1
DEFAULT_PRIORITY = 0.5
HIGH = 42


# TODO research AI no longer use this method, rename and move this method elsewhere
def get_research_index():
    return get_aistate().character.get_research_index()


def has_low_aggression():
    return get_aistate().character.prefer_research_low_aggression()


def conditional_priority(func_if_true, func_if_false, cond_func):
    """
    returns a priority dependent on a condition, either a function or an object attribute
    :type func_if_true: ()
    :type func_if_false: ()
    :type cond_func:(str) -> bool
    :rtype float
    """
    def get_conditial_priority(tech_name=""):
        if cond_func():
            return execute(func_if_true, tech_name=tech_name)
        else:
            return execute(func_if_false, tech_name=tech_name)
    return get_conditial_priority


def get_main_ship_designer_list():
    if not MAIN_SHIP_DESIGNER_LIST:
        MAIN_SHIP_DESIGNER_LIST.extend([ShipDesignAI.WarShipDesigner(), ShipDesignAI.StandardTroopShipDesigner(),
                                        ShipDesignAI.StandardColonisationShipDesigner()])
    return MAIN_SHIP_DESIGNER_LIST


def execute(value, tech_name=None):
    """
    If value is callable return result of its call,
    otherwise return value.
    """
    return value(tech_name=tech_name) if callable(value) else value


def ship_usefulness(base_priority_func, designer=None):
    def wrapper(tech_name=""):
        if designer is None:
            designer_list = get_main_ship_designer_list()
        elif isinstance(designer, int):
            designer_list = get_main_ship_designer_list()[:designer+1][-1:]
        else:
            return 0.0
        useful = 0.0
        for this_designer in designer_list:
            useful = max(useful, get_ship_tech_usefulness(tech_name, this_designer))
        return useful * execute(base_priority_func)
    return wrapper


def has_star(star_type):
    if star_type not in empire_stars:
        empire_stars[star_type] = len(AIstate.empireStars.get(star_type, [])) != 0
    return empire_stars[star_type]


def if_enemies(true_val, false_val):
    return conditional_priority(true_val,
                                false_val,
                                cond_func=lambda: get_aistate().misc.get('enemies_sighted', {}))


def if_dict(this_dict, this_key, true_val, false_val):
    return conditional_priority(true_val,
                                false_val,
                                cond_func=lambda: this_dict.get(this_key, False))


def if_tech_target(tech_target, false_val, true_val):
    return conditional_priority(
        true_val,
        false_val,
        cond_func=lambda: tech_is_complete(tech_target))


def has_only_bad_colonizers():
    most_adequate = 0
    for specName in ColonisationAI.empire_colonizers:
        environs = {}
        this_spec = fo.getSpecies(specName)
        if not this_spec:
            continue
        for ptype in [fo.planetType.swamp, fo.planetType.radiated, fo.planetType.toxic, fo.planetType.inferno,
                      fo.planetType.barren, fo.planetType.tundra, fo.planetType.desert, fo.planetType.terran,
                      fo.planetType.ocean, fo.planetType.asteroids]:
            environ = this_spec.getPlanetEnvironment(ptype)
            environs.setdefault(environ, []).append(ptype)
        most_adequate = max(most_adequate, len(environs.get(fo.planetEnvironment.adequate, [])))
    return most_adequate == 0


def get_max_stealth_species():
    stealth_grades = {'BAD': -15, 'GOOD': 15, 'GREAT': 40, 'ULTIMATE': 60}
    stealth = -999
    stealth_species = ""
    for specName in ColonisationAI.empire_colonizers:
        this_spec = fo.getSpecies(specName)
        if not this_spec:
            continue
        this_stealth = stealth_grades.get(get_ai_tag_grade(list(this_spec.tags), "STEALTH"), 0)
        if this_stealth > stealth:
            stealth_species = specName
            stealth = this_stealth
    result = (stealth_species, stealth)
    return result


def get_initial_research_target():
    # TODO: consider cases where may want lesser target
    return Dep.ART_MINDS


def get_ship_tech_usefulness(tech, ship_designer):
    this_tech = fo.getTech(tech)
    if not this_tech:
        debug("Invalid Tech specified")
        return 0
    unlocked_items = this_tech.unlockedItems
    unlocked_hulls = []
    unlocked_parts = []
    for item in unlocked_items:
        if item.type == fo.unlockableItemType.shipPart:
            unlocked_parts.append(item.name)
        elif item.type == fo.unlockableItemType.shipHull:
            unlocked_hulls.append(item.name)
    if not (unlocked_parts or unlocked_hulls):
        return 0
    old_designs = ship_designer.optimize_design(consider_fleet_count=False)
    new_designs = ship_designer.optimize_design(additional_hulls=unlocked_hulls, additional_parts=unlocked_parts,
                                                consider_fleet_count=False)
    if not (old_designs and new_designs):
        # AI is likely defeated; don't bother with logging error message
        return 0
    old_rating, old_pid, old_design_id, old_cost, old_stats = old_designs[0]
    old_rating = old_rating
    new_rating, new_pid, new_design_id, new_cost, new_stats = new_designs[0]
    new_rating = new_rating
    if new_rating > old_rating:
        ratio = (new_rating - old_rating) / (new_rating + old_rating)
        return ratio * 1.5 + 0.5
    else:
        return 0


def get_population_boost_priority(tech_name=""):
    return 2


def get_stealth_priority(tech_name=""):
    max_stealth_species = get_max_stealth_species()
    if max_stealth_species[1] > 0:
        debug("Has a stealthy species %s. Increase stealth tech priority for %s", max_stealth_species[0], tech_name)
        return 1.5
    else:
        return 0.1


def get_xeno_genetics_priority(tech_name=""):
    if not get_aistate().character.may_research_xeno_genetics_variances():
        return get_population_boost_priority()
    if has_only_bad_colonizers():
        # Empire only have lousy colonisers, xeno-genetics are really important for them
        debug("Empire has only lousy colonizers, increase priority to xeno_genetics")
        return get_population_boost_priority() * 3
    else:
        # TODO: assess number of planets with Adequate/Poor planets owned or considered for colonies
        return 0.6 * get_population_boost_priority()


def get_artificial_black_hole_priority(tech_name=""):
    if has_star(fo.starType.blackHole) or not has_star(fo.starType.red):
        debug("Already have black hole, or does not have a red star to turn to black hole. Skipping ART_BLACK_HOLE")
        return 0
    for tech in Dep.SHIP_TECHS_REQUIRING_BLACK_HOLE:
        if tech_is_complete(tech):
            debug("Solar hull is researched, needs a black hole to produce it. Research ART_BLACK_HOLE now!")
            return 999
    return 1


def get_hull_priority(tech_name):
    hull = 1
    offtrack_hull = 0.05

    chosen_hull = choices.hull
    organic = hull if chosen_hull % 2 == 0 or choices.extra_organic_hull else offtrack_hull
    robotic = hull if chosen_hull % 2 == 1 or choices.extra_robotic_hull else offtrack_hull
    if ColonisationAI.got_ast:
        extra = choices.extra_asteroid_hull
        asteroid = hull if chosen_hull == 2 or extra else offtrack_hull
        if asteroid == hull and not extra:
            organic = offtrack_hull
            robotic = offtrack_hull
    else:
        asteroid = 0
    if has_star(fo.starType.blue) or has_star(fo.starType.blackHole):
        extra = choices.extra_energy_hull
        energy = hull if chosen_hull == 3 or extra else offtrack_hull
        if energy == hull and not extra:
            organic = offtrack_hull
            robotic = offtrack_hull
            asteroid = offtrack_hull
    else:
        energy = 0

    useful = max(
        get_ship_tech_usefulness(tech_name, ShipDesignAI.WarShipDesigner()),
        get_ship_tech_usefulness(tech_name, ShipDesignAI.StandardTroopShipDesigner()),
        get_ship_tech_usefulness(tech_name, ShipDesignAI.StandardColonisationShipDesigner()))

    if get_aistate().misc.get('enemies_sighted', {}):
        aggression = 1
    else:
        aggression = 0.1

    if tech_name in Dep.ROBOTIC_HULL_TECHS:
        return robotic * useful * aggression
    elif tech_name in Dep.ORGANIC_HULL_TECHS:
        return organic * useful * aggression
    elif tech_name in Dep.ASTEROID_HULL_TECHS:
        return asteroid * useful * aggression
    elif tech_name in Dep.ENERGY_HULL_TECHS:
        return energy * useful * aggression
    else:
        return useful * aggression

# TODO boost genome bank if enemy is using bioterror
# TODO for supply techs consider starlane density and planet density


def get_priority(tech_name):
    """
    Get tech priority. the default is just above. 0 if not useful (but doesn't hurt to research),
    < 0 to prevent AI to research it
    """
    return execute(priority_funcs[tech_name], tech_name=tech_name)


def calculate_research_requirements():
    """Calculate RPs and prerequisites of every tech, in (prereqs, cost, time)."""
    empire = fo.getEmpire()
    research_reqs.clear()

    completed_techs = get_completed_techs()
    for tech_name in fo.techs():
        if tech_is_complete(tech_name):
            research_reqs[tech_name] = ([], 0, 0, 0)
            continue
        this_tech = fo.getTech(tech_name)
        prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire.empireID) if preReq not in completed_techs]
        base_cost = this_tech.researchCost(empire.empireID)
        progress = empire.researchProgress(tech_name)
        cost = max(0.0, base_cost - progress)
        proportion_remaining = cost / max(base_cost, 1.0)
        this_time = this_tech.researchTime(fo.empireID())
        turns_needed = max(1, math.ceil(proportion_remaining * this_time))  # even if fully paid needs one turn
        per_turn_cost = float(base_cost) / max(1.0, this_time)

        # TODO: the following timing calc treats prereqs as inherently sequential; consider in parallel when able
        for prereq in prereqs:
            prereq_tech = fo.getTech(prereq)
            if not prereq_tech:
                continue
            base_cost = prereq_tech.researchCost(empire.empireID)
            progress = empire.researchProgress(prereq)
            prereq_cost = max(0.0, base_cost - progress)
            proportion_remaining = prereq_cost / max(base_cost, 1.0)
            this_time = prereq_tech.researchTime(fo.empireID())
            turns_needed += max(1, math.ceil(proportion_remaining * this_time))
            cost += prereq_cost
        research_reqs[tech_name] = (prereqs, cost, turns_needed, per_turn_cost)


def tech_cost_sort_key(tech_name):
    return research_reqs.get(tech_name, ([], 0, 0, 0))[REQS_COST_IDX]


def tech_time_sort_key(tech_name):
    return research_reqs.get(tech_name, ([], 0, 0, 0))[REQS_TIME_IDX]


def init():
    """
    Set handlers for all techs that present in game.
    """
    choices.init()
    # prefixes for tech search. Check for prefix will be applied in same order as they defined
    defensive = get_aistate().character.prefer_research_defensive()
    prefixes = [
        (Dep.DEFENSE_TECHS_PREFIX, 2.0 if defensive else if_enemies(1.0, 0.2)),
        (Dep.WEAPON_PREFIX, ship_usefulness(if_enemies(1.0, 0.2), MIL_IDX))
    ]

    tech_handlers = (
        (Dep.PRO_MICROGRAV_MAN, conditional_priority(3.5, LOW, state.have_asteroids)),
        (Dep.PRO_ORBITAL_GEN, conditional_priority(3.0, LOW, state.have_gas_giant)),
        (Dep.PRO_SINGULAR_GEN, conditional_priority(3.0, LOW, partial(has_star, fo.starType.blackHole))),
        (Dep.GRO_XENO_GENETICS, get_xeno_genetics_priority),
        (Dep.LRN_XENOARCH, conditional_priority(LOW, conditional_priority(5.0, LOW, state.have_ruins), has_low_aggression)),
        (Dep.LRN_ART_BLACK_HOLE, get_artificial_black_hole_priority),
        (Dep.GRO_GENOME_BANK, LOW),
        (Dep.CON_CONC_CAMP, ZERO),
        (Dep.NEST_DOMESTICATION_TECH, conditional_priority(ZERO, conditional_priority(3.0, LOW, state.have_nest), has_low_aggression)),
        (Dep.UNRESEARCHABLE_TECHS, -1.0),
        (Dep.UNUSED_TECHS, ZERO),
        (Dep.THEORY_TECHS, ZERO),
        (Dep.PRODUCTION_BOOST_TECHS, conditional_priority(1.5, 0.6, state.population_with_industry_focus())),
        (Dep.RESEARCH_BOOST_TECHS, if_tech_target(get_initial_research_target(), 2.1, 2.5)),
        (Dep.PRODUCTION_AND_RESEARCH_BOOST_TECHS, 2.5),
        (Dep.POPULATION_BOOST_TECHS, get_population_boost_priority),
        (Dep.SUPPLY_BOOST_TECHS, if_tech_target(Dep.SUPPLY_BOOST_TECHS[0], 1.0, 0.5)),
        (Dep.METER_CHANGE_BOOST_TECHS, 1.0),
        (Dep.DETECTION_TECHS, 0.5),
        (Dep.STEALTH_TECHS, get_stealth_priority),
        (Dep.DAMAGE_CONTROL_TECHS, if_enemies(0.5, 0.1)),
        (Dep.HULL_TECHS, get_hull_priority),
        (Dep.ARMOR_TECHS, ship_usefulness(if_enemies(1.0, 0.1,), MIL_IDX)),
        (Dep.ENGINE_TECHS, ship_usefulness(0.6 if choices.engine else 0.1, None)),
        (Dep.FUEL_TECHS, ship_usefulness(1.0 if choices.fuel else 0.1, None)),
        (Dep.SHIELD_TECHS, ship_usefulness(if_enemies(1.0, 0.1), MIL_IDX)),
        (Dep.TROOP_POD_TECHS, ship_usefulness(if_enemies(0.3, 0.1), TROOP_IDX)),
        (Dep.COLONY_POD_TECHS, ship_usefulness(0.5, COLONY_IDX)),
        # first priority techs add last to override previous values
        (Dep.GRO_PLANET_ECOL, HIGH),
        (Dep.LRN_ALGO_ELEGANCE, HIGH)
    )

    for k, v in tech_handlers:
        if isinstance(k, basestring):
            k = (k, )  # wrap single techs to tuple
        for tech in k:
            priority_funcs[tech] = v

    # add all techs priority_funcs
    # if tech already in priority_funcs do nothing
    # if tech starts with prefix add prefix handler
    # otherwise print warning and add DEFAULT_PRIORITY
    for tech in [tech for tech in fo.techs() if not tech_is_complete(tech)]:
        if tech in priority_funcs:
            continue
        for prefix, handler in prefixes:
            if tech.startswith(prefix):
                priority_funcs[tech] = handler
                break
        else:
            debug("Tech %s does not have a priority, falling back to default." % tech)
            priority_funcs[tech] = DEFAULT_PRIORITY


def generate_research_orders():
    """Generate research orders."""

    if use_classic_research_approach():
        debug("Classical research approach is used")
        generate_classic_research_orders()
        return
    else:
        debug('New research approach is used')

    # initializing priority functions here within generate_research_orders() to avoid import race
    if not priority_funcs:
        init()

    empire = fo.getEmpire()
    empire_id = empire.empireID
    completed_techs = get_completed_techs()
    debug("Research Queue Management on turn %d:", fo.currentTurn())
    debug("ColonisationAI survey:")
    debug('  have asteroids: %s', state.have_asteroids)
    debug('  have gas giant: %s', state.have_gas_giant)
    debug('  have ruins: %s', state.have_ruins)

    resource_production = empire.resourceProduction(fo.resourceType.research)
    debug("\nTotal Current Research Points: %.2f\n", resource_production)
    debug("Techs researched and available for use:")
    print_in_columns(sorted(completed_techs))

    #
    # report techs currently at head of research queue
    #
    research_queue = empire.researchQueue
    research_queue_list = get_research_queue_techs()
    tech_turns_left = {}
    if research_queue_list:
        debug("Techs currently at head of Research Queue:")
        for element in list(research_queue)[:10]:
            tech_turns_left[element.tech] = element.turnsLeft
            this_tech = fo.getTech(element.tech)
            if not this_tech:
                warn("Can't retrieve tech ", element.tech)
                continue
            missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq not in completed_techs]
            # unlocked_items = [(uli.name, uli.type) for uli in this_tech.unlocked_items]
            unlocked_items = [uli.name for uli in this_tech.unlockedItems]
            if not missing_prereqs:
                debug("  %25s allocated %6.2f RP -- unlockable items: %s", element.tech, element.allocation, unlocked_items)
            else:
                debug("  %25s allocated %6.2f RP -- missing preReqs: %s -- unlockable items: %s", element.tech, element.allocation, missing_prereqs, unlocked_items)
        debug('')

    #
    # calculate all research priorities, as in get_priority(tech) / total cost of tech (including prereqs)
    #
    calculate_research_requirements()
    total_rp = empire.resourceProduction(fo.resourceType.research)

    if total_rp <= 0:  # No RP available - no research.
        return

    base_priorities = {}
    priorities = {}
    on_path_to = {}
    for tech_name in fo.techs():
        this_tech = fo.getTech(tech_name)
        if not this_tech or tech_is_complete(tech_name):
            continue
        base_priorities[tech_name] = priorities[tech_name] = get_priority(tech_name)

    # inherited priorities are modestly attenuated by total time
    timescale_period = 30.0
    for tech_name, priority in base_priorities.iteritems():
        if priority >= 0:
            turns_needed = max(research_reqs[tech_name][REQS_TIME_IDX], math.ceil(float(research_reqs[tech_name][REQS_COST_IDX]) / total_rp))
            time_attenuation = 2**(-max(0.0, turns_needed - 5) / timescale_period)
            attenuated_priority = priority * time_attenuation
            for prereq in research_reqs.get(tech_name, ([], 0, 0, 0))[REQS_PREREQS_IDX]:
                if prereq in priorities and attenuated_priority > priorities[prereq]:  # checking like this to keep finished techs out of priorities
                    priorities[prereq] = attenuated_priority
                    on_path_to[prereq] = tech_name

    # final priorities are scaled by a combination of relative per-turn cost and relative total cost
    for tech_name, priority in priorities.iteritems():
        if priority >= 0:
            relative_turn_cost = max(research_reqs[tech_name][REQS_PER_TURN_COST_IDX], 0.1) / total_rp
            relative_total_cost = max(research_reqs[tech_name][REQS_COST_IDX], 0.1) / total_rp
            cost_factor = 2.0 / (relative_turn_cost + relative_total_cost)
            adjusted_priority = float(priority) * cost_factor
            # if priority > 1:
            #    print "tech %s has raw priority %.1f and adjusted priority %.1f, with %.1f total remaining cost, %.1f min turns needed and %.1f projected turns needed" % (tech_name, priority, adjusted_priority, research_reqs[tech_name][REQS_COST_IDX], research_reqs[tech_name][REQS_TIME_IDX], turns_needed)
            priorities[tech_name] = adjusted_priority

    #
    # put in highest priority techs until all RP spent, with  time then cost as minor sorting keys
    #
    possible = sorted(priorities.keys(), key=tech_cost_sort_key)
    possible.sort(key=tech_time_sort_key)
    possible.sort(key=priorities.__getitem__, reverse=True)

    missing_prereq_list = []
    debug("Research priorities")
    debug("  %-25s %8s %8s %8s %-25s %s", "Name", "Priority", "Cost", "Time", "As Prereq To", "Missing Prerequisties")
    for idx, tech_name in enumerate(possible[:20]):
        tech_info = research_reqs[tech_name]
        debug("  %-25s %8.4f %8.2f %8.2f %-25s %s", tech_name, priorities[tech_name], tech_info[1], tech_info[2], on_path_to.get(tech_name, ""), tech_info[0])
        missing_prereq_list.extend([prereq for prereq in tech_info[0] if prereq not in possible[:idx] and not tech_is_complete(prereq)])
    debug('')

    if missing_prereq_list:
        debug('Prerequirements seeming out of order:')
        debug("  %-25s %8s %8s %8s %8s %-25s %s", "Name", "Priority", "Base Prio", "Cost", "Time", "As Prereq To", "Missing Prerequisties")
        for tech_name in missing_prereq_list:
            tech_info = research_reqs[tech_name]
            debug("  %-25s %8.4f %8.4f %8.2f %8.2f %-25s %s", tech_name, priorities[tech_name], base_priorities[tech_name], tech_info[1], tech_info[2], on_path_to.get(tech_name, ""), tech_info[0])

    debug("Enqueuing techs, already spent %.2f RP of %.2f RP", fo.getEmpire().researchQueue.totalSpent, total_rp)
    possible = [x for x in possible if x not in set(get_research_queue_techs())]
    # some floating point issues can cause AI to enqueue every tech......
    while empire.resourceProduction(fo.resourceType.research) - empire.researchQueue.totalSpent > 0.001 and possible:
        to_research = possible.pop(0)  # get tech with highest priority
        fo.issueEnqueueTechOrder(to_research, -1)
        fo.updateResearchQueue()
        debug("  %-25s %6.2f RP/turn %6.2f RP", to_research, fo.getTech(to_research).perTurnCost(empire.empireID),
              fo.getTech(to_research).researchCost(empire.empireID))

    debug("Finish research orders, spent %.2f RP of %.2f RP\n", fo.getEmpire().researchQueue.totalSpent,
          empire.resourceProduction(fo.resourceType.research))


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

    debug("Techs in possible list after enqueues to Research Queue:")
    for _, tech in possible:
        debug("    " + tech)
    debug('')

    # iterate through techs in order of cost
    fo.updateResearchQueue()
    total_spent = fo.getEmpire().researchQueue.totalSpent
    debug("Enqueuing techs. already spent RP: %s total RP: %s", total_spent, total_rp)

    while total_rp > 0 and possible:
        cost, name = possible.pop()  # get chipest
        total_rp -= cost
        fo.issueEnqueueTechOrder(name, -1)
        debug("    enqueued tech %s  : cost: %s RP", name, cost)
    debug('')


def get_possible_projects():
    """get possible projects"""
    preliminary_projects = []
    empire = fo.getEmpire()
    for tech_name in fo.techs():
        if empire.getTechStatus(tech_name) == fo.techStatus.researchable:
            preliminary_projects.append(tech_name)
    return set(preliminary_projects) - set(TechsListsAI.unusable_techs())


def get_completed_techs():
    """Get completed and available for use techs."""
    return [tech for tech in fo.techs() if tech_is_complete(tech)]


def get_research_queue_techs():
    """Get list of techs in research queue."""
    return [element.tech for element in fo.getEmpire().researchQueue]


def exclude_tech(tech_name):
    return ((not get_aistate().character.may_research_tech(tech_name))
            or tech_name in TechsListsAI.unusable_techs()
            or tech_name in Dep.UNRESEARCHABLE_TECHS)


def generate_classic_research_orders():
    """generate research orders"""
    report_adjustments = False
    empire = fo.getEmpire()
    empire_id = empire.empireID
    aistate = get_aistate()
    enemies_sighted = aistate.misc.get('enemies_sighted', {})
    galaxy_is_sparse = ColonisationAI.galaxy_is_sparse()
    debug("Research Queue Management:")
    resource_production = empire.resourceProduction(fo.resourceType.research)
    debug("\nTotal Current Research Points: %.2f\n", resource_production)
    debug("Techs researched and available for use:")
    completed_techs = sorted(list(get_completed_techs()))
    tlist = completed_techs + [" "] * 3
    tlines = zip(tlist[0::3], tlist[1::3], tlist[2::3])
    for tline in tlines:
        debug("%25s %25s %25s", *tline)
    debug('')

    #
    # report techs currently at head of research queue
    #
    research_queue = empire.researchQueue
    research_queue_list = get_research_queue_techs()
    total_rp = empire.resourceProduction(fo.resourceType.research)
    inProgressTechs.clear()
    tech_turns_left = {}
    if research_queue_list:
        debug("Techs currently at head of Research Queue:")
        for element in list(research_queue)[:10]:
            tech_turns_left[element.tech] = element.turnsLeft
            if element.allocation > 0.0:
                inProgressTechs[element.tech] = True
            this_tech = fo.getTech(element.tech)
            if not this_tech:
                warn("Can't retrieve tech ", element.tech)
                continue
            missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq not in completed_techs]
            # unlocked_items = [(uli.name, uli.type) for uli in this_tech.unlocked_items]
            unlocked_items = [uli.name for uli in this_tech.unlockedItems]
            if not missing_prereqs:
                debug("    %25s allocated %6.2f RP -- unlockable items: %s ", element.tech, element.allocation, unlocked_items)
            else:
                debug("    %25s allocated %6.2f RP -- missing preReqs: %s -- unlockable items: %s ", element.tech, element.allocation, missing_prereqs, unlocked_items)
        debug('')
    #
    # set starting techs, or after turn 100 add any additional default techs
    #
    if (fo.currentTurn() <= 2) or ((total_rp - research_queue.totalSpent) > 0):
        research_index = get_research_index()
        if fo.currentTurn() == 1:
            # do only this one on first turn, to facilitate use of a turn-1 savegame for testing of alternate
            # research strategies
            new_tech = ["LRN_PHYS_BRAIN", "LRN_ALGO_ELEGANCE"]
        else:
            new_tech = TechsListsAI.sparse_galaxy_techs(research_index) if galaxy_is_sparse else TechsListsAI.primary_meta_techs(research_index)
        debug("Empire %s (%d) is selecting research index %d", empire.name, empire_id, research_index)
        # techs_to_enqueue = (set(new_tech)-(set(completed_techs)|set(research_queue_list)))
        techs_to_enqueue = new_tech[:]
        tech_base = set(completed_techs + research_queue_list)
        techs_to_add = []
        for tech in techs_to_enqueue:
            if tech not in tech_base:
                this_tech = fo.getTech(tech)
                if this_tech is None:
                    warn("Desired tech '%s' appears to not exist" % tech)
                    continue
                missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq not in tech_base]
                techs_to_add.extend(missing_prereqs + [tech])
                tech_base.update(missing_prereqs + [tech])
        cum_cost = 0
        debug("  Enqueued Tech: %20s \t\t %8s \t %s", "Name", "Cost", "CumulativeCost")
        for name in techs_to_add:
            try:
                enqueue_res = fo.issueEnqueueTechOrder(name, -1)
                if enqueue_res == 1:
                    this_tech = fo.getTech(name)
                    this_cost = 0
                    if this_tech:
                        this_cost = this_tech.researchCost(empire_id)
                        cum_cost += this_cost
                    debug("    Enqueued Tech: %20s \t\t %8.0f \t %8.0f", name, this_cost, cum_cost)
                else:
                    warn("    Failed attempt to enqueued Tech: " + name)
            except:
                warn("    Failed attempt to enqueued Tech: " + name, exc_info=True)

        debug('\n\nAll techs:')
        debug('=' * 20)
        alltechs = fo.techs()
        print_in_columns(sorted(fo.techs()), columns=3)

        debug('\n\nAll unqueued techs:')
        debug('=' * 20)
        # coveredTechs = new_tech+completed_techs
        print_in_columns([tn for tn in alltechs if tn not in tech_base], columns=3)
        debug('')

        if fo.currentTurn() == 1:
            return
        if True:
            research_queue_list = get_research_queue_techs()
            def_techs = TechsListsAI.defense_techs_1()
            for def_tech in def_techs:
                if (aistate.character.may_research_tech_classic(def_tech)
                        and def_tech not in research_queue_list[:5] and not tech_is_complete(def_tech)):
                    res = fo.issueEnqueueTechOrder(def_tech, min(3, len(research_queue_list)))
                    debug("Empire is very defensive, so attempted to fast-track %s, got result %d", def_tech, res)
        if False:  # with current stats of Conc Camps, disabling this fast-track
            research_queue_list = get_research_queue_techs()
            if "CON_CONC_CAMP" in research_queue_list and aistate.character.may_research_tech_classic("CON_CONC_CAMP"):
                insert_idx = min(40, research_queue_list.index("CON_CONC_CAMP"))
            else:
                insert_idx = max(0, min(40, len(research_queue_list) - 10))
            if "SHP_DEFLECTOR_SHIELD" in research_queue_list and aistate.character.may_research_tech_classic("SHP_DEFLECTOR_SHIELD"):
                insert_idx = min(insert_idx, research_queue_list.index("SHP_DEFLECTOR_SHIELD"))
            for cc_tech in ["CON_ARCH_PSYCH", "CON_CONC_CAMP"]:
                if (cc_tech not in research_queue_list[:insert_idx + 1] and not tech_is_complete(cc_tech)
                        and aistate.character.may_research_tech_classic(cc_tech)):
                    res = fo.issueEnqueueTechOrder(cc_tech, insert_idx)
                    msg = "Empire is very aggressive, so attempted to fast-track %s, got result %d" % (cc_tech, res)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        debug(msg)

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

    nest_tech = Dep.NEST_DOMESTICATION_TECH
    artif_minds = Dep.ART_MINDS

    if state.have_nest and not tech_is_complete(nest_tech):
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
            debug(msg)
        research_queue_list = get_research_queue_techs()

    #
    # Supply range and detection range
    if False:  # disabled for now, otherwise just to help with cold-folding / organization
        if len(aistate.colonisablePlanetIDs) == 0:
            best_colony_site_score = 0
        else:
            best_colony_site_score = aistate.colonisablePlanetIDs.items()[0][1]
        if len(aistate.colonisableOutpostIDs) == 0:
            best_outpost_site_score = 0
        else:
            best_outpost_site_score = aistate.colonisableOutpostIDs.items()[0][1]
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
                        debug(msg)
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
                            debug(msg)
            elif not tech_is_complete("CON_GAL_INFRA"):
                num_techs_accelerated += 1
                if ("CON_GAL_INFRA" not in research_queue_list[:1+num_techs_accelerated]) and (
                        tech_is_complete("PRO_SINGULAR_GEN")):
                    res = fo.issueEnqueueTechOrder("CON_GAL_INFRA", num_techs_accelerated)
                    msg = "Empire has poor colony/outpost prospects, so attempted to fast-track %s, got result %d" % ("CON_GAL_INFRA", res)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        debug(msg)
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
                        debug(msg)
                research_queue_list = get_research_queue_techs()

    #
    # check to accelerate xeno_arch
    if True:  # just to help with cold-folding /  organization
        if (state.have_ruins and not tech_is_complete("LRN_XENOARCH")
                and aistate.character.may_research_tech_classic("LRN_XENOARCH")):
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
                            debug(msg)
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
                debug("for tech %s, target_slot %s, target_index:%s ; num_techs_accelerated:%s", move_tech, target_slot, target_index, num_techs_accelerated)
                if tech_is_complete(move_tech):
                    continue
                if target_index <= num_techs_accelerated:
                    num_techs_accelerated += 1
                if move_tech not in research_queue_list[:1 + target_index]:
                    fo.issueEnqueueTechOrder(move_tech, target_index)
                    msg = "Research: To prioritize %s, have advanced %s to slot %d" % (tech, move_tech, target_index)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        debug(msg)
                    target_index += 1
    #
    # check to accelerate asteroid or GG tech
    if True:  # just to help with cold-folding / organization
        if state.have_asteroids:
            insert_idx = num_techs_accelerated if "GRO_SYMBIOTIC_BIO" not in research_queue_list else research_queue_list.index("GRO_SYMBIOTIC_BIO")
            ast_tech = "PRO_MICROGRAV_MAN"
            if not (tech_is_complete(ast_tech) or ast_tech in research_queue_list[:(1 + insert_idx)]):
                res = fo.issueEnqueueTechOrder(ast_tech, insert_idx)
                num_techs_accelerated += 1
                msg = "Asteroids: plan to colonize an asteroid belt, so attempted to fast-track %s , got result %d" % (ast_tech, res)
                if report_adjustments:
                    chat_human(msg)
                else:
                    debug(msg)
                research_queue_list = get_research_queue_techs()
            elif tech_is_complete("SHP_ZORTRIUM_PLATE"):
                insert_idx = (1 + insert_idx) if "LRN_FORCE_FIELD" not in research_queue_list else max(1 + insert_idx, research_queue_list.index("LRN_FORCE_FIELD") - 1)
                for ast_tech in ["SHP_ASTEROID_HULLS", "SHP_IMPROVED_ENGINE_COUPLINGS"]:
                    if not tech_is_complete(ast_tech) and ast_tech not in research_queue_list[:insert_idx + 1]:
                        res = fo.issueEnqueueTechOrder(ast_tech, insert_idx)
                        num_techs_accelerated += 1
                        insert_idx += 1
                        msg = "Asteroids: plan to colonize an asteroid belt, so attempted to fast-track %s , got result %d" % (ast_tech, res)
                        debug(msg)
                        if report_adjustments:
                            chat_human(msg)
                research_queue_list = get_research_queue_techs()
        if state.have_gas_giant and not tech_is_complete("PRO_ORBITAL_GEN"):
            fusion_idx = 0 if "PRO_FUSION_GEN" not in research_queue_list else (1 + research_queue_list.index("PRO_FUSION_GEN"))
            forcefields_idx = 0 if "LRN_FORCE_FIELD" not in research_queue_list else (1 + research_queue_list.index("LRN_FORCE_FIELD"))
            insert_idx = max(fusion_idx, forcefields_idx) if enemies_sighted else fusion_idx
            if "PRO_ORBITAL_GEN" not in research_queue_list[:insert_idx+1]:
                res = fo.issueEnqueueTechOrder("PRO_ORBITAL_GEN", insert_idx)
                num_techs_accelerated += 1
                msg = "GasGiant: plan to colonize a gas giant, so attempted to fast-track %s, got result %d" % ("PRO_ORBITAL_GEN", res)
                debug(msg)
                if report_adjustments:
                    chat_human(msg)
                research_queue_list = get_research_queue_techs()
    #
    # assess if our empire has any non-lousy colonizers, & boost gro_xeno_gen if we don't
    if True:  # just to help with cold-folding / organization
        if got_ggg_tech and got_sym_bio and (not got_xeno_gen):
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
                    if (xg_tech not in research_queue_list[:1 + num_techs_accelerated] and not tech_is_complete(xg_tech)
                            and aistate.character.may_research_tech_classic(xg_tech)):
                        res = fo.issueEnqueueTechOrder(xg_tech, insert_idx)
                        num_techs_accelerated += 1
                        msg = "Empire has poor colonizers, so attempted to fast-track %s, got result %d" % (xg_tech, res)
                        debug(msg)
                        if report_adjustments:
                            chat_human(msg)
                research_queue_list = get_research_queue_techs()
    #
    # check to accelerate distrib thought
    if True:  # just to help with cold-folding / organization
        if not tech_is_complete("LRN_DISTRIB_THOUGHT"):
            got_telepathy = False
            for specName in state.get_empire_planets_by_species():
                this_spec = fo.getSpecies(specName)
                if this_spec and ("TELEPATHIC" in list(this_spec.tags)):
                    got_telepathy = True
                    break
            pop_threshold = 100 if got_telepathy else 300
            if empire.population() > pop_threshold:
                insert_idx = num_techs_accelerated
                for dt_ech in ["LRN_PHYS_BRAIN", "LRN_TRANSLING_THT", "LRN_PSIONICS", "LRN_DISTRIB_THOUGHT"]:
                    if (dt_ech not in research_queue_list[:insert_idx + 2] and not tech_is_complete(dt_ech)
                            and aistate.character.may_research_tech_classic(dt_ech)):
                        res = fo.issueEnqueueTechOrder(dt_ech, insert_idx)
                        num_techs_accelerated += 1
                        insert_idx += 1
                        fmt_str = "Empire has a telepathic race, so attempted to fast-track %s (got result %d)"
                        fmt_str += " with current target_RP %.1f and current pop %.1f, on turn %d"
                        msg = fmt_str % (dt_ech, res, resource_production, empire.population(), fo.currentTurn())
                        debug(msg)
                        if report_adjustments:
                            chat_human(msg)
                research_queue_list = get_research_queue_techs()
    #
    # check to accelerate quant net
    if False:  # disabled for now, otherwise just to help with cold-folding / organization
        if aistate.character.may_research_tech_classic("LRN_QUANT_NET") and (state.population_with_research_focus() >= 40):
            if not tech_is_complete("LRN_QUANT_NET"):
                insert_idx = num_techs_accelerated  # TODO determine min target slot if reenabling
                for qnTech in ["LRN_NDIM_SUBSPACE", "LRN_QUANT_NET"]:
                    if qnTech not in research_queue_list[:insert_idx + 2] and not tech_is_complete(qnTech):
                        res = fo.issueEnqueueTechOrder(qnTech, insert_idx)
                        num_techs_accelerated += 1
                        insert_idx += 1
                        msg = "Empire has many researchers, so attempted to fast-track %s (got result %d) on turn %d" % (qnTech, res, fo.currentTurn())
                        debug(msg)
                        if report_adjustments:
                            chat_human(msg)
                research_queue_list = get_research_queue_techs()

    #
    # if we own a blackhole, accelerate sing_gen and conc camp
    if True:  # just to help with cold-folding / organization
        if (fo.currentTurn() > 50 and len(AIstate.empireStars.get(fo.starType.blackHole, [])) != 0 and
                aistate.character.may_research_tech_classic("PRO_SINGULAR_GEN") and not tech_is_complete(Dep.PRO_SINGULAR_GEN) and
                tech_is_complete(Dep.PRO_SOL_ORB_GEN)):
            # sing_tech_list = [ "LRN_GRAVITONICS" , "PRO_SINGULAR_GEN"]  # formerly also "CON_ARCH_PSYCH", "CON_CONC_CAMP",
            sing_gen_tech = fo.getTech(Dep.PRO_SINGULAR_GEN)
            sing_tech_list = [pre_req for pre_req in sing_gen_tech.recursivePrerequisites(empire_id) if not tech_is_complete(pre_req)]
            sing_tech_list += [Dep.PRO_SINGULAR_GEN]
            for singTech in sing_tech_list:
                if singTech not in research_queue_list[:num_techs_accelerated + 1]:
                    res = fo.issueEnqueueTechOrder(singTech, num_techs_accelerated)
                    num_techs_accelerated += 1
                    msg = "have a black hole star outpost/colony, so attempted to fast-track %s, got result %d" % (singTech, res)
                    debug(msg)
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
                        fo.issueEnqueueTechOrder("SHP_WEAPON_4_2", max(0, idx - 18))

    # TODO: Remove the following example code
    # Example/Test code for the new ShipDesigner functionality
    techs = ["SHP_WEAPON_4_2", "SHP_TRANSSPACE_DRIVE", "SHP_INTSTEL_LOG", "SHP_ASTEROID_HULLS", ""]
    for tech in techs:
        this_tech = fo.getTech(tech)
        if not this_tech:
            debug("Invalid Tech specified")
            continue
        unlocked_items = this_tech.unlockedItems
        unlocked_hulls = []
        unlocked_parts = []
        for item in unlocked_items:
            if item.type == fo.unlockableItemType.shipPart:
                debug("Tech %s unlocks a ShipPart: %s", tech, item.name)
                unlocked_parts.append(item.name)
            elif item.type == fo.unlockableItemType.shipHull:
                debug("Tech %s unlocks a ShipHull: %s", tech, item.name)
                unlocked_hulls.append(item.name)
        if not (unlocked_parts or unlocked_hulls):
            debug("No new ship parts/hulls unlocked by tech %s", tech)
            continue
        old_designs = ShipDesignAI.WarShipDesigner().optimize_design(consider_fleet_count=False)
        new_designs = ShipDesignAI.WarShipDesigner().optimize_design(additional_hulls=unlocked_hulls,
                                                                     additional_parts=unlocked_parts,
                                                                     consider_fleet_count=False)
        if not (old_designs and new_designs):
            # AI is likely defeated; don't bother with logging error message
            continue
        old_rating, old_pid, old_design_id, old_cost, old_stats = old_designs[0]
        old_design = fo.getShipDesign(old_design_id)
        new_rating, new_pid, new_design_id, new_cost, new_stats = new_designs[0]
        new_design = fo.getShipDesign(new_design_id)
        if new_rating > old_rating:
            debug("Tech %s gives access to a better design!", tech)
            debug("old best design: Rating %.5f", old_rating)
            debug("old design specs: %s - %s", old_design.hull, list(old_design.parts))
            debug("new best design: Rating %.5f", new_rating)
            debug("new design specs: %s - %s", new_design.hull, list(new_design.parts))
        else:
            debug("Tech %s gives access to new parts or hulls but there seems to be no military advantage.", tech)


def use_classic_research_approach():
    """
    Returns flag if old research approach should be used.

    By default return True

    If need to run game in new research approach:

    1) create config file:

    [main]
    new_research=True

    2) run game with flag  --ai-config=<path to config file>
    """
    from common.option_tools import get_option_dict
    return not get_option_dict().get('new_research', False)
