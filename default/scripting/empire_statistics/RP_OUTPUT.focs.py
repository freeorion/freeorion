from focs._effects import LocalCandidate, OwnedBy, Source, Statistic, Sum
from focs._empire_statistics import EmpireStatistic

EmpireStatistic(
    name="RP_OUTPUT", value=Statistic(float, Sum, value=LocalCandidate.Research, condition=OwnedBy(empire=Source.Owner))
)
