from focs._conditions import OwnedBy
from focs._empire_statistics import EmpireStatistic
from focs._enums import Sum
from focs._sources import LocalCandidate, Source
from focs._value_refs import (
    Statistic,
)

EmpireStatistic(
    name="PP_OUTPUT", value=Statistic(float, Sum, value=LocalCandidate.Industry, condition=OwnedBy(empire=Source.Owner))
)
