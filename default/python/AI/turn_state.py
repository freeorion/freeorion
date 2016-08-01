from collections import namedtuple
import freeOrionAIInterface as fo

PlanetInfo = namedtuple('PlanetInfo', ['pid', 'species', 'owner', 'system_id'])


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
        self.__best_pilot_rating = 1e-8
        self.__medium_pilot_rating = 1e-8
        self.__planets = {}

    def update(self):
        self.__init__()
        universe = fo.getUniverse()
        for pid in universe.planetIDs:
            planet = universe.getPlanet(pid)
            self.__planets[pid] = PlanetInfo(pid, planet.speciesName, planet.owner, planet.systemID)

    def get_empire_species_systems(self):
        """
        Return dict from system id to planet ids of empire with species.

        :rtype: dict[int, list[int]]
        """
        # TODO: as currently used, is duplicative with combo of foAI.foAIstate.popCtrSystemIDs and foAI.foAIstate.colonizedSystems
        empire_id = fo.empireID()
        planets_with_species = (x for x in self.__planets.itervalues() if x.owner == empire_id and x.species)
        result = {}
        for x in planets_with_species:
            result.setdefault(x.system_id, []).append(x.pid)
        return result

    def get_inhabited_planets(self):
        """
        Return set of empire planet ids with species.

        :rtype: set
        """
        empire_id = fo.empireID()
        return {x.pid for x in self.__planets.itervalues() if x.owner == empire_id and x.species}

    def get_planets_for_species(self, species):
        """
        Return list of empire planet ids with species.

        :rtype: list[int]
        """
        if not species:
            return []
        empire_id = fo.empireID()
        return [x.pid for x in self.__planets.itervalues() if x.owner == empire_id and x.species == species]

    def get_species_planets(self):
        """
        Return dict for empire from species to list of planet ids.

        :rtype: dict[str, list[int]]
        """
        empire_id = fo.empireID()
        result = {}
        for x in (x for x in self.__planets.itervalues() if x.owner == empire_id and x.species):
            result.setdefault(x.species, []).append(x.pid)
        return result

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

    def set_have_ruins(self):
        self.__have_ruins = True

    @property
    def have_nest(self):
        return self.__have_nest

    def set_have_nest(self):
        self.__have_nest = True

    @property
    def have_computronium(self):
        return self.__have_computronium

    def set_have_computronium(self):
        self.__have_computronium = True

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
