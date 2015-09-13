import sys
import util
from random import random, uniform, choice
from math import sin, cos, pi, hypot

import freeorion as fo


def execute_turn_events():
    print "Executing turn events for turn", fo.current_turn()

    # creating fields
    systems = fo.get_systems()
    radius = fo.get_universe_width() / 2.0
    if random() < max(0.0003 * radius, 0.03):
        if random() < 0.4:
            field_type = "FLD_MOLECULAR_CLOUD"
            size = 5.0
        else:
            field_type = "FLD_ION_STORM"
            size = 5.0

        x = y = radius
        dist_from_center = 0.0
        while (dist_from_center < radius) or any(hypot(fo.get_x(s) - x, fo.get_y(s) - y) < 50.0 for s in systems):
            angle = random() * 2.0 * pi
            dist_from_center = radius + uniform(min(max(radius * 0.02, 10), 50.0), min(max(radius * 0.05, 20), 100.0))
            x = radius + (dist_from_center * sin(angle))
            y = radius + (dist_from_center * cos(angle))

        print "...creating new", field_type, "field, at distance", dist_from_center, "from center"
        if fo.create_field(field_type, x, y, size) == fo.invalid_object():
            print >> sys.stderr, "Turn events: couldn't create new field"

    gsd = fo.get_galaxy_setup_data()
    # creating monsters
    monster_freq = fo.monster_frequency(gsd.monsterFrequency)
    # monster freq ranges from 30 (= one monster per 30 systems) to 3 (= one monster per 3 systems)
    # (example: low monsters and 150 Systems results in 150 / 30 * 0.001 = 0.005)
    if monster_freq > 0 and random() < len(systems) / monster_freq * 0.001:
        if random() < 0.7:
            monster_type = "SM_KRILL_1"
        else:
            monster_type = "SM_FLOATER"

        print "...creating new", monster_type

        # search for systems without planets or fleets
        candidates = [s for s in systems if len(fo.sys_get_planets(s)) <= 0 and len(fo.sys_get_fleets(s)) <= 0]
        system = choice(candidates)
        # if system selection fails, report an error
        if system == fo.invalid_object():
            util.report_error("Python execute_turn_events: unable to find system for monster spawn")

        # create monster fleet
        monster_fleet = fo.create_monster_fleet(system)
        # if fleet creation fails, report an error
        if monster_fleet == fo.invalid_object():
            util.report_error("Python execute_turn_events: unable to create new monster fleet at %s" % system)
        else:
            # create monster, if creation fails, report an error
            monster = fo.create_monster(monster_type, monster_fleet)
            if monster == fo.invalid_object():
                util.report_error("Python execute_turn_events: unable to create monster %s" % monster_type)

    return True
