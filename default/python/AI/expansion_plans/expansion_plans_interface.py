from collections import OrderedDict
from collections.abc import Iterable

from common.fo_typing import PlanetId, SpeciesName
from freeorion_tools.caching import cache_for_current_turn

_the_planner = None


@cache_for_current_turn
def colonies_targeted() -> frozenset[PlanetId]:
    return _the_planner.colonies_targeted()


@cache_for_current_turn
def outposts_targeted() -> frozenset[PlanetId]:
    return _the_planner.outposts_targeted()


def get_colonisable_planet_ids(include_targeted: bool = False) -> OrderedDict[PlanetId, tuple[float, SpeciesName]]:
    return _the_planner.get_colonisable_planet_ids(include_targeted)


def get_colonisable_outpost_ids(include_targeted: bool = False) -> OrderedDict[PlanetId, tuple[float, SpeciesName]]:
    return _the_planner.get_colonisable_outpost_ids(include_targeted)


def update_colonisable_planet_ids(new_list: Iterable) -> None:
    return _the_planner.update_colonisable_planet_ids(new_list)


def update_colonisable_outpost_ids(new_list: Iterable) -> None:
    return _the_planner.update_colonisable_outpost_ids(new_list)
