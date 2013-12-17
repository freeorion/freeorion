import sys
import random
import math

import foUniverseGenerator as fo


# tuple of galaxy shapes to randomly choose from when shape is "random"
galaxy_shapes = (fo.galaxyShape.spiral2,    fo.galaxyShape.spiral3,     fo.galaxyShape.spiral4,
                 fo.galaxyShape.cluster,    fo.galaxyShape.elliptical,  fo.galaxyShape.ring,
                 fo.galaxyShape.irregular,  fo.galaxyShape.test)

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

def createUniverse():

    print "Python Universe Generator"

    # get universe object
    universe = fo.getUniverse()
    # fetch universe and player setup data
    gsd = fo.getGalaxySetupData()
    psd = fo.getPlayerSetupData()
    total_players = len(psd)

    # initialize RNG
    random.seed(gsd.seed)
    
    # make sure there are enough systems for the given number of players 
    print "Universe creation requested with %d systems for %d players" % (gsd.size, total_players)
    new_size = adjustUniverseSize(gsd.size, total_players)
    if new_size > gsd.size:
        gsd.size = new_size
        print "Too few systems for the requested number of players, number of systems adjusted accordingly"
    print "Creating universe with %d systems for %d players" % (gsd.size, total_players)

    # get typical width for universe based on number of systems
    width = fo.calcTypicalUniverseWidth(gsd.size)
    universe.width = width
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

    # Let UniverseGenerator::CreateUniverse do the rest that hasn't been implemented
    # into the Python universe generator yet
    fo.createUniverse(universe,
                      gsd.size,              gsd.shape,           gsd.age,
                      gsd.starlaneFrequency, gsd.planetDensity,   gsd.specialsFrequency,
                      gsd.monsterFrequency,  gsd.nativeFrequency, system_positions,
                      psd)

    print "Python Universe Generator completed"
