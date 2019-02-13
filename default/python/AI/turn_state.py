from collections import namedtuple
from logging import warn

from freeorion_tools import ReadOnlyDict

import freeOrionAIInterface as fo

import AIDependencies
from AIDependencies import INVALID_ID
from EnumsAI import FocusType

PlanetInfo = namedtuple('PlanetInfo', ['pid', 'species_name', 'owner', 'system_id'])


class State(object):
    """
    This class represent state for current turn.
    It contains variables that renewed each turn.
    Before each turn `update` method should be called at the beginning of each turn.

    Each variable should have a getter that returns value or deepcopy of value if it is immutable
    and one or couple setters (if need to change inner collections in collection).

    This behaviour is similar for `freeorion_tools` but all to change state from external location.
    """

    def __init__(self):
        self.__have_gas_giant = False
        self.__have_asteroids = False
        self.__have_ruins = False
        self.__have_nest = False
        self.__have_computronium = False
        self.__have_panopticon = False
        self.__best_pilot_rating = 1e-8
        self.__medium_pilot_rating = 1e-8
        self.__planet_info = {}  # map from planet_id to PlanetInfo
        self.__num_researchers = 0  # population with research focus
        self.__num_industrialists = 0  # population with industry focus

        # supply info - negative values indicate jumps away from supply
        self.__system_supply = {}  # map from system_id to supply
        self.__systems_by_jumps_to_supply = {}  # map from supply to list of system_ids
        self.__empire_planets_by_system = {}

        # building info
        self.__drydock_locations = ReadOnlyDict()  # map from system id to planet id where empire has a drydock

    def update(self):
        """
        Must be called at each turn (before first use) to update inner state.
        """
        self.__init__()
        self.__update_planets()
        self.__update_supply()
        self.__update_buildings()

    def __update_planets(self):
        """
        Update information about planets.
        """
        universe = fo.getUniverse()
        empire_id = fo.empireID()
        for pid in universe.planetIDs:
            planet = universe.getPlanet(pid)
            self.__planet_info[pid] = PlanetInfo(pid, planet.speciesName, planet.owner, planet.systemID)

            if planet.ownedBy(empire_id):
                population = planet.currentMeterValue(fo.meterType.population)
                if AIDependencies.ANCIENT_RUINS_SPECIAL in planet.specials:
                    self.__have_ruins = True
                if AIDependencies.PANOPTICON_SPECIAL in planet.specials:
                    self.__have_panopticon = True
                if population > 0 and AIDependencies.COMPUTRONIUM_SPECIAL in planet.specials:
                        self.__have_computronium = True  # TODO: Check if species can set research focus

                if planet.focus == FocusType.FOCUS_INDUSTRY:
                    self.__num_industrialists += population
                elif planet.focus == FocusType.FOCUS_RESEARCH:
                    self.__num_researchers += population

    def __update_buildings(self):
        universe = fo.getUniverse()
        empire_id = fo.empireID()
        drydocks = {}
        for building_id in universe.buildingIDs:
            building = universe.getBuilding(building_id)
            if not building:
                continue
            if building.buildingTypeName == AIDependencies.BLD_SHIPYARD_ORBITAL_DRYDOCK and building.ownedBy(empire_id):
                drydocks.setdefault(building.systemID, []).append(building.planetID)
        self.__drydock_locations = ReadOnlyDict({k: tuple(v) for k, v in drydocks.iteritems()})

    def __update_supply(self):
        """
        Update information about supply.
        """
        self.__system_supply.update(fo.getEmpire().supplyProjections())
        for sys_id, supply_val in self.__system_supply.iteritems():
            self.__systems_by_jumps_to_supply.setdefault(min(0, supply_val), []).append(sys_id)

        # By converting the lists to immutable tuples now, we don't have to return copies when queried.
        for key in self.__systems_by_jumps_to_supply:
            self.__systems_by_jumps_to_supply[key] = tuple(self.__systems_by_jumps_to_supply[key])

    def get_system_supply(self, sys_id):
        """Get the supply level of a system.

        Negative values indicate jumps away from supply.

        :type sys_id: int
        :return: Supply value of a system or -99 if system is not connected
        :rtype: int
        """
        retval = self.__system_supply.get(sys_id, None)
        if retval is None:
            # This is only expected to happen if a system has no path to any supplied system.
            # As the current code should not allow such queries, this is logged as warning.
            # If future code breaks this assumption, feel free to adjust logging.
            warn("Queried supply value of a system not mapped in empire.supplyProjections(): %d" % sys_id)
            return -99  # pretend it is very far away from supply
        return retval

    def get_systems_by_supply_tier(self, supply_tier):
        """Get systems with supply tier.

        The current implementation does not distinguish between positive supply levels and caps at 0.
        Negative values indicate jumps away from supply.

        :type supply_tier: int
        :return: system_ids in specified supply tier
        :rtype: tuple[int]
        """
        if supply_tier > 0:
            warn("The current implementation does not distinguish between positive supply levels. "
                 "Interpreting the query as supply_tier=0 (indicating system in supply).")
            supply_tier = 0
        return self.__systems_by_jumps_to_supply.get(supply_tier, tuple())

    def get_empire_planets_by_system(self, sys_id=None, include_outposts=True):
        """
        Return dict from system id to planet ids of empire with species.

        :rtype: ReadOnlyDict[int, list[int]]
        """
        # TODO: as currently used, is duplicative with combo of get_aistate().popCtrSystemIDs
        if include_outposts not in self.__empire_planets_by_system:
            empire_id = fo.empireID()
            empire_planets = (x for x in self.__planet_info.itervalues()
                              if x.owner == empire_id and (x.species_name or include_outposts))
            result = {}
            for x in empire_planets:
                result.setdefault(x.system_id, []).append(x.pid)
            self.__empire_planets_by_system[include_outposts] = ReadOnlyDict(
                    {k: tuple(v) for k, v in result.iteritems()}
            )
        if sys_id is not None:
            return self.__empire_planets_by_system[include_outposts].get(sys_id, tuple())
        return self.__empire_planets_by_system[include_outposts]

    def get_inhabited_planets(self):
        """
        Return frozenset of empire planet ids with species.

        :rtype: frozenset[int]
        """
        empire_id = fo.empireID()
        return frozenset(x.pid for x in self.__planet_info.itervalues() if x.owner == empire_id and x.species_name)

    def get_empire_outposts(self):
        empire_id = fo.empireID()
        return tuple(x.pid for x in self.__planet_info.itervalues() if x.owner == empire_id and not x.species_name)

    def get_all_empire_planets(self):
        empire_id = fo.empireID()
        return tuple(x.pid for x in self.__planet_info.itervalues() if x.owner == empire_id)

    def get_empire_planets_with_species(self, species_name):
        """
        Return tuple of empire planet ids with species.

        :param species_name: species name
        :type species_name: str
        :rtype: tuple[int]
        """
        if not species_name:
            return ()
        return tuple(self.get_empire_planets_by_species().get(species_name, []))

    def get_empire_planets_by_species(self):
        """
        Return dict for empire from species to list of planet ids.

        :rtype: dict[str, list[int]]
        """
        empire_id = fo.empireID()
        result = {}
        for x in (x for x in self.__planet_info.itervalues() if x.owner == empire_id and x.species_name):
            result.setdefault(x.species_name, []).append(x.pid)
        return result

    def get_unowned_empty_planets(self):
        """Return the set of planets that are not owned by any player and have no natives.

        :rtype: set[int]
        """
        return {x.pid for x in self.__planet_info.itervalues() if x.owner == INVALID_ID and not x.species_name}

    def get_number_of_colonies(self):
        return len(self.get_inhabited_planets())

    def population_with_research_focus(self):
        return self.__num_researchers

    def population_with_industry_focus(self):
        return self.__num_industrialists

    def get_empire_drydocks(self):
        """Return a map from system ids to planet ids where empire drydocks are located"""
        return self.__drydock_locations

    @property
    def have_gas_giant(self):
        return self.__have_gas_giant

    def set_have_gas_giant(self):
        self.__have_gas_giant = True

    @property
    def have_asteroids(self):
        return self.__have_asteroids

    def set_have_asteroids(self):
        self.__have_asteroids = True

    @property
    def have_ruins(self):
        return self.__have_ruins

    @property
    def have_panopticon(self):
        return self.__have_panopticon

    @property
    def have_nest(self):
        return self.__have_nest

    def set_have_nest(self):
        self.__have_nest = True

    @property
    def have_computronium(self):
        return self.__have_computronium

    @property
    def best_pilot_rating(self):
        return self.__best_pilot_rating

    def set_best_pilot_rating(self, value):
        self.__best_pilot_rating = value

    @property
    def medium_pilot_rating(self):
        return self.__medium_pilot_rating

    def set_medium_pilot_rating(self, value):
        self.__medium_pilot_rating = value


state = State()
