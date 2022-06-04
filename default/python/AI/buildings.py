import freeOrionAIInterface as fo
from collections import defaultdict
from copy import copy
from enum import Enum
from typing import DefaultDict, FrozenSet, List, Mapping, NamedTuple, Set, Tuple

from aistate_interface import get_aistate
from common.fo_typing import BuildingName, PlanetId, SystemId
from empire.buildings_locations import get_best_pilot_facilities
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
    for pid in BuildingType.SHIPYARD_ORBITAL_DRYDOCK.built_at():
        planet = universe.getPlanet(pid)
        drydocks.setdefault(planet.systemID, []).append(pid)
    return ReadOnlyDict({k: tuple(v) for k, v in drydocks.items()})


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
    TRANSLATOR = "BLD_TRANSLATOR"
    MILITARY_COMMAND = "BLD_MILITARY_COMMAND"
    SHIPYARD_BASE = "BLD_SHIPYARD_BASE"
    SHIPYARD_ORBITAL_DRYDOCK = "BLD_SHIPYARD_ORBITAL_DRYDOCK"
    SHIPYARD_CON_NANOROBO = "BLD_SHIPYARD_CON_NANOROBO"
    SHIPYARD_CON_GEOINT = "BLD_SHIPYARD_CON_GEOINT"
    SHIPYARD_CON_ADV_ENGINE = "BLD_SHIPYARD_CON_ADV_ENGINE"
    SHIPYARD_AST = "BLD_SHIPYARD_AST"
    SHIPYARD_AST_REF = "BLD_SHIPYARD_AST_REF"
    SHIPYARD_ORG_ORB_INC = "BLD_SHIPYARD_ORG_ORB_INC"
    SHIPYARD_ORG_CELL_GRO_CHAMB = "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB"
    SHIPYARD_ORG_XENO_FAC = "BLD_SHIPYARD_ORG_XENO_FAC"
    SHIPYARD_ENRG_COMP = "BLD_SHIPYARD_ENRG_COMP"
    SHIPYARD_ENRG_SOLAR = "BLD_SHIPYARD_ENRG_SOLAR"
    NEUTRONIUM_FORGE = "BLD_NEUTRONIUM_FORGE"  # not a shipyard by name, but only used for building ships

    @staticmethod
    def get(name: BuildingName):
        """Get BuildType for a given BuildingName"""
        for bt in BuildingType:
            if bt.value == name:
                return bt
        raise ValueError(f"BuildingType.get(): got unknown name {name}")

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

    def queued_in(self) -> List[PlanetId]:
        """
        Return list of planet ids where this building is queued.
        """
        return [element.locationID for element in fo.getEmpire().productionQueue if (element.name == self.value)]

    def queued_at_sys(self) -> List[PlanetId]:
        """
        Return list of system ids where this building is queued.
        """
        return [fo.getUniverse().getPlanet(pid).systemID for pid in self.queued_in()]

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

    @staticmethod
    def get_system_ship_facilities():
        return frozenset({BuildingType.SHIPYARD_AST, BuildingType.SHIPYARD_AST_REF})

    def get_best_pilot_facilities(self) -> FrozenSet[PlanetId]:
        return get_best_pilot_facilities(self.value)


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
        BuildingType.SHIPYARD_ORBITAL_DRYDOCK: BuildingType.SHIPYARD_BASE,
        BuildingType.SHIPYARD_CON_NANOROBO: BuildingType.SHIPYARD_ORBITAL_DRYDOCK,
        BuildingType.SHIPYARD_CON_GEOINT: BuildingType.SHIPYARD_ORBITAL_DRYDOCK,
        BuildingType.SHIPYARD_CON_ADV_ENGINE: BuildingType.SHIPYARD_ORBITAL_DRYDOCK,
        BuildingType.SHIPYARD_AST_REF: BuildingType.SHIPYARD_AST,
        BuildingType.SHIPYARD_ORG_ORB_INC: BuildingType.SHIPYARD_BASE,
        BuildingType.SHIPYARD_ORG_CELL_GRO_CHAMB: BuildingType.SHIPYARD_ORG_ORB_INC,
        BuildingType.SHIPYARD_ORG_XENO_FAC: BuildingType.SHIPYARD_ORG_ORB_INC,
        BuildingType.SHIPYARD_ENRG_COMP: BuildingType.SHIPYARD_BASE,
        BuildingType.SHIPYARD_ENRG_SOLAR: BuildingType.SHIPYARD_ENRG_COMP,
        # not a technical prerequisite, but it has no purpose without a shipyard
        BuildingType.NEUTRONIUM_FORGE: BuildingType.SHIPYARD_BASE,
    }
)
