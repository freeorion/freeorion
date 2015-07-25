import sys
from random import random, uniform
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

    return True
