import freeOrionAIInterface as fo
from logging import debug, error, info, warning
from operator import itemgetter

import AIDependencies
import AIstate
import ExplorationAI
import FleetUtilsAI
import PlanetUtilsAI
from AIDependencies import INVALID_ID, OUTPOSTING_TECH
from aistate_interface import get_aistate
from colonization.calculate_planet_colonization_rating import (
    calculate_planet_colonization_rating,
)
from colonization.colony_score import MINIMUM_COLONY_SCORE
from colonization.planet_supply import get_planet_supply
from common.print_utils import Number, Sequence, Table, Text
from empire.colony_builders import can_build_colony_for_species, get_colony_builders
from EnumsAI import EmpireProductionTypes, MissionType, PriorityType, ShipRoleType
from expansion_plans import (
    get_colonisable_outpost_ids,
    get_colonisable_planet_ids,
    update_colonisable_outpost_ids,
    update_colonisable_planet_ids,
)
from freeorion_tools import get_ship_part
from freeorion_tools.caching import cache_for_current_turn
from freeorion_tools.timers import AITimer
from target import TargetPlanet
from turn_state import (
    get_colonized_planets_in_system,
    get_empire_outposts,
    get_number_of_colonies,
    get_unowned_empty_planets,
)
from turn_state.design import get_best_ship_info

colonization_timer = AITimer("getColonyFleets()")

_all_colony_opportunities = {}


@cache_for_current_turn
def colony_pod_cost_turns():
    empire = fo.getEmpire()
    empire_id = empire.empireID
    loc = INVALID_ID
    pid = INVALID_ID
    parts = [get_ship_part(part) for part in list(empire.availableShipParts)]
    colo_parts = [
        part for part in parts if part.partClass in frozenset({fo.shipPartClass.colony}) and part.capacity > 0
    ]
    if colo_parts:
        colo_part = max(colo_parts, key=lambda x: x.capacity)
        base_cost = colo_part.productionCost(empire_id, pid, loc)
        build_turns = colo_part.productionTime(empire_id, pid, loc)
    else:
        base_cost = 0
        build_turns = 0
        debug("no available colony parts with capacity > 0")
    return (base_cost * (1 + get_number_of_colonies() * AIDependencies.COLONY_POD_UPKEEP), build_turns)


def outpod_pod_cost():
    return AIDependencies.OUTPOST_POD_COST * (1 + get_number_of_colonies() * AIDependencies.COLONY_POD_UPKEEP)


def galaxy_is_sparse():
    setup_data = fo.getGalaxySetupData()
    avg_empire_systems = setup_data.size // len(fo.allEmpireIDs())
    return (setup_data.monsterFrequency <= fo.galaxySetupOptionMonsterFreq.veryLow) and (
        (avg_empire_systems >= 40) or ((avg_empire_systems >= 35) and (setup_data.shape != fo.galaxyShape.elliptical))
    )


def get_colony_fleets():
    """examines known planets, collects various colonization data, to be later used to send colony fleets"""
    universe = fo.getUniverse()
    empire = fo.getEmpire()

    colonization_timer.start("Identify Existing colony/outpost targets")
    colony_targeted_planet_ids = FleetUtilsAI.get_targeted_planet_ids(universe.planetIDs, MissionType.COLONISATION)
    all_colony_targeted_system_ids = PlanetUtilsAI.get_systems(colony_targeted_planet_ids)
    colony_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.COLONISATION)
    num_colony_fleets = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(colony_fleet_ids))

    outpost_targeted_planet_ids = FleetUtilsAI.get_targeted_planet_ids(universe.planetIDs, MissionType.OUTPOST)
    outpost_targeted_planet_ids.extend(
        FleetUtilsAI.get_targeted_planet_ids(universe.planetIDs, MissionType.ORBITAL_OUTPOST)
    )
    all_outpost_targeted_system_ids = PlanetUtilsAI.get_systems(outpost_targeted_planet_ids)
    outpost_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.OUTPOST)
    num_outpost_fleets = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(outpost_fleet_ids))

    debug("Colony Targeted SystemIDs: %s" % all_colony_targeted_system_ids)
    debug("Colony Targeted PlanetIDs: %s" % colony_targeted_planet_ids)
    debug(colony_fleet_ids and "Colony Fleet IDs: %s" % colony_fleet_ids or "Available Colony Fleets: 0")
    debug("Colony Fleets Without Missions: %s" % num_colony_fleets)
    debug("")
    debug("Outpost Targeted SystemIDs: %s" % all_outpost_targeted_system_ids)
    debug("Outpost Targeted PlanetIDs: %s" % outpost_targeted_planet_ids)
    debug(outpost_fleet_ids and "Outpost Fleet IDs: %s" % outpost_fleet_ids or "Available Outpost Fleets: 0")
    debug("Outpost Fleets Without Missions: %s" % num_outpost_fleets)
    debug("")

    # export targeted systems for other AI modules
    AIstate.colonyTargetedSystemIDs = all_colony_targeted_system_ids
    AIstate.outpostTargetedSystemIDs = all_outpost_targeted_system_ids

    colonization_timer.start("Identify colony base targets")
    # keys are sets of ints; data is doubles
    available_pp = {tuple(el.key()): el.data() for el in empire.planetsWithAvailablePP}

    avail_pp_by_sys = {}
    for p_set in available_pp:
        avail_pp_by_sys.update([(sys_id, available_pp[p_set]) for sys_id in set(PlanetUtilsAI.get_systems(p_set))])

    evaluated_colony_planet_ids = list(
        get_unowned_empty_planets().union(get_empire_outposts()) - set(colony_targeted_planet_ids)
    )  # places for possible colonyBase

    aistate = get_aistate()
    outpost_base_manager = aistate.orbital_colonization_manager

    for pid in evaluated_colony_planet_ids:  # TODO: reorganize
        planet = universe.getPlanet(pid)
        if not planet:
            continue
        sys_id = planet.systemID
        for pid2 in get_colonized_planets_in_system(sys_id):
            planet2 = universe.getPlanet(pid2)
            if not (planet2 and can_build_colony_for_species(planet2.speciesName)):
                continue
            if planet.unowned:
                outpost_base_manager.create_new_plan(pid, pid2)

    colonization_timer.start("Initiate outpost base construction")

    reserved_outpost_base_targets = outpost_base_manager.get_targets()
    debug("Current qualifyingOutpostBaseTargets: %s" % reserved_outpost_base_targets)
    outpost_base_manager.build_bases()

    colonization_timer.start("Evaluate All Colony Opportunities")
    evaluated_outpost_planet_ids = list(
        get_unowned_empty_planets()
        - set(outpost_targeted_planet_ids)
        - set(colony_targeted_planet_ids)
        - set(reserved_outpost_base_targets)
    )
    _all_colony_opportunities.clear()
    _all_colony_opportunities.update(
        assign_colonisation_values(evaluated_colony_planet_ids, MissionType.COLONISATION, None, return_all=True)
    )
    evaluated_colony_planets = {pid: values[0] for pid, values in _all_colony_opportunities.items()}
    colonization_timer.start("Evaluate Outpost Opportunities")

    sorted_planets = list(evaluated_colony_planets.items())
    sorted_planets.sort(key=itemgetter(1), reverse=True)

    _print_colony_candidate_table(sorted_planets, show_detail=False)

    sorted_planets = [(planet_id, score[:2]) for planet_id, score in sorted_planets if score[0] > 0]
    # export planets for other AI modules
    update_colonisable_planet_ids(sorted_planets)

    evaluated_outpost_planets = assign_colonisation_values(evaluated_outpost_planet_ids, MissionType.OUTPOST, None)
    # if outposted planet would be in supply range, let outpost value be best of outpost value or colonization value
    for pid in set(evaluated_outpost_planets).intersection(evaluated_colony_planets):
        if get_planet_supply(pid, -99) >= 0:
            evaluated_outpost_planets[pid] = (
                max(evaluated_colony_planets[pid][0], evaluated_outpost_planets[pid][0]),
                "",
            )

    colonization_timer.stop()

    sorted_outposts = list(evaluated_outpost_planets.items())
    sorted_outposts.sort(key=itemgetter(1), reverse=True)

    _print_outpost_candidate_table(sorted_outposts)

    sorted_outposts = [(planet_id, score[:2]) for planet_id, score in sorted_outposts if score[0] > 0]
    # export outposts for other AI modules
    update_colonisable_outpost_ids(sorted_outposts)
    colonization_timer.stop_print_and_clear()


# TODO: clean up suppliable versus annexable
def assign_colonisation_values(planet_ids, mission_type, species, detail=None, return_all=False):
    """Creates a dictionary that takes planetIDs as key and their colonisation score as value."""
    if detail is None:
        detail = []
    orig_detail = detail
    planet_values = {}
    if mission_type == MissionType.OUTPOST:
        # print "\n=========\nAssigning Outpost Values\n========="
        try_species = [""]
    elif species is not None:
        # print "\n=========\nAssigning Colony Values\n========="
        if isinstance(species, str):
            try_species = [species]
        elif isinstance(species, list):
            try_species = species
        else:
            try_species = [species.name]
    else:
        # print "\n=========\nAssigning Colony Values\n========="
        try_species = list(get_colony_builders())
    for planet_id in planet_ids:
        pv = []
        for spec_name in try_species:
            # TODO: this function never returned detail to its caller, can we remove it?
            detail = orig_detail[:]
            # appends (score, species_name, detail)
            pv.append(
                (
                    calculate_planet_colonization_rating(
                        planet_id=planet_id,
                        mission_type=mission_type,
                        spec_name=spec_name,
                        detail=detail,
                    ),
                    spec_name,
                    detail,
                )
            )
        all_sorted = sorted(pv, reverse=True)
        best = all_sorted[:1]
        if best:
            if return_all:
                planet_values[planet_id] = all_sorted
            else:
                planet_values[planet_id] = best[0]
                # print best[0][2]
    return planet_values


def assign_colony_fleets_to_colonise():
    aistate = get_aistate()
    aistate.orbital_colonization_manager.assign_bases_to_colonize()

    # assign fleet targets to colonisable planets
    all_colony_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.COLONISATION)
    send_colony_ships(
        FleetUtilsAI.extract_fleet_ids_without_mission_types(all_colony_fleet_ids),
        list(get_colonisable_planet_ids().items()),
        MissionType.COLONISATION,
    )

    # assign fleet targets to colonisable outposts
    all_outpost_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.OUTPOST)
    send_colony_ships(
        FleetUtilsAI.extract_fleet_ids_without_mission_types(all_outpost_fleet_ids),
        list(get_colonisable_outpost_ids().items()),
        MissionType.OUTPOST,
    )


def send_colony_ships(colony_fleet_ids, evaluated_planets, mission_type):  # noqa: C901
    """sends a list of colony ships to a list of planet_value_pairs"""
    fleet_pool = colony_fleet_ids[:]
    try_all = False
    if mission_type == MissionType.OUTPOST:
        cost = 20 + outpod_pod_cost()
    else:
        try_all = True
        cost = 20 + colony_pod_cost_turns()[1]
        if fo.currentTurn() < 50:
            cost *= 0.4  # will be making fast tech progress so value is underestimated
        elif fo.currentTurn() < 80:
            cost *= 0.8  # will be making fast-ish tech progress so value is underestimated

    potential_targets = [
        (pid, (score, specName))
        for (pid, (score, specName)) in evaluated_planets
        if score > (0.8 * cost) and score > MINIMUM_COLONY_SCORE
    ]

    debug(f"Colony/outpost ship matching: fleets {fleet_pool} to planets {evaluated_planets}")

    if try_all:
        debug("Trying best matches to current colony ships")
        best_scores = dict(evaluated_planets)
        potential_targets = []
        for pid, ratings in _all_colony_opportunities.items():
            for rating in ratings:
                if rating[0] >= 0.75 * best_scores.get(pid, [9999])[0]:
                    potential_targets.append((pid, rating))
        potential_targets.sort(key=itemgetter(1), reverse=True)

    # added a lot of checking because have been getting mysterious exception, after too many recursions to get info
    fleet_pool = set(fleet_pool)
    universe = fo.getUniverse()
    for fid in fleet_pool:
        fleet = universe.getFleet(fid)
        if not fleet or fleet.empty:
            warning("Bad fleet ( ID %d ) given to colonization routine; will be skipped" % fid)
            fleet_pool.remove(fid)
            continue
        report_str = "Fleet ID (%d): %d ships; species: " % (fid, fleet.numShips)
        for sid in fleet.shipIDs:
            ship = universe.getShip(sid)
            if not ship:
                report_str += "NoShip, "
            else:
                report_str += "%s, " % ship.speciesName
        debug(report_str)
    debug("")
    already_targeted = []
    # for planetID_value_pair in evaluatedPlanets:
    aistate = get_aistate()
    while fleet_pool and potential_targets:
        target = potential_targets.pop(0)
        if target in already_targeted:
            continue
        planet_id = target[0]
        if planet_id in already_targeted:
            continue
        planet = universe.getPlanet(planet_id)
        sys_id = planet.systemID
        if (
            aistate.systemStatus.setdefault(sys_id, {}).setdefault("monsterThreat", 0) > 2000
            or fo.currentTurn() < 20
            and aistate.systemStatus[sys_id]["monsterThreat"] > 200
        ):
            debug(
                "Skipping colonization of system %s due to Big Monster, threat %d"
                % (universe.getSystem(sys_id), aistate.systemStatus[sys_id]["monsterThreat"])
            )
            already_targeted.append(planet_id)
            continue

        # make sure not to run into stationary guards
        if ExplorationAI.system_could_have_unknown_stationary_guard(sys_id):
            ExplorationAI.request_emergency_exploration(sys_id)
            continue

        this_spec = target[1][1]
        found_fleets = []
        try:
            this_fleet_list = FleetUtilsAI.get_fleets_for_mission(
                target_stats={},
                min_stats={},
                cur_stats={},
                starting_system=sys_id,
                species=this_spec,
                fleet_pool_set=fleet_pool,
                fleet_list=found_fleets,
            )
        except Exception as e:
            error(e, exc_info=True)
            continue
        if not this_fleet_list:
            fleet_pool.update(found_fleets)  # just to be safe
            continue  # must have no compatible colony/outpost ships
        fleet_id = this_fleet_list[0]
        already_targeted.append(planet_id)
        ai_target = TargetPlanet(planet_id)
        aistate.get_fleet_mission(fleet_id).set_target(mission_type, ai_target)


def _print_outpost_candidate_table(candidates, show_detail=False):
    """Print a summary for the outpost candidates in a table format to log.

    :param candidates: list of (planet_id, (score, species, details)) tuples
    """
    __print_candidate_table(candidates, mission="Outposts", show_detail=show_detail)


def _print_colony_candidate_table(candidates, show_detail=False):
    """Print a summary for the colony candidates in a table format to log.

    :param candidates: list of (planet_id, (score, species, details)) tuples
    """
    __print_candidate_table(candidates, mission="Colonization", show_detail=show_detail)


def __print_candidate_table(candidates, mission, show_detail=False):
    universe = fo.getUniverse()
    if mission == "Colonization":
        first_column = Text("Score/Species")

        def get_first_column_value(x):
            return round(x[0], 2), x[1]

    elif mission == "Outposts":
        first_column = Number("Score")
        get_first_column_value = itemgetter(0)
    else:
        warning(f"__print_candidate_table({candidates}, {mission}): Invalid mission type")
        return
    columns = [first_column, Text("Planet"), Text("System"), Sequence("Specials")]
    if show_detail:
        columns.append(Sequence("Detail"))
    candidate_table = Table(*columns, table_name="Potential Targets for %s in Turn %d" % (mission, fo.currentTurn()))
    for planet_id, score_tuple in candidates:
        if score_tuple[0] > 0.5:
            planet = universe.getPlanet(planet_id)
            entries = [
                get_first_column_value(score_tuple),
                planet,
                universe.getSystem(planet.systemID),
                planet.specials,
            ]
            if show_detail:
                entries.append(score_tuple[-1])
            candidate_table.add_row(*entries)
    info(candidate_table)


class OrbitalColonizationPlan:
    def __init__(self, target_id: int, source_id: int):
        """
        :param target_id: id of the target planet to colonize
        :param source_id: id of the planet which should build the colony base
        """
        self.target = target_id
        self.source = source_id
        self.base_enqueued = False
        self.fleet_id = INVALID_ID
        self.__score = 0
        self.__last_score_update = -1

    def assign_base(self, fleet_id: int) -> bool:
        """
        Assign an outpost base fleet to execute the plan.

        It is expected that the fleet consists of only that one outpost base.

        :return: True on success, False on failure
        """
        if self.base_assigned:
            warning("Assigned a base to a plan that was already assigned a base to.")
            return False
        # give orders to perform the mission
        target = TargetPlanet(self.target)
        fleet_mission = get_aistate().get_fleet_mission(fleet_id)
        fleet_mission.set_target(MissionType.ORBITAL_OUTPOST, target)
        self.fleet_id = fleet_id
        return True

    def enqueue_base(self) -> bool:
        """
        Enqueue the base according to the plan.

        :return: True on success, False on failure
        """
        if self.base_enqueued:
            warning("Tried to enqueue a base eventhough already done that.")
            return False

        # find the best possible base design for the source planet
        universe = fo.getUniverse()
        best_ship, _, _ = get_best_ship_info(PriorityType.PRODUCTION_ORBITAL_OUTPOST, self.source)
        if best_ship is None:
            warning("Can't find optimized outpost base design at %s" % (universe.getPlanet(self.source)))
            try:
                best_ship = next(
                    design
                    for design in fo.getEmpire().availableShipDesigns
                    if "SD_OUTPOST_BASE" == fo.getShipDesign(design).name
                )
                debug("Falling back to base design SD_OUTPOST_BASE")
            except StopIteration:
                # fallback design not available
                return False

        # enqueue the design at the source planet
        retval = fo.issueEnqueueShipProductionOrder(best_ship, self.source)
        debug(
            f"Enqueueing Outpost Base at {universe.getPlanet(self.source)} for {universe.getPlanet(self.target)} with result {retval}"
        )

        if not retval:
            warning("Failed to enqueue outpost base at %s" % universe.getPlanet(self.source))
            return False

        self.base_enqueued = True
        return True

    @property
    def base_assigned(self):
        if self.fleet_id == INVALID_ID:
            return False

        fleet = fo.getUniverse().getFleet(self.fleet_id)
        if fleet:
            return True

        debug("The fleet assigned to the OrbitalColonizationPlan doesn't exist anymore.")
        self.fleet_id = INVALID_ID
        return False

    @property
    def score(self):
        if self.__last_score_update != fo.currentTurn():
            self.__update_score()
        return self.__score

    def __update_score(self):
        planet_score = calculate_planet_colonization_rating(
            planet_id=self.target,
            mission_type=MissionType.OUTPOST,
            spec_name=None,
            detail=None,
        )
        for species in get_colony_builders():
            this_score = calculate_planet_colonization_rating(
                planet_id=self.target,
                mission_type=MissionType.COLONISATION,
                spec_name=species,
                detail=None,
            )
            planet_score = max(planet_score, this_score)
        self.__last_score_update = fo.currentTurn()
        self.__score = planet_score

    def is_valid(self) -> bool:
        """
        Check the colonization plan for validity, i.e. if it could be executed in the future.

        The plan is valid if it is possible to outpost the target planet
        and if the planet envisioned to build the outpost bases can still do so.
        """
        universe = fo.getUniverse()

        # make sure target is valid
        target = universe.getPlanet(self.target)
        if target is None or (not target.unowned) or target.speciesName:
            return False

        # make sure source is valid
        source = universe.getPlanet(self.source)
        if not (
            source
            and source.ownedBy(fo.empireID())
            and source.speciesName
            and fo.getSpecies(source.speciesName).canColonize
        ):
            return False

        # appears to be valid
        return True


class OrbitalColonizationManager:
    """
    The OrbitalColonizationManager handles orbital colonization for the AI.

    :type _colonization_plans: dict[int, OrbitalColonizationPlan]
    :type num_enqueued_bases: int
    """

    def __init__(self):
        self._colonization_plans = {}
        self.num_enqueued_bases = 0

    def get_targets(self) -> list[int]:
        """
        Return all planets for which an orbital colonization plan exists.
        """
        return list(self._colonization_plans.keys())

    def create_new_plan(self, target_id: int, source_id: int):
        """
        Create and keep track of a new colonization plan for a target planet.

        :param target_id: id of the target planet
        :param source_id: id of the planet which is supposed to build the base
        """
        if target_id in self._colonization_plans:
            warning("Already have a colonization plan for this planet. Doing nothing.")
            return
        self._colonization_plans[target_id] = OrbitalColonizationPlan(target_id, source_id)

    def turn_start_cleanup(self):  # noqa: C901
        universe = fo.getUniverse()
        # clean up invalid or finished plans
        for pid in list(self._colonization_plans.keys()):
            if not self._colonization_plans[pid].is_valid():
                del self._colonization_plans[pid]

        # parse the production queue and find bases which no longer have valid
        # targets (e.g. the planet was colonized already).
        self.num_enqueued_bases = 0
        unaccounted_plans = dict(self._colonization_plans)

        # Check which plans still have valid bases assigned (possibly interrupted by combat last turn)
        for pid in list(unaccounted_plans.keys()):
            if unaccounted_plans[pid].base_assigned:
                del unaccounted_plans[pid]

        # find enqueued bases which are no longer needed and dequeue those.
        items_to_dequeue = []
        aistate = get_aistate()
        for idx, element in enumerate(fo.getEmpire().productionQueue):
            if element.buildType != EmpireProductionTypes.BT_SHIP or element.turnsLeft == -1:
                continue

            role = aistate.get_ship_role(element.designID)
            if role != ShipRoleType.BASE_OUTPOST:
                continue

            self.num_enqueued_bases += 1
            # check if a target for this base remains
            original_target = next(
                (
                    target
                    for target, plan in unaccounted_plans.items()
                    if plan.source == element.locationID and plan.base_enqueued
                ),
                None,
            )
            if original_target:
                debug("Base built at %d still has its original target." % element.locationID)
                del unaccounted_plans[original_target]
                continue

            # the original target may be no longer valid but maybe there is another
            # orbital colonization plan which wasn't started yet and has the same source planet
            alternative_target = next(
                (target for target, plan in unaccounted_plans.items() if plan.source == element.locationID), None
            )
            if alternative_target:
                debug(
                    "Reassigning base built at %d to new target %d as old target is no longer valid"
                    % (element.locationID, alternative_target)
                )
                self._colonization_plans[alternative_target].base_enqueued = True
                del unaccounted_plans[alternative_target]
                continue

            # final try: unstarted plans with source in the same system
            target_system = universe.getSystem(universe.getPlanet(element.locationID).systemID)
            alternative_plan = next(
                (
                    plan
                    for target, plan in unaccounted_plans.items()
                    if plan.source in target_system.planetIDs and not plan.base_enqueued and not plan.base_assigned
                ),
                None,
            )
            if alternative_plan:
                debug(
                    "Reassigning base enqueued at %d to new plan with target %d. Previous source was %d"
                    % (element.locationID, alternative_plan.target, alternative_plan.source)
                )
                alternative_plan.source = element.locationID
                alternative_plan.base_enqueued = True
                del unaccounted_plans[alternative_plan.target]
                continue

            debug(
                "Could not find a target for the outpost base enqueued at %s" % universe.getPlanet(element.locationID)
            )
            items_to_dequeue.append(idx)

        # TODO: Stop Building for targets with now insufficient colonization score

        # delete last items first so that queue index of remaining items
        # does not have to be adjusted
        # TODO: Only pause the production if could become valid again
        items_to_dequeue.sort(reverse=True)
        for idx in items_to_dequeue:
            fo.issueDequeueProductionOrder(idx)
            self.num_enqueued_bases -= 1

    def build_bases(self):
        empire = fo.getEmpire()
        if not empire.techResearched(OUTPOSTING_TECH):
            return

        considered_plans = [
            plan
            for plan in self._colonization_plans.values()
            if not plan.base_enqueued and plan.score > MINIMUM_COLONY_SCORE
        ]
        queue_limit = max(1, int(2 * empire.productionPoints / outpod_pod_cost()))
        for colonization_plan in sorted(considered_plans, key=lambda x: x.score, reverse=True):
            if self.num_enqueued_bases >= queue_limit:
                debug("Base enqueue limit (%d) reached." % queue_limit)
                return

            success = colonization_plan.enqueue_base()
            if success:
                self.num_enqueued_bases += 1

    def assign_bases_to_colonize(self):
        universe = fo.getUniverse()
        all_outpost_base_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.ORBITAL_OUTPOST)
        avail_outpost_base_fleet_ids = FleetUtilsAI.extract_fleet_ids_without_mission_types(all_outpost_base_fleet_ids)
        for fid in avail_outpost_base_fleet_ids:
            fleet = universe.getFleet(fid)
            if not fleet:
                continue
            sys_id = fleet.systemID
            system = universe.getSystem(sys_id)

            avail_plans = [
                plan
                for plan in self._colonization_plans.values()
                if plan.target in system.planetIDs and not plan.base_assigned
            ]
            avail_plans.sort(key=lambda x: x.score, reverse=True)
            for plan in avail_plans:
                success = plan.assign_base(fid)
                if success:
                    break
