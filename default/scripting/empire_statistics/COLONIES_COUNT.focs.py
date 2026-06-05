from focs._conditions import OwnedBy, Planet, Population
from focs._empire_statistics import EmpireStatistic
from focs._sources import Source
from focs._value_refs import (
    StatisticCount,
)

EmpireStatistic(
    name="COLONIES_COUNT",
    value=StatisticCount(float, condition=Planet() & Population(low=0.02) & OwnedBy(empire=Source.Owner)),
)
