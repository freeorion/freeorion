import random
import fo_universe_generator as fo
import util
import statistics


def generate_monsters(monster_freq, systems):
    """
    Adds space monsters to systems.
    """

    # first, calculate the basic chance for monster generation in a system
    # based on the monster frequency that has been passed
    # get the corresponding value for the specified monster frequency from the universe tables
    inverse_monster_chance = fo.monster_frequency(monster_freq)
    # as the value in the universe table is higher for a lower frequency, we have to invert it
    # exception: a value of 0 means no monsters, in this case return immediately
    if inverse_monster_chance <= 0:
        return
    basic_chance = 1.0 / float(inverse_monster_chance)
    print "Default monster spawn chance:", basic_chance

    # get all monster fleets that have a spawn rate and limit both > 0 and at least one monster ship design in it
    # (a monster fleet with no monsters in it is pointless) and store them with a spawn counter in a dict
    # this counter will be set to the spawn limit initially and decreased every time the monster fleet is spawned
    fleet_plans = {fp: fp.spawn_limit() for fp in fo.load_monster_fleet_plan_list("space_monster_spawn_fleets.txt")
                   if fp.spawn_rate() > 0.0 and fp.spawn_limit() > 0 and fp.ship_designs()}
    if not fleet_plans:
        return
    # dump a list of all monster fleets meeting these conditions and their properties to the log
    print "Monster fleets available for generation at game start:"
    for fleet_plan in fleet_plans:
        print "...", fleet_plan.name(), ": spawn rate", fleet_plan.spawn_rate(),\
              "/ spawn limit", fleet_plan.spawn_limit()

    # for each system in the list that has been passed to this function, find a monster fleet that can be spawned at
    # the system and which hasn't already been added too many times, then attempt to add that monster fleet by
    # testing the spawn rate chance
    for system in systems:
        # filter out all monster fleets whose location condition allows this system and whose counter hasn't reached 0
        suitable_fleet_plans = [fp for fp, counter in fleet_plans.iteritems() if counter and fp.location(system)]
        # if there are no suitable monster fleets for this system, continue with the next
        if not suitable_fleet_plans:
            continue

        # randomly select one monster fleet out of the suitable ones and then test if we want to add it to this system
        # by making a roll against the basic chance multiplied by the spawn rate of this monster fleet
        fleet_plan = random.choice(suitable_fleet_plans)
        if random.random() > basic_chance * fleet_plan.spawn_rate():
            # no, test failed, continue with the next system
            continue

        # all prerequisites and the test have been met, now spawn this monster fleet in this system
        print "Spawn", fleet_plan.name(), "at", fo.get_name(system)
        # decrement counter for this monster fleet
        fleet_plans[fleet_plan] -= 1
        # create monster fleet
        monster_fleet = fo.create_monster_fleet(system)
        # if fleet creation fails, report an error and try to continue with next system
        if monster_fleet == fo.invalid_object():
            util.report_error("Python generate_monsters: unable to create new monster fleet %s" % fleet_plan.name())
            continue
        # add monsters to fleet
        for design in fleet_plan.ship_designs():
            # create monster, if creation fails, report an error and try to continue with the next design
            if fo.create_monster(design, monster_fleet) == fo.invalid_object():
                util.report_error("Python generate_monsters: unable to create monster %s" % design)

    # finally, compile some statistics to be dumped to the log later
    statistics.monsters_summary = [(fp.name(), fp.spawn_limit() - counter) for fp, counter in fleet_plans.iteritems()]
