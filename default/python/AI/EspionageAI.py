import freeOrionAIInterface as fo
from logging import error, warning
from typing import List, Optional, Union

import AIDependencies
from AIDependencies import ALL_EMPIRES
from EnumsAI import EmpireMeters
from freeorion_tools import get_species_stealth
from freeorion_tools.caching import cache_for_current_turn


def default_empire_detection_strength():
    # TODO doublecheck typical AI research times for Radar, for below default value
    return (
        AIDependencies.DETECTION_TECH_STRENGTHS["SPY_DETECT_1"]
        if fo.currentTurn() < 40
        else AIDependencies.DETECTION_TECH_STRENGTHS["SPY_DETECT_2"]
    )


@cache_for_current_turn
def get_empire_detection(empire_id: int) -> float:
    """
    Returns the detection strength for the provided empire ID.

    If passed the ALL_EMPIRES ID, then it returns the max detection strength across
    all empires except for the current AI's empire.
    """
    if empire_id == ALL_EMPIRES:
        return get_max_empire_detection(fo.allEmpireIDs())

    empire = fo.getEmpire(empire_id)
    if empire:
        return empire.getMeter(EmpireMeters.DETECTION_STRENGTH).initial
    else:
        warning("AI failed to retrieve empire ID %d, in game with %d empires." % (empire_id, len(fo.allEmpireIDs())))
        return default_empire_detection_strength()


def get_max_empire_detection(empire_list: Union[List[int], "fo.IntVec"]) -> float:
    """
    Returns the max detection strength across all empires except for the current AI's empire.

    :param empire_list: list of empire IDs
    :return: max detection strength of provided empires, excluding the current self empire
    """
    max_detection = 0
    for this_empire_id in empire_list:
        if this_empire_id != fo.empireID():
            max_detection = max(max_detection, get_empire_detection(this_empire_id))
    return max_detection


def colony_detectable_by_empire(
    planet_id: int,
    species_name: Optional[str] = None,
    empire: Union[int, List[int]] = ALL_EMPIRES,
    future_stealth_bonus: int = 0,
    default_result: bool = True,
) -> bool:
    """
    Predicts if a planet/colony is/will-be detectable by an empire.

    The passed empire value can be a single empire ID or a list of empire IDs.  If passed ALL_empires, will use the list
    of all empires.  When using a list of empires (or when passed ALL_EMPIRES), the present empire is excluded.  To
    check for the present empire the current empire ID must be passed as a simple int.  Ignores current ownership of the
    planet unless the passed empire value is a simple int matching fo.empireID(), because in most cases we are concerned
    about (i) visibility to us of a planet we do not own, or (ii) visibility by enemies of a planet we own or expect to
    own at the time we are making the visibility projection for (even if they may own it right now).  The only case
    where current ownership matters is when we are considering whether to abort a colonization mission, which might be
    for a planet we already own.

    :param planet_id: required, the planet of concern
    :param species_name: will override the existing planet species if provided
    :param empire: empire ID (or list of empire IDs) whose detection ability is of concern
    :param future_stealth_bonus: can specify a projected future stealth bonus, such as from a stealth tech
    :param default_result: generally for offensive assessments should be False, for defensive should be True
    :return: whether the planet is predicted to be detectable

    """
    # The future_stealth_bonus can be used if the AI knows it has researched techs that would grant a stealth bonus to
    # the planet once it was colonized/captured

    planet = fo.getUniverse().getPlanet(planet_id)
    if not planet:
        error("Couldn't retrieve planet ID %d." % planet_id)
        return default_result
    if species_name is None:
        species_name = planet.speciesName

    # in case we are checking about aborting a colonization mission
    if empire == fo.empireID() and planet.ownedBy(empire):
        return True

    # could just check stealth meter, but this approach might allow us to plan ahead a bit even if the planet
    # is temporarily stealth boosted by temporary effects like ion storm
    planet_stealth = AIDependencies.BASE_PLANET_STEALTH
    if planet.specials:
        planet_stealth += max([AIDependencies.STEALTH_SPECIAL_STRENGTHS.get(_spec, 0) for _spec in planet.specials])
    # TODO: check planet buildings for stealth bonuses

    # if the planet already has an existing stealth special, then the most common situation is that it would be
    # overlapping with or superseded by the future_stealth_bonus, not additive with it.
    planet_stealth = max(planet_stealth, AIDependencies.BASE_PLANET_STEALTH + future_stealth_bonus)
    species_stealth_mod = get_species_stealth(species_name)
    total_stealth = planet_stealth + species_stealth_mod
    if isinstance(empire, int):
        empire_detection = get_empire_detection(empire)
    else:
        empire_detection = get_max_empire_detection(empire)
    return total_stealth < empire_detection
