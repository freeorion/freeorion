import freeOrionAIInterface as fo
from logging import debug

from AIDependencies import Tags
from buildings import Shipyard
from colonization.claimed_stars import has_claimed_star
from colonization.colony_score import MINIMUM_COLONY_SCORE, debug_rating
from common.fo_typing import SpeciesName
from empire.pilot_rating import best_pilot_rating, medium_pilot_rating
from freeorion_tools import (
    get_species_attack_troops,
    get_species_fuel,
    get_species_ship_shields,
    get_species_tag_grade,
    tech_is_complete,
    tech_soon_available,
)
from freeorion_tools.caching import cache_for_session

GOOD_PILOT_RATING_OLD = 4.0
GREAT_PILOT_RATING_OLD = 6.0
ULT_PILOT_RATING_OLD = 12.0

BAD_PILOT_RATING = 0.4
GOOD_PILOT_RATING = 2.0
GREAT_PILOT_RATING = 3.0
ULT_PILOT_RATING = 4.0
# shield vales: -0.5 to 1.5, so rating effect is -0.3 to 0.9
# worth of shields is not easy to compare with weapons, but ultimate shields should be close to one weapon level
SHIELD_SCALING = 0.6
DETECTION_SCALING = 0.08  # detection values: -1 to 3, so rating effect is -0.08 to 0.32
FUEL_SCALING = 0.1  # fuel values: -0.5 to 1.5, so rating effect is -0.05 to 0.15
# troop values range from 0.5 to 2 (or 3, but there is currently no species with ultimate troops)
# TROOP_FACTOR is not used by rate_piloting, only in rate_colony_for_pilots
TROOP_SCALING = 0.15

_pilot_tags_rating = {
    "BAD": BAD_PILOT_RATING,
    "GOOD": GOOD_PILOT_RATING,
    "GREAT": GREAT_PILOT_RATING,
    "ULTIMATE": ULT_PILOT_RATING,
}

_detection_tags_rating = {
    "BAD": -1,
    "GOOD": 1,
    "GREAT": 2,
    "ULTIMATE": 3,
}


@cache_for_session
def rate_piloting(species_name: SpeciesName) -> float:
    """
    Rate species as pilots.
    Weapon skill is the most important factor, shields are also a major factor.
    Does also include small modifications for fuel and detections skills and even disliking shipyards.
    """
    # TODO rate for different purposes, e.g. when building ships, dislikes should not be considered,
    #  when building scouts, vision is very important, etc.
    species = fo.getSpecies(species_name)
    if not species or not species.canProduceShips:
        return 0.0
    weapon_grade_tag = get_species_tag_grade(species_name, Tags.WEAPONS)
    weapon_val = _pilot_tags_rating.get(weapon_grade_tag, 1.0)
    shield_value = SHIELD_SCALING * get_species_ship_shields(species_name)
    detection_val = detection_value(species_name) * DETECTION_SCALING
    # TODO add something for Sly and Laenfa ability to regenerate fuel quickly in certain systems?
    fuel_val = get_species_fuel(species_name) * FUEL_SCALING
    dislikes = sum(1 for bld in Shipyard if bld.value in species.dislikes)
    # basic shipyard is specifically important, cannot build any ships without it
    discount = 0.93 if "BLD_SHIPYARD_BASE" in species.dislikes else 0.96
    result = (weapon_val + shield_value + detection_val + fuel_val) * discount**dislikes
    debug(
        f"rate_piloting {species_name}: {result:.2f} (w={weapon_val}, s={shield_value}, d={detection_val}, "
        f"f={fuel_val}, dislike-discount={discount}, dislikes={dislikes})"
    )
    return result


@cache_for_session
def detection_value(species_name: SpeciesName) -> int:
    """
    Give a value for the species detection skill as a number.
    BAD gives -1, AVERAGE gives 0, GOOD gives 1, etc.
    """
    detection_grade_tag = get_species_tag_grade(species_name, Tags.DETECTION)
    return _detection_tags_rating.get(detection_grade_tag, 0)


def rate_planetary_piloting(pid: int) -> float:
    universe = fo.getUniverse()
    planet = universe.getPlanet(pid)
    if not planet:
        return 0.0
    return rate_piloting(planet.speciesName)


def _check_star_for_energy_hulls(
    star_type: fo.starType, energy_hull: bool, solar_hull: bool, artificial_black_holes: bool
) -> float:
    has_blackhole = has_claimed_star(fo.starType.blackHole)
    has_blue = has_claimed_star(fo.starType.blue) or has_blackhole
    bh_val = (1.5 if solar_hull else 1.0) * (0.6 if has_blackhole else 1.0)
    if energy_hull:
        if star_type == fo.starType.blackHole:
            return bh_val
        if star_type == fo.starType.blue:
            return 0.5 if has_blue else 0.8  # no solar hulls
        if star_type == fo.starType.white:
            return 0.2 if has_blue else 0.3  # neither quantum nor solar
        if star_type == fo.starType.red and artificial_black_holes:
            return bh_val * 0.75
    return 0.0


def _check_system_for_asteroid_hulls(system: fo.system, asteroid_hull: bool) -> float:
    result = 0.0
    universe = fo.getUniverse()
    for pid in system.planetIDs:
        planet = universe.getPlanet(pid)
        if planet and planet.type == fo.planetType.asteroids:
            if planet.owner == fo.empireID():
                result = 1.0
            elif planet.unowned:
                result = max(result, 0.5)
    return result


def rate_colony_for_pilots(planet: fo.planet, species: fo.species, details: list) -> float:
    if not species or not species.canProduceShips:
        return 0.0

    pilot_rating = rate_piloting(species.name)
    best_pilots = best_pilot_rating()
    medium_pilots = medium_pilot_rating()
    # TODO: determine current best troopers, if we have only bad troops, even conquering a planet with
    #  average troops is helpful.
    troop_value = max(get_species_attack_troops(species.name) - 1.0, 0.0)
    pilot_value = 2 * pilot_rating - best_pilots - medium_pilots
    if pilot_value < 0.0:
        if troop_value > 0.0:
            # No good battleships, but good troopers are worth something
            result = MINIMUM_COLONY_SCORE * 0.1 * troop_value
            details.append(f"pilot_value: {result}")
            debug_rating(f"rate_colony_for_pilots result={result} from troops={troop_value:.1f}")
            return result
        return 0.0

    if pilot_value > best_pilots:
        pilot_value += pilot_value - best_pilots
    pilot_value += TROOP_SCALING * troop_value

    # No need for future check, basic tech only allows energy scouts
    energy_hull = tech_is_complete("SHP_FRC_ENRG_COMP")
    solar_hull = tech_soon_available("SHP_SOLAR_CONT", 2)
    artificial_black_holes = tech_soon_available("LRN_ART_BLACK_HOLE", 2)
    universe = fo.getUniverse()
    system = universe.getSystem(planet.systemID)
    energy_hull_value = _check_star_for_energy_hulls(system.starType, energy_hull, solar_hull, artificial_black_holes)

    asteroid_hull = tech_soon_available("SHP_ASTEROID_HULLS", 4)
    asteroid_hull_value = _check_system_for_asteroid_hulls(system, asteroid_hull)

    planet_value = energy_hull_value + asteroid_hull_value + 1
    result = planet_value * pilot_value * MINIMUM_COLONY_SCORE * 0.5
    details.append(f"pilot_value: {result}")
    debug_rating(
        f"rate_colony_for_pilots result={result}: pilot_rating={pilot_value:.2f} best/med={best_pilots:.2f}/"
        f"{medium_pilots:.2f}, troops={troop_value:.1f}, energy_hull={energy_hull}, asteroid_hull={asteroid_hull}"
    )
    return result
