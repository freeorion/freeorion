from focs._effects import (
    OwnedBy,
    Planet,
    Source,
)
from focs._empire_statistics import EmpireStatistic
from focs._value_refs import (
    StatisticCount,
)

EmpireStatistic(name="PLANET_COUNT", value=StatisticCount(float, condition=Planet() & OwnedBy(empire=Source.Owner)))
