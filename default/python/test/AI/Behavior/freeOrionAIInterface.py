# Fake fo for testing Behavior purposes

# TODO Find a better way to import the static portions of fo without the test chaos, overhead and random state of fo.

class aggression(object):
    """Aggression enumeration."""
    beginner=0
    turtle=1
    cautious=2
    typical=3
    aggressive=4
    maniacal=5

def userString(x):
    """userString mock"""
    return "UserString %s" % x

def userStringList(x):
    """userStringList mock"""
    return "UserStringList %s" % x
