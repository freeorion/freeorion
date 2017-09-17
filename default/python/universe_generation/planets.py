import random
import sys

import freeorion as fo

import universe_tables as tables
import util


# tuple of all valid planet sizes (with "no world")
planet_sizes_all = (fo.planetSize.tiny, fo.planetSize.small, fo.planetSize.medium, fo.planetSize.large,
                    fo.planetSize.huge, fo.planetSize.asteroids, fo.planetSize.gasGiant, fo.planetSize.noWorld)

# tuple of planet sizes without "no world"
planet_sizes = (fo.planetSize.tiny, fo.planetSize.small, fo.planetSize.medium, fo.planetSize.large,
                fo.planetSize.huge, fo.planetSize.asteroids, fo.planetSize.gasGiant)

# tuple of "real" planet sizes (without "no world", asteroids and gas giants)
planet_sizes_real = (fo.planetSize.tiny, fo.planetSize.small, fo.planetSize.medium,
                     fo.planetSize.large, fo.planetSize.huge)

# tuple of all available planet types (with asteroids and gas giants)
planet_types = (fo.planetType.swamp, fo.planetType.radiated, fo.planetType.toxic, fo.planetType.inferno,
                fo.planetType.barren, fo.planetType.tundra, fo.planetType.desert, fo.planetType.terran,
                fo.planetType.ocean, fo.planetType.asteroids, fo.planetType.gasGiant)

# tuple of available planet types without asteroids and gas giants ("real" planets)
planet_types_real = (fo.planetType.swamp, fo.planetType.radiated, fo.planetType.toxic, fo.planetType.inferno,
                     fo.planetType.barren, fo.planetType.tundra, fo.planetType.desert, fo.planetType.terran,
                     fo.planetType.ocean)


def base_chance_of_planet(planet_density, galaxy_shape, star_type, orbit, planet_size):
    return (tables.DENSITY_MOD_TO_PLANET_SIZE_DIST[planet_density][planet_size]
            + tables.STAR_TYPE_MOD_TO_PLANET_SIZE_DIST[star_type][planet_size]
            + tables.ORBIT_MOD_TO_PLANET_SIZE_DIST[orbit][planet_size]
            + tables.GALAXY_SHAPE_MOD_TO_PLANET_SIZE_DIST[galaxy_shape][planet_size])


def can_have_planets(star_type, orbits, planet_density, galaxy_shape):
    """
    Return True if a system can have any planets.
    """
    for orbit in orbits:
        base_chance_none = base_chance_of_planet(planet_density, galaxy_shape, star_type, orbit, fo.planetSize.noWorld)
        base_chance_any = max([base_chance_of_planet(planet_density, galaxy_shape, star_type, orbit, size)
                               for size in planet_sizes])

        if base_chance_any > base_chance_none - 100:
            return True
    return False


def calc_planet_size(star_type, orbit, planet_density, galaxy_shape):
    """
    Calculate planet size for a potential new planet based on planet density setup option, star type and orbit number.
    """
    # try to pick a planet size by making a series of "rolls" (1-100)
    # for each planet size, and take the highest modified roll
    planet_size = fo.planetSize.unknown
    try:
        max_roll = 0
        for size in planet_sizes_all:
            roll = (random.randint(1, 100)
                    + base_chance_of_planet(planet_density, galaxy_shape, star_type, orbit, size))
            if max_roll < roll:
                max_roll = roll
                planet_size = size
    except:
        # in case of an error play it safe and set planet size to invalid
        planet_size = fo.planetSize.unknown
        util.report_error("Python calc_planet_size: Pick planet size failed" + str(sys.exc_info()[1]))

    # if we got an invalid planet size (for whatever reason),
    # just select one randomly from the global tuple based
    # only on the planet density setup option
    if planet_size == fo.planetSize.unknown:
        if random.randint(1, 10) <= planet_density:
            planet_size = random.choice(planet_sizes)
        else:
            planet_size = fo.planetSize.noWorld

    return planet_size


def calc_planet_type(star_type, orbit, planet_size):
    """
    Calculate planet type randomly for a potential new planet.

    TODO: take into account star type and orbit number for determining planet type.
    """
    # check specified planet size to determine if we want a planet at all
    if planet_size in planet_sizes:
        # if yes, determine planet type based on planet size...
        if planet_size == fo.planetSize.gasGiant:
            return fo.planetType.gasGiant
        elif planet_size == fo.planetSize.asteroids:
            return fo.planetType.asteroids
        else:
            return random.choice(planet_types_real)
    else:
        return fo.planetType.unknown


def generate_a_planet(system, star_type, orbit, planet_density, galaxy_shape):
    """
    Place a planet in an orbit of a system. Return True on success
    """
    planet_size = calc_planet_size(star_type, orbit, planet_density, galaxy_shape)
    if planet_size not in planet_sizes:
        return False
    # ok, we want a planet, determine planet type and generate the planet
    planet_type = calc_planet_type(star_type, orbit, planet_size)
    if planet_type == fo.planetType.unknown:
        return False
    if fo.create_planet(planet_size, planet_type, system, orbit, "") == fo.invalid_object():
        # create planet failed, report an error
        util.report_error("Python generate_systems: create planet in system %d failed" % system)
        return False
    return True
