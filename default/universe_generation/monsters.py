import sys
import random
import foUniverseGenerator as fo
import util


def generate_monsters(monster_freq, systems):
    """
    Adds space monsters to systems
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

    # get a list with all monster fleets that have a spawn rate and limit both > 0 and at least one monster
    # ship design in it (a monster fleet with no monsters in it is pointless)
    # as we can't rely on monster fleet names being unique, we nned to assign an unique id to each monster fleet by
    # enumerating the monster fleets we get and store them with their ids in a list of tuples
    # we can't use a Python dict with the fleet plans as keys in this case because the fleet plan objects are not
    # immutable
    fleet_plans = [(id, fp) for id, fp in enumerate(fo.load_monster_fleet_plan_list("space_monster_spawn_fleets.txt"))
                   if fp.spawn_rate() > 0.0 and fp.spawn_limit() > 0 and fp.ship_designs()]
    if not fleet_plans:
        return
    # dump a list of all monster fleets meeting these conditions and their properties to the log
    print "Monster fleets available for generation at game start:"
    for id, fleet_plan in fleet_plans:
        print "...", fleet_plan.name(), ": spawn rate", fleet_plan.spawn_rate(),\
              "/ spawn limit", fleet_plan.spawn_limit()

    # initialize count of how many of each monster fleet plan has been created
    # here we are going to use the ids we just assigned to the monster fleets
    monster_fleets_created = {id: 0 for id, fp in fleet_plans}

    # for each system in the list that has been passed to this function, find a monster whose location condition
    # allows the system, which hasn't already been added too many times, and then attempt to add that monster by
    # testing the spawn rate chance
    for system in systems:
        # for this system, find a suitable monster fleet plan
        # start by shuffling our monster fleet list, so each time
        # the monster fleets are considered in a new random order
        random.shuffle(fleet_plans)

        # then, consider each monster fleet until one has been found or we run out of monster fleets
        # (the latter case means that no monsters are added to this system)
        for fleet_plan_id, fleet_plan in fleet_plans:
            # check if the spawn limit for this monster fleet has already been reached (that is, if this monster fleet
            # has already been added the maximal allowed number of times)
            if monster_fleets_created[fleet_plan_id] >= fleet_plan.spawn_limit():
                # if yes, consider next monster fleet
                continue

            # check if this system matches the location condition for this monster fleet
            # (meaning, if this monster fleet can be added to this system at all)
            if not fleet_plan.location(system):
                # if not, consider next monster fleet
                continue

            # we have found a monster fleet that meets all prerequisites
            # now do the test if we want to add the selected monster fleet to this system by making a roll against
            # the basic chance multiplied by the spawn rate of the monster fleet
            if random.random() > basic_chance * fleet_plan.spawn_rate():
                # no, test failed, break out of the monster fleet loop and continue with the next system
                break

            # all prerequisites and the test have been met, now spawn this monster fleet in this system
            print "Spawn", fleet_plan.name(), "at", fo.get_name(system)
            # increase counter for this monster fleet
            monster_fleets_created[fleet_plan_id] += 1
            # create monster fleet
            monster_fleet = fo.create_monster_fleet(system)
            # if fleet creation fails, report an error and try to continue with next system
            if monster_fleet == fo.invalid_object():
                util.report_error("Python generate_monsters: unable to create new monster fleet %s" % fleet_plan.name())
                break
            # add monsters to fleet
            for design in fleet_plan.ship_designs():
                # create monster, if creation fails, report an error and try to continue with the next design
                if fo.create_monster(design, monster_fleet) == fo.invalid_object():
                    util.report_error("Python generate_monsters: unable to create monster %s" % design)

            #continue with next system
            break
