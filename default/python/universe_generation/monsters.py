import random

import freeorion as fo

import universe_statistics
import universe_tables
from galaxy import DisjointSets
from util import MapGenerationError, report_error


class StarlaneAlteringMonsters(object):
    def __init__(self, systems):
        self.systems = systems
        self.placed = set()
        self.removed_lanes = set()
        self.starlanes = [(s1, s2) for s1 in systems for s2 in fo.sys_get_starlanes(s1) if s1 < s2]

    def can_place_at(self, system, plan):
        """
        Check if the starlane altering monster fleet ''plan'' can be placed at ''system''
        without disjoining the galaxy map.
        Compute the disjoint set of the galaxy without the starlanes
        from the proposed system.  Return False if there will be more
        connected regions than the number of placed starlane altering
        monsters plus one, otherwise True.
        """
        local_lanes = {(min(system, s), max(system, s)) for s in fo.sys_get_starlanes(system)}

        dsets = DisjointSets()
        for s in self.systems:
            dsets.add(s)

        for lane in self.starlanes:
            if lane in self.removed_lanes or lane in local_lanes:
                continue
            dsets.link(lane[0], lane[1])

        num_contiguous_regions = len(dsets.complete_sets())
        expected_num_contiguous_regions = (len(self.placed) + 2)

        if num_contiguous_regions > expected_num_contiguous_regions:
            return False

        if num_contiguous_regions < expected_num_contiguous_regions:
            report_error("Number of contiguous regions %d is below the expected number "
                         "of contiguous regions %d when placing %d monster %s that can "
                         "break starlanes."
                         % (num_contiguous_regions, expected_num_contiguous_regions,
                            len(self.placed) + 1, plan.name()))
            return False

        return True

    def place(self, system, plan):
        """
        Create map altering monster fleet ''plan'' at ''system''.
        """
        local_lanes = {(min(system, s), max(system, s)) for s in fo.sys_get_starlanes(system)}

        populate_monster_fleet(plan, system)
        if system not in self.placed:
            self.placed.add(system)
            self.removed_lanes |= local_lanes


def populate_monster_fleet(fleet_plan, system):
    """
    Create a monster fleet in ''system'' according to ''fleet_plan''
    """

    # create monster fleet
    monster_fleet = fo.create_monster_fleet(system)

    if monster_fleet == fo.invalid_object():
        raise MapGenerationError("Python generate_monsters: unable to create new monster fleet %s"
                                 % fleet_plan.name())

    # add monsters to fleet
    for design in fleet_plan.ship_designs():
        if fo.create_monster(design, monster_fleet) == fo.invalid_object():
            raise MapGenerationError("Python generate_monsters: unable to create monster %s" % design)

    print "Spawn", fleet_plan.name(), "at", fo.get_name(system)


def generate_monsters(monster_freq, systems):
    """
    Adds space monsters to systems.
    """
    # first, calculate the basic chance for monster generation in a system
    # based on the monster frequency that has been passed
    # get the corresponding value for the specified monster frequency from the universe tables
    basic_chance = universe_tables.MONSTER_FREQUENCY[monster_freq]
    # a value of 0 means no monsters, in this case return immediately
    if basic_chance <= 0:
        return
    print "Default monster spawn chance:", basic_chance
    expectation_tally = 0.0
    actual_tally = 0

    # get all monster fleets that have a spawn rate and limit both > 0 and at least one monster ship design in it
    # (a monster fleet with no monsters in it is pointless) and store them in a list
    fleet_plans = fo.load_monster_fleet_plan_list()

    # create a map where we store a spawn counter for each monster fleet
    # this counter will be set to the spawn limit initially and decreased every time the monster fleet is spawned
    # this map (dict) needs to be separate from the list holding the fleet plans because the order in which items
    # are stored in a dict is undefined (can be different each time), which would result in different distribution
    # even when using the same seed for the RNG
    spawn_limits = {fp: fp.spawn_limit() for fp in fleet_plans
                    if fp.spawn_rate() > 0.0 and fp.spawn_limit() > 0 and fp.ship_designs()}

    # map nests to monsters for ease of reporting
    nest_name_map = {"KRAKEN_NEST_SPECIAL": "SM_KRAKEN_1",
                     "SNOWFLAKE_NEST_SPECIAL": "SM_SNOWFLAKE_1",
                     "JUGGERNAUT_NEST_SPECIAL": "SM_JUGGERNAUT_1"}
    tracked_plan_tries = {name: 0 for name in nest_name_map.values()}
    tracked_plan_counts = {name: 0 for name in nest_name_map.values()}
    tracked_plan_valid_locations = {fp: 0 for fp in fleet_plans if fp.name() in tracked_plan_counts}

    if not fleet_plans:
        return

    universe = fo.get_universe()

    # Fleet plans that include ships capable of altering starlanes.
    # @content_tag{CAN_ALTER_STARLANES} universe_generator special handling
    # for fleets containing a hull design with this tag.
    fleet_can_alter_starlanes = {fp for fp in fleet_plans
                                 if any([universe.getGenericShipDesign(design).hull_type.hasTag("CAN_ALTER_STARLANES")
                                         for design in fp.ship_designs()])}

    # dump a list of all monster fleets meeting these conditions and their properties to the log
    print "Monster fleets available for generation at game start:"
    fp_location_cache = {}
    for fleet_plan in fleet_plans:
        print "...", fleet_plan.name(), ": spawn rate", fleet_plan.spawn_rate(),
        print "/ spawn limit", fleet_plan.spawn_limit(),
        print "/ effective chance", basic_chance * fleet_plan.spawn_rate(),
        fp_location_cache[fleet_plan] = set(fleet_plan.locations(systems))
        print ("/ can be spawned at", len(fp_location_cache[fleet_plan]),
               "of", len(systems), "systems")
        if fleet_plan.name() in nest_name_map.values():
            universe_statistics.tracked_monsters_chance[fleet_plan.name()] = basic_chance * fleet_plan.spawn_rate()

    # initialize a manager for monsters that can alter the map
    # required to prevent their placement from disjoining the map
    starlane_altering_monsters = StarlaneAlteringMonsters(systems)

    # collect info for tracked monster nest valid locations
    planets = [p for s in systems for p in fo.sys_get_planets(s)]
    tracked_nest_valid_locations = {nest: len(fo.special_locations(nest, planets))
                                    for nest in nest_name_map}

    # for each system in the list that has been passed to this function, find a monster fleet that can be spawned at
    # the system and which hasn't already been added too many times, then attempt to add that monster fleet by
    # testing the spawn rate chance
    random.shuffle(systems)
    for system in systems:
        # collect info for tracked monster valid locations
        for fp in tracked_plan_valid_locations:
            if system in fp_location_cache[fp]:
                tracked_plan_valid_locations[fp] += 1

        # filter out all monster fleets whose location condition allows this system and whose counter hasn't reached 0.
        suitable_fleet_plans = [fp for fp in fleet_plans
                                if system in fp_location_cache[fp]
                                and spawn_limits.get(fp, 0)
                                and (fp not in fleet_can_alter_starlanes
                                     or starlane_altering_monsters.can_place_at(system, fp))]
        # if there are no suitable monster fleets for this system, continue with the next
        if not suitable_fleet_plans:
            continue

        # randomly select one monster fleet out of the suitable ones and then test if we want to add it to this system
        # by making a roll against the basic chance multiplied by the spawn rate of this monster fleet
        expectation_tally += basic_chance * sum([fp.spawn_rate()
                                                 for fp in suitable_fleet_plans]) / len(suitable_fleet_plans)
        fleet_plan = random.choice(suitable_fleet_plans)
        if fleet_plan.name() in tracked_plan_tries:
            tracked_plan_tries[fleet_plan.name()] += 1
        if random.random() > basic_chance * fleet_plan.spawn_rate():
            print("\t\t At system %4d rejected monster fleet %s from %d suitable fleets"
                  % (system, fleet_plan.name(), len(suitable_fleet_plans)))
            # no, test failed, continue with the next system
            continue
        actual_tally += 1
        if fleet_plan.name() in tracked_plan_counts:
            tracked_plan_counts[fleet_plan.name()] += 1

        # all prerequisites and the test have been met, now spawn this monster fleet in this system
        # create monster fleet
        try:
            if fleet_plan in fleet_can_alter_starlanes:
                starlane_altering_monsters.place(system, fleet_plan)
            else:
                populate_monster_fleet(fleet_plan, system)
            # decrement counter for this monster fleet
            spawn_limits[fleet_plan] -= 1

        except MapGenerationError as err:
            report_error(str(err))
            continue

    print "Actual # monster fleets placed: %d; Total Placement Expectation: %.1f" % (actual_tally, expectation_tally)
    # finally, compile some statistics to be dumped to the log later
    universe_statistics.monsters_summary = [
        (fp.name(), fp.spawn_limit() - counter) for fp, counter in spawn_limits.iteritems()
    ]
    universe_statistics.tracked_monsters_tries.update(tracked_plan_tries)
    universe_statistics.tracked_monsters_summary.update(tracked_plan_counts)
    universe_statistics.tracked_monsters_location_summary.update(
        (fp.name(), count) for fp, count in tracked_plan_valid_locations.iteritems())
    universe_statistics.tracked_nest_location_summary.update(
        (nest_name_map[nest], count) for nest, count in tracked_nest_valid_locations.items())
