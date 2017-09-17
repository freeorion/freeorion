import math
import random
import sys
from hashlib import md5

error_list = []


def int_hash(s):
    h = md5()
    h.update(s)
    return int(h.hexdigest(), 16)


def seed_rng(seed):
    """
    Seeds the default RNG with the specified seed
    """
    random.seed(seed)
    # The following jumpahead call can be uncommented to work around this issue of the Mersenne Twister PRNG
    # (quoted from wikipedia http://en.wikipedia.org/wiki/Mersenne_twister):
    # "It can take a long time to start generating output that passes randomness tests, if the initial state is highly
    # non-random-- particularly if the initial state has many zeros. A consequence of this is that two instances of the
    # generator, started with initial states that are almost the same, will usually output nearly the same sequence
    # for many iterations, before eventually diverging."
    # random.jumpahead(999999)


def distance((x1, y1), (x2, y2)):
    """
    Calculates linear distance between two coordinates.
    """
    return math.hypot(float(x1) - float(x2), float(y1) - float(y2))


def report_error(msg):
    """
    Handles error messages.
    """
    error_list.append(msg)
    print >> sys.stderr, msg


class MapGenerationError(RuntimeError):
    """Map generation runtime error."""
