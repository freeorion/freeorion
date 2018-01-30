import random

import freeorion as fo

import universe_statistics
from names import get_name_list, random_name
from options import (HS_ACCEPTABLE_PLANET_SIZES, HS_ACCEPTABLE_PLANET_TYPES, HS_MAX_JUMP_DISTANCE_LIMIT,
                     HS_MIN_DISTANCE_PRIORITY_LIMIT, HS_MIN_PLANETS_IN_VICINITY_PER_SYSTEM,
                     HS_MIN_PLANETS_IN_VICINITY_TOTAL, HS_MIN_SYSTEMS_IN_VICINITY, HS_VICINITY_RANGE)
from planets import calc_planet_size, calc_planet_type, planet_sizes_real, planet_types_real
from starsystems import pick_star_type, star_types_real
from util import report_error


def get_empire_name_generator():
    """
    String generator, return random empire name from string list,
    if string list is empty generate random name.
    """
    empire_names = get_name_list("EMPIRE_NAMES")
    random.shuffle(empire_names)
    while True:
        if empire_names:
            yield empire_names.pop()
        else:
            yield random_name(5)


# generate names for empires, use next(empire_name_generator) to get next name.
empire_name_generator = get_empire_name_generator()


def get_starting_species_pool():
    """
    Empire species pool generator, return random empire species and ensure somewhat even distribution
    """
    # fill the initial pool with two sets of all playable species
    # this way we have somewhat, but not absolutely strict even distribution of starting species at least when there
    # is only a few number of players (some species can occur twice at max while others not at all)
    pool = fo.get_playable_species() * 2

    # randomize order in initial pool so we don't get the same species all the time
    random.shuffle(pool)
    # generator loop
    while True:
        # if our pool is exhausted (because we have more players than species instances in our initial pool)
        # refill the pool with one set of all playable species
        if not pool:
            pool = fo.get_playable_species()
            # again, randomize order in refilled pool so we don't get the same species all the time
            random.shuffle(pool)
        # pick and return next species, and remove it from our pool
        yield pool.pop()


# generates starting species for empires, use next(starting_species_pool) to get next species
starting_species_pool = get_starting_species_pool()


def count_planets_in_systems(systems, planet_types_filter=HS_ACCEPTABLE_PLANET_TYPES):
    """
    Return the total number of planets in the specified group of systems,
    only count the planet types specified in planet_types_filter.
    """
    num_planets = 0
    for system in systems:
        num_planets += len([p for p in fo.sys_get_planets(system) if fo.planet_get_type(p) in planet_types_filter])
    return num_planets


def calculate_home_system_merit(system):
    """Calculate the system's merit as the number of planets within HS_VICINTIY_RANGE."""
    return count_planets_in_systems(fo.systems_within_jumps_unordered(HS_VICINITY_RANGE, [system]))


def min_planets_in_vicinity_limit(num_systems):
    """
    Calculates the minimum planet limit for the specified number of systems.
    This limit is the lower of HS_MIN_PLANETS_IN_VICINITY_TOTAL or HS_MIN_PLANETS_IN_VICINITY_PER_SYSTEM
    planets per system.
    """
    return min(HS_MIN_PLANETS_IN_VICINITY_TOTAL, num_systems * HS_MIN_PLANETS_IN_VICINITY_PER_SYSTEM)


class HomeSystemFinder(object):
    """Finds a set of home systems with a least ''num_home_systems'' systems."""
    def __init__(self, _num_home_systems):
        # cache of sytem merits
        self.system_merit = {}
        self.num_home_systems = _num_home_systems

    def find_home_systems_for_min_jump_distance(self, systems_pool, min_jumps):
        """
        Return a good list of home systems or an empty list if there are fewer than num_home_systems in the pool.

        A good list of home systems are at least the specified minimum number of jumps apart,
        with the best minimum system merit of all such lists picked randomly from the ''systems_pool''.

        Algorithm:
        Make several attempts to find systems that match the condition
        of being at least min_jumps apart.
        Use the minimum merit of the best num_home_system systems found
        to compare the candidate with the current best set of systems.
        On each attempt use the minimum merit of the current best set of home
        systems to truncate the pool of candidates.
        """

        # precalculate the system merits
        for system in systems_pool:
            if system not in self.system_merit:
                self.system_merit[system] = calculate_home_system_merit(system)

        # The list of merits and systems sorted in descending order by merit.
        all_merit_system = sorted([(self.system_merit[s], s)
                                   for s in systems_pool], reverse=True)

        current_merit_lower_bound = 0
        best_candidate = []

        # Cap the number of attempts when the found number of systems is less than the target
        # num_home_systems because this indicates that the min_jumps is too large and/or the
        # systems_pool is too small to ever succeed.

        # From experimentation with cluster and 3 arm spiral galaxies, with low, med and high
        # starlane density and (number of systems, number of home systems) pairs of (9999, 399),
        # (999, 39) and (199, 19) the following was observered.  The distribution of candidate
        # length is a normal random variable with standard deviation approximately equal to

        # expected_len_candidate_std = (len(systems) ** (1.0/2.0)) * 0.03

        # which is about 1 for 1000 systems.  It is likely that anylen(candidate) is within 1
        # standard deviation of the expected len(candidate)

        # If we are within the MISS_THRESHOLD of the target then try up to num_complete_misses more times.
        MISS_THRESHOLD = 3
        num_complete_misses_remaining = 4

        # Cap the number of attempts to the smaller of the number of systems in the pool, or 100
        attempts = min(100, len(systems_pool))

        while attempts and num_complete_misses_remaining:
            # use a local pool of all candidate systems better than the worst threshold merit
            all_merit_system = [(m, s) for (m, s) in all_merit_system if m > current_merit_lower_bound]
            local_pool = {s for (m, s) in all_merit_system}

            if len(local_pool) < self.num_home_systems:
                if not best_candidate:
                    print ("Failing in find_home_systems_for_min_jump_distance because "
                           "current_merit_lower_bound = {} trims local pool to {} systems "
                           "which is less than num_home_systems {}.".format(
                               current_merit_lower_bound, len(local_pool), self.num_home_systems))
                break

            attempts = min(attempts - 1, len(local_pool))

            candidate = []
            while local_pool:
                member = random.choice(list(local_pool))
                candidate.append(member)

                # remove all neighbors from the local pool
                local_pool -= set(fo.systems_within_jumps_unordered(min_jumps, [member]))

            # Count complete misses when number of candidates is not close to the target.
            if len(candidate) < (self.num_home_systems - MISS_THRESHOLD):
                num_complete_misses_remaining -= 1

            if len(candidate) < self.num_home_systems:
                continue

            # Calculate the merit of the current attempt.  If it is the best so far
            # keep it and update the merit_threshold
            merit_system = sorted([(self.system_merit[s], s)
                                   for s in candidate], reverse=True)[:self.num_home_systems]

            (merit, system) = merit_system[-1]

            # If we have a better candidate, set the new lower bound and try for a better candidate.
            if merit > current_merit_lower_bound:
                print ("Home system set merit lower bound improved from {} to "
                       "{}".format(current_merit_lower_bound, merit))
                current_merit_lower_bound = merit
                best_candidate = [s for (_, s) in merit_system]

                # Quit successfully if the lowest merit system meets the minimum threshold
                if merit >= min_planets_in_vicinity_limit(
                        fo.systems_within_jumps_unordered(HS_VICINITY_RANGE, [system])):
                    break

        return best_candidate


def find_home_systems(num_home_systems, pool_list, jump_distance, min_jump_distance):
    """
    Tries to find a specified number of home systems which are as far apart from each other as possible.
    Starts with the specified jump distance and reduces that limit until enough systems can be found or the
    jump distance drops below the specified minimum jump distance limit (in this case fail).
    For each jump distance several attempts are made: one for each pool passed in pool_list.
    This parameter contains a list of tuples, each tuple has a pool of systems as first element and a description
    of the pool for logging purposes as second element.
    """

    finder = HomeSystemFinder(num_home_systems)
    # try to find home systems, decrease the min jumps until enough systems can be found, or the jump distance drops
    # below the specified minimum jump distance (which means failure)
    while jump_distance >= min_jump_distance:
        print "Trying to find", num_home_systems, "home systems that are at least", jump_distance, "jumps apart..."

        # try to pick our home systems by iterating over the pools we got
        for pool, pool_label in pool_list:
            print "...use", pool_label

            # check if the pool has enough systems to pick from
            if len(pool) <= num_home_systems:
                # no, the pool has less systems than home systems requested, so just skip trying using that pool
                print "...pool only has", len(pool), "systems, skip trying to use it"
                continue

            # try to pick home systems
            home_systems = finder.find_home_systems_for_min_jump_distance(pool, jump_distance)
            # check if we got enough
            if len(home_systems) >= num_home_systems:
                # yes, we got what we need, return the home systems we found
                print "...", len(home_systems), "systems found"
                return home_systems
            else:
                # no, try next pool
                print "...only", len(home_systems), "systems found"

        # we did not find enough home systems with the current jump distance requirement,
        # so decrease the jump distance and try again
        jump_distance -= 1

    # all attempts came up with too few systems, return empty list to indicate failure
    return []


def add_planets_to_vicinity(vicinity, num_planets, gsd):
    """
    Adds the specified number of planets to the specified systems.
    """
    print "Adding", num_planets, "planets to the following systems:", vicinity

    # first, compile a list containing all the free orbits in the specified systems
    # begin with adding the free orbits of all systems that have a real star (that is, no neutron star, black hole,
    # and not no star), if that isn't enough, also one, by one, add the free orbits of neutron star, black hole and
    # no star systems (in that order) until we have enough free orbits

    # for that, we use this list of tuples
    # the first tuple contains all real star types, the following tuples the neutron, black hole and no star types,
    # so we can iterate over this list and only add the free orbits of systems that match the respective star types
    # each step
    # this way we can prioritize the systems we want to add planets to by star type
    acceptable_star_types_list = [
        star_types_real,
        (fo.starType.noStar,),
        (fo.starType.neutron,),
        (fo.starType.blackHole,)
    ]

    # store the free orbits as a list of tuples of (system, orbit)
    free_orbits_map = []

    # now, iterate over the list of acceptable star types
    for acceptable_star_types in acceptable_star_types_list:
        # check all systems in the list of systems we got passed into this function
        for system in vicinity:
            # if this system has a star type we want to accept in this step, add its free orbits to our list
            if fo.sys_get_star_type(system) in acceptable_star_types:
                free_orbits_map.extend([(system, orbit) for orbit in fo.sys_free_orbits(system)])
        # check if we got enough free orbits after completing this step
        # we want 4 times as much free orbits as planets we want to add, that means each system shouldn't get more
        # than 2-3 additional planets on average
        if len(free_orbits_map) > (num_planets * 4):
            break

    # if we got less free orbits than planets that should be added, something is wrong
    # in that case abort and log an error
    if len(free_orbits_map) < num_planets:
        report_error("Python add_planets_to_vicinity: less free orbits than planets to add - cancelled")

    print "...free orbits available:", free_orbits_map
    # as we will pop the free orbits from this list afterwards, shuffle it to randomize the order of the orbits
    random.shuffle(free_orbits_map)

    # add the requested number of planets
    while num_planets > 0:
        # get the next free orbit from the list we just compiled
        system, orbit = free_orbits_map.pop()

        # check the star type of the system containing the orbit we got
        star_type = fo.sys_get_star_type(system)
        if star_type in [fo.starType.noStar, fo.starType.blackHole]:
            # if it is a black hole or has no star, change the star type
            # pick a star type, continue until we get a real star
            # don't accept neutron, black hole or no star
            print "...system picked to add a planet has star type", star_type
            while star_type not in star_types_real:
                star_type = pick_star_type(gsd.age)
            print "...change that to", star_type
            fo.sys_set_star_type(system, star_type)

        # pick a planet size, continue until we get a size that matches the HS_ACCEPTABLE_PLANET_SIZES option
        planet_size = fo.planetSize.unknown
        while planet_size not in HS_ACCEPTABLE_PLANET_SIZES:
            planet_size = calc_planet_size(star_type, orbit, fo.galaxySetupOption.high, gsd.shape)

        # pick an according planet type
        planet_type = calc_planet_type(star_type, orbit, planet_size)

        # finally, create the planet in the system and orbit we got
        print "...adding", planet_size, planet_type, "planet to system", system
        if fo.create_planet(planet_size, planet_type, system, orbit, "") == fo.invalid_object():
            report_error("Python add_planets_to_vicinity: create planet in system %d failed" % system)

        # continue with next planet
        num_planets -= 1


def compile_home_system_list(num_home_systems, systems, gsd):
    """
    Compiles a list with a requested number of home systems.
    """
    print "Compile home system list:", num_home_systems, "systems requested"

    # if the list of systems to choose home systems from is empty, report an error and return empty list
    if not systems:
        report_error("Python generate_home_system_list: no systems to choose from")
        return []

    # calculate an initial minimal number of jumps that the home systems should be apart,
    # based on the total number of systems to choose from and the requested number of home systems
    # don't let min_jumps be either:
    # a.) larger than a defined limit, because an unreasonably large number is really not at all needed,
    #     and with large galaxies an excessive amount of time can be used in failed attempts
    # b.) lower than the minimum jump distance limit that should be considered high priority (see options.py),
    #     otherwise no attempt at all would be made to enforce the other requirements for home systems (see below)
    min_jumps = min(HS_MAX_JUMP_DISTANCE_LIMIT, max(int(float(len(systems)) / float(num_home_systems * 2)),
                                                    HS_MIN_DISTANCE_PRIORITY_LIMIT))

    # home systems must have a certain minimum of systems and planets in their near vicinity
    # we will try to select our home systems from systems that match this criteria, if that fails, we will select our
    # home systems from all systems and add the missing number planets to the systems in their vicinity afterwards
    # the minimum system and planet limit and the jump range that defines the "near vicinity" are controlled by the
    # HS_* option constants in options.py (see there)

    # we start by building two additional pools of systems: one that contains all systems that match the criteria
    # completely (meets the min systems and planets limit), and one that contains all systems that match the criteria
    # at least partially (meets the min systems limit)
    pool_matching_sys_and_planet_limit = []
    pool_matching_sys_limit = []
    for system in systems:
        systems_in_vicinity = fo.systems_within_jumps_unordered(HS_VICINITY_RANGE, [system])
        if len(systems_in_vicinity) >= HS_MIN_SYSTEMS_IN_VICINITY:
            pool_matching_sys_limit.append(system)
            if count_planets_in_systems(systems_in_vicinity) >= min_planets_in_vicinity_limit(len(systems_in_vicinity)):
                pool_matching_sys_and_planet_limit.append(system)
    print (len(pool_matching_sys_and_planet_limit),
           "systems meet the min systems and planets in the near vicinity limit")
    print len(pool_matching_sys_limit), "systems meet the min systems in the near vicinity limit"

    # now try to pick the requested number of home systems
    # we will do this by calling find_home_systems, which takes a list of tuples defining the pools from which to pick
    # the home systems; it will use the pools in the order in which they appear in the list, so put better pools first

    # we will make two attempts: the first one with the filtered pools we just created, and tell find_home_systems
    # to start with the min_jumps jumps distance we calculated above, but not to go lower than
    # HS_MIN_DISTANCE_PRIORITY_LIMIT

    print "First attempt: trying to pick home systems from the filtered pools of preferred systems"
    pool_list = [
        # the better pool is of course the one where all systems meet BOTH the min systems and planets limit
        (pool_matching_sys_and_planet_limit, "pool of systems that meet both the min systems and planets limit"),
        # next the less preferred pool where all systems at least meets the min systems limit
        # specify 0 as number of requested home systems to pick as much systems as possible
        (pool_matching_sys_limit, "pool of systems that meet at least the min systems limit"),
    ]
    home_systems = find_home_systems(num_home_systems, pool_list, min_jumps, HS_MIN_DISTANCE_PRIORITY_LIMIT)

    # check if the first attempt delivered a list with enough home systems
    # if not, we make our second attempt, this time disregarding the filtered pools and using all systems, starting
    # again with the min_jumps jump distance limit and specifying 0 as number of required home systems to pick as much
    # systems as possible
    if len(home_systems) < num_home_systems:
        print "Second attempt: trying to pick home systems from all systems"
        home_systems = find_home_systems(num_home_systems, [(systems, "complete pool")], min_jumps, 1)

    # check if the selection process delivered a list with enough home systems
    # if not, our galaxy obviously is too crowded, report an error and return an empty list
    if len(home_systems) < num_home_systems:
        report_error("Python generate_home_system_list: requested %d homeworlds in a galaxy with %d systems"
                     % (num_home_systems, len(systems)))
        return []

    # check if we got more home systems than we requested
    if len(home_systems) > num_home_systems:
        # yes: calculate the number of planets in the near vicinity of each system
        # and store that value with each system in a map
        hs_planets_in_vicinity_map = {s: calculate_home_system_merit(s) for s in home_systems}
        # sort the home systems by the number of planets in their near vicinity using the map
        # now only pick the number of home systems we need, taking those with the highest number of planets
        home_systems = sorted(home_systems, key=hs_planets_in_vicinity_map.get, reverse=True)[:num_home_systems]

    # make sure all our home systems have a "real" star (that is, a star that is not a neutron star, black hole,
    # or even no star at all) and at least one planet in it
    for home_system in home_systems:
        # if this home system has no "real" star, change star type to a randomly selected "real" star
        if fo.sys_get_star_type(home_system) not in star_types_real:
            star_type = random.choice(star_types_real)
            print "Home system", home_system, "has star type", fo.sys_get_star_type(home_system),\
                  ", changing that to", star_type
            fo.sys_set_star_type(home_system, star_type)

        # if this home system has no planets, create one in a random orbit
        # we take random values for type and size, as these will be set to suitable values later
        if not fo.sys_get_planets(home_system):
            print "Home system", home_system, "has no planets, adding one"
            planet = fo.create_planet(random.choice(planet_sizes_real),
                                      random.choice(planet_types_real),
                                      home_system, random.randint(0, fo.sys_get_num_orbits(home_system) - 1), "")
            # if we couldn't create the planet, report an error and return an empty list
            if planet == fo.invalid_object():
                report_error("Python generate_home_system_list: couldn't create planet in home system")
                return []

    # finally, check again if all home systems meet the criteria of having the required minimum number of planets
    # within their near vicinity, if not, add the missing number of planets
    print "Checking if home systems have the required minimum of planets within the near vicinity..."
    for home_system in home_systems:
        # calculate the number of missing planets, and add them if this number is > 0
        systems_in_vicinity = fo.systems_within_jumps_unordered(HS_VICINITY_RANGE, [home_system])
        num_systems_in_vicinity = len(systems_in_vicinity)
        num_planets_in_vicinity = count_planets_in_systems(systems_in_vicinity)
        num_planets_to_add = min_planets_in_vicinity_limit(num_systems_in_vicinity) - num_planets_in_vicinity
        print "Home system", home_system, "has", num_systems_in_vicinity, "systems and", num_planets_in_vicinity,\
            "planets in the near vicinity, required minimum:", min_planets_in_vicinity_limit(num_systems_in_vicinity)
        if num_planets_to_add > 0:
            systems_in_vicinity.remove(home_system)  # don't add planets to the home system, so remove it from the list
            # sort the systems_in_vicinity before adding, since the C++ engine doesn't guarrantee the same
            # platform independence as python.
            add_planets_to_vicinity(sorted(systems_in_vicinity), num_planets_to_add, gsd)

    # as we've sorted the home system list before, lets shuffle it to ensure random order and return
    random.shuffle(home_systems)
    return home_systems


def setup_empire(empire, empire_name, home_system, starting_species, player_name):
    """
    Sets up various aspects of an empire, like empire name, homeworld, etc.
    """

    # set empire name, if no one is given, pick one randomly
    if not empire_name:
        print "No empire name set for player", player_name, ", picking one randomly"
        empire_name = next(empire_name_generator)
    fo.empire_set_name(empire, empire_name)
    print "Empire name for player", player_name, "is", empire_name

    # check starting species, if no one is given, pick one randomly
    if starting_species == "RANDOM" or not starting_species:
        print "Picking random starting species for player", player_name
        starting_species = next(starting_species_pool)
    print "Starting species for player", player_name, "is", starting_species
    universe_statistics.empire_species[starting_species] += 1

    # pick a planet from the specified home system as homeworld
    planet_list = fo.sys_get_planets(home_system)
    # if the system is empty, report an error and return false, indicating failure
    if not planet_list:
        report_error("Python setup_empire: got home system with no planets")
        return False
    homeworld = random.choice(planet_list)

    # set selected planet as empire homeworld with selected starting species
    fo.empire_set_homeworld(empire, homeworld, starting_species)

    # set homeworld focus
    # check if the preferred focus for the starting species is among
    # the foci available on the homeworld planet
    available_foci = fo.planet_available_foci(homeworld)
    preferred_focus = fo.species_preferred_focus(starting_species)
    if preferred_focus in available_foci:
        # if yes, set the homeworld focus to the preferred focus
        print "Player", player_name, ": setting preferred focus", preferred_focus, "on homeworld"
        fo.planet_set_focus(homeworld, preferred_focus)
    elif len(available_foci) > 0:
        # if no, and there is at least one available focus,
        # just take the first of the list
        if preferred_focus == "":
            print "Player", player_name, ": starting species", starting_species, "has no preferred focus, using",\
                  available_foci[0], "instead"
        else:
            print "Player", player_name, ": preferred focus", preferred_focus, "for starting species",\
                  starting_species, "not available on homeworld, using", available_foci[0], "instead"
        fo.planet_set_focus(homeworld, available_foci[0])
    else:
        # if no focus is available on the homeworld, don't set any focus
        print "Player", player_name, ": no available foci on homeworld for starting species", starting_species

    # give homeworld starting buildings
    print "Player", player_name, ": add starting buildings to homeworld"
    for item in fo.load_starting_buildings():
        fo.create_building(item.name, homeworld, empire)

    # unlock starting techs, buildings, hulls, ship parts, etc.
    # use default content file
    print "Player", player_name, ": add unlocked items"
    for item in fo.load_item_spec_list():
        fo.empire_unlock_item(empire, item.type, item.name)

    # add premade ship designs to empire
    print "Player", player_name, ": add premade ship designs"
    for ship_design in fo.design_get_premade_list():
        fo.empire_add_ship_design(empire, ship_design)

    # add starting fleets to empire
    # use default content file
    print "Player", player_name, ": add starting fleets"
    fleet_plans = fo.load_fleet_plan_list()
    for fleet_plan in fleet_plans:
        # first, create the fleet
        fleet = fo.create_fleet(fleet_plan.name(), home_system, empire)
        # if the fleet couldn't be created, report an error and try to continue with the next fleet plan
        if fleet == fo.invalid_object():
            report_error("Python setup empire: couldn't create fleet %s" % fleet_plan.name())
            continue
        # second, iterate over the list of ship design names in the fleet plan
        for ship_design in fleet_plan.ship_designs():
            # create a ship in the fleet
            # if the ship couldn't be created, report an error and try to continue with the next ship design
            if fo.create_ship("", ship_design, starting_species, fleet) == fo.invalid_object():
                report_error("Python setup empire: couldn't create ship %s for fleet %s"
                             % (ship_design, fleet_plan.name()))
    return True
