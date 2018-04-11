# Mock module for tests


class aggression(object):
    """Aggression enumeration."""
    beginner = 0
    turtle = 1
    cautious = 2
    typical = 3
    aggressive = 4
    maniacal = 5


class planetSize(object):
    """PlanetSize enumeration."""
    tiny = 1
    small = 2
    medium = 3
    large = 4
    huge = 5
    asteroids = 6
    gasGiant = 7
    noWorld = 0
    unknown = -1


def userString(x):
    """userString mock"""
    return "UserString %s" % x


def userStringList(x):
    """userStringList mock"""
    return "UserStringList %s" % x
