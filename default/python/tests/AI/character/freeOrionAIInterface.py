# Fake fo for testing Trait purposes

# TODO Find a better way to import the static portions of fo without the test chaos, overhead and random state of fo.

# The following line can replace the contents of this file if
# the __init__.py in AI/freeorion_debug is an empty file
# and an empty __init__.py file is added to freeorion_debug/ide_tools/result/
# from freeorion_debug.ide_tools.result.freeOrionAIInterface import aggression, userString, userStringList


class aggression(object):
    """Aggression enumeration."""
    beginner = 0
    turtle = 1
    cautious = 2
    typical = 3
    aggressive = 4
    maniacal = 5


def userString(x):
    """userString mock"""
    return "UserString %s" % x


def userStringList(x):
    """userStringList mock"""
    return "UserStringList %s" % x
