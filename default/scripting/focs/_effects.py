# Placeholder for module.
from typing import cast

from focs._effects import (
    LocalCandidate,
    StatisticIf,
)


def InGame():
    """Returns a condition which matches objects on the map

    :returns: a focs condition matching objects with non-negative ID
    """
    return cast(int, LocalCandidate.ID) >= 0


def StatisticElse(type_, *, condition):
    return 1 - StatisticIf(type_, condition=condition)
