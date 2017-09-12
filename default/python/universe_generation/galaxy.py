import sys
from collections import defaultdict
from math import acos, ceil, cos, floor, pi, sin, sqrt
from random import gauss, randint, random, uniform

import freeorion as fo

import universe_tables
import util


class AdjacencyGrid(object):
    def __init__(self, universe_width):
        assert universe_tables.MIN_SYSTEM_SEPARATION < universe_tables.MAX_STARLANE_LENGTH
        self.min_dist = universe_tables.MIN_SYSTEM_SEPARATION
        self.max_dist = universe_tables.MAX_STARLANE_LENGTH
        self.cell_size = min(max(universe_width / 50, self.min_dist), self.max_dist / sqrt(2))
        self.width = int(universe_width / self.cell_size) + 1
        self.grid = defaultdict(set)
        print "Adjacency Grid: width {}, cell size {}".format(self.width, self.cell_size)

    def cell(self, pos):
        """Returns cell index."""
        return int(pos[0] / self.cell_size), int(pos[1] / self.cell_size)

    def insert_pos(self, pos):
        """Inserts pos."""
        self.grid[self.cell(pos)].add(pos)

    def remove_pos(self, pos):
        """Removes pos."""
        self.grid[self.cell(pos)].discard(pos)

    def _square_indices_containing_cell(self, (cell_x, cell_y), radius):
        """Return a square of indices containing cell."""
        upper_left_x = max(0, cell_x - radius)
        upper_left_y = max(0, cell_y - radius)
        lower_right_x = min(self.width - 1, cell_x + radius)
        lower_right_y = min(self.width - 1, cell_y + radius)
        return [(x, y) for x in range(upper_left_x, lower_right_x + 1)
                for y in range(upper_left_y, lower_right_y + 1)]

    def _generate_rings_around_point(self, pos, max_radius=None):
        """Yields successive rings of indices containing cell."""
        cell = self.cell(pos)
        i_radius = 0
        i_radius_max = ceil(max_radius / self.cell_size) if max_radius else (self.width + 1)
        outer = set()
        ring = {1}
        while ring and i_radius <= i_radius_max:
            inner = outer
            outer = set(self._square_indices_containing_cell(cell, i_radius))
            ring = outer - inner
            yield ring
            i_radius += 1

    def neighbors(self, p):
        """Return all neighbors within max_dist."""
        _neighbors = []
        i_radius_close_enough = floor(self.max_dist / self.cell_size)
        for i, ring in enumerate(self._generate_rings_around_point(p, self.max_dist)):
            if i <= i_radius_close_enough:
                _neighbors += [pos for cell in ring for pos in self.grid[cell]]
            else:
                _neighbors += [pos for cell in ring for pos in self.grid[cell] if
                               util.distance(pos, p) <= self.max_dist]
        return _neighbors

    def nearest_neighbor(self, p):
        """ Return nearest neighbor at any distance."""
        for ring in self._generate_rings_around_point(p):
            candidates = [pos for cell in ring for pos in self.grid[cell]]
            if candidates:
                (_, pt) = min((util.distance(pos, p), pos) for pos in candidates)
                return pt
        return None

    def too_close_to_other_positions(self, pos):
        """
        Checks if the specified position is too close to the positions stored in the grid
        """
        pos_cell = self.cell(pos)
        return any(util.distance(p2, pos) < self.min_dist
                   for p2_cell in self._square_indices_containing_cell(pos_cell, 1)
                   for p2 in self.grid[p2_cell])


class DSet(object):
    """
    A set of pos that is disjoint (not connected) to other sets.

    If the set is of one item then it has no parent. parent equals None.
    Otherwise the set points to the root/representative item of the set via parent.

    A private child class of DisjointSets to store the parent and rank of a disjoint set object
    """
    def __init__(self, pos):
        self.pos = pos

        # stores None or the parent
        self.parent = None

        # starts at 0, never decreases and only increases
        # when this object is the parent and two equal rank sets are merged
        self.rank = 0

    def bind_parent(self, parent):
        """Bind to parent."""
        assert self is not parent
        self.parent = parent

    def inc_rank(self):
        """Increment rank."""
        self.rank += 1


class DisjointSets(object):
    """
    A set of disjoint sets.

    It supports the operations of:
    add(pos)      -- Creates a new single item set containing pos if it isn't already a set. O(1)
    link(p1, p2)  -- Links two sets containing p1 and p2 together.
                     This will add(p1) and add(p2) if necessary. O(1)
    root(pos)     -- Gets the root/representative object for the set.
                     Creates a new set with pos if not already a set. O(1)
    complete_sets -- Returns a list of list of all sets. O(n)

    See https://en.wikipedia.org/wiki/Disjoint-set_data_structure
    """
    def __init__(self):
        self.dsets = {}

    def add(self, pos):
        """Creates a new single item set containing pos if it isn't already a set. O(1)"""
        if pos not in self.dsets:
            self.dsets[pos] = DSet(pos)

    def link(self, p1, p2):
        """ Creates a link between the sets containing p1 and p2"""
        root1 = self.root(p1)
        if p1 == p2:
            # print " self link"
            return
        root2 = self.root(p2)
        if root1 == root2:
            # print " same discrete set link"
            return
        ds1 = self.dsets[root1]
        ds2 = self.dsets[root2]
        assert not ds1.parent and not ds2.parent

        # Always attach the "smaller" lower rank as the child of the "larger" higher rank tree
        if ds1.rank > ds2.rank:
            # print " left link"
            ds2.bind_parent(ds1)
        else:
            # print " right link"
            ds1.bind_parent(ds2)
            if ds1.rank == ds2.rank:
                # print " even link"
                ds2.inc_rank()

    def root(self, pos):
        """
        Returns the key position of the cluster containing pos
        Adds pos if absent
        """
        root = self._has_root(pos)
        if root:
            return root[0]
        self.add(pos)
        return pos

    def _has_root(self, pos):
        """
        Check if pos is the root of a cluster otherwise shorten
        the tree while tranversing it and return the root
        """
        # traverse tree and fetch parents for compression
        def parents(p1, children):
            # print "pp p1 ", p1.pos, " children a ", children
            if p1.parent:
                # print "pp deeper ", p1.pos, " chlidren b ", children
                children.append(p1)
                return parents(p1.parent, children)
            else:
                # print "pp done p1 ", p1.pos, " children c ", children
                return (p1, children)

        if pos not in self.dsets:
            return []

        (root, children) = parents(self.dsets[pos], [])
        # squash the chain of children
        for child in children:
            child.bind_parent(root)
        return [root.pos]

    def complete_sets(self):
        """ returns a list of list of all sets O(n)."""
        ret = defaultdict(list)
        for pos in self.dsets.keys():
            ret[self.root(pos)].append(pos)
        return ret.values()


class Clusterer(object):
    """
    Computes all clusters of positions separated by more than MAX_STARLANE_LENGTH

    This creates a disjoint set of all of the positions
    Adds all of the seperations less than MAX_STARLANE_LENGTH
    Then returns the clusters of stars when requested
    """
    def __init__(self, positions, adjacency_grid):
        self.adjacency_grid = adjacency_grid
        self.dsets = DisjointSets()

        for pos in positions:
            self._add(pos)

        self.clusters = set()
        for cluster in self.dsets.complete_sets():
            # Decorate each cluster with its length to speed sorting
            self.clusters.add((len(cluster), frozenset(cluster)))

    def __len__(self):
        return len(self.clusters)

    def _add(self, pos):
        """Adds pos."""
        for neighbor in self.adjacency_grid.neighbors(pos):
            # print "Clusterer adding ", pos,  " and ", neighbor
            self.dsets.link(pos, neighbor)

    def smallest_isolated_cluster(self):
        """
        Return None if all of the positions are in one cluster
        or the smallest cluster
        """
        # Sort (min) and return the undecorated cluster.
        return None if len(self.clusters) < 2 else min(self.clusters)[1]

    def stitch_clusters(self, p1, p2, stitches):
        """
        Stitch the clusters containing ''p1'' and ''p2''
        together with the positions in ''stitches''

        This assumes that the stitching is correctly formed.
        ''p1'' and ''p2'' are the closest positions between
        the clusters and the positions in ''stitches'' are only
        between ''p1'' and ''p2''

        After stitch_clusters there will be one fewer clusters in
        the clusterer, unless there were fewer than two clusters
        to begin with.

        If ''p1'' or ''p2'' are not in clusters then stitch_clusters
        just stitches two random clusters together.  This preserves
        the invariant that stitch_clusters always reduces the number
        of clusters.
        """
        len_dset1 = None
        len_dset2 = None
        for len_dset in self.clusters:
            if p1 in len_dset[1]:
                len_dset1 = len_dset
            if p2 in len_dset[1]:
                len_dset2 = len_dset

        if not len_dset1 or not len_dset2:
            util.report_error("p1 and p2 must be points in disjoint sets of positions")
            if len(self.clusters) < 2:
                return
            len_dset1, len_dset2 = list(self.clusters)[0:2]

        self.clusters.remove(len_dset1)
        self.clusters.remove(len_dset2)
        # Decorate the new cluster with its length to speed sorting
        new_set = len_dset1[1].union(len_dset2[1]).union(frozenset(stitches))
        self.clusters.add((len(new_set), new_set))


def stitching_positions(p1, p2):
    """
    Returns a list of positions between p1 and p2 between MIN_SYSTEM_SEPARATION and MAX_STARLANE_LENGTH apart
    """
    if 2 * universe_tables.MIN_SYSTEM_SEPARATION >= universe_tables.MAX_STARLANE_LENGTH:
        util.report_error("MAX_STARLANE_LENGTH must be twice MIN_SYSTEM_SEPARATION to "
                          "allow extra positions to be added to enforce MAX_STARLANE_LENGTH")
        return []

    max_dist = universe_tables.MAX_STARLANE_LENGTH
    min_dist = universe_tables.MIN_SYSTEM_SEPARATION
    p1_p2_dist = util.distance(p1, p2)
    if p1_p2_dist < max_dist:
        return []

    # Pick a random point in an 2:1 ratio ellipse and then rotate and slide it in between p1 and p2
    p1_p2_theta = acos((p2[0] - p1[0]) / p1_p2_dist)
    p1_p2_scale = (p1_p2_dist - 2*min_dist) / 2
    p1_p2_translation = ((p1[0] + p2[0]) / 2, (p1[1] + p2[1]) / 2)

    radius_0space = uniform(0.0, 1.0)
    angle_0space = uniform(0.0, 2.0 * pi)
    rotated_point = (radius_0space * p1_p2_scale * cos(angle_0space + p1_p2_theta),
                     radius_0space * p1_p2_scale * sin(angle_0space + p1_p2_theta))

    p3 = (rotated_point[0] + p1_p2_translation[0], rotated_point[1] + p1_p2_translation[1])

    return [p3] + stitching_positions(p1, p3) + stitching_positions(p2, p3)


def enforce_max_distance(positions, adjacency_grid):
    """
    Adds extra positions between groups of positions to guarantee
    that no groups of positions are separated from all other groups
    of positions by more than universe_tables.MAX_STARLANE_LENGTH

    Find all groups of positions, where every member is within
    MAX_STARLANE_LENGTH of at least one other member of the group,
    but all members of the group are more than MAX_STARLANE_LENGTH
    from all positions in other groups.

    For each group:
      - Remove it from the adjacency grid and find its nearest neighboring position.
      - Add positions between the nearest neighbor and the group to
        ensure every position has at least one neighbor within
        MAX_STARLANE_LENGTH.
    """
    # Find all clusters
    clusterer = Clusterer(positions, adjacency_grid)

    if len(clusterer) == 1:
        print "All systems positioned in a single connected cluster"
    else:
        print("{} clusters separated by more than the MAX_STARLANE_LENGTH."
              "  Starting to fill gaps.".format(len(clusterer)))

    while len(clusterer) > 1:
        smallest_cluster = clusterer.smallest_isolated_cluster()
        print("Searching for nearest neighbor position to a cluster "
              "with {} positions.".format(len(smallest_cluster)))
        for pos in smallest_cluster:
            adjacency_grid.remove_pos(pos)
        # Find nearest neighbour
        (_, p1, p2) = min([(util.distance(pos, nn), pos, nn)
                           for pos in smallest_cluster
                           for nn in [adjacency_grid.nearest_neighbor(pos)] if nn])

        for pos in smallest_cluster:
            adjacency_grid.insert_pos(pos)
        # Add extra positions
        extra_positions = stitching_positions(p1, p2)
        for i_extra, p3 in enumerate(extra_positions):
            print("Adding {} of {} extra positions at {} to enforce "
                  "max spacing between {} and {}.".format(
                      i_extra + 1, len(extra_positions), p3, p1, p2))
            adjacency_grid.insert_pos(p3)
            positions.append(p3)
        clusterer.stitch_clusters(p1, p2, extra_positions)

        if len(clusterer) == 1:
            print "All systems now positioned in a single connected cluster"
        else:
            print("{} clusters separated by more the MAX_STARLANE_LENGTH.  "
                  "Continuing to fill gaps.".format(len(clusterer)))


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


def spiral_galaxy_calc_positions(positions, adjacency_grid, arms, size, width):
    """
    Calculate positions for the spiral galaxy shapes.
    """
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
            if adjacency_grid.too_close_to_other_positions((x, y)):
                attempts -= 1
                continue

            # add the new star location
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
            break

        if not attempts:
            print("Spiral galaxy shape: giving up on placing star {}, can't "
                  "find position sufficiently far from other systems."
                  .format(i))


def elliptical_galaxy_calc_positions(positions, adjacency_grid, size, width):
    """
    Calculate positions for the elliptical galaxy shape.
    """
    ellipse_width_vs_height = uniform(0.4, 0.6)
    rotation = uniform(0.0, pi)
    rotation_sin = sin(rotation)
    rotation_cos = cos(rotation)
    gap_constant = 0.95
    gap_size = 1.0 - gap_constant * gap_constant * gap_constant

    for i in range(size):
        attempts = 100
        while attempts > 0:
            radius = uniform(0.0, gap_constant)
            # adjust for bigger density near center and create gap
            radius = radius * radius * radius + gap_size
            angle = uniform(0.0, 2.0 * pi)

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
            if adjacency_grid.too_close_to_other_positions((x, y)):
                attempts -= 1
                continue

            # add the new star location
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
            break

        if not attempts:
            print("Elliptical galaxy shape: giving up on placing star {}, "
                  "can't find position sufficiently far from other systems"
                  .format(i))


def disc_galaxy_calc_positions(positions, adjacency_grid, size, width):
    """
    Calculate positions for the disc galaxy shape.
    """
    center_x, center_y = width / 2.0, width / 2.0

    for i in range(size):
        attempts = 100
        while attempts > 0:
            radius = uniform(0.0, width / 2.0)
            angle = uniform(0.0, 2.0 * pi)
            x = center_x + radius * cos(angle)
            y = center_y + radius * sin(angle)

            if (x < 0) or (width <= x) or (y < 0) or (width <= y):
                attempts -= 1
                continue

            # see if new star is too close to any existing star; if so, we try again
            if adjacency_grid.too_close_to_other_positions((x, y)):
                attempts -= 1
                continue

            # add the new star location
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
            break

        if not attempts:
            print("Disc galaxy shape: giving up on placing star {}, can't find"
                  " position sufficiently far from other systems".format(i))


def cluster_galaxy_calc_positions(positions, adjacency_grid, size, width):
    """
    Calculate positions for the cluster galaxy shape.
    """
    if size < 1:
        print >> sys.stderr, "Cluster galaxy requested for less than 1 star"
        return

    if size == 1:
        pos = (width / 2.0, width / 2.0)
        positions.append(pos)
        return

    # Typically a galaxy with 100 systems should have ~5 clusters
    avg_clusters = max(2, round(size / 20))
    # Add a bit of random variation (+/- 20%)
    clusters = max(2, randint(round((avg_clusters * 8) / 10), round((avg_clusters * 12) / 10)))

    # probability of systems which don't belong to a cluster
    system_noise = 0.15
    ellipse_width_vs_height = uniform(0.2, 0.5)
    # a list of tuples of tuples: first tuple holds cluster position,
    # second tuple stores help values for cluster rotation (sin,cos)
    clusters_position = []

    for i in range(clusters):
        attempts = 100
        while attempts > 0:
            # prevent cluster position near borders (and on border)
            x = ((random() * 2.0 - 1.0) / (clusters + 1.0)) * clusters
            y = ((random() * 2.0 - 1.0) / (clusters + 1.0)) * clusters
            # ensure all clusters have a min separation to each other (search isn't optimized, not worth the effort)
            if all(((cp[0][0] - x) * (cp[0][0] - x) + (cp[0][1] - y) * (cp[0][1] - y)) > (2.0 / clusters)
                   for cp in clusters_position):
                break
            attempts -= 1
        rotation = uniform(0.0, pi)
        clusters_position.append(((x, y), (sin(rotation), cos(rotation))))

    for i in range(size):
        attempts = 100
        while attempts > 0:
            if random() < system_noise:
                x = random() * 2.0 - 1.0
                y = random() * 2.0 - 1.0
            else:
                cluster = clusters_position[i % len(clusters_position)]
                radius = random()
                angle = uniform(0.0, 2.0 * pi)

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
            if adjacency_grid.too_close_to_other_positions((x, y)):
                attempts -= 1
                continue

            # add the new star location
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
            break

        if not attempts:
            print("Cluster galaxy shape: giving up on placing star {}, can't "
                  "find position sufficiently far from other systems"
                  .format(i))


def ring_galaxy_calc_positions(positions, adjacency_grid, size, width):
    """
    Calculate positions for the ring galaxy shape.
    """
    ring_width = width / 4.0
    ring_radius = (width - ring_width) / 2.0

    for i in range(size):
        attempts = 100
        while attempts > 0:
            theta = uniform(0.0, 2.0 * pi)
            radius = gauss(ring_radius, ring_width / 3.0)

            x = width / 2.0 + radius * cos(theta)
            y = width / 2.0 + radius * sin(theta)

            if (x < 0) or (width <= x) or (y < 0) or (width <= y):
                attempts -= 1
                continue

            # see if new star is too close to any existing star; if so, we try again
            if adjacency_grid.too_close_to_other_positions((x, y)):
                attempts -= 1
                continue

            # add the new star location
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
            break

        if not attempts:
            print("Ring galaxy shape: giving up on placing star {}, can't"
                  " find position sufficiently far from other systems"
                  .format(i))


def box_galaxy_calc_positions(positions, adjacency_grid, size, width):
    """
    Calculate positions for the box galaxy shape.
    """
    for i in range(size):
        attempts = 100
        while attempts > 0:
            x = width * random()
            y = width * random()

            if (x < 0) or (width <= x) or (y < 0) or (width <= y):
                attempts -= 1
                continue

            # see if new star is too close to any existing star; if so, we try again
            if adjacency_grid.too_close_to_other_positions((x, y)):
                attempts -= 1
                continue

            # add the new star location
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
            break

        if not attempts:
            print("Box galaxy shape: giving up on placing star {}, can't find"
                  " position sufficiently far from other systems".format(i))


def irregular_galaxy_calc_positions(positions, adjacency_grid, size, width):
    """
    Calculate positions for the irregular galaxy shape.
    """
    max_delta = max(min(float(universe_tables.MAX_STARLANE_LENGTH), width / 10.0), adjacency_grid.min_dist * 2.0)
    print "Irregular galaxy shape: max delta distance = {}".format(max_delta)
    origin_x, origin_y = width / 2.0, width / 2.0
    prev_x, prev_y = origin_x, origin_y
    reset_to_origin = 0
    for _ in range(size):
        attempts = 100
        found = False
        while (attempts > 0) and not found:
            attempts -= 1
            x = prev_x + uniform(-max_delta, max_delta)
            y = prev_y + uniform(-max_delta, max_delta)
            if util.distance((x, y), (origin_x, origin_y)) > width * 0.45:
                prev_x, prev_y = origin_x, origin_y
                reset_to_origin += 1
                continue
            found = not adjacency_grid.too_close_to_other_positions((x, y))
            if attempts % 10:
                prev_x, prev_y = x, y
        if found:
            pos = (x, y)
            adjacency_grid.insert_pos(pos)
            positions.append(pos)
        prev_x, prev_y = x, y
    print "Reset to origin {} times".format(reset_to_origin)


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
    print("...the leftmost system position is at x coordinate {}"
          .format(min_x))
    print("...the uppermost system position is at y coordinate {}"
          .format(min_y))
    print("...the rightmost system position is at x coordinate {}"
          .format(max_x))
    print("...the lowermost system position is at y coordinate {}"
          .format(max_y))

    # calculate the actual universe width by determining the width and height of an rectangle that encompasses all
    # positions, and take the greater of the two as the new actual width for the universe
    # also add a constant value to the width so we have some small space around
    width = max_x - min_x
    height = max_y - min_y
    actual_width = max(width, height) + 20.0
    print "...recalculated universe width: {}".format(actual_width)

    # shift all positions so the entire map is centered in a quadratic box of the width we just calculated
    # this box defines the extends of our universe
    delta_x = ((actual_width - width) / 2) - min_x
    delta_y = ((actual_width - height) / 2) - min_y
    print "...shifting all system positions by {}/{}".format(delta_x, delta_y)
    new_positions = [(p[0] + delta_x, p[1] + delta_y) for p in positions]

    print("...the leftmost system position is now at x coordinate {}"
          .format(min(new_positions, key=lambda p: p[0])[0]))
    print("...the uppermost system position is now at y coordinate {}"
          .format(min(new_positions, key=lambda p: p[1])[1]))
    print("...the rightmost system position is now at x coordinate {}"
          .format(max(new_positions, key=lambda p: p[0])[0]))
    print("...the lowermost system position is now at y coordinate {}"
          .format(max(new_positions, key=lambda p: p[1])[1]))

    return actual_width, new_positions


def calc_star_system_positions(gsd):
    """
    Calculates list of positions (x, y) for a given galaxy shape,
    number of systems and width
    Uses universe generator helper functions provided by the API
    """

    # calculate typical width for universe based on number of systems
    width = calc_universe_width(gsd.shape, gsd.size)
    print "Set universe width to {}".format(width)
    fo.set_universe_width(width)

    positions = []
    adjacency_grid = AdjacencyGrid(width)

    print "Creating {} galaxy shape".format(gsd.shape)
    if gsd.shape == fo.galaxyShape.spiral2:
        spiral_galaxy_calc_positions(positions, adjacency_grid, 2, gsd.size, width)
    elif gsd.shape == fo.galaxyShape.spiral3:
        spiral_galaxy_calc_positions(positions, adjacency_grid, 3, gsd.size, width)
    elif gsd.shape == fo.galaxyShape.spiral4:
        spiral_galaxy_calc_positions(positions, adjacency_grid, 4, gsd.size, width)
    elif gsd.shape == fo.galaxyShape.elliptical:
        elliptical_galaxy_calc_positions(positions, adjacency_grid, gsd.size, width)
    elif gsd.shape == fo.galaxyShape.disc:
        disc_galaxy_calc_positions(positions, adjacency_grid, gsd.size, width)
    elif gsd.shape == fo.galaxyShape.cluster:
        cluster_galaxy_calc_positions(positions, adjacency_grid, gsd.size, width)
    elif gsd.shape == fo.galaxyShape.ring:
        ring_galaxy_calc_positions(positions, adjacency_grid, gsd.size, width)
    elif gsd.shape == fo.galaxyShape.irregular:
        irregular_galaxy_calc_positions(positions, adjacency_grid, gsd.size, width)

    # Check if any positions have been calculated...
    if not positions:
        # ...if not, fall back on box shape
        box_galaxy_calc_positions(positions, adjacency_grid, gsd.size, width)

    enforce_max_distance(positions, adjacency_grid)

    # to avoid having too much "extra space" around the system positions of our galaxy map, recalculate the universe
    # width and shift all positions accordingly
    width, positions = recalc_universe_width(positions)
    print "Set universe width to {}".format(width)
    fo.set_universe_width(width)

    return positions
