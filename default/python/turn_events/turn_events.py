from common.configure_logging import redirect_logging_to_freeorion_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()

import sys
from random import random, uniform, randint, choice
from math import sin, cos, pi

import freeorion as fo
from universe_tables import MONSTER_FREQUENCY


def execute_turn_events():
    print("Executing turn events for turn", fo.current_turn())

    # creating fields
    systems = fo.get_systems()
    radius = fo.get_universe_width() / 2.0
    field_types = {
        1: "FLD_MOLECULAR_CLOUD",
        2: "FLD_ION_STORM",
        3: "FLD_NANITE_SWARM",
        4: "FLD_METEOR_BLIZZARD",
        5: "FLD_VOID_RIFT"
    }

    if random() < max(0.00015 * radius, 0.03):
        field_type = field_types.get(randint(1, 5), "FLD_ERROR")
        size = 5.0
        x = y = radius
        dist_from_center = uniform(0.35, 1.0) * radius
        angle = random() * 2.0 * pi
        x = radius + (dist_from_center * sin(angle))
        y = radius + (dist_from_center * cos(angle))

        print("...creating new", field_type, "field, at distance", dist_from_center, "from center")
        if fo.create_field(field_type, x, y, size) == fo.invalid_object():
            print("Turn events: couldn't create new field", file=sys.stderr)

    # creating monsters
    gsd = fo.get_galaxy_setup_data()
    monster_freq = MONSTER_FREQUENCY[gsd.monsterFrequency]
    # monster freq ranges from 1/30 (= one monster per 30 systems) to 1/3 (= one monster per 3 systems)
    # (example: low monsters and 150 Systems results in 150 / 30 * 0.01 = 0.05)
    if monster_freq > 0 and random() < len(systems) * monster_freq * 0.01:
        # only spawn Krill at the moment, other monsters can follow in the future
        if random() < 1:
            monster_type = "SM_KRILL_1"
        else:
            monster_type = "SM_FLOATER"

        # search for systems without planets or fleets
        candidates = [s for s in systems if len(fo.sys_get_planets(s)) <= 0 and len(fo.sys_get_fleets(s)) <= 0]
        if not candidates:
            print("Turn events: unable to find system for monster spawn", file=sys.stderr)
        else:
            system = choice(candidates)
            print("...creating new", monster_type, "at", fo.get_name(system))

            # create monster fleet
            monster_fleet = fo.create_monster_fleet(system)
            # if fleet creation fails, report an error
            if monster_fleet == fo.invalid_object():
                print("Turn events: unable to create new monster fleet", file=sys.stderr)
            else:
                # create monster, if creation fails, report an error
                monster = fo.create_monster(monster_type, monster_fleet)
                if monster == fo.invalid_object():
                    print("Turn events: unable to create monster in fleet", file=sys.stderr)

    return True
