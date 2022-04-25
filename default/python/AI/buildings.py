import freeOrionAIInterface as fo
from collections import defaultdict
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
    SCANNING_FACILITY = "BLD_SCANNING_FACILITY"
    SHIPYARD_ENRG_COMP = "BLD_SHIPYARD_ENRG_COMP"
    SHIPYARD_AST = "BLD_SHIPYARD_AST"
    TRANSLATOR = "BLD_TRANSLATOR"

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
        return get_planet_opinion(self.value)

    def queued_in(self) -> List[PlanetId]:
        """
        Return list of planet ids where this building is queued.
        """
        return [element.locationID for element in fo.getEmpire().productionQueue if (element.name == self.value)]

    def built_at(self, include_queued_buildings: bool = False) -> Set[PlanetId]:
        ret = _get_building_locations()[self.value].planets
        if include_queued_buildings:
            ret.update(set(self.queued_in()))
        return ret

    def built_at_sys(self, include_queued_buildings: bool = False) -> Set[SystemId]:
        ret = _get_building_locations()[self.value].systems
        if include_queued_buildings:
            universe = fo.getUniverse()
            ret.update(set(universe.getPlanet(pid).systemID for pid in self.queued_in()))
        return ret

    def can_be_enqueued(self, planet: PlanetId) -> bool:
        return fo.getBuildingType(self.value).canBeEnqueued(fo.empireID(), planet)

    def can_be_produced(self, planet: PlanetId) -> bool:
        return fo.getBuildingType(self.value).canBeProduced(fo.empireID(), planet)

    def production_cost(self, planet: PlanetId) -> float:
        return fo.getBuildingType(self.value).productionCost(fo.empireID(), planet)

    def production_time(self, planet: PlanetId) -> int:
        return fo.getBuildingType(self.value).productionTime(fo.empireID(), planet)

    def turn_cost(self, planet: PlanetId) -> float:
        return self.production_cost(planet) / self.production_time(planet)


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
        for name in map(universe.getBuilding, planet.buildingIDs):
            val = ret.get(name, _BuildingLocations(set(), set()))
            val.planets.add(pid)
            val.systems.add(planet.systemID)
    return ret
