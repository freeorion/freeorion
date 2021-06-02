import itertools
import math
import random
import sys
from hashlib import md5
from typing import Iterable, Iterator, Tuple

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


def distance(start, end):
    """
    Calculates linear distance between two coordinates.
    """
    x1, y1 = start
    x2, y2 = end
    return math.hypot(float(x1) - float(x2), float(y1) - float(y2))


def report_error(msg):
    """
    Handles error messages.
    """
    error_list.append(msg)
    print(msg, file=sys.stderr)


def unique_product(first: Iterable[int], second: Iterable[int]) -> Iterator[Tuple[int, int]]:
    """
    Create all possible unique pairs for two iterables.
    Both iterables should consist of comparable objects.
    Pair is sorted in ascending order.
    """
    return filter(lambda x: x[0] < x[1], itertools.product(first, second))


class MapGenerationError(RuntimeError):
    """Map generation runtime error."""
