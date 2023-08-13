import freeorion as fo
import random
import sys
from itertools import product

import planets
import universe_tables
import util

# tuple of available star types
star_types = (
    fo.starType.blue,
    fo.starType.white,
    fo.starType.yellow,
    fo.starType.orange,
    fo.starType.red,
    fo.starType.neutron,
    fo.starType.blackHole,
    fo.starType.noStar,
)

# tuple of "real" star types (that is, blue to red, not neutron, black hole or even no star)
star_types_real = (fo.starType.blue, fo.starType.white, fo.starType.yellow, fo.starType.orange, fo.starType.red)


def pick_star_type(galaxy_age):
    """
    Picks and returns a star type based on universe tables distribution modifiers.
    """

    # try to pick a star type by making a series of "rolls" (1-100)
    # for each star type, and take the highest modified roll
    star_type = fo.starType.unknown
    try:
        max_roll = 0
        for candidate in star_types:
            roll = (
                random.randint(1, 100)
                + universe_tables.UNIVERSE_AGE_MOD_TO_STAR_TYPE_DIST[galaxy_age][candidate]
                + universe_tables.BASE_STAR_TYPE_DIST[candidate]
            )
            if max_roll < roll:
                max_roll = roll
                star_type = candidate
    except:  # noqa: E722
        # in case of an error play save and set star type to invalid
        star_type = fo.starType.unknown
        util.report_error("Python pick_star_type: Pick star type failed\n" + sys.exc_info()[1])

    # if we got an invalid star type (for whatever reason),
    # just select one randomly from the global tuple
    if star_type == fo.starType.unknown:
        star_type = random.choice(star_types)
    return star_type


def name_planets(system):
    """
    Sets the names of the planets of the specified system.

    Planet name is system name + planet number (as roman number)
    unless it's an asteroid belt, in that case name is system
    name + 'asteroid belt' (localized).
    """
    # iterate over all planets in the system
    sys_name = fo.get_name(system)
    for planet in fo.sys_get_planets(system):
        name = fo.userString("NEW_PLANET_NAME")
        name = name.replace("%1%", sys_name)
        name = name.replace("%2%", fo.planet_cardinal_suffix(planet))
        name = name.strip()
        fo.set_name(planet, name)


def can_have_no_planets(star_type):
    """
    Return True is system is allowed to have no planets.
    This intentionally only checks the STAR_TYPE_MOD_TO_PLANET_SIZE_DIST so that
    the following code forces most stars to have a planetary body so that in the
    GUI the presence of a star implies a planet
    """
    return universe_tables.STAR_TYPE_MOD_TO_PLANET_SIZE_DIST[star_type][fo.planetSize.noWorld] > 0


def generate_systems(pos_list, gsd):
    """
    Generates and populates star systems at all positions in specified list.
    """
    sys_list = []
    for position in pos_list:
        star_type = pick_star_type(gsd.age)
        system = fo.create_system(star_type, "", position[0], position[1])
        if system == fo.invalid_object():
            # create system failed, report an error and try to continue with next position
            util.report_error(
                f"Python generate_systems: create system at position ({position[0]:f}, {position[1]:f}) failed"
            )
            continue
        sys_list.append(system)

        orbits = list(range(fo.sys_get_num_orbits(system)))

        if not planets.can_have_planets(star_type, orbits, gsd.planet_density, gsd.shape):
            continue

        # Try to generate planets in each orbit.
        # If after each orbit is tried once there are no planets then
        # keep trying until a single planet is placed.
        # Except for black hole systems, which can be empty.

        at_least_one_planet = False
        random.shuffle(orbits)
        for orbit in orbits:
            if planets.generate_a_planet(system, star_type, orbit, gsd.planet_density, gsd.shape):
                at_least_one_planet = True

        if at_least_one_planet or can_have_no_planets(star_type):
            continue

        recursion_limit = 1000
        for _, orbit in product(range(recursion_limit), orbits):
            if planets.generate_a_planet(system, star_type, orbit, gsd.planet_density, gsd.shape):
                break
        else:
            # Intentionally non-modal.  Should be a warning.
            print(
                (
                    "Python generate_systems: place planets in system %d at position (%.2f, %.2f) failed"
                    % (system, position[0], position[1])
                ),
                file=sys.stderr,
            )

    return sys_list
