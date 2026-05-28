from focs._effects import (
    OwnedBy,
    Planet,
    Population,
    Source,
)
from focs._empire_statistics import EmpireStatistic
from focs._value_refs import (
    StatisticCount,
)

EmpireStatistic(
    name="COLONIES_COUNT",
    value=StatisticCount(float, condition=Planet() & Population(low=0.02) & OwnedBy(empire=Source.Owner)),
)
