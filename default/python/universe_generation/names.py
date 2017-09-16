import random
from itertools import cycle

import freeorion as fo


# tuples of consonants and vowels for random name generation
consonants = ("b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r", "s", "t", "v", "w", "x", "y", "z")
vowels = ("a", "e", "i", "o", "u")


_random_letter_generator = (random.choice(x) for x in cycle((consonants, vowels)))


def random_name(size):
    """
    Return random name of given size.

    It rotates between consonants and vowels.
    Rotation is global first letter depends on prev calls.
    """
    return ''.join(next(_random_letter_generator) for _ in xrange(size)).capitalize()


def get_name_list(name_list):
    """
    Retrieves a list of names from the string tables.
    """
    return fo.user_string(name_list).splitlines()
