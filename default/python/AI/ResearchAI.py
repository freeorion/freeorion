import freeOrionAIInterface as fo
import random
from itertools import islice
from logging import debug, error, info, warning

import AIDependencies as Dep
import AIstate
import ColonisationAI
import ShipDesignAI
import TechsListsAI
from aistate_interface import get_aistate
from common.print_utils import as_columns
from empire.colony_builders import get_colony_builders
from freeorion_tools import tech_is_complete
from ProductionAI import translators_wanted
from turn_state import (
    get_empire_planets_by_species,
    have_asteroids,
    have_gas_giant,
    have_nest,
    have_ruins,
)


class Choices:
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


choices = Choices()

# Priorities
ZERO = 0.0
LOW = 0.1
HIGH = 42


# TODO research AI no longer use this method, rename and move this method elsewhere
def get_research_index():
    return get_aistate().character.get_research_index()


def generate_research_orders():
    """Generate research orders."""
    generate_classic_research_orders()


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
        return all(
            [
                empire.getTechStatus(tech_name) == fo.techStatus.researchable,
                not tech_is_complete(tech_name),
                not exclude_tech(tech_name),
                tech_name not in queued_techs,
            ]
        )

    # (cost, name) for all tech that possible to add to queue, cheapest last
    possible = sorted(
        [(fo.getTech(tech).researchCost(empire.empireID), tech) for tech in fo.techs() if is_possible(tech)],
        reverse=True,
    )

    debug("Techs in possible list after enqueues to Research Queue:")
    for _, tech in possible:
        debug("    " + tech)
    debug("")

    # iterate through techs in order of cost
    fo.updateResearchQueue()
    total_spent = fo.getEmpire().researchQueue.totalSpent
    debug("Enqueuing techs. already spent RP: %s total RP: %s", total_spent, total_rp)

    while total_rp > 0 and possible:
        cost, name = possible.pop()  # get chipest
        total_rp -= cost
        fo.issueEnqueueTechOrder(name, -1)
        debug("    enqueued tech %s  : cost: %s RP", name, cost)
    debug("")


def get_completed_techs() -> list[str]:
    """Get completed and available for use techs."""
    return [tech for tech in fo.techs() if tech_is_complete(tech)]


def get_research_queue_techs():
    """Get list of techs in research queue."""
    return [element.tech for element in fo.getEmpire().researchQueue]


def exclude_tech(tech_name):
    return (
        (not get_aistate().character.may_research_tech(tech_name))
        or tech_name in TechsListsAI.unusable_techs()
        or tech_name in Dep.UNRESEARCHABLE_TECHS
    )


def research_now(tech_name: str, with_prerequisites: bool = True) -> None:
    todo = [tech_name]
    empire = fo.getEmpire()
    if with_prerequisites:
        tech = fo.getTech(tech_name)
        todo += [t for t in tech.recursivePrerequisites(empire.empireID)]
    for name in todo:
        if not empire.researchQueue.inQueue(name):
            fo.issueEnqueueTechOrder(name, 0)
        else:
            for element in empire.researchQueue:
                if element.tech == name:
                    # research points are not lost when a tech is dequeued
                    fo.issueDequeueTechOrder(name)
                    fo.issueEnqueueTechOrder(name, 0)
                    break


def generate_classic_research_orders():  # noqa: C901
    """generate research orders"""
    empire = fo.getEmpire()
    empire_id = empire.empireID
    aistate = get_aistate()
    enemies_sighted = aistate.misc.get("enemies_sighted", {})
    galaxy_is_sparse = ColonisationAI.galaxy_is_sparse()

    resource_production = empire.resourceProduction(fo.resourceType.research)
    completed_techs = sorted(list(get_completed_techs()))
    _print_research_order_header(resource_production, completed_techs)
    _print_research_queue_head(completed_techs)

    research_queue = empire.researchQueue
    research_queue_list = get_research_queue_techs()
    total_rp = empire.resourceProduction(fo.resourceType.research)
    #
    # set starting techs, or after turn 100 add any additional default techs
    #
    if (fo.currentTurn() <= 2) or ((total_rp - research_queue.totalSpent) > 0):
        research_index = get_research_index()
        if fo.currentTurn() == 1:
            # do only this one on first turn, to facilitate use of a turn-1 savegame for testing of alternate
            # research strategies
            new_tech = ["LRN_PHYS_BRAIN", "GRO_PLANET_ECOL"]
        else:
            new_tech = (
                TechsListsAI.sparse_galaxy_techs(research_index)
                if galaxy_is_sparse
                else TechsListsAI.primary_meta_techs(research_index)
            )
        debug("Empire %s (%d) is selecting research index %d", empire.name, empire_id, research_index)
        # techs_to_enqueue = (set(new_tech)-(set(completed_techs)|set(research_queue_list)))
        techs_to_enqueue = new_tech[:]
        tech_base = set(completed_techs + research_queue_list)
        techs_to_add = []
        for tech in techs_to_enqueue:
            if tech not in tech_base:
                this_tech = fo.getTech(tech)
                if this_tech is None:
                    error("Desired tech '%s' appears to not exist" % tech)
                    continue
                missing_prereqs = [
                    preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq not in tech_base
                ]
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
                    warning("    Failed attempt to enqueued Tech: " + name)
            except:  # noqa: E722
                warning("    Failed attempt to enqueued Tech: " + name, exc_info=True)

        debug("\n\nAll techs:")
        debug("=" * 20)
        alltechs = fo.techs()
        info(as_columns(sorted(fo.techs()), columns=3))

        debug("\n\nAll unqueued techs:")
        debug("=" * 20)
        # coveredTechs = new_tech+completed_techs
        as_columns([tn for tn in alltechs if tn not in tech_base], columns=3)
        debug("")

        if fo.currentTurn() == 1:
            return
        research_queue_list = get_research_queue_techs()
        def_techs = TechsListsAI.defense_techs_1()
        for def_tech in def_techs:
            if (
                aistate.character.may_research_tech_classic(def_tech)
                and def_tech not in research_queue_list[:5]
                and not tech_is_complete(def_tech)
            ):
                res = fo.issueEnqueueTechOrder(def_tech, min(3, len(research_queue_list)))
                debug("Empire is very defensive, so attempted to fast-track %s, got result %d", def_tech, res)
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
    artif_minds = Dep.LRN_ARTIF_MINDS_1

    if have_nest() and not tech_is_complete(nest_tech):
        if artif_minds in research_queue_list:
            insert_idx = 1 + research_queue_list.index(artif_minds)
        else:
            insert_idx = 1
        res = fo.issueEnqueueTechOrder(nest_tech, insert_idx)
        num_techs_accelerated += 1
        debug("Have a monster nest, so attempted to fast-track %s, got result %d", nest_tech, res)
        research_queue_list = get_research_queue_techs()

    #

    #
    # check to accelerate xeno_arch
    if True:  # just to help with cold-folding /  organization
        if (
            have_ruins()
            and not tech_is_complete("LRN_XENOARCH")
            and aistate.character.may_research_tech_classic("LRN_XENOARCH")
        ):
            if artif_minds in research_queue_list:
                insert_idx = 7 + research_queue_list.index(artif_minds)
            elif "GRO_SYMBIOTIC_BIO" in research_queue_list:
                insert_idx = research_queue_list.index("GRO_SYMBIOTIC_BIO") + 1
            else:
                insert_idx = num_techs_accelerated
            if "LRN_XENOARCH" not in research_queue_list[:insert_idx]:
                for xenoTech in ["LRN_XENOARCH", "LRN_TRANSLING_THT", "LRN_PHYS_BRAIN", "LRN_ALGO_ELEGANCE"]:
                    if not tech_is_complete(xenoTech) and xenoTech not in research_queue_list[: (insert_idx + 4)]:
                        res = fo.issueEnqueueTechOrder(xenoTech, insert_idx)
                        num_techs_accelerated += 1
                        debug(
                            "ANCIENT_RUINS: have an ancient ruins, so attempted to fast-track %s to enable LRN_XENOARCH, got result %d",
                            xenoTech,
                            res,
                        )
                research_queue_list = get_research_queue_techs()

    #
    # check to accelerate asteroid or GG tech
    if True:  # just to help with cold-folding / organization
        if have_asteroids():
            insert_idx = (
                num_techs_accelerated
                if "GRO_SYMBIOTIC_BIO" not in research_queue_list
                else research_queue_list.index("GRO_SYMBIOTIC_BIO")
            )
            ast_tech = "PRO_MICROGRAV_MAN"
            if not (tech_is_complete(ast_tech) or ast_tech in research_queue_list[: (1 + insert_idx)]):
                res = fo.issueEnqueueTechOrder(ast_tech, insert_idx)
                num_techs_accelerated += 1
                debug(
                    "Asteroids: plan to colonize an asteroid belt, so attempted to fast-track %s , got result %d",
                    ast_tech,
                    res,
                )
                research_queue_list = get_research_queue_techs()
            elif tech_is_complete("SHP_ZORTRIUM_PLATE"):
                insert_idx = (
                    (1 + insert_idx)
                    if "LRN_FORCE_FIELD" not in research_queue_list
                    else max(1 + insert_idx, research_queue_list.index("LRN_FORCE_FIELD") - 1)
                )
                for ast_tech in ["SHP_ASTEROID_HULLS", "SHP_IMPROVED_ENGINE_COUPLINGS"]:
                    if not tech_is_complete(ast_tech) and ast_tech not in research_queue_list[: insert_idx + 1]:
                        res = fo.issueEnqueueTechOrder(ast_tech, insert_idx)
                        num_techs_accelerated += 1
                        insert_idx += 1
                        debug(
                            "Asteroids: plan to colonize an asteroid belt, so attempted to fast-track %s , got result %d",
                            ast_tech,
                            res,
                        )
                research_queue_list = get_research_queue_techs()
        if have_gas_giant() and not tech_is_complete("PRO_ORBITAL_GEN"):
            fusion_idx = (
                0 if "PRO_FUSION_GEN" not in research_queue_list else (1 + research_queue_list.index("PRO_FUSION_GEN"))
            )
            forcefields_idx = (
                0
                if "LRN_FORCE_FIELD" not in research_queue_list
                else (1 + research_queue_list.index("LRN_FORCE_FIELD"))
            )
            insert_idx = max(fusion_idx, forcefields_idx) if enemies_sighted else fusion_idx
            if "PRO_ORBITAL_GEN" not in research_queue_list[: insert_idx + 1]:
                res = fo.issueEnqueueTechOrder("PRO_ORBITAL_GEN", insert_idx)
                num_techs_accelerated += 1
                debug(
                    "GasGiant: plan to colonize a gas giant, so attempted to fast-track %s, got result %d",
                    "PRO_ORBITAL_GEN",
                    res,
                )
                research_queue_list = get_research_queue_techs()
    #
    # assess if our empire has any non-lousy colonizers, & boost gro_xeno_gen if we don't
    if True:  # just to help with cold-folding / organization
        if got_ggg_tech and got_sym_bio and (not got_xeno_gen):
            most_adequate = 0
            for specName in get_colony_builders():
                environs = {}
                this_spec = fo.getSpecies(specName)
                if not this_spec:
                    continue
                for ptype in [
                    fo.planetType.swamp,
                    fo.planetType.radiated,
                    fo.planetType.toxic,
                    fo.planetType.inferno,
                    fo.planetType.barren,
                    fo.planetType.tundra,
                    fo.planetType.desert,
                    fo.planetType.terran,
                    fo.planetType.ocean,
                    fo.planetType.asteroids,
                ]:
                    environ = this_spec.getPlanetEnvironment(ptype)
                    environs.setdefault(environ, []).append(ptype)
                most_adequate = max(most_adequate, len(environs.get(fo.planetEnvironment.adequate, [])))
            if most_adequate == 0:
                insert_idx = num_techs_accelerated
                for xg_tech in ["GRO_XENO_GENETICS", "GRO_GENETIC_ENG"]:
                    if (
                        xg_tech not in research_queue_list[: 1 + num_techs_accelerated]
                        and not tech_is_complete(xg_tech)
                        and aistate.character.may_research_tech_classic(xg_tech)
                    ):
                        res = fo.issueEnqueueTechOrder(xg_tech, insert_idx)
                        num_techs_accelerated += 1
                        debug(
                            "Empire has poor colonizers, so attempted to fast-track %s, got result %d",
                            xg_tech,
                            res,
                        )
                research_queue_list = get_research_queue_techs()
    #
    # check to accelerate translinguistics
    if True:  # just to help with cold-folding / organization
        # planet is needed to determine the cost. Without a capital we have bigger problems anyway...
        if not tech_is_complete("LRN_TRANSLING_THT") and translators_wanted():
            insert_idx = num_techs_accelerated
            for dt_ech in ["LRN_TRANSLING_THT", "LRN_PHYS_BRAIN", "LRN_ALGO_ELEGANCE"]:
                if (
                    dt_ech not in research_queue_list[: insert_idx + 2]
                    and not tech_is_complete(dt_ech)
                    and aistate.character.may_research_tech_classic(dt_ech)
                ):
                    res = fo.issueEnqueueTechOrder(dt_ech, insert_idx)
                    num_techs_accelerated += 1
                    insert_idx += 1
                    fmt_str = "Empire wants to build translators, so attempted to fast-track %s (got result %d)"
                    fmt_str += " with current target_RP %.1f and current pop %.1f, on turn %d"
                    debug(fmt_str, dt_ech, res, resource_production, empire.population(), fo.currentTurn())
            research_queue_list = get_research_queue_techs()
    #
    # check to accelerate distrib thought
    if True:  # just to help with cold-folding / organization
        if not tech_is_complete("LRN_DISTRIB_THOUGHT"):
            got_telepathy = False
            for specName in get_empire_planets_by_species():
                this_spec = fo.getSpecies(specName)
                if this_spec and ("TELEPATHIC" in list(this_spec.tags)):
                    got_telepathy = True
                    break
            pop_threshold = 100 if got_telepathy else 300
            if empire.population() > pop_threshold:
                insert_idx = num_techs_accelerated
                for dt_ech in ["LRN_PHYS_BRAIN", "LRN_TRANSLING_THT", "LRN_PSIONICS", "LRN_DISTRIB_THOUGHT"]:
                    if (
                        dt_ech not in research_queue_list[: insert_idx + 2]
                        and not tech_is_complete(dt_ech)
                        and aistate.character.may_research_tech_classic(dt_ech)
                    ):
                        res = fo.issueEnqueueTechOrder(dt_ech, insert_idx)
                        num_techs_accelerated += 1
                        insert_idx += 1
                        fmt_str = "Empire has a telepathic race, so attempted to fast-track %s (got result %d)"
                        fmt_str += " with current target_RP %.1f and current pop %.1f, on turn %d"
                        debug(fmt_str, dt_ech, res, resource_production, empire.population(), fo.currentTurn())
                research_queue_list = get_research_queue_techs()

    # if we own a blackhole, accelerate sing_gen and conc camp
    if True:  # just to help with cold-folding / organization
        if (
            fo.currentTurn() > 50
            and len(AIstate.empireStars.get(fo.starType.blackHole, [])) != 0
            and aistate.character.may_research_tech_classic("PRO_SINGULAR_GEN")
            and not tech_is_complete(Dep.PRO_SINGULAR_GEN)
            and tech_is_complete("LRN_EVERYTHING")
        ):
            # sing_tech_list = [ "LRN_GRAVITONICS" , "PRO_SINGULAR_GEN"]  # formerly also "CON_ARCH_PSYCH", "CON_CONC_CAMP",
            sing_gen_tech = fo.getTech(Dep.PRO_SINGULAR_GEN)
            sing_tech_list = [
                pre_req for pre_req in sing_gen_tech.recursivePrerequisites(empire_id) if not tech_is_complete(pre_req)
            ]
            sing_tech_list += [Dep.PRO_SINGULAR_GEN]
            for singTech in sing_tech_list:
                if singTech not in research_queue_list[: num_techs_accelerated + 1]:
                    res = fo.issueEnqueueTechOrder(singTech, num_techs_accelerated)
                    num_techs_accelerated += 1
                    debug(
                        "have a black hole star outpost/colony, so attempted to fast-track %s, got result %d",
                        singTech,
                        res,
                    )
            research_queue_list = get_research_queue_techs()

    #
    # if got deathray from Ruins, remove most prereqs from queue
    if True:  # just to help with cold-folding / organization
        if tech_is_complete("SHP_WEAPON_4_1"):
            this_tech = fo.getTech("SHP_WEAPON_4_1")
            if this_tech:
                missing_prereqs = [
                    preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq in research_queue_list
                ]
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
        new_designs = ShipDesignAI.WarShipDesigner().optimize_design(
            additional_hulls=unlocked_hulls, additional_parts=unlocked_parts, consider_fleet_count=False
        )
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


def _print_research_queue_head(completed_techs: list[str], first_n_techs=10):
    empire_id = fo.getEmpire().empireID
    research_queue = fo.getEmpire().researchQueue

    debug("Techs currently at head of Research Queue")

    for element in islice(research_queue, first_n_techs):
        this_tech = fo.getTech(element.tech)
        if not this_tech:
            warning("Can't retrieve tech %s", element.tech)
            continue
        missing_prereqs = [
            pre_req for pre_req in this_tech.recursivePrerequisites(empire_id) if pre_req not in completed_techs
        ]
        unlocked_items = [uli.name for uli in this_tech.unlockedItems]
        if not missing_prereqs:
            debug(
                "%-27s allocated %6.2f RP (%d turns left) -- unlocks: %s",
                element.tech,
                element.allocation,
                element.turnsLeft,
                unlocked_items,
            )
        else:
            debug(
                "%-27s allocated %6.2f RP -- missing preReqs: %s -- unlocks: %s",
                element.tech,
                element.allocation,
                missing_prereqs,
                unlocked_items,
            )
    debug("")


def _print_research_order_header(resource_production, completed_techs):
    debug("Research Queue Management:")
    debug("\nTotal Current Research Points: %.2f\n", resource_production)
    debug("Techs researched and available for use:")

    tlist = completed_techs + [" "] * 3
    for tline in zip(tlist[0::3], tlist[1::3], tlist[2::3]):
        debug("%25s %25s %25s", *tline)
    debug("")
