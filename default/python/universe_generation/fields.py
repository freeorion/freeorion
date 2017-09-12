from random import choice, sample, uniform

import freeorion as fo

from util import report_error


def generate_fields(systems):
    """
    Generates stationary fields in some randomly chosen empty no star systems.
    """
    # filter out all empty no star systems
    candidates = [s for s in systems if (fo.sys_get_star_type(s) == fo.starType.noStar) and (not fo.sys_get_planets(s))]
    # make sure we have at least one empty no star system, otherwise return without creating any fields
    if not candidates:
        print "...no empty no star systems found, no fields created"
        return
    # pick 10-20% of all empty no star systems to create stationary fields in them, but at least one
    accepted = sample(candidates, max(int(len(candidates) * uniform(0.1, 0.2)), 1))
    for system in accepted:
        # randomly pick a field type
        field_type = choice(["FLD_NEBULA_1", "FLD_NEBULA_2"])
        # and create the field
        if fo.create_field_in_system(field_type, uniform(40, 120), system) == fo.invalid_object():
            # create field failed, report an error
            report_error("Python generate_fields: create field %s in system %d failed" % (field_type, system))
    print "...fields created in %d systems out of %d empty no star systems" % (len(accepted), len(candidates))
