"""
This module encapsulate some basic buildings manipulation.
It's organized as a set of enums with various methods.
Each enum represent a set of building types with similar usage.

Keep in mind when implementing enum:
 - It encapsulate FOCS names and fo API usage exposing convenient API for AI.
 - It should not contain any decision logic, , only provide information that is easy to handle.
   For example direct manipulation like enqueue or information like if this building could be constructed.
 - It should not accept or expose any fo.* object.
 - We don't need to keep enums names the same as FOCS ids, so keep them short but recognizable.


TODO: Use building ids only inside that file, change other code to accept enums.
"""

from __future__ import annotations

import freeOrionAIInterface as fo
from collections import defaultdict
from copy import copy
from enum import Enum
from itertools import chain
from typing import DefaultDict, Iterator, List, Mapping, NamedTuple, Set, Tuple, Union

from aistate_interface import get_aistate
from common.fo_typing import BuildingId, BuildingName, PlanetId, SystemId
from freeorion_tools import ReadOnlyDict
from freeorion_tools.caching import cache_for_current_turn
from PlanetUtilsAI import Opinion, get_planet_opinion
from turn_state import get_all_empire_planets


@cache_for_current_turn
def get_empire_drydocks() -> Mapping[SystemId, Tuple[PlanetId]]:
    """
    Return a map from system ids to planet ids where empire drydocks are located.
    """
    universe = fo.getUniverse()
    drydocks = {}
    for pid in Shipyard.ORBITAL_DRYDOCK.built_at():
        planet = universe.getPlanet(pid)
        drydocks.setdefault(planet.systemID, []).append(pid)
    return ReadOnlyDict({k: tuple(v) for k, v in drydocks.items()})


class _BuildingOperations:
    """
    Mixin class for building enums.

    This class contains operations that applicable for all building enums.
    """

    def is_this_type(self, building_id: BuildingId):
        """Return whether the building with the given identifier is of this type."""
        return fo.getUniverse().getBuilding(building_id) == self.value

    def enqueue(self, pid: PlanetId) -> bool:
        """
        Add building to production queue and return True if succeeded.
        """
        return bool(fo.issueEnqueueBuildingProductionOrder(self.value, pid))

    def available(self) -> bool:
        """
        Return true if this building is available for empire and may be built by this AI.
        """
        character = get_aistate().character
        return character.may_build_building(self.value) and fo.getEmpire().buildingTypeAvailable(self.value)

    def get_opinions(self) -> Opinion:
        """
        Returns opinions about the building at the empire's planets.
        """
        return get_planet_opinion(self.value)

    def queued_at(self) -> List[PlanetId]:
        """
        Return list of planet ids where this building is queued.
        """
        return [element.locationID for element in fo.getEmpire().productionQueue if (element.name == self.value)]

    def queued_at_sys(self) -> List[PlanetId]:
        """
        Return list of system ids where this building is queued.
        """
        return [fo.getUniverse().getPlanet(pid).systemID for pid in self.queued_at()]

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
        ret.update(set(self.queued_at()))
        return ret

    def built_or_queued_at_sys(self) -> Set[SystemId]:
        """
        Return List of system ids where the building either exists or is queued.
        """
        built_at_systems = _get_building_locations()[self.value].systems
        ret = copy(built_at_systems)
        ret.update(set(self.queued_at_sys()))
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

    def prerequisite(self):  # not yet. -> Optional[BuildingType]:
        """Return another building type, that this is based on or None."""
        return _prerequisites.get(self, None)


class BuildingType(_BuildingOperations, Enum):
    """
    Represent basic buildings.

    Start adding new buildings here.
    There are 2 things why you want to move anything to separate enum.

    1) names became shorter, for example BuildingType.SHIPYARD_BASE -> Shipyard.BASE
    2) you need new method exclusively to that building/buildings.
    """

    AUTO_HISTORY_ANALYSER = "BLD_AUTO_HISTORY_ANALYSER"
    BLACK_HOLE_POW_GEN = "BLD_BLACK_HOLE_POW_GEN"
    COLLECTIVE_NET = "BLD_COLLECTIVE_NET"
    CULTURE_ARCHIVES = "BLD_CULTURE_ARCHIVES"
    CULTURE_LIBRARY = "BLD_CULTURE_LIBRARY"
    ENCLAVE_VOID = "BLD_ENCLAVE_VOID"
    GAS_GIANT_GEN = "BLD_GAS_GIANT_GEN"
    GENOME_BANK = "BLD_GENOME_BANK"
    INDUSTRY_CENTER = "BLD_INDUSTRY_CENTER"
    INTERSPECIES_ACADEMY = "BLD_INTERSPECIES_ACADEMY"
    LIGHTHOUSE = "BLD_LIGHTHOUSE"
    MEGALITH = "BLD_MEGALITH"
    MILITARY_COMMAND = "BLD_MILITARY_COMMAND"
    NEUTRONIUM_SYNTH = "BLD_NEUTRONIUM_SYNTH"
    NEUTRONIUM_EXTRACTOR = "BLD_NEUTRONIUM_EXTRACTOR"
    PALACE = "BLD_IMPERIAL_PALACE"
    REGIONAL_ADMIN = "BLD_REGIONAL_ADMIN"
    SCANNING_FACILITY = "BLD_SCANNING_FACILITY"
    SCRYING_SPHERE = "BLD_SCRYING_SPHERE"
    SOL_ORB_GEN = "BLD_SOL_ORB_GEN"
    SPACE_ELEVATOR = "BLD_SPACE_ELEVATOR"
    SPATIAL_DISTORT_GEN = "BLD_SPATIAL_DISTORT_GEN"
    STARGATE = "BLD_STARGATE"
    STOCKPILING_CENTER = "BLD_STOCKPILING_CENTER"
    TRANSLATOR = "BLD_TRANSLATOR"
    XENORESURRECTION_LAB = "BLD_XENORESURRECTION_LAB"


class Shipyard(_BuildingOperations, Enum):
    """
    Represent buildings required to build ships.
    """

    BASE = "BLD_SHIPYARD_BASE"
    ORBITAL_DRYDOCK = "BLD_SHIPYARD_ORBITAL_DRYDOCK"
    NANOROBO = "BLD_SHIPYARD_CON_NANOROBO"
    GEO = "BLD_SHIPYARD_CON_GEOINT"
    ADV_ENGINE = "BLD_SHIPYARD_CON_ADV_ENGINE"
    ASTEROID = "BLD_SHIPYARD_AST"
    ASTEROID_REF = "BLD_SHIPYARD_AST_REF"
    ORG_ORB_INC = "BLD_SHIPYARD_ORG_ORB_INC"
    ORG_CELL_GRO_CHAMB = "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB"
    XENO_FACILITY = "BLD_SHIPYARD_ORG_XENO_FAC"
    ENRG_COMP = "BLD_SHIPYARD_ENRG_COMP"
    ENRG_SOLAR = "BLD_SHIPYARD_ENRG_SOLAR"
    NEUTRONIUM_FORGE = "BLD_NEUTRONIUM_FORGE"  # not a shipyard by name, but only used for building ships

    @staticmethod
    def get_system_ship_facilities():
        return frozenset({Shipyard.ASTEROID, Shipyard.ASTEROID_REF})


def iterate_buildings_types() -> Iterator[Union[BuildingType, Shipyard]]:
    """
    Iterator over all building types.
    """
    yield from chain(BuildingType, Shipyard)


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


_prerequisites = ReadOnlyDict(
    {
        Shipyard.ORBITAL_DRYDOCK: Shipyard.BASE,
        Shipyard.NANOROBO: Shipyard.ORBITAL_DRYDOCK,
        Shipyard.GEO: Shipyard.ORBITAL_DRYDOCK,
        Shipyard.ADV_ENGINE: Shipyard.ORBITAL_DRYDOCK,
        Shipyard.ASTEROID_REF: Shipyard.ASTEROID,
        Shipyard.ORG_ORB_INC: Shipyard.BASE,
        Shipyard.ORG_CELL_GRO_CHAMB: Shipyard.ORG_ORB_INC,
        Shipyard.XENO_FACILITY: Shipyard.ORG_ORB_INC,
        Shipyard.ENRG_COMP: Shipyard.BASE,
        Shipyard.ENRG_SOLAR: Shipyard.ENRG_COMP,
        # not a technical prerequisite, but it has no purpose without a shipyard
        Shipyard.NEUTRONIUM_FORGE: Shipyard.BASE,
    }
)
