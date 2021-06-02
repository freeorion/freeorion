import freeOrionAIInterface as fo  # pylint: disable=import-error
from AIDependencies import (
    Tags,
)

from freeorion_tools.caching import cache_for_session
from freeorion_tools import get_species_tag_grade

GOOD_PILOT_RATING = 4.0
GREAT_PILOT_RATING = 6.0
ULT_PILOT_RATING = 12.0

_pilot_tags_rating = {
        'NO': 1e-8,
        'BAD': 0.75,
        'GOOD': GOOD_PILOT_RATING,
        'GREAT': GREAT_PILOT_RATING,
        'ULTIMATE': ULT_PILOT_RATING
    }


@cache_for_session
def rate_piloting_tag(species_name: str) -> float:
    weapon_grade_tag = get_species_tag_grade(species_name, Tags.WEAPONS)
    return _pilot_tags_rating.get(weapon_grade_tag, 1.0)


def rate_planetary_piloting(pid: int) -> float:
    universe = fo.getUniverse()
    planet = universe.getPlanet(pid)
    if not planet:
        return 0.0
    return rate_piloting_tag(planet.speciesName)
