import sys
from random import random, uniform, randint, gauss, choice
from math import pi, sin, cos, sqrt

import freeorion as fo
import util
import universe_tables


# tuple of galaxy shapes to randomly choose from when shape is "random"
shapes = (fo.galaxyShape.spiral2,    fo.galaxyShape.spiral3,     fo.galaxyShape.spiral4,
          fo.galaxyShape.cluster,    fo.galaxyShape.elliptical,  fo.galaxyShape.disc,
          fo.galaxyShape.ring,       fo.galaxyShape.box,         fo.galaxyShape.irregular)


class AdjacencyGrid:
    def __init__(self, universe_width):
        self.min_dist = universe_tables.MIN_SYSTEM_SEPARATION
        self.cell_size = max(universe_width / 50, self.min_dist)
        self.width = int(universe_width / self.cell_size) + 1
        self.grid = [[[] for _ in range(self.width)] for _ in range(self.width)]
        print "Adjacency Grid: width", self.width, ", cell size", self.cell_size

    def insert_pos(self, pos):
        self.grid[int(pos[0] / self.cell_size)][int(pos[1] / self.cell_size)].append(pos)

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
        return any(util.distance(pos[0], pos[1], x, y) < self.min_dist for cx in range(upper_left_x, lower_right_x + 1)
                   for cy in range(upper_left_y, lower_right_y + 1) for pos in self.grid[cx][cy])


def calc_universe_width(shape, size):
    """
    Calculates typical universe width based on galaxy shape and number of star systems.
    A 150 star universe should be 1000 units across.
    """
    width = (1000.0 / sqrt(150.0)) * sqrt(float(size))
    if shape in [fo.galaxyShape.elliptical, fo.galaxyShape.irregular]:
        width *= 1.4
    elif shape == fo.galaxyShape.disc:
        width *= 1.2
    return width


def get_systems_within_jumps(origin_system, jumps):
    """
    Returns all systems within jumps jumps of system origin_system (including origin_system).
    If jumps is 0, return list that only contains system origin_system.
    If jumps is negative, return empty list.
    """
    if jumps < 0:
        return []
    matching_systems = [origin_system]
    next_origin_systems = [origin_system]
    while jumps > 0:
        origin_systems = list(next_origin_systems)
        next_origin_systems = []
        for system in origin_systems:
            neighbor_systems = [s for s in fo.sys_get_starlanes(system) if s not in matching_systems]
            next_origin_systems.extend(neighbor_systems)
            matching_systems.extend(neighbor_systems)
        jumps -= 1
    return matching_systems


def spiral_galaxy_calc_positions(positions, arms, size, width):
    """
    Calculate positions for the spiral galaxy shapes.
    """
    adjacency_grid = AdjacencyGrid(width)
    arm_offset = uniform(0.0, 2.0 * pi)
    arm_angle = (2.0 * pi) / float(arms)
    arm_spread = (0.3 * pi) / float(arms)
    arm_length = 1.5 * pi
    center = 0.25

    for i in range(size):
        attempts = 100
        while attempts > 0:
            radius = random()
            if radius < center:
                angle = uniform(0.0, 2.0 * pi)
                x = radius * cos(arm_offset + angle)
                y = radius * sin(arm_offset + angle)
            else:
                arm = randint(0, arms - 1) * arm_angle
                angle = gauss(0.0, arm_spread)
                x = radius * cos(arm_offset + arm + angle + radius * arm_length)
                y = radius * sin(arm_offset + arm + angle + radius * arm_length)

            x = (x + 1) * width / 2.0
            y = (y + 1) * width / 2.0
            if (x < 0.0) or (width <= x) or (y < 0.0) or (width <= y):
                continue

            # see if new star is too close to any existing star.
            # if so, we try again or give up
            if adjacency_grid.too_close_to_other_positions(x, y):
                attempts -= 1
                continue

            # add the new star location
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
            break

        if not attempts:
            print "Spiral galaxy shape: giving up on placing star", i,\
                  ", can't find position sufficiently far from other systems"


def elliptical_galaxy_calc_positions(positions, size, width):
    """
    Calculate positions for the elliptical galaxy shape.
    """
    adjacency_grid = AdjacencyGrid(width)
    ellipse_width_vs_height = uniform(0.4, 0.6)
    rotation = uniform(0.0, pi)
    rotation_sin = sin(rotation)
    rotation_cos = cos(rotation)
    gap_constant = 0.95
    gap_size = 1.0 - gap_constant * gap_constant * gap_constant

    # random number generators
    radius_dist = lambda: uniform(0.0, gap_constant)
    random_angle  = lambda: uniform(0.0, 2.0 * pi)

    for i in range(size):
        attempts = 100
        while attempts > 0:
            radius = radius_dist()
            # adjust for bigger density near center and create gap
            radius = radius * radius * radius + gap_size
            angle  = random_angle()

            # rotate for individual angle and apply elliptical shape
            x1 = radius * cos(angle)
            y1 = radius * sin(angle) * ellipse_width_vs_height

            # rotate for ellipse angle
            x = x1 * rotation_cos - y1 * rotation_sin
            y = x1 * rotation_sin + y1 * rotation_cos

            # move from [-1.0, 1.0] universe coordinates
            x = (x + 1.0) * width / 2.0
            y = (y + 1.0) * width / 2.0

            # discard stars that are outside boundaries (due to possible rounding errors)
            if (x < 0) or (x >= width) or (y < 0) or (y >= width):
                attempts -= 1
                continue

            # see if new star is too close to any existing star; if so, we try again
            if adjacency_grid.too_close_to_other_positions(x, y):
                attempts -= 1
                continue

            # add the new star location
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
            break

        if not attempts:
            print "Elliptical galaxy shape: giving up on placing star", i,\
                  ", can't find position sufficiently far from other systems"


def disc_galaxy_calc_positions(positions, size, width):
    """
    Calculate positions for the disc galaxy shape.
    """
    adjacency_grid = AdjacencyGrid(width)
    center_x, center_y = width / 2.0, width / 2.0

    for i in range(size):
        attempts = 100
        while attempts > 0:
            radius = uniform(0.0, width / 2.0)
            angle = uniform (0.0, 2.0 * pi)
            x = center_x + radius * cos(angle)
            y = center_y + radius * sin(angle)

            if (x < 0) or (width <= x) or (y < 0) or (width <= y):
                attempts -= 1
                continue

            # see if new star is too close to any existing star; if so, we try again
            if adjacency_grid.too_close_to_other_positions(x, y):
                attempts -= 1
                continue

            # add the new star location
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
            break

        if not attempts:
            print "Disc galaxy shape: giving up on placing star", i,\
                  ", can't find position sufficiently far from other systems"


def cluster_galaxy_calc_positions(positions, clusters, size, width):
    """
    Calculate positions for the cluster galaxy shape.
    """
    if size < 1:
        print >> sys.stderr, "Cluster galaxy requested for 0 stars"
        return
    if clusters < 1:
        print >> sys.stderr, "Cluster galaxy requested for 0 clusters, defaulting to 1"
        clusters = 1

    adjacency_grid = AdjacencyGrid(width)

    # probability of systems which don't belong to a cluster
    system_noise = 0.15
    ellipse_width_vs_height = uniform(0.2,0.5)
    # a list of tuples of tuples: first tuple holds cluster position,
    # second tuple stores help values for cluster rotation (sin,cos)
    clusters_position = []

    random_angle = lambda: uniform(0.0, 2.0*pi)

    for i in range(clusters):
        attempts = 100
        while attempts > 0:
            # prevent cluster position near borders (and on border)
            x = ((random() * 2.0 - 1.0) / (clusters + 1.0)) * clusters
            y = ((random() * 2.0 - 1.0) / (clusters + 1.0)) * clusters
            # ensure all clusters have a min separation to each other (search isn't optimized, not worth the effort)
            if all(((cp[0][0] - x) * (cp[0][0] - x) + (cp[0][1] - y) * (cp[0][1] - y)) > (2.0/clusters)
                   for cp in clusters_position):
                break
            attempts -= 1
        rotation = uniform(0.0, pi)
        clusters_position.append(((x, y), (sin(rotation),cos(rotation))))

    for i in range(size):
        attempts = 100
        while attempts > 0:
            if random() < system_noise:
                x = random() * 2.0 - 1.0
                y = random() * 2.0 - 1.0
            else:
                cluster = clusters_position[i % len(clusters_position)]
                radius = random()
                angle = random_angle()

                x1 = radius * cos(angle)
                y1 = radius * sin(angle) * ellipse_width_vs_height

                x = x1 * cluster[1][1] + y1 * cluster[1][0]
                y = (-x1) * cluster[1][0] + y1 * cluster[1][1]

                x = x / sqrt(float(clusters)) + cluster[0][0]
                y = y / sqrt(float(clusters)) + cluster[0][1]

            x = (x + 1) * width / 2.0
            y = (y + 1) * width / 2.0

            if (x < 0) or (width <= x) or (y < 0) or (width <= y):
                attempts -= 1
                continue

            # see if new star is too close to any existing star; if so, we try again
            if adjacency_grid.too_close_to_other_positions(x, y):
                attempts -= 1
                continue

            # add the new star location
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
            break

        if not attempts:
            print "Cluster galaxy shape: giving up on placing star", i,\
                  ", can't find position sufficiently far from other systems"


def ring_galaxy_calc_positions(positions, size, width):
    """
    Calculate positions for the ring galaxy shape.
    """
    adjacency_grid = AdjacencyGrid(width)
    ring_width = width / 4.0
    ring_radius = (width - ring_width) / 2.0

    theta_dist = lambda: uniform(0.0, 2.0 * pi)
    radius_dist = lambda: gauss(ring_radius, ring_width / 3.0)

    for i in range(size):
        attempts = 100
        while attempts > 0:
            theta = theta_dist()
            radius = radius_dist()

            x = width / 2.0 + radius * cos(theta)
            y = width / 2.0 + radius * sin(theta)

            if (x < 0) or (width <= x) or (y < 0) or (width <= y):
                attempts -= 1
                continue

            # see if new star is too close to any existing star; if so, we try again
            if adjacency_grid.too_close_to_other_positions(x, y):
                attempts -= 1
                continue

            # add the new star location
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
            break

        if not attempts:
            print "Ring galaxy shape: giving up on placing star", i,\
                  ", can't find position sufficiently far from other systems"


def box_galaxy_calc_positions(positions, size, width):
    """
    Calculate positions for the box galaxy shape.
    """
    adjacency_grid = AdjacencyGrid(width)

    for i in range(size):
        attempts = 100
        while attempts > 0:
            x = width * random()
            y = width * random()

            if (x < 0) or (width <= x) or (y < 0) or (width <= y):
                attempts -= 1
                continue

            # see if new star is too close to any existing star; if so, we try again
            if adjacency_grid.too_close_to_other_positions(x, y):
                attempts -= 1
                continue

            # add the new star location
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
            break

        if not attempts:
            print "Box galaxy shape: giving up on placing star", i,\
                  ", can't find position sufficiently far from other systems"


def irregular_galaxy_calc_positions(positions, size, width):
    """
    Calculate positions for the irregular galaxy shape.
    """
    adjacency_grid = AdjacencyGrid(width)
    max_delta = max(min(float(universe_tables.MAX_STARLANE_LENGTH), width / 10.0), adjacency_grid.min_dist * 2.0)
    print "Irregular galaxy shape: max delta distance =", max_delta
    origin_x, origin_y = width / 2.0, width / 2.0
    prev_x, prev_y = origin_x, origin_y
    reset_to_origin = 0
    for n in range(size):
        attempts = 100
        found = False
        while (attempts > 0) and not found:
            attempts -= 1
            x = prev_x + uniform(-max_delta, max_delta)
            y = prev_y + uniform(-max_delta, max_delta)
            if util.distance(x, y, origin_x, origin_y) > width * 0.45:
                prev_x, prev_y = origin_x, origin_y
                reset_to_origin += 1
                continue
            found = not adjacency_grid.too_close_to_other_positions(x, y)
            if attempts % 10:
                prev_x, prev_y = x, y
        if found:
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
        prev_x, prev_y = x, y
    print "Reset to origin", reset_to_origin, "times"


def recalc_universe_width(positions):
    """
    Recalculates the universe width. This is done by shifting all positions by a delta so too much "extra space"
    beyond the uppermost, lowermost, leftmost and rightmost positions is cropped, and adjust the universe width
    accordingly.

    Returns the new universe width and the recalculated positions.
    """
    print "Recalculating universe width..."
    # first, get the uppermost, lowermost, leftmost and rightmost positions
    # (these are those with their x or y coordinate closest to or farthest away from the x or y axis)
    min_x = min(positions, key=lambda p: p[0])[0]
    min_y = min(positions, key=lambda p: p[1])[1]
    max_x = max(positions, key=lambda p: p[0])[0]
    max_y = max(positions, key=lambda p: p[1])[1]
    print "...the leftmost system position is at x coordinate", min_x
    print "...the uppermost system position is at y coordinate", min_y
    print "...the rightmost system position is at x coordinate", max_x
    print "...the lowermost system position is at y coordinate", max_y

    # calculate the actual universe width by determining the width and height of an rectangle that encompasses all
    # positions, and take the greater of the two as the new actual width for the universe
    # also add a constant value to the width so we have some small space around
    width = max_x - min_x
    height = max_y - min_y
    actual_width = max(width, height) + 20.0
    print "...recalculated universe width:", actual_width

    # shift all positions so the entire map is centered in a quadratic box of the width we just calculated
    # this box defines the extends of our universe
    delta_x = ((actual_width - width) / 2) - min_x
    delta_y = ((actual_width - height) / 2) - min_y
    print "...shifting all system positions by", delta_x, "/", delta_y
    new_positions = [(p[0] + delta_x, p[1] + delta_y) for p in positions]

    print "...the leftmost system position is now at x coordinate", min(new_positions, key=lambda p: p[0])[0]
    print "...the uppermost system position is now at y coordinate", min(new_positions, key=lambda p: p[1])[1]
    print "...the rightmost system position is now at x coordinate", max(new_positions, key=lambda p: p[0])[0]
    print "...the lowermost system position is now at y coordinate", max(new_positions, key=lambda p: p[1])[1]

    return actual_width, new_positions


def calc_star_system_positions(shape, size):
    """
    Calculates list of positions (x, y) for a given galaxy shape,
    number of systems and width
    Uses universe generator helper functions provided by the API
    """
    # if shape is "random", randomly pick a galaxy shape
    if shape == fo.galaxyShape.random:
        shape = choice(shapes)

    # calculate typical width for universe based on number of systems
    width = calc_universe_width(shape, size)
    print "Set universe width to", width
    fo.set_universe_width(width)

    positions = []

    print "Creating", shape, "galaxy shape"
    if shape == fo.galaxyShape.spiral2:
        spiral_galaxy_calc_positions(positions, 2, size, width)
    elif shape == fo.galaxyShape.spiral3:
        spiral_galaxy_calc_positions(positions, 3, size, width)
    elif shape == fo.galaxyShape.spiral4:
        spiral_galaxy_calc_positions(positions, 4, size, width)
    elif shape == fo.galaxyShape.elliptical:
        elliptical_galaxy_calc_positions(positions, size, width)
    elif shape == fo.galaxyShape.disc:
        disc_galaxy_calc_positions(positions, size, width)
    elif shape == fo.galaxyShape.cluster:
        # Typically a galaxy with 100 systems should have ~5 clusters
        avg_clusters = size / 20
        if avg_clusters < 2:
            avg_clusters = 2
        # Add a bit of random variation (+/- 20%)
        clusters = randint((avg_clusters * 8) / 10, (avg_clusters * 12) / 10)
        if clusters >= 2:
            cluster_galaxy_calc_positions(positions, clusters, size, width)
    elif shape == fo.galaxyShape.ring:
        ring_galaxy_calc_positions(positions, size, width)
    elif shape == fo.galaxyShape.irregular:
        irregular_galaxy_calc_positions(positions, size, width)

    # Check if any positions have been calculated...
    if not positions:
        # ...if not, fall back on box shape
        box_galaxy_calc_positions(positions, size, width)

    # to avoid having too much "extra space" around the system positions of our galaxy map, recalculate the universe
    # width and shift all positions accordingly
    width, positions = recalc_universe_width(positions)
    print "Set universe width to", width
    fo.set_universe_width(width)

    return positions
