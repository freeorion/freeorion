from focs._empire_statistics import EmpireStatistic
from focs._enums import ResourceInfluence
from focs._sources import Source
from focs._value_refs import (
    EmpireStockpile,
)

EmpireStatistic(name="IP_STOCKPILE", value=EmpireStockpile(empire=Source.Owner, resource=ResourceInfluence))
