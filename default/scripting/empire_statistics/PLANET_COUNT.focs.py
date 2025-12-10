from focs._effects import OwnedBy, Planet, Source, StatisticCount
from focs._empire_statistics import EmpireStatistic

EmpireStatistic(name="PLANET_COUNT", value=StatisticCount(float, condition=Planet() & OwnedBy(empire=Source.Owner)))
