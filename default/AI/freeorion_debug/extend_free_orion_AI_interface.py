"""
This module extend freeOrionAIInterface from python side.
Check that you have import of this module from FreeOrionAI.py


Adding to string:
=================

Adds object string methods to enable output as
{unique_object_prefix}{id}<{name}>
"P(1)<Sirius>" would be for the planet with id=1 and name=Sirius
This will help to track objects around logs.

str vs repr:
============

If you print item or %s it __str__ will be called if you repr, print list of items or %r __repr__ will be called.

Code organisation:
==================

keep fo.<attr> in same block

def planet_to_string():
    pass


def planet_repr():
    pass

fo.planet.__str__ = planet_to_string
fo.planet.__repr__ = planet_repr


Logger:
=======
wrap any fo function or method to logger to get more info about it.
don't commit logger wrapped function to repo.

"""

import freeOrionAIInterface as fo
from freeorion_tools import dict_from_map


PLANET = 'P'
SYSTEM = 'S'
FLEET = 'F'
SHIP_DESIGN = 'D'


def to_map(method):
    def wrapper(*args):
        return dict_from_map(method(*args))
    return wrapper

fo.universe.getVisibilityTurnsMap = to_map(fo.universe.getVisibilityTurnsMap)


def to_str(prefix, id, name):
    return '{}_{}<{}>'.format(prefix, id, name)

fo.to_str = to_str

def system_to_string(system):
    return to_str(SYSTEM, system.systemID, system.name)

fo.system.__repr__ = system_to_string

def design_to_string(design):
    return to_str(SHIP_DESIGN, design.id, design.name(True))
fo.shipDesign.__repr__ = design_to_string


def planet_to_string(planet):
    return to_str(PLANET, planet.id, planet.name)

fo.planet.__repr__ = planet_to_string


def fleet_to_string(fleet):
    return to_str(FLEET, fleet.id, fleet.name)


fo.fleet.__repr__ = fleet_to_string


def set_to_string(val):
    return '{%s}' % ', '.join(map(str, val))


fo.StringSet.__str__ = set_to_string
fo.IntSet.__str__ = set_to_string


def int_int_map_to_string(int_int_map):
    return str(dict_from_map(int_int_map))


fo.IntIntMap.__str__ = int_int_map_to_string


def logger(function):
    def inner(*args, **kwargs):
        print "%s(*%s, **%s)" % (function.__name__, args, kwargs)
        return function(*args, **kwargs)
    return inner
