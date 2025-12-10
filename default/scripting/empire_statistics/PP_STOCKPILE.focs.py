from focs._effects import EmpireStockpile, ResourceIndustry, Source
from focs._empire_statistics import EmpireStatistic

EmpireStatistic(name="PP_STOCKPILE", value=EmpireStockpile(empire=Source.Owner, resource=ResourceIndustry))
