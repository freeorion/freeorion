from focs._effects import LocalCandidate, OwnedBy, Source, Statistic, Sum
from focs._empire_statistics import EmpireStatistic

EmpireStatistic(
    name="IP_OUTPUT",
    value=Statistic(float, Sum, value=LocalCandidate.Influence, condition=OwnedBy(empire=Source.Owner)),
)
