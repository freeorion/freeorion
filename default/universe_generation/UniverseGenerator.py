import sys
import random
import math
import os

from pprint import pprint

import foUniverseGenerator as fo

## star group naming options 
# if star_groups_use_chars is true use entries from the stringtable entry STAR_GROUP_CHARS (single characters), 
# otherwise use words like 'Alpha' from the stringtable entry STAR_GROUP_WORDS. 
star_groups_use_chars = True
postfix_stargroup_modifiers = True # if false then prefix 

# the target proportion of systems to be given individual names, dependent on size of galaxy
target_indiv_ratio_small = 0.6
target_indiv_ratio_large = 0.3
naming_large_galaxy_size = 200

#tuples of consonants and vowels for random name generation
consonants = ("b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r", "s", "t", "v", "w", "x", "y", "z")
vowels = ("a", "e", "i", "o", "u")

# tuple of galaxy shapes to randomly choose from when shape is "random"
galaxy_shapes = (fo.galaxyShape.spiral2,    fo.galaxyShape.spiral3,     fo.galaxyShape.spiral4,
                 fo.galaxyShape.cluster,    fo.galaxyShape.elliptical,  fo.galaxyShape.ring,
                 fo.galaxyShape.irregular,  fo.galaxyShape.test)

# tuple of available star types
star_types = (fo.starType.blue, fo.starType.white,   fo.starType.yellow,    fo.starType.orange,
              fo.starType.red,  fo.starType.neutron, fo.starType.blackHole, fo.starType.noStar)

# tuple of "real" star types (that is, blue to red, not neutron, black hole or even no star)
real_star_types = (fo.starType.blue,   fo.starType.white, fo.starType.yellow,
                   fo.starType.orange, fo.starType.red)

# should match SYSTEM_ORBITS from System.cpp
system_orbits = 9

# tuple of all valid planet sizes
planet_sizes_all = (fo.planetSize.tiny, fo.planetSize.small,     fo.planetSize.medium,   fo.planetSize.large,
                    fo.planetSize.huge, fo.planetSize.asteroids, fo.planetSize.gasGiant, fo.planetSize.noWorld)

# tuple of planet sizes without "no world"
planet_sizes = (fo.planetSize.tiny, fo.planetSize.small,     fo.planetSize.medium,   fo.planetSize.large,
                fo.planetSize.huge, fo.planetSize.asteroids, fo.planetSize.gasGiant)

# tuple of "real" planet sizes (without "no world", asteroids and gas giants)
real_planet_sizes = (fo.planetSize.tiny,  fo.planetSize.small, fo.planetSize.medium,
                     fo.planetSize.large, fo.planetSize.huge)

# tuple of all available planet types (with asteroids and gas giants)
planet_types_all = (fo.planetType.swamp,  fo.planetType.radiated,  fo.planetType.toxic,  fo.planetType.inferno,
                    fo.planetType.barren, fo.planetType.tundra,    fo.planetType.desert, fo.planetType.terran,
                    fo.planetType.ocean,  fo.planetType.asteroids, fo.planetType.gasGiant)

# tuple of available planet types without asteroids and gas giants
planet_types = (fo.planetType.swamp,  fo.planetType.radiated,  fo.planetType.toxic,  fo.planetType.inferno,
                fo.planetType.barren, fo.planetType.tundra,    fo.planetType.desert, fo.planetType.terran,
                fo.planetType.ocean)

# for starname modifiers
stargroup_words=[]
stargroup_chars=[]
stargroup_modifiers = []
greek_letters = ["Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta", "Iota", "Kappa",
                "Lambda", "Mu", "Nu", "Xi", "Omicron", "Pi", "Rho", "Sigma", "Tau", "Upsilon", "Phi",
                "Chi", "Psi", "Omega" ]

# global lists for star system names, empire names
star_names = []
empire_names = []

# Reads a list of strings from a content file
def load_string_list(file_name):
    try:
        with open(file_name, "r") as f:
            string_list = f.read().splitlines()
        for i in range(0, len(string_list)):
            string_list[i] = string_list[i].strip('"')
        return string_list
    except:
        print "Unable to access", file_name
        print sys.exc_info()[1]
        return []

# Retrieves a list of names from the string tables
def get_name_list(name_list):
    return fo.userString(name_list).splitlines()

# Returns a random star system name
def get_star_name():
    # try to get a name from the global list
    try:
        # pop names from the list until we get an non-empty string
        # this ensures empty lines in the starnames file are skipped
        # if the list is exhausted, this will raise an IndexError exception
        # this case will be caught and handled in the except clause
        name = ""
        while len(name) == 0:
            name = star_names.pop()
    except IndexError:
        # global list is exhausted, generate a random name instead
        for i in range(0, 3):
            name = name + random.choice(consonants + vowels).upper()
        name = name + "-" + str(random.randint(1000, 9999))
    return name

# Returns a random empire name
def get_empire_name():
    # try to get a name from the global list
    try:
        # pop names from the list until we get an non-empty string
        # this ensures empty lines in the empire names file are skipped
        # if the list is exhausted, this will raise an IndexError exception
        # this case will be caught and handled in the except clause
        name = ""
        while len(name) == 0:
            name = empire_names.pop()
    except:
        # in case of an error set name to empty string
        name = ""

    # check if we got a valid name...
    if len(name) == 0:
        # ...no (either the global list is exhausted, or an error occured)
        # in this case generate a random name instead
        name = random.choice(consonants).upper() + random.choice(vowels) + random.choice(consonants) + random.choice(vowels) + random.choice(consonants)

    return name

# This function checks if there are enough systems to give all
# players adequately-separated homeworlds, and increases the
# number of systems accordingly if not 
def adjust_universe_size(size, total_players):
    min_sys = total_players*3;
    if size < min_sys:
        return min_sys
    else:
        return size

# Calculate positions for the "Python Test" galaxy shape
# Being the original guy I am, I just create a grid... ;)
def test_galaxy_calc_positions(positions, size, width):
    for y in xrange(int(width*0.1), int(width*0.9), int((width*0.8) / math.sqrt(size))):
        for x in xrange(int(width*0.1), int(width*0.9), int((width*0.8) / math.sqrt(size))):
            positions.append(fo.SystemPosition(float(x), float(y)))



# Generate a new star system at the specified position
#def generateSystem(position, name = ""):
def pick_star_type():

    # try to pick a star type by making a series of "rolls" (1-100)
    # for each star type, and take the highest modified roll
    star_type = fo.starType.unknown
    try:
        max_roll = 0
        for candidate in star_types:
            roll = random.randint(1, 100) + fo.universeAgeModToStarTypeDist(gsd.age, candidate) + fo.baseStarTypeDist(candidate)
            if max_roll < roll:
                max_roll = roll
                star_type = candidate
    except:
        # in case of an error play save and set star type to invalid
        star_type = fo.starType.unknown
        print "Python generateSystem: Pick star type failed"
        print sys.exc_info()[1]

    # if we got an invalid star type (for whatever reason),
    # just select one randomly from the global tuple
    if star_type == fo.starType.unknown:
        star_type = random.choice(star_types)

    # get a name, create and insert the system into the universe
    # and return ID of the newly created system
    #return fo.createSystem(star_type, (name or get_star_name()), position.x, position.y)
    return star_type

# Calculate planet size for a potential new planet based on
# planet density setup option, star type and orbit number
def calc_planet_size(star_type, orbit):

    # try to pick a planet size by making a series of "rolls" (1-100)
    # for each planet size, and take the highest modified roll
    planet_size = fo.planetSize.unknown
    try:
        max_roll = 0        
        for candidate in planet_sizes_all:
            roll = random.randint(1, 100) \
                + fo.densityModToPlanetSizeDist(gsd.planetDensity, candidate) \
                + fo.starTypeModToPlanetSizeDist(star_type, candidate) \
                + fo.orbitModToPlanetSizeDist(orbit, candidate) \
                + fo.galaxyShapeModToPlanetSizeDist(gsd.shape, candidate)
            if max_roll < roll:
                max_roll = roll
                planet_size = candidate
    except:
        # in case of an error play save and set planet size to invalid
        planet_size = fo.planetSize.unknown
        print "Python calc_planet_size: Pick planet size failed"
        print sys.exc_info()[1]

    # if we got an invalid planet size (for whatever reason),
    # just select one randomly from the global tuple based
    # only on the planet density setup option
    if planet_size == fo.planetSize.unknown:
        if random.randint(1, 10) <= gsd.planetDensity:
            planet_size = random.choice(planet_sizes)
        else:
            planet_size = fo.planetSize.noWorld

    return planet_size

# Calculate planet type for a potential new planet
# TEMP: For now, pick planet type randomly, unless it is required by size
# TODO: Consider using the universe tables that modify planet type again,
#       this has been (temporarily?) disabled in C code. But the respective
#       tables are there, the Python interface to them is in place, and
#       this function is already prepared to take all necessary parameters.
#       So if anyone feels like experimenting, go for it... :)
def calc_planet_type(star_type, orbit, planet_size):

    planet_type = fo.planetType.unknown

    # check specified planet size to determine if we want a planet at all
    if  planet_size in planet_sizes:
        # if yes, determine planet type based on planet size...
        if planet_size == fo.planetSize.gasGiant:
            planet_type = fo.planetType.gasGiant
        elif planet_size == fo.planetSize.asteroids:
            planet_type = fo.planetType.asteroids
        else:
            planet_type = random.choice(planet_types)

    return planet_type

# Generate a new planet in specified system and orbit
def generate_planet(planet_size, planet_type, system, orbit):
    try:
        planet = fo.createPlanet(planet_size, planet_type, system, orbit, "")
    except:
        planet = fo.invalidObject();
        print "Python generate_planet: Create planet failed"
        print sys.exc_info()[1]
    return planet

# Sets the names of the planets of the specified system
# planet name is system name + planet number (as roman number)
# unless it's an asteroid belt, in that case name is system
# name + "asteroid belt" (localized)
def name_planets(system):
    planet_number = 1
    # iterate over all planets in the system
    for planet in fo.sysGetPlanets(system):
        # use different naming methods for "normal" planets and asteroid belts
        if fo.planetGetType(planet) == fo.planetType.asteroids:
            # get localized text from stringtable
            name = fo.userString("PL_ASTEROID_BELT_OF_SYSTEM")
            # %1% parameter in the localized string is the system name
            name = name.replace("%1%", fo.getName(system))
        else:
            # set name to system name + planet number as roman number...
            name = fo.getName(system) + " " + fo.romanNumber(planet_number)
            # ...and increase planet number
            planet_number = planet_number + 1
        # do the actual renaming
        fo.setName(planet, name)

# Checks if a system is too close to the other home systems
# Home systems should be at least 200 units (linear distance)
# and 2 jumps apart 
def is_too_close_to_other_home_systems(system, home_systems):
    for home_system in home_systems:
        if fo.linearDistance(system, home_system) < 200:
            return True
        elif fo.jumpDistance(system, home_system) < 2:
            return True
    return False

# Generates a list with a requested number of home systems
# choose the home systems from the specified list
def generate_home_system_list(num_home_systems, systems):

    # id the list of systems to choose home systems from is empty, raise an exception
    if len(systems) == 0:
        err_msg = "Python generate_home_system_list: no systems to choose from"
        print err_msg
        raise Exception(err_msg)
    
    # initialize list of home systems
    home_systems = []

    # loop and get a new home systems until we have the requested number
    while len(home_systems) < num_home_systems:

        # try to choose a system until too many attempty failed or a system has been found
        attempts = 0
        found = False
        while (attempts < 100) and not found:
            attempts = attempts + 1
            # randomly choose one system from the list we got
            candidate = random.choice(systems)
            # for the first 50 attempts, only consider systems with "real" stars and at least one planet
            # if we haven't found a system after 50 attempts, consider all systems
            if (attempts < 50):
                if fo.sysGetStarType(candidate) not in real_star_types:
                    continue
                if len(fo.sysGetPlanets(candidate)) == 0:
                    continue
            # if our candidate is too close to the already choosen home systems, don't use it
            if is_too_close_to_other_home_systems(candidate, home_systems):
                continue
            # if our candidate passed the above tests, add it to our list
            home_systems.append(candidate)
            found = True

        # if no system could be found, just attempt to find one that's not
        # already a home system and disregard any other conditions
        if not found:
            print "Couldn't find homeworld #", len(home_systems) + 1, "after 100 attempts, just trying to find one now that's not already a home system and disregard any other condition"
            attempts = 0
            while (attempts < 50) and not found:
                attempts = attempts +1
                # again, choose one system from the list we got
                candidate = random.choice(systems)
                # but now just check if it has already been choosen as home system
                if candidate in home_systems:
                    # if yes, try again
                    continue
                # if our candidate passed the test, add it to our list
                home_systems.append(candidate)
                found = True

        # if we still haven't found a suitable system, our galaxy obviously is too crowded
        # in that case, throw a fit, em, exception ;)
        if not found:
            raise Exception("Python generate_home_system_list: requested %d homeworlds in a galaxy with %d systems, aborting" % (num_home_systems, len(systems)))

        # if choosen system has no "real" star, change star type to a randomly selected "real" star
        if fo.sysGetStarType(candidate) not in real_star_types:
            star_type = random.choice(real_star_types)
            print "Home system #", len(home_systems), "has star type", fo.sysGetStarType(candidate), ", changing that to", star_type
            fo.sysSetStarType(candidate, star_type)

        # if choosen system has no planets, create one in a random orbit
        # we take random values for type and size, as these will be
        # set to suitable values later
        if len(fo.sysGetPlanets(candidate)) == 0:
            print "Home system #", len(home_systems), "has no planets, adding one"
            if generate_planet(random.choice(real_planet_sizes), random.choice(planet_types), candidate, random.randint(0, fo.sysGetNumOrbits(candidate) - 1)) == fo.invalidObject():
                # generate planet failed, throw an exception
                raise Exception("Python generate_home_system_list: couldn't create planet in home system")

    return home_systems

# Sets up various aspects of an empire, like empire name, homeworld, etc.
def setup_empire(empire, empire_name, home_system, starting_species, player_name):

    # set empire name, if no one is given, pick one randomly
    if empire_name == "":
        print "No empire name set for player", player_name, ", picking one randomly"
        empire_name = get_empire_name()
    fo.empireSetName(empire, empire_name)
    print "Empire name for player", player_name, "is", empire_name

    # check starting species, if no one is given, pick one randomly
    if starting_species == "":
        print "No starting species set for player", player_name, ", picking one randomly"
        starting_species = random.choice(fo.getPlayableSpecies())
    print "Starting species for player", player_name, "is", starting_species

    # pick a planet from the specified home system as homeworld
    planets = fo.sysGetPlanets(home_system)
    # if the system is empty, throw an exception
    if len(planets) == 0:
        raise Exception("Python setup_empire: got home system with no planets")
    homeworld = random.choice(planets)

    # set selected planet as empire homeworld with selected starting species
    fo.empireSetHomeworld(empire, homeworld, starting_species)

    # set homeworld focus
    # check if the preferred focus for the starting species is among
    # the foci available on the homeworld planet
    available_foci = fo.planetAvailableFoci(homeworld)
    preferred_focus = fo.speciesPreferredFocus(starting_species)
    if preferred_focus in available_foci:
        # if yes, set the homeworld focus to the preferred focus
        print "Player", player_name, ": setting preferred focus", preferred_focus, "on homeworld"
        fo.planetSetFocus(homeworld, preferred_focus)
    elif len(available_foci) > 0:
        # if no, and there is at least one available focus,
        # just take the first of the list
        if preferred_focus == "":
            print "Player", player_name, ": starting species", starting_species, "has no preferred focus, using", available_foci[0], "instead"
        else:
            print "Player", player_name, ": preferred focus", preferred_focus, "for starting species", starting_species, "not available on homeworld, using", available_foci[0], "instead"
        fo.planetSetFocus(homeworld, available_foci[0])
    else:
        # if no focus is available on the homeworld, don't set any focus
        print "Player", player_name, ": no available foci on homeworld for starting species", starting_species

    # give homeworld starting buildings
    # use the list provided in starting_buildings.txt
    print "Player", player_name, ": add starting buildings to homeworld"
    for building in load_string_list("../starting_buildings.txt"):
        fo.createBuilding(building, homeworld, empire)

    # unlock starting techs, buildings, hulls, ship parts, etc.
    # use content file preunlocked_items.txt
    print "Player", player_name, ": add unlocked items"
    for item in fo.loadItemSpecList("preunlocked_items.txt"):
        fo.empireUnlockItem(empire, item.type, item.name)

    # add premade ship designs to empire
    print "Player", player_name, ": add premade ship designs"
    for ship_design in fo.designGetPremadeList():
        fo.empireAddShipDesign(empire, ship_design)

    # add starting fleets to empire
    # use content file starting_fleets.txt
    print "Player", player_name, ": add starting fleets"
    fleet_plans = fo.loadFleetPlanList("starting_fleets.txt")
    for fleet_plan in fleet_plans:
        # first, create the fleet
        fleet = fo.createFleet(fleet_plan.name(), home_system, empire)
        # if the fleet couldn't be created, throw an exception
        if fleet == fo.invalidObject():
            raise Exception("Python setup empire: couldn't create fleet " + fleet_plan.name())
        # second, iterate over the list of ship design names in the fleet plan
        for ship_design in fleet_plan.shipDesigns():
            # create a ship in the fleet
            # if the ship couldn't be created, throw an exception
            if fo.createShip("", ship_design, starting_species, fleet) == fo.invalidObject:
                raise Exception("Python setup empire: couldn't create ship " + ship_design + " for fleet " + fleet_plan.name())

# used in clustering
def recalc_centers(ctrs, points, assignments):
    tallies = [ [[],[]] for ctr in ctrs ]
    for index_p, index_ctr in enumerate(assignments):
        tallies[index_ctr][0].append(points[index_p][0])
        tallies[index_ctr][1].append(points[index_p][1])
    for index_ctr, tally in enumerate(tallies):
        num_here = len(tally[0])
        if num_here==0:
            pass # leave ctr unchanged if no points assigned to it last round
        else:
            ctrs[index_ctr] = [float(sum(tally[0]))/num_here, float(sum(tally[1]))/num_here]

# used in clustering
def assign_clusters(points, ctrs):
    assignments = []
    for point in points:
        best_dist_sqr = 1e20
        best_ctr=0
        for index_ctr, ctr in enumerate(ctrs):
            this_dist_sqr = (ctr[0] - point[0])**2 + (ctr[1] - point[1])**2
            if this_dist_sqr < best_dist_sqr:
                best_dist_sqr = this_dist_sqr
                best_ctr = index_ctr
        assignments.append( best_ctr )
    return assignments

# returns a list, same size as positions argument, containing indices from 0 to num_star_groups
def cluster_stars(positions, num_star_groups):
    if num_star_groups > len(positions):
        return [ [pos] for pos in positions ]
    centers = [[pos.x, pos.y] for pos in random.sample(positions, num_star_groups)]
    all_coords = [(pos.x, pos.y) for pos in positions]
    clusters = [[], []]
    clusters[0] = assign_clusters(all_coords, centers)#assign clusters based on init centers
    oldC = 0
    for loop in range(1): #main loop to try getting some convergence of center assignments
        recalc_centers(centers, all_coords, clusters[oldC])#get new centers
        clusters[1-oldC] = assign_clusters(all_coords, centers)#assign clusters based on new centers
        if clusters[0] == clusters[1]:
            break# stop iterating if no change in cluster assignments
        oldC = 1-oldC
    else:
        if loop > 0: #if here at loop 0, then didn't try for convergence
            print "falling through system clustering iteration loop without convergence"
        pass
    return clusters[1-oldC]
    
def check_deep_space(group_list, star_type_assignments, planet_size_assignments):
    deep_space=[]
    not_deep=[]
    for systemxy in group_list:
        if star_type_assignments.get(systemxy, fo.starType.noStar) != fo.starType.noStar:
            not_deep.append(systemxy)
        else:
            for psize in planet_size_assignments.get(systemxy,{}).values():
                if psize != fo.planetSize.noWorld:
                    not_deep.append(systemxy)
                    break
            else:
                deep_space.append(systemxy)
    return not_deep,  deep_space

def name_group(group_list, group_name, star_type_assignments, planet_size_assignments):
    group_size = len(group_list)
    if group_size == 1:
        return [(group_list[0], group_name)]
    modifiers = list(stargroup_modifiers) # copy the list so we can safely add to it if needed
    not_deep,  deep_space = check_deep_space(group_list, star_type_assignments, planet_size_assignments)
    these_systems = not_deep + deep_space #so that unnamed deep space will get the later starg group modifiers
    while len(modifiers) < group_size: #emergency fallback
        trial_mod = random.choice(stargroup_modifiers) + " " + "".join(random.sample(consonants + vowels, 3)).upper()
        if trial_mod not in modifiers:
            modifiers.append( trial_mod )
    if postfix_stargroup_modifiers:
        name_list = [ group_name + " " + modifier for modifier in modifiers ]
    else:
        name_list = [ modifier + " " + group_name for modifier in modifiers ]
    return zip(these_systems, name_list)

def create_universe():

    print "Python Universe Generator"

    # fetch universe and player setup data
    global gsd, psd_list
    gsd = fo.getGalaxySetupData()
    psd_list = fo.getPlayerSetupData()
    total_players = len(psd_list)

    # initialize RNG
    random.seed(gsd.seed)

    # store list of possible star system names in global container
    global star_names
    star_names = get_name_list("STAR_NAMES")
    # randomly shuffle the list so we don't get the names always
    # in the same order when we pop names from the list later
    random.shuffle(star_names)
    
    # make sure there are enough systems for the given number of players 
    print "Universe creation requested with %d systems for %d players" % (gsd.size, total_players)
    new_size = adjust_universe_size(gsd.size, total_players)
    if new_size > gsd.size:
        gsd.size = new_size
        print "Too few systems for the requested number of players, number of systems adjusted accordingly"
    print "Creating universe with %d systems for %d players" % (gsd.size, total_players)

    # get typical width for universe based on number of systems
    width = fo.calcTypicalUniverseWidth(gsd.size)
    fo.setUniverseWidth(width)
    print "Set universe width to", width

    # Calling universe generator helper functions to calculate positions
    # for the requested galaxy shape and number of systems
    system_positions = fo.SystemPositionVec()
    if gsd.shape == fo.galaxyShape.random:
        gsd.shape = random.choice(galaxy_shapes)
    if gsd.shape == fo.galaxyShape.spiral2:
        fo.spiralGalaxyCalcPositions(system_positions, 2, gsd.size, width, width)
    elif gsd.shape == fo.galaxyShape.spiral3:
        fo.spiralGalaxyCalcPositions(system_positions, 3, gsd.size, width, width)
    elif gsd.shape == fo.galaxyShape.spiral4:
        fo.spiralGalaxyCalcPositions(system_positions, 4, gsd.size, width, width)
    elif gsd.shape == fo.galaxyShape.elliptical:
        fo.ellipticalGalaxyCalcPositions(system_positions, gsd.size, width, width)
    elif gsd.shape == fo.galaxyShape.cluster:
        # Typically a galaxy with 100 systems should have ~5 clusters
        avg_clusters = gsd.size / 20
        if avg_clusters < 2:
            avg_clusters = 2
        # Add a bit of random variation (+/- 20%)
        clusters = random.randint((avg_clusters * 8) / 10, (avg_clusters * 12) / 10)
        if clusters >= 2:
            fo.clusterGalaxyCalcPositions(system_positions, clusters, gsd.size, width, width)
    elif gsd.shape == fo.galaxyShape.ring:
        fo.ringGalaxyCalcPositions(system_positions, gsd.size, width, width)
    elif gsd.shape == fo.galaxyShape.test:
        test_galaxy_calc_positions(system_positions, gsd.size, width)
    # Check if any positions have been calculated...
    if len(system_positions) <= 0:
        # ...if not, fall back on irregular shape
        gsd.shape = fo.galaxyShape.irregular
        fo.irregularGalaxyPositions(system_positions, gsd.size, width, width)
    gsd.size = len(system_positions)
    print gsd.shape, "galaxy created, final number of systems:", gsd.size

    # choose star types and planet sizes, before choosting names, so naming can have special handling of Deep Space
    star_type_assignments = {}
    planet_size_assignments = {}
    planet_count_dist = {}
    for position in system_positions:
        star_type = pick_star_type() # needed to determine planet size
        star_type_assignments[(position.x, position.y)] = star_type
        these_planets = {}
        planet_count = 0
        for orbit in range(0, system_orbits):
            # check for each orbit if a planet shall be created by determining planet size
            planet_size = calc_planet_size(star_type, orbit)
            these_planets[orbit] = planet_size
            if planet_size in planet_sizes:
                planet_count += 1
        planet_size_assignments[(position.x, position.y)] = these_planets
        planet_count_dist.setdefault(planet_count, [0])[0] += 1
    print "\n Planet Count Distribution: planets_in_system | num_systems"
    for planet_count,sys_count in planet_count_dist.items():
        print "\t\t\t%2d  | %5d"%(planet_count, sys_count[0])
    print

    # will name name a portion of stars on a group basis, where the stars of each group share the same base star name, 
    # suffixed by different (default greek) letters or characters (options at top of file)
    star_name_map = {}
    group_names = get_name_list("STAR_GROUP_NAMES")
    potential_group_names = []
    individual_names = []
    stargroup_words[:] = get_name_list("STAR_GROUP_WORDS")
    stargroup_chars[:] = get_name_list("STAR_GROUP_CHARS")
    stargroup_modifiers[:] = [stargroup_words, stargroup_chars][ star_groups_use_chars ]
    for starname in star_names:
        if len(starname) > 6: #if starname is long, don't allow it for groups
            individual_names.append(starname)
            continue
        # any names that already have a greek letter in them can only be used for individual stars, not groups
        for namepart in starname.split():
            if namepart in greek_letters:
                individual_names.append(starname)
                break
        else:
            potential_group_names.append(starname)

    if potential_group_names == []:
        potential_group_names.append("XYZZY")

    # ensure at least a portion of galaxy gets individual starnames
    num_systems = len(system_positions)
    target_indiv_ratio = [target_indiv_ratio_small, target_indiv_ratio_large][ num_systems >= naming_large_galaxy_size ]
    #TODO improve the following calc to be more likely to hit target_indiv_ratio if more or less than 50% potential_group_names used for groups
    num_individual_stars = int(max(min( num_systems * target_indiv_ratio, 
                               len(individual_names)+int(0.5*len(potential_group_names))),
                               num_systems - 0.8*len(stargroup_modifiers)*(len(group_names)+int(0.5*len(potential_group_names)))))
    star_group_size = 1 + int( (num_systems-num_individual_stars)/(max(1, len(group_names)+int(0.5*len(potential_group_names)))))
    # make group size a bit bigger than min necessary, at least a trio
    star_group_size = max(3, star_group_size)
    num_star_groups = 1 + int(num_systems/star_group_size) #initial value
    #print "num_individual_stars:", num_individual_stars, "star group size: ", star_group_size, "num_star_groups", num_star_groups
    #print "num indiv names:", len(individual_names), "num group_names:", len(group_names), "num potential_group_names:", len(potential_group_names)

    # first cluster all systems, then remove some to be individually named (otherwise groups can have too many individually
    # named systems in their middle).  First remove any that are too small (only 1 or 2 systems).  The clusters with the most systems 
    # are generally the most closely spaced, and though they might make good logical candidates for groups, their names are then prone 
    # to overlapping on the galaxy map, so after removing small groups, remove the groups with the most systems.
    position_list = list(system_positions)
    random.shuffle(position_list) #just to be sure it is randomized
    init_cluster_assgts = cluster_stars(position_list, num_star_groups)
    star_groups = {}
    for index_pos, index_group in enumerate(init_cluster_assgts):
        this_pos = position_list[index_pos]
        star_groups.setdefault(index_group, []).append( (this_pos.x, this_pos.y) )
    indiv_systems = []
    
    # remove groups with only one non-deep-system
    for groupindex, group_list in star_groups.items():
        max_can_transfer = len(potential_group_names)-len(star_groups)+len(individual_names)-len(indiv_systems)
        if max_can_transfer <= 0:
            break
        elif max_can_transfer <= len(group_list):
            continue
        not_deep,  deep_space = check_deep_space(group_list, star_type_assignments, planet_size_assignments)
        if len(not_deep) > 1:
            continue
        for systemxy in star_groups[groupindex]:
            indiv_systems.append(systemxy)
        del star_groups[groupindex]

    # remove tiny groups
    group_sizes = [(len(group), index) for index,group in star_groups.items()]
    group_sizes.sort()
    while (len(indiv_systems) < num_individual_stars) and len(group_sizes)>0:
        groupsize, groupindex = group_sizes.pop()
        max_can_transfer = len(potential_group_names)-len(star_groups)+len(individual_names)-len(indiv_systems)
        if (max_can_transfer <= 0) or (groupsize > 2):
            break
        if max_can_transfer <= groupsize:
            continue
        for systemxy in star_groups[groupindex]:
            indiv_systems.append(systemxy)
        del star_groups[groupindex]
        
    # remove largest (likely most compact) groups
    while (len(indiv_systems) < num_individual_stars) and len(group_sizes)>0:
        groupsize, groupindex = group_sizes.pop(-1)
        max_can_transfer = len(potential_group_names)-len(star_groups)+len(individual_names)-len(indiv_systems)
        if max_can_transfer <= 0:
            break
        if max_can_transfer <= groupsize:
            continue
        for systemxy in star_groups[groupindex]:
            indiv_systems.append(systemxy)
        del star_groups[groupindex]

    num_star_groups = len(star_groups)
    num_individual_stars = len(indiv_systems)
    random.shuffle(potential_group_names)
    random.shuffle(individual_names)
    random.shuffle(group_names)
    num_for_indiv = min(max(len(potential_group_names)/2, num_individual_stars+1-len(individual_names)),len(potential_group_names))
    individual_names.extend( potential_group_names[:num_for_indiv])
    group_names.extend( potential_group_names[num_for_indiv:])

    #print "sampling for %d indiv names from list of %d total indiv names"%(num_individual_stars, len(individual_names))
    indiv_name_sample = random.sample(individual_names, num_individual_stars)
    #indiv_name_assignments = zip([(pos.x, pos.y) for pos in position_list[:num_individual_stars]], indiv_name_sample)
    indiv_name_assignments = zip(indiv_systems, indiv_name_sample)
    star_name_map.update( indiv_name_assignments )
    #print "sampling for %d group names from list of %d total group names"%(num_star_groups, len(group_names))
    group_name_sample = random.sample(group_names, num_star_groups)
    for index_group, group_list in enumerate(star_groups.values()):
        star_name_map.update( name_group(group_list, group_name_sample[index_group], star_type_assignments, planet_size_assignments) )

    # Generate and populate systems
    systems = []
    planet_type_summary = {}
    for position in system_positions:
        systemxy = (position.x, position.y)
        star_type = star_type_assignments.get(systemxy, fo.starType.noStar)
        star_name = star_name_map.get(systemxy, "") or get_star_name()
        system = fo.createSystem(star_type, star_name, position.x, position.y)
        systems.append(system)
        these_planets = planet_size_assignments[systemxy]
        for orbit in range(0, fo.sysGetNumOrbits(system)):
            # check for each orbit if a planet shall be created by determining planet size
            planet_size = these_planets.get(orbit, fo.planetSize.noWorld)
            if planet_size in planet_sizes:
                # ok, we want a planet, determine planet type and generate the planet
                planet_type = calc_planet_type(star_type, orbit, planet_size)
                planet_type_summary.setdefault(planet_type, [0])[0]+=1
                generate_planet(planet_size, planet_type, system, orbit)
    print len(systems), "systems generated and populated"
    planet_type_names=dict(zip(planet_types_all, ["Swamp", "Radiated", "Toxic", "Inferno", "Barren", "Tundra", "Desert", "Terran", "Ocean", "Asteroids", "Gas Giant"]))
    planet_total = sum([type_tally[0] for type_tally in planet_type_summary.values()])
    print "\nPlanet Type Summary for a total of %d placed planets"%(planet_total)
    for planet_type in planet_type_summary:
        print "\t %12s: %.1f%%"%(planet_type_names[planet_type], (100.0*planet_type_summary.get(planet_type,[0])[0])/planet_total)
    print
    # generate Starlanes
    fo.generateStarlanes(gsd.starlaneFrequency)
    print "Starlanes generated"

    print "Generate list of home systems..."
    home_systems = generate_home_system_list(total_players, systems)
    print "...systems choosen:", home_systems
    
    # store list of possible empire names in global container
    global empire_names
    print "Load list of empire names..."
    empire_names = get_name_list("EMPIRE_NAMES")
    print "...", len(empire_names), "names loaded"
    # randomly shuffle the list so we don't get the names always
    # in the same order when we pop names from the list later
    random.shuffle(empire_names)
    
    # set up empires for each player
    for psd_entry in psd_list:
        empire = psd_entry.key()
        psd = psd_entry.data()
        home_system = home_systems.pop()
        setup_empire(empire, psd.empireName, home_system, psd.startingSpecies, psd.playerName)

    # iterate over all systems and name their planets
    # this needs to be done after empire home systems have been set, as
    # during that process asteroid belts might be changed into planets,
    # and we need to know the final type of a planet to name it
    print "Set planet names"
    for system in systems:
        name_planets(system)

    print "Python Universe Generator completed"
