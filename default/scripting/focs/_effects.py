# Placeholder for module.
from focs._effects import (
    LocalCandidate,
    StatisticIf,
)


def InGame():
    """Returns a condition which matches objects on the map

    :returns: a focs condition matching objects with positive ID
    """
    return LocalCandidate.ID > 0


def StatisticElse(type_, *, condition):
    return 1 - StatisticIf(type_, condition=condition)
