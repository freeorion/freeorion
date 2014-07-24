import random
import fo_universe_generator as fo
import util


# tuple of galaxy shapes to randomly choose from when shape is "random"
shapes = (fo.galaxyShape.spiral2,    fo.galaxyShape.spiral3,     fo.galaxyShape.spiral4,
          fo.galaxyShape.cluster,    fo.galaxyShape.elliptical,  fo.galaxyShape.ring,
          fo.galaxyShape.irregular1, fo.galaxyShape.irregular2)


class AdjacencyGrid:
    def __init__(self, universe_width):
        self.min_dist = fo.min_system_separation()
        self.cell_size = max(universe_width / 50, self.min_dist)
        self.width = int(universe_width / self.cell_size) + 1
        self.grid = [[[] for _ in range(self.width)] for _ in range(self.width)]
        print "Adjacency Grid: width", self.width, ", cell size", self.cell_size

    def insert_pos(self, pos):
        self.grid[int(pos.x / self.cell_size)][int(pos.y / self.cell_size)].append(pos)

    def too_close_to_other_positions(self, x, y):
        """
        Checks if the specified position is too close to the positions stored in the grid
        """
        cell_x = int(x / self.cell_size)
        cell_y = int(y / self.cell_size)
        upper_left_x = max(0, cell_x - 1)
        upper_left_y = max(0, cell_y - 1)
        lower_right_x = min(self.width - 1, cell_x + 1)
        lower_right_y = min(self.width - 1, cell_y + 1)
        return any(util.distance(pos.x, pos.y, x, y) < self.min_dist for cx in range(upper_left_x, lower_right_x + 1)
                   for cy in range(upper_left_y, lower_right_y + 1) for pos in self.grid[cx][cy])


def irregular2_galaxy_calc_positions(positions, size, width):
    """
    Calculate positions for the 'Python Test' galaxy shape
    """
    adjacency_grid = AdjacencyGrid(width)
    max_delta = max(min(float(fo.max_starlane_length()), width / 10.0), adjacency_grid.min_dist * 2.0)
    print "test galaxy shape: max delta distance =", max_delta
    origin_x, origin_y = width / 2.0, width / 2.0
    prev_x, prev_y = origin_x, origin_y
    reset_to_origin = 0
    for n in range(size):
        attempts = 100
        found = False
        while (attempts > 0) and not found:
            attempts -= 1
            x = prev_x + random.uniform(-max_delta, max_delta)
            y = prev_y + random.uniform(-max_delta, max_delta)
            if util.distance(x, y, origin_x, origin_y) > width * 0.45:
                prev_x, prev_y = origin_x, origin_y
                reset_to_origin += 1
                continue
            found = not adjacency_grid.too_close_to_other_positions(x, y)
            if attempts % 10:
                prev_x, prev_y = x, y
        if found:
            pos = fo.SystemPosition(x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
        prev_x, prev_y = x, y
    print "Reset to origin", reset_to_origin, "times"


def calc_star_system_positions(shape, size):
    """
    Calculates list of positions (x, y) for a given galaxy shape,
    number of systems and width
    Uses universe generator helper functions provided by the API
    """

    # calculate typical width for universe based on number of systems
    width = fo.calc_typical_universe_width(size)
    if shape == fo.galaxyShape.irregular2:
        width *= 1.4
    fo.set_universe_width(width)
    print "Set universe width to", width

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
    elif shape == fo.galaxyShape.irregular2:
        irregular2_galaxy_calc_positions(positions, size, width)

    # Check if any positions have been calculated...
    if not positions:
        # ...if not, fall back on irregular1 shape
        fo.irregular_galaxy_positions(positions, size, width, width)

    return positions
