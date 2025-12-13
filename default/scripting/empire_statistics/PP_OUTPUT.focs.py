from focs._effects import LocalCandidate, OwnedBy, Source, Statistic, Sum
from focs._empire_statistics import EmpireStatistic

EmpireStatistic(
    name="PP_OUTPUT", value=Statistic(float, Sum, value=LocalCandidate.Industry, condition=OwnedBy(empire=Source.Owner))
)
