import freeOrionAIInterface as fo
from collections import defaultdict
from copy import copy
from enum import Enum
from typing import DefaultDict, List, NamedTuple, Set

from common.fo_typing import BuildingName, PlanetId, SystemId
from freeorion_tools.caching import cache_for_current_turn
from PlanetUtilsAI import Opinion, get_planet_opinion
from turn_state import get_all_empire_planets


class BuildingType(Enum):
    """
    Enum to represent fo building template.

    This class encapsulate FOCS names and fo API usage exposing convenient API for AI.
    It should not contain any decision logic, only provide information that is easy to handle.

    It should not accept or expose any fo.* object.

    Note: It does not required to have exactly the same name and value.
    """

    GAS_GIANT_GEN = "BLD_GAS_GIANT_GEN"
    PALACE = "BLD_IMPERIAL_PALACE"
    REGIONAL_ADMIN = "BLD_REGIONAL_ADMIN"
    SCANNING_FACILITY = "BLD_SCANNING_FACILITY"
    SHIPYARD_ENRG_COMP = "BLD_SHIPYARD_ENRG_COMP"
    SHIPYARD_AST = "BLD_SHIPYARD_AST"
    TRANSLATOR = "BLD_TRANSLATOR"
    MILITARY_COMMAND = "BLD_MILITARY_COMMAND"

    def enqueue(self, pid: PlanetId) -> bool:
        """
        Add building to production queue and return True if succeeded.
        """
        return bool(fo.issueEnqueueBuildingProductionOrder(self.value, pid))

    def available(self) -> bool:
        """
        Return true if this building is available for empire.
        """
        return fo.getEmpire().buildingTypeAvailable(self.value)

    def get_opinions(self) -> Opinion:
        """
        Returns opinions about the building at the empire's planets.
        """
        return get_planet_opinion(self.value)

    def queued_in(self) -> List[PlanetId]:
        """
        Return list of planet ids where this building is queued.
        """
        return [element.locationID for element in fo.getEmpire().productionQueue if (element.name == self.value)]

    def built_at(self) -> Set[PlanetId]:
        """
        Return List of planet ids where the building exists.
        """
        return _get_building_locations()[self.value].planets

    def built_at_sys(self) -> Set[SystemId]:
        """
        Return List of system ids where the building exists.
        """
        return _get_building_locations()[self.value].systems

    def built_or_queued_at(self) -> Set[PlanetId]:
        """
        Return List of planet ids where the building either exists or is queued.
        """
        built_at_planets = _get_building_locations()[self.value].planets
        ret = copy(built_at_planets)
        ret.update(set(self.queued_in()))
        return ret

    def built_or_queued_at_sys(self) -> Set[SystemId]:
        """
        Return List of system ids where the building either exists or is queued.
        """
        built_at_systems = _get_building_locations()[self.value].systems
        ret = copy(built_at_systems)
        ret.update({fo.getUniverse().getPlanet(pid).systemID for pid in self.queued_in()})
        return ret

    def can_be_enqueued(self, planet: PlanetId) -> bool:
        """
        Return whether the building can be enqueued at the given planet.
        """
        return fo.getBuildingType(self.value).canBeEnqueued(fo.empireID(), planet)

    def can_be_produced(self, planet: PlanetId) -> bool:
        """
        Return whether the building can be produced at the given planet.
        """
        return fo.getBuildingType(self.value).canBeProduced(fo.empireID(), planet)

    def production_cost(self, planet: PlanetId) -> float:
        """
        Returns overall production cost of the building at the given planet.
        """
        return fo.getBuildingType(self.value).productionCost(fo.empireID(), planet)

    def production_time(self, planet: PlanetId) -> int:
        """
        Returns minimum number of turns the building takes to finish at the given planet.
        """
        return fo.getBuildingType(self.value).productionTime(fo.empireID(), planet)

    def turn_cost(self, planet: PlanetId) -> float:
        """
        Returns production points then can be spent per turn for the building at the given planet.
        """
        return fo.getBuildingType(self.value).perTurnCost(fo.empireID(), planet)


class _BuildingLocations(NamedTuple):
    """
    A set of planets and systems that already contain a building.
    """

    planets: Set[PlanetId]
    systems: Set[SystemId]


# Cannot use BuildingType as key since not all buildings may have an enum value
@cache_for_current_turn
def _get_building_locations() -> DefaultDict[BuildingName, _BuildingLocations]:
    universe = fo.getUniverse()
    ret = defaultdict(lambda: _BuildingLocations(set(), set()))
    for pid in get_all_empire_planets():
        planet = universe.getPlanet(pid)
        for building in map(universe.getBuilding, planet.buildingIDs):
            val = ret[building.buildingTypeName]
            val.planets.add(pid)
            val.systems.add(planet.systemID)
    return ret
