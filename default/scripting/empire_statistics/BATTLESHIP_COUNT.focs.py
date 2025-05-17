from focs._effects import Armed, OwnedBy, Ship, Source, StatisticCount
from focs._empire_statistics import EmpireStatistic

EmpireStatistic(
    name="BATTLESHIP_COUNT", value=StatisticCount(float, condition=Ship & Armed & OwnedBy(empire=Source.Owner))
)
