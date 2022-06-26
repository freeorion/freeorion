import freeOrionAIInterface as fo

import AIDependencies
from EnumsAI import FocusType
from freeorion_tools.caching import cache_for_current_turn
from turn_state._planet_state import _get_planets_info


class EmpireResources:
    def __init__(self):
        self.have_gas_giant = False
        self.have_asteroids = False
        self.owned_asteroid_coatings = 0
        self.have_ruins = False
        self.have_nest = False
        self.have_computronium = False
        self.have_honeycomb = False
        self.have_worldtree = False
        self.num_researchers = 0  # population with research focus
        self.num_industrialists = 0  # population with industry focus

        empire_id = fo.empireID()
        universe = fo.getUniverse()

        for planet_info in _get_planets_info().values():
            planet = universe.getPlanet(planet_info.pid)
            if planet.ownedBy(empire_id):

                if AIDependencies.ANCIENT_RUINS_SPECIAL in planet.specials:
                    self.have_ruins = True

                if AIDependencies.WORLDTREE_SPECIAL in planet.specials:
                    self.have_world_tree = True

                if AIDependencies.ASTEROID_COATING_OWNED_SPECIAL in planet.specials:
                    self.owned_asteroid_coatings += 1

                if planet.focus == FocusType.FOCUS_RESEARCH and AIDependencies.COMPUTRONIUM_SPECIAL in planet.specials:
                    self.have_computronium = True

                if planet.focus == FocusType.FOCUS_INDUSTRY and AIDependencies.HONEYCOMB_SPECIAL in planet.specials:
                    self.have_honeycomb = True

                population = planet.currentMeterValue(fo.meterType.population)
                if planet.focus == FocusType.FOCUS_INDUSTRY:
                    self.num_industrialists += population
                elif planet.focus == FocusType.FOCUS_RESEARCH:
                    self.num_researchers += population


@cache_for_current_turn
def _get_planet_catalog() -> EmpireResources:
    return EmpireResources()


def population_with_research_focus() -> int:
    return _get_planet_catalog().num_researchers


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
    return _get_planet_catalog().have_computronium


def have_honeycomb() -> bool:
    return _get_planet_catalog().have_honeycomb


def have_worldtree() -> bool:
    return _get_planet_catalog().have_worldtree
