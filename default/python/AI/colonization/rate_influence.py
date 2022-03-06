import freeOrionAIInterface as fo
from math import sqrt

from AIDependencies import SPECIES_INFLUENCE_MODIFIER, Tags
from aistate_interface import get_aistate
from EnumsAI import PriorityType
from freeorion_tools import get_species_tag_grade

INFLUENCE_SPECIALS = [
    "FRACTAL_GEODES_SPECIAL",
    "MIMETIC_ALLOY_SPECIAL",
    "SHIMMER_SILK_SPECIAL",
    "SPARK_FOSSILS_SPECIAL_DESC",
    "SUCCULENT_BARNACLES_SPECIAL",
]
# NamedReal name = "SPECIAL_INFLUENCE_FOCUS_BONUS" value = 3.0
# NamedReal name = "ARTISANS_INFLUENCE_FLAT_FOCUS" value = 4.0
SPECIAL_FLAT = 3.0
ARTISAN_FLAT = 4.0
# TBD artisan bonus for non-focused planets.
# well, first we'll have to teach the AI to use workshops at all...
PLC_ARTISAN = "PLC_ARTISAN_WORKSHOPS"
TAG_ARTISTIC = "ARTISTIC"


def rate_influence(planet, species, max_pop):
    """
    Give a rating how useful the colony would be for generating influence.
    This should be used as an alternative for industry/research.
    """
    if not species:
        return 0.0
    infl_tag_mod = SPECIES_INFLUENCE_MODIFIER.get(get_species_tag_grade(species.name, Tags.INFLUENCE), 1)
    production = sqrt(max_pop) * infl_tag_mod
    for special in planet.specials:
        if special in INFLUENCE_SPECIALS:
            production += SPECIAL_FLAT
    production *= infl_tag_mod
    if fo.getEmpire().policyAdopted(PLC_ARTISAN) and TAG_ARTISTIC in species.tags:
        production += ARTISAN_FLAT
    influence_priotity = get_aistate().get_priority(PriorityType.RESOURCE_INFLUENCE)
    # a production of 1 is hardly worth anything
    return (15 + influence_priotity / 10) * (production - 1)
