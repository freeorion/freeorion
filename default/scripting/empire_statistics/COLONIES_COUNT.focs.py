from focs._effects import OwnedBy, Planet, Population, Source, StatisticCount
from focs._empire_statistics import EmpireStatistic

EmpireStatistic(
    name="COLONIES_COUNT",
    value=StatisticCount(float, condition=Planet() & Population(low=0.02) & OwnedBy(empire=Source.Owner)),
)
