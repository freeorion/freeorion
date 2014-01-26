import sys
import random
import math
import os

from pprint import pprint

import foUniverseGenerator as fo



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

# global lists for star system names, empire names
star_names = []
empire_names = []

# Reads a list of strings from a content file
def loadStringList(file_name):
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
def getNameList(name_list):
    return fo.userString(name_list).splitlines()

# Returns a random star system name
def getStarName():
    # try to get a name from the global list
    try:
        # pop names from the list until we get an non-empty string
        # this ensures empty lines in the starnames file are skipped
        # if the list is exhausted, this will raise an IndexError exception
        # this case will be caught and handled in the except clause
        name = ""
        while len(name) == 0:
            name = star_names.pop()
    except:
        print sys.exc_info()[1]
        # in case of an error set name to empty string
        name = ""

    # check if we got a valid name...
    if len(name) == 0:
        # ...no (either the global list is exhausted, or an error occured)
        # in this case generate a random name instead
        for i in range(0, 3):
            name = name + random.choice(consonants + vowels).upper()
        name = name + "-" + str(random.randint(1000, 9999))

    return name

# Returns a random empire name
def getEmpireName():
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
def adjustUniverseSize(size, total_players):
    min_sys = total_players*3;
    if size < min_sys:
        return min_sys
    else:
        return size

# Calculate positions for the "Python Test" galaxy shape
# Being the original guy I am, I just create a grid... ;)
def testGalaxyCalcPositions(positions, size, width):
    for y in xrange(int(width*0.1), int(width*0.9), int((width*0.8) / math.sqrt(size))):
        for x in xrange(int(width*0.1), int(width*0.9), int((width*0.8) / math.sqrt(size))):
            positions.append(fo.SystemPosition(float(x), float(y)))

# Generate a new star system at the specified position
def generateSystem(position):

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
    return fo.createSystem(star_type, getStarName(), position.x, position.y)

# Calculate planet size for a potential new planet based on
# planet density setup option, star type and orbit number
def calcPlanetSize(star_type, orbit):

    # try to pick a planet size by making a series of "rolls" (1-100)
    # for each planet size, and take the highest modified roll
    planet_size = fo.planetSize.unknown
    try:
        max_roll = 0        
        for candidate in planet_sizes_all:
            roll = random.randint(1, 100) \
                + fo.densityModToPlanetSizeDist(gsd.planetDensity, candidate) \
                + fo.starTypeModToPlanetSizeDist(star_type, candidate) \
                + fo.orbitModToPlanetSizeDist(orbit, candidate)
            if max_roll < roll:
                max_roll = roll
                planet_size = candidate
    except:
        # in case of an error play save and set planet size to invalid
        planet_size = fo.planetSize.unknown
        print "Python calcPlanetSize: Pick planet size failed"
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
def calcPlanetType(star_type, orbit, planet_size):

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
def generatePlanet(planet_size, planet_type, system, orbit):
    try:
        planet = fo.createPlanet(planet_size, planet_type, system, orbit, "")
    except:
        planet = fo.invalidObject();
        print "Python generatePlanet: Create planet failed"
        print sys.exc_info()[1]
    return planet

# Sets the names of the planets of the specified system
# planet name is system name + planet number (as roman number)
# unless it's an asteroid belt, in that case name is system
# name + "asteroid belt" (localized)
def namePlanets(system):
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
def isTooCloseToOtherHomeSystems(system, home_systems):
    for home_system in home_systems:
        if fo.linearDistance(system, home_system) < 200:
            return True
        elif fo.jumpDistance(system, home_system) < 1:
            return True
    return False

# Generates a list with a requested number of home systems
# choose the home systems from the specified list
def generateHomeSystemList(num_home_systems, systems):

    # id the list of systems to choose home systems from is empty, raise an exception
    if len(systems) == 0:
        err_msg = "Python generateHomeSystemList: no systems to choose from"
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
            if isTooCloseToOtherHomeSystems(candidate, home_systems):
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
            raise Exception("Python generateHomeSystemList: requested %d homeworlds in a galaxy with %d systems, aborting" % (num_home_systems, len(systems)))

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
            if generatePlanet(random.choice(real_planet_sizes), random.choice(planet_types), candidate, random.randint(0, fo.sysGetNumOrbits(candidate) - 1)) == fo.invalidObject():
                # generate planet failed, throw an exception
                raise Exception("Python generateHomeSystemList: couldn't create planet in home system")

    return home_systems

# Sets up various aspects of an empire, like empire name, homeworld, etc.
def setupEmpire(empire, empire_name, home_system, starting_species, player_name):

    # set empire name, if no one is given, pick one randomly
    if empire_name == "":
        print "No empire name set for player", player_name, ", picking one randomly"
        empire_name = getEmpireName()
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
        raise Exception("Python setupEmpire: got home system with no planets")
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
    for building in loadStringList("starting_buildings.txt"):
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

def createUniverse():

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
    star_names = getNameList("STAR_NAMES")
    # randomly shuffle the list so we don't get the names always
    # in the same order when we pop names from the list later
    random.shuffle(star_names)
    
    # make sure there are enough systems for the given number of players 
    print "Universe creation requested with %d systems for %d players" % (gsd.size, total_players)
    new_size = adjustUniverseSize(gsd.size, total_players)
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
        testGalaxyCalcPositions(system_positions, gsd.size, width)
    # Check if any positions have been calculated...
    if len(system_positions) <= 0:
        # ...if not, fall back on irregular shape
        gsd.shape = fo.galaxyShape.irregular
        fo.irregularGalaxyPositions(system_positions, gsd.size, width, width)
    gsd.size = len(system_positions)
    print gsd.shape, "galaxy created, final number of systems:", gsd.size

    # Generate and populate systems
    systems = []
    for position in system_positions:
        system = generateSystem(position)
        systems.append(system)
        star_type = fo.sysGetStarType(system) # needed to determine planet size (and maybe in future also type?)
        for orbit in range(0, fo.sysGetNumOrbits(system)):
            # check for each orbit if a planet shall be created by determining planet size
            planet_size = calcPlanetSize(star_type, orbit)
            if planet_size in planet_sizes:
                # ok, we want a planet, determine planet type and generate the planet
                planet_type = calcPlanetType(star_type, orbit, planet_size)
                generatePlanet(planet_size, planet_type, system, orbit)
    print len(systems), "systems generated and populated"

    # generate Starlanes
    fo.generateStarlanes(gsd.starlaneFrequency)
    print "Starlanes generated"

    print "Generate list of home systems..."
    home_systems = generateHomeSystemList(total_players, systems)
    print "...systems choosen:", home_systems
    
    # store list of possible empire names in global container
    global empire_names
    print "Load list of empire names..."
    empire_names = getNameList("EMPIRE_NAMES")
    print "...", len(empire_names), "names loaded"
    # randomly shuffle the list so we don't get the names always
    # in the same order when we pop names from the list later
    random.shuffle(empire_names)
    
    # set up empires for each player
    for psd_entry in psd_list:
        empire = psd_entry.key()
        psd = psd_entry.data()
        home_system = home_systems.pop()
        setupEmpire(empire, psd.empireName, home_system, psd.startingSpecies, psd.playerName)

    # iterate over all systems and name their planets
    # this needs to be done after empire home systems have been set, as
    # during that process asteroid belts might be changed into planets,
    # and we need to know the final type of a planet to name it
    print "Set planet names"
    for system in systems:
        namePlanets(system)

    print "Python Universe Generator completed"
