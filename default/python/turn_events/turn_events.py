from common.configure_logging import redirect_logging_to_freeorion_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()

import freeorion as fo
import sys
from math import cos, pi, sin
from random import choice, random, uniform

from universe_tables import MONSTER_FREQUENCY


def execute_turn_events():
    print("Executing turn events for turn", fo.current_turn())

    # creating fields
    systems = fo.get_systems()
    radius = fo.get_universe_width() / 2.0
    field_types = ["FLD_MOLECULAR_CLOUD", "FLD_ION_STORM", "FLD_NANITE_SWARM", "FLD_CORROSIVE_NANITE_SWARM",
                   "FLD_METEOR_BLIZZARD", "FLD_VOID_RIFT"]

    if random() < max(0.00015 * radius, 0.03):
        field_type = choice(field_types)
        size = 5.0
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
    # monster freq ranges from 0.0 .. 1/30 (= one monster per 30 systems) .. 1/3 (= one monster per 3 systems) .. 99/100 (almost everywhere)
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
