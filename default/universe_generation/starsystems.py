import sys
import random

import foUniverseGenerator as fo

import planets


# tuple of available star types
star_types = (fo.starType.blue, fo.starType.white,   fo.starType.yellow,    fo.starType.orange,
              fo.starType.red,  fo.starType.neutron, fo.starType.blackHole, fo.starType.noStar)

# tuple of "real" star types (that is, blue to red, not neutron, black hole or even no star)
star_types_real = (fo.starType.blue,   fo.starType.white, fo.starType.yellow,
                   fo.starType.orange, fo.starType.red)


def pick_star_type(galaxy_age):
    """
    Picks and returns a star type based on universe tables distribution modifiers
    """

    # try to pick a star type by making a series of "rolls" (1-100)
    # for each star type, and take the highest modified roll
    star_type = fo.starType.unknown
    try:
        max_roll = 0
        for candidate in star_types:
            roll = random.randint(1, 100) \
                + fo.universe_age_mod_to_star_type_dist(galaxy_age, candidate) \
                + fo.base_star_type_dist(candidate)
            if max_roll < roll:
                max_roll = roll
                star_type = candidate
    except:
        # in case of an error play save and set star type to invalid
        star_type = fo.starType.unknown
        print "Python pick_star_type: Pick star type failed"
        print sys.exc_info()[1]

    # if we got an invalid star type (for whatever reason),
    # just select one randomly from the global tuple
    if star_type == fo.starType.unknown:
        star_type = random.choice(star_types)
    return star_type


def generate_system(star_type, pos_x, pos_y, name=""):
    """
    Generates a new star system at the specified position with the specified star type
    """
    # create and insert the system into the universe
    # and return ID of the newly created system
    try:
        system = fo.create_system(star_type, name, pos_x, pos_y)
    except:
        system = fo.invalid_object()
        print "Python generate_system: Create system failed"
        print sys.exc_info()[1]
    return system


def name_planets(system):
    """
    Sets the names of the planets of the specified system
    planet name is system name + planet number (as roman number)
    unless it's an asteroid belt, in that case name is system
    name + 'asteroid belt' (localized)
    """
    planet_number = 1
    # iterate over all planets in the system
    for planet in fo.sys_get_planets(system):
        # use different naming methods for "normal" planets and asteroid belts
        if fo.planet_get_type(planet) == fo.planetType.asteroids:
            # get localized text from stringtable
            name = fo.user_string("PL_ASTEROID_BELT_OF_SYSTEM")
            # %1% parameter in the localized string is the system name
            name = name.replace("%1%", fo.get_name(system))
        else:
            # set name to system name + planet number as roman number...
            name = fo.get_name(system) + " " + fo.roman_number(planet_number)
            # ...and increase planet number
            planet_number += 1
        # do the actual renaming
        fo.set_name(planet, name)


def generate_systems(pos_list, gsd):
    """
    Generates and populates star systems at all positions in specified list
    """
    sys_list = []
    for position in pos_list:
        star_type = pick_star_type(gsd.age)
        system = generate_system(star_type, position.x, position.y)
        sys_list.append(system)
        for orbit in range(0, fo.sys_get_num_orbits(system) - 1):
            # check for each orbit if a planet shall be created by determining planet size
            planet_size = planets.calc_planet_size(star_type, orbit, gsd.planetDensity, gsd.shape)
            if planet_size in planets.planet_sizes:
                # ok, we want a planet, determine planet type and generate the planet
                planet_type = planets.calc_planet_type(star_type, orbit, planet_size)
                planets.generate_planet(planet_size, planet_type, system, orbit)
    return sys_list