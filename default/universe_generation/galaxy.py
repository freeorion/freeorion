import math
import random
import foUniverseGenerator as fo


# tuple of galaxy shapes to randomly choose from when shape is "random"
shapes = (fo.galaxyShape.spiral2,    fo.galaxyShape.spiral3,     fo.galaxyShape.spiral4,
          fo.galaxyShape.cluster,    fo.galaxyShape.elliptical,  fo.galaxyShape.ring,
          fo.galaxyShape.irregular,  fo.galaxyShape.test)


def test_galaxy_calc_positions(positions, size, width):
    """
    Calculate positions for the 'Python Test' galaxy shape
    Being the original guy I am, I just create a grid... ;)
    """
    for y in xrange(int(width*0.1), int(width*0.9), int((width*0.8) / math.sqrt(size))):
        for x in xrange(int(width*0.1), int(width*0.9), int((width*0.8) / math.sqrt(size))):
            positions.append(fo.SystemPosition(float(x), float(y)))


def calc_star_system_positions(shape, size, width):
    """
    Calculates list of positions (x, y) for a given galaxy shape,
    number of systems and width
    Uses universe generator helper functions provided by the API
    """

    positions = fo.SystemPositionVec()
    if shape == fo.galaxyShape.random:
        shape = random.choice(shapes)

    if shape == fo.galaxyShape.spiral2:
        fo.spiral_galaxy_calc_positions(positions, 2, size, width, width)
    elif shape == fo.galaxyShape.spiral3:
        fo.spiral_galaxy_calc_positions(positions, 3, size, width, width)
    elif shape == fo.galaxyShape.spiral4:
        fo.spiral_galaxy_calc_positions(positions, 4, size, width, width)
    elif shape == fo.galaxyShape.elliptical:
        fo.elliptical_galaxy_calc_positions(positions, size, width, width)
    elif shape == fo.galaxyShape.cluster:
        # Typically a galaxy with 100 systems should have ~5 clusters
        avg_clusters = size / 20
        if avg_clusters < 2:
            avg_clusters = 2
        # Add a bit of random variation (+/- 20%)
        clusters = random.randint((avg_clusters * 8) / 10, (avg_clusters * 12) / 10)
        if clusters >= 2:
            fo.cluster_galaxy_calc_positions(positions, clusters, size, width, width)
    elif shape == fo.galaxyShape.ring:
        fo.ring_galaxy_calc_positions(positions, size, width, width)
    elif shape == fo.galaxyShape.test:
        test_galaxy_calc_positions(positions, size, width)

    # Check if any positions have been calculated...
    if not positions:
        # ...if not, fall back on irregular shape
        shape = fo.galaxyShape.irregular
        fo.irregular_galaxy_positions(positions, size, width, width)

    return positions