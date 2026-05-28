from focs._effects import (
    Armed,
    OwnedBy,
    Ship,
    Source,
)
from focs._empire_statistics import EmpireStatistic
from focs._value_refs import (
    StatisticCount,
)

EmpireStatistic(
    name="BATTLESHIP_COUNT", value=StatisticCount(float, condition=Ship & Armed & OwnedBy(empire=Source.Owner))
)
