import freeOrionAIInterface as fo
import math
import random
from typing import Dict, FrozenSet, Iterable, Iterator, List, Optional, Tuple, Union

import ShipDesignAI
from empire.ship_builders import get_shipyards
from EnumsAI import PriorityType
from freeorion_tools.caching import cache_for_current_turn
from freeorion_tools.timers import AITimer
from turn_state._planet_state import get_inhabited_planets


@cache_for_current_turn
def get_design_repository() -> Dict[PriorityType, Tuple[float, int, int, float]]:
    """Calculate the best designs for each ship class available at this turn."""
    design_repository = {}  # dict of tuples (rating,pid,designID,cost) sorted by rating and indexed by priority type

    design_timer = AITimer("ShipDesigner")
    design_timer.start("Updating cache for new turn")

    # TODO Don't use PriorityType but introduce more reasonable Enum
    designers = [
        ("Orbital Invasion", PriorityType.PRODUCTION_ORBITAL_INVASION, ShipDesignAI.OrbitalTroopShipDesigner),
        ("Invasion", PriorityType.PRODUCTION_INVASION, ShipDesignAI.StandardTroopShipDesigner),
        (
            "Orbital Colonization",
            PriorityType.PRODUCTION_ORBITAL_COLONISATION,
            ShipDesignAI.OrbitalColonisationShipDesigner,
        ),
        ("Colonization", PriorityType.PRODUCTION_COLONISATION, ShipDesignAI.StandardColonisationShipDesigner),
        ("Orbital Outposter", PriorityType.PRODUCTION_ORBITAL_OUTPOST, ShipDesignAI.OrbitalOutpostShipDesigner),
        ("Outposter", PriorityType.PRODUCTION_OUTPOST, ShipDesignAI.StandardOutpostShipDesigner),
        ("Orbital Defense", PriorityType.PRODUCTION_ORBITAL_DEFENSE, ShipDesignAI.OrbitalDefenseShipDesigner),
        ("Scouts", PriorityType.PRODUCTION_EXPLORATION, ShipDesignAI.ScoutShipDesigner),
    ]

    for timer_name, priority_type, designer in designers:
        design_timer.start(timer_name)
        design_repository[priority_type] = designer().optimize_design()
    best_military_stats = ShipDesignAI.WarShipDesigner().optimize_design()
    best_carrier_stats = ShipDesignAI.CarrierShipDesigner().optimize_design()
    best_stats = best_military_stats + best_carrier_stats if random.random() < 0.8 else best_military_stats
    best_stats.sort(reverse=True)
    design_repository[PriorityType.PRODUCTION_MILITARY] = best_stats
    design_timer.start("Krill Spawner")
    ShipDesignAI.KrillSpawnerShipDesigner().optimize_design()  # just designing it, building+mission not supported yet
    if fo.currentTurn() % 10 == 0:
        design_timer.start("Printing")
        ShipDesignAI.Cache.print_best_designs()
    design_timer.stop_print_and_clear()
    return design_repository


@cache_for_current_turn
def cur_best_military_design_rating() -> float:
    """
    Find and return the default combat rating of our best military design.
    """
    design_repository = get_design_repository()

    priority = PriorityType.PRODUCTION_MILITARY
    if design_repository.get(priority, None) and design_repository[priority][0]:
        # the rating provided by the ShipDesigner does not
        # reflect the rating used in threat considerations
        # but takes additional factors (such as cost) into
        # account. Therefore, we want to calculate the actual
        # rating of the design as provided by CombatRatingsAI.
        _, _, _, _, stats = design_repository[priority][0]
        # TODO: Should this consider enemy stats?
        rating = stats.convert_to_combat_stats().get_rating()
        return max(rating, 0.001)
    return 0.001


def _get_locations(locations: Union[int, Iterable[int], None]) -> FrozenSet[int]:
    if locations is None:
        return get_inhabited_planets()

    if isinstance(locations, int):
        locations = (locations,)
    return get_inhabited_planets().intersection(locations)


def get_best_ship_info(
    priority: PriorityType, loc: Union[int, Iterable[int], None] = None
) -> Tuple[Optional[int], Optional["fo.shipDesign"], Optional[List[int]]]:
    """Returns 3 item tuple: designID, design, buildLocList."""
    planet_ids = _get_locations(loc)
    if not planet_ids:
        return None, None, None
    return _get_best_ship_info(priority, tuple(planet_ids))


def _get_best_ship_info(
    priority: PriorityType, planet_ids: Tuple[int]
) -> Tuple[Optional[int], Optional["fo.shipDesign"], Optional[List[int]]]:
    design_repository = get_design_repository()

    if priority in design_repository:
        best_designs = design_repository[priority]
        if not best_designs:
            return None, None, None

        # best_designs are already sorted by rating high to low, so the top rating is the first encountered within
        # our planet search list
        for design_stats in best_designs:
            top_rating, pid, top_id, cost, stats = design_stats
            if pid in planet_ids:
                break
        else:
            return None, None, None  # apparently can't build for this priority within the desired planet group
        valid_locs = [
            pid_
            for rating, pid_, design_id, _, _ in best_designs
            if rating == top_rating and design_id == top_id and pid_ in planet_ids
        ]
        return top_id, fo.getShipDesign(top_id), valid_locs
    else:
        return None, None, None  # must be missing a Shipyard or other orbital (or missing tech)


def get_best_ship_ratings(planet_ids: Tuple[int]) -> List[Tuple[float, int, int, "fo.shipDesign"]]:
    """
    Returns list of [partition, pid, designID, design] tuples, currently only for military ships.

    Since we haven't yet implemented a way to target military ship construction at/near particular locations
    where they are most in need, and also because our rating system is presumably useful-but-not-perfect, we want to
    distribute the construction across the Resource Group and across similarly rated designs, preferentially choosing
    the best rated design/loc combo, but if there are multiple design/loc combos with the same or similar ratings then
    we want some chance of choosing  those alternate designs/locations.

    The approach to this taken below is to treat the ratings akin to an energy to be used in a statistical mechanics
    type partition function. 'tally' will compute the normalization constant.
    So first go through and calculate the tally as well as convert each individual contribution to
    the running total up to that point, to facilitate later sampling.  Then those running totals are
    renormalized by the final tally, so that a later random number selector in the range [0,1) can be
    used to select the chosen design/loc.
    """
    return list(_get_best_ship_ratings(tuple(planet_ids)))


@cache_for_current_turn
def _get_best_ship_ratings(planet_ids: Tuple[int]) -> Iterator[Tuple[float, int, int, "fo.shipDesign"]]:
    design_repository = get_design_repository()
    priority = PriorityType.PRODUCTION_MILITARY
    planet_ids = set(planet_ids).intersection(get_shipyards())

    if priority not in design_repository:
        return

    build_choices = design_repository[priority]
    loc_choices = [
        [rating, pid, design_id, fo.getShipDesign(design_id)]
        for (rating, pid, design_id, cost, stats) in build_choices
        if pid in planet_ids
    ]
    if not loc_choices:
        return
    best_rating = loc_choices[0][0]
    tally = 0
    ret_val = []
    for rating, pid, design_id, design in loc_choices:
        if rating < 0.7 * best_rating:
            break
        p = math.exp(10 * (rating / best_rating - 1))
        tally += p
        ret_val.append((tally, pid, design_id, design))
    for base_tally, pid, design_id, design in ret_val:
        yield base_tally / tally, pid, design_id, design
