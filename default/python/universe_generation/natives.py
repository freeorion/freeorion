import freeorion as fo
import itertools
import random

import planets
import universe_statistics
import universe_tables

natives_for_planet_type = {}
planet_types_for_natives = {}


def generate_natives(native_freq, systems, empire_home_systems):  # noqa: C901
    """
    Adds non-empire-affiliated native populations to planets.
    And to asteroids and to gasgiants as well.
    """

    # first, calculate the chance for natives based on the native frequency that has been passed
    # get the corresponding value for the specified natives frequency from the universe tables
    native_chance = universe_tables.NATIVE_FREQUENCY[native_freq]
    # a value of 0 means no natives, in this case return immediately
    if native_chance <= 0:
        return

    # compile a list of systems where natives can be placed
    # select only systems sufficiently far away from player home systems
    # list of systems safe for natives
    EMPIRE_TO_NATIVE_MIN_DIST = 2
    empire_exclusions = set(
        itertools.chain.from_iterable(
            fo.systems_within_jumps_unordered(EMPIRE_TO_NATIVE_MIN_DIST, [e]) for e in empire_home_systems
        )
    )
    native_safe_planets = set(
        itertools.chain.from_iterable([fo.sys_get_planets(s) for s in systems if s not in empire_exclusions])
    )

    print("Number of planets far enough from players for natives to be allowed:", len(native_safe_planets))
    # if there are no "native safe" planets at all, we can stop here
    if not native_safe_planets:
        return

    # get all native species
    native_species = fo.get_native_species()
    print("Species that can be added as natives:")
    print("... " + "\n... ".join(native_species))

    # create a map with a list for each planet type containing the species
    # for which this planet type is a good environment
    # we will need this afterwards when picking natives for a planet
    natives_for_planet_type.clear()  # just to be safe
    natives_for_planet_type.update({planet_type: [] for planet_type in planets.planet_types})
    planet_types_for_natives.clear()
    planet_types_for_natives.update({species: set() for species in native_species})

    # iterate over all native species we got
    for species in native_species:
        # check the planet environment for all planet types for this species
        for planet_type in planets.planet_types:
            # if this planet type is a good environment for the species, add it to the list for this planet type
            if fo.species_get_planet_environment(species, planet_type) == fo.planetEnvironment.good:
                natives_for_planet_type[planet_type].append(species)
                planet_types_for_natives[species].add(planet_type)
        # if the species simply has no good environment to live in, try the same as above for adequate instead
        # this is needed for a couple of new native species with only adequate and no good environments
        if len(planet_types_for_natives[species]) < 1:
            for planet_type in planets.planet_types:
                if fo.species_get_planet_environment(species, planet_type) == fo.planetEnvironment.adequate:
                    natives_for_planet_type[planet_type].append(species)
                    planet_types_for_natives[species].add(planet_type)
        # if the species still has no fitting environment then try poor next
        if len(planet_types_for_natives[species]) < 1:
            for planet_type in planets.planet_types:
                if fo.species_get_planet_environment(species, planet_type) == fo.planetEnvironment.poor:
                    natives_for_planet_type[planet_type].append(species)
                    planet_types_for_natives[species].add(planet_type)

    # randomly add species to planets
    # iterate over the list of "native safe" planets we compiled earlier
    for candidate in native_safe_planets:
        # select a native species to put on this planet
        planet_type = fo.planet_get_type(candidate)
        # check if we have any native species that like this planet type
        if not natives_for_planet_type[planet_type]:
            # no, continue with next planet
            continue
        universe_statistics.potential_native_planet_summary[planet_type] += 1
        # temporarily dampen native frequencies on roids and GGs until better galaxy setup options are in place
        factor = 1
        if planet_type in [fo.planetType.asteroids, fo.planetType.gasGiant]:
            factor = 5
        # make a "roll" against the chance for natives to determine if we shall place natives on this planet
        if random.random() > native_chance / factor:
            # no, continue with next planet
            continue
        universe_statistics.settled_native_planet_summary[planet_type] += 1

        # randomly pick one of the native species available for this planet type
        natives = random.choice(natives_for_planet_type[planet_type])

        # put the selected natives on the planet
        fo.planet_set_species(candidate, natives)
        # set planet as homeworld for that species
        fo.species_add_homeworld(natives, candidate)
        # set planet focus
        # check if the preferred focus for the native species is among the foci available on this planet
        available_foci = fo.planet_available_foci(candidate)
        preferred_focus = fo.species_preferred_focus(natives)
        if preferred_focus in available_foci:
            # if yes, set the planet focus to the preferred focus
            fo.planet_set_focus(candidate, preferred_focus)
        elif available_foci:
            # if no, and there is at least one available focus, just take the first of the list
            # otherwise don't set any focus
            fo.planet_set_focus(candidate, available_foci[0])
            print("preferred focus", preferred_focus, "not available, focus on", available_foci[0], "instead")
        print("Added native", natives, "to planet", fo.get_name(candidate))

        # increase the statistics counter for this native species, so a species summary can be dumped to the log later
        universe_statistics.species_summary[natives] += 1
