from focs._effects import (
    ResourceInfluence,
    Source,
)
from focs._empire_statistics import EmpireStatistic
from focs._value_refs import (
    EmpireStockpile,
)

EmpireStatistic(name="IP_STOCKPILE", value=EmpireStockpile(empire=Source.Owner, resource=ResourceInfluence))
