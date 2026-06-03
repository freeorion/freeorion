from focs._conditions import OwnedBy
from focs._effects import LocalCandidate, Source, Sum
from focs._empire_statistics import EmpireStatistic
from focs._value_refs import (
    Statistic,
)

EmpireStatistic(
    name="IP_OUTPUT",
    value=Statistic(float, Sum, value=LocalCandidate.Influence, condition=OwnedBy(empire=Source.Owner)),
)
