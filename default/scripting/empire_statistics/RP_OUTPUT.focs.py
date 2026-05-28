from focs._effects import (
    LocalCandidate,
    OwnedBy,
    Source,
    Sum,
)
from focs._empire_statistics import EmpireStatistic
from focs._value_refs import (
    Statistic,
)

EmpireStatistic(
    name="RP_OUTPUT", value=Statistic(float, Sum, value=LocalCandidate.Research, condition=OwnedBy(empire=Source.Owner))
)
