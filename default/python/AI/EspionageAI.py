from logging import error

import freeOrionAIInterface as fo  # pylint: disable=import-error
import AIDependencies
from AIDependencies import ALL_EMPIRES


def get_empire_detection(empire_id):
    # TODO doublecheck typical AI research times for Radar, for below default value
    empire_detection = 10 if fo.currentTurn() < 40 else 30
    empire = None
    if empire_id != ALL_EMPIRES:
        empire = fo.getEmpire(empire_id)
    if empire:
        # TODO: expose Empire.GetMeter to automatically take into account detection specials
        for techname in sorted(AIDependencies.DETECTION_TECH_STRENGTHS, reverse=True):
            if empire.techResearched(techname):
                empire_detection = AIDependencies.DETECTION_TECH_STRENGTHS[techname]
                break
    return empire_detection


def colony_detectable_by_empire(planet_id=None, species_name=None, species_tags=None, empire_id=ALL_EMPIRES,
                                future_stealth_bonus=0):
    # The future_stealth_bonus can be used if the AI knows it has researched techs that would grant a stealth bonus to
    # the planet once it was colonized/captured

    empire_detection = get_empire_detection(empire_id)
    planet_stealth = AIDependencies.BASE_PLANET_STEALTH
    if planet_id is not None:
        planet = fo.getUniverse().getPlanet(planet_id)
        if planet:
            species_name = planet.speciesName
            # could just check stealth meter, but this approach might allow us to plan ahead a bit even if the planet
            # is temporarily stealth boosted by temporary effects like ion storm
            planet_stealth = max([AIDependencies.BASE_PLANET_STEALTH] +
                                 [AIDependencies.STEALTH_SPECIAL_STRENGTHS.get(_spec, 0) for _spec in planet.specials])
        else:
            error("Couldn't retrieve planet ID %d." % planet_id)
    planet_stealth = max(planet_stealth, AIDependencies.BASE_PLANET_STEALTH + future_stealth_bonus)
    if species_name is not None:
        species = fo.getSpecies(species_name)
        if species:
            species_tags = species.tags
        elif species_name == "":
            species_tags = []
        else:
            error("Couldn't retrieve species named '%s'." % species_name)
            return False
    if species_tags is None:
        species_tags = []
    total_stealth = planet_stealth + sum([AIDependencies.STEALTH_STRENGTHS_BY_SPECIES_TAG.get(tag, 0) for tag in species_tags])
    return total_stealth < empire_detection
