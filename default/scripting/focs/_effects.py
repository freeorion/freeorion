# Placeholder for module.
from typing import cast

from focs._effects import (
    LocalCandidate,  # pyrefly: ignore  # import from self
    StatisticIf,  # pyrefly: ignore  # import from self
)


def InGame():
    """Returns a condition which matches objects on the map

    :returns: a focs condition matching objects with non-negative ID
    """
    return cast(int, LocalCandidate.ID) >= 0


def StatisticElse(type_, *, condition):
    return 1 - StatisticIf(type_, condition=condition)
