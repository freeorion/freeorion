from functools import reduce
from itertools import chain
from typing import Iterable, Union


def _combine_ratings(rating1: float, rating2: float) -> float:
    """Combines two combat ratings to a total rating.

    The formula takes into account the fact that the combined strength of two ships is more than the
    sum of its individual ratings. Basic idea as follows:

    We use the following definitions

    r: rating
    a: attack
    s: structure

    where we take into account effective values after accounting for e.g. shields effects.

    We generally define the rating of a ship as
    r_i = a_i*s_i                                                                   (1)

    A natural extension for the combined rating of two ships is
    r_tot = (a_1+a_2)*(s_1+s_2)                                                     (2)

    Assuming         a_i approx s_i                                                 (3)
    It follows that  a_i approx sqrt(r_i) approx s_i                                (4)
    And thus         r_tot = (sqrt(r_1)+sqrt(r_2))^2 = r1 + r2 + 2*sqrt(r1*r2)      (5)

    Note that this function has commutative and associative properties.
    """
    return rating1 + rating2 + 2 * (rating1 * rating2) ** 0.5


def combine_ratings(first: Union[float, Iterable], *ratings: float) -> float:
    """
    Combine multiple ratings to one.
    """
    if isinstance(first, (int, float)):
        first = (first,)
    return reduce(_combine_ratings, chain(first, ratings), 0.0)
