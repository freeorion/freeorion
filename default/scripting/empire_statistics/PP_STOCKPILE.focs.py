from focs._effects import ResourceIndustry
from focs._empire_statistics import EmpireStatistic
from focs._sources import Source
from focs._value_refs import (
    EmpireStockpile,
)

EmpireStatistic(name="PP_STOCKPILE", value=EmpireStockpile(empire=Source.Owner, resource=ResourceIndustry))
