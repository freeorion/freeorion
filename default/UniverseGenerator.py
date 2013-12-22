import sys
import random
import math
import os

import foUniverseGenerator as fo


# tuple of galaxy shapes to randomly choose from when shape is "random"
galaxy_shapes = (fo.galaxyShape.spiral2,    fo.galaxyShape.spiral3,     fo.galaxyShape.spiral4,
                 fo.galaxyShape.cluster,    fo.galaxyShape.elliptical,  fo.galaxyShape.ring,
                 fo.galaxyShape.irregular,  fo.galaxyShape.test)

# tuple of available star types
star_types = (fo.starType.blue, fo.starType.white,   fo.starType.yellow,    fo.starType.orange,
              fo.starType.red,  fo.starType.neutron, fo.starType.blackHole, fo.starType.noStar)

# list of star system names
star_names = []

# Reads list of star system names from content file
# and stores them in a global container
def loadStarNames():
    global star_names
    try:
        with open("starnames.txt", "r") as f:
            star_names = f.read().splitlines()
            # randomly shuffle the list so we don't get the names always
            # in the same order when we pop names from the list later
            random.shuffle(star_names)
    except:
        print "Unable to access starnames.txt"
        star_names = []

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
        # in case of an error set name to empty string
        name = ""

    # check if we got a valid name...
    if len(name) == 0:
        # ...no (either the global list is exhausted, or an error occured)
        # in this case generate a random name instead
        for i in range(0, 3):
            name = name + chr(random.randint(ord("A"), ord("Z")))
        name = name + "-" + str(random.randint(1000, 9999))

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
        print "Python generateSystem(): Pick star type failed"
        print sys.exc_info()[1]

    # if we got an invalid star type (for whatever reason),
    # just select one randomly from the global tuple
    if star_type == fo.starType.unknown:
        star_type = random.choice(star_types)

    # get a name, create and insert the system into the universe
    # and return ID of the newly created system
    return fo.createSystem(star_type, getStarName(), position.x, position.y)

def createUniverse():

    print "Python Universe Generator"
    print "Current directory:", os.getcwd()

    # fetch universe and player setup data
    global gsd, psd
    gsd = fo.getGalaxySetupData()
    psd = fo.getPlayerSetupData()
    total_players = len(psd)

    # initialize RNG
    random.seed(gsd.seed)

    # store list of possible star system names in global container
    loadStarNames()
    
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
    print "Set universe width to %f" % width

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

    # Generate systems at the calculated positions
    for position in system_positions:
        generateSystem(position)

    # Let UniverseGenerator::CreateUniverse do the rest that hasn't been implemented
    # in the Python universe generator yet
    fo.createUniverse(gsd.size,              gsd.shape,           gsd.age,
                      gsd.starlaneFrequency, gsd.planetDensity,   gsd.specialsFrequency,
                      gsd.monsterFrequency,  gsd.nativeFrequency, system_positions,
                      psd)

    print "Python Universe Generator completed"
