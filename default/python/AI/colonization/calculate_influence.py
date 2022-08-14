import freeOrionAIInterface as fo
from math import sqrt

from aistate_interface import get_aistate
from buildings import BuildingType
from EnumsAI import FocusType, PriorityType
from freeorion_tools import get_named_real, get_species_influence
from turn_state import get_empire_planets_by_species

INFLUENCE_SPECIALS = [
    "FRACTAL_GEODES_SPECIAL",
    "MIMETIC_ALLOY_SPECIAL",
    "SHIMMER_SILK_SPECIAL",
    "SPARK_FOSSILS_SPECIAL",
    "SUCCULENT_BARNACLES_SPECIAL",
]
SPECIAL_FLAT = get_named_real("SPECIAL_INFLUENCE_FOCUS_BONUS")
ARTISAN_FLAT = get_named_real("ARTISANS_INFLUENCE_FLAT_FOCUS")
PLC_ARTISAN = "PLC_ARTISAN_WORKSHOPS"
TAG_ARTISTIC = "ARTISTIC"


def calculate_influence(planet: fo.planet, species: fo.species, max_pop: float, stability: float) -> float:
    """
    Calculate how much influence the planet could generate with influence focus.
    """
    if not species or FocusType.FOCUS_INFLUENCE not in species.foci:
        return 0.0
    production = sqrt(max_pop)
    production += sum(SPECIAL_FLAT for special in planet.specials if special in INFLUENCE_SPECIALS)
    if fo.getEmpire().policyAdopted(PLC_ARTISAN) and TAG_ARTISTIC in species.tags:
        offset = stability - get_named_real("ARTISANS_MIN_STABILITY_FOCUS")
        production += ARTISAN_FLAT / (1.0 if offset >= 0 else offset**2 + 2)  # grant a little, if close
    # So far all those flat bonuses are affected by the species multiplier
    production *= get_species_influence(species.name)

    translator = BuildingType.TRANSLATOR
    bonus = get_named_real("TRANSLATOR_PER_SPECIES_INFLUENCE") * len(get_empire_planets_by_species()) ** 0.5
    # We may consider conquering a planet that has one. Note that translator.build_at shows only our translators
    if any(translator.is_this_type(bld_id) for bld_id in planet.buildingIDs):
        production += bonus
    elif translator.available():  # we'd have to build it first, so use only half of the bonus
        production += 0.5 * production
    # TODO Species InterDesign Academy
    return production


def rate_influence(planet, species, max_pop):  # TODO: remove with the planet rating old code...
    """
    Give a rating how useful the colony would be for generating influence.
    This should be used as an alternative for industry/research.
    """
    production = calculate_influence(planet, species, max_pop, 10.0)
    # TBD check for nearly universal translator:
    # 1. We could rate a planet that already has one
    # 2. The ability to build one could remove the -1 below
    # Other factors still missing: policies Terraforming, Indoctrination
    # and Environmentalism (most likely never used by AI).
    influence_priority = get_aistate().get_priority(PriorityType.RESOURCE_INFLUENCE)
    # a production of 1 is hardly worth anything
    return (15 + influence_priority / 10) * (production - 1)
