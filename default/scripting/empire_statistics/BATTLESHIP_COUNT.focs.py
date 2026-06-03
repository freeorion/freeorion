from focs._conditions import Armed, OwnedBy, Ship
from focs._effects import Source
from focs._empire_statistics import EmpireStatistic
from focs._value_refs import (
    StatisticCount,
)

EmpireStatistic(
    name="BATTLESHIP_COUNT", value=StatisticCount(float, condition=Ship & Armed & OwnedBy(empire=Source.Owner))
)
