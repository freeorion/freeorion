import freeOrionAIInterface as fo

import AIDependencies
from common.fo_typing import PlanetId
from EnumsAI import FocusType
from freeorion_tools.caching import cache_for_current_turn
from turn_state._planet_state import _get_planets_info


class EmpireResources:
    def __init__(self):  # noqa: C901
        self.have_gas_giant = False
        self.have_asteroids = False
        self.owned_asteroid_coatings = 0
        self.have_ruins = False
        self.have_nest = False
        self.have_computronium = False
        self.pids_computronium = []
        self.have_honeycomb = False
        self.pids_honeycomb = []
        self.have_worldtree = False
        self.num_researchers = 0  # population with research focus
        self.num_industrialists = 0  # population with industry focus
        self.luxury_planets = {}  # list of planets per luxury special

        empire_id = fo.empireID()
        universe = fo.getUniverse()
        INDUSTRY = FocusType.FOCUS_INDUSTRY
        RESEARCH = FocusType.FOCUS_RESEARCH

        for planet_info in _get_planets_info().values():
            planet = universe.getPlanet(planet_info.pid)
            if planet.ownedBy(empire_id):
                if AIDependencies.ANCIENT_RUINS_SPECIAL in planet.specials:
                    self.have_ruins = True

                if AIDependencies.WORLDTREE_SPECIAL in planet.specials:
                    self.have_world_tree = True

                if AIDependencies.ASTEROID_COATING_OWNED_SPECIAL in planet.specials:
                    self.owned_asteroid_coatings += 1

                if AIDependencies.COMPUTRONIUM_SPECIAL in planet.specials and RESEARCH in planet.availableFoci:
                    self.pids_computronium.append(planet.id)
                    if planet.focus == RESEARCH:
                        self.have_computronium = True

                if AIDependencies.HONEYCOMB_SPECIAL in planet.specials and INDUSTRY in planet.availableFoci:
                    self.pids_honeycomb.append(planet.id)
                    if planet.focus == INDUSTRY:
                        self.have_honeycomb = True

                for special in AIDependencies.luxury_specials:
                    if special in planet.specials:
                        self.luxury_planets.setdefault(special, []).append(planet)

                population = planet.currentMeterValue(fo.meterType.population)
                if planet.focus == INDUSTRY:
                    self.num_industrialists += population
                elif planet.focus == RESEARCH:
                    self.num_researchers += population


@cache_for_current_turn
def _get_planet_catalog() -> EmpireResources:
    return EmpireResources()


def population_with_industry_focus() -> int:
    return _get_planet_catalog().num_industrialists


def have_gas_giant() -> bool:
    return _get_planet_catalog().have_gas_giant


def set_have_gas_giant():
    _get_planet_catalog().have_gas_giant = True


def have_asteroids() -> bool:
    return _get_planet_catalog().have_asteroids


def owned_asteroid_coatings() -> int:
    return _get_planet_catalog().owned_asteroid_coatings


def set_have_asteroids():
    _get_planet_catalog().__have_asteroids = True


def have_ruins() -> bool:
    return _get_planet_catalog().have_ruins


def have_nest() -> bool:
    return _get_planet_catalog().have_nest


def set_have_nest():
    _get_planet_catalog().have_nest = True


def have_computronium() -> bool:
    """Return True if we have a planet with a computronium moon, which is set to research focus."""
    return _get_planet_catalog().have_computronium


def computronium_candidates() -> list[PlanetId]:
    """Return list of own planets that have a computronium moon and a species capable of research."""
    return _get_planet_catalog().pids_computronium


def have_honeycomb() -> bool:
    """Return True if we have a planet with a honeycomb special, which is set to production focus."""
    return _get_planet_catalog().have_honeycomb


def honeycomb_candidates() -> list[PlanetId]:
    """Return list of own planets that have the honeycomb special and a species capable of production."""
    return _get_planet_catalog().pids_honeycomb


def have_worldtree() -> bool:
    return _get_planet_catalog().have_worldtree


def luxury_resources() -> dict[str, list[fo.planet]]:
    return _get_planet_catalog().luxury_planets
