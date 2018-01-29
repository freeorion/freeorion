import freeOrionAIInterface


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


freeOrionAIInterface.aggression = aggression
freeOrionAIInterface.userString = userString
freeOrionAIInterface.userStringList = userStringList
