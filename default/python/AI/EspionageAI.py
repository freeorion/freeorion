from logging import error, warn

import freeOrionAIInterface as fo  # pylint: disable=import-error
import AIDependencies
from AIDependencies import ALL_EMPIRES
from EnumsAI import EmpireMeters
from freeorion_tools import cache_by_session_with_turnwise_update


def default_empire_detection_strength():
    # TODO doublecheck typical AI research times for Radar, for below default value
    return (AIDependencies.DETECTION_TECH_STRENGTHS["SPY_DETECT_1"] if fo.currentTurn() < 40
            else AIDependencies.DETECTION_TECH_STRENGTHS["SPY_DETECT_2"])


@cache_by_session_with_turnwise_update
def get_empire_detection(empire_id):
    """
    Returns the detection strength for the provided empire ID.

    If passed the ALL_EMPIRES ID, then it returns the max detection strength across
    all empires except for the current AI's empire.

    :rtype: float
    """
    if empire_id == ALL_EMPIRES:
        return get_max_empire_detection(fo.allEmpireIDs())

    empire = fo.getEmpire(empire_id)
    if empire:
        return empire.getMeter(EmpireMeters.DETECTION_STRENGTH).initial
    else:
        warn("AI failed to retrieve empire ID %d, in game with %d empires." % (empire_id, len(fo.allEmpireIDs())))
        return default_empire_detection_strength()


def get_max_empire_detection(empire_list):
    """
    Returns the max detection strength across all empires except for the current AI's empire.

    :param empire_list: list of empire IDs
    :type empire_list: list[int] | fo.IntVec
    :return: max detection strength of provided empires, excluding the current self empire
    :rtype: float
    """
    max_detection = 0
    for this_empire_id in empire_list:
        if this_empire_id != fo.empireID():
            max_detection = max(max_detection, get_empire_detection(this_empire_id))
    return max_detection


def colony_detectable_by_empire(planet_id, species_name=None, empire=ALL_EMPIRES, future_stealth_bonus=0,
                                default_result=True):
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
    :type planet_id: int
    :param species_name: will override the existing planet species if provided
    :type species_name: str
    :param empire: empire ID (or list of empire IDs) whose detection ability is of concern
    :type empire: int | list[int]
    :param future_stealth_bonus: can specify a projected future stealth bonus, such as from a stealth tech
    :type future_stealth_bonus: int
    :param default_result: generally for offensive assessments should be False, for defensive should be True
    :type default_result: bool
    :return: whether the planet is predicted to be detectable
    :rtype: bool
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

    species = fo.getSpecies(species_name)
    if species:
        species_tags = species.tags
    elif species_name == "":
        species_tags = []
    else:
        error("Couldn't retrieve species named '%s'." % species_name)
        return default_result

    # could just check stealth meter, but this approach might allow us to plan ahead a bit even if the planet
    # is temporarily stealth boosted by temporary effects like ion storm
    planet_stealth = AIDependencies.BASE_PLANET_STEALTH
    if planet.specials:
        planet_stealth += max([AIDependencies.STEALTH_SPECIAL_STRENGTHS.get(_spec, 0) for _spec in planet.specials])
    # TODO: check planet buildings for stealth bonuses

    # if the planet already has an existing stealth special, then the most common situation is that it would be
    # overlapping with or superseded by the future_stealth_bonus, not additive with it.
    planet_stealth = max(planet_stealth, AIDependencies.BASE_PLANET_STEALTH + future_stealth_bonus)

    total_stealth = planet_stealth + sum([AIDependencies.STEALTH_STRENGTHS_BY_SPECIES_TAG.get(tag, 0)
                                          for tag in species_tags])
    if isinstance(empire, int):
        empire_detection = get_empire_detection(empire)
    else:
        empire_detection = get_max_empire_detection(empire)
    return total_stealth < empire_detection
