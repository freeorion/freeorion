# Placeholder for module.
from focs._effects import (
    StatisticIf,
)


def StatisticElse(type_, *, condition):
    return 1 - StatisticIf(type_, condition=condition)
