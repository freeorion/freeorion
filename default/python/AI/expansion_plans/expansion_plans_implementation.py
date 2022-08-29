import freeOrionAIInterface as fo
from collections import OrderedDict as odict
from itertools import chain
from logging import debug
from typing import FrozenSet, Iterable, OrderedDict, Tuple

from aistate_interface import get_aistate
from common.fo_typing import PlanetId, SpeciesName
from EnumsAI import MissionType
from expansion_plans import expansion_plans_interface
from FleetUtilsAI import get_targeted_planet_ids


def initialise_expansion_plans():
    expansion_plans_interface._the_planner = ExpansionPlanner()


class ExpansionPlanner:
    @staticmethod
    def colonies_targeted() -> FrozenSet[PlanetId]:
        colony_ship_pids = get_targeted_planet_ids(fo.getUniverse().planetIDs, MissionType.COLONISATION)
        colony_building_pids = [e.locationID for e in fo.getEmpire().productionQueue if e.name.startswith("BLD_COL_")]
        debug(f"colonies_targeted: colony_ship_pids={colony_ship_pids}, colony_building_pids={colony_building_pids}")
        return frozenset(chain(colony_ship_pids, colony_building_pids))

    @staticmethod
    def outposts_targeted() -> FrozenSet[PlanetId]:
        outpost_ship_pids = get_targeted_planet_ids(fo.getUniverse().planetIDs, MissionType.OUTPOST)
        outpost_base_pids = get_targeted_planet_ids(fo.getUniverse().planetIDs, MissionType.ORBITAL_OUTPOST)
        debug(f"outpost_targeted: outpost_ship_pids={outpost_ship_pids}, outpost_base_pids={outpost_base_pids}")
        return frozenset(chain(outpost_ship_pids, outpost_base_pids))

    @staticmethod
    def get_colonisable_planet_ids(include_targeted: bool = False) -> OrderedDict[PlanetId, Tuple[float, SpeciesName]]:
        if include_targeted:
            return get_aistate().colonisablePlanetIDs
        else:
            return odict(
                (pid, values)
                for (pid, values) in get_aistate().colonisablePlanetIDs.items()
                if pid not in expansion_plans_interface.colonies_targeted()  # interface function is cached
            )

    @staticmethod
    def get_colonisable_outpost_ids(include_targeted: bool = False) -> OrderedDict[PlanetId, Tuple[float, SpeciesName]]:
        if include_targeted:
            return get_aistate().colonisableOutpostIDs
        else:
            return odict(
                (pid, values)
                for (pid, values) in get_aistate().colonisableOutpostIDs.items()
                if pid not in expansion_plans_interface.outposts_targeted()  # interface function is cached
            )

    @staticmethod
    def update_colonisable_planet_ids(new_list: Iterable) -> None:
        get_aistate().colonisablePlanetIDs.clear()
        get_aistate().colonisablePlanetIDs.update(new_list)

    @staticmethod
    def update_colonisable_outpost_ids(new_list: Iterable) -> None:
        get_aistate().colonisableOutpostIDs.clear()
        get_aistate().colonisableOutpostIDs.update(new_list)
